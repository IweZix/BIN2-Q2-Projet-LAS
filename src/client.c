#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#include "utils_v1.h"
#include "header.h"
#include "socket.h"
#include "tuile.h"

int main(int argc, char const *argv[]) {

  printf("Bienvenue dans le programe d'inscription au serveur de jeu\n");
  printf("Pour participer entrez votre nom :\n");
  StructMessage msg;
  fgets(msg.message, sizeof(msg.message), stdin);
  //msg.message[ret - 1] = '\0';
  msg.message[strlen(msg.message) - 1] = '\0';
  msg.code = INSCRIPTION_REQUEST;
  printf("Votre nom est : %s\n", msg.message);

  int port = atoi(argv[1]);
  int sockfd = initSocketClient(port);
  swrite(sockfd, &msg, sizeof(msg));

  /* wait server response */
  sread(sockfd, &msg, sizeof(msg));

  if (msg.code == INSCRIPTION_OK) {
    printColor("\n%s\n", "Inscription réussie", 32);
  } else {
    printColor("\n%s\n", "Inscription échouée", 31);
  }

  StructClientCommunication tuileCommunication;

  int plateau[20] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  };

  int nbTuiles = 0;

  while (nbTuiles < 20) {
    sread(sockfd, &tuileCommunication, sizeof(tuileCommunication));
    while (tuileCommunication.code != ENVOIE_TUILE) {
      sread(sockfd, &tuileCommunication, sizeof(tuileCommunication));
    }
    
    printColor("\n%s\n", "Voici le plateau actuel", 32);
    printPlateau(plateau);
    printColor("\n%s", "Voici la tuile que vous avez reçu : ", 32);
    printf("%d\n", tuileCommunication.tuile);

    int position;
    printColor("\n%s", "Entrez la position où vous voulez placer la tuile : ", 32);
    scanf("%d", &position);
    position = position-1;

    if (position < 0 || position > 19) {
      printColor("\n%s\n", "Position invalide", 31);
      continue;
    }

    while (plateau[position] != 0) {
      if (position == 19) {
        position = 0;
      } else {
        position++;
      }
    }

    plateau[position] = tuileCommunication.tuile;
    
    nbTuiles++;

    tuileCommunication.emplacement = position;
    tuileCommunication.code = RECEPTION_EMPLACEMENT;

    swrite(sockfd, &tuileCommunication, sizeof(tuileCommunication));
  }

  printColor("\n%s\n", "Partie terminée", 32);

  printColor("\n%s\n", "Voici le plateau final", 32);
  printPlateau(plateau);

  int score = calculeScore(plateau);
  printf("Votre score est de %d\n", score);
  
  swrite(sockfd, &score, sizeof(int));

  Player players[MAX_PLAYER];
  for (int i = 0; i < MAX_PLAYER; i++) {
    sread(sockfd, &players[i], sizeof(Player));
  }
  
  printColor("\n%s\n", "Voici le tableau des scores", 32);
  for (int i = 0; i < MAX_PLAYER; i++) {
      if (players[i].pseudo[0] != '\0' && players[i].score > 0 && players[i].socketfd > 0) {
          printf("%s : %d\n", players[i].pseudo, players[i].score);
      }
  }


  
  close(sockfd);
  return 0;
}
