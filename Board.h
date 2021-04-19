#ifndef BOARD
#define BOARD

#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>

struct _board {
	size_t columns, rows;
	atomic_int_fast8_t * data;
};
typedef struct _board board_t;

struct _board_subdivider {
	board_t board;
	size_t top, left;
	size_t bottom, right;
};
typedef struct _board_subdivider board_subdivider_t;

/* Funciones sobre el tablero */

/* Creación del tablero */
board_t board_init(size_t col, size_t row);

/* Destroy board */
void board_destroy(board_t board);

/**
 * Crea las subdivisiones, estas usan una tabla subyacente para
 * indexar relativamente
 */
board_subdivider_t subdivider_board_init(board_t source, size_t left, size_t top, size_t right, size_t bottom);

/**
 * Destruye las subdivisiones
 */
void subdivider_destroy(board_subdivider_t * subrogates);

/**
 * Subdivide, guarda en len la cantidad de subdivisiones
 */
board_subdivider_t *subdivide(size_t * len, board_t board, size_t nuproc, size_t minArea);

/**
 * Devuelve true si la posicion relativa en la tabla esta determinada
 */
bool subdivider_board_is_set(board_subdivider_t board, int col, int row);

/**
 * Devuelve la posicion relativa en la tabla
 */
enum State board_subdivider_get(board_subdivider_t sub_board, int col, int row);

/**
 * Muestra la subtabla
 */
void board_sub_show(board_subdivider_t board);

/* Leer el tablero en una posición (col, row) */
enum State board_get(board_t board, unsigned int col, unsigned int row);

/* Asignarle un valor 'val' a la posición (col, row) del tablero*/
void board_set(board_t board, unsigned int col, unsigned int row, enum State val);

/* Función para mostrar el tablero */
void board_show(board_t board);

#endif
