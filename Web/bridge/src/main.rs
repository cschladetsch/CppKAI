mod kai_binary;
mod kai_conn;
mod protocol;
mod ws_handler;

#[cfg(test)]
mod tests {
    mod test_parse_sup;
    mod test_protocol;
    mod test_kai_conn;
    mod test_ws_handler;
}

use std::net::SocketAddr;

use clap::Parser;
use tokio::{net::TcpListener, sync::broadcast};
use tracing::info;
use tracing_subscriber::{layer::SubscriberExt, util::SubscriberInitExt, EnvFilter};

#[derive(Parser, Debug)]
#[command(name = "kai-bridge", about = "WebSocket ↔ KAI Registry bridge")]
struct Args {
    /// KAI TCP address
    #[arg(long, default_value = "127.0.0.1:7272")]
    kai_addr: String,

    /// WebSocket listen address
    #[arg(long, default_value = "0.0.0.0:7171")]
    listen: SocketAddr,
}

#[tokio::main]
async fn main() {
    let args = Args::parse();

    tracing_subscriber::registry()
        .with(EnvFilter::try_from_default_env().unwrap_or_else(|_| "info".into()))
        .with(tracing_subscriber::fmt::layer())
        .init();

    // SUP broadcast channel - 256 slot ring buffer
    let (sup_tx, _) = broadcast::channel::<protocol::ServerFrame>(256);

    // Spawn the KAI connection task
    let eval_tx = kai_conn::spawn(args.kai_addr.clone(), sup_tx.clone()).await;

    let listener = TcpListener::bind(args.listen).await.unwrap();
    info!("kai-bridge listening on ws://{}/ws", args.listen);
    info!("KAI backend: {}", args.kai_addr);

    loop {
        match listener.accept().await {
            Ok((stream, peer)) => {
                let eval_tx = eval_tx.clone();
                let sup_rx = sup_tx.subscribe();
                tokio::spawn(ws_handler::accept(stream, peer, eval_tx, sup_rx));
            }
            Err(e) => tracing::error!("Accept error: {e}"),
        }
    }
}
