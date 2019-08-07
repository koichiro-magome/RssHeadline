#define _CRT_SECURE_NO_WARNINGS
#pragma warning(push)
#pragma warning(disable : 6387)

#include <atlbase.h>
#include <atlstr.h>
#include <string>

#include "RssConstant.h"
#include "RssParser.h"

namespace Common {

	std::vector<RssItem> RssParser::ParseRss(std::string rss) {

		std::vector<RssItem> result;

		// Create xml reader
		auto pReader = (CComPtr<IXmlReader>)nullptr;
		if (FAILED(CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), 0))) {
			MessageBox(NULL, _T("CreateXmlReader failed"), _T("Warning"), MB_OK | MB_ICONWARNING);
			return result;
		}

		// Create file stream
		CComPtr<IStream> pStream;
		pStream.Attach(SHCreateMemStream((BYTE*)rss.c_str(), (unsigned int)rss.length()));
		if (pStream == nullptr) {
			MessageBox(NULL, _T("SHCreateStreamOnFile failed"), _T("Warning"), MB_OK | MB_ICONWARNING);
			return result;
		}

		// Set file stream
		if (FAILED(pReader->SetInput(pStream))) {
			MessageBox(NULL, _T("SetInput failed"), _T("Warning"), MB_OK | MB_ICONWARNING);
			return result;
		}

		XmlNodeType nodeType;
		HRESULT hr;
		auto elementName = (LPCWSTR)nullptr;
		auto elementValue = (LPCWSTR)nullptr;
		auto attributeName = (LPCWSTR)nullptr;
		auto attributeValue = (LPCWSTR)nullptr;
		auto* rssItem = new RssItem();
		auto* tempTitle = new TCHAR[BufferLengthTitle]();
		auto* tempLink = new TCHAR[BufferLengthLink]();
		auto isPreserveTitle = false;

		while (S_OK == pReader->Read(&nodeType)) {

			switch (nodeType) {

			case XmlNodeType_Element:

				// Get element name
				if (FAILED(pReader->GetLocalName(&elementName, NULL))) {
					MessageBox(NULL, _T("GetLocalName failed"), _T("Warning"), MB_OK | MB_ICONWARNING);
					return result;
				}

				hr = pReader->MoveToFirstAttribute();

				if (S_FALSE == hr) {
					break;
				}

				if (S_OK != hr) {
					MessageBox(NULL, _T("MoveToFirstAttribute failed"), _T("Warning"), MB_OK | MB_ICONWARNING);
					return result;
				}

				do {
					
					if (FAILED(pReader->GetLocalName(&attributeName, NULL))) {
						MessageBox(NULL, _T("GetLocalName failed"), _T("Warning"), MB_OK | MB_ICONWARNING);
						return result;
					}
					
					if (FAILED(pReader->GetValue(&attributeValue, NULL))) {
						MessageBox(NULL, _T("GetValue failed"), _T("Warning"), MB_OK | MB_ICONWARNING);
						return result;
					}
					
					if (isPreserveTitle
						&& lstrcmp(attributeName, _T("href")) == 0) {

						// Set link
						tempLink = new TCHAR[BufferLengthLink]();
						_tcscpy(tempLink, attributeValue);
						rssItem->SetLink(tempLink);

						// Push entity
						result.push_back(*rssItem);
					
						isPreserveTitle = false;
					
						break;
					}
				
				} while (S_OK == pReader->MoveToNextAttribute());

				break;

			case XmlNodeType_Text:
			case XmlNodeType_CDATA:

				// Get element value
				if (FAILED(pReader->GetValue(&elementValue, NULL))) {
					MessageBox(NULL, _T("GetValue failed"), _T("Warning"), MB_OK | MB_ICONWARNING);

					return result;
				}

				// Title
				if (lstrcmp(elementName, _T("title")) == 0) {

					// Create entity
					rssItem = new RssItem();

					// Set title
					lstrcat(tempTitle, _T(" "));
					tempTitle = new TCHAR[BufferLengthTitle]();
					_tcscpy(tempTitle, elementValue);
					rssItem->SetTitle(tempTitle);

					isPreserveTitle = true;
				}
				// Link
				else if (isPreserveTitle
						 && lstrcmp(elementName, _T("link")) == 0) {

					// Set link
					tempLink = new TCHAR[BufferLengthLink]();
					_tcscpy(tempLink, elementValue);
					rssItem->SetLink(tempLink);

					// Push entity
					result.push_back(*rssItem);

					isPreserveTitle = false;
				}

				break;
			}
		}

		return result;
	}
}
#pragma warning(pop)
