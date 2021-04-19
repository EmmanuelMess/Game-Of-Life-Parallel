#include <stdio.h>
#include <sys/sysinfo.h>
#include <string.h>
#include "Game.h"
#include "Board.h"

int main(int argc, char *argv[]) {
	if(argc < 2) {
		perror("Falta archivo");
	}

	game_t * game = loadGame(argv[1]);

	board_t *lastGamePointer = congwayGoL(&(game->board), game->cycles, get_nprocs());

	char * finalStateFilename = malloc((strlen(argv[1]) + 2) * sizeof(char ));
	strcpy(finalStateFilename, argv[1]);
	strcpy(strrchr(finalStateFilename, '.') + 1, "final");

	writeBoard(*lastGamePointer, finalStateFilename);

	free(finalStateFilename);

	board_destroy(*lastGamePointer);
	free(lastGamePointer);
	free(game);

	return 0;
}
