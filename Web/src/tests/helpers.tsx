import React from 'react'
import type { ReactNode } from 'react'
import { KaiContext } from '../context/KaiContext'
import type { KaiSocket } from '../hooks/useKaiSocket'
import { initialState } from '../hooks/useKaiSocket'
import type { KaiState } from '../hooks/useKaiSocket'
import { vi } from 'vitest'

/** Build a mock KaiSocket with partial state overrides. */
export function mockSocket(overrides: Partial<KaiState> = {}): KaiSocket {
  return {
    state: { ...initialState, ...overrides },
    evaluate: vi.fn(),
    selectAddr: vi.fn(),
    clearRepl: vi.fn(),
  }
}

/** Wrap a component under test with a KaiContext providing `socket`. */
export function withKai(socket: KaiSocket) {
  return function Wrapper({ children }: { children: ReactNode }) {
    return (
      <KaiContext.Provider value={socket}>
        {children}
      </KaiContext.Provider>
    )
  }
}
