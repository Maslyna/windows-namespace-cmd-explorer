#pragma once

#include <vector>
#include <Windows.h>
#include <winternl.h>
#include "ntnames.h"
#include "ntobj.h"
#include "ntfunctions.h"

typedef enum OBJECT_TYPE {
	DIRECTORY_OBJECT,
	SYMBOLICLINK_OBJECT,
	MUTANT_OBJECT,
	SECTION_OBJECT,
	EVENT_OBJECT,
	SEMAPHORE_OBJECT,
	TIMER_OBJECT,
	KEY_OBJECT,
	EVENTPAIR_OBJECT,
	IOCOMPLETION_OBJECT,
	FILE_OBJECT
};


// INIT FUNCTION

#ifndef INIT_NT_MODULE_H
#define INIT_NT_MODULE_H
int InitNtModule() {
	HMODULE hNtDll = ::GetModuleHandle(_T("ntdll.dll"));

	*(FARPROC*)&NtOpenDirectoryObject = ::GetProcAddress(hNtDll, "NtOpenDirectoryObject");
	*(FARPROC*)&NtOpenSymbolicLinkObject = ::GetProcAddress(hNtDll, "NtOpenSymbolicLinkObject");
	*(FARPROC*)&NtOpenMutant = ::GetProcAddress(hNtDll, "NtOpenMutant");
	*(FARPROC*)&NtOpenSection = ::GetProcAddress(hNtDll, "NtOpenSection");
	*(FARPROC*)&NtOpenEvent = ::GetProcAddress(hNtDll, "NtOpenEvent");
	*(FARPROC*)&NtOpenSemaphore = ::GetProcAddress(hNtDll, "NtOpenSemaphore");
	*(FARPROC*)&NtOpenTimer = ::GetProcAddress(hNtDll, "NtOpenTimer");
	
	//*(FARPROC*)&NtQueryObject = ::GetProcAddress(hNtDll, "NtQueryObject");
	*(FARPROC*)&NtQueryDirectoryObject = ::GetProcAddress(hNtDll, "NtQueryDirectoryObject");
	*(FARPROC*)&NtQuerySymbolicLinkObject = ::GetProcAddress(hNtDll, "NtQuerySymbolicLinkObject");
	*(FARPROC*)&NtQueryMutant = ::GetProcAddress(hNtDll, "NtQueryMutant");
	*(FARPROC*)&NtQuerySection = ::GetProcAddress(hNtDll, "NtQuerySection");
	*(FARPROC*)&NtQueryEvent = ::GetProcAddress(hNtDll, "NtQueryEvent");
	*(FARPROC*)&NtQuerySemaphore = ::GetProcAddress(hNtDll, "NtQuerySemaphore");
	*(FARPROC*)&NtQueryTimer = ::GetProcAddress(hNtDll, "NtQueryTimer");


	*(FARPROC*)&RtlInitUnicodeString_ = ::GetProcAddress(hNtDll, "RtlInitUnicodeString");
	*(FARPROC*)&NtClose_ = ::GetProcAddress(hNtDll, "NtClose");

	if (!NtOpenDirectoryObject || !RtlInitUnicodeString_ || !NtClose_
		|| !NtOpenSymbolicLinkObject || !NtOpenMutant || !NtOpenSection || !NtOpenEvent || !NtOpenSemaphore|| !NtOpenTimer) {
		return 1;
	}
	if (!NtQueryDirectoryObject || !NtQuerySymbolicLinkObject || !NtQueryMutant
		|| !NtQuerySection || !NtQueryEvent || !NtQuerySemaphore || !NtQueryTimer) {
		return 1;
	}
	return 0;
}

#endif // !INIT_NT_MODULE_H

#ifndef OPEN_DIRECTORY_OBJECT_H
#define OPEN_DIRECTORY_OBJECT_H
int OpenDirectoryObject(NTSTATUS& ntStatus, OBJECT_ATTRIBUTES& oa, UNICODE_STRING& objname, HANDLE& hDeviceDir, PCWSTR path) {
	RtlInitUnicodeString_(&objname, path);
	InitializeObjectAttributes(&oa, &objname, 0, NULL, NULL);
	ntStatus = NtOpenDirectoryObject(&hDeviceDir, DIRECTORY_QUERY | DIRECTORY_TRAVERSE, &oa);

	if (!NT_SUCCESS(ntStatus)) {
		delete hDeviceDir;
		return 1;
	}

	return 0;
}
#endif // OPEN_DIRECTORY_OBJECT_H

#ifndef PROCESS_DIRECTORY_OBJECT_H
#define PROCESS_DIRECTORY_OBJECT_H

int ProcessDirectoryObjects(NTSTATUS& ntStatus, HANDLE& hDeviceDir, BYTE* buf, std::vector<OBJECT_DIRECTORY_INFORMATION>& entries) {
	ULONG start = 0, idx = 0, bytes;
	BOOLEAN restart = TRUE;

	while (true) {
		ntStatus = NtQueryDirectoryObject(hDeviceDir, buf, BUFFER_SIZE, FALSE, restart, &idx, &bytes);

		if (NT_SUCCESS(ntStatus)) {
			POBJECT_DIRECTORY_INFORMATION const pdilist = reinterpret_cast<POBJECT_DIRECTORY_INFORMATION>(PBYTE(buf));
			for (ULONG i = 0; i < idx - start; i++)
			{
				entries.push_back(pdilist[i]);
			}
		}

		if (STATUS_MORE_ENTRIES == ntStatus)
		{
			start = idx;
			restart = FALSE;
			continue;
		}

		if ((ntStatus == STATUS_SUCCESS) || (ntStatus == STATUS_NO_MORE_ENTRIES))
		{
			return 0;
		}
	}
	
	return 0;
}
#endif // !PROCESS_DIRECTORY_OBJECT_H

#ifndef PROCESS_SYMBOLIC_LINK_OBJECT_H
#define PROCESS_SYMBOLIC_LINK_OBJECT_H
NTSTATUS ProcessSymbolicLinkObject(HANDLE& handle, UNICODE_STRING& link) {
	return NtQuerySymbolicLinkObject(&handle, &link, NULL);
}
#endif // ! PROCESS_SYMBOLIC_LINK_OBJECT_H


#ifndef NT_OPEN_OBJECT_H
#define NT_OPEN_OBJECT_H
NTSTATUS NtOpenObject(OBJECT_TYPE type, HANDLE& handle, OBJECT_ATTRIBUTES& openStruct, ACCESS_MASK access, LPCWSTR path) {
	UNICODE_STRING ustr;

	RtlInitUnicodeString_(&ustr, path);

	InitializeObjectAttributes(&openStruct, &ustr, OBJ_CASE_INSENSITIVE, NULL, NULL);

	switch (type)
	{
	case DIRECTORY_OBJECT:      return NtOpenDirectoryObject(&handle, access, &openStruct);
	case SYMBOLICLINK_OBJECT:   return NtOpenSymbolicLinkObject(&handle, access, &openStruct);
	case MUTANT_OBJECT:         return NtOpenMutant(&handle, access, &openStruct);
	case SECTION_OBJECT:        return NtOpenSection(&handle, access, &openStruct);
	case EVENT_OBJECT:          return NtOpenEvent(&handle, access, &openStruct);
	case SEMAPHORE_OBJECT:      return NtOpenSemaphore(&handle, access, &openStruct);
	case TIMER_OBJECT:          return NtOpenTimer(&handle, access, &openStruct);
	default:
		return ERROR_INVALID_FUNCTION;
	}
}
#endif // !NT_OPEN_OBJECT_H

#ifndef GET_NT_OBJECT_TYPE_H
#define GET_NT_OBJECT_TYPE_H
int GetObjectType(PCWSTR objectTypeStr, OBJECT_TYPE& result)
{
	if		(wcscmp(objectTypeStr, L"Directory") == 0)			result = DIRECTORY_OBJECT;
	else if (wcscmp(objectTypeStr, L"SymbolicLink") == 0)		result = SYMBOLICLINK_OBJECT;
	else if (wcscmp(objectTypeStr, L"Mutant") == 0)				result = MUTANT_OBJECT;
	else if (wcscmp(objectTypeStr, L"Section") == 0)			result = SECTION_OBJECT;
	else if (wcscmp(objectTypeStr, L"Event") == 0)				result = EVENT_OBJECT;
	else if (wcscmp(objectTypeStr, L"Semaphore") == 0)			result = SEMAPHORE_OBJECT;
	else if (wcscmp(objectTypeStr, L"Timer") == 0)				result = TIMER_OBJECT;
	else return -1;
	return 0;
}
#endif