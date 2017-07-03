
//+================================================+
//                         ntp_bypass_softmcu_query.c
// 
// This Utility is designed to:
//         1. examine server log to know ntpd server status 
//
//+================================================+

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define NTP_BYPASS_SOFTMCU_SERVER_LOG_PATH         "/tmp/ntp_bypass_softmcu_server.log"

int main(int argc, char *argv[])
{
	char server_str[256];
	char data_buf[2048];
	FILE *log_fh = NULL;
	long file_len = 0;
	int result;
	size_t readed;
	char *prev_ptr = NULL, *cur_ptr = NULL, *final_ptr = NULL;
	int server_str_len;
	int offset_data_sec, offset_data_micro_sec;
	
	if (argc != 2)
	{
		printf("Invalid number of arguments.\n");
		printf("Usage: %s NTPD_SERVER_STRING\n", argv[0]);

		return -1;
	}

	snprintf(server_str, 255, "%s, stratum", argv[1]);
	server_str[255] = '\0';
	server_str_len = strlen(server_str);

	log_fh = fopen(NTP_BYPASS_SOFTMCU_SERVER_LOG_PATH, "r");

	if (log_fh == NULL)
	{
		printf("Failed to open log file %s.\n", NTP_BYPASS_SOFTMCU_SERVER_LOG_PATH);

		return -2;
	}

	result = fseek(log_fh, 0, SEEK_END);

	if (result != 0)
	{
		printf("Failed to seek to file end: %s\n", strerror(errno));
		fclose(log_fh);

		return -3;
	}

	file_len = ftell(log_fh);

	if (file_len > 2048)
	{
		fseek(log_fh, -2048, SEEK_END);
	}
	else
	{
		fseek(log_fh, 0, SEEK_SET);
	}

	readed = fread(data_buf, 1, 2047, log_fh);

	if (readed > 0)
	{
		cur_ptr = data_buf;
		data_buf[readed] = '\0';
		
		while (1)
		{
			cur_ptr = strstr(cur_ptr, server_str);

			if (cur_ptr)
			{
				cur_ptr += server_str_len;
				prev_ptr = cur_ptr;
			}
			else
			{
				final_ptr = prev_ptr;

				break;
			}
		}

		if (final_ptr)
		{
			result = sscanf(final_ptr, " %*d, offset %d.%d", &offset_data_sec, &offset_data_micro_sec);

			if (result != 2)
			{
				printf("Failed to parse %s\n", final_ptr);

				return -4;
			}
			else if (offset_data_sec == 0 && offset_data_micro_sec == 0)
			{
				printf(" Status: Fail\n");

				return 0;
			}
			else
			{
				printf("*Status: OK\n");

				return 0;
			}
		}
		else
		{
			printf("Failed to find substring %s\n", server_str);

			return -5;
		}
	}
	else
	{
		printf("Failed to read from server log: %s\n", strerror(errno));
		fclose(log_fh);
		
		return -6;
	}

	fclose(log_fh);

	return 0;
}

