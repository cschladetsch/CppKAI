/// Unit tests for `kai_conn::parse_sup`.
///
/// parse_sup must:
///   - return Some(ServerFrame::Sup) for well-formed SUP lines
///   - return None for anything that isn't a SUP line
///   - handle nested/complex JSON state objects
///   - never panic on malformed input

use serde_json::json;

use crate::kai_conn::parse_sup;
use crate::protocol::ServerFrame;

// ── helpers ──────────────────────────────────────────────────────────────────

fn sup_addr_and_state(line: &str) -> Option<(String, serde_json::Value)> {
    match parse_sup(line)? {
        ServerFrame::Sup { addr, state, .. } => Some((addr, state)),
        _ => None,
    }
}

// ── happy path ────────────────────────────────────────────────────────────────

#[test]
fn parses_simple_sup() {
    let (addr, state) = sup_addr_and_state(r#"SUP 1:0:42 {"x":1}"#).unwrap();
    assert_eq!(addr, "1:0:42");
    assert_eq!(state, json!({"x": 1}));
}

#[test]
fn parses_nested_json_state() {
    let line = r#"SUP node1:reg2:7 {"pos":{"x":1.0,"y":2.0},"health":100}"#;
    let (addr, state) = sup_addr_and_state(line).unwrap();
    assert_eq!(addr, "node1:reg2:7");
    assert_eq!(state["pos"]["x"], json!(1.0));
    assert_eq!(state["health"], json!(100));
}

#[test]
fn parses_array_state() {
    let line = r#"SUP 0:0:1 [1,2,3]"#;
    let (addr, state) = sup_addr_and_state(line).unwrap();
    assert_eq!(addr, "0:0:1");
    assert_eq!(state, json!([1, 2, 3]));
}

#[test]
fn parses_empty_object_state() {
    let line = r#"SUP a:b:c {}"#;
    let (addr, state) = sup_addr_and_state(line).unwrap();
    assert_eq!(addr, "a:b:c");
    assert_eq!(state, json!({}));
}

#[test]
fn preserves_addr_with_colons() {
    // Three-part address node:reg:# where each part may itself contain digits
    let line = r#"SUP 127:255:0 {"v":true}"#;
    let (addr, _) = sup_addr_and_state(line).unwrap();
    assert_eq!(addr, "127:255:0");
}

#[test]
fn timestamp_is_nonzero() {
    let line = r#"SUP 1:0:0 {"k":"v"}"#;
    match parse_sup(line).unwrap() {
        ServerFrame::Sup { ts, .. } => assert!(ts > 0),
        _ => panic!("wrong variant"),
    }
}

// ── rejection cases ───────────────────────────────────────────────────────────

#[test]
fn rejects_empty_string() {
    assert!(parse_sup("").is_none());
}

#[test]
fn rejects_non_sup_prefix() {
    assert!(parse_sup("RESULT 3").is_none());
    assert!(parse_sup("ERROR bad").is_none());
    assert!(parse_sup("READY kai-webconsole").is_none());
}

#[test]
fn rejects_sup_without_addr() {
    // "SUP " with nothing after
    assert!(parse_sup("SUP ").is_none());
}

#[test]
fn rejects_sup_without_json() {
    // addr present but no space + JSON
    assert!(parse_sup("SUP 1:0:42").is_none());
}

#[test]
fn rejects_sup_with_invalid_json() {
    assert!(parse_sup(r#"SUP 1:0:42 {not json}"#).is_none());
}

#[test]
fn rejects_lowercase_sup() {
    // Protocol is case-sensitive
    assert!(parse_sup(r#"sup 1:0:0 {}"#).is_none());
}

#[test]
fn rejects_whitespace_only() {
    assert!(parse_sup("   ").is_none());
}
