import { useCallback, useEffect, useRef } from 'react'

interface Props {
  onDrag: (dx: number) => void
}

export function ResizeHandle({ onDrag }: Props) {
  const dragging = useRef(false)
  const lastX = useRef(0)

  const onMouseDown = useCallback((e: React.MouseEvent) => {
    e.preventDefault()
    dragging.current = true
    lastX.current = e.clientX
  }, [])

  useEffect(() => {
    const onMove = (e: MouseEvent) => {
      if (!dragging.current) return
      onDrag(e.clientX - lastX.current)
      lastX.current = e.clientX
    }
    const onUp = () => { dragging.current = false }

    window.addEventListener('mousemove', onMove)
    window.addEventListener('mouseup', onUp)
    return () => {
      window.removeEventListener('mousemove', onMove)
      window.removeEventListener('mouseup', onUp)
    }
  }, [onDrag])

  return (
    <div
      onMouseDown={onMouseDown}
      className="w-1 bg-kai-border hover:bg-kai-copper/50 cursor-col-resize shrink-0 transition-colors duration-150 active:bg-kai-copper/70"
    />
  )
}
