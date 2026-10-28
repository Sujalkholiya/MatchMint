#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "dsa.h"

static void test_binary_tree_bracket(void) {
    printf("[TEST] 1. Knockout Binary Tree Bracket Engine...\n");

    DSATeam teams[8];
    for (int i = 0; i < 8; i++) {
        snprintf(teams[i].id, sizeof(teams[i].id), "team-%d", i + 1);
        snprintf(teams[i].name, sizeof(teams[i].name), "Team %d", i + 1);
        teams[i].seed = i + 1;
    }

    int match_count = 0;
    TreeNode *root = tree_create_bracket("comp-test", teams, 8, "2026-10-30", &match_count);
    assert(root != NULL);
    assert(match_count == 7); // 4 QFs + 2 SFs + 1 Final

    DSAMatch matches[16];
    int flattened = tree_flatten_matches(root, matches, 16);
    assert(flattened == 7);

    // Verify Round 1 Fold Pairing: 1v8, 4v5, 2v7, 3v6
    assert(strcmp(matches[0].team1_id, "team-1") == 0);
    assert(strcmp(matches[0].team2_id, "team-8") == 0);

    assert(strcmp(matches[1].team1_id, "team-4") == 0);
    assert(strcmp(matches[1].team2_id, "team-5") == 0);

    assert(strcmp(matches[2].team1_id, "team-2") == 0);
    assert(strcmp(matches[2].team2_id, "team-7") == 0);

    assert(strcmp(matches[3].team1_id, "team-3") == 0);
    assert(strcmp(matches[3].team2_id, "team-6") == 0);

    // Verify Winner Advancement:
    // Match 1 (team-1 vs team-8) -> winner team-1 advances to SF 1 slot 1
    int res = tree_advance_winner(root, matches[0].id, "team-1");
    assert(res == 0);

    // Match 2 (team-4 vs team-5) -> winner team-4 advances to SF 1 slot 2
    res = tree_advance_winner(root, matches[1].id, "team-4");
    assert(res == 0);

    // Flatten again to check Semifinal 1 participants
    flattened = tree_flatten_matches(root, matches, 16);
    assert(strcmp(matches[4].team1_id, "team-1") == 0);
    assert(strcmp(matches[4].team2_id, "team-4") == 0);

    // Complete Semifinal 1 -> winner team-1 advances to Final
    res = tree_advance_winner(root, matches[4].id, "team-1");
    assert(res == 0);

    flattened = tree_flatten_matches(root, matches, 16);
    assert(strcmp(matches[6].team1_id, "team-1") == 0); // Final slot 1

    // Verify Rollback Protection:
    // Attempting to revert QF 1 while SF 1 is completed must fail (-2)
    res = tree_revert_advancement(root, matches[0].id, "team-1");
    assert(res == -2);

    tree_free(root);
    printf("  -> Knockout Binary Tree tests PASSED.\n");
}

static void test_cyclic_schedule(void) {
    printf("[TEST] 2. Cyclic Fixed-Pivot Round-Robin Scheduler...\n");

    // Case A: Even team count (6 teams -> 15 matches, 5 rounds)
    DSATeam teams6[6];
    for (int i = 0; i < 6; i++) {
        snprintf(teams6[i].id, sizeof(teams6[i].id), "team-%d", i + 1);
        snprintf(teams6[i].name, sizeof(teams6[i].name), "Squad %d", i + 1);
    }

    DSAMatch matches6[30];
    int count6 = cyclic_generate_fixtures("comp-rr6", teams6, 6, "2026-10-30", matches6, 30);
    assert(count6 == 15); // 6 * 5 / 2 = 15

    // Verify no team plays itself and no duplicate matches
    for (int i = 0; i < count6; i++) {
        assert(strcmp(matches6[i].team1_id, matches6[i].team2_id) != 0);

        for (int j = i + 1; j < count6; j++) {
            bool dup1 = (strcmp(matches6[i].team1_id, matches6[j].team1_id) == 0 && strcmp(matches6[i].team2_id, matches6[j].team2_id) == 0);
            bool dup2 = (strcmp(matches6[i].team1_id, matches6[j].team2_id) == 0 && strcmp(matches6[i].team2_id, matches6[j].team1_id) == 0);
            assert(!dup1 && !dup2);
        }
    }

    // Case B: Odd team count (5 teams -> 10 matches, 5 rounds, 1 BYE per round)
    DSATeam teams5[5];
    for (int i = 0; i < 5; i++) {
        snprintf(teams5[i].id, sizeof(teams5[i].id), "team-%d", i + 1);
        snprintf(teams5[i].name, sizeof(teams5[i].name), "Squad %d", i + 1);
    }

    DSAMatch matches5[30];
    int count5 = cyclic_generate_fixtures("comp-rr5", teams5, 5, "2026-10-30", matches5, 30);
    assert(count5 == 10); // 5 * 4 / 2 = 10

    for (int i = 0; i < count5; i++) {
        assert(strcmp(matches5[i].team1_id, "BYE") != 0);
        assert(strcmp(matches5[i].team2_id, "BYE") != 0);
    }

    printf("  -> Cyclic Round-Robin tests PASSED.\n");
}

static void test_merge_sort_standings(void) {
    printf("[TEST] 3. Multi-Attribute Merge Sort Standings Engine...\n");

    DSATeam teams[5] = {
        { .name = "Team A", .points = 6, .score_diff = 4, .score_for = 5, .wins = 2, .seed = 1 },
        { .name = "Team B", .points = 6, .score_diff = 4, .score_for = 6, .wins = 2, .seed = 2 }, // Higher score_for than Team A
        { .name = "Team C", .points = 4, .score_diff = 0, .score_for = 3, .wins = 1, .seed = 3 },
        { .name = "Team D", .points = 3, .score_diff = -1, .score_for = 2, .wins = 1, .seed = 4 },
        { .name = "Team E", .points = 3, .score_diff = -2, .score_for = 1, .wins = 1, .seed = 5 },
    };

    merge_sort_standings(teams, 5);

    // Team B should be rank 1 (tie on points and diff, but more goals for: 6 vs 5)
    assert(strcmp(teams[0].name, "Team B") == 0);
    assert(teams[0].rank == 1);

    // Team A should be rank 2
    assert(strcmp(teams[1].name, "Team A") == 0);
    assert(teams[1].rank == 2);

    // Team C should be rank 3
    assert(strcmp(teams[2].name, "Team C") == 0);
    assert(teams[2].rank == 3);

    // Team D should be rank 4 (better goal diff than Team E: -1 vs -2)
    assert(strcmp(teams[3].name, "Team D") == 0);
    assert(teams[3].rank == 4);

    // Team E should be rank 5
    assert(strcmp(teams[4].name, "Team E") == 0);
    assert(teams[4].rank == 5);

    printf("  -> Merge Sort Standings tests PASSED.\n");
}

static void test_interval_conflict_detector(void) {
    printf("[TEST] 4. Interval Overlap Scheduling Conflict Detector...\n");

    // Epoch base: 1000000000 (e.g. 14:00)
    int64_t base = 1000000000;

    DSAInterval intervals[4] = {
        // M1: 14:00 - 15:00 at Main Stadium Turf
        { .match_id = "m1", .venue = "Main Stadium Turf", .start_time_epoch = base, .duration_minutes = 60 },
        // M2: 14:30 - 15:30 at Main Stadium Turf (OVERLAPS M1)
        { .match_id = "m2", .venue = "Main Stadium Turf", .start_time_epoch = base + 1800, .duration_minutes = 60 },
        // M3: 15:00 - 16:00 at Main Stadium Turf (ADJACENT to M1, but OVERLAPS M2)
        { .match_id = "m3", .venue = "Main Stadium Turf", .start_time_epoch = base + 3600, .duration_minutes = 60 },
        // M4: 14:15 - 15:15 at East Arena (DIFFERENT VENUE, NO CONFLICT)
        { .match_id = "m4", .venue = "East Arena", .start_time_epoch = base + 900, .duration_minutes = 60 },
    };

    DSAConflict conflicts[8];
    int count = detect_scheduling_conflicts(intervals, 4, conflicts, 8);

    // Expected conflicts:
    // (M1, M2) on Main Stadium Turf
    // (M2, M3) on Main Stadium Turf
    assert(count == 2);
    assert(strcmp(conflicts[0].match1_id, "m1") == 0 && strcmp(conflicts[0].match2_id, "m2") == 0);
    assert(strcmp(conflicts[1].match1_id, "m2") == 0 && strcmp(conflicts[1].match2_id, "m3") == 0);

    // Test adjacent interval invariant:
    assert(intervals_overlap(100, 200, 200, 300) == false); // Adjacent
    assert(intervals_overlap(100, 201, 200, 300) == true);  // Overlapping

    printf("  -> Interval Overlap Conflict Detector tests PASSED.\n");
}

static void test_audit_list(void) {
    printf("[TEST] 5. Linked List Audit Trail & Rollback Engine...\n");

    DSAHistoryNode *list = audit_list_create();

    DSAHistoryNode rec1 = { .id = "h1", .action_type = "RESULT_ENTERED", .can_undo = true };
    DSAHistoryNode rec2 = { .id = "h2", .action_type = "RESULT_ENTERED", .can_undo = false };

    audit_list_push(&list, &rec1);
    audit_list_push(&list, &rec2);

    DSAHistoryNode *found = audit_list_find(list, "h1");
    assert(found != NULL);
    assert(strcmp(found->id, "h1") == 0);

    // Test undo eligibility:
    // If downstream match is not completed, can undo
    assert(audit_list_can_undo(found, false) == true);
    // If downstream match is completed, CANNOT undo
    assert(audit_list_can_undo(found, true) == false);

    audit_list_free(list);
    printf("  -> Linked List Audit Trail tests PASSED.\n");
}

int main(void) {
    printf("=====================================================\n");
    printf("  MatchMint DSA Algorithm Test Suite\n");
    printf("=====================================================\n");

    test_binary_tree_bracket();
    test_cyclic_schedule();
    test_merge_sort_standings();
    test_interval_conflict_detector();
    test_audit_list();

    printf("=====================================================\n");
    printf("  ALL 5 CORE DSA MODULES VERIFIED & PASSED (100%%)\n");
    printf("=====================================================\n");
    return 0;
}
