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
#include <sys/socket.h>

#include "utils_v1.h"
#include "header.h"

/**
 * Create a socket server
*/
int initSocketServer() {
  int sockfd = ssocket();
  sbind(SERVER_PORT, sockfd);
  slisten(sockfd, BACKLOG);

  printf("%d", sockfd);
  char buffer[200] = "Le serveur tourne sur le port ";
  char port[10];
  sprintf(port, "%d", SERVER_PORT);
  strcat(buffer, port);
  printColor("\n%s\n", buffer, 32);

  return sockfd;
}

/**
 * Handle SIGINT signal
*/
void sigint_handler() {
  printColor("\n%s\n","Server is shutting down by SIGINT...", 33);
  exit(0);
}

volatile sig_atomic_t end = 0;

/**
 * Handle SIGALRM signal
*/
void sigalrm_handler(int sig) {
  printColor("\n%s\n","La période d'inscription est terminée", 33);
  end = 1;
}


int main(int argc, char const *argv[]) {

  /**
   * Initialisation server
  */
  ssigaction(SIGINT, sigint_handler);
  ssigaction(SIGALRM, sigalrm_handler);

  // Create one semaphore to store the actual piece
  int sem_id = sem_create(SEM_KEY, 2, PERM, 1);

  // Create shared memory
  int shm_id = sshmget(KEY, MAX_PLAYER*sizeof(Player), IPC_CREAT | PERM);
  // Attach shared memory
  Player *shared_memory = sshmat(shm_id);

  printf("%d", sem_id);
  
  int sockfd = initSocketServer();
  printf("%d", sockfd);

  StructMessage msg;
  Player players[MAX_PLAYER];
  int nbPlayer = 0;
  
  struct pollfd fds[MAX_PLAYER];

  /**
   * Inscription
  */
  alarm(30);
  while (!end) {

    // accept connection
    int newsockfd = accept(sockfd, NULL, NULL);
    if (newsockfd > 0) {
      // read pseudo
      sread(newsockfd, &msg, sizeof(int));
      // stdout pseudo
      char buffer[200] = "Inscription demandée par le joueur ";
      strcat(buffer, msg.message);
      printColor("\n%s\n", buffer, 32);

      if (nbPlayer < MAX_PLAYER) {
        strcpy(players[nbPlayer].pseudo, msg.message);
        players[nbPlayer].socketfd = newsockfd;
        players[nbPlayer].score = 0;

        shared_memory[nbPlayer] = players[nbPlayer];

        fds[nbPlayer].fd = newsockfd;
        fds[nbPlayer].events = POLLIN;

        nbPlayer++;

        msg.code = INSCRIPTION_OK;
        swrite(newsockfd, &msg, sizeof(msg));

        printColor("\n%s\n", "Inscription réussie", 32);
      } else {
        msg.code = INSCRIPTION_KO;
        swrite(newsockfd, &msg, sizeof(msg));

        printColor("\n%s\n", "Inscription échouée", 31);
      }
      
      printf("Nb Inscriptions : %i\n", nbPlayer);
    }
  }

  if (nbPlayer < 2) {
    printColor("\n%s\n", "Il n'y a pas assez de joueurs pour lancer la partie", 31);
    // Detach shared memory
    sshmdt(shared_memory);
    // Destroy shared memory
    sshmdelete(shm_id);
    // Destroy semaphore
    sem_delete(sem_id);
    return 0;
  }
  

  for (int i = 0; i < nbPlayer; i++) {
    printf("%d\n", fds[i].fd);
  }

  /**
   * Génération des tuiles
  */
  int tuilesBase[31] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
  };

  int tuilesRestantes[31] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
  };

  int tuiles[20];

  for (int i = 0; i < 20; i++) {
    int random = randomIntBetween(0, 30);
    while (tuilesRestantes[random] == 0) {
      random = randomIntBetween(0, 30);
    }
    tuiles[i] = tuilesBase[random];
    tuilesRestantes[random]--;    
  }

  for (int i = 0; i < 20; i++) {
    printf("ici : %d\n", i);
    printf("%d\n", tuiles[i]);
  }

  

  
  



  // Detach shared memory
  sshmdt(shared_memory);
  // Destroy shared memory
  sshmdelete(shm_id);
  // Destroy semaphore
  sem_delete(sem_id);
  return 0;
}