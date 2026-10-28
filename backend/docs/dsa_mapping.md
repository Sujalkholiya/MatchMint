# MatchMint — DSA to Feature Mapping & Complexity Specification

This document satisfies the Problem-Based Learning (PBL) academic and engineering requirements for the data structures and algorithms powering MatchMint.

---

## 1. Summary Matrix

| Feature | Data Structure / Algorithm | Source File | Time Complexity | Auxiliary Space | Test Target |
|---|---|---|:---:|:---:|---|
| **Knockout Bracket Engine** | Binary Tree with Fold Pairing | `src/dsa/bracket_tree.c` | $O(N)$ build, $O(\log N)$ progression | $O(N)$ | `test_binary_tree_bracket` |
| **Round-Robin Scheduler** | Cyclic Fixed-Pivot Rotation (Circle Method) | `src/dsa/cyclic_schedule.c` | $O(N^2)$ | $O(N)$ | `test_cyclic_schedule` |
| **Leaderboard & Standings** | Multi-Attribute Stable Merge Sort | `src/dsa/merge_sort.c` | $O(N \log N)$ | $O(N)$ | `test_merge_sort_standings` |
| **Scheduling Conflict Engine** | 1D Interval Overlap Detector | `src/dsa/interval_conflict.c` | $O(M^2)$ pairwise, $O(1)$ overlap check | $O(1)$ | `test_interval_conflict_detector` |
| **One-Click Result Rollback** | Dynamic Singly-Linked List & Undo Stack | `src/dsa/audit_list.c` | $O(1)$ insert, $O(K)$ lookup | $O(K)$ | `test_audit_list` |

---

## 2. Detailed Algorithmic Specifications

### 2.1 Knockout Binary Tree Bracket Engine
- **File**: `backend/src/dsa/bracket_tree.c`
- **Application Problem**: Single-elimination tournaments require structured progression where winners of preliminary matches feed directly into parent matches up to the championship final.
- **Why this Data Structure**: A full binary tree naturally mirrors single-elimination tournament topologies: leaf nodes are opening round fixtures; internal nodes are subsequent rounds (Quarter-Finals, Semi-Finals); and the root node is the Final.
- **Seeding Rule**: First round fold pairing:
  - 8-team tree: `(1 vs 8)`, `(4 vs 5)`, `(2 vs 7)`, `(3 vs 6)`.
- **Complexity**:
  - Construction: $O(N)$ where $N$ is the number of participating teams.
  - Winner Advancement: $O(\log N)$ tree height traversal to set parent slot.
  - Space: $O(N)$ dynamically allocated tree nodes.

### 2.2 Cyclic Fixed-Pivot Round-Robin Scheduler
- **File**: `backend/src/dsa/cyclic_schedule.c`
- **Application Problem**: Generate a schedule where each team plays every other team once, balancing home/away slots, with no team playing itself and no duplicate matches.
- **Why this Algorithm**: The circle method keeps index 0 fixed and rotates the remaining $N-1$ elements in a cycle. It guarantees that after $N-1$ rounds, every pair has met exactly once.
- **Odd Count Handling**: An extra `BYE` dummy team is appended. The team paired with `BYE` each round is skipped.
- **Complexity**:
  - Even $N$: $N-1$ rounds, $N/2$ matches/round $\rightarrow$ $N(N-1)/2$ matches.
  - Odd $N$: $N$ rounds, $(N-1)/2$ matches/round $\rightarrow$ $N(N-1)/2$ matches.
  - Time Complexity: $O(N^2)$.
  - Space Complexity: $O(N)$ for participant rotation array.

### 2.3 Multi-Attribute Stable Merge Sort
- **File**: `backend/src/dsa/merge_sort.c`
- **Application Problem**: Tournament rankings depend on a strict priority chain:
  1. Points (descending)
  2. Score difference (`scoreFor - scoreAgainst`, descending)
  3. Total scores scored (descending)
  4. Total wins (descending)
  5. Deterministic seed fallback
- **Why this Algorithm**: Merge Sort is guaranteed $O(N \log N)$ worst-case time complexity and is **stable** (preserves equal order without unpredictable oscillations).
- **Complexity**:
  - Time: $O(N \log N)$ (best, average, and worst case).
  - Space: $O(N)$ temporary merge buffer.

### 2.4 Interval Overlap Scheduling Conflict Detector
- **File**: `backend/src/dsa/interval_conflict.c`
- **Application Problem**: Multiple tournament matches share limited campus arenas and pitches. The system must prevent two matches from being assigned to the same venue at overlapping times.
- **Interval Condition**: Two intervals $[s_1, e_1)$ and $[s_2, e_2)$ overlap if and only if:
  $$\text{start}_1 < \text{end}_2 \land \text{end}_1 > \text{start}_2$$
  Adjacent matches (e.g. 14:00–15:00 and 15:00–16:00) share an endpoint and do not clash.
- **Complexity**:
  - Single check: $O(1)$ arithmetic comparison.
  - Pairwise collision scanner: $O(M^2)$ where $M$ is the count of matches in the competition.

### 2.5 Linked List Audit Trail & Rollback Engine
- **File**: `backend/src/dsa/audit_list.c`
- **Application Problem**: MatchMint provides instant one-click result undo. The backend must maintain the historical chronological sequence of score entries and previous state snapshots.
- **Rollback Invariant**: A match result cannot be undone if the downstream match that depends on its winner has already completed.
- **Complexity**:
  - Insertion (push to head): $O(1)$.
  - Lookup by audit ID: $O(K)$ where $K$ is the history depth (capped at 20 recent records in accordance with frontend specs).
  - Memory: $O(K)$.
