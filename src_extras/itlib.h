/* ITlib.h --

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
#pragma once
#define MAX_SECTION_NUM         20
//----------------------------------------------------------------
class ITLibrary 
{
protected:
private:
public:
	DWORD					dwFileSize;
	char					*pMem;
	ITLibrary();
	~ITLibrary();
	DWORD PEAlign(DWORD dwTarNum,DWORD dwAlignTo);
	PIMAGE_SECTION_HEADER ImageRVA2Section(PIMAGE_NT_HEADERS pimage_nt_headers,DWORD dwRVA);
	DWORD RVA2Offset(PCHAR pImageBase,DWORD dwRVA);
	void OpenFileName(char* FileName);
	int GetImportDllName(int (__cdecl *callback1) (PCHAR, DWORD, DWORD, DWORD, DWORD, DWORD));
	int GetImportProcName(int iModule, int (__cdecl *callback2)(DWORD, DWORD, DWORD, DWORD, PCHAR, DWORD));
};
//----------------------------------------------------------------
