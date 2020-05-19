#pragma once

#define URL_DEFAULT_ARR_LEN 10
#define URL_DELIMITER "/"

char *getURL(char *HTTPBuffer);
char **parseURL(char *HTTPBuffer, int *numberOfFolders);