import {
  useCallback,
  useEffect,
  useRef,
  useState,
  KeyboardEvent,
} from 'react'
import { useKai } from '../context/KaiContext'
import type { Lang, ReplEntry } from '../types/kai'

// ── Output entry ──────────────────────────────────────────────────────────────

function Entry({ entry }: { entry: ReplEntry }) {
  const ts = new Date(entry.ts).toLocaleTimeString('en-AU', {
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
    hour12: false,
  })

  if (entry.kind === 'input') {
    return (
      <div className="flex gap-2 items-start py-0.5">
        <span className="text-kai-copper shrink-0 select-none">π›</span>
        <span className="text-kai-text break-all">{entry.text}</span>
        <span className="ml-auto text-kai-muted shrink-0 pl-4">{ts}</span>
      </div>
    )
  }

  if (entry.kind === 'result') {
    return (
      <div className="flex gap-2 items-start py-0.5 pl-4">
        <span className="text-kai-dim shrink-0 select-none">←</span>
        <span className="text-kai-green break-all">{entry.text}</span>
      </div>
    )
  }

  return (
    <div className="flex gap-2 items-start py-0.5 pl-4">
      <span className="text-kai-red shrink-0 select-none">!</span>
      <span className="text-kai-red break-all">{entry.text}</span>
    </div>
  )
}

// ── Panel ─────────────────────────────────────────────────────────────────────

export function Repl() {
  const { state, evaluate, clearRepl } = useKai()
  const [src, setSrc] = useState('')
  const [lang, setLang] = useState<Lang>('pi')
  const [histIdx, setHistIdx] = useState(-1)
  const outputRef = useRef<HTMLDivElement>(null)
  const inputRef = useRef<HTMLTextAreaElement>(null)

  // Scroll output to bottom when new entries arrive
  useEffect(() => {
    const el = outputRef.current
    if (el) el.scrollTop = el.scrollHeight
  }, [state.repl.length])

  // History: only input entries
  const history = state.repl
    .filter(e => e.kind === 'input')
    .map(e => e.text)

  const submit = useCallback(() => {
    const trimmed = src.trim()
    if (!trimmed) return
    evaluate(trimmed, lang)
    setSrc('')
    setHistIdx(-1)
  }, [src, lang, evaluate])

  const onKeyDown = useCallback(
    (e: KeyboardEvent<HTMLTextAreaElement>) => {
      if (e.key === 'Enter' && e.shiftKey) {
        e.preventDefault()
        submit()
        return
      }

      // History navigation
      const lines = src.split('\n')
      const atFirstLine =
        (inputRef.current?.selectionStart ?? 0) <= lines[0].length

      const atLastLine = (() => {
        const pos = inputRef.current?.selectionStart ?? 0
        const total = src.length
        const lastNewline = src.lastIndexOf('\n')
        return pos > lastNewline && pos <= total
      })()

      if (e.key === 'ArrowUp' && atFirstLine && history.length > 0) {
        e.preventDefault()
        const next = Math.min(histIdx + 1, history.length - 1)
        setHistIdx(next)
        setSrc(history[history.length - 1 - next] ?? '')
      }

      if (e.key === 'ArrowDown' && atLastLine) {
        e.preventDefault()
        if (histIdx <= 0) {
          setHistIdx(-1)
          setSrc('')
        } else {
          const next = histIdx - 1
          setHistIdx(next)
          setSrc(history[history.length - 1 - next] ?? '')
        }
      }
    },
    [src, histIdx, history, submit],
  )

  const prompt = lang === 'pi' ? 'π›' : 'ρ›'

  return (
    <div className="flex flex-col h-full overflow-hidden">
      {/* Header */}
      <div className="flex items-center gap-3 px-3 py-2 border-b border-kai-border shrink-0">
        <span className="font-sans text-xs font-medium text-kai-dim uppercase tracking-wider">REPL</span>
        <div className="flex items-center gap-1 ml-auto">
          {(['pi', 'rho'] as Lang[]).map(l => (
            <button
              key={l}
              onClick={() => setLang(l)}
              className={[
                'px-2 py-0.5 rounded font-mono text-xs transition-colors',
                lang === l
                  ? 'bg-kai-muted text-kai-text'
                  : 'text-kai-muted hover:text-kai-dim',
              ].join(' ')}
            >
              {l}
            </button>
          ))}
          <button
            onClick={clearRepl}
            className="ml-2 px-2 py-0.5 font-mono text-xs text-kai-muted hover:text-kai-dim transition-colors"
          >
            clear
          </button>
        </div>
      </div>

      {/* Output */}
      <div
        ref={outputRef}
        className="flex-1 overflow-y-auto px-3 py-2 font-mono text-xs space-y-0.5"
      >
        {state.repl.length === 0 && (
          <p className="text-kai-muted italic text-center py-8">
            Shift+Enter to evaluate · ↑↓ for history
          </p>
        )}
        {state.repl.map(entry => (
          <Entry key={entry.id} entry={entry} />
        ))}
      </div>

      {/* Input */}
      <div className="border-t border-kai-border shrink-0 flex items-start gap-2 px-3 py-2">
        <span className="font-mono text-xs text-kai-copper pt-1 select-none shrink-0">
          {prompt}
        </span>
        <textarea
          ref={inputRef}
          value={src}
          onChange={e => setSrc(e.target.value)}
          onKeyDown={onKeyDown}
          rows={src.split('\n').length || 1}
          className={[
            'flex-1 bg-transparent font-mono text-xs text-kai-text resize-none outline-none',
            'placeholder:text-kai-muted leading-5',
          ].join(' ')}
          placeholder={lang === 'pi' ? '1 2 +' : 'x := 42'}
          spellCheck={false}
          autoCapitalize="none"
          autoCorrect="off"
        />
      </div>
    </div>
  )
}
