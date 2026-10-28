#include "dsa.h"
#include <stdio.h>
#include <string.h>

bool intervals_overlap(int64_t start1, int64_t end1, int64_t start2, int64_t end2) {
    // Two intervals strictly overlap if: start1 < end2 AND end1 > start2
    // Adjacent intervals (end1 == start2 or start1 == end2) do NOT overlap.
    return (start1 < end2 && end1 > start2);
}

int detect_scheduling_conflicts(
    const DSAInterval *intervals,
    int count,
    DSAConflict *out_conflicts,
    int max_conflicts
) {
    if (!intervals || count < 2 || !out_conflicts || max_conflicts <= 0) return 0;

    int conflict_count = 0;

    for (int i = 0; i < count; i++) {
        int64_t start1 = intervals[i].start_time_epoch;
        int64_t end1 = start1 + (intervals[i].duration_minutes * 60);

        for (int j = i + 1; j < count; j++) {
            // Check only if assigned to the same venue
            if (strcmp(intervals[i].venue, intervals[j].venue) != 0) continue;

            int64_t start2 = intervals[j].start_time_epoch;
            int64_t end2 = start2 + (intervals[j].duration_minutes * 60);

            if (intervals_overlap(start1, end1, start2, end2)) {
                if (conflict_count < max_conflicts) {
                    DSAConflict *c = &out_conflicts[conflict_count++];
                    snprintf(c->type, sizeof(c->type), "VENUE_COLLISION");
                    snprintf(c->match1_id, sizeof(c->match1_id), "%s", intervals[i].match_id);
                    snprintf(c->match2_id, sizeof(c->match2_id), "%s", intervals[j].match_id);
                    snprintf(c->venue, sizeof(c->venue), "%s", intervals[i].venue);
                    snprintf(c->time_slot, sizeof(c->time_slot), "Collision between match %s and %s", intervals[i].match_id, intervals[j].match_id);
                    snprintf(c->description, sizeof(c->description), "Venue clash at '%s': Match %s and Match %s overlap.", intervals[i].venue, intervals[i].match_id, intervals[j].match_id);
                }
            }
        }
    }

    return conflict_count;
}
