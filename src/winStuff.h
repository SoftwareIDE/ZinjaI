#ifdef __WIN32__ 

#ifndef winStuff_H
#define winStuff_H

#include <tlhelp32.h>
#include <Windows.h>
#include <stddef.h>
#include <stdlib.h>
#include <cstdio>


bool dbp_checked=false;
bool dbp_present=false;
typedef BOOL WINAPI (*dbp_proto)(HANDLE);
dbp_proto dbp_function;

bool winLoadDBP() {
	if (dbp_checked) return dbp_present;
	dbp_checked=true;
	
	HINSTANCE hinstLib = LoadLibrary(TEXT("kernel32.dll"));
	if (hinstLib == NULL) {
		dbp_present=false;
		return false;
	}
	
	// Get function pointer
	dbp_function = (dbp_proto)GetProcAddress(hinstLib, "DebugBreakProcess");
	if (dbp_function == NULL) {
		dbp_present=false;
		return false;
	}
	
	dbp_present=true;
	return true;
}




// based on code taken from http://stackoverflow.com/questions/1173342/terminate-a-process-tree-c-for-windows
long winGetChildPid(long pid) {
	DWORD child_pid=0, myprocID = pid; // your main process id
	PROCESSENTRY32 pe;
	memset(&pe, 0, sizeof(PROCESSENTRY32));
	pe.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hSnap = :: CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (::Process32First(hSnap, &pe)) {
		BOOL bContinue = TRUE;
		// kill child processes
		while (bContinue) {
			// only kill child processes
			if (pe.th32ParentProcessID == myprocID && pe.th32ProcessID>0 &&
				(pe.th32ProcessID<child_pid || child_pid==0) )
				child_pid=pe.th32ProcessID;
			bContinue = ::Process32Next(hSnap, &pe);
		}
	}
	return child_pid;
}

// based on code taken from http://www.mingw.org/wiki/Workaround_for_GDB_Ctrl_C_Interrupt
bool winDebugBreak(long proc_id) {
	HANDLE proc;
	BOOL break_result;
	if (proc_id == 0) return false;
	proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)proc_id);
	if (proc == NULL) return false;
//	break_result = DebugBreakProcess(proc);
	break_result = dbp_function(proc);
	if (!break_result) {
		CloseHandle(proc);
		return false;
	}
	CloseHandle(proc);
	return true;
}

#endif

#endif
