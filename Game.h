#ifndef GAME_TYPES
#define GAME_TYPES
#include "Board.h"

enum State {
	UNSET = 0,
	ALIVE = 1,
	DEAD = 2
};

/**
 * Mantiene el juego
 */
struct _game {
	board_t board;
	unsigned int cycles;
};

typedef struct _game game_t;

/* Cargamos el juego desde un archivo */
game_t *loadGame(const char *filename);

/* Guardamos el tablero 'board' en el archivo 'filename' */
void writeBoard(board_t board, const char *filename);

/* Simulamos el Juego de la Vida de Conway con tablero 'board' la cantidad de
ciclos indicados en 'cycles' en 'nuprocs' unidades de procesamiento*/
board_t *congwayGoL(board_t *board, unsigned int cycles, int nuproc);

#endif
