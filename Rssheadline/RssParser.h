#pragma once
#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")
#include <vector>
#include "RssItem.h"

namespace Common {

	// Rss parser
	class RssParser
	{
	public:

		// Parse Rss
		std::vector<RssItem> ParseRss(std::string);
	};
}
