
//+================================================+
//                         ntp_bypass_softmcu_server.c 
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
#include <pthread.h>
#include <arpa/inet.h>

#define NTP_BYPASS_SOFTMCU_SERVER_LOG_PATH         "/tmp/ntp_bypass_softmcu_server.log"
#define NTP_BYPASS_SOFTMCU_SERVER_PORT             10004

char g_ntpd_server_str[256];
FILE *g_log_fh = NULL;
pthread_mutex_t g_offset_data_mutex = PTHREAD_MUTEX_INITIALIZER;
int g_offset_data_sec = 0;
int g_offset_data_nsec = 0;
int g_time_updated = 0;

void* ReqTimeData(void *arg);
int SystemPipedCommand(const char* cmd, char *out, int out_len);

int main(int argc, char *argv[])
{
	int  rc;
	pthread_t req_time_data_thread;
	struct timespec tv;
	int listen_sk, data_sk;
	struct sockaddr_in listen_addr, client_addr;
	socklen_t client_addr_len;
	char data_buf[64];
	ssize_t readed;
	int optval;
	int optlen;
	
	if (argc != 2)
	{
		printf("Invalid number of arguments.\n");
		printf("Usage: %s NTPD_SERVER_STRINGS\n", argv[0]);

		return -1;
	}

	if (fork())
		exit(0);

	snprintf(g_ntpd_server_str, 255, "%s", argv[1]);
	g_ntpd_server_str[255] = '\0';

	g_log_fh = fopen(NTP_BYPASS_SOFTMCU_SERVER_LOG_PATH, "w");

	if (g_log_fh == NULL)
	{
		printf("Failed to open log file %s.\n", NTP_BYPASS_SOFTMCU_SERVER_LOG_PATH);

		return -2;
	}

	fprintf(g_log_fh, "Input server string: %s\n", argv[1]);
	fprintf(g_log_fh, "Server string to be used: %s\n", g_ntpd_server_str);

	fflush(g_log_fh);

	if ((listen_sk = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		fprintf(g_log_fh, "Failed to create tcp socket: %s.\n", strerror(errno));

		fclose(g_log_fh);
		
		return -3;
	}

	optval = 1;
	setsockopt(listen_sk, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	sleep(5);
	
	bzero((char*)&listen_addr, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	listen_addr.sin_port = htons(NTP_BYPASS_SOFTMCU_SERVER_PORT);

	if (bind(listen_sk, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) == -1)
	{
		fprintf(g_log_fh, "Failed to bind tcp socket: %s.\n", strerror(errno));

		fclose(g_log_fh);
		close(listen_sk);

		return -4;
	}

	if (listen(listen_sk, 1) == -1)
	{
		fprintf(g_log_fh, "Failed to listen tcp socket: %s.\n", strerror(errno));

		fclose(g_log_fh);
		close(listen_sk);

		return -4;
	}
	
	rc = pthread_mutex_init(&g_offset_data_mutex, NULL);

	if (rc != 0)
	{
		fprintf(g_log_fh, "pthread_mutex_init returned with error %d\n", rc);

		fclose(g_log_fh);
		return -3;
	}
	
	rc = pthread_create(&req_time_data_thread,
					NULL,
					ReqTimeData,
					NULL);

	if (rc != 0)
	{
		return -2;
	}

	while (1)
	{
		client_addr_len = sizeof(client_addr);
		data_sk = accept(listen_sk, (struct sockaddr *)&client_addr, &client_addr_len);

		if (data_sk == -1)
		{
			fprintf(g_log_fh, "Failed to accept tcp socket: %s.\n", strerror(errno));

			sleep(5);
		}
		else
		{
			readed = read(data_sk, data_buf, 11);

			if (readed != 0)
			{
				/* Update time */
				pthread_mutex_lock(&g_offset_data_mutex);

                /*
				if (g_offset_data_sec != 0 || g_offset_data_nsec != 0)
				{
					int clock_gettime_result, clock_settime_result;

					clock_gettime_result = clock_gettime(CLOCK_REALTIME, &tv);
					
					tv.tv_sec += g_offset_data_sec;
					tv.tv_nsec += g_offset_data_nsec;

					if (tv.tv_nsec >= 1000000000)
					{
						tv.tv_nsec -= 1000000000;
						tv.tv_sec++;
					}

					clock_settime_result = clock_settime(CLOCK_REALTIME, &tv);
					system("hwclock -w");

					if (clock_gettime_result || clock_settime_result)
					{
						fprintf(g_log_fh, "ERROR!!!!!!!! clock_gettime_result %d, clock_settime_result %d.\n",
							clock_gettime_result, clock_settime_result);
					}
					fprintf(g_log_fh, "\nTime adjusted by %d.%09d\n\n", g_offset_data_sec, g_offset_data_nsec);
					g_time_updated = 1;
					g_offset_data_sec = 0;
					g_offset_data_nsec = 0;
				}
                */
				pthread_mutex_unlock(&g_offset_data_mutex);

				strcpy(data_buf, "Done");

				send(data_sk, data_buf, 4, 0);
			}

			close(data_sk);
		}
	}

	return 0;
}

void* ReqTimeData(void *arg)
{
	char cmd_out[2048];
	char cmd[1024];
    char vcmd_out[2048];
	char vcmd[1024];
	int result;
	char *time_server_ptr;
	char *offset_ptr;
	int offset_data_sec, offset_data_micro_sec;

	snprintf(cmd, sizeof(cmd), "/usr/sbin/ntpdate -q %s", g_ntpd_server_str);

	while (1)
	{
		pthread_mutex_lock(&g_offset_data_mutex);

		if (g_time_updated)
		{
			g_time_updated = 0;
		}

		pthread_mutex_unlock(&g_offset_data_mutex);
		
		result = SystemPipedCommand(cmd, cmd_out, 2048);

		if (result == 0)
		{
			time_server_ptr = strstr(cmd_out, "time server");

			if (time_server_ptr)
			{
				offset_ptr = strstr(time_server_ptr, "offset");

				if (offset_ptr)
				{
					result = sscanf(offset_ptr, "offset %d.%d", &offset_data_sec, &offset_data_micro_sec);

					if (result == 2)
					{
						pthread_mutex_lock(&g_offset_data_mutex);

						if (!g_time_updated)
						{
							g_offset_data_sec = offset_data_sec;
							g_offset_data_nsec = offset_data_micro_sec * 1000;
						}

						fprintf(g_log_fh, "offset %d.%06d\n", offset_data_sec, offset_data_micro_sec);
						fprintf(g_log_fh, "g_offset_data %d.%09d\n\n", g_offset_data_sec, g_offset_data_nsec);
						
						pthread_mutex_unlock(&g_offset_data_mutex);
					}
					else
					{
						fprintf(g_log_fh, "\nparse offset string %s failed with %d\n\n", offset_ptr, result);
					}
				}
				else
				{
					fprintf(g_log_fh, "\nFailed to find string %s\n\n", "offset");
				}
			}
			else
			{
				fprintf(g_log_fh, "\nFailed to find string %s\n\n", "time server");
			}
		}

        //update time here.
    	snprintf(vcmd, sizeof(vcmd), "/usr/sbin/ntpdate -v %s", g_ntpd_server_str);
        fprintf(g_log_fh, vcmd);
        result = SystemPipedCommand(vcmd, vcmd_out, 2048);
        
    	if (result == 0)
    	{
            SystemPipedCommand("hwclock -w", vcmd_out, 2048);
            fprintf(g_log_fh, "hwclock -w\n");
        }
        fprintf(g_log_fh, "===================================================\n");

		fflush(g_log_fh);
		sleep(120);
	}

	return NULL;
}

int SystemPipedCommand(const char* cmd, char *out, int out_len)
{
	char line[256];
	FILE* fpipe = popen(cmd, "r");
	int left_len, line_len;
	int rc;

	fprintf(g_log_fh, "\nInput cmd:  %s\n", cmd);

	if (NULL == fpipe)
	{
		fprintf(g_log_fh, "popen:  %s  :  %s  (%d)\n",
			cmd, strerror(errno), errno);

		return -1;
	}

	out[0] = '\0';
	left_len = out_len - 1;
	while (fgets(line, 256, fpipe))
	{
		line_len = strlen(line);
		strncat(out, line, left_len);

		if (left_len > line_len)
		{
			left_len -= line_len;
		}
		else
		{
			left_len = 0;
			break;
		}
	}

	rc = pclose(fpipe);

	fprintf(g_log_fh, "pclose: return %d.\n", rc);
	fprintf(g_log_fh, "Output: \n%s\n", out);
	
	return 0;
}
