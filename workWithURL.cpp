#include <stdlib.h>
#include "workWithURL.h"
#include "requestHTTP.h"
#include "processSystem.h"

char *getURL(char *HTTPBuffer)
{
	char *startOfURL = strstr(HTTPBuffer, HTTP_DELIMITER) + 2;
	char *endOfURL = strstr(startOfURL, HTTP_DELIMITER);
	int lengthOfURL = abs(endOfURL - startOfURL);
	char *result = (char*)calloc(lengthOfURL + 1, sizeof(char));
	memcpy_s(result, lengthOfURL, startOfURL, lengthOfURL);
	return result;
}

char **parseURL(char *HTTPBuffer, int *numberOfFolders) {
	char *URL = getURL(HTTPBuffer);
	int numberOfSymbol = 0, lastSymbol=0;
	(*numberOfFolders) = URL_DEFAULT_ARR_LEN;
	char **result = (char**)malloc((*numberOfFolders) * sizeof(char*));
	int i = 0, lengthOfName;
	char *nextFolder = NULL;
	char *folder = strtok_s(URL, URL_DELIMITER, &nextFolder);
	while (folder != NULL) {
		if (i == (*numberOfFolders)) {

			(*numberOfFolders) = (*numberOfFolders) * 2 + 1;
			result = (char**)realloc(result, (*numberOfFolders) * sizeof(char*));
		}
		lengthOfName = strlen(folder);
		result[i] = (char*)malloc((lengthOfName + 1) * sizeof(char));
		strcpy_s(result[i++], lengthOfName + 1, folder);

		folder = strtok_s(NULL, URL_DELIMITER, &nextFolder);
	}

	(*numberOfFolders) = i;
	result = (char**)realloc(result, (*numberOfFolders) * sizeof(char*));

	free(URL);
	return result ;
}