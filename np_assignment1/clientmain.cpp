
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "calcLib.h"
#include <netdb.h>

#define DEBUG 1
#define MAX_TOKEN 3
#define MAX_RES_LEN 100

// Function that do operations
void caculate_operands(char *ret, char *operation, float op1, float op2)
{
  float result = 0;

#ifdef DEBUG
  printf("\n%s-%f-%f\n", operation, op1, op2);
#endif

  if (!strcmp(operation, "add") || !strcmp(operation, "fadd"))
    result = op1 + op2;
  if (!strcmp(operation, "mul") || !strcmp(operation, "fmul"))
    result = op1 * op2;
  if (!strcmp(operation, "div") || !strcmp(operation, "fdiv"))
    result = op1 / op2;
  if (!strcmp(operation, "sub") || !strcmp(operation, "fsub"))
    result = op1 - op2;

  /*  For my debug
      printf("\nret:%f\n",result);
  */
  sprintf(ret, "%f", result);

  return;
}

int main(int argc, char *argv[])
{
  int sockfd = 0, n = 0;
  char recvBuff[1024];
  struct addrinfo serv_addr;
  char const *supported_prot = "1.0";
  char result[MAX_RES_LEN];
  struct addrinfo hints, *res, *p;
  struct sockaddr_in sin;
  socklen_t len;
  int status;
  char myIP[16];
  //   Check number of argument

  if (argc != 2)
  {
    fprintf(stderr, "usage: showip hostname\n");
    return 1;
  }
  // Seperating IP and port
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);

  /* Do magic chage string to int*/
  int port = atoi(Destport);
  printf("Host %s, and port %d.\n", Desthost, port);
  //Clearing hints
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(Desthost, Destport, &hints, &res)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    printf("status %d", status);
    return 2;
  } 

  for (p = res; p != NULL; p = p->ai_next)
  {
    /* Create the socket */
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      perror("socket");
      continue;
    }

   break;
  }

if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
               }

  freeaddrinfo(res);

  memcpy(&serv_addr, res->ai_addr, sizeof(serv_addr));

  // Make connection to server
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\n Error : Connect Failed \n");
    return 1;
  }

  len = sizeof(struct sockaddr_in);
  if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
  {
    perror("getsockname");
    exit(3);
  }
  inet_ntop(AF_INET, &sin.sin_addr, myIP, sizeof(myIP));
#ifdef DEBUG
  printf("connected to %s:%d  and local port %s:%u\n", Desthost, port, myIP, sin.sin_port);
#endif

  // Check for  Protocol Support
  if ((n = recv(sockfd, recvBuff, sizeof(recvBuff) - 1, 0)) > 0)
  {
    recvBuff[n] = 0;
    if (!strcmp(recvBuff, "ERROR\n"))
    {
      printf("You Got Error!\nBYE\n");
      return 0;
    }



    // Print Received protocol     fputs(recvBuff, stdout);

    char *found_substr = strstr(recvBuff, supported_prot);
    // For my debug
    //  printf("found:%s\n",found_substr);

    if (found_substr != NULL)
    {
      char const *response = "OK\n";
      // Send OK response to the Server
      send(sockfd, response, strlen(response), 0);
    }
  }
  // Receive operation from the server


  if ((n = recv(sockfd, recvBuff, sizeof(recvBuff) - 1, 0)) > 0)
  {
    char *token;
    char tokens[MAX_TOKEN][MAX_RES_LEN];
    recvBuff[n] = 0;
    printf("Assigmnet is  %s", recvBuff);

    // recognize the operation and value
    token = strtok(recvBuff, " ");
    int i = 0;
    while (token != NULL && i < MAX_TOKEN)
    {
      strcpy(tokens[i], token);
      token = strtok(NULL, " ");
      i++;
    }
    char result[MAX_RES_LEN];
    char response[MAX_RES_LEN + 1];
    float o1;
    float o2;
    // Get number to send to calc funtion
    sscanf(tokens[1], "%f", &o1);
    sscanf(tokens[2], "%f", &o2);
    // printf("%f--%f\n",o1,o2);
    caculate_operands(result, tokens[0], o1, o2);
    // For my debug
    //  printf("sending back result of calculation:%s\n",result);

    sprintf(response, "%s\n", result);

    send(sockfd, response, strlen(response), 0);
  }


//char str1[20];

//   printf("Enter name: ");
//   scanf("%19s", str1);

  // receiving  OK from the server
  if ((n = recv(sockfd, recvBuff, sizeof(recvBuff) - 1, 0)) > 0)
  {
    char *token1;
    recvBuff[n] = 0;

    if (!strcmp(recvBuff, "OK\n"))
    {

      token1 = strtok(recvBuff, "\n");
#ifdef DEBUG
      printf("Calculated result to %s", result);
#endif
      printf("%s my result = %s\n", token1, result);
      return 0;
    }
  }

  if (n <= 0)
  {
    printf("\n Read error \n");
  }
  return 0;
}
