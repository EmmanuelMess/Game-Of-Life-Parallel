cmake_minimum_required(VERSION 3.19)
project(entrega_tp_1 C)

set(CMAKE_C_STANDARD 11)

add_executable(entrega_tp_1 main.c Board.h Game.h Board.c Game.c CyclesTable.c CyclesTable.h)
target_link_libraries(entrega_tp_1 pthread)