/// Unit tests for protocol frame serde.
///
/// BrowserFrame: JSON → Rust (deserialise)
/// ServerFrame:  Rust → JSON (serialise), including shape of the `kind` tag

use serde_json::{json, Value};

use crate::protocol::{BrowserFrame, Lang, ServerFrame};

// ── BrowserFrame deserialisation ─────────────────────────────────────────────

#[test]
fn browser_eval_pi_deserialises() {
    let raw = r#"{"kind":"eval","src":"1 2 +","lang":"pi"}"#;
    let frame: BrowserFrame = serde_json::from_str(raw).unwrap();
    match frame {
        BrowserFrame::Eval { src, lang } => {
            assert_eq!(src, "1 2 +");
            assert!(matches!(lang, Some(Lang::Pi)));
        }
    }
}

#[test]
fn browser_eval_rho_deserialises() {
    let raw = r#"{"kind":"eval","src":"x := 42","lang":"rho"}"#;
    let frame: BrowserFrame = serde_json::from_str(raw).unwrap();
    match frame {
        BrowserFrame::Eval { src, lang } => {
            assert_eq!(src, "x := 42");
            assert!(matches!(lang, Some(Lang::Rho)));
        }
    }
}

#[test]
fn browser_eval_omitted_lang_is_none() {
    let raw = r#"{"kind":"eval","src":"1 2 +"}"#;
    let frame: BrowserFrame = serde_json::from_str(raw).unwrap();
    match frame {
        BrowserFrame::Eval { lang, .. } => assert!(lang.is_none()),
    }
}

#[test]
fn browser_eval_null_lang_is_none() {
    let raw = r#"{"kind":"eval","src":"1 2 +","lang":null}"#;
    let frame: BrowserFrame = serde_json::from_str(raw).unwrap();
    match frame {
        BrowserFrame::Eval { lang, .. } => assert!(lang.is_none()),
    }
}

#[test]
fn browser_unknown_kind_is_error() {
    let raw = r#"{"kind":"subscribe","topic":"registry"}"#;
    assert!(serde_json::from_str::<BrowserFrame>(raw).is_err());
}

#[test]
fn browser_missing_src_is_error() {
    let raw = r#"{"kind":"eval"}"#;
    assert!(serde_json::from_str::<BrowserFrame>(raw).is_err());
}

// ── ServerFrame serialisation ─────────────────────────────────────────────────

fn ser(f: &ServerFrame) -> Value {
    serde_json::to_value(f).unwrap()
}

#[test]
fn result_frame_has_correct_shape() {
    let f = ServerFrame::Result {
        src: "1 2 +".into(),
        output: "3".into(),
        ts: 1234567890,
    };
    let v = ser(&f);
    assert_eq!(v["kind"], "result");
    assert_eq!(v["src"],    "1 2 +");
    assert_eq!(v["output"], "3");
    assert_eq!(v["ts"],     1234567890u64);
}

#[test]
fn sup_frame_has_correct_shape() {
    let f = ServerFrame::Sup {
        addr: "1:0:42".into(),
        state: json!({"x": 1}),
        ts: 999,
    };
    let v = ser(&f);
    assert_eq!(v["kind"],       "sup");
    assert_eq!(v["addr"],       "1:0:42");
    assert_eq!(v["state"]["x"], 1);
    assert_eq!(v["ts"],         999u64);
}

#[test]
fn error_frame_has_correct_shape() {
    let f = ServerFrame::Error {
        msg: "compile failed".into(),
        ts: 1,
    };
    let v = ser(&f);
    assert_eq!(v["kind"], "error");
    assert_eq!(v["msg"],  "compile failed");
}

#[test]
fn tree_frame_empty_domains() {
    let f = ServerFrame::Tree { domains: vec![] };
    let v = ser(&f);
    assert_eq!(v["kind"],    "tree");
    assert_eq!(v["domains"], json!([]));
}

#[test]
fn tree_frame_with_domains() {
    use crate::protocol::{Domain, KaiObject, Registry};
    use std::collections::HashMap;

    let f = ServerFrame::Tree {
        domains: vec![Domain {
            name: "default".into(),
            registries: vec![Registry {
                name: "main".into(),
                objects: vec![KaiObject {
                    addr: "0:0:1".into(),
                    kind: "int".into(),
                    state: HashMap::new(),
                }],
            }],
        }],
    };
    let v = ser(&f);
    assert_eq!(v["domains"][0]["name"],                          "default");
    assert_eq!(v["domains"][0]["registries"][0]["name"],         "main");
    assert_eq!(v["domains"][0]["registries"][0]["objects"][0]["addr"], "0:0:1");
}

#[test]
fn server_frames_use_snake_case_kind() {
    // Regression: ensure serde renames are applied (no CamelCase leaking)
    for (frame, expected_kind) in [
        (ServerFrame::Result { src: "".into(), output: "".into(), ts: 0 }, "result"),
        (ServerFrame::Error  { msg: "".into(), ts: 0 },                    "error"),
        (ServerFrame::Tree   { domains: vec![] },                          "tree"),
    ] {
        let v = ser(&frame);
        assert_eq!(v["kind"], expected_kind, "wrong kind for {expected_kind}");
    }
}
