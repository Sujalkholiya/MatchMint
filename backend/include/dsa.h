#ifndef MATCHMINT_DSA_H
#define MATCHMINT_DSA_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DSA_STR_LEN 64
#define DSA_DESC_LEN 256

/* --- Data Models for DSA Operations --- */

typedef struct {
    char id[DSA_STR_LEN];
    char name[DSA_STR_LEN];
    char short_tag[8];
    int seed;
    char captain[DSA_STR_LEN];
    int roster_count;
    int wins;
    int draws;
    int losses;
    int points;
    int score_for;
    int score_against;
    int score_diff;
    int rank;
    char status[16];
} DSATeam;

typedef struct {
    char id[DSA_STR_LEN];
    char competition_id[DSA_STR_LEN];
    int round;
    char round_name[DSA_STR_LEN];
    int match_number;
    char team1_id[DSA_STR_LEN];
    char team2_id[DSA_STR_LEN];
    int team1_score;
    int team2_score;
    bool has_team1_score;
    bool has_team2_score;
    char winner_id[DSA_STR_LEN];
    char status[16]; // "upcoming" or "completed"
    char scheduled_time[DSA_STR_LEN];
    char venue[DSA_STR_LEN];
    char next_match_id[DSA_STR_LEN];
    int next_match_slot; // 1 or 2
    char tie_breaker_details[DSA_DESC_LEN];
} DSAMatch;

typedef struct {
    char match_id[DSA_STR_LEN];
    char venue[DSA_STR_LEN];
    int64_t start_time_epoch;
    int duration_minutes;
} DSAInterval;

typedef struct {
    char type[32]; // "VENUE_COLLISION"
    char match1_id[DSA_STR_LEN];
    char match2_id[DSA_STR_LEN];
    char venue[DSA_STR_LEN];
    char time_slot[DSA_DESC_LEN];
    char description[DSA_DESC_LEN];
} DSAConflict;

typedef struct DSAHistoryNode {
    char id[DSA_STR_LEN];
    char competition_id[DSA_STR_LEN];
    char match_id[DSA_STR_LEN];
    char action_type[32]; // "RESULT_ENTERED", "RESULT_REVERTED", "FIXTURES_GENERATED"
    char description[DSA_DESC_LEN];
    char timestamp[DSA_STR_LEN];
    bool can_undo;
    int team1_score_prev;
    int team2_score_prev;
    bool has_prev_scores;
    char winner_id_prev[DSA_STR_LEN];
    char status_prev[16];
    struct DSAHistoryNode *next;
} DSAHistoryNode;

/* ========================================================================= */
/* 1. BINARY TREE: KNOCKOUT BRACKET ENGINE                                   */
/* ========================================================================= */

typedef struct TreeNode {
    DSAMatch match;
    struct TreeNode *left;   // Source for team1 (slot 1)
    struct TreeNode *right;  // Source for team2 (slot 2)
    struct TreeNode *parent; // Target parent where winner advances
} TreeNode;

/**
 * Builds binary tournament tree for single-elimination tournament.
 * Uses fold pairing for Round 1 (1v8, 4v5, 2v7, 3v6 for 8 teams).
 *
 * @param competition_id Unique tournament ID
 * @param teams Array of seeded participating teams
 * @param team_count Number of teams (4, 8, 16)
 * @param start_date Base tournament start date
 * @param out_match_count Returns total generated match nodes
 * @return Root of the binary tree (championship final)
 */
TreeNode *tree_create_bracket(
    const char *competition_id,
    const DSATeam *teams,
    int team_count,
    const char *start_date,
    int *out_match_count
);

/**
 * Frees all allocated tree nodes.
 */
void tree_free(TreeNode *root);

/**
 * Advances winner of match_id to parent node slot (1 or 2).
 *
 * @return 0 on success, -1 if match not found, -2 if parent full
 */
int tree_advance_winner(TreeNode *root, const char *match_id, const char *winner_id);

/**
 * Reverts advancement: clears winner from parent node slot.
 */
int tree_revert_advancement(TreeNode *root, const char *match_id, const char *winner_id);

/**
 * Flattens all binary tree matches into an array ordered by round and match_number.
 */
int tree_flatten_matches(TreeNode *root, DSAMatch *out_matches, int max_matches);

/* ========================================================================= */
/* 2. CYCLIC FIXED-PIVOT ROTATION: ROUND-ROBIN SCHEDULER                     */
/* ========================================================================= */

/**
 * Generates single round-robin tournament fixtures using cyclic rotation.
 * Index 0 remains the fixed pivot; remaining elements rotate clockwise.
 * Handles odd team counts with an automatic BYE.
 *
 * @return Total matches generated, or -1 on error.
 */
int cyclic_generate_fixtures(
    const char *competition_id,
    const DSATeam *teams,
    int team_count,
    const char *start_date,
    DSAMatch *out_matches,
    int max_matches
);

/* ========================================================================= */
/* 3. MERGE SORT: MULTI-ATTRIBUTE TOURNAMENT STANDINGS                       */
/* ========================================================================= */

/**
 * Sorts array of teams using stable O(n log n) Merge Sort by:
 * 1. Points (descending)
 * 2. Score Difference (descending)
 * 3. Score For (descending)
 * 4. Wins (descending)
 * Sets team->rank to (index + 1).
 */
void merge_sort_standings(DSATeam *teams, int count);

/* ========================================================================= */
/* 4. INTERVAL OVERLAP: SCHEDULING CONFLICT DETECTION                        */
/* ========================================================================= */

/**
 * Checks if two time intervals overlap:
 * start1 < end2 && end1 > start2 (strictly overlapping, adjacent is allowed)
 */
bool intervals_overlap(int64_t start1, int64_t end1, int64_t start2, int64_t end2);

/**
 * Scans intervals and detects venue clashes.
 *
 * @return Total conflicts detected.
 */
int detect_scheduling_conflicts(
    const DSAInterval *intervals,
    int count,
    DSAConflict *out_conflicts,
    int max_conflicts
);

/* ========================================================================= */
/* 5. LINKED LIST: AUDIT TRAIL AND ROLLBACK ENGINE                           */
/* ========================================================================= */

/**
 * Creates empty history list.
 */
DSAHistoryNode *audit_list_create(void);

/**
 * Prepend history node (LIFO recent first).
 */
void audit_list_push(DSAHistoryNode **head, const DSAHistoryNode *entry);

/**
 * Find history record by ID.
 */
DSAHistoryNode *audit_list_find(DSAHistoryNode *head, const char *history_id);

/**
 * Determines if result can be undone safely.
 * Returns true if can_undo is true and downstream match has not completed.
 */
bool audit_list_can_undo(const DSAHistoryNode *record, bool is_downstream_completed);

/**
 * Frees linked list.
 */
void audit_list_free(DSAHistoryNode *head);

#endif /* MATCHMINT_DSA_H */
