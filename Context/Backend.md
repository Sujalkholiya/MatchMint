# MatchMint — Comprehensive Backend API Specification

This document provides the complete, production-ready specification for all Backend APIs required to power the **MatchMint** frontend application. 

It covers every feature present in the application: **Landing Page showcases, Organizer Command Deck, Tournaments/Competitions, Team Rosters, Single-Elimination Binary Tree Bracket Engine, Round-Robin Cyclic Engine, Match Scheduling & Conflict Detection, Real-time Score Entry & Tie-breakers, One-Click Result Rollbacks (Audit Trails), Points Table & Standings Recalculation, and Database Backup/Import/Reset**.

---

## 📑 Table of Contents
1. [Architecture & System Design Overview](#1-architecture--system-design-overview)
2. [Data Models & Entities](#2-data-models--entities)
3. [Authentication & Session APIs](#3-authentication--session-apis)
4. [Authorization & RBAC (Role-Based Access Control)](#4-authorization--rbac-apis)
5. [User & Organizer Profile APIs](#5-user--organizer-profile-apis)
6. [Competition & Tournament Management APIs](#6-competition--tournament-management-apis)
7. [Team & Squad Roster APIs](#7-team--squad-roster-apis)
8. [Tournament Engine, Brackets & Fixture Generation APIs](#8-tournament-engine-brackets--fixture-generation-apis)
9. [Match Operations & Scheduling APIs](#9-match-operations--scheduling-apis)
10. [Scorekeeping, Result Entry & Deciders APIs](#10-scorekeeping-result-entry--deciders-apis)
11. [Audit Trail, History & One-Click Rollback APIs](#11-audit-trail-history--one-click-rollback-apis)
12. [Standings & Points Table APIs](#12-standings--points-table-apis)
13. [Dashboard & Analytics APIs](#13-dashboard--analytics-apis)
14. [Venues & Scheduling Conflict Detection APIs](#14-venues--scheduling-conflict-detection-apis)
15. [Backup, Export, Import & Database Reset APIs](#15-backup-export-import--database-reset-apis)
16. [Real-time / WebSocket Events Specification](#16-real-time--websocket-events-specification)
17. [Standard API Error Codes & Responses](#17-standard-api-error-codes--responses)

---

## 1. Architecture & System Design Overview

- **Base URL**: `/api/v1`
- **Communication Protocol**: RESTful JSON over HTTPS + WebSocket / Server-Sent Events (SSE) for live scores and bracket updates.
- **Authentication Strategy**: Bearer Token (JWT access token + rotating Refresh Token via HttpOnly cookie or secure payload).
- **Global Headers**:
  - `Content-Type: application/json`
  - `Authorization: Bearer <access_token>` (for protected endpoints)
- **Standard Response Envelope**:
  ```json
  {
    "success": true,
    "message": "Operation completed successfully",
    "data": { ... },
    "meta": {
      "page": 1,
      "limit": 20,
      "total": 100
    }
  }
  ```

---

## 2. Data Models & Entities

### 2.1 User
| Field | Type | Description |
|---|---|---|
| `id` | UUID / String | Unique User ID |
| `name` | String | User display name |
| `email` | String | Unique email (e.g. campus email) |
| `passwordHash` | String | Argon2id / Bcrypt hashed password |
| `role` | Enum | `SUPER_ADMIN`, `ORGANIZER`, `REFEREE`, `CAPTAIN`, `SPECTATOR` |
| `avatarUrl` | String? | Profile image URL |
| `campus` | String? | University / Department affiliation |
| `createdAt` / `updatedAt` | Timestamp | Record timestamps |

### 2.2 Competition
| Field | Type | Description |
|---|---|---|
| `id` | String | Unique tournament identifier (e.g. `comp-gehu-2026`) |
| `name` | String | Tournament title (e.g. "GEHU Winter Cup 2026") |
| `category` | Enum | `sports` or `esports` |
| `format` | Enum | `single_elimination` or `round_robin` |
| `status` | Enum | `upcoming`, `live`, `completed` |
| `description` | String | Editorial summary |
| `totalTeams` | Integer | Max expected teams (e.g. 4, 6, 8, 16) |
| `registeredTeamsCount` | Integer | Current registered squads count |
| `currentRound` | Integer | Active round number (1-indexed) |
| `totalRounds` | Integer | Total rounds in tournament structure |
| `venues` | Array<String> | Assigned arenas/pitches/labs |
| `matchDurationMinutes` | Integer | Duration in minutes (e.g. 30, 50, 60, 75, 90) |
| `startDate` | Date / String | YYYY-MM-DD |
| `endDate` | Date / String | YYYY-MM-DD |
| `pointsForWin` | Integer | Default: 3 |
| `pointsForDraw` | Integer | Default: 1 (or 0 for knockout) |
| `pointsForLoss` | Integer | Default: 0 |
| `createdBy` | String (FK) | Organizer User ID |

### 2.3 Team
| Field | Type | Description |
|---|---|---|
| `id` | String | Unique team identifier |
| `competitionId` | String (FK) | Competition foreign key |
| `name` | String | Team name (e.g. "Falcons") |
| `shortTag` | String | 3-4 character tag (e.g. "FLC") |
| `seed` | Integer | Seeding number (1, 2, 3...) |
| `captain` | String | Captain name |
| `email` | String | Captain contact email |
| `rosterCount` | Integer | Number of squad members |
| `wins` | Integer | Total wins |
| `draws` | Integer | Total draws |
| `losses` | Integer | Total losses |
| `points` | Integer | Total league points |
| `scoreFor` | Integer | Total points/goals scored |
| `scoreAgainst` | Integer | Total points/goals conceded |
| `scoreDiff` | Integer | Net difference (`scoreFor - scoreAgainst`) |
| `rank` | Integer | Current position on the leaderboard |
| `status` | Enum | `active` or `eliminated` |

### 2.4 Match
| Field | Type | Description |
|---|---|---|
| `id` | String | Unique match ID (e.g. `match-comp-gehu-2026-r1-m1`) |
| `competitionId` | String (FK) | Foreign key to Competition |
| `round` | Integer | Round sequence number |
| `roundName` | String | E.g. "Quarter-Final 1", "Round 1", "Final" |
| `matchNumber` | Integer | Global match sequence number |
| `team1Id` | String? (FK) | Home team ID (nullable for TBD downstream) |
| `team2Id` | String? (FK) | Away team ID (nullable for TBD downstream) |
| `team1Score` | Integer? | Home score |
| `team2Score` | Integer? | Away score |
| `winnerId` | String? (FK) | Winner team ID |
| `status` | Enum | `upcoming` or `completed` |
| `scheduledTime` | Timestamp/Str | Scheduled date and time |
| `venue` | String | Venue name |
| `nextMatchId` | String? (FK) | Target match for single-elimination progression |
| `nextMatchSlot` | Enum? (1 \| 2) | Target slot in the next match |
| `tieBreakerDetails` | String? | E.g. "5-4 on penalties", "Overtime Round 2" |

### 2.5 HistoryRecord (Audit & Rollback Log)
| Field | Type | Description |
|---|---|---|
| `id` | String | Unique audit entry ID (e.g. `hist-1730000000`) |
| `competitionId` | String (FK) | Associated competition |
| `matchId` | String? (FK) | Target match ID |
| `actionType` | Enum | `RESULT_ENTERED`, `RESULT_REVERTED`, `FIXTURES_GENERATED` |
| `description` | String | Human readable description |
| `timestamp` | Timestamp | Time of event |
| `previousState` | JSON? | Snapshot: `{ team1Score, team2Score, winnerId, status }` |
| `canUndo` | Boolean | True if eligible for one-click rollback |
| `performedBy` | String (FK) | User ID of organizer |

---

## 3. Authentication & Session APIs

### 3.1 Register User
- **Method**: `POST`
- **Route**: `/api/v1/auth/register`
- **Access**: Public
- **Request Body**:
  ```json
  {
    "name": "Aarav Sharma",
    "email": "aarav.falcons@campus.edu",
    "password": "StrongPassword123!",
    "campus": "Graphic Era Hill University",
    "role": "ORGANIZER"
  }
  ```
- **Response (201 Created)**:
  ```json
  {
    "success": true,
    "message": "User registered successfully",
    "data": {
      "user": {
        "id": "usr-101",
        "name": "Aarav Sharma",
        "email": "aarav.falcons@campus.edu",
        "role": "ORGANIZER",
        "campus": "Graphic Era Hill University"
      },
      "accessToken": "eyJhbGciOi...",
      "refreshToken": "d8a1c9e..."
    }
  }
  ```

### 3.2 Login
- **Method**: `POST`
- **Route**: `/api/v1/auth/login`
- **Access**: Public
- **Request Body**:
  ```json
  {
    "email": "aarav.falcons@campus.edu",
    "password": "StrongPassword123!"
  }
  ```
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "message": "Login successful",
    "data": {
      "user": {
        "id": "usr-101",
        "name": "Aarav Sharma",
        "email": "aarav.falcons@campus.edu",
        "role": "ORGANIZER"
      },
      "accessToken": "eyJhbGciOi...",
      "refreshToken": "d8a1c9e..."
    }
  }
  ```

### 3.3 Refresh Access Token
- **Method**: `POST`
- **Route**: `/api/v1/auth/refresh-token`
- **Access**: Public (Valid Refresh Token required)
- **Request Body**:
  ```json
  {
    "refreshToken": "d8a1c9e..."
  }
  ```
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "data": {
      "accessToken": "eyJhbGciOi...",
      "refreshToken": "new_refresh_token_string"
    }
  }
  ```

### 3.4 Logout
- **Method**: `POST`
- **Route**: `/api/v1/auth/logout`
- **Access**: Protected (Bearer Token)
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "message": "Session invalidated successfully"
  }
  ```

### 3.5 Forgot & Reset Password
- `POST /api/v1/auth/forgot-password` (Body: `{ "email": "..." }`) -> Sends reset email / code.
- `POST /api/v1/auth/reset-password` (Body: `{ "token": "...", "newPassword": "..." }`) -> Resets password.

---

## 4. Authorization & RBAC APIs

### 4.1 Permission Matrix
| Role | View Tournaments & Brackets | Register Teams | Create Tournaments | Edit Schedules | Enter / Undo Scores | DB Backup / Restore |
|---|:---:|:---:|:---:|:---:|:---:|:---:|
| **SPECTATOR** | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **CAPTAIN** | ✅ | ✅ (Own Squad) | ❌ | ❌ | ❌ | ❌ |
| **REFEREE** | ✅ | ❌ | ❌ | ❌ | ✅ (Assigned) | ❌ |
| **ORGANIZER** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **SUPER_ADMIN** | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |

### 4.2 List Roles & Permissions
- **Method**: `GET`
- **Route**: `/api/v1/auth/roles`
- **Access**: Protected (`SUPER_ADMIN`, `ORGANIZER`)

### 4.3 Assign Role to User
- **Method**: `PATCH`
- **Route**: `/api/v1/auth/users/:userId/role`
- **Access**: Protected (`SUPER_ADMIN`)
- **Request Body**:
  ```json
  {
    "role": "ORGANIZER"
  }
  ```

---

## 5. User & Organizer Profile APIs

### 5.1 Get Current User (`me`)
- **Method**: `GET`
- **Route**: `/api/v1/users/me`
- **Access**: Protected (Bearer Token)
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "data": {
      "id": "usr-101",
      "name": "Campus Admin",
      "email": "admin@campus.edu",
      "role": "ORGANIZER",
      "campus": "Graphic Era Hill University",
      "avatarUrl": null
    }
  }
  ```

### 5.2 Update Current User Profile
- **Method**: `PUT`
- **Route**: `/api/v1/users/me`
- **Access**: Protected
- **Request Body**:
  ```json
  {
    "name": "Campus Lead Admin",
    "campus": "Main Campus Arena"
  }
  ```

### 5.3 Change Password
- **Method**: `PATCH`
- **Route**: `/api/v1/users/me/password`
- **Access**: Protected
- **Request Body**:
  ```json
  {
    "currentPassword": "OldPassword123!",
    "newPassword": "NewStrongPassword456!"
  }
  ```

---

## 6. Competition & Tournament Management APIs

### 6.1 List Competitions
- **Method**: `GET`
- **Route**: `/api/v1/competitions`
- **Access**: Public
- **Query Parameters**:
  - `category` (optional): `all` | `sports` | `esports`
  - `format` (optional): `all` | `single_elimination` | `round_robin`
  - `status` (optional): `upcoming` | `live` | `completed`
  - `search` (optional): string
  - `page` / `limit`: Pagination parameters
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "data": [
      {
        "id": "comp-gehu-2026",
        "name": "GEHU Winter Cup 2026",
        "category": "sports",
        "format": "single_elimination",
        "status": "live",
        "description": "Inter-department 7v7 football championship...",
        "totalTeams": 8,
        "registeredTeamsCount": 8,
        "currentRound": 2,
        "totalRounds": 3,
        "venues": ["Main Stadium Turf", "East Arena Field 2"],
        "matchDurationMinutes": 60,
        "startDate": "2026-09-24",
        "endDate": "2026-09-28",
        "pointsForWin": 3,
        "pointsForDraw": 1,
        "pointsForLoss": 0
      }
    ]
  }
  ```

### 6.2 Get Single Competition Details
- **Method**: `GET`
- **Route**: `/api/v1/competitions/:id`
- **Access**: Public
- **Response (200 OK)**: Returns full competition object with team count, match count, and rounds.

### 6.3 Create New Competition
- **Method**: `POST`
- **Route**: `/api/v1/competitions`
- **Access**: Protected (`ORGANIZER`, `SUPER_ADMIN`)
- **Request Body**:
  ```json
  {
    "name": "CyberClash Masters 2026",
    "category": "esports",
    "format": "single_elimination",
    "totalTeams": 8,
    "startDate": "2026-10-30",
    "matchDurationMinutes": 60,
    "venues": ["Main Arena Pod 1", "Arena Pod 2"],
    "autoGeneratePlaceholderTeams": true,
    "autoGenerateFixtures": true
  }
  ```
- **Response (201 Created)**: Returns the newly created competition object.

### 6.4 Update Competition Settings
- **Method**: `PUT` / `PATCH`
- **Route**: `/api/v1/competitions/:id`
- **Access**: Protected (`ORGANIZER`, `SUPER_ADMIN`)
- **Request Body**:
  ```json
  {
    "name": "GEHU Winter Cup 2026 - Finals Edition",
    "description": "Updated championship description",
    "venues": ["Main Stadium", "East Field 2", "Arena 3"],
    "status": "live"
  }
  ```

### 6.5 Delete / Archive Competition
- **Method**: `DELETE`
- **Route**: `/api/v1/competitions/:id`
- **Access**: Protected (`ORGANIZER`, `SUPER_ADMIN`)
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "message": "Competition deleted successfully"
  }
  ```

---

## 7. Team & Squad Roster APIs

### 7.1 List Teams
- **Method**: `GET`
- **Route**: `/api/v1/teams`
- **Access**: Public
- **Query Parameters**:
  - `competitionId` (optional): Filter teams participating in a specific competition.
  - `status` (optional): `active` | `eliminated`
  - `search` (optional): Filter by name or short tag.
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "data": [
      {
        "id": "team-falcons",
        "competitionId": "comp-gehu-2026",
        "name": "Falcons",
        "shortTag": "FLC",
        "seed": 1,
        "captain": "Aarav Sharma",
        "email": "aarav.falcons@campus.edu",
        "rosterCount": 11,
        "wins": 2,
        "draws": 0,
        "losses": 0,
        "points": 6,
        "scoreFor": 5,
        "scoreAgainst": 1,
        "scoreDiff": 4,
        "rank": 1,
        "status": "active"
      }
    ]
  }
  ```

### 7.2 Get Team Details
- **Method**: `GET`
- **Route**: `/api/v1/teams/:id`
- **Access**: Public

### 7.3 Register New Team (Add Squad)
- **Method**: `POST`
- **Route**: `/api/v1/teams`
- **Access**: Protected (`ORGANIZER`, `CAPTAIN`, `SUPER_ADMIN`)
- **Request Body**:
  ```json
  {
    "competitionId": "comp-gehu-2026",
    "name": "Viper Strikers",
    "captain": "Rohit Negi",
    "email": "rohit.viper@campus.edu",
    "rosterCount": 7
  }
  ```
- **Business Logic**:
  - Automatically derives `shortTag` (e.g. `VIP`).
  - Sets initial stats (`wins: 0`, `draws: 0`, `losses: 0`, `points: 0`, `rank`).
  - Increments `registeredTeamsCount` on the parent competition.
- **Response (201 Created)**: Returns the newly registered Team object.

### 7.4 Update Team Information
- **Method**: `PUT` / `PATCH`
- **Route**: `/api/v1/teams/:id`
- **Access**: Protected (`ORGANIZER`, `CAPTAIN`)
- **Request Body**:
  ```json
  {
    "name": "Viper Strikers Elite",
    "captain": "Rohit Negi",
    "rosterCount": 8,
    "email": "rohit.viper@campus.edu"
  }
  ```

### 7.5 Delete / Remove Team
- **Method**: `DELETE`
- **Route**: `/api/v1/teams/:id`
- **Access**: Protected (`ORGANIZER`, `SUPER_ADMIN`)
- **Business Logic**: Checks if tournament fixtures have started; if so, prevents deletion or handles BYE assignment.

---

## 8. Tournament Engine, Brackets & Fixture Generation APIs

### 8.1 Generate Fixtures for Competition
- **Method**: `POST`
- **Route**: `/api/v1/competitions/:id/generate-fixtures`
- **Access**: Protected (`ORGANIZER`, `SUPER_ADMIN`)
- **Request Body**:
  ```json
  {
    "seedingMethod": "standard_seed_fold",
    "resetExisting": false
  }
  ```
- **Tournament Engine Algorithms Executed on Backend**:
  1. **Single Elimination**:
     - Binary tree bracket construction (`totalSlots = 2^roundsCount`).
     - Fold pairing for Round 1: `(1 vs 8)`, `(4 vs 5)`, `(2 vs 7)`, `(3 vs 6)`.
     - Creation of downstream match slots with `nextMatchId` and `nextMatchSlot (1 | 2)`.
  2. **Round Robin**:
     - Cyclic schedule generation (`teamList.length - 1` rounds).
     - Fixed pivot rotation algorithm.
     - Odd-team `BYE` handling.
  3. Updates competition status to `live`, `currentRound: 1`.
  4. Appends audit log `FIXTURES_GENERATED`.
- **Response (201 Created)**:
  ```json
  {
    "success": true,
    "message": "Generated 7 fixtures for GEHU Winter Cup 2026.",
    "data": {
      "competitionId": "comp-gehu-2026",
      "format": "single_elimination",
      "fixturesCount": 7,
      "totalRounds": 3
    }
  }
  ```

### 8.2 Get Bracket Visualization Structure
- **Method**: `GET`
- **Route**: `/api/v1/competitions/:id/bracket`
- **Access**: Public
- **Description**: Returns tree-structured or round-grouped bracket nodes optimized for client-side rendering with progression pointers.
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "data": {
      "competitionId": "comp-gehu-2026",
      "format": "single_elimination",
      "rounds": [
        {
          "round": 1,
          "roundName": "Quarter-Final",
          "matches": [ ... ]
        },
        {
          "round": 2,
          "roundName": "Semi-Final",
          "matches": [ ... ]
        },
        {
          "round": 3,
          "roundName": "Final",
          "matches": [ ... ]
        }
      ]
    }
  }
  ```

### 8.3 Reset / Clear Fixtures
- **Method**: `POST`
- **Route**: `/api/v1/competitions/:id/reset-fixtures`
- **Access**: Protected (`ORGANIZER`, `SUPER_ADMIN`)
- **Description**: Resets fixtures back to ungenerated status if no matches have finished.

---

## 9. Match Operations & Scheduling APIs

### 9.1 List Matches
- **Method**: `GET`
- **Route**: `/api/v1/matches`
- **Access**: Public
- **Query Parameters**:
  - `competitionId` (optional): Filter by competition.
  - `status` (optional): `upcoming` | `completed`
  - `round` / `roundName` (optional): Filter by round.
  - `date` (optional): Filter matches scheduled on a specific date (e.g. today).
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "data": [
      {
        "id": "match-comp-gehu-2026-r1-m1",
        "competitionId": "comp-gehu-2026",
        "round": 1,
        "roundName": "Quarter-Final 1",
        "matchNumber": 1,
        "team1Id": "team-falcons",
        "team2Id": "team-mountain",
        "team1Score": null,
        "team2Score": null,
        "winnerId": null,
        "status": "upcoming",
        "scheduledTime": "2026-09-24 14:00",
        "venue": "Main Stadium Turf",
        "nextMatchId": "match-comp-gehu-2026-r2-m1",
        "nextMatchSlot": 1
      }
    ]
  }
  ```

### 9.2 Get Match by ID
- **Method**: `GET`
- **Route**: `/api/v1/matches/:id`
- **Access**: Public

### 9.3 Reschedule Match (Time & Venue Allocation)
- **Method**: `PATCH`
- **Route**: `/api/v1/matches/:id/schedule`
- **Access**: Protected (`ORGANIZER`, `SUPER_ADMIN`)
- **Request Body**:
  ```json
  {
    "scheduledTime": "2026-09-25 16:30",
    "venue": "East Arena Field 2"
  }
  ```
- **Business Logic**:
  - Validates venue against competition's available venues list.
  - Automatically runs conflict detection against other matches in the same venue during the match window (`scheduledTime ± matchDurationMinutes`).
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "message": "Match schedule updated.",
    "data": {
      "match": { ... },
      "conflicts": []
    }
  }
  ```

---

## 10. Scorekeeping, Result Entry & Deciders APIs

### 10.1 Submit Match Result
- **Method**: `POST`
- **Route**: `/api/v1/matches/:id/result`
- **Access**: Protected (`ORGANIZER`, `REFEREE`, `SUPER_ADMIN`)
- **Request Body**:
  ```json
  {
    "team1Score": 3,
    "team2Score": 1,
    "tieBreakerDetails": "Won in extra time (optional)",
    "tieWinnerId": null
  }
  ```
- **Backend Business Logic**:
  1. Validates numerical non-negative scores.
  2. Ensures knockout format matches cannot end in a draw without a valid `tieWinnerId`.
  3. Resolves `winnerId`:
     - Team 1 if `team1Score > team2Score`.
     - Team 2 if `team2Score > team1Score`.
     - Tie winner or `null` for Round Robin draws.
  4. Updates target match status to `completed`.
  5. **Auto-advance Winner (Single Elimination)**:
     - If `nextMatchId` and `nextMatchSlot` exist, updates the downstream match's `team1Id` or `team2Id` with `winnerId`.
  6. **Standings Recalculation**:
     - Automatically recalculates wins, draws, losses, goals for/against, goal diff, points, and rank for all teams in the competition.
  7. **Audit Trail Logging**:
     - Creates a snapshot record in `HistoryRecord` with `canUndo: true`, storing the previous state.
     - Disables `canUndo` on older results for this match.
  8. Emits a WebSocket/SSE event for real-time leaderboard and bracket updates.
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "message": "Result recorded: Falcons 3 — 1 Mountain Strikers",
    "data": {
      "match": { ... },
      "nextMatchUpdated": true,
      "auditRecordId": "hist-1730001234"
    }
  }
  ```

### 10.2 Edit Completed Match Result
- **Method**: `PUT`
- **Route**: `/api/v1/matches/:id/result`
- **Access**: Protected (`ORGANIZER`, `SUPER_ADMIN`)
- **Description**: Updates scores for an already completed match and adjusts standings and next-round team slots accordingly.

---

## 11. Audit Trail, History & One-Click Rollback APIs

### 11.1 List Activity History / Audit Logs
- **Method**: `GET`
- **Route**: `/api/v1/competitions/:id/history`
- **Access**: Public
- **Query Parameters**:
  - `limit`: Default 20
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "data": [
      {
        "id": "hist-1730001234",
        "timestamp": "14:45",
        "competitionId": "comp-gehu-2026",
        "matchId": "match-comp-gehu-2026-r1-m1",
        "actionType": "RESULT_ENTERED",
        "description": "Falcons (3) vs (1) Mountain Strikers",
        "canUndo": true
      }
    ]
  }
  ```

### 11.2 One-Click Result Undo / Rollback
- **Method**: `POST`
- **Route**: `/api/v1/history/:historyId/undo`
- **Access**: Protected (`ORGANIZER`, `SUPER_ADMIN`)
- **Safety & Integrity Checks**:
  1. Checks if `canUndo` is `true`.
  2. If the match has a `nextMatchId`, checks if the downstream match has already been completed. If completed, rejects undo with error: `"Cannot undo: Next round match has already been played."`
  3. If downstream match is not played, safely removes the advanced winner from the downstream match slot (`team1Id = null` or `team2Id = null`).
  4. Restores original `team1Score`, `team2Score`, `winnerId`, and `status = 'upcoming'`.
  5. Recalculates standings to restore points table.
  6. Creates a new audit log `RESULT_REVERTED` with `canUndo: false`.
  7. Sets original history record's `canUndo = false`.
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "message": "Result successfully undone. Leaderboard and qualification status restored.",
    "data": {
      "revertedMatchId": "match-comp-gehu-2026-r1-m1",
      "revertAuditId": "hist-revert-1730001300"
    }
  }
  ```

### 11.3 Get Latest Undoable Action
- **Method**: `GET`
- **Route**: `/api/v1/competitions/:id/history/latest-undoable`
- **Access**: Public
- **Description**: Returns the single most recent result entry eligible for undo.

---

## 12. Standings & Points Table APIs

### 12.1 Get Live Competition Standings
- **Method**: `GET`
- **Route**: `/api/v1/competitions/:id/standings`
- **Access**: Public
- **Description**: Returns all teams sorted by:
  1. Points (descending)
  2. Score Difference (descending)
  3. Score For (descending)
  4. Wins (descending)
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "data": [
      {
        "rank": 1,
        "teamId": "team-falcons",
        "name": "Falcons",
        "shortTag": "FLC",
        "played": 2,
        "wins": 2,
        "draws": 0,
        "losses": 0,
        "scoreFor": 5,
        "scoreAgainst": 1,
        "scoreDiff": 4,
        "points": 6
      }
    ]
  }
  ```

### 12.2 Force Recalculate Standings (Admin Utility)
- **Method**: `POST`
- **Route**: `/api/v1/competitions/:id/recalculate-standings`
- **Access**: Protected (`ORGANIZER`, `SUPER_ADMIN`)

---

## 13. Dashboard & Analytics APIs

### 13.1 Get Dashboard Metric Overview
- **Method**: `GET`
- **Route**: `/api/v1/dashboard/metrics`
- **Access**: Public
- **Description**: Returns aggregated numbers for the 3-stat metric row:
  - `activeTournamentsCount`
  - `registeredSquadsCount`
  - `upcomingMatchesCount`
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "data": {
      "activeTournaments": 3,
      "registeredSquads": 22,
      "upcomingMatches": 9
    }
  }
  ```

### 13.2 Get Today's Matches Feed
- **Method**: `GET`
- **Route**: `/api/v1/dashboard/today-matches`
- **Access**: Public
- **Description**: Returns today's pending upcoming fixtures with team names, scheduled time, and venue for immediate score recording.

### 13.3 Get Featured Competition
- **Method**: `GET`
- **Route**: `/api/v1/dashboard/featured-competition`
- **Access**: Public
- **Description**: Returns the active featured tournament for the dashboard hero banner.

---

## 14. Venues & Scheduling Conflict Detection APIs

### 14.1 List Venues
- **Method**: `GET`
- **Route**: `/api/v1/venues`
- **Access**: Public
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "data": [
      { "id": "ven-1", "name": "Main Stadium Turf", "type": "sports" },
      { "id": "ven-2", "name": "East Arena Field 2", "type": "sports" },
      { "id": "ven-3", "name": "LAN Arena Pod 1", "type": "esports" }
    ]
  }
  ```

### 14.2 Detect Scheduling Conflicts
- **Method**: `POST`
- **Route**: `/api/v1/competitions/:id/check-conflicts`
- **Access**: Public / Protected
- **Request Body (Optional proposed schedule change)**:
  ```json
  {
    "matchId": "match-comp-gehu-2026-r1-m2",
    "proposedTime": "2026-09-24 14:30",
    "proposedVenue": "Main Stadium Turf"
  }
  ```
- **Conflict Criteria**:
  - `VENUE_COLLISION`: Two matches assigned to the exact same venue with overlapping intervals (`time < otherEnd && end > otherTime`).
- **Response (200 OK)**:
  ```json
  {
    "success": true,
    "hasConflicts": true,
    "conflicts": [
      {
        "type": "VENUE_COLLISION",
        "match1Id": "match-comp-gehu-2026-r1-m1",
        "match2Id": "match-comp-gehu-2026-r1-m2",
        "venue": "Main Stadium Turf",
        "timeSlot": "2026-09-24 14:00 & 2026-09-24 14:30",
        "description": "Venue clash at 'Main Stadium Turf': Match #1 and Match #2 overlap."
      }
    ]
  }
  ```

---

## 15. Backup, Export, Import & Database Reset APIs

### 15.1 Export Database Backup (JSON)
- **Method**: `GET`
- **Route**: `/api/v1/settings/backup/export`
- **Access**: Protected (`ORGANIZER`, `SUPER_ADMIN`)
- **Response (200 OK)**: Full JSON payload containing:
  - `competitions`: Array<Competition>
  - `teams`: Array<Team>
  - `matches`: Array<Match>
  - `history`: Array<HistoryRecord>

### 15.2 Import Database Backup (JSON)
- **Method**: `POST`
- **Route**: `/api/v1/settings/backup/import`
- **Access**: Protected (`SUPER_ADMIN`)
- **Request Body**:
  ```json
  {
    "competitions": [ ... ],
    "teams": [ ... ],
    "matches": [ ... ],
    "history": [ ... ]
  }
  ```
- **Business Logic**: Validates JSON schema against MatchMint entity requirements; performs atomic database replacement or update.

### 15.3 Reset to Default Sample Data
- **Method**: `POST`
- **Route**: `/api/v1/settings/reset`
- **Access**: Protected (`SUPER_ADMIN`, `ORGANIZER`)
- **Description**: Clears modifications and re-seeds default competitions (`GEHU Winter Cup 2026`, `CyberClash Masters`, `Inter-College League`), squads, and bracket fixtures.

---

## 16. Real-time / WebSocket Events Specification

For cinematic live updates on spectator brackets and scoreboards, the backend exposes a WebSocket gateway (`/ws` or `/api/v1/realtime`):

### Client Subscriptions:
- `subscribe:competition` `{ "competitionId": "comp-gehu-2026" }`
- `subscribe:match` `{ "matchId": "match-comp-gehu-2026-r1-m1" }`

### Server Emitted Events:
| Event Name | Payload Description |
|---|---|
| `MATCH_SCORE_UPDATED` | `{ matchId, team1Score, team2Score, status, winnerId }` |
| `BRACKET_SLOT_ADVANCED` | `{ targetMatchId, slot: 1 \| 2, advancedTeam: { ... } }` |
| `STANDINGS_RECALCULATED` | `{ competitionId, standings: [ ... ] }` |
| `MATCH_RESULT_REVERTED` | `{ matchId, competitionId, message }` |
| `FIXTURES_GENERATED` | `{ competitionId, fixturesCount }` |

---

## 17. Standard API Error Codes & Responses

All error responses adhere to the following standard structure:

```json
{
  "success": false,
  "error": {
    "code": "VALIDATION_ERROR",
    "message": "Knockout tournament matches cannot conclude as a draw. Please select a tie-break winner.",
    "details": []
  }
}
```

### Common HTTP Status Codes:
- `200 OK`: Request succeeded.
- `201 Created`: Resource successfully created.
- `400 Bad Request`: Input validation failed / Invalid score submission.
- `401 Unauthorized`: Missing or expired JWT token.
- `403 Forbidden`: Insufficient role permissions (e.g. Spectator trying to enter scores).
- `404 Not Found`: Target match, team, or tournament not found.
- `409 Conflict`: Scheduling collision or unable to undo because downstream match completed.
- `500 Internal Server Error`: Unhandled server exception.

---

*This specification is finalized and directly mirrors the operational workflow of the MatchMint frontend application. It serves as the single source of truth for constructing the entire backend.*