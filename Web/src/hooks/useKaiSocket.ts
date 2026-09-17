import { useCallback, useEffect, useReducer, useRef } from 'react'
import type {
  ConnectionState,
  Domain,
  KaiObject,
  ReplEntry,
  ServerFrame,
} from '../types/kai'

// ── State ─────────────────────────────────────────────────────────────────────

export interface KaiState {
  connectionState: ConnectionState
  domains: Domain[]
  objects: Record<string, KaiObject>
  repl: ReplEntry[]
  selectedAddr: string | null
  stack: unknown[]
}

const initial: KaiState = {
  connectionState: 'connecting',
  domains: [],
  objects: {},
  repl: [],
  selectedAddr: null,
  stack: [],
}

// ── Reducer ───────────────────────────────────────────────────────────────────

export type Action =
  | { type: 'CONNECTED' }
  | { type: 'DISCONNECTED' }
  | { type: 'TREE'; domains: Domain[] }
  | { type: 'SUP'; addr: string; state: Record<string, unknown>; ts: number }
  | { type: 'RESULT'; src: string; output: string; ts: number }
  | { type: 'ERROR'; msg: string; ts: number }
  | { type: 'INPUT'; src: string; ts: number }
  | { type: 'SELECT'; addr: string | null }
  | { type: 'CLEAR_REPL' }
  | { type: 'STACK'; items: unknown[] }

let nextId = 0
const id = () => ++nextId

export const initialState: KaiState = initial

export function reducer(state: KaiState, action: Action): KaiState {
  switch (action.type) {
    case 'CONNECTED':
      return { ...state, connectionState: 'connected' }

    case 'DISCONNECTED':
      return { ...state, connectionState: 'disconnected' }

    case 'TREE':
      return { ...state, domains: action.domains }

    case 'SUP': {
      const existing = state.objects[action.addr]
      const updated: KaiObject = {
        addr: action.addr,
        type: existing?.type ?? 'unknown',
        state: action.state,
      }
      return {
        ...state,
        objects: { ...state.objects, [action.addr]: updated },
      }
    }

    case 'STACK':
      return { ...state, stack: action.items }

    case 'INPUT':
      return {
        ...state,
        repl: [
          ...state.repl,
          { id: id(), kind: 'input', text: action.src, ts: action.ts },
        ],
      }

    case 'RESULT':
      return {
        ...state,
        repl: [
          ...state.repl,
          { id: id(), kind: 'result', text: action.output, src: action.src, ts: action.ts },
        ],
      }

    case 'ERROR':
      return {
        ...state,
        repl: [
          ...state.repl,
          { id: id(), kind: 'error', text: action.msg, ts: action.ts },
        ],
      }

    case 'SELECT':
      return { ...state, selectedAddr: action.addr }

    case 'CLEAR_REPL':
      return { ...state, repl: [] }

    default:
      return state
  }
}

// ── Hook ──────────────────────────────────────────────────────────────────────

export interface KaiSocket {
  state: KaiState
  evaluate: (src: string, lang?: 'pi' | 'rho') => void
  selectAddr: (addr: string | null) => void
  clearRepl: () => void
}

export function useKaiSocket(url: string): KaiSocket {
  const [state, dispatch] = useReducer(reducer, initial)
  const wsRef = useRef<WebSocket | null>(null)
  const backoffRef = useRef(1000)
  const mountedRef = useRef(true)

  const connect = useCallback(() => {
    if (!mountedRef.current) return

    dispatch({ type: 'DISCONNECTED' })
    const ws = new WebSocket(url)
    wsRef.current = ws

    ws.onopen = () => {
      if (!mountedRef.current) return
      backoffRef.current = 1000
      dispatch({ type: 'CONNECTED' })
    }

    ws.onmessage = (ev) => {
      if (!mountedRef.current) return
      try {
        const frame = JSON.parse(ev.data) as ServerFrame
        switch (frame.kind) {
          case 'tree':
            dispatch({ type: 'TREE', domains: frame.domains })
            break
          case 'sup':
            dispatch({ type: 'SUP', addr: frame.addr, state: frame.state, ts: frame.ts })
            break
          case 'result':
            dispatch({ type: 'RESULT', src: frame.src, output: frame.output, ts: frame.ts })
            break
          case 'error':
            dispatch({ type: 'ERROR', msg: frame.msg, ts: frame.ts })
            break
          case 'stack':
            dispatch({ type: 'STACK', items: frame.items })
            break
        }
      } catch {
        // ignore malformed frames
      }
    }

    ws.onclose = () => {
      if (!mountedRef.current) return
      dispatch({ type: 'DISCONNECTED' })
      const delay = backoffRef.current
      backoffRef.current = Math.min(backoffRef.current * 2, 30_000)
      setTimeout(connect, delay)
    }

    ws.onerror = () => {
      ws.close()
    }
  }, [url])

  useEffect(() => {
    mountedRef.current = true
    connect()
    return () => {
      mountedRef.current = false
      wsRef.current?.close()
    }
  }, [connect])

  const evaluate = useCallback((src: string, lang?: 'pi' | 'rho') => {
    const ws = wsRef.current
    if (!ws || ws.readyState !== WebSocket.OPEN) return
    dispatch({ type: 'INPUT', src, ts: Date.now() })
    ws.send(JSON.stringify({ kind: 'eval', src, lang }))
  }, [])

  const selectAddr = useCallback((addr: string | null) => {
    dispatch({ type: 'SELECT', addr })
  }, [])

  const clearRepl = useCallback(() => {
    dispatch({ type: 'CLEAR_REPL' })
  }, [])

  return { state, evaluate, selectAddr, clearRepl }
}
