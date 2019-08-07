#define _CRT_SECURE_NO_WARNINGS
#define SECURITY_WIN32
#pragma warning(push)
#pragma warning(disable : 6054)

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <fstream>
#include <iostream>
#include <locale>
#include <regex>
#include <string>
#include <vector>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/applink.c>

#include "HttpClient.h"

namespace Common {

	std::string HttpClient::Receive(char* url) {

		std::string stringBuffer;
		auto recvSizeTotal = 0LU;

		char httpString[UrlMaxSize];
		char serverString[UrlMaxSize];
		char pathString[UrlMaxSize];
		char httpGetString[UrlMaxSize];

		memset(httpString, '\0', sizeof(httpString));
		memset(serverString, '\0', sizeof(serverString));
		memset(pathString, '\0', sizeof(pathString));
		memset(httpGetString, '\0', sizeof(httpGetString));

		// Split Url
		if (!SplitUrl(url, httpString, serverString, pathString)) {

			stringBuffer = CreateErrorString((char*)"Url is invalid.");

			return stringBuffer;
		}
		
		// Create "GET" string 
		CreateHttpGetString(serverString, pathString, httpGetString);

		addrinfo resource;
		SOCKET sock;

		// Determine either Https or Http
		auto isHttps = DetermineHttps(url);

		if (!isHttps) {

			// Open socket (http)
			if (!OpenSocket(serverString, (char*)HttpPort, &resource, &sock)) {

				stringBuffer = CreateErrorString((char*)"Failed to resolve host name or create socket. (http)");

				return stringBuffer;
			}

			/// Send data (http)
			if (!SendData(&sock, httpGetString, (unsigned long)strlen(httpGetString))) {

				stringBuffer = CreateErrorString((char*)"Failed to send message. (http)");
				
				return stringBuffer;
			}

			// Receive data (http)
			if (!RecvData(&sock, &stringBuffer, &recvSizeTotal)) {

				stringBuffer = CreateErrorString((char*)"Failed to receive message. (http)");

				return stringBuffer;
			}
		}
		else {

			// Open socket (https)
			if (!OpenSocket(serverString, (char*)HttpsPort, &resource, &sock)) {

				stringBuffer = CreateErrorString((char*)"Failed to resolve host name or create socket. (https)");

				return stringBuffer;
			}

			auto ctx = (SSL_CTX*)nullptr;
			auto ssl = (SSL*)nullptr;

			// Setup Ssl (https)
			if (!SetupSsl(&sock, &ctx, &ssl)) {

				stringBuffer = CreateErrorString((char*)"Failed to negotiate server. (https)");

				return stringBuffer;
			}

			// Send data (https)
			SendDataSsl(ssl, httpGetString, (int)strlen(httpGetString));

			// Receive data (https)
			if (!RecvDataSsl(&sock, ctx, ssl, &stringBuffer, &recvSizeTotal)) {

				stringBuffer = CreateErrorString((char*)"Failed to receive message. (https)");

				return stringBuffer;
			}

			// Close Ssl (https)
			CloseSsl(ctx, ssl);
		}

		// Close socket
		CloseSocket(&sock);

		// Get return code
		auto httpReturnCode = GetHttpReturnCode(&stringBuffer);

		if (httpReturnCode != 200) {
			
			stringBuffer = CreateErrorString("Server returns error code. (%d)", httpReturnCode);

			return stringBuffer;
		}

		// Cut http header
		if (!CutHttpHeader(&stringBuffer)) {

			return stringBuffer;
		}

#ifdef _DEBUG
		std::ofstream wf;
		wf.open("log.txt", std::ios::out);
		wf << stringBuffer;
#endif

		return stringBuffer;
	}

	bool HttpClient::SplitUrl(char* url, char* httpString, char* serverString, char* pathString) {

		char work[UrlMaxSize];
		
		memset(work, '\0', sizeof(work));

		char* token;

		memcpy(work, url, UrlMaxSize);

		// Cut http string
		if ((token = strtok(work, HttpAddressString)) != nullptr) {

			memcpy(httpString, token, strlen(token));

			// Cut server string
			if ((token = strtok(NULL, HttpSlashString)) != nullptr) {

				memcpy(serverString, token, strlen(token));

				// Cut url string
				if ((token = strtok(NULL, "")) != nullptr) {

					memcpy(pathString, token, strlen(token));

					return true;
				}
			}
		}

		return false;
	}

	void HttpClient::CreateHttpGetString(char* serverString, char* pathString, char* httpGetString) {

		// Create get string 
		sprintf(httpGetString, HttpGetString, pathString, serverString);
	}

	std::string HttpClient::CreateErrorString(const char* errorMessage, ...) {

		std::string retVal;

		char temp[512];
		memset(temp, '\0', sizeof(temp));

		// Create error string 
		va_list args;
		va_start(args, errorMessage);
		vsprintf(temp, errorMessage, args);
		va_end(args);

		char buffer[1024];
		memset(buffer, '\0', sizeof(buffer));

		sprintf(buffer, ErrorString, temp);

		retVal = buffer;

		return retVal;
	}

	bool HttpClient::DetermineHttps(char* url) {

		auto* tmpPtr = strstr(url, HttpsString);

		if (tmpPtr != nullptr) {

			return true;
		}

		return false;
	}

	bool HttpClient::OpenSocket(char* serverString, char* port, addrinfo* resource, SOCKET* sock) {

		// Start up
		WSADATA wsaData;

		auto isStartup = WSAStartup(MAKEWORD(2, 0), &wsaData);

		if (isStartup != 0) {

			return false;
		}

		addrinfo hint;

		ZeroMemory(&hint, sizeof(hint));
		hint.ai_family = AF_INET;
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_protocol = IPPROTO_TCP;

		// Create address info
		auto retVal = getaddrinfo(serverString, port, &hint, &resource);
		
		if (retVal != 0) {

			WSACleanup();

			return false;
		}

		// Open socket
		*sock = socket(resource->ai_family, resource->ai_socktype, resource->ai_protocol);

		if (connect(*sock, resource->ai_addr, (int)resource->ai_addrlen) != 0) {

			freeaddrinfo(resource);
			
			WSACleanup();

			return false;
		}

		freeaddrinfo(resource);

		return true;
	}

	bool HttpClient::SendData(SOCKET* sock, char* data, unsigned long length) {

		auto sendSizeTotal = 0LU;

		do {

			// Send data
			auto sendSize = send(*sock, data, length, 0);

			if (sendSize < 0) {
				
				CloseSocket(sock);

				return false;
			}
		
			sendSizeTotal += sendSize;

		} while (sendSizeTotal < length);

		return true;
	}

	bool HttpClient::RecvData(SOCKET* sock, std::string* data, unsigned long* recvSizeTotal) {

		while (true) {

			char charBuffer[HttpBufferSize];
			memset(charBuffer, '\0', sizeof(charBuffer));
			
			// Receive data
			auto recvSize = recv(*sock, charBuffer, sizeof(charBuffer) - 1, 0);

			if (recvSize == 0) {
				break;
			}
			else if (recvSize < 0) {

				CloseSocket(sock);

				return false;
			}

			*data += charBuffer;
			*recvSizeTotal += recvSize;
		}

		return true;
	}

	bool HttpClient::SetupSsl(SOCKET* sock, SSL_CTX** ctx, SSL** ssl) {

		SSL_load_error_strings();
		SSL_library_init();

		*ctx = SSL_CTX_new(TLS_client_method());
		SSL_CTX_set_min_proto_version(*ctx, TLS1_1_VERSION);
		*ssl = SSL_new(*ctx);
		SSL_set_options(*ssl, SSL_OP_NO_TICKET);
		
		auto retVal = SSL_set_fd(*ssl, (int)*sock);
		
		if (retVal != 1) {
			return false;
		}
		
		retVal = SSL_connect(*ssl);

		if (retVal != 1) {
			return false;
		}

		return true;
	}

	void HttpClient::SendDataSsl(SSL* ssl, char* data, int length) {

		SSL_write(ssl, data, length);
	}

	bool HttpClient::RecvDataSsl(SOCKET* sock, SSL_CTX* ctx, SSL* ssl, std::string* data, unsigned long* recvSizeTotal) {

		while (true) {

			char charBuffer[HttpBufferSize];
			memset(charBuffer, '\0', sizeof(charBuffer));

			// Receive data
			auto recvSize = SSL_read(ssl, charBuffer, sizeof(charBuffer) - 1);

			if (recvSize == 0) {
				break;
			}
			else if (recvSize < 0) {

				CloseSsl(ctx, ssl);
				CloseSocket(sock);

				return false;
			}

			*data += charBuffer;
			*recvSizeTotal += recvSize;
		}

		return true;
	}

	int HttpClient::GetHttpReturnCode(std::string* data) {

		// Cut http header
		auto retVal = 0;

		std::regex regexp(HttpCodeRegex);
		std::match_results<const char*> results;

		std::regex_search(data->c_str(), results, regexp, std::regex_constants::match_default);
		std::match_results<const char*>::const_iterator it = results.begin();
		
		if (it != results.end()) {
			retVal = atoi(it->str().c_str());
		}

		return retVal;
	}

	bool HttpClient::CutHttpHeader(std::string* data) {

		// Cut http header
		auto pos = data->find(HttpDelimString);
		*data = data->substr(pos + (unsigned long)sizeof(HttpDelimString) - 1LU);

		return true;
	}

	void HttpClient::CloseSsl(SSL_CTX* ctx, SSL* ssl) {

		SSL_shutdown(ssl);
		SSL_free(ssl);
		SSL_CTX_free(ctx);
		ERR_free_strings();
	}

	void HttpClient::CloseSocket(SOCKET* sock) {

		closesocket(*sock);
		WSACleanup();
	}
}
#pragma warning(pop)
