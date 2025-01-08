@echo off

echo Compilation des fichiers source dans WSL...

wsl gcc -c include/socket_management.c -Wall -I./include

echo Compilation Socket Management : Fait

wsl gcc -c T3N_server_V0.c -Wall -I./include
wsl gcc -o server T3N_server_V0.o socket_management.o -Wall -I./include
wt -w 0 nt wsl ./server

echo Compilation et lancement serveur : Fait

wsl gcc -c T3N_client_V0.c -Wall -I./include
wsl gcc -o client T3N_client_V0.o socket_management.o -Wall -I./include
wt -w 0 nt wsl ./client 127.0.0.1 5000

echo Compilation et lancement client : Fait

pause

del server
del client
del socket_management.o
del T3N_client_V0.o
del T3N_server_V0.o

exit