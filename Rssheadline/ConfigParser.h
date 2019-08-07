#pragma once
#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")
#include <vector>
#include "ConfigItem.h"

namespace Common {

	// Rss parser
	class ConfigParser
	{
	public:

		// Parse Rss
		std::vector<ConfigItem> ParseConfig(TCHAR*);
	};
}
