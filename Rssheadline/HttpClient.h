#pragma once
#define SECURITY_WIN32

#include <sspi.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "HttpConstant.h"

namespace Common {

	/// Http client
	class HttpClient
	{
	private:

		struct _SecHandle Credential;
		struct _SecHandle SecureHandle;

		/// Separate Url
		bool SplitUrl(char*, char*, char*, char*);

		/// Create "GET" string 
		void CreateHttpGetString(char*, char*, char*);

		/// Create Error string 
		std::string CreateErrorString(const char*, ...);

		/// Determine either Https or Http
		bool DetermineHttps(char*);

		/// Open socket
		bool OpenSocket(char*, char*, struct addrinfo*, SOCKET*);

		/// Setup Ssl (https)
		bool SetupSsl(SOCKET*, SSL_CTX**, SSL**);

		/// Send data (http)
		bool SendData(SOCKET*, char*, unsigned long);

		/// Receive data (http)
		bool RecvData(SOCKET*, std::string*, unsigned long*);

		/// Send data (https)
		void SendDataSsl(SSL*, char*, int);

		/// Receive data (https)
		bool RecvDataSsl(SOCKET*, SSL_CTX*, SSL*, std::string*, unsigned long*);

		/// Get http return code
		int GetHttpReturnCode(std::string*);

		/// Cut http header
		bool CutHttpHeader(std::string*);

		/// Receive data (https)
		void CloseSsl(SSL_CTX*, SSL*);

		/// Close socket
		void CloseSocket(SOCKET*);

	public:

		/// Receive http contents
		std::string Receive(char*);
	};
}
