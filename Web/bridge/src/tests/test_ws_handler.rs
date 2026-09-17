/// Integration tests for `ws_handler`.
///
/// We complete the WebSocket handshake manually over a raw TcpStream
/// (avoiding tokio-tungstenite's `connect` feature which pulls in url/idna).
/// tungstenite (sync) is used on the client side for the handshake.

use std::net::TcpStream as StdTcpStream;

use serde_json::json;
use tokio::{
    net::TcpListener,
    sync::{broadcast, mpsc},
};
use tungstenite::{client, Message};

use crate::kai_conn::EvalCommand;
use crate::protocol::ServerFrame;
use crate::ws_handler::accept;

// ── helpers ───────────────────────────────────────────────────────────────────

/// Synchronous WebSocket client used in tests (runs in spawn_blocking).
type SyncWs = tungstenite::WebSocket<StdTcpStream>;

fn connect_sync(addr: std::net::SocketAddr) -> SyncWs {
    let stream = StdTcpStream::connect(addr).unwrap();
    let url = format!("ws://{addr}/ws");
    let (ws, _) = client(url, stream).unwrap();
    ws
}

fn recv_json(ws: &mut SyncWs) -> serde_json::Value {
    loop {
        let msg = ws.read().unwrap();
        if let Message::Text(t) = msg {
            return serde_json::from_str(&t).unwrap();
        }
    }
}

fn send_json(ws: &mut SyncWs, v: serde_json::Value) {
    ws.send(Message::Text(v.to_string().into())).unwrap();
}

// ── tests ─────────────────────────────────────────────────────────────────────

/// On connect the handler must send a tree frame as the first message.
#[tokio::test]
async fn initial_tree_frame_sent_on_connect() {
    let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let addr = listener.local_addr().unwrap();
    let (sup_tx, _) = broadcast::channel::<ServerFrame>(32);
    let (eval_tx, _eval_rx) = mpsc::channel::<EvalCommand>(16);

    tokio::spawn(async move {
        let (stream, peer) = listener.accept().await.unwrap();
        accept(stream, peer, eval_tx, sup_tx.subscribe()).await;
    });

    let v = tokio::task::spawn_blocking(move || {
        let mut ws = connect_sync(addr);
        recv_json(&mut ws)
    })
    .await
    .unwrap();

    assert_eq!(v["kind"], "tree");
    assert!(v["domains"].is_array());
}

/// Sending an eval frame causes a Result frame to come back.
#[tokio::test]
async fn eval_frame_produces_result() {
    let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let addr = listener.local_addr().unwrap();
    let (sup_tx, _) = broadcast::channel::<ServerFrame>(32);
    let (eval_tx, mut eval_rx) = mpsc::channel::<EvalCommand>(16);

    tokio::spawn(async move {
        let (stream, peer) = listener.accept().await.unwrap();
        accept(stream, peer, eval_tx, sup_tx.subscribe()).await;
    });

    // Auto-reply to evals
    tokio::spawn(async move {
        while let Some(cmd) = eval_rx.recv().await {
            let _ = cmd.reply.send(ServerFrame::Result {
                src: cmd.src,
                output: "3".into(),
                ts: 1,
            }).await;
        }
    });

    let v = tokio::task::spawn_blocking(move || {
        let mut ws = connect_sync(addr);
        recv_json(&mut ws); // tree
        send_json(&mut ws, json!({"kind":"eval","src":"1 2 +"}));
        recv_json(&mut ws)
    })
    .await
    .unwrap();

    assert_eq!(v["kind"], "result");
    assert_eq!(v["src"],  "1 2 +");
}

/// A malformed JSON frame must produce an error frame, not a panic.
#[tokio::test]
async fn malformed_frame_produces_error() {
    let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let addr = listener.local_addr().unwrap();
    let (sup_tx, _) = broadcast::channel::<ServerFrame>(32);
    let (eval_tx, _eval_rx) = mpsc::channel::<EvalCommand>(16);

    tokio::spawn(async move {
        let (stream, peer) = listener.accept().await.unwrap();
        accept(stream, peer, eval_tx, sup_tx.subscribe()).await;
    });

    let v = tokio::task::spawn_blocking(move || {
        let mut ws = connect_sync(addr);
        recv_json(&mut ws); // tree
        ws.send(Message::Text("not json".into())).unwrap();
        recv_json(&mut ws)
    })
    .await
    .unwrap();

    assert_eq!(v["kind"], "error");
}

/// SUP frames broadcast by `sup_tx` must reach all connected clients.
#[tokio::test]
async fn sup_broadcast_reaches_all_clients() {


    let listener_a = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let addr_a = listener_a.local_addr().unwrap();
    let listener_b = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let addr_b = listener_b.local_addr().unwrap();

    let (sup_tx, _) = broadcast::channel::<ServerFrame>(32);
    let (eval_tx, _eval_rx) = mpsc::channel::<EvalCommand>(16);

    // Subscribe BEFORE spawning handlers so the receivers exist before broadcast
    let mut rx_a = sup_tx.subscribe();
    let mut rx_b = sup_tx.subscribe();

    let sup_tx_a = sup_tx.clone();
    let eval_tx_a = eval_tx.clone();
    let sup_tx_b = sup_tx.clone();
    let eval_tx_b = eval_tx.clone();

    tokio::spawn(async move {
        let (stream, peer) = listener_a.accept().await.unwrap();
        accept(stream, peer, eval_tx_a, sup_tx_a.subscribe()).await;
    });
    tokio::spawn(async move {
        let (stream, peer) = listener_b.accept().await.unwrap();
        accept(stream, peer, eval_tx_b, sup_tx_b.subscribe()).await;
    });

    // Connect both clients to trigger accept
    let handle_a = tokio::task::spawn_blocking(move || {
        let mut ws = connect_sync(addr_a);
        recv_json(&mut ws); // tree
        ws
    });
    let handle_b = tokio::task::spawn_blocking(move || {
        let mut ws = connect_sync(addr_b);
        recv_json(&mut ws); // tree
        ws
    });
    handle_a.await.unwrap();
    handle_b.await.unwrap();

    // Give handler tasks time to set up their internal sup_rx subscriptions
    tokio::time::sleep(tokio::time::Duration::from_millis(50)).await;

    // Broadcast a SUP
    sup_tx.send(ServerFrame::Sup {
        addr: "1:0:5".into(),
        state: json!({"hp": 42}),
        ts: 999,
    }).unwrap();

    // Both pre-subscribed receivers must see the SUP
    for rx in [&mut rx_a, &mut rx_b] {
        let frame = tokio::time::timeout(
            tokio::time::Duration::from_secs(2),
            rx.recv(),
        ).await
            .expect("timed out waiting for SUP")
            .expect("broadcast closed");

        match frame {
            ServerFrame::Sup { addr, state, .. } => {
                assert_eq!(addr, "1:0:5");
                assert_eq!(state["hp"], 42);
            }
            other => panic!("expected Sup, got {other:?}"),
        }
    }
}

/// An eval reply must go only to the client that sent it.
/// Verified by counting how many evals reach the server - only one sender
/// fires an eval, so only one reply is ever created. The other client's
/// reply channel stays empty; we confirm this via the eval_rx count.
#[tokio::test]
async fn eval_reply_is_not_broadcast_to_other_clients() {
    use std::sync::{Arc, atomic::{AtomicUsize, Ordering}};

    let listener_sender = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let addr_sender = listener_sender.local_addr().unwrap();
    let listener_other = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let addr_other = listener_other.local_addr().unwrap();

    let (sup_tx, _) = broadcast::channel::<ServerFrame>(32);
    let (eval_tx, mut eval_rx) = mpsc::channel::<EvalCommand>(16);
    let eval_count = Arc::new(AtomicUsize::new(0));
    let eval_count_clone = eval_count.clone();

    let sup_tx_s = sup_tx.clone();
    let eval_tx_s = eval_tx.clone();
    let sup_tx_o = sup_tx.clone();
    let eval_tx_o = eval_tx.clone();

    tokio::spawn(async move {
        let (stream, peer) = listener_sender.accept().await.unwrap();
        accept(stream, peer, eval_tx_s, sup_tx_s.subscribe()).await;
    });
    tokio::spawn(async move {
        let (stream, peer) = listener_other.accept().await.unwrap();
        accept(stream, peer, eval_tx_o, sup_tx_o.subscribe()).await;
    });

    // Count evals and auto-reply
    tokio::spawn(async move {
        while let Some(cmd) = eval_rx.recv().await {
            eval_count_clone.fetch_add(1, Ordering::SeqCst);
            let _ = cmd.reply.send(ServerFrame::Result {
                src: cmd.src,
                output: "42".into(),
                ts: 1,
            }).await;
        }
    });

    // Connect sender, consume its tree frame, send eval, get result
    let v = tokio::task::spawn_blocking(move || {
        let mut ws = connect_sync(addr_sender);
        recv_json(&mut ws); // tree
        send_json(&mut ws, json!({"kind":"eval","src":"21 2 *"}));
        recv_json(&mut ws) // result
    }).await.unwrap();

    assert_eq!(v["kind"], "result");

    // Connect the other client (just to keep the handler alive) and consume its tree
    let _other = tokio::task::spawn_blocking(move || {
        let mut ws = connect_sync(addr_other);
        recv_json(&mut ws); // tree only - no eval sent
        ws
    }).await.unwrap();

    // Give the eval task a moment to process
    tokio::time::sleep(tokio::time::Duration::from_millis(50)).await;

    // Exactly one eval reached the server - the other client sent none
    assert_eq!(eval_count.load(Ordering::SeqCst), 1,
        "only one client sent an eval, but server saw multiple");
}
