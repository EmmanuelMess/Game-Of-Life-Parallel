#include <pthread.h>
#include <assert.h>
#include "CyclesTable.h"

list_t list_init(size_t nthreads, board_t firstBoard) {
	linked_list_t *node = malloc(sizeof(linked_list_t));
	*node = (linked_list_t) {
		.next = NULL,
		.frees = 0,
		.index = 0,
		.board = firstBoard
	};

	list_t list = {
		.nthreads = nthreads,
		.lock = malloc(sizeof(pthread_mutex_t)),
		.list = node
	};

	pthread_mutex_init(list.lock, NULL);

	return list;
}

void linked_list_destroy(linked_list_t *nodo) {
	if(nodo == NULL) {
		return;
	}

	linked_list_t *next = nodo->next;

	free(nodo);

	linked_list_destroy(next);
}

void list_destroy(list_t list) {
	linked_list_destroy(list.list);
	free(list.lock);
}

board_t linked_list_get(linked_list_t *node, size_t cycle, size_t nthreads) {
	if(node->index == cycle) {
		return node->board;
	}

	assert(node->index < cycle);

	if(node->next == NULL) {
		linked_list_t *nextNode = malloc(sizeof(linked_list_t));
		*nextNode = (linked_list_t) {
			.next = NULL,
			.frees = 0,
			.index = node->index + 1,
			.board = board_init(node->board.columns, node->board.rows)
		};

		node->next = nextNode;

		return linked_list_get(nextNode, cycle, nthreads);
	}

	return linked_list_get(node->next, cycle, nthreads);
}

board_t list_get(list_t *list, size_t cycle) {
	pthread_mutex_lock(list->lock);

	board_t r = linked_list_get(list->list, cycle, list->nthreads);

	pthread_mutex_unlock(list->lock);

	return r;
}

linked_list_t * linked_list_free(linked_list_t *nodo, size_t cycle, size_t nthreads) {
	if(nodo->index == cycle) {
		nodo->frees++;

		if(nodo->frees == nthreads) {
			linked_list_t *next = nodo->next;

			board_destroy(nodo->board);
			free(nodo);
			return next;
		}

		return NULL;
	}

	assert(nodo->index < cycle);

	return linked_list_free(nodo->next, cycle, nthreads);
}

void list_free_cycle(list_t *list, size_t cycle) {
	pthread_mutex_lock(list->lock);

	linked_list_t *firstNode = linked_list_free(list->list, cycle, list->nthreads);

	if(firstNode != NULL) {
		list->list = firstNode;
	}

	pthread_mutex_unlock(list->lock);
}
