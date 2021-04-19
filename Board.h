#ifndef BOARD
#define BOARD

#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>

/******************************************************************************/
/* Definición de la estructura de datos del tablero */

struct _board {
	size_t columns, rows;
	atomic_int_fast8_t * data;
};
typedef struct _board board_t;

struct _board_subrogate {
	board_t board;
	size_t top, left;
	size_t bottom, right;
};
typedef struct _board_subrogate board_sub_t;
/******************************************************************************/
/******************************************************************************/

/* Funciones sobre el tablero */

/* Creación del tablero */
board_t board_init(size_t col, size_t row);

/* Destroy board */
void board_destroy(board_t board);

board_sub_t subrogate_board_init(board_t source, size_t left, size_t top, size_t right, size_t bottom);

void subrogates_destroy(board_sub_t * subrogates);

board_sub_t *divide_into_subrogates(size_t * len, board_t board, size_t nuproc, size_t minArea);

bool subrogate_board_is_set(const board_sub_t board, int col, int row);

void board_subrogate_destroy(board_sub_t sub_board);

enum State board_sub_get(board_sub_t sub_board, int col, int row);

void board_sub_show(board_sub_t board);

/* Leer el tablero en una posición (col, row) */
enum State board_get(board_t board, unsigned int col, unsigned int row);

/* Asignarle un valor 'val' a la posición (col, row) del tablero*/
void board_set(board_t board, unsigned int col, unsigned int row, enum State val);

/* Función para mostrar el tablero */
void board_show(board_t board);

#endif
