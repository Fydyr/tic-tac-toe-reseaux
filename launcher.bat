@echo off

echo Compilation des fichiers source dans WSL...

wsl gcc -fopenmp -c include/socket_management.c -Wall -I./include
wsl gcc -c include/tictactoe.c -Wall -I./include

echo Compilation fichiers 'include' : Fait

wsl gcc -fopenmp -c server.c -Wall -I./include
wsl gcc -fopenmp -o server server.o socket_management.o tictactoe.o -Wall -I./include
wt -w 0 nt wsl ./server

echo Compilation et lancement serveur : Fait

wsl gcc -fopenmp -c client.c -Wall -I./include
wsl gcc -fopenmp -o client client.o socket_management.o tictactoe.o -Wall -I./include
wt -w 0 nt wsl ./client 127.0.0.1 5000
wt -w 0 nt wsl ./client 127.0.0.1 5000

echo Compilation et lancement client : Fait

pause

del server
del client
del socket_management.o
del tictactoe.o
del client.o
del server.o

exit