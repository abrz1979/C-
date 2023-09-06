#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <netdb.h>
#include "poll.h"
#include <calcLib.h>

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{

  int connfd;
  char recvBuff[1024];
  char sendBuff[1024];
  int sockfd; // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;
  char const *reply = "ERROR\n";
  // Check number of argumnets
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

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  // Getting address information
  if ((rv = getaddrinfo(NULL, Destport, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  else
  {
    printf("the getaddress is ok ");
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
                         p->ai_protocol)) == -1)
    {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                   sizeof(int)) == -1)
    {
      perror("setsockopt");
      exit(1);
    }
    // Binding start
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(sockfd);
      perror("server: bind");
      exit(1);
    }

    break;
  }

  freeaddrinfo(servinfo); // all done with this structure
  if (p == NULL)
  {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  // Listening start
  if (listen(sockfd, 5) == -1)
  {
    perror("listen");
    close(sockfd);
    exit(1);
  }

  memset(sendBuff, '0', sizeof(sendBuff));
  printf("Entering loop....\r\n");
  while (1)
  {
    sin_size = sizeof(their_addr);
    printf("Accepting...\r\n");
    connfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (connfd == -1)
    {
      perror("accept");
      continue;
    }

    // Getting peer ip address
    inet_ntop(their_addr.ss_family,
              get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof s);

    printf("Accepted...\r\n\r\n\r\n");

    printf("server: Connection  from %s\n", s);

    strcpy(sendBuff, "TEXT TCP 1.0\r\n");
    if (send(connfd, sendBuff, strlen(sendBuff), 0) < 0)
    {
      printf("Send Error");
      exit(0);
    }
    // Polling mechanism for 5s
    struct pollfd pfd[1];
    pfd[0].fd = connfd;
    pfd[0].events = POLLIN;
    int ret = poll(pfd, 1, 5000);
    if (ret == 0)
    {
      printf("P1 Time out....\n");
      if (send(connfd, reply, strlen(sendBuff), 0) < 0)
      {
        printf("Send Error");
        exit(0);
      }

      close(connfd);
    }
    else if (ret < 0)
    {
      printf("Error in connection !!!");
    }

    // Clear Memory
    memset(recvBuff, 0, sizeof(recvBuff));

    // creating Random Number and result

    initCalcLib();
    char *ptr;
    ptr = randomType();
    double f1, f2, fresult;
    int iresult, i1, i2;
    i1 = randomInt();
    i2 = randomInt();
    //  printf("Float\t");
    f1 = randomFloat();
    f2 = randomFloat();
    printf("  Int Values: %d %d \n", i1, i2);
    printf("Float Values: %8.8g %8.8g \n", f1, f2);
    /* Act differently depending on what operator you got, judge type by first char in string. If 'f' then a float */

    if (ptr[0] == 'f')
    {
      /* At this point, ptr holds operator, f1 and f2 the operands. Now we work to determine the reference result. */

      if (strcmp(ptr, "fadd") == 0)
      {
        fresult = f1 + f2;
      }
      else if (strcmp(ptr, "fsub") == 0)
      {
        fresult = f1 - f2;
      }
      else if (strcmp(ptr, "fmul") == 0)
      {
        fresult = f1 * f2;
      }
      else if (strcmp(ptr, "fdiv") == 0)
      {
        fresult = f1 / f2;
      }
      printf("%s %8.8g %8.8g = %8.8g\n", ptr, f1, f2, fresult);
    }
    else
    {
      if (strcmp(ptr, "add") == 0)
      {
        iresult = i1 + i2;
      }
      else if (strcmp(ptr, "sub") == 0)
      {
        iresult = i1 - i2;
      }
      else if (strcmp(ptr, "mul") == 0)
      {
        iresult = i1 * i2;
      }
      else if (strcmp(ptr, "div") == 0)
      {
        iresult = i1 / i2;
      }

      printf("%s %d %d = %d \n", ptr, i1, i2, iresult);
    }

    // Receive Ok from client
    int n = recv(connfd, recvBuff, sizeof(recvBuff), 0);
    if (n < 0)
    {
      printf("Receive Error");
      exit(0);
    }

    // Sending random numbers
    if (n > 0)
    {
      recvBuff[n] = 0;
      printf("Received: %s", recvBuff);
      if (strcmp(recvBuff, "OK") >= 0) // before it was strstr
      {
        if (ptr[0] == 'f')
        {
          sprintf(sendBuff, "%s %f %f\r\n", ptr, f1, f2);
        }
        else
        {
          sprintf(sendBuff, "%s %d %d\r\n", ptr, i1, i2);
        }
        if (send(connfd, sendBuff, strlen(sendBuff), 0) < 0)
        {
          printf("\n Error In Sending ");
          exit(0);
        }
      }
    }

    pfd[0].fd = connfd;
    pfd[0].events = POLLIN;
    ret = poll(pfd, 1, 5000);
 if (ret == 0)
    {
      printf("P2 Time out....\n");
      if (send(connfd, reply, strlen(sendBuff), 0) < 0)
      {
        printf("Send Error");
      }
      close(connfd);
    }
    else if (ret < 0)
    {
      printf("Error in connection !!!");
    }

    memset(recvBuff, 0, sizeof(recvBuff));
    n = recv(connfd, recvBuff, sizeof(recvBuff), 0);
    if (n > 0)
    {
      recvBuff[n] = 0;
      char strResult[20];

      if (ptr[0] == 'f')
      {
        sprintf(strResult, "%f", fresult);
      }
      else
      {
        sprintf(strResult, "%d", iresult);
      }

      if (!strstr(strResult, recvBuff))
      {
        strcpy(sendBuff, "OK\n");
      }
      else
      {
        sprintf(sendBuff, "ERROR\n%s", strResult);
      }
      if (send(connfd, sendBuff, strlen(sendBuff), 0) < 0)
      {
        printf("Error Sending..!!");
        return (0);
      }
    }
    close(connfd);
    sleep(1);
  }
}
