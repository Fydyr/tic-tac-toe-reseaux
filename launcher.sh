#!/bin/bash

gcc -c include/socket_management.c -Wall -I./include 
gcc -c server.c -Wall -I./include  
gcc -o server server.o socket_management.o -Wall -I./include
gcc -c client.c -Wall -I./include  
gcc -o client client.o socket_management.o -Wall -I./include