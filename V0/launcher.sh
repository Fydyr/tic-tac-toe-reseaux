#!/bin/bash

gcc -c include/socket_management.c -Wall -I./include 
gcc -c T3N_server_V0.c -Wall -I./include  
gcc -o server T3N_server_V0.o socket_management.o -Wall -I./include
gcc -c T3N_client_V0.c -Wall -I./include  
gcc -o client T3N_client_V0.o socket_management.o -Wall -I./include
gnome-terminal -- bash -c "ls; exec bash"
gnome-terminal -- bash -c "./server; exec bash"
gnome-terminal -- bash -c "./client; exec bash"
