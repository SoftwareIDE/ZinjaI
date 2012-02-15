#include<iostream>
#include <windows.h>
#include "itlib.h"
using namespace std;

static int callback1(PCHAR pDllName, DWORD OriginalFirstThunk, DWORD TimeDateStamp, 
	DWORD ForwarderChain, DWORD Name, DWORD FirstThunk) {
		cout<<"   "<<pDllName<<endl;
		return 0;
}


int main (int argc, char *argv[]) {
	ITLibrary itlib;
	itlib.OpenFileName(argv[1]);
	itlib.GetImportDllName(callback1);
	return 0;
}

