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

int PrintHelp();
int PrintDirectoryObjectsInfo(std::vector<OBJECT_DIRECTORY_INFORMATION> entries);
int PrintSymbolicLinkInfo(PUNICODE_STRING link);
int PrintMutantInfo(PMUTANT_BASIC_INFORMATION mutant);
int PrintSectionInfo(PSECTION_BASIC_INFORMATION section);
int PrintEventInfo(PEVENT_BASIC_INFORMATION eventObj);
int PrintSemaphoreInfo(PSEMAPHORE_BASIC_INFORMATION semaphore);
int PrintTimerInfo(PTIMER_BASIC_INFORMATION timer);

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

int PrintHelp() {
	_tprintf(_T("\n\n"));
	_tprintf(_T("\n\n"));
	_tprintf(_T("This program was created by Maslyna (Mykhailo) like project for university in DEC 2023.\n"));
	_tprintf(_T("Please forgive me for the bad code. :( C++ is not for me.\n"));
	_tprintf(_T("\n\n"));
	_tprintf(_T("Command list:\n"));
	_tprintf(_T("\t--help              (-h) : notes about program\n"));
	_tprintf(_T("\t--path *string arg* (-p) : insert path\n"));
	_tprintf(_T("\t--open              (-o) : open specific object\n"));
	_tprintf(_T("\t                         : with this parameter it is necessary to specify the object type\n"));
	_tprintf(_T("\t--type *string arg* (-t) : select object type, what would be opened\n"));
	_tprintf(_T("\n\nTYPE LIST:\n"));
	_tprintf(_T("\tDirectory\n\tSymbolicLink\n\tMutant\n\tSection\n\tEvent\n\tSemaphore\n\tTimer\n\t"));
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

int PrintSymbolicLinkInfo(PUNICODE_STRING link) {
	std::wstring formatString = L"| %-" + std::to_wstring(link->Length) + L"s |\n";
	_tprintf(_T("%s\n"), std::wstring(link->Length + 4, '-').c_str());
	_tprintf(formatString.c_str(), L"Link to");
	_tprintf(_T("%s\n"), std::wstring(link->Length + 4, '-').c_str());
	_tprintf(formatString.c_str(), link->Buffer);
	_tprintf(_T("%s\n"), std::wstring(link->Length + 4, '-').c_str());
	return 0;
}

int PrintMutantInfo(PMUTANT_BASIC_INFORMATION mutant) {
	_tprintf(_T("Current count:\t%d\n"), mutant->CurrentCount);
	_tprintf(_T("Owned by caller:\t%s\n"), mutant->OwnedByCaller ? L"true" : L"false");
	_tprintf(_T("Abandoned state:\t%s\n"), mutant->AbandonedState ? L"true" : L"false");
	return 0;
}

int PrintSectionInfo(PSECTION_BASIC_INFORMATION section) {
	_tprintf(_T("Base:\t%d\n"), reinterpret_cast<int>(section->Base));
	_tprintf(_T("Attributes:\t%d\n"), section->Attributes);
	_tprintf(_T("Size:\t%f\n"), section->Size.QuadPart);
	return 0;
}

int PrintEventInfo(PEVENT_BASIC_INFORMATION eventObj) {
	std::wstring type;
	switch (eventObj->EventType)
	{
	case NotificationEvent:
		type = L"Notification Event";
		break;
	case SynchronizationEvent:
		type = L"Synchorization Event";
		break;
	default:
		type = L"Unknown Event";
		break;
	}
	_tprintf(_T("Event type:\t%s\n"), type);
	_tprintf(_T("Event state:\t%d\n", eventObj->EventState));
	return 0;
}

int PrintSemaphoreInfo(PSEMAPHORE_BASIC_INFORMATION semaphore) {
	_tprintf(_T("Current count:\t%d\n"), semaphore->CurrentCount);
	_tprintf(_T("Maximum count:\t%d\n"), semaphore->MaximumCount);
	return 0;
}

int PrintTimerInfo(PTIMER_BASIC_INFORMATION timer) {
	_tprintf(_T("Remaining time:\t%f\n"), timer->RemainingTime.QuadPart);
	_tprintf(_T("Timer state:\t%s\n"), timer->TimerState ? L"true" : L"false");
	return 0;
}