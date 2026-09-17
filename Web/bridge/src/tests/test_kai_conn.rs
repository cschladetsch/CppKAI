/// Integration tests for `kai_conn` eval/reply routing.
///
/// We spin up a real tokio TcpListener to act as a mock KAI process,
/// then drive `kai_conn::spawn` against it and assert on the frames
/// that come back through the reply channel.

use tokio::{
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader},
    net::TcpListener,
    sync::{broadcast, mpsc},
    time::{timeout, Duration},
};

use crate::kai_conn::{spawn, EvalCommand};
use crate::protocol::ServerFrame;

// ── helpers ───────────────────────────────────────────────────────────────────

/// Bind a listener on an OS-assigned port and return (addr_string, listener).
async fn ephemeral_listener() -> (String, TcpListener) {
    let l = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let addr = l.local_addr().unwrap().to_string();
    (addr, l)
}

const TIMEOUT: Duration = Duration::from_secs(5);

// ── tests ─────────────────────────────────────────────────────────────────────

/// Send one eval, mock KAI responds with a RESULT line.
/// The reply channel must receive a ServerFrame::Result with matching src.
#[tokio::test]
async fn eval_reply_routes_to_sender() {
    let (addr, listener) = ephemeral_listener().await;
    let (sup_tx, _sup_rx) = broadcast::channel::<ServerFrame>(16);

    let eval_tx = spawn(addr, sup_tx).await;

    // Mock KAI: accept one connection, read one line, respond with RESULT
    tokio::spawn(async move {
        let (stream, _) = listener.accept().await.unwrap();
        let (reader, mut writer) = stream.into_split();
        let mut lines = BufReader::new(reader).lines();

        if let Ok(Some(_cmd)) = lines.next_line().await {
            writer.write_all(b"RESULT 3\n").await.unwrap();
        }
    });

    // Give the bridge task time to connect
    tokio::time::sleep(Duration::from_millis(50)).await;

    let (reply_tx, mut reply_rx) = mpsc::channel::<ServerFrame>(4);
    eval_tx
        .send(EvalCommand { src: "1 2 +".into(), reply: reply_tx })
        .await
        .unwrap();

    let frame = timeout(TIMEOUT, reply_rx.recv()).await
        .expect("timed out")
        .expect("channel closed");

    match frame {
        ServerFrame::Result { src, output, .. } => {
            assert_eq!(src, "1 2 +");
            assert_eq!(output, "RESULT 3");
        }
        other => panic!("expected Result, got {other:?}"),
    }
}

/// A SUP line from KAI must be broadcast, not sent to the pending eval reply.
#[tokio::test]
async fn sup_line_is_broadcast_not_routed_to_eval() {
    let (addr, listener) = ephemeral_listener().await;
    let (sup_tx, mut sup_rx) = broadcast::channel::<ServerFrame>(16);

    let eval_tx = spawn(addr, sup_tx).await;

    tokio::spawn(async move {
        let (stream, _) = listener.accept().await.unwrap();
        let (reader, mut writer) = stream.into_split();
        let mut lines = BufReader::new(reader).lines();

        // Wait for the eval, but first emit a SUP
        if lines.next_line().await.is_ok() {
            writer
                .write_all(b"SUP 1:0:42 {\"x\":1}\nRESULT 3\n")
                .await
                .unwrap();
        }
    });

    tokio::time::sleep(Duration::from_millis(50)).await;

    let (reply_tx, mut reply_rx) = mpsc::channel::<ServerFrame>(4);
    eval_tx
        .send(EvalCommand { src: "1 2 +".into(), reply: reply_tx })
        .await
        .unwrap();

    // The broadcast channel should receive the SUP
    let sup_frame = timeout(TIMEOUT, sup_rx.recv()).await
        .expect("timed out waiting for SUP broadcast")
        .expect("broadcast channel error");

    match sup_frame {
        ServerFrame::Sup { addr, .. } => assert_eq!(addr, "1:0:42"),
        other => panic!("expected Sup, got {other:?}"),
    }

    // The reply channel should receive the RESULT (not the SUP)
    let reply_frame = timeout(TIMEOUT, reply_rx.recv()).await
        .expect("timed out waiting for RESULT reply")
        .expect("reply channel closed");

    assert!(matches!(reply_frame, ServerFrame::Result { .. }));
}

/// When KAI closes the connection, the bridge task should not send an eval
/// result (the reply channel simply closes / goes idle). This verifies no panic.
#[tokio::test]
async fn kai_disconnect_does_not_panic() {
    let (addr, listener) = ephemeral_listener().await;
    let (sup_tx, _) = broadcast::channel::<ServerFrame>(16);

    let eval_tx = spawn(addr, sup_tx).await;

    // Mock KAI: accept then immediately drop the connection
    tokio::spawn(async move {
        let (_stream, _) = listener.accept().await.unwrap();
        // drop stream → connection closed
    });

    tokio::time::sleep(Duration::from_millis(100)).await;

    // Sending an eval after KAI has gone should not panic the bridge task
    let (reply_tx, _reply_rx) = mpsc::channel::<ServerFrame>(4);
    let _ = eval_tx
        .send(EvalCommand { src: "1".into(), reply: reply_tx })
        .await;

    // Give the bridge a moment to handle the broken connection
    tokio::time::sleep(Duration::from_millis(100)).await;
    // If we're still here, no panic occurred
}

/// Eval replies go to the correct client when two evals are sent sequentially.
#[tokio::test]
async fn sequential_evals_reply_to_correct_senders() {
    let (addr, listener) = ephemeral_listener().await;
    let (sup_tx, _) = broadcast::channel::<ServerFrame>(16);

    let eval_tx = spawn(addr, sup_tx).await;

    tokio::spawn(async move {
        let (stream, _) = listener.accept().await.unwrap();
        let (reader, mut writer) = stream.into_split();
        let mut lines = BufReader::new(reader).lines();

        while let Ok(Some(line)) = lines.next_line().await {
            // Echo back RESULT <line> so we can distinguish responses
            let response = format!("RESULT {line}\n");
            writer.write_all(response.as_bytes()).await.unwrap();
        }
    });

    tokio::time::sleep(Duration::from_millis(50)).await;

    let (tx1, mut rx1) = mpsc::channel::<ServerFrame>(4);
    let (tx2, mut rx2) = mpsc::channel::<ServerFrame>(4);

    eval_tx
        .send(EvalCommand { src: "first".into(), reply: tx1 })
        .await
        .unwrap();

    let f1 = timeout(TIMEOUT, rx1.recv()).await.unwrap().unwrap();
    match f1 {
        ServerFrame::Result { src, .. } => assert_eq!(src, "first"),
        other => panic!("expected Result for first eval, got {other:?}"),
    }

    eval_tx
        .send(EvalCommand { src: "second".into(), reply: tx2 })
        .await
        .unwrap();

    let f2 = timeout(TIMEOUT, rx2.recv()).await.unwrap().unwrap();
    match f2 {
        ServerFrame::Result { src, .. } => assert_eq!(src, "second"),
        other => panic!("expected Result for second eval, got {other:?}"),
    }
}
