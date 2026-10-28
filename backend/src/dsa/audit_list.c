#include "dsa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DSAHistoryNode *audit_list_create(void) {
    return NULL;
}

void audit_list_push(DSAHistoryNode **head, const DSAHistoryNode *entry) {
    if (!head || !entry) return;

    DSAHistoryNode *new_node = (DSAHistoryNode *)malloc(sizeof(DSAHistoryNode));
    if (!new_node) return;

    *new_node = *entry;
    new_node->next = *head;
    *head = new_node;
}

DSAHistoryNode *audit_list_find(DSAHistoryNode *head, const char *history_id) {
    if (!history_id) return NULL;
    DSAHistoryNode *curr = head;
    while (curr) {
        if (strcmp(curr->id, history_id) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

bool audit_list_can_undo(const DSAHistoryNode *record, bool is_downstream_completed) {
    if (!record) return false;
    if (!record->can_undo) return false;
    // Critical rollback safety invariant:
    // Cannot undo a match result if the next-round match has already finished!
    if (is_downstream_completed) return false;

    return true;
}

void audit_list_free(DSAHistoryNode *head) {
    DSAHistoryNode *curr = head;
    while (curr) {
        DSAHistoryNode *next = curr->next;
        free(curr);
        curr = next;
    }
}
