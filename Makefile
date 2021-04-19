all:
	gcc main.c Board.c Board.h Game.c Game.h CyclesTable.c CyclesTable.h -o simulador -lpthread -lm