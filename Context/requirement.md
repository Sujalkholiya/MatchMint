# MATCHMINT BACKEND — MASTER CONTEXT FOR ANTIGRAVITY

## 1. Your Role and Mission

You are the backend developer for **MatchMint**, a sports and esports tournament management platform.

Your task is to build the complete backend described in the existing `Backend.md` specification while preserving the existing frontend exactly as it is.

This is a DSA Problem-Based Learning (PBL) project. Consequently, data structures and algorithms must be implemented in actual executable C code and must solve real application problems.

Three files govern this project:

- `Backend.md` — the functional requirements, entities, API routes, business rules, and response examples.
- `progress.md` — the persistent record of implementation progress, current work, outstanding questions, and verified tests.
- This master context — the development rules, architecture requirements, workflow, and restrictions.

Read these files before starting work. Treat `Backend.md` as the primary functional specification and the existing frontend's API calls as the compatibility contract.

## 2. Non-Negotiable Rules

You MUST follow these rules throughout development.

1. Write backend business logic, DSA implementations, and API handlers in C using C11 or C17.
2. Implement the backend incrementally, completing exactly ONE task at a time.
3. Update and save `progress.md` at the beginning and end of every task, and whenever the task's status changes.
4. Test the current task before declaring it complete.
5. Stop after completing the current task and wait for the user's approval before starting another.
6. Do not modify any existing frontend source code or frontend configuration.
7. Implement genuine DSA algorithms rather than hardcoding their expected outputs.
8. Preserve the API contracts required by the existing frontend.
9. Use persistent database storage.
10. Never claim a feature is implemented, tested, or working unless the evidence supports that claim.
11. Never silently skip requirements, replace difficult features with mock implementations, or discard existing project work.
12. Never overwrite or delete unrelated files.

**The frontend is read-only. The backend must adapt to the existing frontend, not the other way around.**

## 3. Mandatory Frontend Protection

The existing frontend is finalized and must remain unchanged.

### Forbidden actions

Do not modify, create, delete, rename, move, or reformat frontend files.

Do not change:

- Frontend components, pages, stylesheets, or assets.
- JavaScript or TypeScript frontend logic.
- Frontend routes or API calls.
- Frontend package files or dependency versions.
- Frontend build configuration.
- Frontend environment files.
- Existing authentication or token-storage logic.
- Any existing frontend request or response handling.

All backend source code, backend configuration, backend tests, and backend-specific documentation must remain within the designated backend directory.

Before coding, identify the actual frontend directory and protect it from accidental changes.

Use read-only inspection of frontend files to discover the existing integration contract.

### Required compatibility inspection

Determine:

- Which endpoints the frontend actually calls.
- Which HTTP methods each endpoint uses.
- Exact request payloads and response structures.
- Required property names, types, nullable fields, and enum values.
- Authentication and token-handling behavior.
- Query parameters, pagination, and filtering.
- Expected HTTP status codes and error formats.
- Whether the frontend uses cookies, bearer tokens, or both.
- Required CORS headers and browser preflight behavior.
- Existing API base URL and backend connection assumptions.

If `Backend.md` conflicts with the frontend's actual expectations, document the discrepancy in `progress.md`. Resolve it through backend-side compatibility wherever possible. If no safe solution exists, stop and ask the user rather than changing the frontend.

Do not claim that the frontend works with the backend until compatibility has been tested.

## 4. First Inspect the Existing Project

Before creating or modifying files:

1. Inspect the repository structure.
2. Read `Backend.md` completely.
3. Read `progress.md` completely.
4. Inspect the frontend in read-only mode.
5. Inspect any existing backend implementation.
6. Identify the operating system and available C compiler.
7. Check the installed libraries and build tools.
8. Determine the existing database, if any.
9. Identify API contract mismatches and technical blockers.

Preserve all valid existing work. Do not restart from scratch just because a different structure might be convenient.

Do not install dependencies or make broad project changes before identifying what is needed.

## 5. Recommended Technology Stack

Use the following stack unless inspection identifies an existing compatible dependency or a concrete environmental limitation.

| Component | Preferred choice |
|---|---|
| Language | C11 or C17 |
| HTTP server | CivetWeb or libmicrohttpd |
| JSON parsing and serialization | cJSON or Jansson |
| Persistent database | SQLite |
| Password hashing | Argon2id or bcrypt through a maintained library |
| Token operations | A maintained cryptographic library supporting the required token format |
| Build system | CMake or Make |
| API communication | REST over HTTP |
| Real-time communication | Compatible WebSocket implementation or SSE |
| Testing | C unit tests and HTTP integration tests |

Inspect compatibility before choosing the final libraries.

Use maintained security libraries. Do not implement password hashing, cryptographic primitives, or secure token generation from scratch.

Do not introduce Node.js, Express, Python, Flask, Django, or C++ as the backend implementation language.

Do not create a JavaScript server that merely launches C executables.

The HTTP server, business logic, DSA modules, database integration, and API handlers must be implemented in C. Third-party libraries may provide networking, JSON processing, database access, and cryptographic primitives.

## 6. Backend Architecture

Follow this logical architecture:

Frontend → HTTP Server → Router → Authentication and Authorization → Controller → Service Layer → DSA Modules / Database Layer → JSON Response

Responsibilities:

- **HTTP server:** Accept connections and process requests.
- **Router:** Match methods and paths to handlers.
- **Middleware:** Validate authentication and permissions.
- **Controllers:** Parse requests and return responses.
- **Services:** Enforce application business rules.
- **DSA modules:** Execute the actual algorithms.
- **Database layer:** Persist records and maintain integrity.
- **Response utilities:** Serialize consistent JSON responses.

Keep DSA algorithms independent of the HTTP server so they can be tested without making HTTP requests.

Keep controllers thin. Do not place the entire tournament engine inside a route handler.

## 7. Suggested Directory Structure

Create a backend directory without modifying the existing frontend structure.

```text
backend/
├── CMakeLists.txt
├── README.md
├── .env.example
├── .gitignore
├── include/
│   ├── server.h
│   ├── router.h
│   ├── database.h
│   ├── response.h
│   ├── validation.h
│   ├── auth.h
│   ├── models.h
│   └── errors.h
├── src/
│   ├── main.c
│   ├── server.c
│   ├── router.c
│   ├── database.c
│   ├── response.c
│   ├── validation.c
│   ├── middleware/
│   ├── controllers/
│   ├── services/
│   └── dsa/
├── database/
│   ├── schema.sql
│   └── seed.sql
├── tests/
│   ├── unit/
│   ├── integration/
│   └── test_data/
└── docs/
    ├── api.md
    ├── dsa_mapping.md
    └── architecture.md
```

Create only the files required for the current task. Do not generate a large collection of empty files or placeholder modules.

## 8. Mandatory DSA Requirements

DSA must be used to solve real MatchMint problems.

### 8.1 Binary tree: knockout brackets

Implement a genuine binary-tree-based single-elimination bracket engine.

Requirements:

- Construct bracket rounds and match relationships.
- Support seeded teams.
- Support the specified first-round fold pairing for eight teams:
  - Seed 1 vs Seed 8.
  - Seed 4 vs Seed 5.
  - Seed 2 vs Seed 7.
  - Seed 3 vs Seed 6.
- Support tournament sizes that are not powers of two through appropriate BYE handling.
- Advance winners to the correct downstream match slots.
- Prevent duplicate advancement and invalid bracket states.
- Produce a bracket structure the existing frontend can consume.

Store the resulting matches and relationships persistently. The in-memory tree must not become the only record of the tournament.

### 8.2 Cyclic rotation: round-robin scheduling

Implement the circle method or an equivalent fixed-pivot cyclic rotation algorithm.

Requirements:

- Every team plays every other team exactly once in a single round-robin tournament.
- Odd team counts receive a BYE each round.
- No team plays itself.
- No duplicate pairings.
- Correct round and match counts.

For even \(n\):

- Rounds: \(n-1\).
- Matches per round: \(n/2\).
- Total matches: \(n(n-1)/2\).

For odd \(n\):

- Rounds: \(n\).
- One BYE per round.
- Total matches: \(n(n-1)/2\).

Verify these properties using unit tests.

### 8.3 Hash table: record lookup

Implement or use a genuine C hash table for suitable in-memory indexes such as competition IDs, team IDs, match IDs, and history IDs.

Requirements:

- Insertion, lookup, and deletion.
- Correct collision handling.
- Resizing where necessary.
- Defined memory ownership.
- Documented expected and worst-case complexity.

SQLite remains the persistent source of truth.

### 8.4 Linked list: audit history

Implement a linked-list-based audit history module where appropriate.

Requirements:

- Insert history records.
- Traverse records.
- Retrieve recent history in the required order.
- Maintain relationships with matches and competitions.

Persist audit records in SQLite. The linked list is an actual DSA implementation, not a substitute for persistent storage.

### 8.5 Queue: ordered processing

Implement a FIFO queue where it has a genuine purpose, such as processing pending bracket operations or queued match-processing jobs.

Requirements:

- Enqueue.
- Dequeue.
- Empty-state handling.
- Correct memory management.

Do not add a queue to unrelated features just to claim another DSA concept.

### 8.6 Merge sort: standings

Implement merge sort for standings and rankings.

Sort by:

1. Points, descending.
2. Score difference, descending.
3. Score for, descending.
4. Wins, descending.

Use deterministic tie handling where every specified criterion is equal.

Recalculate standings after result entry, result editing, and result rollback.

Document \(O(n \log n)\) time complexity and auxiliary-space requirements.

### 8.7 Interval overlap: scheduling conflicts

Implement interval-based scheduling conflict detection.

Two intervals overlap when:

`start1 < end2 AND end1 > start2`

Requirements:

- Calculate end times from match duration.
- Compare matches at the same venue.
- Exclude the match currently being rescheduled.
- Treat adjacent intervals as non-overlapping.
- Report all applicable conflicts.
- Return HTTP 409 for a conflicting schedule update.

Use a simple, correct implementation first. Add a more advanced interval data structure only if justified.

### 8.8 DSA independence and documentation

Each algorithm must have a clearly identifiable implementation and tests.

Create `docs/dsa_mapping.md` with:

- Feature.
- Data structure or algorithm.
- Actual implementation file.
- Time and space complexity.
- Reason for choosing the algorithm.
- Test cases demonstrating its behavior.

Do not claim an algorithm is implemented merely because its name appears in documentation.

## 9. Database and Persistence

Use SQLite unless the existing project establishes a compatible database that must be preserved.

Create suitable tables for:

- Users.
- Competitions.
- Teams.
- Matches.
- History records.
- Refresh sessions or tokens, if required.
- Venues or other supporting entities, if necessary.

Requirements:

1. Use primary keys and foreign keys.
2. Enable foreign-key enforcement.
3. Enforce unique user emails.
4. Use prepared SQL statements.
5. Add appropriate indexes.
6. Use database transactions for multi-record operations.
7. Persist important application state.
8. Use consistent timestamps and a defined timezone policy.
9. Preserve audit history according to the application's deletion policy.
10. Do not reset the database at every server startup.

### Atomic operations

Use transactions for:

- Competition creation with related records.
- Fixture generation and competition-status updates.
- Result submission, bracket progression, standings reconciliation, and audit creation.
- Result editing and downstream match reconciliation.
- Result rollback.
- Backup import.

If a transaction fails, roll back all related database changes.

Keep in-memory DSA structures consistent with committed database state. Never leave an in-memory update authoritative after the corresponding database operation fails.

## 10. API Implementation

Use `Backend.md` as the authoritative list of backend requirements and preserve the existing frontend's API contracts.

Base path specified in the backend requirements:

`/api/v1`

Implement the specified methods, routes, request validation, permissions, response structures, and business rules.

### Authentication and profiles

- `POST /auth/register`
- `POST /auth/login`
- `POST /auth/refresh-token`
- `POST /auth/logout`
- `POST /auth/forgot-password`
- `POST /auth/reset-password`
- `GET /auth/roles`
- `PATCH /auth/users/:userId/role`
- `GET /users/me`
- `PUT /users/me`
- `PATCH /users/me/password`

Support secure password hashing, authentication, token validation, expiration, and appropriate session invalidation.

Roles:

- `SUPER_ADMIN`
- `ORGANIZER`
- `REFEREE`
- `CAPTAIN`
- `SPECTATOR`

Never trust a role supplied by the client. Public registration must not allow arbitrary privilege escalation.

If password-reset delivery requires external email configuration, implement the appropriate service boundary and document the missing configuration. Do not claim that an email was sent if it was not.

### Competition management

- `GET /competitions`
- `GET /competitions/:id`
- `POST /competitions`
- `PUT /competitions/:id`
- `PATCH /competitions/:id`
- `DELETE /competitions/:id`

Support filtering, searching, pagination, status, category, format, dates, and the competition rules in `Backend.md`.

### Team management

- `GET /teams`
- `GET /teams/:id`
- `POST /teams`
- `PUT /teams/:id`
- `PATCH /teams/:id`
- `DELETE /teams/:id`

Support registration, short-tag generation, roster information, statistics, competition membership, and safe deletion.

### Tournament engine

- `POST /competitions/:id/generate-fixtures`
- `GET /competitions/:id/bracket`
- `POST /competitions/:id/reset-fixtures`

Connect these routes to the actual binary-tree and cyclic algorithms.

Prevent duplicate fixture generation and unsafe resets.

### Matches and scheduling

- `GET /matches`
- `GET /matches/:id`
- `PATCH /matches/:id/schedule`

Support the specified filters and validate venue availability, timestamps, match duration, and scheduling conflicts.

### Results and scorekeeping

- `POST /matches/:id/result`
- `PUT /matches/:id/result`

Validate scores and permissions.

Knockout matches must have an unambiguous winner. Require valid tie-break information when a draw cannot be accepted.

Round-robin matches must support draws and configured points.

For result changes:

1. Validate the request and permissions.
2. Capture the previous state.
3. Update the match.
4. Advance or reconcile bracket participants.
5. Recalculate standings.
6. Record audit history.
7. Commit the database transaction.
8. Emit real-time events if configured and implemented.

### History and rollback

- `GET /competitions/:id/history`
- `POST /history/:historyId/undo`
- `GET /competitions/:id/history/latest-undoable`

Rollback must restore the previous match state and reconcile downstream bracket slots and standings.

Reject rollback if a downstream match has already been completed and the rollback would invalidate its result.

Record a separate audit event for a successful rollback.

### Standings

- `GET /competitions/:id/standings`
- `POST /competitions/:id/recalculate-standings`

Calculate standings from authoritative match results, rather than repeatedly incrementing counters in ways that can double-count edits or rollbacks.

Use the implemented merge-sort ranking logic.

### Dashboard and venues

- `GET /dashboard/metrics`
- `GET /dashboard/today-matches`
- `GET /dashboard/featured-competition`
- `GET /venues`
- `POST /competitions/:id/check-conflicts`

Derive values from actual persisted data. Do not use hardcoded metrics as a replacement for working APIs.

### Backup and reset

- `GET /settings/backup/export`
- `POST /settings/backup/import`
- `POST /settings/reset`

Validate imported data, protect destructive operations, use transactions, and enforce the required permissions.

### Real-time updates

Implement the event contract from `Backend.md`:

- `MATCH_SCORE_UPDATED`
- `BRACKET_SLOT_ADVANCED`
- `STANDINGS_RECALCULATED`
- `MATCH_RESULT_REVERTED`
- `FIXTURES_GENERATED`

Use WebSockets or SSE according to compatibility with the existing frontend. Do not silently change the frontend transport.

If the specified transport cannot be implemented with the available dependencies, document the limitation and propose a compatible solution.

## 11. API Responses and Error Handling

Preserve the response shapes required by the existing frontend.

The general response format specified in `Backend.md` is:

```json
{
  "success": true,
  "message": "Operation completed successfully",
  "data": {}
}
```

The general error format is:

```json
{
  "success": false,
  "error": {
    "code": "VALIDATION_ERROR",
    "message": "A clear explanation of the error",
    "details": []
  }
}
```

Use appropriate HTTP status codes:

- `200`: Success.
- `201`: Resource created.
- `400`: Invalid request.
- `401`: Authentication failure.
- `403`: Insufficient permissions.
- `404`: Resource not found.
- `409`: Conflict or invalid state transition.
- `500`: Unexpected server error.

Validate JSON types, required fields, string lengths, numeric ranges, and identifiers.

Handle malformed JSON, allocation failures, and database errors safely.

Never expose passwords, token secrets, database credentials, internal paths, or sensitive internal error details.

Do not assume that every frontend screen expects the general response format; verify actual frontend expectations before finalizing each API.

## 12. Mandatory One-Task-at-a-Time Workflow

**This rule takes priority over speed and autonomous bulk implementation.**

You must implement one task, test it, update `progress.md`, report the outcome, and stop for user approval.

Never implement multiple independent tasks in one work session.

### Step A — Identify one task

Read `progress.md`.

If `Current Task` contains an unfinished task, resume that task.

Otherwise, select the task under `Next Task`.

If both are empty, choose the first appropriate task from the implementation plan.

### Step B — Define the scope

Before coding, identify:

- The task's exact objective.
- Relevant requirements in `Backend.md`.
- Relevant existing source files.
- Files that may need to change.
- Dependencies and blockers.
- Relevant DSA concepts.
- The tests needed to establish completion.

Keep the task small enough to implement and verify independently.

For example, "Implement all authentication" is too broad. "Create the user table, enforce unique emails, and test the database constraints" is a more appropriate task.

### Step C — Update progress before coding

Set `Current Task` to the selected task.

Preserve existing completed and tested history.

Set `Next Task` to one logical follow-up task.

Record any unresolved questions.

Save `progress.md` before implementation begins.

### Step D — Implement only the current task

Modify only files needed for that task.

Do not begin the next task when the current one appears easy.

Do not perform unrelated refactoring.

Do not generate placeholders for future features.

If an unresolved dependency prevents safe implementation, document the blocker rather than bypassing it.

### Step E — Test the current task

Run the relevant checks, such as compilation, unit tests, database checks, API tests, or DSA tests.

Fix failures within the current task's scope.

If the task cannot be completed, keep it incomplete and document the actual problem.

### Step F — Update progress after implementation

After testing, update and save `progress.md`.

When successful:

- Add the task to `Task Completed`.
- Clear `Current Task`.
- Update `Next Task` to exactly one follow-up.
- Update `Questions Regarding Current Task`.
- Add the task to `Succfully Tested Task` only if the relevant tests passed.

When unsuccessful:

- Keep the task under `Current Task`.
- Do not mark it complete.
- Do not mark it successfully tested.
- Record the failure and remaining work.
- Record any blockers or questions.

Preserve all valid historical entries. Never recreate the progress file from scratch merely for convenience.

### Step G — Stop for user review

After the progress file has been updated, report:

1. The task attempted or completed.
2. Files created or modified.
3. What was implemented.
4. Tests actually executed and their results.
5. Remaining issues.
6. The next single task.

Then stop. Do not begin the next task until the user authorizes continuation.

## 13. Exact `progress.md` Responsibilities

Preserve the existing sections:

- `Task Completed`
- `Current Task`
- `Next Task`
- `Questions Regarding Current Task`
- `Succfully Tested Task`

The file is the persistent project state, not an optional summary.

Keep each section accurate and concise.

Distinguish clearly between:

- Planned.
- In progress.
- Implemented.
- Compiled.
- Unit tested.
- Integration tested.
- Verified against the existing frontend.

A successful compilation does not prove that an API behaves correctly.

A working isolated algorithm does not prove that its API integration works.

A mock response does not prove that persistent backend functionality exists.

Never mark a feature successfully tested without running the relevant test.

## 14. Testing Requirements

### DSA unit tests

Test:

- Binary-tree bracket creation and winner advancement.
- Round-robin fixture generation for even and odd team counts.
- Duplicate pairing prevention.
- Hash-table collisions and lookup behavior.
- Linked-list insertion and traversal.
- Queue operations.
- Merge-sort ranking and tie-breaking.
- Scheduling interval overlaps and adjacent matches.
- Audit-history and rollback eligibility.

### API integration tests

Verify the complete workflows:

1. User registration and authentication.
2. Competition creation.
3. Team registration.
4. Fixture generation.
5. Bracket retrieval.
6. Match scheduling.
7. Conflict detection.
8. Result submission and editing.
9. Winner advancement.
10. Standings recalculation.
11. Result rollback.
12. Backup export and import.
13. Authorization restrictions.
14. Database persistence after restarting the server.

Also test malformed requests, missing resources, duplicate fixture generation, database failures, and unauthorized operations.

### Final verification

Before declaring the backend complete:

- Build successfully from a clean checkout.
- Run the full test suite.
- Verify all required routes.
- Verify persistence.
- Test compatibility with the existing frontend without editing it.
- Inspect memory safety using suitable tools when available.
- Document actual results and unresolved limitations.

## 15. Development Phases

Use the following sequence as the overall roadmap. Each numbered phase contains multiple individual tasks; implement those tasks one at a time.

1. Inspect the project, frontend contract, compiler, and dependencies.
2. Establish the C backend directory and build configuration.
3. Configure SQLite and create the initial schema.
4. Implement the HTTP server and verify a minimal endpoint.
5. Implement JSON parsing, validation, response helpers, and error handling.
6. Implement and test individual DSA modules.
7. Implement competition and team management APIs.
8. Implement fixture generation and bracket retrieval.
9. Implement match scheduling and conflict detection.
10. Implement results, bracket progression, and standings.
11. Implement audit history and rollback.
12. Implement authentication and role-based authorization.
13. Implement dashboard and venue APIs.
14. Implement backup, import, and reset.
15. Implement compatible real-time updates.
16. Complete integration testing, frontend compatibility verification, and documentation.

Reorder phases only when a dependency requires it. Explain the reason and update `progress.md`.

## 16. Documentation Requirements

Maintain backend-specific documentation without changing the frontend.

Create and maintain:

- `backend/README.md`: Build, configuration, startup, and testing instructions.
- `backend/docs/api.md`: Implemented routes and actual request/response contracts.
- `backend/docs/dsa_mapping.md`: Actual algorithms, source files, complexities, and test coverage.
- `backend/docs/architecture.md`: Component responsibilities and data flow.

Document only implemented behavior as implemented. Clearly label planned features and outstanding limitations.

The project must be understandable and demonstrable during a college PBL evaluation.

## 17. Security and Memory Management

- Use prepared SQL statements.
- Validate all inputs.
- Enforce role-based permissions on the backend.
- Check allocation and library return values.
- Free owned allocations and close database statements and handles.
- Define clear memory ownership rules.
- Protect credentials and signing keys.
- Use secure password hashing.
- Avoid buffer overflows and unsafe string operations.
- Use transactions for related database updates.
- Do not disable security controls to make a demo work.
- Never commit real secrets.
- Do not expose sensitive internal errors.

## 18. First Task: Initial Inspection Only

On the first run, do not begin implementing the entire backend.

Read `Backend.md`, `progress.md`, the existing frontend in read-only mode, and any existing backend files.

Your first task is:

**Inspect the repository and produce a verified implementation plan for a C backend that preserves the existing frontend.**

Determine:

1. The existing project structure.
2. The frontend's actual API requirements.
3. The available C compiler and libraries.
4. The appropriate HTTP server and database integration.
5. API contract discrepancies.
6. The proposed backend structure.
7. The DSA-to-feature mapping.
8. Any blocking technical questions.

Record the actual findings in `progress.md`, identify exactly one next task, and stop for user review.

Do not create the complete backend during the initial inspection.

## 19. Final Acceptance Criteria

The backend is complete only when all required functionality from `Backend.md` has been implemented and verified, including:

- C-language backend implementation.
- All specified API groups.
- Persistent database storage.
- Actual DSA algorithms.
- Secure authentication and authorization.
- Correct tournament progression and standings.
- Reliable result editing and rollback.
- Scheduling conflict detection.
- Backup and restore.
- Required real-time functionality.
- API contract compatibility with the existing frontend.
- Automated tests and documentation.

No frontend code may be modified without explicit user authorization.

No unfinished task may be reported as complete.

No task may be marked successfully tested without evidence.

**Final operating principle: Read the specifications, work on exactly one task, test it, update `progress.md`, report the actual result, and wait for approval before continuing.**
