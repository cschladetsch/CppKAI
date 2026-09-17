import '@testing-library/jest-dom'

// ── WebSocket mock ────────────────────────────────────────────────────────────
// A minimal mock that lets tests control open/message/close/error events.

export class MockWebSocket {
  static instances: MockWebSocket[] = []

  url: string
  readyState: number = WebSocket.CONNECTING

  onopen:    ((ev: Event) => void) | null = null
  onmessage: ((ev: MessageEvent) => void) | null = null
  onclose:   ((ev: CloseEvent) => void) | null = null
  onerror:   ((ev: Event) => void) | null = null

  private _sent: string[] = []

  constructor(url: string) {
    this.url = url
    MockWebSocket.instances.push(this)
  }

  send(data: string) {
    this._sent.push(data)
  }

  close() {
    this.readyState = WebSocket.CLOSED
    this.onclose?.(new CloseEvent('close'))
  }

  // ── Test helpers ──────────────────────────────────────────────────────────

  /** Simulate the connection being established. */
  open() {
    this.readyState = WebSocket.OPEN
    this.onopen?.(new Event('open'))
  }

  /** Simulate a message arriving from the server. */
  receive(data: unknown) {
    this.onmessage?.(new MessageEvent('message', { data: JSON.stringify(data) }))
  }

  /** Simulate a server-side close. */
  serverClose() {
    this.readyState = WebSocket.CLOSED
    this.onclose?.(new CloseEvent('close'))
  }

  /** All frames sent by the app. */
  get sent(): unknown[] {
    return this._sent.map(s => JSON.parse(s))
  }

  /** Last frame sent by the app. */
  get lastSent(): unknown | undefined {
    return this.sent[this.sent.length - 1]
  }

  static reset() {
    MockWebSocket.instances = []
  }

  static latest(): MockWebSocket {
    return MockWebSocket.instances[MockWebSocket.instances.length - 1]
  }
}

// Install globally
beforeEach(() => {
  MockWebSocket.reset()
  vi.stubGlobal('WebSocket', MockWebSocket)
})

afterEach(() => {
  vi.unstubAllGlobals()
})
