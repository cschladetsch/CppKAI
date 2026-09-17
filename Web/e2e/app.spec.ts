import { test, expect, Page } from '@playwright/test'

// ── WebSocket mock injected into the page ─────────────────────────────────────
//
// The bridge won't be running in CI. We intercept `new WebSocket(...)` and
// expose `window.__ws` so tests can simulate server frames and assert on
// frames the app sends.

async function injectWsMock(page: Page) {
  await page.addInitScript(() => {
    class FakeWS extends EventTarget {
      static instance: FakeWS | null = null
      readyState = 0 // CONNECTING
      sent: unknown[] = []

      constructor(_url: string) {
        super()
        FakeWS.instance = this
        ;(window as any).__ws = this
      }

      send(data: string) {
        this.sent.push(JSON.parse(data))
      }

      close() {
        this.readyState = 3
        this.dispatchEvent(new CloseEvent('close'))
      }

      // Test helpers called from the outside via page.evaluate
      _open() {
        this.readyState = 1
        this.dispatchEvent(new Event('open'))
      }

      _receive(frame: unknown) {
        this.dispatchEvent(
          new MessageEvent('message', { data: JSON.stringify(frame) })
        )
      }
    }

    ;(window as any).WebSocket = FakeWS
  })
}

async function openWs(page: Page) {
  await page.evaluate(() => (window as any).__ws._open())
}

async function receive(page: Page, frame: unknown) {
  await page.evaluate((f) => (window as any).__ws._receive(f), frame)
}

async function lastSent(page: Page): Promise<unknown> {
  return page.evaluate(() => {
    const ws = (window as any).__ws
    return ws?.sent[ws.sent.length - 1]
  })
}

// ── tests ─────────────────────────────────────────────────────────────────────

test.beforeEach(async ({ page }) => {
  await injectWsMock(page)
  await page.goto('/')
})

// ── layout ────────────────────────────────────────────────────────────────────

test('renders three panels', async ({ page }) => {
  await expect(page.getByText('Registry')).toBeVisible()
  await expect(page.getByText('REPL')).toBeVisible()
  await expect(page.getByText('Inspector')).toBeVisible()
})

test('shows KAI brand label in status bar', async ({ page }) => {
  await expect(page.getByText('KAI')).toBeVisible()
})

// ── connection status ─────────────────────────────────────────────────────────

test('shows connecting or disconnected before WS opens', async ({ page }) => {
  const text = await page.locator('text=/connecting|disconnected/').textContent()
  expect(text).toBeTruthy()
})

test('shows connected after WS opens', async ({ page }) => {
  await openWs(page)
  await expect(page.getByText('connected')).toBeVisible()
})

// ── registry tree ─────────────────────────────────────────────────────────────

test('shows waiting message before tree frame', async ({ page }) => {
  await expect(page.getByText(/waiting for tree/i)).toBeVisible()
})

test('renders domain and object from tree frame', async ({ page }) => {
  await openWs(page)
  await receive(page, {
    kind: 'tree',
    domains: [
      {
        name: 'default',
        registries: [
          {
            name: 'main',
            objects: [{ addr: '0:0:1', type: 'int', state: {} }],
          },
        ],
      },
    ],
  })
  await expect(page.getByText('default')).toBeVisible()
  await expect(page.getByText('0:0:1')).toBeVisible()
})

test('clicking an object selects it in the inspector', async ({ page }) => {
  await openWs(page)
  await receive(page, {
    kind: 'tree',
    domains: [
      {
        name: 'default',
        registries: [
          {
            name: 'main',
            objects: [{ addr: '0:0:1', type: 'int', state: { hp: 42 } }],
          },
        ],
      },
    ],
  })
  await receive(page, {
    kind: 'sup',
    addr: '0:0:1',
    state: { hp: 42 },
    ts: 1,
  })
  await page.getByText('0:0:1').click()
  await expect(page.getByText('hp')).toBeVisible()
  await expect(page.getByText('42')).toBeVisible()
})

// ── REPL ──────────────────────────────────────────────────────────────────────

test('typing and Shift+Enter sends eval frame', async ({ page }) => {
  await openWs(page)
  await page.getByPlaceholder('1 2 +').fill('1 2 +')
  await page.keyboard.press('Shift+Enter')
  const frame = await lastSent(page)
  expect(frame).toMatchObject({ kind: 'eval', src: '1 2 +', lang: 'pi' })
})

test('input is cleared after Shift+Enter', async ({ page }) => {
  await openWs(page)
  const textarea = page.getByPlaceholder('1 2 +')
  await textarea.fill('1 2 +')
  await page.keyboard.press('Shift+Enter')
  await expect(textarea).toHaveValue('')
})

test('result frame appears in repl output', async ({ page }) => {
  await openWs(page)
  await receive(page, { kind: 'result', src: '1 2 +', output: '3', ts: 1 })
  await expect(page.getByText('3')).toBeVisible()
})

test('error frame appears in repl output', async ({ page }) => {
  await openWs(page)
  await receive(page, { kind: 'error', msg: 'compile failed', ts: 1 })
  await expect(page.getByText('compile failed')).toBeVisible()
})

test('switching to rho changes lang in eval frame', async ({ page }) => {
  await openWs(page)
  await page.getByRole('button', { name: 'rho' }).click()
  await page.getByPlaceholder('x := 42').fill('x := 1')
  await page.keyboard.press('Shift+Enter')
  const frame = await lastSent(page)
  expect(frame).toMatchObject({ kind: 'eval', lang: 'rho' })
})

test('clear button empties repl output', async ({ page }) => {
  await openWs(page)
  await receive(page, { kind: 'result', src: 'a', output: '1', ts: 1 })
  await expect(page.getByText('1')).toBeVisible()
  await page.getByRole('button', { name: 'clear' }).click()
  await expect(page.getByText('1')).not.toBeVisible()
})

// ── inspector ─────────────────────────────────────────────────────────────────

test('inspector shows prompt when nothing selected', async ({ page }) => {
  await expect(page.getByText(/select an object/i)).toBeVisible()
})

test('inspector updates when SUP arrives for selected object', async ({ page }) => {
  await openWs(page)
  await receive(page, {
    kind: 'tree',
    domains: [
      {
        name: 'default',
        registries: [
          { name: 'main', objects: [{ addr: '0:0:1', type: 'int', state: {} }] },
        ],
      },
    ],
  })
  await page.getByText('0:0:1').click()
  await receive(page, { kind: 'sup', addr: '0:0:1', state: { hp: 77 }, ts: 1 })
  await expect(page.getByText('77')).toBeVisible()
})

// ── resize handles ────────────────────────────────────────────────────────────

test('resize handle exists between panels', async ({ page }) => {
  // Two resize handles between 3 panels
  const handles = page.locator('.cursor-col-resize')
  await expect(handles).toHaveCount(2)
})

test('dragging resize handle changes panel width', async ({ page }) => {
  const handle = page.locator('.cursor-col-resize').first()
  const box = await handle.boundingBox()
  if (!box) throw new Error('handle not found')

  // Get initial width of left panel (Registry)
  const leftPanel = page.locator('text=Registry').locator('../..')
  const beforeBox = await leftPanel.boundingBox()

  // Drag handle 50px right
  await page.mouse.move(box.x + box.width / 2, box.y + box.height / 2)
  await page.mouse.down()
  await page.mouse.move(box.x + box.width / 2 + 50, box.y + box.height / 2)
  await page.mouse.up()

  const afterBox = await leftPanel.boundingBox()
  // Width should have increased
  expect(afterBox?.width).toBeGreaterThan((beforeBox?.width ?? 0) + 10)
})
