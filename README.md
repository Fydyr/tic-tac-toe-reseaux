# Morpion - Version 1

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

## Compilation et lancement

Sur **Windows** :

> Veillez à avoir une version de WSL fonctionnelle installée sur l'appareil.

Executer la commande

```cmd
./launcher.bat
```

Pour fermer l'application, fermer le terminal client puis le terminal serveur.

Peu importe si la commande `CTRL+C` est executée.

Sur **Linux** :

Executer les commandes

```bash
./launcher.sh
./server # Terminal 1
./client 127.0.0.1 5000 # Terminal 2
```
Pour fermer les deux terminaux il faut faire un `CTRL+C` dans le terminal dans lequel vous avez lancer le client cela fermera le second programme en même temps.

Si vous n'arrivez pas a lancer le programme client avec le port aller dans le fichier `server.c` et changer le port avec par exemple `5001` et changer également le port dans la commande dans votre terminal pour lancer le client.

## Auteurs

- Edouard Alluin
- Julien Behani
- Enzo Fournier
- Adrien Mallevaey

**Copyright ©️ Tous droits réservés - 2025**
