#ifndef BOARD
#define BOARD

#include <stdlib.h>
#include <stdbool.h>

/******************************************************************************/
/* Definición de la estructura de datos del tablero */

struct _board {
	size_t columns, rows;
	u_int8_t * data;
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

board_sub_t *divide_into_subrogates(size_t * len, board_t board, size_t nuproc, size_t minArea);

void subrogates_destroy(board_sub_t * subrogates);

void board_subrogate_destroy(board_sub_t sub_board);

bool board_sub_get(board_sub_t sub_board, int col, int row);

void board_sub_show(board_sub_t board);

/* Leer el tablero en una posición (col, row) */
bool board_get(board_t board, unsigned int col, unsigned int row);

/* Leer el tablero en una posición asumiendo que el tablero es 'redondo'.*/
char board_get_round(board_t board, int col, int row);

/* Asignarle un valor 'val' a la posición (col, row) del tablero*/
void board_set(board_t board, unsigned int col, unsigned int row, bool val);

/* Leer de una lista de caracteres e interpretarla como un tablero */
int board_load(board_t board, char *str);

/* Función para mostrar el tablero */
/* La función 'board_show'*/
void board_show(board_t board);

#endif
