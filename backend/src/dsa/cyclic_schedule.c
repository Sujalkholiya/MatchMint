#include "dsa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cyclic_generate_fixtures(
    const char *competition_id,
    const DSATeam *teams,
    int team_count,
    const char *start_date,
    DSAMatch *out_matches,
    int max_matches
) {
    if (team_count < 2 || !teams || !out_matches) return -1;

    // Handle odd team counts by adding a dummy BYE team
    bool is_odd = (team_count % 2 != 0);
    int total_participants = is_odd ? team_count + 1 : team_count;

    DSATeam *participants = (DSATeam *)calloc(total_participants, sizeof(DSATeam));
    if (!participants) return -1;

    for (int i = 0; i < team_count; i++) {
        participants[i] = teams[i];
    }

    if (is_odd) {
        snprintf(participants[total_participants - 1].id, sizeof(participants[0].id), "BYE");
        snprintf(participants[total_participants - 1].name, sizeof(participants[0].name), "BYE");
    }

    int total_rounds = total_participants - 1;
    int matches_per_round = total_participants / 2;
    int match_counter = 1;
    int match_idx = 0;

    for (int round = 1; round <= total_rounds; round++) {
        for (int i = 0; i < matches_per_round; i++) {
            DSATeam *t1 = &participants[i];
            DSATeam *t2 = &participants[total_participants - 1 - i];

            // If neither team is BYE, emit a real match
            if (strcmp(t1->id, "BYE") != 0 && strcmp(t2->id, "BYE") != 0) {
                if (match_idx >= max_matches) {
                    free(participants);
                    return match_idx;
                }

                DSAMatch *m = &out_matches[match_idx++];
                memset(m, 0, sizeof(DSAMatch));

                snprintf(m->id, sizeof(m->id), "match-%s-rr%d-m%d", competition_id, round, i + 1);
                snprintf(m->competition_id, sizeof(m->competition_id), "%s", competition_id);
                m->round = round;
                snprintf(m->round_name, sizeof(m->round_name), "Round %d", round);
                m->match_number = match_counter++;

                // Alternate home and away to distribute field advantage
                if (i % 2 == 0) {
                    snprintf(m->team1_id, sizeof(m->team1_id), "%s", t1->id);
                    snprintf(m->team2_id, sizeof(m->team2_id), "%s", t2->id);
                } else {
                    snprintf(m->team1_id, sizeof(m->team1_id), "%s", t2->id);
                    snprintf(m->team2_id, sizeof(m->team2_id), "%s", t1->id);
                }

                snprintf(m->status, sizeof(m->status), "upcoming");
                snprintf(m->scheduled_time, sizeof(m->scheduled_time), "%s 15:00", start_date);
                snprintf(m->venue, sizeof(m->venue), "Main Stadium");
            }
        }

        // Circle algorithm rotation: Keep index 0 fixed, rotate rest clockwise
        DSATeam last = participants[total_participants - 1];
        for (int k = total_participants - 1; k > 1; k--) {
            participants[k] = participants[k - 1];
        }
        participants[1] = last;
    }

    free(participants);
    return match_idx;
}
