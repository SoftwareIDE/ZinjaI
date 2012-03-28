#include "ComplementArchive.h"

#include <wx/filesys.h>
#include <wx/fs_arc.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/utils.h>
#include <iostream>
#include <wx/arrstr.h>
#include <wx/sstream.h>
#include "Application.h"
using namespace std;

#ifdef __WIN32__
#define _DESC_FILE "temp\\desc.ini"
#else
#define _DESC_FILE "temp/desc.ini"
#endif

// functions based on code from http://forums.wxwidgets.org/viewtopic.php?t=2376&p=9825
static wxString FixSlashes(wxString orig) {
	wxString ret=orig;
	int l=orig.Len();
	for(int i=0;i<l;i++) {
		if (orig[i]==
#ifdef __WIN32__
			'/'
#else
			'\\'
#endif
			)
			orig[i]=
#ifdef __WIN32__
			'\\'
#else
			'/'
#endif
			;
	}
	return ret;
}

bool GetFilesAndDesc(bool (*callback)(wxString message, int progress), const wxString aZipFile, int &fcount, int &dcount, wxString &desc) {
	dcount=fcount=0;
	wxZipEntry *entry = new wxZipEntry;
	wxFileInputStream in(aZipFile);
	if (!in) return false;
	wxZipInputStream zip(in);
	while ((entry=zip.GetNextEntry())) {
		wxString name = FixSlashes(entry->GetName());
		if (!entry->IsDir()) {
			zip.OpenEntry(*entry);
			if (!callback("",0)) return true;
			if (name==_DESC_FILE) {
				if (!zip.CanRead()) return false;
				wxStringOutputStream os(&desc);
				zip.Read(os);
			} else fcount++;
		} else dcount++;
	}
	return true;
}

static bool legal_name(wxString name) { // por seguridad, para que no escriba nada fuera del directorio de zinjai
	return (
		name.Len() && 
		name[0]!='.' && 
		name[0]!='\\' && 
		name[0]!='/' && 
		name[0]!='%' &&
		name[0]!='$' && 
		!(name.Len()>1&&name[2]==':')
		);
}

bool CreateDirectories(bool (*callback)(wxString message, int progress), const wxString aZipFile, const wxString aTargetDir="") {
	wxZipEntry *entry = new wxZipEntry;
	wxFileInputStream in(aZipFile);
	if (!in) return false;
	wxZipInputStream zip(in);
	while ((entry=zip.GetNextEntry())) {
		wxString name = FixSlashes(entry->GetName());
		if (entry->IsDir()) {
			if (legal_name(name)) {
				name = aTargetDir + name;
				if (!wxFileName::DirExists(name)) {
					if (!callback(wxString(spanish?"Creando directorio ":"Creating directory ")<<name,1)) return true;
					wxFileName::Mkdir(name, 0777, wxPATH_MKDIR_FULL);
				}
			} else
				if (!callback(wxString(spanish?"Nombre de directorio incorrecto: ":"Wrong directory name: ")<<name,1)) return true;
		}
	}
	return true;
}

bool ExtractFiles(bool (*callback)(wxString message, int progress), const wxString aZipFile, const wxString aTargetDir) {
	wxZipEntry *entry = new wxZipEntry;
	wxFileInputStream in(aZipFile);
	if (!in) return false;
	wxZipInputStream zip(in);
	while ((entry=zip.GetNextEntry())) {
		wxString name = FixSlashes(entry->GetName());
		if (name==_DESC_FILE) continue;
		if (!entry->IsDir()) {
			if (legal_name(name)) {
				name = aTargetDir + name;
				zip.OpenEntry(*entry);
				if (!zip.CanRead()) return false;
				if (!callback(wxString(spanish?"Descomprimiendo archivo ":"Uncompressing file ")<<name,2)) return true;
				wxFileOutputStream file(name);
				if (!file) return false;
				zip.Read(file);
			} else
				if (!callback(wxString(spanish?"Nombre de archivo incorrecto: ":"Wrong file name: ")<<name,2)) return true;
		}
	}
	return true;
}

static wxString GetDir(wxString aTargetDir, wxString file) {
	if (wxFileName(aTargetDir+file).IsDir()) return file;
	else if (file.Contains("/")) return file.BeforeLast('/');
	else if (file.Contains("\\")) return file.BeforeLast('\\');
	else return "";
}

bool CreateZip(bool (*callback)(wxString message, int progress), const wxString aZipFile, const wxString aTargetDir, const wxArrayString &files) {
	wxFFileOutputStream out(aZipFile);
	wxZipOutputStream zip(out);
	wxArrayString dirs;
	for(unsigned int i=0;i<files.GetCount();i++) {
		wxString dir=GetDir(aTargetDir,files[i]);
		if (!callback(wxString(spanish?"Agregando ":"Adding ")<<files[i],1)) return true;
		if (dir.Len() && dirs.Index(dir)==wxNOT_FOUND) {
			wxZipEntry *entry=new wxZipEntry(dir);
			entry->SetIsDir(true);
			zip.PutNextEntry(entry);
			dirs.Add(dir);
		}
		if (!wxFileName(aTargetDir+files[i]).IsDir()) {
			zip.PutNextEntry(files[i]);
			wxFileInputStream file(aTargetDir+files[i]);
			zip<<file;
		}
	}
	return true;
}

bool SetBins(const wxArrayString & files, const wxString aTargetDir) {
	string target=aTargetDir.c_str();
	for(unsigned int i=0;i<files.GetCount();i++)
		system((string("chmod a+x ")+target+files[i].c_str()).c_str());
	return true;
}

bool desc_split(const wxString &atext, complement_info & info) {
	int p;
	wxString text=atext;
	wxString line,key,value;
	text.Replace("\r","");
	while ((p=text.Index('\n'))!=wxNOT_FOUND && text.Len()) {
		if (p==wxNOT_FOUND) p=text.Len();
		line=text.SubString(0,p-1);
		text=text.Mid(p+1);
		if (text=="end") { return true; }
		else if (line.Len() && line[0]!='#') {
			key=line.BeforeFirst('=');
			value=line.AfterFirst('=');
			if (key=="desc_english") { if (info.desc_english.Len()) info.desc_english+="\n"; info.desc_english+=value; }
			else if (key=="desc_spanish") { if (info.desc_spanish.Len()) info.desc_spanish+="\n"; info.desc_spanish+=value; }
			else if (key=="bin") info.bins.Add(value);
			else if (key=="close") { info.closereq = (value=="1" || value.Upper().StartsWith("T")); }
			else if (key=="reqver") { return value.ToLong(&info.reqver); }
		}
	}
	return false; // si estaba bien sale cuando encuentra "end"
}

wxString Multilineze(wxString prefix,wxString text) {
	wxString ret;
	ret<<prefix<<"="<<text;
	ret.Replace("\r",""); 
	ret.Replace("\n",wxString("\n")<<prefix<<"=");
	return ret;
}

bool desc_merge(const complement_info & info, wxString & text) {
	text.Clear();
	text<<Multilineze("desc_english",info.desc_english)<<"\n";
	text<<Multilineze("desc_spanish",info.desc_spanish)<<"\n";
	for (unsigned int i=0;i<info.bins.GetCount();i++)
		text<<"bin="<<info.bins[i]<<"\n";
	text<<"close="<<(info.closereq?"1":"0")<<"\n";
	text<<"reqver="<<info.reqver<<"\n";
	text<<"end\n";
	return true;
}

