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
#include <thread>
#include "SensorData.h"

#include <mariadb/mysql.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define DIM_OK 2
#define DIM_ERR 3
#define N_FIELDS 8

#define MAX_BUFF_SIZE 65536

using namespace std;

//PROTOTYPES
int checkMessage(char *buf);
void sendOK(char *buf, SOCKET s);
void sendERR(char *buf, SOCKET s);
int readDimension(char * buf, SOCKET s);
void DataParser(char *buffer, char IDSensor, vector<string> fieldsboard, MYSQL* conn);

void sendTime(char *buf, SOCKET s);
int gettimeofday(struct timeval * tp, struct timezone * tzp);

//Function executed by each thread
int service(SOCKET ClientSocket);

void cleanBoard(char IDSensor, MYSQL* conn);

//Variables used in order to connect with MariaDB
using namespace std;

// Initialize constants
//MY_HOSTNAME = "192.168.43.170";
const char* MY_HOSTNAME = "192.168.43.38";
const char* MY_DATABASE = "STALPAdb";
const char* MY_USERNAME = "ssluser";
const char* MY_PASSWORD = "sslpass";

const char* PRIVATE_KEY_CLIENT = "client-key.pem";
const char* PUBLIC_CLIENT_KEY = "client-cert.pem";
const char* CERTIFICATE_AUTORITY_FILE = "ca-cert.pem";

const char* MY_SOCKET = NULL;

enum {
	MY_PORT_NO = 3306,
	MY_OPT = 0
};

//MYSQL     *conn;
//MYSQL_RES *res;
//MYSQL_ROW row;

//Prototypes setup connection with MariaDB
bool setup_connection(MYSQL* conn);

//Prototypes used to insert data into DB
bool insertData(MYSQL* conn, SensorData data);

vector<string> split(const string& str, char delimiter);

int main(void)
{
	cout << "Welcome: ESP32-Collector Started!" << endl;

	WSADATA wsaData;

	int iResult, err, iSendResult, dimBuffer, counterthread = 0;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;


	vector<thread> threadPool;

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

	SecureZeroMemory(&server, sizeof(server));
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
	iResult = bind((SOCKET)ListenSocket, (const sockaddr *)result->ai_addr, (int)result->ai_addrlen);
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

	while(1){
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			cout << "Accept failed with error: " << WSAGetLastError() << endl;
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		// No longer need server socket
		//closesocket(ListenSocket);

		threadPool.push_back(thread(service, ClientSocket));
		threadPool[counterthread].detach();
		counterthread++;
	}

	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}

int service(SOCKET ClientSocket){

	int iResult, err, iSendResult, dimBuffer, counterthread = 0;

	char recvbuf[DEFAULT_BUFLEN];
	char timebuf[4 * DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	char* buffer = NULL;
	char IDSensor;

	vector<string> fieldsboard;
	char sep_field = ' ';

	SecureZeroMemory(recvbuf, recvbuflen);

	// Format a MySQL object
	MYSQL* conn = mysql_init(NULL);

	if (!setup_connection(conn)) {
		cout << "Connection with DB failed!" << endl;
	}
	else {

		//Receive the authentication of ESP board
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			//cout << "Bytes received: " << iResult << endl;
			recvbuf[iResult] = '\0';

			//Check the message
			err = checkMessage(recvbuf);
			if (err == 0) {
				cout << "Authentication of: " << recvbuf << endl;

				IDSensor = recvbuf[6];
			
				//Coordinates
				fieldsboard = split(recvbuf, sep_field);

				//Coordinates insertion
				ostringstream str1, str2;


				str1 << "DELETE FROM Coordinates WHERE IDboard ='" << IDSensor << "';";

				if (mysql_query(conn, str1.str().c_str())) {
					cerr << mysql_error(conn) << endl;
				}

				str2 << "INSERT INTO Coordinates (IDboard, CoordX, CoordY) VALUES ('" << IDSensor << "','" << fieldsboard[2] << "','" << fieldsboard[4] << "')";

				if (mysql_query(conn, str2.str().c_str())) {
					cerr << mysql_error(conn) << endl;
				}

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
			return 1;
		}

		//Time sync
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			recvbuf[iResult] = '\0';
			cout << "Received request of " << recvbuf << endl;
			if (strncmp(recvbuf, "TIME REQUEST", 12)==0)
				sendTime(timebuf, ClientSocket);
			else {
				cout << "expecting time request, received other" << endl;
				cleanBoard(IDSensor, conn);
				closesocket(ClientSocket);
				return -1;
			}
		}
		// Receive until the peer shuts down the connection: only more probe request packet 
		SecureZeroMemory(recvbuf, recvbuflen);

		do {
			int zeroData = 0;
			iResult = readDimension(recvbuf, ClientSocket);
			cout << "Received Board " << IDSensor << " dimension: " << iResult << " recvbuf: "<<recvbuf<< endl;
			SecureZeroMemory(recvbuf, recvbuflen);
			if (iResult > 0 && iResult < MAX_BUFF_SIZE) {
				//Allocation of store buffer
				dimBuffer = iResult;
				//18/10/2018 buffer = (char *)malloc((dimBuffer+1) * sizeof(char));  
				try{
					buffer = new char[dimBuffer + 1];
				}
				catch (bad_alloc& ba) {
					cerr << "bad_alloc caught:" << ba.what() << "\n";
					cleanBoard(IDSensor, conn);
					closesocket(ClientSocket);
					return -1;
				}
			}
			else {
				if(iResult==-1){
					//Board is offline, the DB entry will be deleted 
					cleanBoard(IDSensor, conn);
					closesocket(ClientSocket);
					return -1;
				}
				else{
					//The dimension is equal than 0, i'll wait the next dimension value
					zeroData = 1;
				}
			}
			if (zeroData == 0) {
				int nleft = dimBuffer, nread = 0;
				iResult = 0;
				while (nread != dimBuffer) {
					FD_SET ReadSet;

					FD_ZERO(&ReadSet);

					struct timeval tm;

					tm.tv_sec = 180;
					tm.tv_usec = 0;

					DWORD Total;

					// Always look for connection attempts
					FD_SET(ClientSocket, &ReadSet);

					if ((Total = select(0, &ReadSet, 0, NULL, &tm)) == 0) {
						cout << "Timeout expired reading data!!" << endl;

						cleanBoard(IDSensor, conn);
						closesocket(ClientSocket);
						return 0;
					}
					else if (Total == SOCKET_ERROR) {
						closesocket(ClientSocket);
						return -1;
					}
					else {
						iResult = recv(ClientSocket, buffer + nread, nleft, 0);
						if (iResult > 0) {
							//cout << "Bytes received: " << iResult << endl;
							nleft -= iResult;
							nread += iResult;
							if (nread == dimBuffer) {
								buffer[dimBuffer] = '\0';

								//cout << "Data received: " << buffer << endl;
								DataParser(buffer, IDSensor,fieldsboard, conn);
								cout << "Waiting socket board " << IDSensor << " ... " << endl;
								//18/10/2018 free(buffer);
								delete[] buffer;
								buffer = NULL;
							}
						}
						else if (iResult == 0)
							cout << "Connection closing..." << endl;
						else {
							cout << "Recv failed with error: " << WSAGetLastError() << endl;
							cleanBoard(IDSensor, conn);
							closesocket(ClientSocket);
							return -2;
						}
					}
				}
			}
		} while (true);

		// shutdown the connection since we're done
		iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			cout << "Shutdown failed with error: " << WSAGetLastError() << endl;
			closesocket(ClientSocket);
			return -3;
		}
	}
	
	return 0;
}

int checkMessage(char *buf) {

	if (buf[0] == 'E' && buf[1] == 'S' && buf[2] == 'P' && buf[3] == '3' && buf[4] == '2' && buf[5] == '-')
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
	else{
		//cout << "Send OK message correctly\n";
	}
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
	int result, nleft, nread=0;
	// Send the Time
	struct timeval t, p;
	while(true){
	
		gettimeofday(&t, NULL);
		sprintf_s(buf, DEFAULT_BUFLEN, "%d\t%d", t.tv_sec, t.tv_usec);
		result = send(s, buf, DEFAULT_BUFLEN, 0);
		if (result == SOCKET_ERROR) {
			cout << "Send failed with error: " << WSAGetLastError() << "\n";
			closesocket(s);
		}
		else {
			cout << "Send TIME message correctly\n";
		}

		nleft = strlen(buf);
		while(nread < strlen(buf)){
			result = recv(s, buf, strlen(buf), 0);
			if (result > 0) {
				nleft -= result;
				nread += result;
				if (nread == strlen(buf)) {
					gettimeofday(&t, NULL);

					buf[strlen(buf)] = '\0';

					sscanf_s(buf, "%d\t%d", &p.tv_sec, &p.tv_usec);

					if(t.tv_sec == p.tv_sec){
						sendOK(buf,s);

						//cout << "Time board: " << p.tv_sec << "Time collector: " << t.tv_sec << endl;

						return;
					}
				
				}
			}
			else if (result == 0)
				cout << "Connection closing..." << endl;
			else {
				cout << "Recv failed with error: " << WSAGetLastError() << endl;
				closesocket(s);
				return;
			}
		}
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
	FD_SET ReadSet;

	FD_ZERO(&ReadSet);

	struct timeval tm;

	tm.tv_sec = 180; 
	tm.tv_usec = 0;
	DWORD Total;

	// Always look for connection attempts
	FD_SET(s, &ReadSet);

	int result, dimension;

	if ((Total = select(0, &ReadSet, 0, NULL, &tm)) == 0) {
		cout << "Timeout select reading dimension!!" << endl;
		return -1;
	}
	else if (Total == SOCKET_ERROR) {
		cout << "Error" << endl;
		return -1;
	}
	else {
		//There are data in socket
		result = recv(s, buf, DEFAULT_BUFLEN, 0);
		if (result > 0) {
			//cout << "Bytes received: " << result << endl;
			buf[result] = '\0';

			if (strcmp(buf, "") == 0) {
				return 0;
			}

			//Check the message
			dimension = atoi(buf);

			//cout << "Dimension received: " << dimension << endl;

			if (dimension > 0) {
				sendOK(buf, s);
				return dimension;
			}
			if (dimension == 0) {
				sendERR(buf, s);
				return 0;  //in order to skip reading data
			}
			else {
				sendERR(buf, s);
				return -1;
			}
		}
		if (result == 0) {
			return 0;
		}
		if (result < 0) {
			return -1;
		}
	}
}

void DataParser(char *buffer, char IDSensor, vector<string> fieldsboard, MYSQL* conn) {

	vector<string> probes, pr_fields;
	string buf(buffer);
	char sep_probe = '\n', sep_fields = '\t';

	probes = split(buf, sep_probe);

	ostringstream str, str1;
	MYSQL_RES *res;
	MYSQL_ROW row;

	str << "SELECT * FROM Coordinates WHERE IDboard ='" << IDSensor << "'";

	if (mysql_query(conn, str.str().c_str())) {
		cerr << mysql_error(conn) << endl;
		return;
	}

	res = mysql_use_result(conn);
	if ((row = mysql_fetch_row(res)) == NULL) {
		str1 << "INSERT INTO Coordinates (IDboard, CoordX, CoordY) VALUES ('" << IDSensor << "','" << fieldsboard[2] << "','" << fieldsboard[4] << "')";
		if (mysql_query(conn, str1.str().c_str())) {
			cerr << mysql_error(conn) << endl;
			return;
		}
	}

	mysql_free_result(res);

	for (string probe : probes) {
		//Retrieve the fields and create a SensorData element
		pr_fields = split(probe, sep_fields);
		if (!pr_fields.empty() && pr_fields.size() == 8) {
			int channel = atoi(pr_fields[0].c_str());
			int RSSI = atoi(pr_fields[1].c_str());
			int sec = atoi(pr_fields[4].c_str());
			int usec = atoi(pr_fields[5].c_str());
			if (pr_fields.size() == N_FIELDS) {
				//There is SSID field
				SensorData data(IDSensor, channel, RSSI, sec, usec, pr_fields[2], pr_fields[3], pr_fields[6], pr_fields[7]);
				//Modifiche apportate per controllare la corretta ricezione dei tag dalle schede
				//Insert into DB
				if (!insertData(conn, data)) {
					cout << "Error during the insertion!" << endl;      //DA REINSERIRE
				}else{
					//data.printData();
				}
			}
			else {
				SensorData data(IDSensor, channel, RSSI, sec, usec, pr_fields[2], pr_fields[3], "", pr_fields[7]);
				//data.printData();
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

bool setup_connection(MYSQL* conn) {

	//Set encrypted connection using SSL
	mysql_ssl_set(conn, PRIVATE_KEY_CLIENT, PUBLIC_CLIENT_KEY, CERTIFICATE_AUTORITY_FILE, NULL, NULL);

	// Establish a MySQL connection
	if (!mysql_real_connect(
		conn,
		MY_HOSTNAME, MY_USERNAME,
		MY_PASSWORD, MY_DATABASE,
		MY_PORT_NO, MY_SOCKET, MY_OPT)) {
		cerr << mysql_error(conn) << endl;
		return false;
	}

	return true;
}

bool insertData(MYSQL* conn, SensorData data) {

	ostringstream str;

	str << "INSERT INTO SensorMeasure (IDboard,HashPacket,MACsender,SSID,Timestamp,RSSI,Channel,ControlSequence,Fingerprint) VALUES " << data.createValues();

	//cout << str.str() << endl;

	//cout << data.createValues();

	if (mysql_query(conn, str.str().c_str())) {
		cerr << mysql_error(conn) << endl;
		return false;
	}

	return true;
}

void cleanBoard(char IDSensor, MYSQL* conn){

	ostringstream str;

	str << "DELETE FROM Coordinates WHERE IDboard ='" << IDSensor << "';";

	if (mysql_query(conn, str.str().c_str())) {
		cerr << mysql_error(conn) << endl;
	}
	
	mysql_close(conn);

	cout << "Board " << IDSensor << " disconnected!" << endl;

	return;
}

