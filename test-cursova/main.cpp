#include <Windows.h>
#include <tchar.h>
#include <cstdio>
#include <winternl.h>
#include <vector>
#include <algorithm>
#include "ntutil.h"
#include "cmdutil.h"


int GetObjects(NTSTATUS& ntStatus, OBJECT_ATTRIBUTES& oa, UNICODE_STRING& objname, HANDLE& hDeviceDir, PCWSTR path, BYTE* buf) {
	RtlInitUnicodeString_(&objname, path);
	InitializeObjectAttributes(&oa, &objname, 0, NULL, NULL);
	ntStatus = NtOpenDirectoryObject(&hDeviceDir, DIRECTORY_QUERY | DIRECTORY_TRAVERSE, &oa);

	if (!NT_SUCCESS(ntStatus)) {
		delete hDeviceDir;
		_tprintf(_T("Failed NtOpenDirectoryObject with 0x%08X\n"), ntStatus);
		return 1;
	}
	
	return 0;
}

int PrintObjects(NTSTATUS& ntStatus, HANDLE& hDeviceDir, BYTE* buf) {
	ULONG start = 0, idx = 0, bytes;
	BOOLEAN restart = TRUE;

	std::vector<OBJECT_DIRECTORY_INFORMATION> entries;

	while (true) {
		ntStatus = NtQueryDirectoryObject(hDeviceDir, PBYTE(buf), BUFFER_SIZE, FALSE, restart, &idx, &bytes);

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
			std::sort(entries.begin(), entries.end(), [](const OBJECT_DIRECTORY_INFORMATION& a, const OBJECT_DIRECTORY_INFORMATION& b) {
				return wcscmp(a.TypeName.Buffer, b.TypeName.Buffer) < 0;
			});
			for (OBJECT_DIRECTORY_INFORMATION& elem : entries) {
				_tprintf(_T("Name: %s\tType: %s\n"), elem.Name.Buffer, elem.TypeName.Buffer);
			}

			break;
		}
	}

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
	BYTE* buf = new BYTE[BUFFER_SIZE];
	
	if (GetObjects(ntStatus, oa, objname, hDeviceDir, path, buf) != 0) {
		_tprintf(_T("Failed to read Directory Objects\n"));

		(void)NtClose_(hDeviceDir);
		delete buf;
		return 1;
	}
	PrintObjects(ntStatus, hDeviceDir, buf);

	(void)NtClose_(hDeviceDir);
	delete buf;
	return 0;
}