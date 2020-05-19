#pragma once

typedef enum _RequestType
{
	NotImplemented,
	Put,
	Get,
	Delete,
	Head
} TRequestType, *PRequestType;

#define PUT_REQUEST "PUT"
#define GET_REQUEST "GET"
#define DELETE_REQUEST "DELETE"
#define HEAD_REQUEST "HEAD"

TRequestType getRequestType(char *HTTPBuffer);
void findRequestType(char stringOfRequest[], PRequestType typeOfRequest);



