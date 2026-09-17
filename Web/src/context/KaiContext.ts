import { createContext, useContext } from 'react'
import type { KaiSocket } from '../hooks/useKaiSocket'

export const KaiContext = createContext<KaiSocket | null>(null)

export function useKai(): KaiSocket {
  const ctx = useContext(KaiContext)
  if (!ctx) throw new Error('useKai must be used within KaiContext.Provider')
  return ctx
}
