#include "cmdutil.h"

int CommandLineParser::GetArgParam(int argc, _TCHAR** argv, PCWSTR elem, PCWSTR& result) {
    for (int i = 1; i < argc; i++) {
        if (_tcscmp(argv[i], elem) == 0 && i + 1 < argc) {
            result = argv[i + 1];
            return 0;
        }
    }
    return 1;
}

bool CommandLineParser::ArgParamExists(int argc, _TCHAR** argv, PCWSTR elem) {
    for (int i = 1; i < argc; i++) {
        if (_tcscmp(argv[i], elem) == 0) {
            return true;
        }
    }
    return false;
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
	_tprintf(_T("Event type:\t%s\n"), type.c_str());
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