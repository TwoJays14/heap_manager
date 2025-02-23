//
// Created by Jermaine on 17/02/2025.
//

#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct DoublyListNode {
  size_t value;
  struct DoublyListNode *next;
  struct DoublyListNode *prev;
} DoublyListNode;

DoublyListNode *new(const int x);
DoublyListNode *prepend(DoublyListNode *head, int x);
DoublyListNode *append(DoublyListNode *head, int x);
DoublyListNode *delete(DoublyListNode *head, int valueToRemove);
void printForward(DoublyListNode *head);
void printBackwards(DoublyListNode *head);
void freeList(DoublyListNode *head);

#endif //DOUBLYLINKEDLIST_H
