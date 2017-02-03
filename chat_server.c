/*
  File: chat_client.c

  Description: TCP chatroom server.
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define SERVER getenv("MYIP")
#define BUFSIZE 1024
#define PORT 3000
#define LISTENQ 8 /* max number of client connections */

/* TODO: Keep track of sockets more elegantly. */
int connections[1000]; // keeps track of sockets
int i = 0; // keep track of connections array last element

struct thread_data {
  int connfd;
  char buf[BUFSIZE];
};

void* respondToClient(void *threadarg) {
  int recvlen;
  char sendstring[2*BUFSIZE];
  char *username = malloc(BUFSIZE * sizeof(char));
  struct thread_data *data;

  data = (struct thread_data *) threadarg;

  /* TODO: Use recvmsg() instead */
  if ((recv(data->connfd, username, BUFSIZE, 0)) > 0) {
    //printf("Strlen of username: %d\n", strlen(username));
    printf("User %s has connected. connfd: %d\n", username, data->connfd);
  }
  // Deal with messages
  while ((recvlen = recv(data->connfd, data->buf, BUFSIZE, 0)) > 0) {
    data->buf[recvlen] = '\0';
    //printf("User %s sent: %s\n", username ,data->buf);
    //send(data->connfd, data->buf, strlen(data->buf), 0);
    for(int j = 0; j <= i; j++) {
      // Send message to other users, but don't send back to originator
      if(connections[j] != -1 && connections[j] != data->connfd && connections[j] != 0) {

        /*TODO: Keep a map of connfds to usernames. Use the length of username
        to set appropriate spaces after carriage return so that it is properly
        erased. */
        strcpy(sendstring, "\r");
        strcat(sendstring, username);
        strcat(sendstring, ": ");
        strcat(sendstring, data->buf);
        printf("%s", sendstring);
        if (send(connections[j], sendstring, strlen(sendstring), 0) < 0)
          perror("error on send");
      }
    }
  }

  if (close(data->connfd) < 0) {
    perror("Error on closing sonnection socket");
  }
  else { // On close
    printf("User %s has disconnected. connfd: %d\n", username, data->connfd);
    for(int j = 0; j<= i; j++) {
      if (connections[j] == data->connfd) {
        connections[j] = -1;
        break;
      }
    }
    free(threadarg);
    free(username);

    pthread_detach(pthread_self());
  }
  return NULL;
}

int main(int argc, char **argv) {
  int listenfd, connfd;
  socklen_t clientlen;
  struct sockaddr_in clientaddr, serveraddr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  memset((char *)&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(PORT);

  bind(listenfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
  listen(listenfd, LISTENQ);

  printf("Server running... Waiting for connections.\n");

  while (1) {
    pthread_t thread;
    struct thread_data *response_data = malloc(sizeof(struct thread_data));
    clientlen = sizeof(clientaddr);
    connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
    if (connfd < 0) {
      perror("Error on accept");
    }
    connections[i++] = connfd;

    response_data->connfd = connfd;

    pthread_create(&thread, NULL, respondToClient, response_data);
  }
}
