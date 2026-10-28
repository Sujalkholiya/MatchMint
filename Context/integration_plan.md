# MatchMint — Frontend-to-C-Backend End-to-End Integration Plan

This document establishes the verified architectural blueprint for connecting the MatchMint React frontend directly to the high-performance C backend without redesigning or altering the frontend UI design, styling, or UX layouts.

---

## 1. System Topology & Environment Configuration

### 1.1 Architecture
```text
┌──────────────────────────────────────────────┐
│        MatchMint Frontend (React 19)        │
│  - App.tsx / Dashboard / Competitions / etc. │
└───────────────────────┬──────────────────────┘
                        │ HTTP REST (JSON) / CORS
                        ▼
┌──────────────────────────────────────────────┐
│          C Backend (CivetWeb / C11)          │
│  - REST Router (/api/v1/*)                   │
│  - Auth Middleware & Controller Layer        │
│  - Service Layer & Business Rules            │
│  - DSA Modules (Tree, Sort, Cycles, Hash)    │
└───────────────────────┬──────────────────────┘
                        │ SQLite C API
                        ▼
┌──────────────────────────────────────────────┐
│           SQLite Database Engine             │
│  - matchmint.db (Foreign Keys, WAL Mode)     │
└──────────────────────────────────────────────┘
```

### 1.2 Frontend Environment Configuration
- A dedicated `.env` and `.env.example` file will be created in the frontend root:
  ```env
  VITE_API_BASE_URL=http://localhost:8080/api/v1
  ```
- The frontend will access this via `import.meta.env.VITE_API_BASE_URL || 'http://localhost:8080/api/v1'`, ensuring zero hardcoded URLs.
- Cross-Origin Resource Sharing (CORS) is handled by the C backend responding to `OPTIONS` preflight requests with:
  - `Access-Control-Allow-Origin: *` (or `http://localhost:5173`)
  - `Access-Control-Allow-Methods: GET, POST, PUT, PATCH, DELETE, OPTIONS`
  - `Access-Control-Allow-Headers: Content-Type, Authorization`

---

## 2. Frontend Service Layer Modernization

### 2.1 Centralized API Client (`src/services/apiClient.ts`)
A lightweight, typed fetch wrapper that handles:
- Configurable base URL.
- JSON request and response parsing.
- Error normalization matching the backend envelope `{ success, data, error }`.
- Authentication Bearer token attachment from `localStorage.getItem('matchmint_token')`.

### 2.2 Modernized `StorageService` (`src/services/storageService.ts`)
Currently, `StorageService` has static synchronous methods using `localStorage`. To integrate with the C backend while maintaining component stability, `StorageService` will be updated to expose async methods:
| Existing Synchronous Method | Async Backend Endpoint Integration |
|---|---|
| `getCompetitions()` | `GET /api/v1/competitions` |
| `getTeams(competitionId?)` | `GET /api/v1/teams?competitionId=...` |
| `getMatches()` | `GET /api/v1/matches` |
| `getHistory()` | `GET /api/v1/competitions/:id/history` or `GET /api/v1/history` |
| `createCompetition(data)` | `POST /api/v1/competitions` |
| `registerTeam(data)` | `POST /api/v1/teams` |
| `submitResult(matchId, s1, s2, tb)` | `POST /api/v1/matches/:id/result` |
| `rollbackResult(historyId)` | `POST /api/v1/history/:historyId/undo` |
| `generateFixtures(competitionId)` | `POST /api/v1/competitions/:id/generate-fixtures` |
| `rescheduleMatch(id, time, venue)` | `PATCH /api/v1/matches/:id/schedule` |
| `exportDatabaseJSON()` | `GET /api/v1/settings/backup/export` |
| `importDatabaseJSON(jsonStr)` | `POST /api/v1/settings/backup/import` |
| `resetToDefaults()` | `POST /api/v1/settings/reset` |

### 2.3 Authoritative DSA Operations in C
The existing client-side `src/services/tournamentEngine.ts` contains duplicate JavaScript logic for:
1. `generateSingleEliminationFixtures` $\rightarrow$ Delegated to C Backend binary tree engine.
2. `generateRoundRobinFixtures` $\rightarrow$ Delegated to C Backend cyclic fixed-pivot rotation engine.
3. `recalculateStandings` $\rightarrow$ Delegated to C Backend merge-sort engine.
4. `detectSchedulingConflicts` $\rightarrow$ Delegated to C Backend interval overlap detector (`POST /api/v1/competitions/:id/check-conflicts`).

*No client-side duplicate algorithms will act as the source of truth.*

---

## 3. Component Integration & Async State Handling

Each page and modal in the frontend will be cleanly connected to async backend operations with zero aesthetic disruption:

### 3.1 `App.tsx` (Root Controller)
- `reloadData()` is converted to an `async` function executing parallel `Promise.all` queries to `/competitions`, `/teams`, `/matches`, and `/history`.
- Component state updates cleanly when data arrives.
- Toast notifications (`showToast`) continue to display backend-generated success/error messages.

### 3.2 Modals
1. **`EnterResultModal.tsx`**:
   - Submits scores to backend `POST /api/v1/matches/:id/result`.
   - Backend automatically advances the winner to the next round slot (binary tree parent), recalculates standings (merge sort), and logs an undoable audit snapshot (linked list).
2. **`ScheduleMatchModal.tsx`**:
   - Calls backend `POST /api/v1/competitions/:id/check-conflicts` when time or venue changes.
   - Submits schedule update to `PATCH /api/v1/matches/:id/schedule`.
3. **`RegisterTeamModal.tsx`**:
   - Submits team data to `POST /api/v1/teams`.
   - On success, triggers parent data refresh and toast notification.

### 3.3 Pages
1. **`Dashboard.tsx`**:
   - Displays live aggregated metrics (`/dashboard/metrics`) and today's matches (`/dashboard/today-matches`).
2. **`Competitions.tsx` & `CompetitionDetails.tsx`**:
   - Fetches bracket tree structure directly from backend `GET /competitions/:id/bracket`.
   - Standings tab displays sorted standings from backend `GET /competitions/:id/standings`.
   - Activity tab displays history log from backend `GET /competitions/:id/history`.
   - "Undo" button triggers backend rollback `POST /history/:historyId/undo`.
3. **`CreateCompetition.tsx`**:
   - Calls backend `POST /competitions`. If "Generate fixtures now" is checked, the backend executes team placeholder registration and bracket generation inside a single atomic database transaction.
4. **`Matches.tsx`**:
   - Loads fixtures from `GET /matches`. Single-click "Undo last result" invokes backend rollback with safety checks.
5. **`Teams.tsx`**:
   - Displays real-time squads from `GET /teams`.
6. **`Settings.tsx`**:
   - Backs up, restores, and resets the SQLite database through backend backup routes.

---

## 4. Step-by-Step Implementation Roadmap

In accordance with the one-task-at-a-time rule:

- [x] **Task 1**: Initial inspection, environment check, and verified architecture plan.
- [x] **Task 1.1**: Detailed Frontend-to-C-Backend integration plan and API mapping.
- [ ] **Task 2**: Directory layout for `backend/`, CMake build system configuration (`CMakeLists.txt`), and minimal C11 compilation baseline with SQLite3 and cJSON.
- [ ] **Task 3**: SQLite3 database schema (`schema.sql`), initial seed migration, and database connection manager (`database.c`).
- [ ] **Task 4**: Core DSA Modules in C:
  - Binary Tree Knockout Bracket Engine (`bracket_tree.c`).
  - Cyclic Fixed-Pivot Round-Robin Engine (`cyclic_schedule.c`).
  - Merge Sort Multi-Attribute Standings Engine (`merge_sort.c`).
  - Interval Overlap Scheduling Conflict Detector (`interval_conflict.c`).
  - Linked List Audit Trail and Rollback Engine (`audit_list.c`).
- [ ] **Task 5**: Embedded C HTTP Server (CivetWeb / Mongoose), REST Router, and JSON Response serializer.
- [ ] **Task 6**: Competition & Team Management REST APIs.
- [ ] **Task 7**: Tournament Engine REST APIs (Fixture Generation, Bracket Tree endpoint, Match Scheduling).
- [ ] **Task 8**: Scorekeeping, Result Entry, Standings Recalculation, and One-Click Rollback APIs.
- [ ] **Task 9**: Dashboard Metrics, Venues, Backup Export/Import & Reset APIs.
- [ ] **Task 10**: Frontend Integration:
  - Add `VITE_API_BASE_URL` and `.env` configuration.
  - Implement `apiClient.ts`.
  - Connect `storageService.ts` and `tournamentEngine.ts` to backend APIs.
  - Connect `App.tsx` and all modal / page actions to async backend endpoints.
- [ ] **Task 11**: End-to-end testing of complete workflow (Tournament Creation $\rightarrow$ Registration $\rightarrow$ Fixture Generation $\rightarrow$ Match Scoring $\rightarrow$ Bracket Advancement $\rightarrow$ Standings $\rightarrow$ Undo Rollback $\rightarrow$ DB Persistence after restart).
