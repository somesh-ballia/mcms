
//+===============================================================================+
//                         NTPBypassClient.c					  |
//										  |
// This Utility is designed to request time data from switch to during startup    |
// as a bypass to NTP server which is not ready at this phase.                    |
//+===============================================================================+

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ctype.h>
#include <string.h>
#include <arpa/inet.h>


#define BUFSIZE 100
#define PORT 10004
  
int main()
{
     int rx, tx, err;
     struct sockaddr_in stream_addr;
     int read_len, write_len;
     
     struct timespec tv;

     char buf[BUFSIZE] = "";
     char *tmp;

     int asyncio = 0;	//0 = sync , non-zero = async

     //Create socket to server
     if((tx = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
     {
       exit(1);
     }

     //Non-Blocking,sync
     //ioctl(tx, FIONBIO, &asyncio);
	
     bzero((char*)&stream_addr, sizeof(stream_addr));
     stream_addr.sin_family = AF_INET;
     stream_addr.sin_addr.s_addr = inet_addr("169.254.128.16");
     stream_addr.sin_port = htons(PORT);
     
     err = connect(tx,(struct sockaddr *) &stream_addr, sizeof(struct sockaddr_in));
     if(err<0)
     {
       exit(2);
     }

     memset(buf, 0, BUFSIZE);
     strncpy(buf, "GET_TIME", 8);
     
     printf("NTPBypassClient Connected to NTPBypassServer...\n");
     //First, send request (opcode is "GET_TIME")
     if(write(tx, buf, strlen(buf)) != 8)
     {
	 exit(3);
     }
     
     //Read response
     memset(buf, 0, BUFSIZE);
     read(tx, &buf, BUFSIZE-1);
     printf("NTPBypassClient Received Time: %s\n", buf);
     
     tmp = strtok(buf, ",");

    if(tmp)
       tv.tv_sec = atoi(tmp);
     else
       exit(4);

     tmp = strtok (NULL, ",");
	 
	 if(tmp)
       tv.tv_nsec = atoi(tmp);
	 else
	   exit(5);

     //Set the clock
     clock_settime(CLOCK_REALTIME, &tv);

     return 0;
}
