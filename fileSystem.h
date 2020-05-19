#pragma once

#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>

#define PATH_FOR_SERVER_ROOT "./Root"
#define PATH_DELIMITER "/"
#define MSG_IS_EMPTY "Empty folder"

void checkOrCreateRoot();
void readDirectory(SOCKET clientSocket, char path[]);
bool isFile(char name[]);
void createFullPath(char *arrayOfPath[], int lengthOfPath, char **path);
void concatPath(char **path, char *toAdd);
void readFile(SOCKET clientSocket, char Path[], char *HTTPBuffer);
void createDirectory(char *arrayOfPath[], int lengthOfPath, char **path);

void getRequest(char *arrayOfPath[], int lengthOfPath, SOCKET clientSocket, char *HTTPBuffer);
void headRequest(char *arrayOfPath[], int lengthOfPath, SOCKET clientSocket);
void putRequest(char *arrayOfPath[], int lengthOfPath, SOCKET clientSocket);
void deleteRequest(char *arrayOfPath[], int lengthOfPath, SOCKET clientSocket);