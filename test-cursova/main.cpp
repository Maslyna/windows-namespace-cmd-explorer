#include <Windows.h>
#include <tchar.h>
#include <cstdio>
#include <winternl.h>
#include <vector>
#include <string>
#include <algorithm>
#include "ntutil.h"
#include "cmdutil.h"

BYTE buffer[BUFFER_SIZE]{};
wchar_t strbuff[MAX_PATH];

int PrintDirectoryObjectsInfo(std::vector<OBJECT_DIRECTORY_INFORMATION> entries);
int PrintSymbolicLinkInfo(PUNICODE_STRING link) {
	std::wstring formatString = L"| %-" + std::to_wstring(link->Length) + L"s |\n";
	_tprintf(_T("%s\n"), std::wstring(link->Length + 4, '-').c_str());
	_tprintf(formatString.c_str(), L"Link to");
	_tprintf(_T("%s\n"), std::wstring(link->Length + 4, '-').c_str());
	_tprintf(formatString.c_str(), link->Buffer);
	_tprintf(_T("%s\n"), std::wstring(link->Length + 4, '-').c_str());
	return 0;
}

int _tmain(int argc, _TCHAR** argv)
{
	if (InitNtModule() != 0) {
		_tprintf(_T("Failed to retrieve ntdll.dll function pointers\n"));
		return 1;
	}
	CommandLineParser parser;
	PCWSTR path = nullptr;
	PCWSTR strtype = L"";
	OBJECT_TYPE type = DIRECTORY_OBJECT;

	NTSTATUS ntStatus;
	OBJECT_ATTRIBUTES oa{};
	UNICODE_STRING objname;
	HANDLE hObj = NULL;

	if (parser.GetArgParam(argc, argv, L"-p", path) != 0) {
		path = L"\\";
	}
	if (parser.ArgParamExists(argc, argv, L"--open") != 0) {
		if (parser.GetArgParam(argc, argv, L"--type", strtype) != 0) {
			if (GetObjectType(strtype, type) != 0) {
				_tprintf(_T("Incorrect object type: %s"), strtype);
				return 1;
			}
		}
		else {
			_tprintf(_T("With param '--open' param '--type' must be spetify"));
			return 1;
		}
	}

	ntStatus = NtOpenObject(type, hObj, oa, MAXIMUM_ALLOWED, path);
	if (!NT_SUCCESS(ntStatus)) {
		_tprintf(_T("Failed to read Directory Objects\n"));

		(void)NtClose_(hObj);
		return 1;
	}

	switch (type)
	{
	case DIRECTORY_OBJECT: {
		std::vector<OBJECT_DIRECTORY_INFORMATION> entries;
		ProcessDirectoryObjects(ntStatus, hObj, buffer, entries);
		PrintDirectoryObjectsInfo(entries);
	}
		break;
	case SYMBOLICLINK_OBJECT: {
		UNICODE_STRING str{};
		ULONG bytesNeeded{};
		str.Buffer = strbuff;
		str.Length = NULL;
		str.MaximumLength = MAX_PATH;
		ntStatus = NtQuerySymbolicLinkObject(hObj, &str, &bytesNeeded);
		
		if (!NT_SUCCESS(ntStatus)) {
			_tprintf(_T("Failed to read symbolic link object %#x"), ntStatus);
			(void)NtClose_(hObj);
			return 1;
		}
		PrintSymbolicLinkInfo(&str);
	}
		break;
	case MUTANT_OBJECT: {
		MUTANT_BASIC_INFORMATION result{};
		ntStatus = NtQueryMutant(hObj, MutantBasicInformation, &result, sizeof(MUTANT_BASIC_INFORMATION), 0);
		if (!NT_SUCCESS(ntStatus)) {
			_tprintf(_T("Failed to read mutant object %#x"), ntStatus);
			(void)NtClose_(hObj);
			return 1;
		}
	}
		break;
	case SECTION_OBJECT:
		break;
	case EVENT_OBJECT:
		break;
	case SEMAPHORE_OBJECT:
		break;
	case TIMER_OBJECT:
		break;
	case KEY_OBJECT:
		break;
	case EVENTPAIR_OBJECT:
		break;
	case IOCOMPLETION_OBJECT:
		break;
	case FILE_OBJECT:
		break;
	default:
		break;
	}

	(void)NtClose_(hObj);
	return 0;
}

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

