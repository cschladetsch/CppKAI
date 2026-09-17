# KAI React Frontend - Build Prompt

2026-09-17 · @Someone

## Project Context

KAI is a custom language/runtime system built around three execution layers: Pi (RPN/stack executor), Rho (infix syntax that transpiles to Pi), and Tau (an IDL for generating network-transparent C++ Agent/Proxy pairs). The runtime features a tri-colour GC, network-transparent continuations, and a Registry/Domain architecture where objects have a three-part address (`node:reg:#`). Executors and objects can migrate between Domains without breaking references. Serialisability is first-class throughout.

The React frontend serves two purposes:

1. **REPL** - interactive input of Rho or Pi expressions, with streamed output, evaluation history, and syntax highlighting
2. **Live dashboard** - a real-time view of Registry contents, object state, and State Update Packet (SUP) propagation across nodes

Existing frontends (CLI, ImGui, WinUI3, WinForms, Unity3D) all treat the object/network layer as opaque - the React frontend follows the same contract.

## Architecture

A small Rust/axum server bridges the browser to the KAI Registry via WebSocket. This avoids CORS complexity and keeps KAI's internals unexposed.

```
Browser (React)
    |
    |  WebSocket (ws://localhost:PORT)
    |
axum bridge (Rust)
    |
    |  KAI native protocol
    |
KAI Registry / Pi executor
```

**Inbound (browser → KAI):** JSON frames `{ "kind": "eval", "src": "1 2 +" }` for Pi, or `{ "kind": "eval", "src": "x := 42", "lang": "rho" }` for Rho. The bridge forwards to the executor and streams result frames back.

**Outbound (KAI → browser):** SUP push frames `{ "kind": "sup", "addr": "node:reg:#", "state": { … } }` emitted whenever KAI's existing broadcast fires. The bridge forwards these as-is; React applies them to local state.

**Error frames:** `{ "kind": "error", "msg": "…" }` for eval failures or network events.

The axum bridge is stateless between connections - KAI holds all object state. The bridge opens a single connection to the local Registry on startup and multiplexes browser clients onto it.

**Repo layout:** Three separate repos, each on a `webui` branch:

| Repo | Language | Purpose |
| --- | --- | --- |
| `CppKAI` | C++ | Any Registry API surface changes for bridge support |
| `kai-bridge` | Rust/axum | New repo - WebSocket↔KAI protocol bridge |
| `kai-web` | React/TS | New repo - browser frontend |

## React App Structure

Three-panel layout, resizable. Stack: React + TypeScript + Vite, Tailwind for layout, CodeMirror 6 for the REPL editor.

**Left panel - Registry Tree** Live collapsible tree of Domains → Registries → objects. Each node shows its `node:reg:#` address. Nodes highlight briefly when a SUP arrives for their address. Clicking a node selects it and populates the inspector.

**Centre panel - REPL** CodeMirror 6 editor accepting Rho (default) or Pi (toggle). Shift+Enter evaluates. Output area below the editor scrolls, shows timestamped result frames and errors. History navigable with Up/Down in the editor (like a shell). Optional: a language toggle button swaps the CodeMirror grammar between Rho (Python-like highlighting) and Pi (Forth-like).

**Right panel - Object Inspector** Shows the selected Registry object's current state as a key/value table. If the object has a world-line (time-space referential history), a simple timeline chart beneath the table. Per-node-pair alpha values shown as a small matrix if present.

**WebSocket hook** A single `useKaiSocket` hook owns the connection, dispatches inbound frames into a `useReducer`, and exposes `evaluate(src, lang)`. All three panels consume state from this hook via context.

## The Prompt

Paste the following into a coding assistant (Claude Code, Cursor, Copilot Workspace, etc.):

---

Build a React + TypeScript frontend for KAI, a custom language runtime. KAI has three execution layers: Pi (RPN/stack), Rho (infix, transpiles to Pi), and Tau (IDL for network-transparent C++ Agent/Proxy generation). Objects live in a Registry/Domain architecture with three-part addresses (`node:reg:#`). A small Rust/axum WebSocket bridge already exists (or will be scaffolded separately) at `ws://localhost:7171`. Do not implement the bridge - only the browser client.

**Stack:** React 18, TypeScript 5, Vite, Tailwind CSS 3, CodeMirror 6. No UI component library - build primitives directly.

**WebSocket protocol (inbound frames the bridge sends):**

```typescript
type KaiFrame =
  | { kind: 'result'; src: string; output: string; ts: number }
  | { kind: 'sup';    addr: string; state: Record<string, unknown>; ts: number }
  | { kind: 'error';  msg: string; ts: number }
  | { kind: 'tree';   domains: Domain[] }  // sent on connect

interface Domain   { name: string; registries: Registry[] }
interface Registry { name: string; objects: KaiObject[] }
interface KaiObject { addr: string; type: string; state: Record<string, unknown> }
```

**Outbound frames (browser sends):**

```typescript
type EvalFrame = { kind: 'eval'; src: string; lang: 'pi' | 'rho' }
```

**Layout:** Three resizable vertical panels separated by drag handles.

1. **Left - Registry Tree** (`RegistryTree.tsx`)
   - Collapsible tree: Domain → Registry → Object
   - Each leaf shows its `addr` string
   - Clicking a leaf fires `onSelect(addr: string)`
   - When a `sup` frame arrives for an addr, briefly flash that leaf (200ms yellow highlight, CSS transition)
2. **Centre - REPL** (`Repl.tsx`)
   - CodeMirror 6 editor, 4-space indent, line numbers on
   - Language toggle button: `Pi` | `Rho` (default Rho). Swapping the button changes the CodeMirror language extension: use `python()` as a close approximation for Rho; use a minimal custom language with no highlighting for Pi
   - Shift+Enter evaluates: sends `{ kind: 'eval', src: editorContent, lang }` over the socket
   - Output pane below the editor: scrolling list of result/error frames, each with a timestamp and monospace output. Errors render in red
   - Up/Down arrow keys in the editor when on the first/last line navigate history (like a shell)
   - A `Clear` button clears output only, not editor content
3. **Right - Object Inspector** (`Inspector.tsx`)
   - Displays the state of the currently selected object (by addr)
   - Key/value table, values rendered as JSON if objects/arrays, plain text otherwise
   - Subscribes to `sup` frames: when a frame arrives for the selected addr, update the displayed state in place with a brief highlight on changed keys
   - Shows addr, type, and last-updated timestamp at the top

**State management:** A single `useKaiSocket(url: string)` hook in `hooks/useKaiSocket.ts`:

- Owns the WebSocket connection with automatic reconnect (exponential backoff, max 30s)
- Dispatches all inbound frames into a `useReducer`
- Exposes: `{ tree, selectedAddr, selectAddr, evaluate, replHistory, connectionState }`
- All three panels receive state via a `KaiContext` (React context + provider)
- `connectionState`: `'connecting' | 'connected' | 'disconnected'` shown in a status bar at the top of the window

**Resize handles:** Pure CSS + mouse events, no library. Store panel widths in `localStorage` under key `kai-panel-widths`.

**File structure to produce:**

```
src/
  App.tsx
  main.tsx
  context/KaiContext.ts
  hooks/useKaiSocket.ts
  components/
    RegistryTree.tsx
    Repl.tsx
    Inspector.tsx
    ResizeHandle.tsx
    StatusBar.tsx
  types/kai.ts       ← the frame types above
vite.config.ts
tailwind.config.ts
package.json
```

Do not add routing, authentication, or testing infrastructure. Do not use any external state library (Zustand, Redux, etc.). Keep all styling in Tailwind utility classes; no CSS modules or emotion.

---

*After the coding assistant produces this scaffold, the next step is the Rust/axum bridge that connects `ws://localhost:7171` to a local KAI Registry process.*

## The Bridge Prompt (`kai-bridge`)

Paste the following into a coding assistant to scaffold the Rust/axum WebSocket bridge:

---

Create a new Rust project called `kai-bridge` on a branch named `webui`. It is a standalone process that bridges browser WebSocket clients to a local KAI Registry process. KAI is a C++ runtime exposing a TCP socket on `127.0.0.1:7272` that accepts newline-delimited text commands (Pi or Rho expressions) and returns newline-delimited text responses. KAI also emits unsolicited State Update Packet (SUP) lines in the format `SUP node:reg:# <json>` whenever object state changes.

**Dependencies (`Cargo.toml`):**

```toml
[dependencies]
axum = { version = "0.7", features = ["ws"] }
tokio = { version = "1", features = ["full"] }
tokio-tungstenite = "0.21"
serde = { version = "1", features = ["derive"] }
serde_json = "1"
tracing = "0.1"
tracing-subscriber = "0.3"
clap = { version = "4", features = ["derive"] }
```

**CLI args (clap):**

- `--kai-addr` - KAI TCP address, default `127.0.0.1:7272`
- `--listen` - WebSocket listen address, default `0.0.0.0:7171`

**Behaviour:**

1. On startup, open a single persistent TCP connection to KAI. Reconnect with exponential backoff (1s, 2s, 4s... max 30s) if KAI is not yet running.
2. Accept WebSocket connections from browsers at `/ws`. Support multiple concurrent browser clients.
3. Inbound WebSocket frame from browser: JSON `{ "kind": "eval", "src": "...", "lang": "pi" | "rho" }`. Forward `src` as a newline-terminated string to KAI's TCP socket.
4. Response line from KAI: forward as `{ "kind": "result", "src": "...", "output": "...", "ts": <unix_ms> }` to the browser that sent the eval (not broadcast).
5. Unsolicited SUP line from KAI (`SUP node:reg:# <json>`): parse and broadcast `{ "kind": "sup", "addr": "node:reg:#", "state": <json>, "ts": <unix_ms> }` to ALL connected browser clients.
6. On new browser connection: send a `{ "kind": "tree", "domains": [] }` frame immediately (tree population is a later concern - empty is fine for now).
7. Error from KAI or parse failure: send `{ "kind": "error", "msg": "...", "ts": <unix_ms> }` to the relevant client.

**Concurrency model:** One `tokio::task` owns the KAI TCP connection and holds a `broadcast::Sender<String>` for SUP frames. Each browser WebSocket connection gets its own task; it holds a clone of the broadcast receiver and a `mpsc::Sender` to write evals to the KAI task.

**CORS:** Allow all origins on the `/ws` route (development only).

**Logging:** Use `tracing` with `INFO` default. Log connect/disconnect events for both KAI and browser clients.

**File structure:**

```
src/
  main.rs       ← clap setup, axum router, spawn kai_conn task
  kai_conn.rs   ← TCP connection to KAI, SUP parsing, broadcast
  ws_handler.rs ← per-browser WebSocket task
  protocol.rs   ← serde types for all frame variants
Cargo.toml
```

Do not implement TLS, authentication, or persistence. Do not attempt to parse KAI's internal binary format - assume all KAI I/O is UTF-8 text over TCP.

---

*Run the bridge with `cargo run -- --kai-addr 127.0.0.1:7272 --listen 0.0.0.0:7171` and point `kai-web` at `ws://localhost:7171/ws`.*

## Binary Protocol Migration

The current text/TCP protocol between `WebConsole` and `kai-bridge` is fragile: KAI values that contain spaces, newlines, string literals, nested continuations, or `{}` blocks cannot be reliably delimited by newlines. The fix is to replace the line-oriented TCP protocol with length-prefixed binary frames using KAI's existing `BinaryStream` serialisation.

### Why binary

KAI already has a complete, battle-tested binary serialisation layer (`BinaryStream`/`BinaryPacket`) used by the ENet network layer. Every KAI object knows how to serialise and deserialise itself. Reusing this for the WebConsole-to-bridge connection means:

- Any KAI value (string, continuation, map, array, nested structure) is represented faithfully
- No escaping, quoting, or delimiter ambiguity
- The bridge can decode type tags and convert to JSON for the browser without reimplementing KAI's object model

### Wire format

Each message is:

```
[4 bytes: uint32 little-endian payload length][N bytes: BinaryStream payload]
```

The payload is a `BinaryStream`-serialised `KaiMessage` struct:

```cpp
struct KaiMessage {
    enum class Kind : uint8_t { Result = 1, Error = 2, Stack = 3 };
    Kind kind;
    std::string src;       // the expression that was evaluated (echoed back)
    Object value;          // the result value (for Result/Stack)
    std::string error;     // error message (for Error)
};
```

For the stack frame, `value` is a `Array` of all stack items top-first.

### Step-by-step prompt

Paste the following into a coding assistant and work through the steps in order. Do not proceed to the next step until the current one compiles and passes its tests.

---

**Step 1 — WebConsole: add a `BinaryStream` response path**

In `CppKAI/Source/App/WebConsole/Main.cpp`, change the TCP protocol from newline-delimited text to length-prefixed binary frames. Keep the existing `READY kai-webconsole\n` greeting as plain text (the bridge already handles it). After that, every response is a binary frame:

1. Define a `send_frame(SOCKET sock, const std::vector<uint8_t>& payload)` helper that writes a 4-byte little-endian length followed by the payload bytes.
2. Define a `make_result_frame(const std::string& src, Object result, Registry& reg)` function that:
   - Creates a `BinaryStream`
   - Writes a 1-byte kind tag (`0x01` = result)
   - Writes `src` as a length-prefixed UTF-8 string
   - Writes the result `Object` using KAI's existing `BinaryStream` operator
   - Returns the bytes as `std::vector<uint8_t>`
3. Define a `make_error_frame(const std::string& src, const std::string& msg)` function (kind tag `0x02`).
4. Define a `make_stack_frame(const std::string& src, Executor& exec, Registry& reg)` function (kind tag `0x03`) that serialises the full data stack as a KAI `Array`.
5. In `Evaluate()`, call `send_frame(client, make_result_frame(...))` instead of `SendLine`.
6. Keep the recv loop reading raw bytes (not lines); inbound eval commands from the bridge remain as newline-terminated UTF-8 text (the bridge still sends `src\n`).

Do not change the bridge yet. Confirm `WebConsole` compiles before proceeding.

---

**Step 2 — kai-bridge: read length-prefixed binary frames from KAI**

In `KaiBridge/src/kai_conn.rs`, replace the line-reader with a binary frame reader:

1. After reading and discarding the `READY` line (still plain text), switch to frame mode.
2. Read 4 bytes → `u32` little-endian = payload length.
3. Read exactly that many bytes → payload.
4. Parse the payload:
   - Byte 0: kind tag (`0x01` result, `0x02` error, `0x03` stack)
   - Bytes 1..N: length-prefixed UTF-8 `src` string
   - Remaining bytes: kind-specific data
5. For kind `0x01` (result): read the next field as a KAI value. Decode it to a JSON-compatible `serde_json::Value` using the type-tag decoder from Step 3.
6. For kind `0x02` (error): read a length-prefixed UTF-8 error string. Dispatch as `ServerFrame::Error`.
7. For kind `0x03` (stack): read a length-prefixed array of KAI values. Dispatch as `ServerFrame::Stack { items: Vec<serde_json::Value> }`.

Add `Stack` to `ServerFrame` in `protocol.rs`:

```rust
Stack { src: String, items: Vec<serde_json::Value>, ts: u64 }
```

Do not change the browser yet. Write a unit test that constructs a synthetic binary frame and asserts the decoder produces the correct `ServerFrame`.

---

**Step 3 — kai-bridge: KAI binary value decoder**

In a new file `KaiBridge/src/kai_binary.rs`, implement a decoder for KAI's `BinaryStream` format. KAI's type system uses numeric type tags. The tags you need for the initial implementation:

| Tag | KAI type | JSON representation |
| --- | --- | --- |
| `Int` | 32-bit signed int | `number` |
| `Float` | 64-bit float | `number` |
| `Bool` | boolean | `boolean` |
| `String` | UTF-8 string | `string` |
| `Array` | sequence | `array` |
| `Void` / `None` | no value | `null` |

To discover the actual tag values, read `CppKAI/Ext/CppKaiCore/Include/KAI/Core/Type/Traits.h` and look for the `Number` enum. Implement `decode_value(bytes: &[u8]) -> Result<(serde_json::Value, &[u8])>` that reads one typed value and returns the remainder. Write unit tests for each type.

---

**Step 4 — kai-web: handle Stack frame**

In `KaiWeb/src/types/kai.ts`, add:

```typescript
interface StackFrame {
  kind: 'stack'
  src: string
  items: unknown[]
  ts: number
}
```

In `KaiWeb/src/hooks/useKaiSocket.ts`:

- Add `STACK` action to the reducer: store `items` in a new `stack: unknown[]` field on `KaiState`
- Dispatch `STACK` when a `stack` frame arrives

In `KaiWeb/src/components/Inspector.tsx`:

- When no object is selected, show the live stack instead of "select an object"
- Render each stack item as a row: index (0 = top) and value
- Highlight the top item
- Update on every `STACK` frame

In `KaiWeb/src/components/Repl.tsx`:

- For `result` entries, show only the top-of-stack value (first item from the stack frame)
- If the stack is empty show `ok`

---

**Step 5 — integration test**

With all three processes running:

1. Type `1 2 +` in the REPL → REPL shows `3`, Inspector shows `[0] 3`
2. Type `3 +` → REPL shows `6`, Inspector shows `[0] 6`
3. Type `1 2 3` → REPL shows `3`, Inspector shows `[0] 3 | [1] 2 | [2] 1` (top first)
4. Type `"hello world"` → REPL shows `hello world`, Inspector shows `[0] hello world`

If step 4 fails (string with space causes a parse error), the binary decoder in Step 3 is incomplete — add more type tags.

## Tau-to-TypeScript Stretch Goal

Tau already generates C++ Agent/Proxy pairs from IDL definitions. The same definitions could emit TypeScript interfaces, giving the Inspector typed props that mirror the KAI object model rather than `Record<string, unknown>`.

The approach: write a small Tau backend (a new emitter, analogous to the C++ emitter) that walks the parsed Tau AST and produces `.ts` files. Each Tau `interface` becomes a TypeScript `interface`; each Tau `agent` becomes a typed state shape the Inspector can import.

This is a separate prompt/task once the scaffold above is working. The scaffold deliberately uses `Record<string, unknown>` throughout so it compiles without the generated types. Swapping in typed interfaces is a drop-in once the emitter exists.

Suggested prompt addition when ready:

> *Given these TypeScript interfaces generated from Tau IDL definitions (attached), replace all `Record<string, unknown>` usages in `Inspector.tsx` and `types/kai.ts` with the generated types. The `sup` frame's `state` field should narrow to the correct interface based on the object's `type` string using a discriminated union.*
