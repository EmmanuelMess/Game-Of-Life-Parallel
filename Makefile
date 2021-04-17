all:
	gcc main.c Board.c Board.h Game.c Game.h -o simulador -lpthread -lm
profile:
	gcc main.c Board.c Board.h Game.c Game.h -pg -o simulador -lpthread -lm