import { describe, it, expect, vi } from 'vitest'
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { StatusBar } from '../components/StatusBar'
import { RegistryTree } from '../components/RegistryTree'
import { Inspector } from '../components/Inspector'
import { Repl } from '../components/Repl'
import { mockSocket, withKai } from './helpers'
import type { Domain } from '../types/kai'

// ── StatusBar ─────────────────────────────────────────────────────────────────

describe('StatusBar', () => {
  it('shows "connected" when connected', () => {
    render(<StatusBar />, { wrapper: withKai(mockSocket({ connectionState: 'connected' })) })
    expect(screen.getByText('connected')).toBeInTheDocument()
  })

  it('shows "disconnected — retrying" when disconnected', () => {
    render(<StatusBar />, { wrapper: withKai(mockSocket({ connectionState: 'disconnected' })) })
    expect(screen.getByText('disconnected — retrying')).toBeInTheDocument()
  })

  it('shows "connecting" when connecting', () => {
    render(<StatusBar />, { wrapper: withKai(mockSocket({ connectionState: 'connecting' })) })
    expect(screen.getByText('connecting')).toBeInTheDocument()
  })

  it('renders the KAI brand label', () => {
    render(<StatusBar />, { wrapper: withKai(mockSocket()) })
    expect(screen.getByText('KAI')).toBeInTheDocument()
  })
})

// ── RegistryTree ──────────────────────────────────────────────────────────────

const domains: Domain[] = [
  {
    name: 'default',
    registries: [
      {
        name: 'main',
        objects: [
          { addr: '0:0:1', type: 'int', state: {} },
          { addr: '0:0:2', type: 'str', state: {} },
        ],
      },
    ],
  },
]

describe('RegistryTree', () => {
  it('shows waiting message when domains is empty', () => {
    render(<RegistryTree />, { wrapper: withKai(mockSocket()) })
    expect(screen.getByText(/waiting for tree/i)).toBeInTheDocument()
  })

  it('renders domain name', () => {
    render(<RegistryTree />, { wrapper: withKai(mockSocket({ domains })) })
    expect(screen.getByText('default')).toBeInTheDocument()
  })

  it('renders registry name', () => {
    render(<RegistryTree />, { wrapper: withKai(mockSocket({ domains })) })
    expect(screen.getByText('main')).toBeInTheDocument()
  })

  it('renders object addresses', () => {
    render(<RegistryTree />, { wrapper: withKai(mockSocket({ domains })) })
    expect(screen.getByText('0:0:1')).toBeInTheDocument()
    expect(screen.getByText('0:0:2')).toBeInTheDocument()
  })

  it('calls selectAddr when an object is clicked', async () => {
    const socket = mockSocket({ domains })
    render(<RegistryTree />, { wrapper: withKai(socket) })
    await userEvent.click(screen.getByText('0:0:1'))
    expect(socket.selectAddr).toHaveBeenCalledWith('0:0:1')
  })

  it('collapses registry on header click', async () => {
    render(<RegistryTree />, { wrapper: withKai(mockSocket({ domains })) })
    // Objects visible initially
    expect(screen.getByText('0:0:1')).toBeInTheDocument()
    // Click registry toggle
    await userEvent.click(screen.getByText('main'))
    expect(screen.queryByText('0:0:1')).not.toBeInTheDocument()
  })

  it('highlights selected object', () => {
    const socket = mockSocket({ domains, selectedAddr: '0:0:1' })
    const { container } = render(<RegistryTree />, { wrapper: withKai(socket) })
    // The selected button should have bg-kai-muted in its class
    const btn = screen.getByText('0:0:1').closest('button')
    expect(btn?.className).toContain('bg-kai-muted')
  })
})

// ── Inspector ─────────────────────────────────────────────────────────────────

describe('Inspector', () => {
  it('shows prompt when nothing selected', () => {
    render(<Inspector />, { wrapper: withKai(mockSocket()) })
    expect(screen.getByText(/select an object/i)).toBeInTheDocument()
  })

  it('shows object addr when selected', () => {
    const socket = mockSocket({
      selectedAddr: '1:0:5',
      objects: { '1:0:5': { addr: '1:0:5', type: 'float', state: { x: 3.14 } } },
    })
    render(<Inspector />, { wrapper: withKai(socket) })
    expect(screen.getByText('1:0:5')).toBeInTheDocument()
  })

  it('renders object type', () => {
    const socket = mockSocket({
      selectedAddr: '1:0:5',
      objects: { '1:0:5': { addr: '1:0:5', type: 'float', state: { x: 3.14 } } },
    })
    render(<Inspector />, { wrapper: withKai(socket) })
    expect(screen.getByText('float')).toBeInTheDocument()
  })

  it('renders state key/value pairs', () => {
    const socket = mockSocket({
      selectedAddr: '1:0:5',
      objects: { '1:0:5': { addr: '1:0:5', type: 'int', state: { hp: 100, mp: 50 } } },
    })
    render(<Inspector />, { wrapper: withKai(socket) })
    expect(screen.getByText('hp')).toBeInTheDocument()
    expect(screen.getByText('100')).toBeInTheDocument()
    expect(screen.getByText('mp')).toBeInTheDocument()
    expect(screen.getByText('50')).toBeInTheDocument()
  })

  it('shows "no state" for empty state object', () => {
    const socket = mockSocket({
      selectedAddr: '1:0:1',
      objects: { '1:0:1': { addr: '1:0:1', type: 'void', state: {} } },
    })
    render(<Inspector />, { wrapper: withKai(socket) })
    expect(screen.getByText(/no state/i)).toBeInTheDocument()
  })

  it('renders string values with quotes', () => {
    const socket = mockSocket({
      selectedAddr: '1:0:1',
      objects: { '1:0:1': { addr: '1:0:1', type: 'str', state: { name: 'hero' } } },
    })
    render(<Inspector />, { wrapper: withKai(socket) })
    expect(screen.getByText('"hero"')).toBeInTheDocument()
  })
})

// ── Repl ──────────────────────────────────────────────────────────────────────

describe('Repl', () => {
  it('renders empty hint when no repl entries', () => {
    render(<Repl />, { wrapper: withKai(mockSocket()) })
    expect(screen.getByText(/Shift\+Enter to evaluate/i)).toBeInTheDocument()
  })

  it('renders input entries', () => {
    const socket = mockSocket({
      repl: [{ id: 1, kind: 'input', text: '1 2 +', ts: 1 }],
    })
    render(<Repl />, { wrapper: withKai(socket) })
    expect(screen.getByText('1 2 +')).toBeInTheDocument()
  })

  it('renders result entries', () => {
    const socket = mockSocket({
      repl: [{ id: 1, kind: 'result', text: '3', src: '1 2 +', ts: 1 }],
    })
    render(<Repl />, { wrapper: withKai(socket) })
    expect(screen.getByText('3')).toBeInTheDocument()
  })

  it('renders error entries', () => {
    const socket = mockSocket({
      repl: [{ id: 1, kind: 'error', text: 'compile failed', ts: 1 }],
    })
    render(<Repl />, { wrapper: withKai(socket) })
    expect(screen.getByText('compile failed')).toBeInTheDocument()
  })

  it('calls evaluate on Shift+Enter', async () => {
    const socket = mockSocket()
    render(<Repl />, { wrapper: withKai(socket) })
    const textarea = screen.getByPlaceholderText(/1 2 \+/)
    await userEvent.type(textarea, '1 2 +')
    fireEvent.keyDown(textarea, { key: 'Enter', shiftKey: true })
    expect(socket.evaluate).toHaveBeenCalledWith('1 2 +', 'pi')
  })

  it('clears input after Shift+Enter', async () => {
    const socket = mockSocket()
    render(<Repl />, { wrapper: withKai(socket) })
    const textarea = screen.getByPlaceholderText(/1 2 \+/) as HTMLTextAreaElement
    await userEvent.type(textarea, '1 2 +')
    fireEvent.keyDown(textarea, { key: 'Enter', shiftKey: true })
    expect(textarea.value).toBe('')
  })

  it('does not call evaluate on plain Enter', async () => {
    const socket = mockSocket()
    render(<Repl />, { wrapper: withKai(socket) })
    const textarea = screen.getByPlaceholderText(/1 2 \+/)
    await userEvent.type(textarea, '1 2 +')
    fireEvent.keyDown(textarea, { key: 'Enter' })
    expect(socket.evaluate).not.toHaveBeenCalled()
  })

  it('switches language on pi/rho button click', async () => {
    const socket = mockSocket()
    render(<Repl />, { wrapper: withKai(socket) })
    await userEvent.click(screen.getByRole('button', { name: 'rho' }))
    // Placeholder should update to rho example
    expect(screen.getByPlaceholderText(/x := 42/)).toBeInTheDocument()
  })

  it('calls clearRepl on clear button click', async () => {
    const socket = mockSocket()
    render(<Repl />, { wrapper: withKai(socket) })
    await userEvent.click(screen.getByRole('button', { name: 'clear' }))
    expect(socket.clearRepl).toHaveBeenCalled()
  })

  it('passes lang to evaluate after switching to rho', async () => {
    const socket = mockSocket()
    render(<Repl />, { wrapper: withKai(socket) })
    await userEvent.click(screen.getByRole('button', { name: 'rho' }))
    const textarea = screen.getByPlaceholderText(/x := 42/)
    await userEvent.type(textarea, 'x := 1')
    fireEvent.keyDown(textarea, { key: 'Enter', shiftKey: true })
    expect(socket.evaluate).toHaveBeenCalledWith('x := 1', 'rho')
  })
})
