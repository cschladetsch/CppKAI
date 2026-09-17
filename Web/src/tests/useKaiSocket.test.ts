import { describe, it, expect, vi, beforeEach } from 'vitest'
import { renderHook, act } from '@testing-library/react'
import { useKaiSocket } from '../hooks/useKaiSocket'
import { MockWebSocket } from './setup'

const URL = 'ws://localhost:7171/ws'

describe('useKaiSocket', () => {
  // ── initial state ───────────────────────────────────────────────────────────

  it('starts in connecting state', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    // Hook dispatches DISCONNECTED on mount then waits for WS open
    expect(['connecting', 'disconnected']).toContain(result.current.state.connectionState)
  })

  it('transitions to connected when WS opens', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { MockWebSocket.latest().open() })
    expect(result.current.state.connectionState).toBe('connected')
  })

  it('transitions to disconnected when WS closes', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { MockWebSocket.latest().open() })
    act(() => { MockWebSocket.latest().serverClose() })
    expect(result.current.state.connectionState).toBe('disconnected')
  })

  // ── frame dispatch ──────────────────────────────────────────────────────────

  it('populates domains on tree frame', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { MockWebSocket.latest().open() })
    act(() => {
      MockWebSocket.latest().receive({
        kind: 'tree',
        domains: [{ name: 'default', registries: [] }],
      })
    })
    expect(result.current.state.domains).toHaveLength(1)
    expect(result.current.state.domains[0].name).toBe('default')
  })

  it('updates object state on sup frame', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { MockWebSocket.latest().open() })
    act(() => {
      MockWebSocket.latest().receive({ kind: 'sup', addr: '1:0:1', state: { x: 99 }, ts: 1 })
    })
    expect(result.current.state.objects['1:0:1']?.state).toEqual({ x: 99 })
  })

  it('appends result to repl on result frame', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { MockWebSocket.latest().open() })
    act(() => {
      MockWebSocket.latest().receive({ kind: 'result', src: '1 2 +', output: '3', ts: 1 })
    })
    expect(result.current.state.repl).toHaveLength(1)
    expect(result.current.state.repl[0]).toMatchObject({ kind: 'result', text: '3' })
  })

  it('appends error to repl on error frame', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { MockWebSocket.latest().open() })
    act(() => {
      MockWebSocket.latest().receive({ kind: 'error', msg: 'oops', ts: 1 })
    })
    expect(result.current.state.repl[0]).toMatchObject({ kind: 'error', text: 'oops' })
  })

  it('ignores malformed frames without throwing', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { MockWebSocket.latest().open() })
    act(() => {
      // Manually fire a raw message event with bad JSON
      MockWebSocket.latest().onmessage?.(
        new MessageEvent('message', { data: 'not json' })
      )
    })
    // Should not have crashed and state is unchanged
    expect(result.current.state.repl).toHaveLength(0)
  })

  // ── evaluate ────────────────────────────────────────────────────────────────

  it('sends eval frame over WS', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { MockWebSocket.latest().open() })
    act(() => { result.current.evaluate('1 2 +', 'pi') })
    expect(MockWebSocket.latest().lastSent).toEqual({ kind: 'eval', src: '1 2 +', lang: 'pi' })
  })

  it('appends input entry to repl when evaluating', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { MockWebSocket.latest().open() })
    act(() => { result.current.evaluate('x := 42', 'rho') })
    expect(result.current.state.repl[0]).toMatchObject({ kind: 'input', text: 'x := 42' })
  })

  it('does not send when WS is not open', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    // WS is CONNECTING, not OPEN
    act(() => { result.current.evaluate('1 2 +') })
    expect(MockWebSocket.latest().sent).toHaveLength(0)
  })

  // ── selectAddr ──────────────────────────────────────────────────────────────

  it('sets selectedAddr', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { result.current.selectAddr('1:0:5') })
    expect(result.current.state.selectedAddr).toBe('1:0:5')
  })

  it('clears selectedAddr with null', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { result.current.selectAddr('1:0:5') })
    act(() => { result.current.selectAddr(null) })
    expect(result.current.state.selectedAddr).toBeNull()
  })

  // ── clearRepl ───────────────────────────────────────────────────────────────

  it('clears repl', () => {
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { MockWebSocket.latest().open() })
    act(() => {
      MockWebSocket.latest().receive({ kind: 'result', src: 'a', output: '1', ts: 1 })
    })
    act(() => { result.current.clearRepl() })
    expect(result.current.state.repl).toHaveLength(0)
  })

  // ── reconnect ────────────────────────────────────────────────────────────────

  it('creates a new WebSocket after disconnect', () => {
    vi.useFakeTimers()
    const { result } = renderHook(() => useKaiSocket(URL))
    act(() => { MockWebSocket.latest().open() })
    const first = MockWebSocket.latest()
    act(() => { first.serverClose() })
    // Advance past the initial 1s backoff
    act(() => { vi.advanceTimersByTime(1100) })
    const second = MockWebSocket.latest()
    expect(second).not.toBe(first)
    vi.useRealTimers()
  })

  // ── cleanup ──────────────────────────────────────────────────────────────────

  it('closes WS on unmount', () => {
    const { unmount } = renderHook(() => useKaiSocket(URL))
    const ws = MockWebSocket.latest()
    const closeSpy = vi.spyOn(ws, 'close')
    unmount()
    expect(closeSpy).toHaveBeenCalled()
  })
})
