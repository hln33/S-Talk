#include "list.h"
#include <stddef.h>
#include <stdio.h>

static List lists[LIST_MAX_NUM_HEADS];
static Node nodes[LIST_MAX_NUM_NODES];
static int listsUsed = 0;
static int nodesUsed = 0;

// linked list type data structure to keep track of free lists
typedef struct freeLists_s FLists;
struct freeLists_s {
	List* head;
};
// linked list type data structure to keep track of free nodes
typedef struct freeNodes_s FNodes;
struct freeNodes_s {
	Node* head;
};
static FLists freeLists = {NULL};
static FNodes freeNodes = {NULL};

// add a deleted list to freeLists for future use
static void addToFreeLists(List* pList) {
	printf("List freed\n");
	pList->nextRecycledList = NULL;
	pList->state = -1; // this signifys that the list is freed
	// add list to freeLists
	if (freeLists.head == NULL) {
		freeLists.head = pList;
	}
	else {
		pList->nextRecycledList = freeLists.head;
		freeLists.head = pList;
	}
}
// return a free List
static List* returnRecycledList() {
		List* recycledList = NULL;
		if (freeLists.head != NULL) {
			recycledList = freeLists.head;
			recycledList->state = LIST_OOB_START;
		}
		// if the head is the last free list, make it null
		if (freeLists.head->nextRecycledList == NULL) {
			freeLists.head = NULL;
		}
		// if we have another free list, shift it over
		else if (freeLists.head->nextRecycledList != NULL) {
			freeLists.head = freeLists.head->nextRecycledList;
		}
		return recycledList;
}

// add a deleted node to freeNodes for future use
static void addToFreeNodes(Node* recycledNode) {
	if (freeNodes.head == NULL) {
		freeNodes.head = recycledNode;
	}
	else {
		recycledNode->previous = freeNodes.head;
		freeNodes.head = recycledNode;
	}
}
// return a free node
static Node* returnRecycledNode() {
		Node* recycledNode = NULL;
		if (freeNodes.head != NULL) {
			recycledNode = freeNodes.head;
		}
		// if the head is the last free node, make it null
		if (freeNodes.head->previous == NULL) {
			freeNodes.head = NULL;
		}
		// if we have another free node, shift it over
		else if (freeNodes.head->previous != NULL) {
			freeNodes.head = freeNodes.head->previous;
		}
		return recycledNode;
}


// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.
List* List_create() {
	// check if we can recycle any lists
	// printf("Lists num:%d\n", listsUsed);
	if (freeLists.head != NULL) {
		return returnRecycledList();
	}

	// check if we are allowed to make a new head
	else if (listsUsed < LIST_MAX_NUM_HEADS) {
		List newList = {0, 0, NULL, NULL, NULL, NULL};
		lists[listsUsed] = newList;
		return &lists[listsUsed++];
	} 

	else {
		printf("Already at Max Number of Lists\n");
		return NULL;
	}
}

// Returns the number of items in pList.
int List_count(List* pList) {
	// if list is NULL/freed
	if (pList->state == -1) {return 0;}

	return pList->size;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList) {
	// if list is NULL/freed
	if (pList->state == -1) {return NULL;}

	// check if list is empty
	if (pList->size == 0) {
		pList->current = NULL;
		return NULL;
	} else {
		pList->current = pList->start;
		return pList->start->item;
	}
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList) {
	// if list is NULL/freed
	if (pList->state == -1) {return NULL;}

	// check if list is empty
	if (pList->size == 0) {
		pList->current = NULL;
		return NULL;
	} else {
		pList->current = pList->end;
		return pList->end->item;
	}
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList) {
	// if list is NULL/freed
	if (pList->state == -1) {return NULL;}

	// if list is empty
	if (pList->size == 0) {
		pList->state = LIST_OOB_END;
	}

	// check if this operation advances beyond end of pList
	else if (pList->current == pList->end) {
		pList->state = LIST_OOB_END;
		pList->current = NULL;
		return NULL;
	}
	// case: current is beyond end
	else if (pList->current == NULL && pList->state == LIST_OOB_END) {
		return NULL;
	}
	// case: current pointer is before start
	else if (pList->current == NULL && pList->state == LIST_OOB_START) {
		pList->current = pList->start;
	}
	else {
		pList->current = pList->current->next;
	}
	return pList->current->item;
}

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList) {
	// if list is NULL/freed
	if (pList->state == -1) {return NULL;}

	// if list is empty
	if (pList->size == 0) {
		pList->state = LIST_OOB_START;
	}

	// check if operation backs up beyond start of pLIst
	else if (pList->current == pList->start) {
		pList->state = LIST_OOB_START;
		pList->current = NULL;
		return NULL;
	}
	// case: current is before start
	else if (pList->current == NULL && pList->state == LIST_OOB_START) {
		return NULL;
	}
	// case: current pointer is beyond end
	else if (pList->current == NULL && pList->state == LIST_OOB_END) {
		pList->current = pList->end;
	}
	else {
		pList->current = pList->current->previous;
	}
	return pList->current->item;
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList) {
	// if list is NULL/freed
	if (pList->state == -1) {return NULL;}
	// if list is empty or pointing to nothing
	if (pList->size == 0 || pList->current == NULL) {
		return NULL;
	}

	return pList->current->item;
}

// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_add(List* pList, void* pItem) {
	// if list is NULL/freed
	if (pList->state == -1) {return -1;}

	Node* addedNode = NULL;
	// check if we can recycle any nodes
	if (freeNodes.head != NULL) {
		addedNode = returnRecycledNode();
		addedNode->item = pItem;
	}
	// if no free nodes then check if we have enough space to add a new node
	else if (nodesUsed >= LIST_MAX_NUM_NODES) {
		printf("Already at Max Number of Nodes\n");
		return -1;
	}
	// if we have no free nodes AND have enough space, make new node
	else {
		Node newNode = {pItem, NULL, NULL};
		nodes[nodesUsed] = newNode;
		addedNode = &nodes[nodesUsed++];
	}

	// case: empty list
	if (pList->size == 0) {
		pList->current = addedNode;
		pList->start = addedNode;
		pList->end = addedNode;
	}

	// case: list has 1 item
	else if (pList->size == 1) {
		pList->current = addedNode;
		pList->current->previous = pList->start;
		pList->end = addedNode;
		pList->start->next = addedNode;
	}

	// case: current is pointing to end
	else if (pList->current == pList->end) {
		addedNode->previous = pList->end;

		pList->end->next = addedNode;
		pList->current = addedNode;
		pList->end = addedNode;
	}

	// case: current is pointing before start
	else if (pList->current == NULL && pList->state == LIST_OOB_START) {
		addedNode->next = pList->start;

		pList->start->previous = addedNode;
		pList->current = addedNode;
		pList->start = addedNode;
	}

	// case: current is pointing beyond end
	else if (pList->current == NULL && pList->state == LIST_OOB_END) {
		addedNode->previous = pList->end;

		pList->end->next = addedNode;
		pList->current = addedNode;
		pList->end = addedNode;
	}

	// case: normal
	else {
		addedNode->previous = pList->current;
		addedNode->next = pList->current->next;

		pList->current->next = addedNode;
		pList->current = addedNode;
	}
	++(pList->size);
	return 0;
}

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert(List* pList, void* pItem) {
	// if list is NULL/freed
	if (pList->state == -1) {return -1;}

	Node* addedNode = NULL;
	// check if we can recycle any nodes
	if (freeNodes.head != NULL) {
		addedNode = returnRecycledNode();
		addedNode->item = pItem;
	}
	// if no free nodes then check if we have enough space to add a new node
	else if (nodesUsed >= LIST_MAX_NUM_NODES) {
		printf("Already at Max Number of Nodes\n");
		return -1;
	}
	// if we have no free nodes AND have enough space, make new node
	else {
		Node newNode = {pItem, NULL, NULL};
		nodes[nodesUsed] = newNode;
		addedNode = &nodes[nodesUsed++];
	}

	// case: empty list
	if (pList->size == 0) {
		pList->current = addedNode;
		pList->start = addedNode;
		pList->end = addedNode;
	}

	// case: current is pointing to start
	else if (pList->current == pList->start) {
		addedNode->next = pList->start;

		pList->start->previous = addedNode;
		pList->current = addedNode;
		pList->start = addedNode;
	}

	// case: current is pointing before start
	else if (pList->current == NULL && pList->state == LIST_OOB_START) {
		addedNode->next = pList->start;

		pList->start->previous = addedNode;
		pList->current = addedNode;
		pList->start = addedNode;
	}

	// case: current is pointing beyond end
	else if (pList->current == NULL && pList->state == LIST_OOB_END) {
		addedNode->previous = pList->end;

		pList->end->next = addedNode;
		pList->current = addedNode;
		pList->end = addedNode;
	}

	// case: normal
	else {
		addedNode->next = pList->current;
		addedNode->previous = pList->current->previous;

		pList->current->previous = addedNode;
		List_prev(pList);
		List_prev(pList);
		pList->current->next = addedNode;

		pList->current = addedNode;
	}
	++(pList->size);
	return 0;
}

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem) {
	// if list is NULL/freed
	if (pList->state == -1) {return -1;}

	Node* addedNode = NULL;
	// check if we can recycle any nodes
	if (freeNodes.head != NULL) {
		addedNode = returnRecycledNode();
		addedNode->item = pItem;
	}
	// if no free nodes then check if we have enough space to add a new node
	else if (nodesUsed >= LIST_MAX_NUM_NODES) {
		printf("Already at Max Number of Nodes\n");
		return -1;
	}
	// if we have no free nodes AND have enough space, make new node
	else {
		Node newNode = {pItem, NULL, NULL};
		nodes[nodesUsed] = newNode;
		addedNode = &nodes[nodesUsed++];
	}

	// case: empty list
	if (pList->size == 0) {
		pList->current = addedNode;
		pList->start = addedNode;
		pList->end = addedNode;
	}

	// case: normal
	else {
		addedNode->previous = pList->end;
		pList->end->next = addedNode;
		pList->end = addedNode;
		pList->current = addedNode;
	}
	++(pList->size);
	return 0;
}

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem) {
	// if list is NULL/freed
	if (pList->state == -1) {return -1;}

	Node* addedNode = NULL;
	// check if we can recycle any nodes
	if (freeNodes.head != NULL) {
		addedNode = returnRecycledNode();
		addedNode->item = pItem;
	}
	// if no free nodes then check if we have enough space to add a new node
	else if (nodesUsed >= LIST_MAX_NUM_NODES) {
		printf("Already at Max Number of Nodes\n");
		return -1;
	}
	// if we have no free nodes AND have enough space, make new node
	else {
		Node newNode = {pItem, NULL, NULL};
		nodes[nodesUsed] = newNode;
		addedNode = &nodes[nodesUsed++];
	}

	// case: empty list
	if (pList->size == 0) {
		pList->current = addedNode;
		pList->start = addedNode;
		pList->end = addedNode;
		++(pList->size);
	}

	// case: normal
	else {
		addedNode->next = pList->start;
		pList->start->previous = addedNode;
		pList->start = addedNode;
		pList->current = addedNode;
		++(pList->size);
	}
	return 0;
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList) {
	// if list is NULL/freed
	if (pList->state == -1) {return NULL;}

	// case: empty list
	if (pList->size == 0) {
		return NULL;
	}
	// case: current pointer is before start
	else if (pList->current == NULL && pList->state == LIST_OOB_START) {
		return NULL;
	}
	// case: current pointer is beyond end
	else if (pList->current == NULL && pList->state == LIST_OOB_END) {
		return NULL;
	}

	Node* removedNode = pList->current;
	// case: list has 1 item
	if (pList->size == 1) {
		pList->end = NULL;
		pList->start = NULL;
		pList->current = NULL;
	}

	// case: current pointer is at start
	else if (pList->current == pList->start) {
		pList->start = pList->start->next;
		pList->start->previous = NULL;
		pList->current = pList->start;
	}

	// case: current pointer is at end
	else if (pList->current == pList->end) {
		pList->end = pList->end->previous;
		pList->end->next = NULL;
		pList->current = pList->end;
	}

	// case: normal
	else {
		Node* before = removedNode->previous;
		Node* after = removedNode->next;

		before->next = after;
		after->previous = before;
		pList->current = after;
	}
	--(pList->size);
	addToFreeNodes(removedNode);
	return removedNode->item;
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2) {
	// if either argument are NULL/freed just return
	if (pList1->state == -1 || pList2->state == -1) {return;}

	// case: pList1 is empty
	if (pList1->size == 0) {
		pList1->current = NULL;
		pList1->state = LIST_OOB_START;

		pList1->start = pList2->start;
		pList1->end = pList2->end;
	}
	// case: pList2 is empty
	else if (pList2->size == 0) {
		return;
	}
	// case: both lists are empty
	else if (pList1->size == 0 && pList2->size == 0) {
		return;
	}
	// case: normal
	else {
		pList1->end->next = pList2->start;
		pList1->end = pList2->end;
	}
	pList1->size = pList1->size + pList2->size;
	
	// free pList2
	pList2->current = NULL;
	pList2->start = NULL;
	pList2->end = NULL;
	pList2->size = 0;

	addToFreeLists(pList2);
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
//typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn) {
	// if list is NULL/freed
	if (pList->state == -1) {return;}

	List_first(pList);
	while (pList->current != NULL) {
		(*pItemFreeFn)(pList->current->item);
		addToFreeNodes(pList->current);
		pList->current->item = NULL;
		List_next(pList);
	}
	pList->current = NULL;
	pList->start = NULL;
	pList->end = NULL;
	pList->size = 0;

	addToFreeLists(pList);
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList) {
	// if list is NULL/freed
	if (pList->state == -1) {return NULL;}
	// case: empty list
	if (pList->size == 0) {
		return NULL;
	}

	Node* removedNode = pList->end;
	// case: list has 1 item
	if (pList->size == 1) {
		pList->current = NULL;
		pList->start = NULL;
		pList->end = NULL;
		//addToFreeLists(pList);
	}

	// case: normal
	else {
		pList->end = pList->end->previous;
		pList->end->next = NULL;
		pList->current = pList->end;
	}
	--(pList->size);
	addToFreeNodes(removedNode);
	return removedNode->item;
}

// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
//typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
	// if list is NULL/freed
	if (pList->state == -1) {return NULL;}
	// case: empty list
	if (pList->size == 0) {
		return NULL;
	}

	// case: current pointer is beyond end
	if (pList->current == NULL && pList->state == LIST_OOB_END) {
		return NULL;
	}

	// case: list has 1 item
	else if (pList->size == 1) {
		if ((*pComparator)(pList->current->item, pComparisonArg)) {
			return pList->current->item;
		}
	}

	// case: current pointer is before start
	else if (pList->current == NULL && pList->state == LIST_OOB_START) {
		pList->current = pList->start;
		while (pList->current != pList->end) {
			if ((*pComparator)(pList->current->item, pComparisonArg)) {
				return pList->current;
			}
			pList->current = pList->current->next;
		}
		// if we are here then no match was found
		pList->current = NULL;
		pList->state = LIST_OOB_END;
	}

	// case: normal
	else {
		while (pList->current != pList->end) {
			if ((*pComparator)(pList->current->item, pComparisonArg)) {
				return pList->current->item;
			}
			pList->current = pList->current->next;
		}
		// if we are here then no match was found
		pList->current = NULL;
		pList->state = LIST_OOB_END;
	}
	return NULL;
}