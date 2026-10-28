#include "dsa.h"
#include <stdio.h>
#include <math.h>

static TreeNode *create_node(const char *competition_id, int round, int match_num, const char *round_name, const char *start_date) {
    TreeNode *node = (TreeNode *)calloc(1, sizeof(TreeNode));
    if (!node) return NULL;

    snprintf(node->match.id, sizeof(node->match.id), "match-%s-r%d-m%d", competition_id, round, match_num);
    snprintf(node->match.competition_id, sizeof(node->match.competition_id), "%s", competition_id);
    node->match.round = round;
    node->match.match_number = match_num;
    snprintf(node->match.round_name, sizeof(node->match.round_name), "%s", round_name);
    snprintf(node->match.status, sizeof(node->match.status), "upcoming");
    snprintf(node->match.scheduled_time, sizeof(node->match.scheduled_time), "%s 14:00", start_date);
    snprintf(node->match.venue, sizeof(node->match.venue), "Main Arena");

    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    return node;
}

static const DSATeam *find_team_by_seed(const DSATeam *teams, int count, int seed) {
    for (int i = 0; i < count; i++) {
        if (teams[i].seed == seed) return &teams[i];
    }
    return NULL;
}

TreeNode *tree_create_bracket(
    const char *competition_id,
    const DSATeam *teams,
    int team_count,
    const char *start_date,
    int *out_match_count
) {
    if (team_count < 2) return NULL;

    int rounds_count = (int)ceil(log2((double)team_count));
    int total_slots = 1 << rounds_count; // 2^rounds_count

    // Standard fold pairing for round 1
    int seed_pairs[8][2];
    int pair_count = total_slots / 2;

    if (total_slots == 8) {
        seed_pairs[0][0] = 1; seed_pairs[0][1] = 8;
        seed_pairs[1][0] = 4; seed_pairs[1][1] = 5;
        seed_pairs[2][0] = 2; seed_pairs[2][1] = 7;
        seed_pairs[3][0] = 3; seed_pairs[3][1] = 6;
    } else if (total_slots == 4) {
        seed_pairs[0][0] = 1; seed_pairs[0][1] = 4;
        seed_pairs[1][0] = 2; seed_pairs[1][1] = 3;
    } else {
        for (int i = 0; i < pair_count; i++) {
            seed_pairs[i][0] = i + 1;
            seed_pairs[i][1] = total_slots - i;
        }
    }

    // Build levels from Round 1 up to Final
    // Level array: pointers to nodes in each round
    TreeNode **current_level = (TreeNode **)calloc(pair_count, sizeof(TreeNode *));
    int match_counter = 1;

    // 1. Create Round 1 matches
    for (int i = 0; i < pair_count; i++) {
        char round_name[64];
        if (rounds_count == 1) {
            snprintf(round_name, sizeof(round_name), "Final");
        } else if (rounds_count == 2) {
            snprintf(round_name, sizeof(round_name), "Semi Final %02d", i + 1);
        } else {
            snprintf(round_name, sizeof(round_name), "Quarter Final %02d", i + 1);
        }

        current_level[i] = create_node(competition_id, 1, match_counter++, round_name, start_date);

        const DSATeam *t1 = find_team_by_seed(teams, team_count, seed_pairs[i][0]);
        const DSATeam *t2 = find_team_by_seed(teams, team_count, seed_pairs[i][1]);

        if (t1) snprintf(current_level[i]->match.team1_id, sizeof(current_level[i]->match.team1_id), "%s", t1->id);
        if (t2) snprintf(current_level[i]->match.team2_id, sizeof(current_level[i]->match.team2_id), "%s", t2->id);
    }

    int current_level_count = pair_count;

    // 2. Build subsequent rounds by pairing nodes into parent nodes
    for (int r = 2; r <= rounds_count; r++) {
        int next_level_count = current_level_count / 2;
        TreeNode **next_level = (TreeNode **)calloc(next_level_count, sizeof(TreeNode *));

        for (int i = 0; i < next_level_count; i++) {
            char round_name[64];
            if (r == rounds_count) {
                snprintf(round_name, sizeof(round_name), "Final");
            } else if (r == rounds_count - 1) {
                snprintf(round_name, sizeof(round_name), "Semi Final %02d", i + 1);
            } else {
                snprintf(round_name, sizeof(round_name), "Round %d Match %d", r, i + 1);
            }

            TreeNode *parent = create_node(competition_id, r, match_counter++, round_name, start_date);
            TreeNode *left_child = current_level[i * 2];
            TreeNode *right_child = current_level[i * 2 + 1];

            parent->left = left_child;
            parent->right = right_child;
            left_child->parent = parent;
            right_child->parent = parent;

            snprintf(left_child->match.next_match_id, sizeof(left_child->match.next_match_id), "%s", parent->match.id);
            left_child->match.next_match_slot = 1;

            snprintf(right_child->match.next_match_id, sizeof(right_child->match.next_match_id), "%s", parent->match.id);
            right_child->match.next_match_slot = 2;

            next_level[i] = parent;
        }

        free(current_level);
        current_level = next_level;
        current_level_count = next_level_count;
    }

    TreeNode *root = current_level[0];
    free(current_level);

    if (out_match_count) {
        *out_match_count = match_counter - 1;
    }

    return root;
}

static TreeNode *find_node_by_id(TreeNode *root, const char *match_id) {
    if (!root || !match_id) return NULL;
    if (strcmp(root->match.id, match_id) == 0) return root;

    TreeNode *found = find_node_by_id(root->left, match_id);
    if (found) return found;
    return find_node_by_id(root->right, match_id);
}

int tree_advance_winner(TreeNode *root, const char *match_id, const char *winner_id) {
    if (!root || !match_id || !winner_id) return -1;

    TreeNode *node = find_node_by_id(root, match_id);
    if (!node) return -1;

    snprintf(node->match.winner_id, sizeof(node->match.winner_id), "%s", winner_id);
    snprintf(node->match.status, sizeof(node->match.status), "completed");

    // Advance to parent if exists
    if (node->parent) {
        if (node->parent->left == node) {
            snprintf(node->parent->match.team1_id, sizeof(node->parent->match.team1_id), "%s", winner_id);
        } else if (node->parent->right == node) {
            snprintf(node->parent->match.team2_id, sizeof(node->parent->match.team2_id), "%s", winner_id);
        }
    }

    return 0;
}

int tree_revert_advancement(TreeNode *root, const char *match_id, const char *winner_id) {
    if (!root || !match_id || !winner_id) return -1;

    TreeNode *node = find_node_by_id(root, match_id);
    if (!node) return -1;

    // Check downstream completion
    if (node->parent && strcmp(node->parent->match.status, "completed") == 0) {
        return -2; // Cannot revert: downstream match already completed!
    }

    // Clear parent slot
    if (node->parent) {
        if (node->parent->left == node && strcmp(node->parent->match.team1_id, winner_id) == 0) {
            node->parent->match.team1_id[0] = '\0';
        } else if (node->parent->right == node && strcmp(node->parent->match.team2_id, winner_id) == 0) {
            node->parent->match.team2_id[0] = '\0';
        }
    }

    node->match.winner_id[0] = '\0';
    snprintf(node->match.status, sizeof(node->match.status), "upcoming");
    return 0;
}

static void collect_matches(TreeNode *node, DSAMatch *out_matches, int *idx, int max_matches) {
    if (!node || *idx >= max_matches) return;
    collect_matches(node->left, out_matches, idx, max_matches);
    collect_matches(node->right, out_matches, idx, max_matches);

    if (*idx < max_matches) {
        out_matches[*idx] = node->match;
        (*idx)++;
    }
}

static int compare_matches(const void *a, const void *b) {
    const DSAMatch *m1 = (const DSAMatch *)a;
    const DSAMatch *m2 = (const DSAMatch *)b;
    if (m1->round != m2->round) return m1->round - m2->round;
    return m1->match_number - m2->match_number;
}

int tree_flatten_matches(TreeNode *root, DSAMatch *out_matches, int max_matches) {
    if (!root || !out_matches || max_matches <= 0) return 0;
    int count = 0;
    collect_matches(root, out_matches, &count, max_matches);
    qsort(out_matches, count, sizeof(DSAMatch), compare_matches);
    return count;
}

void tree_free(TreeNode *root) {
    if (!root) return;
    tree_free(root->left);
    tree_free(root->right);
    free(root);
}
