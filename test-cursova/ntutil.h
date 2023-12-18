#pragma once

#ifndef BUFFER_SIZE
#define BUFFER_SIZE						(0x10000)
#endif 

// NT DIRECTORY

#ifndef DIRECTORY_QUERY					// Query access to the directory object.
#define DIRECTORY_QUERY                 (0x0001)
#endif // !DIRECTORY_QUERY				
#ifndef DIRECTORY_TRAVERSE				// Name-lookup access to the directory object.
#define DIRECTORY_TRAVERSE              (0x0002)
#endif // !DIRECTORY_TRAVERSE			
#ifndef DIRECTORY_CREATE_OBJECT			// Name-creation access to the directory object.
#define DIRECTORY_CREATE_OBJECT			(0x0004)
#endif // !DIRECTORY_CREATE_OBJECT		
#ifndef DIRECTORY_CREATE_SUBDIRECTORY	// Subdirectory-creation access to the directory object.
#define DIRECTORY_CREATE_SUBDIRECTORY	(0x0008)
#endif // !DIRECTORY_CREATE_SUBDIRECTORY


// NT STATUS

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L) // ntsubauth
#endif // STATUS_SUCCESS
#ifndef STATUS_MORE_ENTRIES
#define STATUS_MORE_ENTRIES              ((NTSTATUS)0x00000105L)
#endif // STATUS_MORE_ENTRIES
#ifndef STATUS_NO_MORE_ENTRIES
#define STATUS_NO_MORE_ENTRIES           ((NTSTATUS)0x8000001AL)
#endif // STATUS_NO_MORE_ENTRIES

// NT FUNCTIONS

NTSTATUS(NTAPI* NtOpenDirectoryObject)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
NTSTATUS(NTAPI* NtQueryDirectoryObject)(HANDLE, PVOID, ULONG, BOOLEAN, BOOLEAN, PULONG, PULONG);
VOID(NTAPI* RtlInitUnicodeString_)(PUNICODE_STRING, PCWSTR);
NTSTATUS(NTAPI* NtClose_)(HANDLE);



//OPEN FILE:
//__kernel_entry NTSTATUS NtOpenFile(
//	[out] PHANDLE            FileHandle,
//	[in]  ACCESS_MASK        DesiredAccess,
//	[in]  POBJECT_ATTRIBUTES ObjectAttributes,
//	[out] PIO_STATUS_BLOCK   IoStatusBlock,
//	[in]  ULONG              ShareAccess,
//	[in]  ULONG              OpenOptions
//); 

// DIRECTORY_INFORMATION

typedef struct _OBJECT_DIRECTORY_INFORMATION {
	UNICODE_STRING Name;
	UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, * POBJECT_DIRECTORY_INFORMATION;

// INIT FOUNCTION

int InitNtModule() {
	HMODULE hNtDll = ::GetModuleHandle(_T("ntdll.dll"));
	*(FARPROC*)&NtOpenDirectoryObject = ::GetProcAddress(hNtDll, "NtOpenDirectoryObject");
	*(FARPROC*)&NtQueryDirectoryObject = ::GetProcAddress(hNtDll, "NtQueryDirectoryObject");
	*(FARPROC*)&RtlInitUnicodeString_ = ::GetProcAddress(hNtDll, "RtlInitUnicodeString");
	*(FARPROC*)&NtClose_ = ::GetProcAddress(hNtDll, "NtClose");
	if (!NtOpenDirectoryObject || !NtQueryDirectoryObject || !RtlInitUnicodeString_ || !NtClose_)
	{
		_tprintf(_T("Failed to retrieve ntdll.dll function pointers\n"));
		return 1;
	}
	return 0;
}