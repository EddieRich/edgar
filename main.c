#include <stdlib.h>
#include <curl/curl.h>
#include "edgar.h"

void init()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

void exit_callback()
{
	curl_global_cleanup();
}

int main(int argc, char* argv[])
{
	atexit(exit_callback);
	init();

	get_facts("msft");

	return 0;
}

