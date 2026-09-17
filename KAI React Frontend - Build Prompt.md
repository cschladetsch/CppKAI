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

## Tau-to-TypeScript Stretch Goal

Tau already generates C++ Agent/Proxy pairs from IDL definitions. The same definitions could emit TypeScript interfaces, giving the Inspector typed props that mirror the KAI object model rather than `Record<string, unknown>`.

The approach: write a small Tau backend (a new emitter, analogous to the C++ emitter) that walks the parsed Tau AST and produces `.ts` files. Each Tau `interface` becomes a TypeScript `interface`; each Tau `agent` becomes a typed state shape the Inspector can import.

This is a separate prompt/task once the scaffold above is working. The scaffold deliberately uses `Record<string, unknown>` throughout so it compiles without the generated types. Swapping in typed interfaces is a drop-in once the emitter exists.

Suggested prompt addition when ready:

> *Given these TypeScript interfaces generated from Tau IDL definitions (attached), replace all `Record<string, unknown>` usages in `Inspector.tsx` and `types/kai.ts` with the generated types. The `sup` frame's `state` field should narrow to the correct interface based on the object's `type` string using a discriminated union.*
