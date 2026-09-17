// ── Frames the browser sends ──────────────────────────────────────────────────

export type Lang = 'pi' | 'rho'

export interface EvalFrame {
  kind: 'eval'
  src: string
  lang?: Lang
}

export type BrowserFrame = EvalFrame

// ── Frames the bridge sends ───────────────────────────────────────────────────

export interface ResultFrame {
  kind: 'result'
  src: string
  output: string
  ts: number
}

export interface SupFrame {
  kind: 'sup'
  addr: string
  state: Record<string, unknown>
  ts: number
}

export interface ErrorFrame {
  kind: 'error'
  msg: string
  ts: number
}

export interface TreeFrame {
  kind: 'tree'
  domains: Domain[]
}

export interface StackFrame {
  kind: 'stack'
  src: string
  items: unknown[]
  ts: number
}

export type ServerFrame = ResultFrame | SupFrame | ErrorFrame | TreeFrame | StackFrame

// ── Registry tree ─────────────────────────────────────────────────────────────

export interface Domain {
  name: string
  registries: Registry[]
}

export interface Registry {
  name: string
  objects: KaiObject[]
}

export interface KaiObject {
  addr: string
  type: string
  state: Record<string, unknown>
}

// ── App state ─────────────────────────────────────────────────────────────────

export type ConnectionState = 'connecting' | 'connected' | 'disconnected'

export interface ReplEntry {
  id: number
  kind: 'input' | 'result' | 'error'
  text: string
  src?: string
  ts: number
}
