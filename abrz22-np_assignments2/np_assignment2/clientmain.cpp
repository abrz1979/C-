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
#include "protocol.h"
#include "poll.h"

#define DEBUG 1
#define PROTOCHOL_STATE 1
#define CALCULATION_STATE 2
#define MAX_RES_LEN 100
#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4
#define FADD 5
#define FSUB 6
#define FDIV 8
#define FMUL 7

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



int main(int argc, char *argv[])
{
       int sockfd = 0;
       int n = 0;
       char recvBuff[1024];
       struct addrinfo hints, *res, *p;
       int status;
       char s[INET6_ADDRSTRLEN];
       // Check the number of arguments
       if (argc != 2)
       {
              printf("\n Usage: %s <ip of server>\n", argv[0]);
              return 1;
       }

       // Making Ip and port seperated
       char delim[] = ":";
       char *Desthost = strtok(argv[1], delim);
       char *Destport = strtok(NULL, delim);

       /* Do magic chage string to int*/
       int port = atoi(Destport);
       printf("Host %s, and port %d.\n", Desthost, port);

       // Clear buffer
       memset(recvBuff, '0', sizeof(recvBuff));
       // Creating Socket

       memset(&hints, 0, sizeof hints);
       hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
       hints.ai_socktype = SOCK_DGRAM;

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




       // Make connection to server

if (connect(sockfd, p->ai_addr,p->ai_addrlen ) < 0)
       {
              printf("\n Error : Connect Failed \n");
              return 1;
       }

// Geetting port and Ip address peer
inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
		  s, sizeof s);
 printf("client: connecting to %s\n", s);

freeaddrinfo(res);

#ifdef DEBUG
       //printf("connected to %s:%d  and local port %s:%u\n", Desthost, port, myIP, sin.sin_port);
#endif

       // Structure  of calcMessage as Protocol
       struct calcMessage message = {0};
       message.type = htons(22);
       message.message = htonl(0);
       message.protocol = htons(17);
       message.major_version = htons(1);
       message.minor_version = htons(0);
       // Sending the first message
       n = send(sockfd, &message, sizeof(message), 0);
       if (n < 0)
       {
              printf("\n Error In Sending calcMessage To Server\n");
              return 0;
       }
     
     //This is for making pause for testing
// printf("For Pausing\n"); 
// char str[20];
// scanf("%s",str);

       // Polling mechanism for 2s timeout maximom 2 times
       struct pollfd pfd[1];
       pfd[0].fd = sockfd;
       pfd[0].events = POLLIN;

       int ret = poll(pfd, 1, 2000);
       if (ret == 0)
       {
              printf("P1 poll timed out in first try, trying second ...\n");
              n = send(sockfd, &message, sizeof(message),0);
                   if (n < 0)
                     {
                       printf("\n P1 Error In Sending calcMessage To Server\n");
                      return 0;
                       }
              ret = poll(pfd, 1, 2000);
                 if (ret == 0)
                  {
                     printf("P1 poll timed out in second try, trying! ...\n");
                     n = send(sockfd, &message, sizeof(message),0);
                      if (n < 0)
                        {
                         printf("\nP1 Error In Sending calcMessage To Server\n");
                         return 0;
                          }
                         ret = poll(pfd, 1, 2000);
                            if (ret == 0)
                           {
                              printf("P1 poll timed out in second try, Exit! ...\n");
                              return 0;
                            }
                   }
              if (ret < 0)
                   {
                     printf("P1 Error in second polling!! exiting ...\n");
                     return 0;
                   }
            }
         if (ret < 0)
                   {
                     printf("Error in second polling!! exiting ...\n");
                     return 0;
                   }

       /// End of polling mechanism, if didn't return and program is here, then server has replied, so klet's read the response

 //Struct calcProtocol
       struct calcProtocol prot = {0};
       // Receiving Calcprotocol
       n = recv(sockfd, &prot, sizeof(prot), 0);
     
       if (n < 0)
       {
              printf("\n Error In Reading Prot Message From Server, n=%d bytes read, %d expected\n", n, 12);
              return 0;
       }

       
       
       // For My debug
       //     printf("\nn=%d bytes read\n",n);
    if (n == sizeof(calcMessage))
    {
       //This for checking if the server not suuport client
       memcpy(&message, &prot, sizeof(message));

      // printf("The Message size is %d",strlen(prot));
       //    printf("\nRCalc Message:\n");
       //    printf("\nRtype %d",ntohs(message.type));
       //    printf("\nRprot %d",ntohs(message.protocol));
       //    printf("\nRmessage %d",ntohl(message.message));
       //    printf("\nRmajor_version %d",ntohs(message.major_version));
       //    printf("\nRminor_version %d\n",ntohs(message.minor_version));

       // checking  unsupprted protocol
       if (ntohs(message.type) == 2 && ntohl(message.message) == 2 && ntohs(message.major_version) == 1 && ntohs(message.minor_version) == 0)
       {
              printf("\nNOT OK\n");
              return 0;
       }
    }
       // For my debug

           printf("\nServer Protocol:\n");
           printf("\ntype %d",ntohs(prot.type));
           printf("\nmajor_version %d",ntohs(prot.major_version));
           printf("\nminor_version %d\n",ntohs(prot.minor_version));
           printf("\nid %d",ntohl(prot.id));
           printf("\narith %d",ntohl(prot.arith));
           printf("\ninValue1 %d",ntohl(prot.inValue1));
           printf("\ninValue2 %d",ntohl(prot.inValue2));
           printf("\ninResult %d",ntohl(prot.inResult));
           printf("\nflValue1 %f",prot.flValue1);
           printf("\nflValue2 %f",prot.flValue2);
           printf("\nflResult %d\n",ntohl(prot.flResult));

       switch (ntohl(prot.arith))
       {
       case ADD:
              prot.inResult = htonl(ntohl(prot.inValue1) + ntohl(prot.inValue2));
              printf("ASSIGMNET: %d add %d \n", ntohl(prot.inValue1), ntohl(prot.inValue2));
#ifdef DEBUG
              printf("Calculated the result to %d\n", ntohl(prot.inResult));
#endif
              break;

       case SUB:
              prot.inResult = htonl(ntohl(prot.inValue1) - ntohl(prot.inValue2));
              printf("ASSIGMNET: %d sub %d \n", ntohl(prot.inValue1), ntohl(prot.inValue2));
#ifdef DEBUG
              printf("Calculated the result to %d\n", ntohl(prot.inResult));
#endif
              break;
       case DIV:
              prot.inResult = htonl(ntohl(prot.inValue1) / ntohl(prot.inValue2));
              printf("ASSIGMNET: %d div %d \n", ntohl(prot.inValue1), ntohl(prot.inValue2));
#ifdef DEBUG
              printf("Calculated the result to %d\n", ntohl(prot.inResult));
#endif
              break;
       case MUL:
              prot.inResult = htonl(ntohl(prot.inValue1) * ntohl(prot.inValue2));
              printf("ASSIGMNET:%d mul %d \n", ntohl(prot.inValue1), ntohl(prot.inValue2));
#ifdef DEBUG
              printf("Calculated the result to %d\n", ntohl(prot.inResult));
#endif
              break;
       case FADD:
              prot.flResult = prot.flValue1 + prot.flValue2;
              printf("ASSIGMNET:%f fadd %f \n", prot.flValue1, prot.flValue2);
#ifdef DEBUG
              printf("Calculated the result to %f\n", prot.flResult);
#endif
              break;
       case FSUB:
              prot.flResult = prot.flValue1 - prot.flValue2;
              printf("ASSIGMNET:%f fsub %f \n", prot.flValue1, prot.flValue2);
#ifdef DEBUG
              printf("Calculated the result to %f\n", prot.flResult);
#endif
              break;
       case FMUL:
              prot.flResult = prot.flValue1 * prot.flValue2;
              printf("ASSIGMNET:%f fmul %f \n", prot.flValue1, prot.flValue2);
#ifdef DEBUG
              printf("Calculated the result to %f\n", prot.flResult);
#endif
              break;
       case FDIV:
              prot.flResult = prot.flValue1 / prot.flValue2;
              printf("ASSIGMNET:%f fdiv %f \n", prot.flValue1, prot.flValue2);
#ifdef DEBUG
              printf("Calculated the result to %f\n", prot.flResult);
#endif
              break;
       }

        prot.type = htons(2);
        prot.major_version = htons(1);
        prot.minor_version = htons(0);

//This is for making pause for testing
//  printf("For Pausing\n"); 
//  char str[20];
//  scanf("%s",str);
     
       if (send(sockfd, &prot, sizeof(prot), 0) == -1)
       {
              perror("sendto");
              exit(1);
        }


  ret = poll(pfd, 1, 2000);
       if (ret == 0)
       {
              printf("P2 poll timed out in first try, trying second ...\n");
              n = send(sockfd, &prot, sizeof(prot),0);
                   if (n < 0)
                     {
                       printf("\n P2 Error In Sending calcMessage To Server\n");
                      return 0;
                       }
              ret = poll(pfd, 1, 2000);
                 if (ret == 0)
                  {
                     printf("P2 poll timed out in second try, trying! ...\n");
                     n = send(sockfd, &prot, sizeof(prot),0);
                      if (n < 0)
                        {
                         printf("\nP2 Error In Sending calcMessage To Server\n");
                         return 0;
                          }
                         ret = poll(pfd, 1, 2000);
                            if (ret == 0)
                           {
                              printf("P2 poll timed out in second try, Exit! ...\n");
                             return 0;
                            }
                   }
              if (ret < 0)
                   {
                     printf("P2 Error in second polling!! exiting ...\n");
                     return 0;
                   }
            }
         if (ret < 0)
                   {
                     printf("Error in second polling!! exiting ...\n");
                     return 0;
                   }


       n = recv(sockfd, &message, sizeof(message), 0);
       if (n < 0)
       {
              printf("\n Error In Reading Prot Message From Server, n=%d bytes read, %d expected\n", n, 12);
              return 0;
       }
        
     if (ntohs(message.type) == 2 && ntohl(message.message) == 2 && ntohs(message.major_version) == 1 && ntohs(message.minor_version) == 0)
       {
              printf("\nNOT OK\n");
              return 0;
       }


       // For my Debug
       //      printf("\nn=%d bytes read\n",n);
       //      printf("\nCalc Message:\n");
       //      printf("\ntype %d",ntohs(message.type));
       //      printf("\nprot %d",ntohs(message.protocol));
       //      printf("\nmessage %d\n",ntohl(message.message));
       //      printf("\nmajor_version %d",ntohs(message.major_version));
       //      printf("\nminor_version %d\n",ntohs(message.minor_version));
       if (ntohl(message.message) == 1)
       {
              if (ntohl(prot.arith) == ADD || ntohl(prot.arith) == MUL || ntohl(prot.arith) == DIV || ntohl(prot.arith) == SUB)
              {
                     printf("OK (my result is %d ) \n", ntohl(prot.inResult));
              }
              else
              {
                     printf("OK (my result is %f ) \n", prot.flResult);
              }
       }
       else
       {
              printf("NOK\n");
       }

       return 0;
}
