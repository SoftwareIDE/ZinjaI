/* ITlib.cpp --

   This file is part of the "PE Maker".

   Copyright (C) 2005-2006 Ashkbiz Danehkar
   All Rights Reserved.

   "PE Maker" library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYRIGHT.TXT.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Ashkbiz Danehkar
   <ashkbiz@yahoo.com>
*/
#include "stdafx.h"
#include <winnt.h>
#include <imagehlp.h>//#include <Dbghelp.h>
//#include <string.h>
#include <Winreg.h>
//#include <stdlib.h>
//#include <commctrl.h>
#include "ITLib.h"
#include "PELibErr.h"

//----------------------------------------------------------------
//------- DATA ---------
//HANDLE	pMap			= NULL;
DWORD	dwBytesRead		= 0;
DWORD	dwBytesWritten	= 0;
DWORD	dwFsize			= 0;
HANDLE	hFile			= NULL;
//----------------------------
//------- FUNCTION ---------
//----------------------------------------------------------------
ITLibrary::ITLibrary()
{
	pMem=NULL;
}
//----------------------------------------------------------------
ITLibrary::~ITLibrary()
{
	if(pMem==NULL) GlobalFree(pMem);
}
//----------------------------------------------------------------
void ITLibrary::OpenFileName(char* FileName)
{
	pMem=NULL;
	hFile=CreateFile(FileName,
					 GENERIC_READ,
					 FILE_SHARE_WRITE | FILE_SHARE_READ,
	                 NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		ShowErr(FileErr);
		return;
	}
	dwFsize=GetFileSize(hFile,0);
	if(dwFsize == 0)
	{
		CloseHandle(hFile);
		ShowErr(FsizeErr);
		return;
	}
	pMem=(char*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,dwFsize);
	if(pMem == NULL)
	{
		CloseHandle(hFile);
		ShowErr(MemErr);
		return;
	}
	ReadFile(hFile,pMem,dwFsize,&dwBytesRead,NULL);
	CloseHandle(hFile);
	dwFileSize=dwFsize;
}
//----------------------------------------------------------------
// returns aligned value
DWORD ITLibrary::PEAlign(DWORD dwTarNum,DWORD dwAlignTo)
{	
	DWORD dwtemp;
	dwtemp=dwTarNum/dwAlignTo;
	if((dwTarNum%dwAlignTo)!=0)
	{
		dwtemp++;
	}
	dwtemp=dwtemp*dwAlignTo;
	return(dwtemp);
}
//================================================================
//----------------------------------------------------------------
PIMAGE_SECTION_HEADER ITLibrary::ImageRVA2Section(PIMAGE_NT_HEADERS pimage_nt_headers,DWORD dwRVA)
{
	int i;
	PIMAGE_SECTION_HEADER pimage_section_header=(PIMAGE_SECTION_HEADER)((PCHAR(pimage_nt_headers)) + sizeof(IMAGE_NT_HEADERS));
	for(i=0;i<pimage_nt_headers->FileHeader.NumberOfSections;i++)
	{
		if((pimage_section_header->VirtualAddress) && (dwRVA<=(pimage_section_header->VirtualAddress+pimage_section_header->SizeOfRawData)))
		{
			return ((PIMAGE_SECTION_HEADER)pimage_section_header);
		}
		pimage_section_header++;
	}
	return(NULL);
}
//================================================================
//----------------------------------------------------------------
// calulates the Offset from a RVA
// Base    - base of the MMF
// dwRVA - the RVA to calculate
// returns 0 if an error occurred else the calculated Offset will be returned
DWORD ITLibrary::RVA2Offset(PCHAR pImageBase,DWORD dwRVA)
{
	DWORD _offset;
	PIMAGE_SECTION_HEADER section;
	PIMAGE_DOS_HEADER pimage_dos_header;
	PIMAGE_NT_HEADERS pimage_nt_headers;
	pimage_dos_header = PIMAGE_DOS_HEADER(pImageBase);
	pimage_nt_headers = (PIMAGE_NT_HEADERS)(pImageBase+pimage_dos_header->e_lfanew);
	section=ImageRVA2Section(pimage_nt_headers,dwRVA);
	if(section==NULL)
	{
		return(0);
	}
	_offset=dwRVA+section->PointerToRawData-section->VirtualAddress;
	return(_offset);
}
//----------------------------------------------------------------
int ITLibrary::GetImportDllName(int (__cdecl *callback1) (PCHAR, DWORD, DWORD, DWORD, DWORD, DWORD))
{
	PCHAR		pThunk;
	DWORD		dwThunk;
	PCHAR		pHintName;
	PCHAR		pDllName;
	DWORD		dwImportDirectory;

	PCHAR pImageBase = pMem;
	//----------------------------------------
	PIMAGE_IMPORT_DESCRIPTOR	pimage_import_descriptor;
	PIMAGE_THUNK_DATA			pimage_thunk_data;
	//----------------------------------------
	PIMAGE_DOS_HEADER pimage_dos_header;
	PIMAGE_NT_HEADERS pimage_nt_headers;
	pimage_dos_header = PIMAGE_DOS_HEADER(pImageBase);
	pimage_nt_headers = (PIMAGE_NT_HEADERS)(pImageBase+pimage_dos_header->e_lfanew);
	//----------------------------------------
	dwImportDirectory=RVA2Offset(pImageBase, pimage_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	if(dwImportDirectory==0) 
	{
		return -1;
	}
	//----------------------------------------
	pimage_import_descriptor=(PIMAGE_IMPORT_DESCRIPTOR)(pImageBase+dwImportDirectory);
	//----------------------------------------
	while(pimage_import_descriptor->Name!=0)
	{
		pThunk=pImageBase+pimage_import_descriptor->FirstThunk;
		dwThunk = pimage_import_descriptor->FirstThunk;
		pHintName=pImageBase;
		if(pimage_import_descriptor->OriginalFirstThunk!=0)
		{
			pHintName+=RVA2Offset(pImageBase, pimage_import_descriptor->OriginalFirstThunk);
		}
		else
		{
			pHintName+=RVA2Offset(pImageBase, pimage_import_descriptor->FirstThunk);
		}
		pDllName=pImageBase+RVA2Offset(pImageBase, pimage_import_descriptor->Name);
		callback1(pDllName,
			pimage_import_descriptor->OriginalFirstThunk,
			pimage_import_descriptor->TimeDateStamp,
			pimage_import_descriptor->ForwarderChain,
			pimage_import_descriptor->Name,
			pimage_import_descriptor->FirstThunk);

		pimage_thunk_data=PIMAGE_THUNK_DATA(pHintName);
		pimage_import_descriptor++;
	}
	//----------------------------------------
	return 0;
}
//---------------------------------------------------------
int ITLibrary::GetImportProcName(int iModule, int (__cdecl *callback2)(DWORD, DWORD, DWORD, DWORD, PCHAR, DWORD))
{
	PCHAR		pThunk;
	DWORD		dwThunk;
	PCHAR		pHintName;
	DWORD		dwAPIaddress;
	PCHAR		pDllName;
	PCHAR		pAPIName;
	DWORD		dwImportDirectory;
	int			iCount;
	WORD		Hint;

	PCHAR pImageBase = pMem;
	//----------------------------------------
	PIMAGE_IMPORT_DESCRIPTOR	pimage_import_descriptor;
	PIMAGE_THUNK_DATA			pimage_thunk_data;
	//----------------------------------------
	PIMAGE_DOS_HEADER pimage_dos_header;
	PIMAGE_NT_HEADERS pimage_nt_headers;
	pimage_dos_header = PIMAGE_DOS_HEADER(pImageBase);
	pimage_nt_headers = (PIMAGE_NT_HEADERS)(pImageBase+pimage_dos_header->e_lfanew);
	//----------------------------------------
	dwImportDirectory=RVA2Offset(pImageBase, pimage_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	if(dwImportDirectory==0) 
	{
		return -1;
	}
	//----------------------------------------
	pimage_import_descriptor=(PIMAGE_IMPORT_DESCRIPTOR)(pImageBase+dwImportDirectory);
	//----------------------------------------
	iCount = 0;
	while(pimage_import_descriptor->Name!=0)
	{
		pThunk=pImageBase+pimage_import_descriptor->FirstThunk;
		if(pimage_import_descriptor->OriginalFirstThunk!=0)
		{
			dwThunk = pimage_import_descriptor->OriginalFirstThunk;
		}
		else
		{
			dwThunk = pimage_import_descriptor->FirstThunk;
		}
		pHintName=pImageBase;
		if(pimage_import_descriptor->OriginalFirstThunk!=0)
		{
			pHintName+=RVA2Offset(pImageBase, pimage_import_descriptor->OriginalFirstThunk);
		}
		else
		{
			pHintName+=RVA2Offset(pImageBase, pimage_import_descriptor->FirstThunk);
		}
		pDllName=pImageBase+RVA2Offset(pImageBase, pimage_import_descriptor->Name);

		pimage_thunk_data=PIMAGE_THUNK_DATA(pHintName);
		if(iModule==iCount)
		{
			while(pimage_thunk_data->u1.AddressOfData!=0)
			{
				dwAPIaddress=pimage_thunk_data->u1.AddressOfData;
				if((dwAPIaddress&0x80000000)==0x80000000)
				{
					dwAPIaddress&=0x7FFFFFFF;
					callback2(dwThunk, RVA2Offset(pImageBase,dwThunk),0x80000000&dwAPIaddress, 0 ,NULL, dwAPIaddress);
				}
				else
				{
					pAPIName=pImageBase+RVA2Offset(pImageBase, dwAPIaddress)+2;
					memcpy(&Hint, pAPIName-2, 2);
					callback2(dwThunk, RVA2Offset(pImageBase,dwThunk),dwAPIaddress, Hint ,pAPIName, 0);
				}
				pThunk+=4;
				dwThunk+=4;
				pHintName+=4;
				pimage_thunk_data++;
			}
		}
		pimage_import_descriptor++;
		iCount++;
	}
	//----------------------------------------
	return 0;
}
