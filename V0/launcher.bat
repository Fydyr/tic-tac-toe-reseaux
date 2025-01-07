@echo off

echo Compilation des fichiers source dans WSL...

wsl gcc -c include/socket_management.c -Wall -I./include

wsl gcc -c T3N_server_V0.c -Wall -I./include
wsl gcc -o server T3N_server_V0.o socket_management.o -Wall -I./include
wt -w 0 nt wsl ./server

wsl gcc -c T3N_client_V0.c -Wall -I./include
wsl gcc -o client T3N_client_V0.o socket_management.o -Wall -I./include
wt -w 0 nt wsl ./client 127.0.0.1 5000

echo Lancement du client dans WSL...

pause
