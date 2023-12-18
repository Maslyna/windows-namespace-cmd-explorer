#include <Windows.h>
#include <tchar.h>
#include <cstdio>
#include <winternl.h>
#include <vector>
#include <string>
#include <algorithm>
#include "ntutil.h"
#include "cmdutil.h"


int PrintDirectoryObjectsInfo(std::vector<OBJECT_DIRECTORY_INFORMATION> entries) {
	int maxNameLength = 0;
	int maxTypeLength = 0;

	for (size_t i = 0; i < entries.size(); i++) {
		int nameLength = wcslen(entries[i].Name.Buffer);
		int typeLength = wcslen(entries[i].TypeName.Buffer);
		if (nameLength > maxNameLength) {
			maxNameLength = nameLength;
		}
		if (typeLength > maxTypeLength) {
			maxTypeLength = typeLength;
		}
	}

	std::wstring formatString = L"| %-" + std::to_wstring(maxNameLength) + L"s | %-" + std::to_wstring(maxTypeLength) + L"s |\n";

	_tprintf(_T("%s\n"), std::wstring(maxNameLength + maxTypeLength + 7, '-').c_str());
	_tprintf(formatString.c_str(), _T("Name"), _T("TypeName"));
	_tprintf(_T("%s\n"), std::wstring(maxNameLength + maxTypeLength + 7, '-').c_str());
	for (size_t i = 0; i < entries.size(); i++) {
		_tprintf(formatString.c_str(), entries[i].Name.Buffer, entries[i].TypeName.Buffer);
	}
	_tprintf(_T("%s"), std::wstring(maxNameLength + maxTypeLength + 7, '-').c_str());

	return 0;
}



int _tmain(int argc, _TCHAR** argv)
{
	if (InitNtModule() != 0) {
		_tprintf(_T("Failed to initialise Nt Module\n"));
		return 1;
	}
	CommandLineParser parser;
	PCWSTR path = nullptr;
	
	if (parser.GetArgParam(argc, argv, L"-p", path) != 0) {
		path = L"\\";
	}


	NTSTATUS ntStatus;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING objname;
	HANDLE hDeviceDir = NULL;
	BYTE* buffer = new BYTE[BUFFER_SIZE];
	std::vector<OBJECT_DIRECTORY_INFORMATION> entries;

	if (OpenDirectoryObject(ntStatus, oa, objname, hDeviceDir, path) != 0) {
		_tprintf(_T("Failed to read Directory Objects\n"));

		(void)NtClose_(hDeviceDir);
		delete buffer;
		return 1;
	}

	ProcessDirectoryObjects(ntStatus, hDeviceDir, buffer, entries);
	PrintDirectoryObjectsInfo(entries);

	(void)NtClose_(hDeviceDir);
	delete buffer;
	return 0;
}