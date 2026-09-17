import type { Config } from 'tailwindcss'

export default {
  content: ['./index.html', './src/**/*.{ts,tsx}'],
  theme: {
    extend: {
      fontFamily: {
        mono: ['JetBrains Mono', 'Fira Code', 'Consolas', 'monospace'],
        sans: ['Inter', 'system-ui', 'sans-serif'],
      },
      colors: {
        // KAI palette: deep navy base, copper accent, cool grays
        kai: {
          bg:       '#0d1117',
          surface:  '#161b22',
          border:   '#21262d',
          muted:    '#30363d',
          text:     '#e6edf3',
          dim:      '#7d8590',
          copper:   '#c9984a',
          green:    '#3fb950',
          red:      '#f85149',
          blue:     '#58a6ff',
          purple:   '#bc8cff',
        },
      },
    },
  },
  plugins: [],
} satisfies Config
