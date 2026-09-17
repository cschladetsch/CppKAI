import { useEffect, useRef, useState } from 'react'
import { useKai } from '../context/KaiContext'
import type { Domain, KaiObject, Registry } from '../types/kai'

// ── Flash hook ────────────────────────────────────────────────────────────────

function useFlash(addr: string, objects: Record<string, KaiObject>) {
  const [flashing, setFlashing] = useState(false)
  const prev = useRef<Record<string, unknown> | null>(null)

  useEffect(() => {
    const cur = objects[addr]?.state ?? null
    if (prev.current !== null && cur !== prev.current) {
      setFlashing(true)
      const t = setTimeout(() => setFlashing(false), 300)
      return () => clearTimeout(t)
    }
    prev.current = cur
  }, [addr, objects])

  return flashing
}

// ── Object leaf ───────────────────────────────────────────────────────────────

function ObjectLeaf({
  obj,
  selected,
  objects,
  onSelect,
}: {
  obj: KaiObject
  selected: boolean
  objects: Record<string, KaiObject>
  onSelect: (addr: string) => void
}) {
  const flashing = useFlash(obj.addr, objects)

  return (
    <button
      onClick={() => onSelect(obj.addr)}
      className={[
        'w-full text-left px-3 py-0.5 font-mono text-xs flex items-center gap-2 transition-colors duration-150',
        selected
          ? 'bg-kai-muted text-kai-text'
          : 'text-kai-dim hover:text-kai-text hover:bg-kai-muted/40',
        flashing ? 'text-kai-copper' : '',
      ].join(' ')}
    >
      <span className="text-kai-muted">◆</span>
      <span className="truncate">{obj.addr}</span>
      {obj.type && obj.type !== 'unknown' && (
        <span className="ml-auto text-kai-muted shrink-0">{obj.type}</span>
      )}
    </button>
  )
}

// ── Registry section ──────────────────────────────────────────────────────────

function RegistrySection({
  reg,
  selectedAddr,
  objects,
  onSelect,
}: {
  reg: Registry
  selectedAddr: string | null
  objects: Record<string, KaiObject>
  onSelect: (addr: string) => void
}) {
  const [open, setOpen] = useState(true)

  return (
    <div>
      <button
        onClick={() => setOpen(o => !o)}
        className="w-full text-left px-3 py-0.5 flex items-center gap-1.5 text-kai-dim hover:text-kai-text font-mono text-xs"
      >
        <span className={`transition-transform duration-100 ${open ? 'rotate-90' : ''}`}>▶</span>
        <span>{reg.name}</span>
        <span className="ml-auto text-kai-muted">{reg.objects.length}</span>
      </button>
      {open && (
        <div className="pl-3">
          {reg.objects.length === 0 ? (
            <p className="px-3 py-0.5 font-mono text-xs text-kai-muted italic">empty</p>
          ) : (
            reg.objects.map(obj => (
              <ObjectLeaf
                key={obj.addr}
                obj={obj}
                selected={selectedAddr === obj.addr}
                objects={objects}
                onSelect={onSelect}
              />
            ))
          )}
        </div>
      )}
    </div>
  )
}

// ── Domain section ────────────────────────────────────────────────────────────

function DomainSection({
  domain,
  selectedAddr,
  objects,
  onSelect,
}: {
  domain: Domain
  selectedAddr: string | null
  objects: Record<string, KaiObject>
  onSelect: (addr: string) => void
}) {
  const [open, setOpen] = useState(true)

  return (
    <div className="mb-1">
      <button
        onClick={() => setOpen(o => !o)}
        className="w-full text-left px-3 py-1 flex items-center gap-1.5 text-kai-text font-sans text-xs font-medium"
      >
        <span className={`transition-transform duration-100 text-kai-copper ${open ? 'rotate-90' : ''}`}>▶</span>
        <span>{domain.name}</span>
      </button>
      {open && domain.registries.map(reg => (
        <RegistrySection
          key={reg.name}
          reg={reg}
          selectedAddr={selectedAddr}
          objects={objects}
          onSelect={onSelect}
        />
      ))}
    </div>
  )
}

// ── Panel ─────────────────────────────────────────────────────────────────────

export function RegistryTree() {
  const { state, selectAddr } = useKai()
  const { domains, selectedAddr, objects } = state

  return (
    <div className="flex flex-col h-full overflow-hidden">
      <div className="px-3 py-2 border-b border-kai-border">
        <span className="font-sans text-xs font-medium text-kai-dim uppercase tracking-wider">Registry</span>
      </div>
      <div className="flex-1 overflow-y-auto py-1">
        {domains.length === 0 ? (
          <p className="px-3 py-4 font-mono text-xs text-kai-muted text-center">
            waiting for tree…
          </p>
        ) : (
          domains.map(d => (
            <DomainSection
              key={d.name}
              domain={d}
              selectedAddr={selectedAddr}
              objects={objects}
              onSelect={selectAddr}
            />
          ))
        )}
      </div>
    </div>
  )
}
