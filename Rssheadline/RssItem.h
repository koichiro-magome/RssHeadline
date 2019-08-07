#pragma once

#include <tchar.h>

namespace Common {

	/// Rss item
	class RssItem
	{
	private:

		TCHAR* Title = nullptr;
		TCHAR* Link = nullptr;
		int Length = 0;
		int Position = 0;

	public:

		TCHAR* GetTitle() {
			return Title;
		};

		void SetTitle(TCHAR* title) {
			Title = title;
		};

		TCHAR* GetLink() {
			return Link;
		};

		void SetLink(TCHAR* link) {
			Link = link;
		};

		int GetLength() {
			return Length;
		};

		void SetLength(int length) {
			Length = length;
		};

		int GetPosition() {
			return Position;
		};

		void SetPosition(int position) {
			Position = position;
		};
	};
}
