import { describe, it, expect } from 'vitest'
import { reducer, initialState } from '../hooks/useKaiSocket'
import type { KaiState } from '../hooks/useKaiSocket'

// ── helpers ───────────────────────────────────────────────────────────────────

const s = () => structuredClone(initialState) as KaiState

// ── connection state ──────────────────────────────────────────────────────────

describe('CONNECTED', () => {
  it('sets connectionState to connected', () => {
    const state = reducer(s(), { type: 'CONNECTED' })
    expect(state.connectionState).toBe('connected')
  })

  it('does not mutate other fields', () => {
    const before = s()
    const after = reducer(before, { type: 'CONNECTED' })
    expect(after.domains).toBe(before.domains)
    expect(after.repl).toBe(before.repl)
    expect(after.objects).toBe(before.objects)
  })
})

describe('DISCONNECTED', () => {
  it('sets connectionState to disconnected', () => {
    const state = reducer({ ...s(), connectionState: 'connected' }, { type: 'DISCONNECTED' })
    expect(state.connectionState).toBe('disconnected')
  })
})

// ── tree ──────────────────────────────────────────────────────────────────────

describe('TREE', () => {
  it('replaces domains', () => {
    const domains = [{ name: 'default', registries: [] }]
    const state = reducer(s(), { type: 'TREE', domains })
    expect(state.domains).toEqual(domains)
  })

  it('replaces previous domains', () => {
    const first  = [{ name: 'a', registries: [] }]
    const second = [{ name: 'b', registries: [] }]
    const state = reducer(reducer(s(), { type: 'TREE', domains: first }), { type: 'TREE', domains: second })
    expect(state.domains).toEqual(second)
  })
})

// ── SUP ───────────────────────────────────────────────────────────────────────

describe('SUP', () => {
  it('adds a new object', () => {
    const state = reducer(s(), { type: 'SUP', addr: '1:0:1', state: { x: 1 }, ts: 1 })
    expect(state.objects['1:0:1']).toMatchObject({ addr: '1:0:1', state: { x: 1 } })
  })

  it('updates existing object state', () => {
    const s1 = reducer(s(), { type: 'SUP', addr: '1:0:1', state: { x: 1 }, ts: 1 })
    const s2 = reducer(s1, { type: 'SUP', addr: '1:0:1', state: { x: 2 }, ts: 2 })
    expect(s2.objects['1:0:1'].state).toEqual({ x: 2 })
  })

  it('preserves existing type when updating', () => {
    const base = {
      ...s(),
      objects: { '1:0:1': { addr: '1:0:1', type: 'int', state: { v: 0 } } },
    }
    const next = reducer(base, { type: 'SUP', addr: '1:0:1', state: { v: 42 }, ts: 1 })
    expect(next.objects['1:0:1'].type).toBe('int')
  })

  it('defaults type to unknown for new object', () => {
    const state = reducer(s(), { type: 'SUP', addr: 'x:y:z', state: {}, ts: 1 })
    expect(state.objects['x:y:z'].type).toBe('unknown')
  })

  it('does not affect other objects', () => {
    const s1 = reducer(s(), { type: 'SUP', addr: '1:0:1', state: { a: 1 }, ts: 1 })
    const s2 = reducer(s1, { type: 'SUP', addr: '1:0:2', state: { b: 2 }, ts: 2 })
    expect(s2.objects['1:0:1'].state).toEqual({ a: 1 })
    expect(s2.objects['1:0:2'].state).toEqual({ b: 2 })
  })
})

// ── repl ──────────────────────────────────────────────────────────────────────

describe('INPUT', () => {
  it('appends an input entry', () => {
    const state = reducer(s(), { type: 'INPUT', src: '1 2 +', ts: 1 })
    expect(state.repl).toHaveLength(1)
    expect(state.repl[0]).toMatchObject({ kind: 'input', text: '1 2 +' })
  })

  it('gives each entry a unique id', () => {
    let st = s()
    st = reducer(st, { type: 'INPUT', src: 'a', ts: 1 })
    st = reducer(st, { type: 'INPUT', src: 'b', ts: 2 })
    expect(st.repl[0].id).not.toBe(st.repl[1].id)
  })
})

describe('RESULT', () => {
  it('appends a result entry', () => {
    const state = reducer(s(), { type: 'RESULT', src: '1 2 +', output: '3', ts: 1 })
    expect(state.repl[0]).toMatchObject({ kind: 'result', text: '3', src: '1 2 +' })
  })
})

describe('ERROR', () => {
  it('appends an error entry', () => {
    const state = reducer(s(), { type: 'ERROR', msg: 'compile failed', ts: 1 })
    expect(state.repl[0]).toMatchObject({ kind: 'error', text: 'compile failed' })
  })
})

describe('CLEAR_REPL', () => {
  it('empties the repl', () => {
    let st = s()
    st = reducer(st, { type: 'INPUT', src: 'a', ts: 1 })
    st = reducer(st, { type: 'RESULT', src: 'a', output: '1', ts: 2 })
    st = reducer(st, { type: 'CLEAR_REPL' })
    expect(st.repl).toHaveLength(0)
  })

  it('does not affect objects or domains', () => {
    const before = {
      ...s(),
      objects: { '1:0:1': { addr: '1:0:1', type: 'int', state: {} } },
      domains: [{ name: 'd', registries: [] }],
    }
    const after = reducer(before, { type: 'CLEAR_REPL' })
    expect(after.objects).toBe(before.objects)
    expect(after.domains).toBe(before.domains)
  })
})

// ── SELECT ────────────────────────────────────────────────────────────────────

describe('SELECT', () => {
  it('sets selectedAddr', () => {
    const state = reducer(s(), { type: 'SELECT', addr: '1:0:5' })
    expect(state.selectedAddr).toBe('1:0:5')
  })

  it('clears selectedAddr when null', () => {
    const st = reducer(s(), { type: 'SELECT', addr: '1:0:5' })
    const cleared = reducer(st, { type: 'SELECT', addr: null })
    expect(cleared.selectedAddr).toBeNull()
  })
})

// ── immutability ──────────────────────────────────────────────────────────────

describe('immutability', () => {
  it('returns a new object reference on every action', () => {
    const before = s()
    const after  = reducer(before, { type: 'CONNECTED' })
    expect(after).not.toBe(before)
  })

  it('does not mutate the input state', () => {
    const before = s()
    const copy = JSON.stringify(before)
    reducer(before, { type: 'CONNECTED' })
    expect(JSON.stringify(before)).toBe(copy)
  })
})
