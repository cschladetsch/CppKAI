//! Decoder for KAI's BinaryStream Object serialisation format.
//!
//! Wire layout for one Object:
//!   i32  type_number   (little-endian, from Type::Number::Value)
//!   ...  type-specific payload (depends on type_number)
//!   ...  properties (none for primitives)
//!   i32  num_children
//!   ...  children (label + object, repeated)
//!
//! For the types we care about the payload is:
//!   Void/None  - nothing
//!   Bool(21)   - 1 byte (0/1)
//!   Signed32   - i32 little-endian
//!   Single     - f32 little-endian
//!   Double     - f64 little-endian
//!   String(25) - i32 length + UTF-8 bytes
//!   Array(28)  - i32 count + count Objects

use serde_json::{json, Value};

#[allow(dead_code)]
mod type_num {
    pub const NONE:      i32 = 1;
    pub const VOID:      i32 = 2;
    pub const BOOL:      i32 = 21;
    pub const SIGNED32:  i32 = 22;
    pub const SINGLE:    i32 = 23;
    pub const DOUBLE:    i32 = 24;
    pub const STRING:    i32 = 25;
    pub const ARRAY:     i32 = 28;
}

pub type DecodeResult<'a> = Result<(Value, &'a [u8]), String>;

/// Read a little-endian i32 from the front of `data`.
fn read_i32(data: &[u8]) -> Result<(i32, &[u8]), String> {
    if data.len() < 4 {
        return Err(format!("need 4 bytes for i32, have {}", data.len()));
    }
    let v = i32::from_le_bytes(data[..4].try_into().unwrap());
    Ok((v, &data[4..]))
}

fn read_f32(data: &[u8]) -> Result<(f32, &[u8]), String> {
    if data.len() < 4 {
        return Err(format!("need 4 bytes for f32, have {}", data.len()));
    }
    let v = f32::from_le_bytes(data[..4].try_into().unwrap());
    Ok((v, &data[4..]))
}

fn read_f64(data: &[u8]) -> Result<(f64, &[u8]), String> {
    if data.len() < 8 {
        return Err(format!("need 8 bytes for f64, have {}", data.len()));
    }
    let v = f64::from_le_bytes(data[..8].try_into().unwrap());
    Ok((v, &data[8..]))
}

fn read_bool(data: &[u8]) -> Result<(bool, &[u8]), String> {
    if data.is_empty() {
        return Err("need 1 byte for bool".to_string());
    }
    Ok((data[0] != 0, &data[1..]))
}

fn read_string(data: &[u8]) -> Result<(String, &[u8]), String> {
    let (len, rest) = read_i32(data)?;
    if len < 0 {
        return Err(format!("negative string length: {len}"));
    }
    let len = len as usize;
    if rest.len() < len {
        return Err(format!("string truncated: need {len}, have {}", rest.len()));
    }
    let s = String::from_utf8_lossy(&rest[..len]).into_owned();
    Ok((s, &rest[len..]))
}

/// Skip over properties and children that we don't need to decode.
/// KAI primitives have zero properties and zero children, but we handle
/// the general case so we don't desync on unexpected types.
fn skip_properties_and_children(data: &[u8]) -> Result<&[u8], String> {
    // For now assume no properties (true for all primitives we handle).
    // Skip num_children + children.
    let (num_children, mut rest) = read_i32(data)?;
    for _ in 0..num_children {
        // Each child is: label (String) + Object (recursive)
        let (_, r) = read_string(rest)?;
        let (_, r2) = decode_object(r)?;
        rest = r2;
    }
    Ok(rest)
}

/// Decode one KAI Object from `data`, returning the JSON value and remaining bytes.
pub fn decode_object(data: &[u8]) -> DecodeResult {
    let (type_num, rest) = read_i32(data)?;

    // type_number == 0 means null/invalid Object
    if type_num == 0 {
        return Ok((Value::Null, rest));
    }

    let (value, rest) = match type_num {
        t if t == type_num::NONE || t == type_num::VOID => {
            (Value::Null, rest)
        }
        t if t == type_num::BOOL => {
            let (b, r) = read_bool(rest)?;
            (json!(b), r)
        }
        t if t == type_num::SIGNED32 => {
            let (n, r) = read_i32(rest)?;
            (json!(n), r)
        }
        t if t == type_num::SINGLE => {
            let (f, r) = read_f32(rest)?;
            (json!(f), r)
        }
        t if t == type_num::DOUBLE => {
            let (f, r) = read_f64(rest)?;
            (json!(f), r)
        }
        t if t == type_num::STRING => {
            let (s, r) = read_string(rest)?;
            (json!(s), r)
        }
        t if t == type_num::ARRAY => {
            let (count, mut r) = read_i32(rest)?;
            let mut items = Vec::new();
            for _ in 0..count {
                let (item, remaining) = decode_object(r)?;
                items.push(item);
                r = remaining;
            }
            (json!(items), r)
        }
        other => {
            // Unknown type - return type number as a hint, skip children
            let (_, r) = read_i32(rest).unwrap_or((0, rest));
            (json!({"_type": other, "_unknown": true}), r)
        }
    };

    // Skip properties (zero for primitives) and children
    let rest = skip_properties_and_children(rest)?;

    Ok((value, rest))
}

/// Read a length-prefixed UTF-8 string (u32 LE length + bytes).
pub fn read_length_prefixed_string(data: &[u8]) -> Result<(String, &[u8]), String> {
    if data.len() < 4 {
        return Err("truncated string length".to_string());
    }
    let len = u32::from_le_bytes(data[..4].try_into().unwrap()) as usize;
    let rest = &data[4..];
    if rest.len() < len {
        return Err(format!("string body truncated: need {len}, have {}", rest.len()));
    }
    let s = String::from_utf8_lossy(&rest[..len]).into_owned();
    Ok((s, &rest[len..]))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn make_object(type_num: i32, payload: &[u8]) -> Vec<u8> {
        let mut v = Vec::new();
        v.extend_from_slice(&type_num.to_le_bytes());
        v.extend_from_slice(payload);
        // num_children = 0
        v.extend_from_slice(&0i32.to_le_bytes());
        v
    }

    #[test]
    fn decode_null_object() {
        let data = 0i32.to_le_bytes();
        let (v, rest) = decode_object(&data).unwrap();
        assert_eq!(v, Value::Null);
        assert!(rest.is_empty());
    }

    #[test]
    fn decode_bool_true() {
        let data = make_object(type_num::BOOL, &[1u8]);
        let (v, _) = decode_object(&data).unwrap();
        assert_eq!(v, json!(true));
    }

    #[test]
    fn decode_bool_false() {
        let data = make_object(type_num::BOOL, &[0u8]);
        let (v, _) = decode_object(&data).unwrap();
        assert_eq!(v, json!(false));
    }

    #[test]
    fn decode_signed32() {
        let mut payload = Vec::new();
        payload.extend_from_slice(&42i32.to_le_bytes());
        let data = make_object(type_num::SIGNED32, &payload);
        let (v, _) = decode_object(&data).unwrap();
        assert_eq!(v, json!(42));
    }

    #[test]
    fn decode_signed32_negative() {
        let mut payload = Vec::new();
        payload.extend_from_slice(&(-7i32).to_le_bytes());
        let data = make_object(type_num::SIGNED32, &payload);
        let (v, _) = decode_object(&data).unwrap();
        assert_eq!(v, json!(-7));
    }

    #[test]
    fn decode_float() {
        let mut payload = Vec::new();
        payload.extend_from_slice(&3.14f32.to_le_bytes());
        let data = make_object(type_num::SINGLE, &payload);
        let (v, _) = decode_object(&data).unwrap();
        // f32 precision - just check it's approximately right
        let n = v.as_f64().unwrap();
        assert!((n - 3.14).abs() < 0.001);
    }

    #[test]
    fn decode_string() {
        let s = "hello world";
        let mut payload = Vec::new();
        payload.extend_from_slice(&(s.len() as i32).to_le_bytes());
        payload.extend_from_slice(s.as_bytes());
        let data = make_object(type_num::STRING, &payload);
        let (v, _) = decode_object(&data).unwrap();
        assert_eq!(v, json!("hello world"));
    }

    #[test]
    fn decode_empty_array() {
        let payload = 0i32.to_le_bytes(); // count = 0
        let data = make_object(type_num::ARRAY, &payload);
        let (v, _) = decode_object(&data).unwrap();
        assert_eq!(v, json!([]));
    }

    #[test]
    fn decode_array_of_ints() {
        let mut payload = Vec::new();
        payload.extend_from_slice(&2i32.to_le_bytes()); // count = 2
        // item 0: Signed32 = 10
        payload.extend_from_slice(&type_num::SIGNED32.to_le_bytes());
        payload.extend_from_slice(&10i32.to_le_bytes());
        payload.extend_from_slice(&0i32.to_le_bytes()); // num_children
        // item 1: Signed32 = 20
        payload.extend_from_slice(&type_num::SIGNED32.to_le_bytes());
        payload.extend_from_slice(&20i32.to_le_bytes());
        payload.extend_from_slice(&0i32.to_le_bytes()); // num_children
        let data = make_object(type_num::ARRAY, &payload);
        let (v, _) = decode_object(&data).unwrap();
        assert_eq!(v, json!([10, 20]));
    }
}
