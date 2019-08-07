#define _CRT_SECURE_NO_WARNINGS
#pragma warning(push)
#pragma warning(disable : 6387)

#include <atlbase.h>
#include <atlstr.h>
#include "ConfigConstant.h"
#include "ConfigParser.h"

namespace Common {

	std::vector<ConfigItem> ConfigParser::ParseConfig(TCHAR* configFilePath) {

		std::vector<ConfigItem> result;

		// Create xml reader
		auto pReader = (CComPtr<IXmlReader>)nullptr;
		if (FAILED(CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), 0))) {
			MessageBox(NULL, _T("CreateXmlReader failed"), _T("Warning"), MB_OK | MB_ICONWARNING);
			return result;
		}

		// Create xml file path
		TCHAR xml[MAX_PATH];
		GetModuleFileName(NULL, xml, sizeof(xml) / sizeof(TCHAR));
		PathRemoveFileSpec(xml);
		PathAppend(xml, configFilePath);

		// Create file stream
		auto pStream = (CComPtr<IStream>)nullptr;
		if (FAILED(SHCreateStreamOnFile(xml, STGM_READ, &pStream))) {
			MessageBox(NULL, _T("SHCreateStreamOnFile failed"), _T("Warning"), MB_OK | MB_ICONWARNING);
			return result;
		}

		// Set file stream
		if (FAILED(pReader->SetInput(pStream))) {
			MessageBox(NULL, _T("SetInput failed"), _T("Warning"), MB_OK | MB_ICONWARNING);
			return result;
		}

		XmlNodeType nodeType;
		auto elementName = (LPCWSTR)nullptr;
		auto elementValue = (LPCWSTR)nullptr;
		auto* configItem = new ConfigItem();
		auto* tempUrl = new TCHAR[BufferLengthUrl]();

		while (S_OK == pReader->Read(&nodeType)) {

			switch (nodeType) {

			case XmlNodeType_Element:

				// Get element name
				if (FAILED(pReader->GetLocalName(&elementName, NULL))) {
					MessageBox(NULL, _T("Can't get element name"), _T("Warning"), MB_OK | MB_ICONWARNING);
					return result;
				}

				break;

			case XmlNodeType_Text:

				// Get element value
				if (FAILED(pReader->GetValue(&elementValue, NULL))) {
					MessageBox(NULL, _T("Can't get element value"), _T("Warning"), MB_OK | MB_ICONWARNING);
					return result;
				}

				// Url
				if (lstrcmp(elementName, _T("url")) == 0) {

					// Create entity
					configItem = new ConfigItem();

					// Set url
					tempUrl = new TCHAR[BufferLengthUrl]();
					_tcscpy(tempUrl, elementValue);
					configItem->SetUrl(tempUrl);

					// Push entity
					result.push_back(*configItem);
				}

				break;
			}
		}

		return result;
	}
}
#pragma warning(pop)
