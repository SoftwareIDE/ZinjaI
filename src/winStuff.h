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
	if (hinstLib == nullptr) {
		dbp_present=false;
		return false;
	}
	
	// Get function pointer
	dbp_function = (dbp_proto)GetProcAddress(hinstLib, "DebugBreakProcess");
	if (dbp_function == nullptr) {
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
	if (proc == nullptr) return false;
//	break_result = DebugBreakProcess(proc);
	break_result = dbp_function(proc);
	if (!break_result) {
		CloseHandle(proc);
		return false;
	}
	CloseHandle(proc);
	return true;
}


// as seen on http://stackoverflow.com/questions/1888863/how-to-get-main-window-handle-from-process-id
struct handle_data {
	unsigned long process_id;
	HWND best_handle;
};
static BOOL is_main_window(HWND handle)
{   
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}
static BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle)) {
		return TRUE;
	}
	data.best_handle = handle;
	return FALSE;   
}
static HWND find_main_window(unsigned long process_id)
{
	handle_data data;
	data.process_id = process_id;
	data.best_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.best_handle;
}



static void setFocus(unsigned long int pid) {
	HWND win = find_main_window(pid);
	if (win==0) return;
	SetForegroundWindow(win);
}

#endif

#endif
