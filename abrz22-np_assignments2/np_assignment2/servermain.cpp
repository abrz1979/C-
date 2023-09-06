
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include "protocol.h"
#include "poll.h"
#include <calcLib.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>
#include <time.h>

#define MAXBUFLEN 1450

//Structure for CLient
struct clientId
{
  char *ip;
  int port;
  int die;
  float id;
  float fr;
  unsigned int ir;
  time_t stime;

};

struct clientId clId[100] = {0};

int pfind(int id) // NOT REMOVED
{
  for (int i = 0; i < 100; i++)
  {
    struct clientId client = clId[i];

    if (client.id == id)
      //if (client.die != 1)
      {
        return clId[i].port;
      }
  }
printf("Port not found");
  return 1;
}

unsigned int find(int id) // NOT REMOVED
{
  for (int i = 0; i < 100; i++)
  {
    struct clientId client = clId[i];

    if (client.id == id)
     // if (client.die != 1)
      {
        return clId[i].ir;
      }
  }
printf("Int not found\n");
  return 1;
}



float ffind(int id) // NOT REMOVED
{
  for (int i = 0; i < 100; i++)
  {
    struct clientId client = clId[i];

    if (client.id == id)
     // if (client.die != 1)
      {
        return clId[i].fr;
      }
  }
printf("Float not found\n");
  return 1;
}




// Helper function you can use:

char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
  switch (sa->sa_family)
  {
  case AF_INET:
    inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
              s, maxlen);
    break;

  case AF_INET6:
    inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
              s, maxlen);
    break;

  default:
    strncpy(s, "Unknown AF", maxlen);
    return NULL;
  }

  return s;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}


int sockfd;

int main(int argc, char *argv[])
{

  struct addrinfo hints, *servinfo, *p;
  int rv;
  float id = 1;
  int k =1;
  unsigned int res;
  float fres;
  int numbytes;
  socklen_t addr_len;
  struct sockaddr_storage their_addr;
  struct sockaddr_in *the_addr;
  char recvBuff[1024];
  unsigned int myPort = 0 ;
  initCalcLib();
  char *ptr;
  ptr = randomType();
  double f1, f2, fresult;
  int  i1, i2;
  unsigned int iresult;
  time_t currentTime , start;

//Check Argumnets
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s IP server:Port", argv[0]);
    exit(1);
  }
  char s[INET6_ADDRSTRLEN];
 // char ip[INET6_ADDRSTRLEN];
  // Making Ip and port seperated
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);

  /* Do magic chage string to int*/
  int port = atoi(Destport);
  printf("Host %s, and port %d.\n", Desthost, port);

  // Clear buffer
  memset(recvBuff, '0', sizeof(recvBuff));

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(Desthost, Destport, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
                         p->ai_protocol)) == -1)
    {
      perror("listener: socket");
      exit(1);
    }

    
//Binding
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(sockfd);
      perror("listener: bind");
      continue;
    }

    break;
  }

  if (p == NULL)
  {
    fprintf(stderr, "listener: failed to bind socket\n");
    return 2;
  }

  freeaddrinfo(servinfo);
  

  printf("listener: waiting to recvfrom...\n");
  while (1)
  {

    //printf("The %d computer\n",id);

    printf("Waiting for connection...\n");
    struct calcProtocol prot = {0};
    struct calcMessage message = {0};


    addr_len = sizeof(their_addr);
    numbytes = recvfrom(sockfd, &prot, sizeof(prot), 0, (struct sockaddr *)&their_addr, &addr_len);
    //printf("The  P1 buffersize is %d\n",numbytes);
    
   
    if (numbytes == -1)
    {
      printf("Errno == %d -- %s \n", errno, strerror(errno));
    }

    the_addr = (struct sockaddr_in *)&their_addr;
  //Getting peer address
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    myPort = ntohs(the_addr->sin_port);

    printf("Server: connecting to %s:%u\n", s, myPort);
   

    

    // For My Debug
    //         printf("\nRtype %d",ntohs(message.type));
    //         printf("\nRprot %d",ntohs(message.protocol));
    //         printf("\nRmessage %d",ntohl(message.message));
    //         printf("\nRmajor_version %d",ntohs(message.major_version));
    //         printf("\nRminor_version %d\n",ntohs(message.minor_version));

    

    //Checking the message protocol
    //Check receiving Protocol 
    if (numbytes == sizeof(calcMessage))
    {
      memcpy(&message, &prot, sizeof(message));
      
    if (ntohs(message.type) == 22 && ntohl(message.message) == 0 && ntohs(message.protocol) == 17 && ntohs(message.major_version) == 1 && ntohs(message.minor_version) == 0)

       {
      id ++;
      k++;
    initCalcLib();
    char* ptr;
    ptr = randomType();
    i1 = randomInt();
    i2 = randomInt();
    //  printf("Float\t");
    f1 = randomFloat();
    f2 = randomFloat();

    //printf("the ptr is %s",ptr);
    printf("  Int Values: %d %d \n", i1, i2);
    printf("Float Values: %8.8g %8.8g \n", f1, f2);
    /* Act differently depending on what operator you got, judge type by first char in string. If 'f' then a float */

    if (ptr[0] == 'f')
    {
      /* At this point, ptr holds operator, f1 and f2 the operands. Now we work to determine the reference result. */

      if (strcmp(ptr, "fadd") == 0)
      {
        fresult = f1 + f2;
        prot.arith = htonl(5);
      }
      else if (strcmp(ptr, "fsub") == 0)
      {
        fresult = f1 - f2;
        prot.arith = htonl(6);
      }
      else if (strcmp(ptr, "fmul") == 0)
      {
        fresult = f1 * f2;
        prot.arith = htonl(7);
      }
      else if (strcmp(ptr, "fdiv") == 0)
      {
        fresult = f1 / f2;
        prot.arith = htonl(8);
      }
      printf("%s %8.8g %8.8g = %8.8g\n", ptr, f1, f2, fresult);
    }
    else
    {
      if (strcmp(ptr, "add") == 0)
      {
        iresult = i1 + i2;
        prot.arith = htonl(1);
      }
      else if (strcmp(ptr, "sub") == 0)
      {
        iresult = i1 - i2;
        prot.arith = htonl(2);
      }
      else if (strcmp(ptr, "mul") == 0)
      {
        iresult = i1 * i2;
        prot.arith = htonl(3);
      }
      else if (strcmp(ptr, "div") == 0)
      {
        iresult = i1 / i2;
        prot.arith = htonl(4);
      }

      printf("%s %d %d = %d \n", ptr, i1, i2, iresult);

      printf("Prot id is %d\n", ntohl(prot.id));
    }

      if (ptr[0] == 'f')
      {

        prot.type = htons(1);
        prot.major_version = htons(1);
        prot.minor_version = htons(0);
        prot.flValue1 = f1;
        prot.flValue2 = f2;
        prot.flResult = fresult;
        prot.inValue1 = htons(i1);
        prot.inValue2 = htons(i2);
        prot.id = htonl(id);
        prot.inResult = htonl(iresult);
        clId[k].die = 0; 
        clId[k].port = myPort;
        clId[k].id = id;
        clId[k].fr = fresult;
        clId[k].ir = iresult;
        clId[k].stime = time(&start);
        //printf("the Ftime is %ld\n",time(&start));
    
        
      }
      else
      {
        prot.type = htons(1);
        prot.major_version = htons(1);
        prot.minor_version = htons(0);
        prot.inValue1 = htonl(i1);
        prot.inValue2 = htonl(i2);
        prot.inResult = htonl(iresult);
        prot.flValue1 = f1;
        prot.flValue2 = f2;
        prot.id = htonl(id);
        prot.flResult = fresult;
        clId[k].die = 0; 
        clId[k].port = myPort;
        clId[k].id = id;
        clId[k].ir = iresult;
        clId[k].fr = fresult;
        clId[k].stime = time(&start);
        //printf("the Ftime is %ld\n",time(&start));
      }

      numbytes = sendto(sockfd, &prot, sizeof(prot), 0, (struct sockaddr *)&their_addr, sizeof(their_addr));
      if (numbytes < 0)
      {
        printf("\nP1  Error In Sending ");
        continue;
      }
    }
    else
    //If the protocol is not supported by Server
  
    {
      printf("The protocol not supported\n");
      message.type = htons(2);
      message.message = htonl(2);
      message.protocol = htons(17);
      message.major_version = htons(1);
      message.minor_version = htons(0);

      numbytes = sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&their_addr, sizeof(their_addr));
      if (numbytes < 0)
      {
        printf("\nP1  Error In Sending ");
        continue;
      }
    }
  // Polling mechanism for 10s
   
    struct pollfd pfd[1];
    pfd[0].fd = sockfd;
    pfd[0].events = POLLIN;
    int ret = poll(pfd, 1, 10000);
    if (ret == 0)
    {
      printf("P1 Time out....\n");
      clId[k].die = 1 ;
      continue;
    }
    else if (ret < 0)
    {
      printf("Error in connection !!!");
    }
   
  }

  //Checking Protocol Message
  else
  {
   printf("it is not calcmessage\n");

  
  res = find(ntohl(prot.id));
  fres = ffind(ntohl(prot.id));
    
  //printf("The last is %ld\n",time(&currentTime));
  //printf("The last is %ld\n",clId[ntohl(prot.id)].stime);

  double diff_time = double (time(&currentTime) - clId[ntohl(prot.id)].stime);

 // printf("The tOTALL left is %f\n",diff_time);
  if (diff_time >= 10)
  {
    clId[ntohl(prot.id)].die =1;
  }


 if (((long)(prot.flResult*100) == (long)(fres*100)) || (ntohl(prot.inResult) == res))  // ghabli faghat decimal part bood hala baiad ta 2 ragham ashar ham dar nazar begire
    {
      printf("The result is Match\n");
      printf("the dead is %d\n",clId[ntohl(prot.id)].die);
      if ((clId[ntohl(prot.id)].id == ntohl(prot.id)) && (clId[ntohl(prot.id)].die == 0) && (clId[ntohl(prot.id)].port = myPort))
      {
        printf("it is ok , ID id %d\n", ntohl(prot.id));
        message.message = htonl(1);
        numbytes = sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&their_addr, sizeof(their_addr));
        if (numbytes < 0)
        {
          printf("\nP3  Error In Sending ");
          continue;
        }
      }
      else
      {
       message.type = htons(2);
       message.message = htonl(2);
       message.protocol = htons(17);
       message.major_version = htons(1);
       message.minor_version = htons(0);
       numbytes = sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&their_addr, sizeof(their_addr));
        if (numbytes < 0)
        {
          printf("\nP3  Error In Sending ");
          continue;
        }

        printf("ERROR Result\n");
        continue;
      }
    }
    
    else
    {
      printf("The result is not ok\n");
      message.type = htons(2);
      message.message = htonl(2);
      message.protocol = htons(17);
      message.major_version = htons(1);
      message.minor_version = htons(0);
      numbytes = sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&their_addr, sizeof(their_addr));
        if (numbytes < 0)
        {
          printf("\nP3  Error In Sending ");
          continue;
        }
      continue;
    }
    
  }
    


  } // while loop
  close(sockfd);
  return 0;

} // Main
