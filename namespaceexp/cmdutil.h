#pragma once
#include <string>
#include <vector>
#include <tchar.h>
#include <Windows.h>
#include "ntutil.h"

class CommandLineParser {
public:
    int GetArgParam(int argc, _TCHAR** argv, PCWSTR elem, PCWSTR& result);
    bool ArgParamExists(int argc, _TCHAR** argv, PCWSTR elem);
};


int PrintHelp();
int PrintDirectoryObjectsInfo(std::vector<OBJECT_DIRECTORY_INFORMATION> entries);
int PrintSymbolicLinkInfo(PUNICODE_STRING link);
int PrintMutantInfo(PMUTANT_BASIC_INFORMATION mutant);
int PrintSectionInfo(PSECTION_BASIC_INFORMATION section);
int PrintEventInfo(PEVENT_BASIC_INFORMATION eventObj);
int PrintSemaphoreInfo(PSEMAPHORE_BASIC_INFORMATION semaphore);
int PrintTimerInfo(PTIMER_BASIC_INFORMATION timer);
