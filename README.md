# Morpion - Version 3

<p align="center">
    <img src="./readme/logo.svg" alt="tic-tac-toe-reseaux-banner" width="800">
</p>

## Bibliothèques

- stdio.h
- stdlib.h
- unistd.h
- sys/types.h
- sys/socket.h
- string.h
- netinet/in.h
- arpa/inet.h
- signal.h
-  time.h

## Structure du projet

```sh
└── tic-tac-toe-reseaux/
    ├── Enoncé SAE3.01C3.pdf
    ├── README.md
    ├── readme
    │   └── logo.svg
    ├── SAÉ C3 - cours.pdf
    ├── V1.pdf
    ├── client.c
    ├── include
    │   ├── socket_management.c
    │   ├── socket_management.h
    │   ├── tictactoe.c
    │   └── tictactoe.h
    ├── launcher.bat
    ├── launcher.sh
    └── server.c
```

## Compilation et lancement

Sur **Windows** :

> Veillez à avoir une version de WSL fonctionnelle installée sur l'appareil.

Executer la commande

```cmd
./launcher.bat
```

en cas de problème de port changez les ports dans le launcher/fichier serveur ou utilisez la commandes suivante dans plusieurs terminaux wsl

```cmd
./F3_server_V3 # Si le serveur n'est pas allumé
./F3_client_V3 127.0.0.1 port
```

Pour fermer l'application, fermer le terminal client puis le terminal serveur.

Peu importe si la commande `CTRL+C` est executée.

Sur **Linux** :

Executer les commandes

```bash
./launcher.sh
./server # Terminal 1
./client 127.0.0.1 5000 # Terminal 2
./client 127.0.0.1 5000 # Terminal 3
```
Pour fermer les deux terminaux il faut faire un `CTRL+C` dans le terminal dans lequel vous avez lancer le client cela fermera le second programme en même temps.

Si vous n'arrivez pas a lancer le programme client avec le port aller dans le fichier `server.c` et changer le port avec par exemple `5001` et changer également le port dans la commande dans votre terminal pour lancer le client.

## Auteurs

- Edouard Alluin
- Julien Behani
- Enzo Fournier
- Adrien Mallevaey

**Copyright ©️ Tous droits réservés - 2025**
