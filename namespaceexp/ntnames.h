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