#pragma once

#include <tchar.h>

namespace Common {

	/// Config Item
	class ConfigItem
	{
	private:

		TCHAR* Url = nullptr;

	public:

		TCHAR* GetUrl() {
			return Url;
		};

		void SetUrl(TCHAR* url) {
			Url = url;
		};
	};
}
