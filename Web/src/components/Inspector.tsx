import { useEffect, useRef, useState } from 'react'
import { useKai } from '../context/KaiContext'

// ── Per-key change flash ──────────────────────────────────────────────────────

function useChangedKeys(state: Record<string, unknown>) {
  const [changed, setChanged] = useState<Set<string>>(new Set())
  const prev = useRef<Record<string, unknown>>({})

  useEffect(() => {
    const keys = Object.keys(state)
    const newly = new Set(keys.filter(k => prev.current[k] !== state[k]))
    if (newly.size > 0) {
      setChanged(newly)
      const t = setTimeout(() => setChanged(new Set()), 400)
      prev.current = { ...state }
      return () => clearTimeout(t)
    }
    prev.current = { ...state }
  }, [state])

  return changed
}

// ── Value renderer ────────────────────────────────────────────────────────────

function Value({ v }: { v: unknown }) {
  if (v === null || v === undefined) return <span className="text-kai-muted">null</span>
  if (typeof v === 'boolean') return <span className="text-kai-purple">{String(v)}</span>
  if (typeof v === 'number')  return <span className="text-kai-blue">{v}</span>
  if (typeof v === 'string')  return <span className="text-kai-green">"{v}"</span>
  return <span className="text-kai-dim break-all">{JSON.stringify(v)}</span>
}

// ── Stack view (shown when no object selected) ────────────────────────────────

function StackView() {
  const { state } = useKai()
  const { stack } = state

  return (
    <div className="flex flex-col h-full overflow-hidden">
      <div className="px-3 py-2 border-b border-kai-border shrink-0">
        <span className="font-sans text-xs font-medium text-kai-dim uppercase tracking-wider">Stack</span>
        <span className="ml-2 font-mono text-xs text-kai-muted">{stack.length} item{stack.length !== 1 ? 's' : ''}</span>
      </div>
      {stack.length === 0 ? (
        <div className="flex-1 flex items-center justify-center">
          <p className="font-mono text-xs text-kai-muted">empty</p>
        </div>
      ) : (
        <div className="flex-1 overflow-y-auto">
          <table className="w-full">
            <tbody>
              {stack.map((item, i) => (
                <tr
                  key={i}
                  className={[
                    'border-b border-kai-border/50',
                    i === 0 ? 'bg-kai-muted/20' : '',
                  ].join(' ')}
                >
                  <td className="px-3 py-1.5 font-mono text-xs text-kai-dim align-top w-10 shrink-0">
                    [{stack.length - 1 - i}]
                  </td>
                  <td className="px-3 py-1.5 font-mono text-xs align-top">
                    <Value v={item} />
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </div>
  )
}

// ── Object inspector ──────────────────────────────────────────────────────────

function ObjectView() {
  const { state } = useKai()
  const { selectedAddr, objects } = state
  const obj = selectedAddr ? objects[selectedAddr] : null
  const changed = useChangedKeys(obj?.state ?? {})

  if (!obj) return null

  return (
    <div className="flex flex-col h-full overflow-hidden">
      <div className="px-3 py-2 border-b border-kai-border shrink-0">
        <span className="font-sans text-xs font-medium text-kai-dim uppercase tracking-wider">Inspector</span>
      </div>
      <div className="flex-1 overflow-y-auto">
        <div className="px-3 py-3 border-b border-kai-border space-y-1">
          <div className="font-mono text-xs text-kai-copper break-all">{obj.addr}</div>
          {obj.type && obj.type !== 'unknown' && (
            <div className="font-mono text-xs text-kai-dim">{obj.type}</div>
          )}
        </div>
        {Object.keys(obj.state).length === 0 ? (
          <p className="px-3 py-4 font-mono text-xs text-kai-muted italic text-center">no state</p>
        ) : (
          <table className="w-full">
            <tbody>
              {Object.entries(obj.state).map(([k, v]) => (
                <tr
                  key={k}
                  className={[
                    'border-b border-kai-border/50 transition-colors duration-300',
                    changed.has(k) ? 'bg-kai-copper/10' : '',
                  ].join(' ')}
                >
                  <td className="px-3 py-1.5 font-mono text-xs text-kai-dim align-top w-1/3">{k}</td>
                  <td className="px-3 py-1.5 font-mono text-xs align-top"><Value v={v} /></td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>
    </div>
  )
}

// ── Panel ─────────────────────────────────────────────────────────────────────

export function Inspector() {
  const { state } = useKai()

  return state.selectedAddr ? <ObjectView /> : <StackView />
}
