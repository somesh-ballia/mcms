
//+================================================+
//                         ntp_bypass_softmcu_client.c
// 
// This Utility is designed to:
//         1. request time data from ntpd server (query only in this step)
//         2. save the time offset data
//         3. wait update time request
//         4. update time and respond when complete
// 
// Note:
//         Unlike use ntpdate directly, step 4 should complete quickly
//+================================================+

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

#define NTP_BYPASS_SOFTMCU_CLIENT_LOG_PATH         "/tmp/ntp_bypass_softmcu_client.log"
#define NTP_BYPASS_SOFTMCU_CLIENT_PORT             10004

FILE *g_log_fh = NULL;

#define BUFSIZE 100

int main()
{
	int tx, err;
	struct sockaddr_in stream_addr;
	int read_len, write_len;
	char buf[BUFSIZE] = "";
	char *tmp;

	g_log_fh = fopen(NTP_BYPASS_SOFTMCU_CLIENT_LOG_PATH, "w");

	if (g_log_fh == NULL)
	{
		printf("Failed to open log file %s.\n", NTP_BYPASS_SOFTMCU_CLIENT_LOG_PATH);

		return -1;
	}

	//Create socket to server
	if((tx = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		fprintf(g_log_fh, "Failed to create tcp socket.\n");

		fclose(g_log_fh);
		
		return -1;
	}

	bzero((char*)&stream_addr, sizeof(stream_addr));
	stream_addr.sin_family = AF_INET;
	stream_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	stream_addr.sin_port = htons(NTP_BYPASS_SOFTMCU_CLIENT_PORT);

	err = connect(tx,(struct sockaddr *) &stream_addr, sizeof(struct sockaddr_in));

	if (err < 0)
	{
		fprintf(g_log_fh, "Failed to connect to server 127.0.0.1:%u.\n", NTP_BYPASS_SOFTMCU_CLIENT_PORT);

		fclose(g_log_fh);
		close(tx);
		
		return -2;
	}

	memset(buf, 0, BUFSIZE);
	strncpy(buf, "UPDATE_TIME", 11);

	fprintf(g_log_fh, "Connected to server ...\n");
	
	//First, send request (opcode is "UPDATE_TIME")
	if (write(tx, buf, strlen(buf)) != 11)
	{
		fprintf(g_log_fh, "Failed to send UPDATE_TIME request.\n");

		fclose(g_log_fh);
		close(tx);
		
		return -3;
	}

	//Read response
	memset(buf, 0, BUFSIZE);
	read(tx, &buf, BUFSIZE-1);
	
	fprintf(g_log_fh, "Received response: %s\n", buf);

	close(tx);

	printf("Update done.\n");

	return 0;
}

