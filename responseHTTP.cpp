#include <stdlib.h>
#include <string.h>
#include "responseHTTP.h"
#include "requestHTTP.h"
#include "fileSystem.h"

void getResponse(char *arrayOfPath[], int lengthOfPath, TRequestType typeOfRequest, SOCKET clientSocket, char *HTTPBuffer)
{
	switch (typeOfRequest)
	{
	case Put:
		putRequest(arrayOfPath, lengthOfPath, clientSocket);
		break;
	case Get:
		getRequest(arrayOfPath, lengthOfPath, clientSocket, HTTPBuffer);
		break;
	case Delete:
		deleteRequest(arrayOfPath, lengthOfPath, clientSocket);
		break;
	case Head:
		headRequest(arrayOfPath, lengthOfPath, clientSocket);
		break;
	}
}