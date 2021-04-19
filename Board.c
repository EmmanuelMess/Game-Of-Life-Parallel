#include <stdio.h>
#include <stdbool.h>
#include "Board.h"
#include "Game.h"

board_t board_init(const size_t col, const size_t row) {
	board_t board;
	board.columns = col;
	board.rows = row;
	board.data = calloc(col * row, sizeof(atomic_int_fast8_t));
	return board;
}

void board_destroy(board_t board) {
	free(board.data);
}

enum State board_get(const board_t board, unsigned int col, unsigned int row) {
	size_t index = ((row * board.columns) + col);
	return board.data[index];
}

void board_set(board_t board, unsigned int col, unsigned int row, enum State val) {
	size_t index = ((row * board.columns) + col);
	board.data[index] = val;
}

void board_show(board_t board) {
	for (int i = 0; i < board.rows; ++i) {
		for (int j = 0; j < board.columns; ++j) {
			switch (board_get(board, j, i)) {
				case ALIVE:
					printf("O");
					break;
				case DEAD:
					printf("\254");
					break;
				case UNSET:
					printf("·");
					break;
			}
		}
		printf("\n");
	}
}

board_sub_t subrogate_board_init(const board_t source, size_t left, size_t top, size_t right, size_t bottom) {
	board_sub_t boardSub = {
		.board = source,
		.top = top,
		.left = left,
		.bottom = bottom,
		.right = right
	};

	return boardSub;
}

enum State board_sub_get(board_sub_t sub_board, int col, int row) {
	int absCol = (int) sub_board.left + col;
	int absRow = (int) sub_board.top + row;
	if(absCol < 0) {
		absCol = (int) sub_board.board.columns + absCol;
	}
	if(sub_board.board.columns <= absCol) {
		absCol = absCol - (int) sub_board.board.columns;
	}

	if(absRow < 0) {
		absRow = (int) sub_board.board.rows + absRow;
	}
	if(sub_board.board.rows <= absRow) {
		absRow = absRow - (int) sub_board.board.rows;
	}

	return board_get(sub_board.board, absCol, absRow);
}

bool subrogate_board_is_set(const board_sub_t board, int col, int row) {
	return board_sub_get(board, col, row) != UNSET;
}

void board_sub_show(board_sub_t board) {
	size_t columns = board.right - board.left;
	size_t rows = board.bottom - board.top;

	for (int j = 0; j < rows; ++j) {
		for (int i = 0; i < columns; ++i) {
			switch (board_sub_get(board, i, j)) {
				case ALIVE:
					printf("O");
					break;
				case DEAD:
					printf("X");
					break;
				case UNSET:
					printf("·");
					break;
			}
		}
		printf("\n");
	}
}

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

board_sub_t *divide_into_subrogates(size_t * len, const board_t board, const size_t nuproc, const size_t minArea) {
	size_t columns = board.columns;
	size_t rows = board.rows;
	size_t nareas = (columns * rows) / minArea;
	size_t ncolumns = columns / 8; //Evita que las columnas sean de menos de una linea de cache estandar (64 bytes)
	size_t nthreads = MIN(nareas, MIN(ncolumns, nuproc));

	board_sub_t *subboards;

	if(nthreads <= 1) {
		*len = 1;

		subboards = calloc(1, sizeof(board_sub_t));
		subboards[0] = subrogate_board_init(board, 0, 0, columns, rows);

		return subboards;
	}

	subboards = calloc(nthreads, sizeof(board_sub_t));
	*len = nthreads;

	size_t colPerThread = columns / nthreads;

	for (int i = 0; i < nthreads - 1; ++i) {
		subboards[i] = subrogate_board_init(board, i * colPerThread, 0, (i + 1) * colPerThread, rows);
	}

	subboards[nthreads - 1] = subrogate_board_init(board, (nthreads - 1) * colPerThread, 0, columns, rows);

	return subboards;
}

void subrogates_destroy(board_sub_t * subrogates) {
	free(subrogates);
}