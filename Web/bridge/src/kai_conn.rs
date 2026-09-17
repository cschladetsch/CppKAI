use std::time::Duration;

use tokio::{
    io::{AsyncBufReadExt, AsyncReadExt, AsyncWriteExt, BufReader},
    net::TcpStream,
    sync::{broadcast, mpsc},
    time::sleep,
};
use tracing::{debug, error, info, warn};

use crate::kai_binary::{decode_object, read_length_prefixed_string};
use crate::protocol::{now_ms, ServerFrame};

pub struct EvalCommand {
    pub src: String,
    pub reply: mpsc::Sender<ServerFrame>,
}

pub async fn spawn(
    kai_addr: String,
    sup_tx: broadcast::Sender<ServerFrame>,
) -> mpsc::Sender<EvalCommand> {
    let (eval_tx, eval_rx) = mpsc::channel::<EvalCommand>(64);
    tokio::spawn(run(kai_addr, eval_rx, sup_tx));
    eval_tx
}

async fn run(
    kai_addr: String,
    mut eval_rx: mpsc::Receiver<EvalCommand>,
    sup_tx: broadcast::Sender<ServerFrame>,
) {
    let mut backoff = Duration::from_secs(1);

    loop {
        info!("Connecting to KAI at {}", kai_addr);
        match TcpStream::connect(&kai_addr).await {
            Ok(stream) => {
                info!("Connected to KAI at {}", kai_addr);
                backoff = Duration::from_secs(1);
                handle_connection(stream, &mut eval_rx, &sup_tx).await;
                warn!("KAI connection lost, reconnecting in {:?}", backoff);
            }
            Err(e) => {
                error!("Failed to connect to KAI: {e}, retrying in {:?}", backoff);
            }
        }

        sleep(backoff).await;
        backoff = (backoff * 2).min(Duration::from_secs(30));
    }
}

/// Read exactly `n` bytes from the stream.
async fn read_exact_bytes(
    stream: &mut (impl AsyncReadExt + Unpin),
    n: usize,
) -> Option<Vec<u8>> {
    let mut buf = vec![0u8; n];
    match stream.read_exact(&mut buf).await {
        Ok(_) => Some(buf),
        Err(e) => {
            debug!("read_exact failed: {e}");
            None
        }
    }
}

/// Parse a binary frame payload into a ServerFrame.
fn parse_frame(payload: &[u8], pending_src: &str) -> Option<ServerFrame> {
    if payload.is_empty() {
        return None;
    }

    let kind = payload[0];
    let rest = &payload[1..];

    match kind {
        0x01 => {
            // Result: src string + Object (top of stack)
            let (src, rest) = read_length_prefixed_string(rest).ok()?;
            let (value, _) = decode_object(rest).ok()?;
            Some(ServerFrame::Result {
                src,
                output: value.to_string().trim_matches('"').to_string(),
                ts: now_ms(),
            })
        }
        0x02 => {
            // Error: src string + error message string
            let (src, rest) = read_length_prefixed_string(rest).ok()?;
            let (msg, _) = read_length_prefixed_string(rest).ok()?;
            // "ok" errors are just empty stack - send as result
            if msg == "ok" {
                Some(ServerFrame::Result {
                    src,
                    output: "ok".to_string(),
                    ts: now_ms(),
                })
            } else {
                Some(ServerFrame::Error {
                    msg: format!("{src}: {msg}"),
                    ts: now_ms(),
                })
            }
        }
        0x03 => {
            // Stack: src string + Array of items
            let (src, rest) = read_length_prefixed_string(rest).ok()?;
            let (arr_val, _) = decode_object(rest).ok()?;
            let items = match arr_val {
                serde_json::Value::Array(v) => v,
                other => vec![other],
            };
            Some(ServerFrame::Stack {
                src,
                items,
                ts: now_ms(),
            })
        }
        other => {
            warn!("Unknown frame kind: 0x{other:02x}");
            None
        }
    }
}

async fn handle_connection(
    stream: TcpStream,
    eval_rx: &mut mpsc::Receiver<EvalCommand>,
    sup_tx: &broadcast::Sender<ServerFrame>,
) {
    let (reader, mut writer) = stream.into_split();
    let mut buf_reader = BufReader::new(reader);

    // Read the plain-text READY greeting
    let mut greeting = String::new();
    match buf_reader.read_line(&mut greeting).await {
        Ok(0) | Err(_) => { error!("KAI closed before READY"); return; }
        Ok(_) => {
            if !greeting.contains("READY") {
                warn!("Unexpected greeting: {greeting}");
            }
            info!("KAI ready: {}", greeting.trim());
        }
    }

    // After READY, the stream is binary frames.
    // We need the raw reader back.
    let mut raw = buf_reader.into_inner();

    let mut pending: Option<(String, mpsc::Sender<ServerFrame>)> = None;

    loop {
        tokio::select! {
            // Read a binary frame from KAI
            header = read_exact_bytes(&mut raw, 4) => {
                let header = match header {
                    Some(h) => h,
                    None => { warn!("KAI closed connection"); return; }
                };

                let payload_len = u32::from_le_bytes(header[..4].try_into().unwrap()) as usize;
                if payload_len == 0 || payload_len > 16 * 1024 * 1024 {
                    error!("Implausible frame length: {payload_len}");
                    return;
                }

                let payload = match read_exact_bytes(&mut raw, payload_len).await {
                    Some(p) => p,
                    None => { warn!("KAI frame body truncated"); return; }
                };

                let pending_src = pending.as_ref().map(|(s,_)| s.as_str()).unwrap_or("");
                let frame = parse_frame(&payload, pending_src);

                match frame {
                    Some(f @ ServerFrame::Stack { .. }) => {
                        let _ = sup_tx.send(f);
                    }
                    Some(ServerFrame::Result { .. }) | Some(ServerFrame::Error { .. }) => {
                        if let Some((_, reply_tx)) = pending.take() {
                            if let Some(f) = frame {
                                let _ = reply_tx.send(f).await;
                            }
                        }
                    }
                    Some(frame) => {
                        let _ = sup_tx.send(frame);
                    }
                    None => {
                        debug!("Failed to parse frame of len {payload_len}");
                    }
                }
            }

            // Eval command from a browser handler
            cmd = eval_rx.recv() => {
                match cmd {
                    Some(EvalCommand { src, reply }) => {
                        if let Err(e) = writer.write_all(format!("{src}\n").as_bytes()).await {
                            error!("KAI write error: {e}");
                            let _ = reply.send(ServerFrame::Error {
                                msg: format!("KAI write error: {e}"),
                                ts: now_ms(),
                            }).await;
                            return;
                        }
                        pending = Some((src, reply));
                    }
                    None => return,
                }
            }
        }
    }
}

/// Parse `SUP node:reg:# <json>` into a ServerFrame::Sup (kept for future use).
pub(crate) fn parse_sup(line: &str) -> Option<ServerFrame> {
    let rest = line.strip_prefix("SUP ")?;
    let (addr, json_str) = rest.split_once(' ')?;
    let state: serde_json::Value = serde_json::from_str(json_str).ok()?;
    Some(ServerFrame::Sup {
        addr: addr.to_string(),
        state,
        ts: now_ms(),
    })
}
