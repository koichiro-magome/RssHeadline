#pragma once

#include <windows.h>
#include <tchar.h>

namespace Common {

	// Http buffer size
	const int UrlMaxSize = 256;

	// Http buffer size
	const int HttpBufferSize = 4096 + 1;
	
	// Http string
	const char HttpString[] = "http";

	// Https string
	const char HttpsString[] = "https";

	// Http port
	const char HttpPort[] = "http";

	// Https port
	const char HttpsPort[] = "https";

	// Http address string
	const char HttpAddressString[] = "://";

	// Http address string
	const char HttpSlashString[] = "/";

	// Http delimiter string
	const char HttpDelimString[] = "\r\n\r\n";

	// Http get string
	const char HttpGetString[] = "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\nConnection: keep-alive\r\n\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64; Trident/7.0; Touch; rv:11.0) like Gecko\r\n\r\nAccept: */*\r\n\r\n";

	// Http get string
	const char ErrorString[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><item><title>Error : %s</title><link>about:blank</link></item>";

	// Http code regexp
	const char HttpCodeRegex[] = "[1-9][0-9][0-9]";
}
