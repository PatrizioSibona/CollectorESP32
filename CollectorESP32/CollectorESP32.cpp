// CollectorESP32.cpp: definisce il punto di ingresso dell'applicazione console.
//
#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#include "SensorData.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define DIM_OK 2
#define DIM_ERR 3
#define N_FIELDS 7

using namespace std;

//PROTOTYPES
int checkMessage(char *buf);
void sendOK(char *buf, SOCKET s);
void sendERR(char *buf, SOCKET s);
int readDimension(char * buf, SOCKET s);
void DataParser(char *buffer);

void sendTime(char *buf, SOCKET s);
int gettimeofday(struct timeval * tp, struct timezone * tzp);

int main(void)
{
	WSADATA wsaData;
	int iResult, err, iSendResult, dimBuffer;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo server;

	char recvbuf[DEFAULT_BUFLEN];
	char timebuf[4 * DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	char* buffer;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup failed with error: " << iResult << endl;
		return 1;
	}

	ZeroMemory(&server, sizeof(server));
	server.ai_family = AF_INET;
	server.ai_socktype = SOCK_STREAM;
	server.ai_protocol = IPPROTO_TCP;
	server.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &server, &result);
	if (iResult != 0) {
		cout << "Getaddrinfo failed with error: " << iResult << endl;
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		cout << "Socket failed with error: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		cout << "Bind failed with error: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		cout << "Listen failed with error: " << WSAGetLastError() << endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		cout << "Accept failed with error: " << WSAGetLastError() << endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(ListenSocket);

	ZeroMemory(recvbuf, recvbuflen);

	//Receive the authentication of ESP board
	iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) {
		//cout << "Bytes received: " << iResult << endl;
		recvbuf[iResult] = '\0';

		//Check the message
		err = checkMessage(recvbuf);
		if (err == 0) {
			cout << "Authentication of: " << recvbuf << endl;

			sendOK(recvbuf, ClientSocket);
		}
		else {
			sendERR(recvbuf, ClientSocket);
		}
	}
	else if (iResult == 0)
		cout << "Connection closing... " << endl;
	else {
		cout << "Recv failed with error: " << WSAGetLastError() << endl;
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	//Time sync
	iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) {
		recvbuf[iResult] = '\0';
		//cout << "Received request of" << recvbuf << endl;
		sendTime(timebuf, ClientSocket);
	}
	// Receive until the peer shuts down the connection: only more probe request packet 
	ZeroMemory(recvbuf, recvbuflen);
	//<-- This solution is correct only for one ESP32 board, will be necessary to manage more than one board -->
	do {
		iResult = readDimension(recvbuf, ClientSocket);
		if (iResult > 0) {
			//Allocation of store buffer
			dimBuffer = iResult;
			buffer = (char *)malloc((dimBuffer+1) * sizeof(char));
		}
		else {
			//The dimension is equal than 0, i'll wait the next dimension value
			continue;
		}

		ZeroMemory(recvbuf, recvbuflen);

		iResult = recv(ClientSocket, buffer, dimBuffer, 0);
		if (iResult > 0) {
			cout << "Bytes received: " << iResult << endl;
			buffer[iResult] = '\0';

					//cout << "Data received: " << buffer << endl;
					DataParser(buffer);
					cout << "Waiting ...\n" << endl;
					free(buffer);
				}
			}
			else if (iResult == 0)
				cout << "Connection closing..." << endl;
			else {
				cout << "Recv failed with error: " << WSAGetLastError() << endl;
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
		}

	} while (true);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		cout << "Shutdown failed with error: " << WSAGetLastError() << endl;
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}

int checkMessage(char *buf) {

	if (buf[0] == 'E' && buf[1] == 'S' && buf[2] == 'P')
		return 0;
	else
		return -1;
}

void sendOK(char *buf, SOCKET s) {
	int result;
	// Send the OK response 
	buf[0] = 'O';
	buf[1] = 'K';
	buf[2] = '\0';
	result = send(s, buf, DIM_OK + 1, 0);
	if (result == SOCKET_ERROR) {
		cout << "Send failed with error: " << WSAGetLastError() << "\n";
		closesocket(s);
		WSACleanup();
	}
	else
		cout << "Send OK message correctly\n";
}

void sendERR(char *buf, SOCKET s) {
	int result;
	// Send the ERR response
	buf[0] = 'E';
	buf[1] = 'R';
	buf[2] = 'R';
	buf[2] = '\0';
	result = send(s, buf, DIM_ERR + 1, 0);
	if (result == SOCKET_ERROR) {
		cout << "Send failed with error: " << WSAGetLastError() << "\n";
		closesocket(s);
		WSACleanup();
	}
	else {
		cout << "Send ERR message correctly\n";
	}
	return;
}

void sendTime(char *buf, SOCKET s) {
	int result;
	// Send the Time
	struct timeval t;
	gettimeofday(&t, NULL);
	sprintf_s(buf, DEFAULT_BUFLEN, "%d\t%d", t.tv_sec, t.tv_usec);
	result = send(s, buf, DEFAULT_BUFLEN, 0);
	if (result == SOCKET_ERROR) {
		cout << "Send failed with error: " << WSAGetLastError() << "\n";
		closesocket(s);
		WSACleanup();
	}
	else {
		cout << "Send TIME message correctly\n";
	}
	return;
}
int gettimeofday(struct timeval * tp, struct timezone * tzp) {

	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970 
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);
	//static const uint64_t EPOCH_2018 = ((uint64_t)1514764800000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}


void sendTime(char *buf, SOCKET s) {
	int result;
	// Send the Time
	struct timeval t;
	gettimeofday(&t, NULL);
	sprintf_s(buf, DEFAULT_BUFLEN, "%d\t%d", t.tv_sec, t.tv_usec);
	result = send(s, buf, DEFAULT_BUFLEN, 0);
	if (result == SOCKET_ERROR) {
		cout << "Send failed with error: " << WSAGetLastError() << "\n";
		closesocket(s);
		WSACleanup();
	}
	else {
		cout << "Send TIME message correctly\n";
	}
	return;
}
int gettimeofday(struct timeval * tp, struct timezone * tzp) {

	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970 
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);
	//static const uint64_t EPOCH_2018 = ((uint64_t)1514764800000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}


int readDimension(char * buf, SOCKET s) {
	int result, dimension;
	result = recv(s, buf, DEFAULT_BUFLEN, 0);
	if (result > 0) {
		cout << "Bytes received: " << result << endl;
		buf[result] = '\0';

		//Check the message
		dimension = atoi(buf);

		cout << "Dimension received: " << dimension << endl;

		if (dimension >= 0) {
			sendOK(buf, s);
			return dimension;
		}
		else {
			sendERR(buf, s);
			return -1;
		}
	}
}

void DataParser(char *buffer) {
	vector<string> probes, pr_fields;
	string buf(buffer);
	char sep_probe = '\n', sep_fields = '\t';

	probes = split(buf, sep_probe);

	for (string probe : probes) {
		//Retrieve the fields and create a SensorData element
		pr_fields = split(probe, sep_fields);
		if (!pr_fields.empty()) {
			int channel = atoi(pr_fields[0].c_str());
			int RSSI = atoi(pr_fields[1].c_str());
			int sec = atoi(pr_fields[4].c_str());
			int usec = atoi(pr_fields[5].c_str());
			if (pr_fields.size() == N_FIELDS) {
				//There is SSID field
				SensorData data(channel, RSSI, sec, usec, pr_fields[2], pr_fields[3], pr_fields[6]);
				data.printData();
			}
			else {
				SensorData data(channel, RSSI, sec, usec, pr_fields[2], pr_fields[3], "");
				data.printData();
			}
		}
	}

	return;
}

vector<string> split(const string& s, char delimiter)
{
	vector<std::string> tokens;
	string token;
	istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}