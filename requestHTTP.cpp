#include <stdlib.h>
#include <string.h>
#include "requestHTTP.h"
#include "processSystem.h"


void findRequestType(char stringOfRequest[], PRequestType typeOfRequest) {

	int notEqual;
	notEqual = strcmp(stringOfRequest, PUT_REQUEST);
	if (notEqual == 0) {

		(*typeOfRequest) = Put;
		return;
	}

	notEqual = strcmp(stringOfRequest, GET_REQUEST);
	if (notEqual == 0) {

		(*typeOfRequest) = Get;
		return;
	}

	notEqual = strcmp(stringOfRequest, DELETE_REQUEST);
	if (notEqual == 0) {

		(*typeOfRequest) = Delete;
		return;
	}

	notEqual = strcmp(stringOfRequest, HEAD_REQUEST);
	if (notEqual == 0) {

		(*typeOfRequest) = Head;
		return;
	}
}

TRequestType getRequestType(char *HTTPBuffer) {

	TRequestType Result = NotImplemented;
	char *typePointerEnd, *stringOfRequest;
	typePointerEnd = strstr(HTTPBuffer, HTTP_DELIMITER);
	if (typePointerEnd != NULL) {

		int length = abs(typePointerEnd - HTTPBuffer);
		stringOfRequest = (char*)calloc(length + 1, sizeof(char));
		strncpy_s(stringOfRequest, length + 1, HTTPBuffer, length);
		findRequestType(stringOfRequest, &Result);
	}
	return Result;
}
