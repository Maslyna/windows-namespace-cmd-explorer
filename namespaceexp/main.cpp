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
wchar_t strbuff[MAX_PATH]{};


int _tmain(int argc, _TCHAR** argv)
{
	CommandLineParser parser;

	if (parser.ArgParamExists(argc, argv, L"--help") || parser.ArgParamExists(argc, argv, L"-h")) {
		PrintHelp();
		return 0;
	}

	if (InitNtModule() != 0) {
		_tprintf(_T("Failed to retrieve ntdll.dll function pointers\n"));
		return 1;
	}

	PCWSTR path = nullptr;
	PCWSTR strtype = L"";
	OBJECT_TYPE type = DIRECTORY_OBJECT;

	NTSTATUS ntStatus;
	OBJECT_ATTRIBUTES oa{};
	UNICODE_STRING objname;
	HANDLE hObj = NULL;

	if (parser.GetArgParam(argc, argv, L"-p", path) != 0 && parser.GetArgParam(argc, argv, L"--path", path) != 0) {
		path = L"\\";
	}
	_tprintf(_T("\n\nPath: %s\n\n"), path);
	if (parser.ArgParamExists(argc, argv, L"--open") || parser.ArgParamExists(argc, argv, L"-o")) {
		if (parser.GetArgParam(argc, argv, L"--type", strtype) == 0 || parser.GetArgParam(argc, argv, L"-t", strtype) == 0) {
			if (GetObjectType(strtype, type) != 0) {
				_tprintf(_T("Incorrect object type: %s"), strtype);
				return 1;
			}
		}
		else {
			_tprintf(_T("With param '--open' param '--type' must be specified"));
			return 1;
		}
	}

	ntStatus = NtOpenObject(type, hObj, oa, MAXIMUM_ALLOWED, path);
	if (!NT_SUCCESS(ntStatus)) {
		_tprintf(_T("Failed to read Directory Objects %#x\n"), ntStatus);

		(void)NtClose_(hObj);
		return 1;
	}

	switch (type)
	{
	case DIRECTORY_OBJECT: {
		std::vector<OBJECT_DIRECTORY_INFORMATION> entries;
		ProcessDirectoryObjects(ntStatus, hObj, buffer, entries);
		std::sort(entries.begin(), entries.end(), [](const OBJECT_DIRECTORY_INFORMATION& a, const OBJECT_DIRECTORY_INFORMATION& b) {
			return wcscmp(a.TypeName.Buffer, b.TypeName.Buffer) < 0;
		});
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
		ntStatus = NtQueryMutant(hObj, MutantBasicInformation, &result, sizeof(MUTANT_BASIC_INFORMATION), NULL);
		if (!NT_SUCCESS(ntStatus)) {
			_tprintf(_T("Failed to read mutant object %#x"), ntStatus);
			(void)NtClose_(hObj);
			return 1;
		}
		PrintMutantInfo(&result);
	}
		break;
	case SECTION_OBJECT: {
		SECTION_BASIC_INFORMATION result{};
		ntStatus = NtQuerySection(hObj, SectionBasicInformation, &result, sizeof(SECTION_BASIC_INFORMATION), NULL);
		if (!NT_SUCCESS(ntStatus)) {
			_tprintf(_T("Failed to read section object %#x"), ntStatus);
			(void)NtClose_(hObj);
			return 1;
		}
		PrintSectionInfo(&result);
	}
		break;
	case EVENT_OBJECT: {
		EVENT_BASIC_INFORMATION result{};
		ntStatus = NtQueryEvent(hObj, EventBasicInformation, &result, sizeof(EVENT_BASIC_INFORMATION), NULL);
		if (!NT_SUCCESS(ntStatus)) {
			_tprintf(_T("Failed to read event object %#x"), ntStatus);
			(void)NtClose_(hObj);
			return 1;
		}
		PrintEventInfo(&result);
	}
		break;
	case SEMAPHORE_OBJECT: {
		SEMAPHORE_BASIC_INFORMATION result{};
		ntStatus = NtQuerySemaphore(hObj, SemaphoreBasicInformation, &result, sizeof(SEMAPHORE_BASIC_INFORMATION), NULL);
		if (!NT_SUCCESS(ntStatus)) {
			_tprintf(_T("Failed to read semaphore object %#x"), ntStatus);
			(void)NtClose_(hObj);
			return 1;
		}
		PrintSemaphoreInfo(&result);
	}
		break;
	case TIMER_OBJECT: {
		TIMER_BASIC_INFORMATION result{};
		ntStatus = NtQueryTimer(hObj, TimerBasicInformation, &result, sizeof(TIMER_BASIC_INFORMATION), NULL);
		if (!NT_SUCCESS(ntStatus)) {
			_tprintf(_T("Failed to read timer object %#x"), ntStatus);
			(void)NtClose_(hObj);
			return 1;
		}
		PrintTimerInfo(&result);
	}
		break;
	default:
		break;
	}
	(void)NtClose_(hObj);
	return 0;
}
