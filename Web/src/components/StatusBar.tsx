import { useKai } from '../context/KaiContext'
import type { ConnectionState } from '../types/kai'

const dot: Record<ConnectionState, string> = {
  connecting:   'bg-kai-copper animate-pulse',
  connected:    'bg-kai-green',
  disconnected: 'bg-kai-red',
}

const label: Record<ConnectionState, string> = {
  connecting:   'connecting',
  connected:    'connected',
  disconnected: 'disconnected — retrying',
}

export function StatusBar() {
  const { state } = useKai()
  const cs = state.connectionState

  return (
    <div className="flex items-center justify-between px-4 py-1.5 bg-kai-surface border-b border-kai-border select-none">
      <span className="font-mono text-xs font-medium text-kai-copper tracking-widest">KAI</span>
      <div className="flex items-center gap-2">
        <span className={`w-1.5 h-1.5 rounded-full ${dot[cs]}`} />
        <span className="font-mono text-xs text-kai-dim">{label[cs]}</span>
      </div>
    </div>
  )
}
