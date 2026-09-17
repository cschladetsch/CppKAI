import { useCallback, useState } from 'react'
import { KaiContext } from './context/KaiContext'
import { useKaiSocket } from './hooks/useKaiSocket'
import { StatusBar } from './components/StatusBar'
import { RegistryTree } from './components/RegistryTree'
import { Repl } from './components/Repl'
import { Inspector } from './components/Inspector'
import { ResizeHandle } from './components/ResizeHandle'

const WS_URL = 'ws://localhost:7171/ws'
const STORAGE_KEY = 'kai-panel-widths'
const MIN_PX = 160

function loadWidths(): [number, number, number] {
  try {
    const raw = localStorage.getItem(STORAGE_KEY)
    if (raw) return JSON.parse(raw)
  } catch { /* ignore */ }
  return [20, 50, 30]
}

function saveWidths(w: [number, number, number]) {
  try { localStorage.setItem(STORAGE_KEY, JSON.stringify(w)) } catch { /* ignore */ }
}

function clamp(widths: [number, number, number], totalPx: number): [number, number, number] {
  const minPct = (MIN_PX / totalPx) * 100
  const [a, b, c] = widths
  const total = a + b + c
  // normalize then clamp each to minPct
  const norm: [number, number, number] = [
    (a / total) * 100,
    (b / total) * 100,
    (c / total) * 100,
  ]
  for (let i = 0; i < 3; i++) {
    if (norm[i] < minPct) norm[i] = minPct
  }
  const sum = norm.reduce((s, v) => s + v, 0)
  return norm.map(v => (v / sum) * 100) as [number, number, number]
}

export default function App() {
  const socket = useKaiSocket(WS_URL)
  const [widths, setWidths] = useState<[number, number, number]>(loadWidths)

  const resizeLeft = useCallback((dx: number) => {
    setWidths(prev => {
      const totalPx = window.innerWidth
      const dPct = (dx / totalPx) * 100
      const next: [number, number, number] = [
        prev[0] + dPct,
        prev[1] - dPct,
        prev[2],
      ]
      const clamped = clamp(next, totalPx)
      saveWidths(clamped)
      return clamped
    })
  }, [])

  const resizeRight = useCallback((dx: number) => {
    setWidths(prev => {
      const totalPx = window.innerWidth
      const dPct = (dx / totalPx) * 100
      const next: [number, number, number] = [
        prev[0],
        prev[1] + dPct,
        prev[2] - dPct,
      ]
      const clamped = clamp(next, totalPx)
      saveWidths(clamped)
      return clamped
    })
  }, [])

  return (
    <KaiContext.Provider value={socket}>
      <div className="flex flex-col h-screen bg-kai-bg text-kai-text overflow-hidden">
        <StatusBar />
        <div className="flex flex-1 overflow-hidden">
          {/* Left: Registry Tree */}
          <div
            style={{ width: `${widths[0]}%` }}
            className="overflow-hidden bg-kai-surface border-r border-kai-border"
          >
            <RegistryTree />
          </div>

          <ResizeHandle onDrag={resizeLeft} />

          {/* Centre: REPL */}
          <div
            style={{ width: `${widths[1]}%` }}
            className="overflow-hidden bg-kai-bg"
          >
            <Repl />
          </div>

          <ResizeHandle onDrag={resizeRight} />

          {/* Right: Inspector */}
          <div
            style={{ width: `${widths[2]}%` }}
            className="overflow-hidden bg-kai-surface border-l border-kai-border"
          >
            <Inspector />
          </div>
        </div>
      </div>
    </KaiContext.Provider>
  )
}
