#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include "Board.h"
#include "Game.h"

/**
 * Tabla con todas las opciones para las regiones de 3x3
 * table: tablero 3x3 -> celula central
 */
static bool table[512];

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

	if(tableCreate) {
		return;
	}

	//TODO chequear paralelizabilidad
	for (int i = 0; i < 512; ++i) {//i es el tablero de 3x3 en binario
		int t[9];//t es el tablero de 3x3 descomprimido
		for (int j = 0; j < 9; ++j) {//TODO hacer con tabla de bits
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
				table[i] = true;
			} else {
				table[i] = false;
			}
		} else {
			if(aliveAround == 2 || aliveAround == 3) {
				table[i] = true;
			} else {
				table[i] = false;
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
						board_set(board, board_column_index++, i,  line[j] == 'O');
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
			bool state = board_get(board, j, i);
			while (j + 1 < board.columns && state == board_get(board, j + 1, i)) {
				amount++;
				j++;
			}
			fprintf(file, "%zu%c", amount, (state? 'O': 'X'));
		}

		fprintf(file, "\n");
	}

	fclose(file);
}

#define MIN_AREA 256

typedef struct {
	board_sub_t section_to_read;
	board_t board_write_only;
	pthread_barrier_t *barrier_before;
	pthread_barrier_t *barrier_after;
	size_t cycles;
} data_child_t;

/**
 * Calcula game of life en un rectrangulo arbitrario
 */
static void *process(void * data) {
	data_child_t *dataChild = data;

	for (int c = 0; c < dataChild->cycles; ++c) {
		pthread_barrier_wait(dataChild->barrier_before);
		//main
		pthread_barrier_wait(dataChild->barrier_after);

		board_sub_t subrogateBoard = dataChild->section_to_read;
		board_t finalBoard = dataChild->board_write_only;

		size_t columns = subrogateBoard.right - subrogateBoard.left;
		size_t rows = subrogateBoard.bottom - subrogateBoard.top;

		for (int i = 0; i < columns; ++i) {
			for (int j = 0; j < rows; ++j) {
				int indexToShift = 8;
				int x = 0;
				for (int k = -1; k <= 1; ++k) {
					for (int l = -1; l <= 1; ++l) {
						bool valueInBoard = board_sub_get(subrogateBoard, i + k, j + l);
						x |= (valueInBoard ? 1 : 0) << indexToShift;
						indexToShift--;
					}
				}

				size_t absolutePositionCol = subrogateBoard.left + i;
				size_t absolutePositionRow = subrogateBoard.top + j;

				board_set(finalBoard, absolutePositionCol, absolutePositionRow, table[x]);
			}
		}
	}

	pthread_exit(NULL);
}

void update(data_child_t *arg, board_t readOnly, board_t writeOnlyBoard) {
	arg->board_write_only = writeOnlyBoard;
	arg->section_to_read.board = readOnly;
}

board_t *congwayGoL(board_t *board, unsigned int cycles, const int nuproc) {
	size_t len;
	board_sub_t *subrogates = divide_into_subrogates(&len, *board, nuproc, MIN_AREA);
	pthread_t *tid = calloc(len, sizeof(pthread_t));
	data_child_t *args = calloc(len, sizeof(data_child_t));

	pthread_barrier_t barrier_before;
	pthread_barrier_t barrier_after;
	pthread_barrier_init(&barrier_before, NULL, len + 1); // +1 por el main thread
	pthread_barrier_init(&barrier_after, NULL, len + 1); // +1 por el main thread

	for (int j = 0; j < len; ++j) {
		args[j].barrier_before = &barrier_before;
		args[j].barrier_after = &barrier_after;
		args[j].section_to_read = subrogates[j];
		args[j].cycles = cycles;

		pthread_create(&(tid[j]), NULL, process, &(args[j]));
	}

	board_t writeOnlyBoard = board_init(board->columns, board->rows);
	board_t readOnlyBoard = *board;

	for (int i = 0; i < cycles; ++i) {
		pthread_barrier_wait(&barrier_before);

		for (int j = 0; j < len; ++j) {
			update(&(args[j]), readOnlyBoard, writeOnlyBoard);
		}

		readOnlyBoard = writeOnlyBoard;
		writeOnlyBoard = board_init(board->columns, board->rows);

		pthread_barrier_wait(&barrier_after);
	}

	for (int j = 0; j < len; ++j) {
		pthread_join(tid[j], NULL);
	}

	*board = readOnlyBoard;

	free(args);
	free(tid);
	subrogates_destroy(subrogates);

	return board;
}

