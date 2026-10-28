# MatchMint

A cinematic tournament management web application for campus **Sports** and **Esports** team-vs-team competitions.

[![React](https://img.shields.io/badge/React-19-blue?logo=react&logoColor=white)](https://react.dev/)
[![TypeScript](https://img.shields.io/badge/TypeScript-5-blue?logo=typescript&logoColor=white)](https://www.typescriptlang.org/)
[![Vite](https://img.shields.io/badge/Vite-8-purple?logo=vite&logoColor=white)](https://vite.dev/)
[![Tailwind CSS](https://img.shields.io/badge/TailwindCSS-3.4-38B2AC?logo=tailwind-css&logoColor=white)](https://tailwindcss.com/)
[![Framer Motion](https://img.shields.io/badge/Framer_Motion-12-ff0055?logo=framer&logoColor=white)](https://www.framer.com/motion/)

---

## Overview

MatchMint pairs a high-performance tournament engine with a cinematic, editorial aesthetic. It provides organizers with a distraction-free command deck to generate fair brackets, construct round-robin schedules, record scores in seconds, and execute instant one-click result rollbacks.

### Architecture & Navigation
- **Landing Page (`/`)**: A cinematic showcase featuring mouse-following magnetic physics, scroll-driven character text reveal, a dual-row scroll marquee, and sticky 3D stacking competition cards.
- **Tournament App (`/app`)**: The competition workspace with 5 core views:
  1. **Dashboard**: Featured active tournament, a 3-metric statistics row, and today's upcoming matches.
  2. **Competitions**: Directory of tournaments with category and format filters, plus interactive detail tabs (Bracket/Fixtures, Standings, Teams, Activity).
  3. **Matches**: Upcoming and Completed fixtures with score entry and subtle result undo.
  4. **Teams**: Clean squad roster table with modal registration.
  5. **Settings**: JSON backup export, backup import, and sample data reset.

---

## Visual Design & Palette

- **Typography**: Google Font **Kanit** (weights 300–900) set as the primary typeface across the application; **JetBrains Mono** strictly for scores, seeds, points, and timestamps using `tabular-nums`.
- **Canvas & Surfaces**:
  - Main background: `#0C0C0C`
  - Dark surfaces: `#12151a` and `#171a21`
  - Subtle borders: `#D7E2EA` at 15% opacity (`rgba(215, 226, 234, 0.15)`)
  - Primary text: `#D7E2EA` (muted text at 60% opacity)
- **Primary Pill Accent**:
  - Gradient: `linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)`
  - Inset & drop shadows: `0px 4px 4px rgba(181,1,167,0.25), 4px 4px 12px #7721B1 inset`
  - Outline: `2px solid #ffffff` with `-3px` outline-offset.
- **Active Accents**:
  - Left border for active sidebar items and 1st-place leaderboard ranking: `#B600A8`
  - Progress bars: `linear-gradient(to right, #B600A8, #7621B0)`
- **Hero Typography**:
  - `.hero-heading`: `linear-gradient(180deg, #646973 0%, #BBCCD7 100%)` with `-webkit-background-clip: text` and `-webkit-text-fill-color: transparent`.

---

## Animation & Motion Specifications

All landing components are powered by **Framer Motion 12**:
1. **FadeIn (`whileInView`)**: Viewport-triggered reveal with cubic bezier easing `[0.25, 0.1, 0.25, 1]`.
2. **Magnet**: Mouse-tracking physics that dynamically translates targets within padding range (`dx / strength`, `dy / strength`). Automatically disabled on touch screens and prefers-reduced-motion.
3. **AnimatedText**: Character-by-character scroll opacity reveal (`opacity: 0.2 → 1`) linked to viewport scroll progress.
4. **Scroll Marquee**: Dual horizontal tile tracks driven passively by window scroll (`requestAnimationFrame` throttled).
5. **Sticky Stacking Cards**: 3D scale transforms (`useTransform`) that stack tournaments as user scrolls through the showcase.
6. **Accessibility**: Strict compliance with `prefers-reduced-motion` across all components (disables magnet physics, marquee transforms, float effects, and letter-by-letter reveal).

---

## Expected Image Assets (`/public`)

MatchMint includes high-fidelity React component mockups as fallbacks for all image slots. To replace mockups with live screenshots or custom 3D renders, place files in the following `/public` directories:

### Hero
- `/public/images/hero.png` — 3D championship trophy or hero bracket render (transparent background recommended).

### Scroll Marquee (`/public/marquee/`)
- `/public/marquee/bracket.png` — Tournament knockout bracket view
- `/public/marquee/standings.png` — Points table and standings view
- `/public/marquee/match.png` — Match card with live score
- `/public/marquee/score-modal.png` — Focused score entry modal
- `/public/marquee/fixtures.png` — Round-robin fixture schedule
- `/public/marquee/teams.png` — Squad rosters and captains

### Competitions Showcase (`/public/projects/`)
- `/public/projects/gehu-winter-cup-2026-1.png` — GEHU Winter Cup bracket view (left top)
- `/public/projects/gehu-winter-cup-2026-2.png` — GEHU Winter Cup standings (left bottom)
- `/public/projects/gehu-winter-cup-2026-3.png` — GEHU Winter Cup overview (right tall)
- `/public/projects/cyberclash-masters-1.png` — CyberClash Masters bracket
- `/public/projects/cyberclash-masters-2.png` — CyberClash Masters standings
- `/public/projects/cyberclash-masters-3.png` — CyberClash Masters overview
- `/public/projects/inter-college-league-1.png` — Inter-College League fixtures
- `/public/projects/inter-college-league-2.png` — Inter-College League standings
- `/public/projects/inter-college-league-3.png` — Inter-College League overview

---

## Getting Started

### Installation

```bash
# Clone the repository
git clone https://github.com/Sujalkholiya/matchmint.git
cd matchmint

# Install dependencies
npm install

# Start development server
npm run dev
```

Open `http://localhost:5173/` in your browser.

### Building for Production

```bash
npm run build
```

Compiles all TypeScript declarations and builds production assets into `dist/`.
