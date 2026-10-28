/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  darkMode: 'class',
  theme: {
    extend: {
      colors: {
        dark: {
          bg: '#0C0C0C',
          surface: '#12151a',
          surfaceHover: '#171a21',
          border: 'rgba(215, 226, 234, 0.15)',
        },
        brand: {
          magenta: '#B600A8',
          purple: '#7621B0',
          orange: '#BE4C00',
          darkest: '#18011F',
          text: '#D7E2EA',
        },
        charcoal: {
          950: '#0C0C0C',
          900: '#101217',
          850: '#12151a',
          800: '#171a21',
          750: '#1e232c',
          700: 'rgba(215, 226, 234, 0.15)',
          600: 'rgba(215, 226, 234, 0.25)',
          500: 'rgba(215, 226, 234, 0.40)',
          400: 'rgba(215, 226, 234, 0.60)',
          300: 'rgba(215, 226, 234, 0.75)',
          200: 'rgba(215, 226, 234, 0.90)',
          100: '#D7E2EA',
        },
      },
      fontFamily: {
        sans: [
          'Kanit',
          '-apple-system',
          'BlinkMacSystemFont',
          'Segoe UI',
          'Roboto',
          'sans-serif'
        ],
        mono: [
          'JetBrains Mono',
          'ui-monospace',
          'SFMono-Regular',
          'Menlo',
          'Monaco',
          'Consolas',
          'monospace'
        ]
      },
      boxShadow: {
        'subtle': '0 1px 3px 0 rgba(0, 0, 0, 0.5)',
        'elevated': '0 4px 14px 0 rgba(0, 0, 0, 0.6)',
        'glow-purple': '0 0 20px -2px rgba(182, 0, 168, 0.35)',
        'primary-pill': '0px 4px 4px rgba(181, 1, 167, 0.25), inset 4px 4px 12px #7721B1',
      }
    },
  },
  plugins: [],
}
