#!/bin/bash

gcc -fopenmp -c include/socket_management.c -Wall -I./include 
gcc -fopenmp -c include/tictactoe.c -Wall -I./include
gcc -fopenmp -c server.c -Wall -I./include
gcc -fopenmp -o server server.o socket_management.o tictactoe.o -Wall -I./include
gcc -fopenmp -c client.c -Wall -I./include
gcc -fopenmp -o client client.o socket_management.o tictactoe.o -Wall -I./include
rm server.o client.o socket_management.o tictactoe.o