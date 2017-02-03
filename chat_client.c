/*
  File: chat_client.c

  Description: TCP command line chat client that connects to chatroom server.
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER getenv("MYIP")
#define BUFSIZE 1024
#define PORT 3000
#define LOGOUT "!logout"

struct threaddata {
  char username[BUFSIZE];
  char buf[BUFSIZE];
  int sockfd;
};

int session = 1; // Keep track of session state

void* sendToServer(void *sendargs) {
  struct threaddata *data;
  data = (struct threaddata *) sendargs;

  while (strncmp(data->buf, LOGOUT, 7) != 0) {
    /* TODO: Show username at all times. */
    //printf("%s: ", data->username);
    fgets(data->buf, BUFSIZE, stdin);
    send(data->sockfd, data->buf, strlen(data->buf), 0);
  }
  session = 0;
  close(data->sockfd);
  free(sendargs);
  pthread_detach(pthread_self());
}

void* receiveFromServer(void *recvargs) {
  struct threaddata *data;
  data = (struct threaddata *) recvargs;
  char recvline[BUFSIZE];
  int recvlen;

  while (session) {
    recvlen = recv(data->sockfd, recvline, BUFSIZE, 0);
    if (recvlen > 0) {
      recvline[recvlen] = '\0';
      printf("%s", recvline);
    }
  }
  //free(recvargs);
  pthread_detach(pthread_self());
}

int main(int argc, char **argv) {
  struct sockaddr_in serveraddr;
  pthread_t sender, receiver;
  struct threaddata *sendargs = malloc(sizeof(struct threaddata));

  if ((sendargs->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Problem creating socket");
    exit(1);
  }

  memset((char *)&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = inet_addr(SERVER);
  serveraddr.sin_port = htons(PORT);

  if (connect(sendargs->sockfd, &serveraddr, (socklen_t) sizeof(serveraddr)) < 0) {
    perror("Problem connecting to server");
    exit(2);
  }

  printf("Enter your username: ");
  fgets(sendargs->username, BUFSIZE, stdin);
  if (sendargs->username[strlen(sendargs->username) - 1] == '\n') // Remove the newline
    sendargs->username[strlen(sendargs->username)-1] = '\0';
  printf("Connecting to chat server as %s...\n", sendargs->username);


  // TODO: Use sendmsg() instead.
  send(sendargs->sockfd, sendargs->username, strlen(sendargs->username), 0);

  pthread_create(&sender, NULL, sendToServer, sendargs);
  pthread_create(&receiver, NULL, receiveFromServer, sendargs);

  while(session) {
    sleep(1);
  }
  return 1;
}
