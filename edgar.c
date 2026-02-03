#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <curl/curl.h>
#include <jv.h>
#include <jq.h>

#include "edgar.h"

char filename[FILENAME_MAX];

void error_and_die(char* message, char* param)
{
	printf("\nERROR: %s %s\n\n", message, param);
	exit(EXIT_FAILURE);
}

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;

	FILE* fp = (FILE*)userp;
	if (fp == NULL)
		return 0L;

	fwrite(contents, size, nmemb, fp);
	fflush(fp);

	return realsize;
}

void fetch(char* url)
{
	CURL* pcurl = curl_easy_init();
	if (pcurl)
	{
		FILE* fp = fopen(filename, "wb");
		if (fp == NULL)
		{
			curl_easy_cleanup(pcurl);
			error_and_die("could not open file for write access,", filename);
		}

		curl_easy_setopt(pcurl, CURLOPT_URL, url);
		curl_easy_setopt(pcurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(pcurl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(pcurl, CURLOPT_USERAGENT, "ehandrich@gmail.com");

		CURLcode res = curl_easy_perform(pcurl);
		if (res != CURLE_OK)
		{
			fclose(fp);
			curl_easy_cleanup(pcurl);
			error_and_die("Fetch file failed for ", filename);
		}

		fflush(fp);
		fclose(fp);
	}
	else
	{
		curl_easy_cleanup(pcurl);
		error_and_die("Failed to init curl while fetching", filename);
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
	memset(filename, 0, sizeof(filename));
	sprintf(filename, "/home/ed/Edgar/ticker.txt");
	if (file_age() > 7L)
		fetch("https://www.sec.gov/include/ticker.txt");

	char line[200];
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL)
		error_and_die("get_cid could not open", filename);

	while (!feof(fp))
	{
		memset(line, 0, sizeof(line));
		fgets(line, sizeof(line), fp);
		char* p = strtok(line, "\t\n ");
		if (p == NULL)
			error_and_die("could not find ticker", ticker);

		if (strcasecmp(p, ticker) == 0)
		{
			fclose(fp);
			return atol(strtok(NULL, "\t\n "));
		}
	}

	error_and_die("could not find ticker", ticker);
	return 0L;
}

void get_facts(char* ticker)
{
	long cid = get_cid(ticker);
	memset(filename, 0, sizeof(filename));
	sprintf(filename, "/home/ed/Edgar/%s_facts.json", ticker);
	if (file_age() > 30L)
	{
		char url[200];
		memset(url, 0, sizeof(url));
		sprintf(url, "https://data.sec.gov/api/xbrl/companyfacts/CIK%010ld.json", cid);
		fetch(url);
	}

	jq_state* jq = jq_init();
	if (!jq)
		error_and_die("could not initialize jq library", NULL);

	jv input = jv_load_file(filename, 0);

	if (!jq_compile(jq, ".facts.dei.EntityCommonStockSharesOutstanding.units.shares.[-1].val"))
	{
		jq_teardown(&jq);
		jv_free(input);
		error_and_die("could not compile filter:", "outstanding shares");
	}

	jq_start(jq, input, 0);

	jv result;
	while (jv_is_valid(result = jq_next(jq)))
	{
		printf("%s\n", jv_string_value(result));
		jv_free(result);
	}

	jq_teardown(&jq);
}
