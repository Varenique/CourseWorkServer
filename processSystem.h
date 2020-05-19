#include <WinSock2.h>

#define HTTP_BUFFER_SIZE 65536 
#define HTTP_END_OF_LINE "\r\n"
#define HTTP_DELIMITER " "

bool isTwoDots(char **arrayOfPath, int lengthOfPath);
void recieveRequest(SOCKET clientSocket);

