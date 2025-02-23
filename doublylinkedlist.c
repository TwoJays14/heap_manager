//
// Created by Jermaine on 17/02/2025.
//
#include "doublylinkedlist.h"

DoublyListNode *new(const int x) {
    DoublyListNode *newNode = malloc(sizeof(DoublyListNode));

    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed. errno: %d\n", errno);
        return NULL;
    }

    newNode->value = x;
    newNode->next = NULL;
    newNode->prev = NULL;

    return newNode;
}

DoublyListNode *prepend(DoublyListNode *head, int x) {
    DoublyListNode *newNode = new(x);

    if(newNode == NULL) return head;

    newNode->next = head;
    if(head != NULL) head->prev = newNode;
    return newNode;

}

DoublyListNode *append(DoublyListNode *head, int x ) {
    DoublyListNode *newNode = new(x);

    if(newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for new node. errno: %d\n", errno);
        return head;
    }

    if(head == NULL) {
        return newNode;
    }

    DoublyListNode *current = head;
    while(current->next != NULL) {
        current = current->next;
    }

    current->next = newNode;
    newNode->prev = current;

    return head;
}

DoublyListNode *delete(DoublyListNode *head, int valueToRemove) {
    // if list is empty
    if(head == NULL) {
        return NULL;
    }

    // is the head node is the value to remove
    if(head->value == valueToRemove) {
        DoublyListNode *temp = head->next;
        free(head);
        if(temp != NULL) {
            temp->prev = NULL;
        }
        return temp;
    }

    DoublyListNode *current = head;

    while(current != NULL && current->value != valueToRemove) {
        current = current->next;
    }

    if (current == NULL) {
        return head;
    }

    if(current->prev != NULL) {
        current->prev->next = current->next;
    }
    if(current->next !=NULL) {
        current->next->prev = current->prev;
    }

    free(current);

    return head;
}

void printForward(DoublyListNode *head) {
    DoublyListNode *current = head;

    printf("Forward: ");
    while(current != NULL) {
        printf("%d -> ", current->value);
        current = current->next;
    }
    printf("NULL\n");
}

void printBackwards(DoublyListNode *head) {

    if(head == NULL) {
    printf("Backwards: NULL\n");
        return;
    }

    DoublyListNode *current = head;
    while(current->next != NULL) {
        current = current->next;
    }

    printf("Backwards: ");
    while(current != NULL) {
        printf("%d <- ", current->value);
        current = current->prev;
    }

    printf("NULL\n");
}

void freeList(DoublyListNode *head) {
    DoublyListNode *current = head;

    while (current != NULL) {
        DoublyListNode *next = current->next;
        free(current);
        current = next;
    }
}