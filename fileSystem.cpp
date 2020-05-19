#include <stdlib.h>
#include "fileSystem.h"
#include <WinSock2.h>
#include <fileapi.h>
#include "responseHTTP.h"
#include "workWithURL.h"
#include "processSystem.h"


void checkOrCreateRoot() {

	WIN32_FIND_DATAA directory;												//структура, которая описывает информацию о файле, найденном с помощью WIN32_FIND_DATAA
	HANDLE hDerictory = FindFirstFileA(PATH_FOR_SERVER_ROOT, &directory);			// функция ищет файл по пути и сохраняет информацию в структуру. Возвращает дескриптор найденного файла или INVALID_HANDLE_VALUE, если файл не найден
	if (hDerictory == INVALID_HANDLE_VALUE) {
	
		CreateDirectoryA(PATH_FOR_SERVER_ROOT, NULL);					//функция, создающая новую директорию. Параметры: LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes
	}
}

bool isFile(char name[]) {

	bool result = false;
	if ((strstr(name, ".") != NULL) && (strstr(name, "..") == NULL)) {			//возвращает указатель на первое вхождение
	
		result = true;
	}
	return result;
}

void getRequest(char *pathOfArray[], int lengthOfArray, SOCKET clientSocket, char *HTTPBuffer) {

	int lengthOfPath;
	if (lengthOfArray == 0) {								//покажем все файлы папки
		send(clientSocket, OK_RESPONSE, strlen(OK_RESPONSE), 0);
		lengthOfPath = strlen(PATH_FOR_SERVER_ROOT) + strlen(PATH_DELIMITER) + strlen("*");
		char *pathOfDirectory = (char*)calloc(lengthOfPath + 1, sizeof(char));
		strcat_s(pathOfDirectory, lengthOfPath + 1, PATH_FOR_SERVER_ROOT);//добавляет 3 аргумент к 1
		strcat_s(pathOfDirectory, lengthOfPath + 1, PATH_DELIMITER);
		strcat_s(pathOfDirectory, lengthOfPath + 1, "*");
		readDirectory(clientSocket, pathOfDirectory);
		free(pathOfDirectory);
	}
	else {

		lengthOfPath = strlen(PATH_FOR_SERVER_ROOT);
		char *path = (char*)calloc(lengthOfPath + 1, sizeof(char));
		memcpy_s(path, lengthOfPath + 1, PATH_FOR_SERVER_ROOT, lengthOfPath);
		createFullPath(pathOfArray, lengthOfArray, &path);

		HANDLE hFile;
		WIN32_FIND_DATAA fileStructure;
		hFile = FindFirstFileA(path, &fileStructure);//ищем файл по полному пути и сохраняем в структуру

		if (isFile(pathOfArray[lengthOfArray - 1])) {

			if (hFile == INVALID_HANDLE_VALUE) {

				send(clientSocket, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
			}
			else {

				send(clientSocket, OK_RESPONSE, strlen(OK_RESPONSE), 0);
				readFile(clientSocket, path, HTTPBuffer);
			}
		}
		else {

			if (hFile == INVALID_HANDLE_VALUE) {

				send(clientSocket, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
			}
			else {

				send(clientSocket, OK_RESPONSE, strlen(OK_RESPONSE), 0);
				concatPath(&path, "*");
				readDirectory(clientSocket, path);
			}
		}
	}
}
void createFullPath(char *arrayOfPath[], int lengthOfPath, char **path) {

	for (int i = 0; i < lengthOfPath; i++) {

		concatPath(path, arrayOfPath[i]);
	}
}
void concatPath(char **path, char *toAdd) {

	int length = strlen(*path) + strlen(toAdd) + strlen(PATH_DELIMITER);
	(*path) = (char*)realloc((*path), (length + 1) * sizeof(char));
	strcat_s((*path), length + 1, PATH_DELIMITER);
	strcat_s((*path), length + 1, toAdd);
}

void readDirectory(SOCKET clientSocket, char  pathOfDirectory[]) {

	HANDLE hFile;
	WIN32_FIND_DATA fileStruct;
	hFile = FindFirstFileA(pathOfDirectory, &fileStruct);//получаем дескриптор первого
	bool isEmpty = true;
	while (FindNextFileA(hFile, &fileStruct) != 0) {// продолжает поиск файлов, не гарантирует порядок
			
		send(clientSocket, fileStruct.cFileName, strlen(fileStruct.cFileName), 0);
		send(clientSocket, HTTP_END_OF_LINE, strlen(HTTP_END_OF_LINE), 0);
		isEmpty = false;
	}
	if (isEmpty)
	{
		send(clientSocket, MSG_IS_EMPTY, strlen(MSG_IS_EMPTY), 0);
		send(clientSocket, HTTP_END_OF_LINE, strlen(HTTP_END_OF_LINE), 0);
	}
	FindClose(hFile);
}

void readFile(SOCKET clientSocket, char path[], char *HTTPBuffer) {

	FILE *fileToRead;
	fopen_s(&fileToRead, path, "rb");// read binary, открываем файл
	int resultOfRead;
	do {

		resultOfRead = fread_s(HTTPBuffer, HTTP_BUFFER_SIZE + 1, sizeof(char), HTTP_BUFFER_SIZE, fileToRead);//читаем данные из файла в буфер
		if (resultOfRead != EOF) {

			int size = resultOfRead;
			char *bufferToSend = HTTPBuffer;
			int realSize;
			do {

				realSize = send(clientSocket, bufferToSend, size, 0);
				size -= realSize;
				bufferToSend += realSize;
			} while (size != 0);
		}
	} while (!feof(fileToRead));
	fclose(fileToRead);
}

void headRequest(char *arrayOfPath[], int lengthOfPath, SOCKET clientSocket) {

	int length = strlen(PATH_FOR_SERVER_ROOT);
	char *path = (char*)calloc(length + 1, sizeof(char));
	memcpy_s(path, length + 1, PATH_FOR_SERVER_ROOT, length);
	createFullPath(arrayOfPath, lengthOfPath, &path);

	HANDLE hFile;
	WIN32_FIND_DATA fileStruct;
	hFile = FindFirstFileA(path, &fileStruct);

	if (hFile == INVALID_HANDLE_VALUE) {

		send(clientSocket, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
	}
	else {

		if ((fileStruct.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)) { //дескриптор идентифицирует каталог 

			length = HTTP_BUFFER_SIZE;
			char *responseBuffer = (char*)calloc(length + 1, sizeof(char));
			long long size = (fileStruct.nFileSizeHigh * (MAXDWORD + 1)) + fileStruct.nFileSizeLow;// таким образом определяем размер файла
			sprintf_s(responseBuffer, length + 1, "HTTP/1.1 200 OK\r\nFile-Size: %lld\r\nType: File\r\nName: %s\r\n\r\n\0", size, fileStruct.cFileName);
			send(clientSocket, responseBuffer, strlen(responseBuffer), 0);
			free(responseBuffer);
		}
		else {

			send(clientSocket, NOT_ALLOWED_RESPONSE, strlen(NOT_ALLOWED_RESPONSE), 0);
		}
	}
}

void putRequest(char *arrayOfPath[], int lengthOfPath, SOCKET clientSocket) {

	if (lengthOfPath == 0) {

		send(clientSocket, BAD_RESPONSE, strlen(BAD_RESPONSE), 0);
		return;
	}

	int length = strlen(PATH_FOR_SERVER_ROOT);
	char *path = (char*)calloc(length + 1, sizeof(char));
	memcpy_s(path, length + 1, PATH_FOR_SERVER_ROOT, length);

	createDirectory(arrayOfPath, lengthOfPath, &path);
	HANDLE hFile;
	WIN32_FIND_DATAA fileStruct;
	concatPath(&path, arrayOfPath[lengthOfPath - 1]);
	hFile = FindFirstFileA(path, &fileStruct);
	if (isFile(arrayOfPath[lengthOfPath - 1])) {

		if (hFile == INVALID_HANDLE_VALUE) {

			CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
			send(clientSocket, CREATED_RESPONSE, strlen(CREATED_RESPONSE), 0);
			free(path);
		}
		else {

			send(clientSocket, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			free(path);
		}
	}
	else {

		if (hFile == INVALID_HANDLE_VALUE)
		{
			CreateDirectoryA(path, NULL);
			free(path);
			send(clientSocket, CREATED_RESPONSE, strlen(CREATED_RESPONSE), 0);
		}
		else
		{
			send(clientSocket, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			free(path);
		}
	}
}

void deleteRequest(char *arrayOfPath[], int lengthOfPath, SOCKET clientSocket) {
	if (lengthOfPath == 0) {
		send(clientSocket, FORBIDDEN_RESPONSE, strlen(FORBIDDEN_RESPONSE), 0);
		return;
	}

	int length = strlen(PATH_FOR_SERVER_ROOT);
	char *path = (char*)calloc(length + 1, sizeof(char));
	memcpy_s(path, length + 1, PATH_FOR_SERVER_ROOT, length);

	createFullPath(arrayOfPath, lengthOfPath, &path);

	HANDLE hFile;
	WIN32_FIND_DATA Data;
	hFile = FindFirstFileA(path, &Data);
	if (isFile(arrayOfPath[lengthOfPath - 1])) {

		if (hFile == INVALID_HANDLE_VALUE) {
			send(clientSocket, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
		}
		else {

			if (DeleteFileA(path) != 0) {
				send(clientSocket, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			}
			else {
				send(clientSocket, INTERNAL_ERROR_RESPONSE, strlen(INTERNAL_ERROR_RESPONSE), 0);
			}
		}
	}
	else {
		if (hFile == INVALID_HANDLE_VALUE) {

			send(clientSocket, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
			return;
		}
		else {

			if (RemoveDirectoryA(path) != 0) {

				send(clientSocket, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			}
			else {

				send(clientSocket, INTERNAL_ERROR_RESPONSE, strlen(INTERNAL_ERROR_RESPONSE), 0);
			}
		}
	}
}

void createDirectory(char *arrayOfPath[], int lengthOfPath, char **path) {

	HANDLE hFile;
	WIN32_FIND_DATAA fileStruct;
	for (int i = 0; i < lengthOfPath - 1; i++) {

		concatPath(path, arrayOfPath[i]);
		hFile = FindFirstFileA((*path), &fileStruct);
		if (hFile == INVALID_HANDLE_VALUE) {

			CreateDirectoryA((*path), NULL);
		}
	}
}
