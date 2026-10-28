#include "dsa.h"
#include <stdlib.h>

static int compare_teams(const DSATeam *a, const DSATeam *b) {
    // 1. Points (descending)
    if (a->points != b->points) {
        return (b->points - a->points);
    }
    // 2. Score difference (descending)
    if (a->score_diff != b->score_diff) {
        return (b->score_diff - a->score_diff);
    }
    // 3. Score for (descending)
    if (a->score_for != b->score_for) {
        return (b->score_for - a->score_for);
    }
    // 4. Wins (descending)
    if (a->wins != b->wins) {
        return (b->wins - a->wins);
    }
    // 5. Deterministic fallback: seed (ascending)
    return (a->seed - b->seed);
}

static void merge(DSATeam *arr, DSATeam *temp, int left, int mid, int right) {
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        if (compare_teams(&arr[i], &arr[j]) <= 0) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid) {
        temp[k++] = arr[i++];
    }

    while (j <= right) {
        temp[k++] = arr[j++];
    }

    for (i = left; i <= right; i++) {
        arr[i] = temp[i];
    }
}

static void merge_sort_recursive(DSATeam *arr, DSATeam *temp, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    merge_sort_recursive(arr, temp, left, mid);
    merge_sort_recursive(arr, temp, mid + 1, right);
    merge(arr, temp, left, mid, right);
}

void merge_sort_standings(DSATeam *teams, int count) {
    if (!teams || count <= 1) return;

    DSATeam *temp = (DSATeam *)malloc(sizeof(DSATeam) * count);
    if (!temp) return;

    merge_sort_recursive(teams, temp, 0, count - 1);
    free(temp);

    // Update ranks (1-indexed)
    for (int i = 0; i < count; i++) {
        teams[i].rank = i + 1;
    }
}
