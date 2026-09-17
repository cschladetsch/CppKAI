use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use serde_json::Value;

// ── Frames the browser sends ──────────────────────────────────────────────────

#[derive(Debug, Deserialize)]
#[serde(tag = "kind", rename_all = "snake_case")]
pub enum BrowserFrame {
    Eval { src: String, lang: Option<Lang> },
}

#[derive(Debug, Deserialize, Default, Clone, Copy)]
#[serde(rename_all = "snake_case")]
pub enum Lang {
    #[default]
    Rho,
    Pi,
}

// ── Frames the bridge sends to the browser ────────────────────────────────────

#[derive(Debug, Clone, Serialize)]
#[serde(tag = "kind", rename_all = "snake_case")]
pub enum ServerFrame {
    Result {
        src: String,
        output: String,
        ts: u64,
    },
    Sup {
        addr: String,
        state: Value,
        ts: u64,
    },
    Error {
        msg: String,
        ts: u64,
    },
    Tree {
        domains: Vec<Domain>,
    },
    Stack {
        src: String,
        items: Vec<Value>,
        ts: u64,
    },
}

// ── Registry tree types ───────────────────────────────────────────────────────

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Domain {
    pub name: String,
    pub registries: Vec<Registry>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Registry {
    pub name: String,
    pub objects: Vec<KaiObject>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct KaiObject {
    pub addr: String,
    #[serde(rename = "type")]
    pub kind: String,
    pub state: HashMap<String, Value>,
}

// ── Helpers ───────────────────────────────────────────────────────────────────

pub fn now_ms() -> u64 {
    use std::time::{SystemTime, UNIX_EPOCH};
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default()
        .as_millis() as u64
}
