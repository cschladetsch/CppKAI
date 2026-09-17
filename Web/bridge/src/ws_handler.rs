use std::net::SocketAddr;

use futures_util::{SinkExt, StreamExt};
use tokio::{net::TcpStream, sync::{broadcast, mpsc}};
use tokio_tungstenite::{accept_async, tungstenite::Message};
use tracing::{debug, error, info, warn};

use crate::{
    kai_conn::EvalCommand,
    protocol::{BrowserFrame, ServerFrame, now_ms},
};

pub async fn accept(
    stream: TcpStream,
    peer: SocketAddr,
    eval_tx: mpsc::Sender<EvalCommand>,
    sup_rx: broadcast::Receiver<ServerFrame>,
) {
    // Handshake
    let ws = match accept_async(stream).await {
        Ok(ws) => ws,
        Err(e) => { error!("{peer} WS handshake failed: {e}"); return; }
    };

    info!("Browser connected: {peer}");
    if let Err(e) = handle(ws, peer, eval_tx, sup_rx).await {
        error!("ws_handler error for {peer}: {e}");
    }
    info!("Browser disconnected: {peer}");
}

async fn handle(
    ws: tokio_tungstenite::WebSocketStream<TcpStream>,
    peer: SocketAddr,
    eval_tx: mpsc::Sender<EvalCommand>,
    mut sup_rx: broadcast::Receiver<ServerFrame>,
) -> anyhow::Result<()> {
    let (mut ws_tx, mut ws_rx) = ws.split();

    // Per-client channel for KAI eval replies
    let (reply_tx, mut reply_rx) = mpsc::channel::<ServerFrame>(16);

    // Send initial empty tree on connect
    let tree = ServerFrame::Tree { domains: vec![] };
    ws_tx.send(Message::Text(serde_json::to_string(&tree)?)).await?;

    loop {
        tokio::select! {
            msg = ws_rx.next() => {
                match msg {
                    Some(Ok(Message::Text(text))) => {
                        debug!("{peer} → {text}");
                        match serde_json::from_str::<BrowserFrame>(&text) {
                            Ok(BrowserFrame::Eval { src, lang: _ }) => {
                                eval_tx.send(EvalCommand {
                                    src,
                                    reply: reply_tx.clone(),
                                }).await?;
                            }
                            Err(e) => {
                                let err = ServerFrame::Error {
                                    msg: format!("Bad frame: {e}"),
                                    ts: now_ms(),
                                };
                                ws_tx.send(Message::Text(serde_json::to_string(&err)?)).await?;
                            }
                        }
                    }
                    Some(Ok(Message::Close(_))) | None => break,
                    Some(Ok(_)) => {} // ping/pong/binary
                    Some(Err(e)) => { warn!("{peer} ws error: {e}"); break; }
                }
            }

            reply = reply_rx.recv() => {
                if let Some(frame) = reply {
                    ws_tx.send(Message::Text(serde_json::to_string(&frame)?)).await?;
                }
            }

            sup = sup_rx.recv() => {
                match sup {
                    Ok(frame) => {
                        ws_tx.send(Message::Text(serde_json::to_string(&frame)?)).await?;
                    }
                    Err(broadcast::error::RecvError::Lagged(n)) => {
                        warn!("{peer} lagged by {n} SUP frames");
                    }
                    Err(broadcast::error::RecvError::Closed) => break,                }
            }
        }
    }

    Ok(())
}
