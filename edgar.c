#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <curl/curl.h>

#include "edgar.h"

char filename[FILENAME_MAX];

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;

	FILE* fp = fopen(filename, "ab");
	if (fp == NULL)
		return 0L;

	fwrite(contents, size, nmemb, fp);
	fflush(fp);
	fclose(fp);

	return realsize;
}

void fetch(char* url)
{
	CURL* pcurl = curl_easy_init();
	if (pcurl)
	{
		curl_easy_setopt(pcurl, CURLOPT_URL, url);
		curl_easy_setopt(pcurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(pcurl, CURLOPT_USERAGENT, "ehandrich@gmail.com");

		CURLcode res = curl_easy_perform(pcurl);
		if (res != CURLE_OK)
		{
			printf("Fetch file failed for %s : %s\n\n", filename, curl_easy_strerror(res));
			curl_easy_cleanup(pcurl);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		printf("Failed to init curl while fetching %s\n\n", filename);
		exit(EXIT_FAILURE);
	}
}

long file_age()
{
	struct stat filestat;
	long now = time(NULL);
	long age = now;
	if (stat(filename, &filestat) == 0)
		age = (now - filestat.st_mtime) / 86400;

	return age;
}

long get_cid(char* ticker)
{
	sprintf(filename, "/home/ed/Edgar/ticker.txt");
	if (file_age() > 1L)
		fetch("https://www.sec.gov/include/ticker.txt");

	return 0L;
}

int get_facts(char* ticker)
{
	long cid = get_cid(ticker);
	return 0;
}
