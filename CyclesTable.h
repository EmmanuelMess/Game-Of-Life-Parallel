#ifndef ENTREGA_TP_1_CYCLESTABLE_H
#define ENTREGA_TP_1_CYCLESTABLE_H

#include "Board.h"

typedef struct _linked_list {
	struct _linked_list * next;
	size_t index;
	size_t frees;
	board_t board;
} linked_list_t;

/**
 * Invariante: la lista nunca es vacia
 * Invariante: a cada elemento se libera a lo sumo nthread veces
 */
typedef struct _list {
	size_t nthreads;
	linked_list_t *list;
	pthread_mutex_t *lock;
} list_t;

list_t list_init(size_t nthreads, board_t firstBoard);

void list_destroy(list_t list);

board_t list_get(list_t *list, size_t cycle);

void list_free_cycle(list_t *list, size_t cycle);

#endif //ENTREGA_TP_1_CYCLESTABLE_H
