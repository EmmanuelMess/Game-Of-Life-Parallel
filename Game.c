#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "Board.h"
#include "Game.h"
#include "CyclesTable.h"

/**
 * Tabla con todas las opciones para las regiones de 3x3
 * table: tablero 3x3 -> celula central
 */
static enum State table[512];

/**
 * Ba'haal the end of line eater
 * Come todo hasta la siguiente linea.
 * Si la siguiente linea contiene un EOF al inicio, tambien lo come.
 */
static void comer_finales_de_linea(FILE * archivo) {
	int car;

	while ((car = fgetc(archivo)) == '\r' || car == '\n')
		if (car == '\r')
			fgetc(archivo);          // come el \n

	if (car != EOF) ungetc(car, archivo);
}

static void init_table() {
	static bool tableCreate = false;

	if (tableCreate) {
		return;
	}

	for (int i = 0; i < 512; ++i) {//i es el tablero de 3x3 en binario
		int t[9];//t es el tablero de 3x3 descomprimido
		for (int j = 0; j < 9; ++j) {
			int val = (i >> j) & 1;
			t[j] = val;
		}

		int cellCenter = t[4];
		int aliveAround = 0;
		for (int j = 0; j < 9; ++j) {
			if(j == 4) continue;
			if(t[j] == 1) aliveAround++;
		}

		if(cellCenter == 0) {
			if(aliveAround == 3) {
				table[i] = ALIVE;
			} else {
				table[i] = DEAD;
			}
		} else {
			if(aliveAround == 2 || aliveAround == 3) {
				table[i] = ALIVE;
			} else {
				table[i] = DEAD;
			}
		}
	}

	tableCreate = true;
}

game_t *loadGame(const char *filename) {
	init_table();

	FILE *file = fopen(filename, "r");

	int cycles;
	fscanf(file, "%d ", &cycles);

	int rows;
	fscanf(file, "%d ", &rows);

	int columns;
	fscanf(file, "%d ", &columns);

	comer_finales_de_linea(file);

	board_t board = board_init(columns, rows);

	for (int i = 0; i < rows; ++i) {
		char *line = calloc(2*columns + 1, sizeof(char));

		fscanf(file, "%[^\n]", line);

		unsigned int board_column_index = 0;

		char number[50] = {0};
		unsigned int num_index = 0;

		for (int j = 0; j < strlen(line); ++j) {
			if ('0' <= line[j] && line[j] <= '9') {
				number[num_index++] = line[j];
			} else {
				unsigned int cant = atoi(number);

				if (line[j] == 'O' || line[j] == 'X') {
					for (int k = 0; k < cant; ++k) {
						board_set(board, board_column_index++, i,  line[j] == 'O' ? ALIVE : DEAD);
					}
				} else {
					perror("Error: caracter malo");
				}


				memset(&number, 0, sizeof(number));
				num_index = 0;
			}
		}

		free(line);

		comer_finales_de_linea(file);
	}

	fclose(file);

	game_t *game = malloc(sizeof(game_t));
	game->board = board;
	game->cycles = cycles;

	return game;
}

void writeBoard(board_t board, const char *filename) {
	FILE *file = fopen(filename, "w");

	for (size_t i = 0; i < board.rows; ++i) {
		for (int j = 0; j < board.columns; ++j) {
			size_t amount = 1;
			enum State state = board_get(board, j, i);
			while (j + 1 < board.columns && state == board_get(board, j + 1, i)) {
				amount++;
				j++;
			}
			fprintf(file, "%zu%c", amount, (state == ALIVE? 'O': 'X'));
		}

		fprintf(file, "\n");
	}

	fclose(file);
}

static void processCenter(board_t finalBoard, board_sub_t subrogateBoard) {
	size_t columns = subrogateBoard.right - subrogateBoard.left;
	size_t rows = subrogateBoard.bottom - subrogateBoard.top;

	for (int i = 1; i < columns - 1; ++i) {
		for (int j = 0; j < rows; ++j) {
			int indexToShift = 8;
			int x = 0;
			for (int k = -1; k <= 1; ++k) {
				for (int l = -1; l <= 1; ++l) {
					enum State valueInBoard = board_sub_get(subrogateBoard, i + k, j + l);
					x |= (valueInBoard == ALIVE? 1 : 0) << indexToShift;
					indexToShift--;
				}
			}

			size_t absolutePositionCol = subrogateBoard.left + i;
			size_t absolutePositionRow = subrogateBoard.top + j;

			board_set(finalBoard, absolutePositionCol, absolutePositionRow, table[x]);
		}
	}
}

static void processLeft(board_t finalBoard, board_sub_t subrogateBoard, size_t row) {
	int indexToShift = 8;
	int x = 0;
	for (int k = -1; k <= 1; ++k) {
		for (int l = -1; l <= 1; ++l) {
			enum State valueInBoard = board_sub_get(subrogateBoard, 0 + k, row + l);
			x |= (valueInBoard == ALIVE? 1 : 0) << indexToShift;
			indexToShift--;
		}
	}

	size_t absolutePositionCol = subrogateBoard.left;
	size_t absolutePositionRow = subrogateBoard.top + row;

	board_set(finalBoard, absolutePositionCol, absolutePositionRow, table[x]);
}

static size_t processLeftBorderBlocking(board_t finalBoard, board_sub_t subrogateBoard, size_t startPos, bool blocking) {
	size_t rows = subrogateBoard.bottom - subrogateBoard.top;

	for (int j = startPos; j < rows; ++j) {
		if(blocking) {
			//Esto impide que se calculen valores sobre celdas que no estan computadas
			bool blocked = true;
			while (blocked) {
				blocked = !subrogate_board_is_set(subrogateBoard, -1, j - 1)
				          || !subrogate_board_is_set(subrogateBoard, -1, j)
				          || !subrogate_board_is_set(subrogateBoard, -1, j + 1);
			}
		} else {
			if(!subrogate_board_is_set(subrogateBoard, -1, j - 1)
			   || !subrogate_board_is_set(subrogateBoard, -1, j)
			   || !subrogate_board_is_set(subrogateBoard, -1, j + 1)) {
				return j;
			}
		}
		processLeft(finalBoard, subrogateBoard, j);
	}

	return rows;
}

static void processRight(board_t finalBoard, board_sub_t subrogateBoard, size_t row) {
	size_t columns = subrogateBoard.right - subrogateBoard.left;

	int indexToShift = 8;
	int x = 0;
	for (int k = -1; k <= 1; ++k) {
		for (int l = -1; l <= 1; ++l) {
			enum State valueInBoard = board_sub_get(subrogateBoard, columns - 1 + k, row + l);
			x |= (valueInBoard == ALIVE? 1 : 0) << indexToShift;
			indexToShift--;
		}
	}

	size_t absolutePositionCol = subrogateBoard.right - 1;
	size_t absolutePositionRow = subrogateBoard.top + row;

	board_set(finalBoard, absolutePositionCol, absolutePositionRow, table[x]);
}

static size_t processRightBorderBlocking(board_t finalBoard, board_sub_t subrogateBoard, size_t startPos, bool blocking) {
	size_t rows = subrogateBoard.bottom - subrogateBoard.top;
	size_t columns = subrogateBoard.right - subrogateBoard.left;

	for (int j = startPos; j < rows; ++j) {
		if(blocking) {
			//Esto impide que se calculen valores sobre celdas que no estan computadas
			bool blocked = true;
			while (blocked) {
				blocked = !subrogate_board_is_set(subrogateBoard, columns, j - 1)
				          || !subrogate_board_is_set(subrogateBoard, columns, j)
				          || !subrogate_board_is_set(subrogateBoard, columns, j + 1);
			}
		} else {
			if(!subrogate_board_is_set(subrogateBoard, columns, j - 1)
			   || !subrogate_board_is_set(subrogateBoard, columns, j)
			   || !subrogate_board_is_set(subrogateBoard, columns, j + 1)) {
				return j;
			}
		}

		processRight(finalBoard, subrogateBoard, j);
	}

	return rows;
}

#define MIN_AREA 256

typedef struct {
	board_sub_t section_to_read;
	board_t board_write_only;
	size_t cycles;
	list_t *list;
} data_child_t;

/**
 * Calcula game of life en un rectrangulo arbitrario
 */
static void *process(void * data) {
	data_child_t *dataChild = data;
	list_t *list = dataChild->list;

	for (int c = 0; c < dataChild->cycles; ++c) {
		dataChild->board_write_only = list_get(list, c + 1);
		dataChild->section_to_read.board = list_get(list, c);

		board_sub_t subrogateBoard = dataChild->section_to_read;
		board_t finalBoard = dataChild->board_write_only;
		size_t leftLastProcessed = processLeftBorderBlocking(finalBoard, subrogateBoard, 0, false);
		size_t rightLastProcessed = processRightBorderBlocking(finalBoard, subrogateBoard, 0, false);

		processCenter(finalBoard, subrogateBoard);

		processLeftBorderBlocking(finalBoard, subrogateBoard, leftLastProcessed, true);
		processRightBorderBlocking(finalBoard, subrogateBoard, rightLastProcessed, true);

		list_free_cycle(list, c);
	}

	pthread_exit(NULL);
}

board_t *congwayGoL(board_t *board, unsigned int cycles, const int nuproc) {
	size_t len;
	board_sub_t *subrogates = divide_into_subrogates(&len, *board, nuproc, MIN_AREA);
	pthread_t *tid = calloc(len, sizeof(pthread_t));
	data_child_t *args = calloc(len, sizeof(data_child_t));
	list_t *list = malloc(sizeof(list_t));

	*list = list_init(len, *board);

	for (int j = 0; j < len; ++j) {
		args[j].section_to_read = subrogates[j];
		args[j].cycles = cycles;
		args[j].list = list;

		pthread_create(&(tid[j]), NULL, process, &(args[j]));
	}

	for (int j = 0; j < len; ++j) {
		pthread_join(tid[j], NULL);
	}

	board_t *finalBoard = malloc(sizeof(board_t));
	*finalBoard = list_get(list, cycles);

	list_destroy(*list);
	free(list);
	free(args);
	free(tid);
	subrogates_destroy(subrogates);

	return finalBoard;
}

