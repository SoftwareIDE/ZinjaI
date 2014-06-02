#include "DebugPatcher.h"
#include <wx/arrstr.h>
#include <vector>
#include <wx/msgdlg.h>
#include "DebugManager.h"
#include "mxUtils.h"
#include "ConfigManager.h"
#include <fstream>
using namespace std;

#define DP_ERROR_DUMP 1
#define DP_ERROR_OPEN 2
#define DP_ERROR_READ 4
#define DP_ERROR_CANT 8
#define DP_ERROR_SAME 16

bool DebugPatcher::mem_seg::extract(wxString &s, int &where) {
	int p=s.Find("0x");
	if (p==wxNOT_FOUND) return false;
	s=s.Mid(p+2);
	int r=0, l=s.Len(), i=0;
	while (i<l && ( (s[i]>='0'&&s[i]<='9') || (s[i]>='A'&&s[i]<='F') || (s[i]>='a'&&s[i]<='f') ) ) {
		r*=16;
		if (s[i]>='0'&&s[i]<='9') {
			r+=s[i]-'0';
		} else if (s[i]>='A'&&s[i]<='F') {
			r+=s[i]+10-'A';
		} else {
			r+=s[i]+10-'a';
		}
		i++;
	}
	where=r;
	return true;
}

DebugPatcher::mem_seg::mem_seg(wxString line) {
	extract(line,mini);
	extract(line,mend);
	extract(line,size);
	extract(line,fini);
}

static void Split(wxArrayString &a, wxString &s) {
	int p=s.Find("\n");
	while (p!=wxNOT_FOUND) {
		wxString line=s.Mid(0,p);
		if (line.Len() && line.Last()=='\r') line.RemoveLast();
		a.Add(line);
		s=s.Mid(int(s.Len())>p&&s[p+1]=='\r'?p+2:p+1);
		p=s.Find("\n");
	}
	a.Add(s);
}

void DebugPatcher::Patch ( ) {
	
	if (patched_file.Len()==0 || !wxFileExists(patched_file)) {
		wxMessageBox("Could not open modified binary (have you compiled again?)");
		return;
	}
	
	
	wxString ans=debug->SendCommand("info proc mappings");
	if (!ans.Contains("^done")) {
		wxMessageBox("Could not determine memory mappings");
		return;
	}
	int ms_ok=0,ms_err=0,ms_eq=0,ms_partial=0,ch_ok=0,ch_err=0,ms_can=0;
	ans=utils->UnEscapeString(ans);
	wxArrayString lines; Split(lines,ans);
	for(unsigned int i=0;i<lines.GetCount();i++) { 
		if (lines[i].EndsWith(" (deleted)")) lines[i]=lines[i].Mid(0,lines[i].Len()-10);
		if (lines[i].EndsWith(exe_file)) {
			mem_seg ms(lines[i]);
			int ret=Patch(ms);
			if (ret==0) { 
				if (ch_err) ms_partial++; else ms_ok++;
				ch_ok+=ms.cambios_ok; ch_err+=ms.cambios_error;
			} else if (ret==DP_ERROR_CANT) {
				ms_can++;
			} else if (ret==DP_ERROR_SAME) {
				ms_eq++;
			} else {
				ms_err++;
			}
		}
	}
	
	int ms_all=ms_ok+ms_err+ms_eq+ms_partial+ms_can;
	
	if (ms_all==0) {
		wxMessageBox("Could not identify executable's memory segments");
		return;
	}
	
	if (ms_eq==ms_all) {
		wxMessageBox("No difference found");
		return;
	}
	
	wxString result;
	result<<"Executable's memory segments: "<<ms_all<<"\n";
	result<<"     Not changed: "<<ms_eq<<"\n";
	result<<"     Not identified: "<<ms_can<<"\n";
	result<<"     Succesfully updated: "<<ms_ok<<"\n";
	result<<"     Partially updated: "<<ms_partial<<"\n";
	result<<"     Failed to update: "<<ms_err<<"\n";
	result<<"\n";
	result<<"Changes detected: "<<ch_ok+ch_err<<"\n";
	result<<"     Succesfully applied: "<<ch_ok<<"\n";
	result<<"     Failed to apply: "<<ch_err<<"\n";
	wxMessageBox(result);
	
}

int DebugPatcher::Patch (mem_seg & ms) {
	
	if (!temp_file.Len()) temp_file=DIR_PLUS_FILE(config->temp_dir,"gdb_dump.for_patch");
	
	wxString ans=debug->SendCommand(wxString("dump memory ")<<utils->EscapeString(temp_file)<<" "<<ms.mini<<" "<<ms.mend);
	if (!ans.Contains("^done")) return DP_ERROR_DUMP;
	
//	wxString fnew=project?project->GetExePath():main_window->GetCurrentSource()->GetBinaryFileName().GetFullPath();
//	wxString fold=for_patch_done;
//#ifdef __WIN32__
//	wxString saux=fnew; fnew=fold; fold=saux;
//#endif
//	
//	if (!debug->for_patch_done.Len()) {
//		wxMessageBox("No patched binary found. Recompile first.");
//	}
//	
	
	// abrir los binarios y obtener las diferencias
	ifstream f1(temp_file.c_str(),ios::binary|ios::ate), f2(patched_file.c_str(),ios::binary|ios::ate);
	if (!f1.is_open()||!f2.is_open()) return DP_ERROR_OPEN;
	f1.seekg(0,ios::beg); f2.seekg(ms.fini,ios::beg);

	// comparar y obtener los cambios
	vector<pair<int,char> > v; char c1,c2;
	for(int i=0;i<ms.size;i++) { 
		if (!f1.read(&c1,1)) 
			return DP_ERROR_READ;
		if (!f2.read(&c2,1)) 
			return DP_ERROR_READ;
		if (c1!=c2) {
			v.push_back(make_pair(i,c2));
			if (v.size()>max_cambios) 
				return DP_ERROR_CANT;
		}
	}
	
	if (v.empty()) return DP_ERROR_SAME;
	
	ms.cambios_error=ms.cambios_ok=0;
	for(unsigned int i=0;i<v.size();i++) { 
		wxString cmd; 
		cmd<<"set (*((char*)("<<ms.mini+v[i].first<<")))="<<int(v[i].second);
		wxString ans=debug->SendCommand(cmd);
		if (ans.Contains("^done")) ms.cambios_ok++; else ms.cambios_error++;
	}
	return 0;
	
//	// buscar en gdb donde empieza la zona de memoria del binario
//	wxString base=debug->SendCommand("info address main");
//	int x=base.Find(" 0x"); if (x==wxNOT_FOUND) {
//		wxMessageBox("Could not determine memory address");
//		return;
//	}
//	base=base.Mid(x).BeforeFirst('.'); int pvalid=0, pinvalid=bsize;
//	while (pinvalid-pvalid>1) {
//		int pmid=(pvalid+pinvalid)/2;
//		wxString cmd; cmd<<"p (int)(*((char*)("<<base<<"-"<<pmid<<")))";
//		cmd=SendCommand(cmd);
//		if (cmd.Contains("^error")) pinvalid=pmid; else pvalid=pmid;
//	}
//	

}


DebugPatcher::~DebugPatcher ( ) {
	if (temp_file.Len() && wxFileExists(temp_file)) wxRemoveFile(temp_file);
#ifdef __WIN32__
	if (patched_file.Len() && wxFileExists(patched_file)) wxRemoveFile(patched_file);
#endif
}

