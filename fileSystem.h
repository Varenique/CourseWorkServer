#pragma once

#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>

#define PATH_FOR_SERVER_ROOT "./Root"
#define PATH_DELIMITER "/"
#define MSG_DIR_IS_EMPTY "Empty folder"

void checkOrCreateRoot();
void getRequest(char *PathArr[], int ArrLen, SOCKET Sock, char *HTTPBuff);
void readDirectory(SOCKET Sock, char Path[]);
BOOL isFile(char Name[]);
void CreateFullPath(char *PathArr[], int ArrLen, char **Path);
void ConcatPath(char **Path, const char *NextName);
void readFile(SOCKET Sock, char Path[], char *HTTPBuff);

void putRequest(char *PathArr[], int ArrLen, SOCKET Sock, char *HTTPBuff, int ReciveRes);

void WriteInFile(FILE *Out, char *Buff, int Len);
void deleteRequest(char *PathArr[], int ArrLen, SOCKET Sock);
void headRequest(char *PathArr[], int ArrLen, SOCKET Sock);

int DelDir(char *Path);

void CreateDir(char *PathArr[], int ArrLen, char **Path);
void FileCopy(char *PathArr[], int ArrLen, SOCKET Sock, char *HTTPBuff);
