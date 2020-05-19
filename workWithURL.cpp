#include <stdlib.h>
#include "workWithURL.h"
#include "reciveHTTP.h"

char *GetURL(char *BuffWithURl)
{
	char *StartOfURL = strstr(BuffWithURl, HTTP_DELIMITER) + 2/*need for truncate first '/' symbol*/;
	char *EndOfURL = strstr(StartOfURL, HTTP_DELIMITER);
	int URLLen = abs(EndOfURL - StartOfURL);
	char *Result = (char*)calloc(URLLen + 1, sizeof(char));
	memcpy_s(Result, URLLen, StartOfURL, URLLen);
	return Result;
}

char **ParseURL(char *BuffWithURl, int *numberOfFolders)
{
	char *URL = GetURL(BuffWithURl);
	if (strlen(URL) == 0)
	{
		/*if need to  show server root directory*/
	}
	int numberOfSymbol = 0, lastSymbol=0;
	(*numberOfFolders) = URL_DEFAULT_ARR_LEN;
	char **Result = (char**)malloc((*numberOfFolders) * sizeof(char*));
	int i = 0, lengthOfName;
	char *nextFolder = NULL;
	//while (URL[numberOfSymbol] != NULL) {
	//	
	//	if (URL[numberOfSymbol] == '/') {
	//		if (i == (*numberOfFolders)) {
	//			(*numberOfFolders) = (*numberOfFolders) * 2 + 1;
	//			Result = (char**)realloc(Result, (*numberOfFolders) * sizeof(char*));
	//		}
	//		int lengthOfFolder = numberOfSymbol - lastSymbol;
	//		
	//		Result[i] = (char*)malloc((lengthOfFolder) * sizeof(char));
	//		strcpy_s(Result[i++], lengthOfFolder, URL[lastSymbol]);
	//		i++;
	//		lastSymbol = numberOfSymbol + 1;
	//	}
	//	numberOfSymbol++;

	//}
	//if (URL == nullptr) {
	//	i = 0;
	//}
	char *folder = strtok_s(URL, URL_DELIMITER, &nextFolder);
	while (folder != NULL)
	{
		if (i == (*numberOfFolders))
		{
			(*numberOfFolders) = (*numberOfFolders) * 2 + 1;
			Result = (char**)realloc(Result, (*numberOfFolders) * sizeof(char*));
		}
		lengthOfName = strlen(folder);
		Result[i] = (char*)malloc((lengthOfName + 1) * sizeof(char));
		strcpy_s(Result[i++], lengthOfName + 1, folder);

		folder = strtok_s(NULL, URL_DELIMITER, &nextFolder);
	}

	(*numberOfFolders) = i;
	Result = (char**)realloc(Result, (*numberOfFolders) * sizeof(char*));

	free(URL);
	return Result ;
}