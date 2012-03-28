#ifndef COMPLEMENTARCHIVE_H
#define COMPLEMENTARCHIVE_H
#include <wx/string.h>
#include <wx/arrstr.h>

struct complement_info {
	wxString desc_english; // english description
	wxString desc_spanish; // spanish description
	wxArrayString bins; // files to set executable permission
	bool closereq;
	long reqver;
	complement_info():closereq(false),reqver(0){}
};
bool desc_split(const wxString &text, complement_info &info);
bool desc_merge(const complement_info &info, wxString &text);

bool GetFilesAndDesc(bool (*callback)(wxString message), const wxString aZipFile, wxArrayString &files, wxString &desc);
bool CreateDirectories(bool (*callback)(wxString message), const wxString aZipFile, const wxString aTargetDir);
bool ExtractFiles(bool (*callback)(wxString message), const wxString aZipFile, const wxString aTargetDir);
bool SetBins(const wxArrayString &files, const wxString aTargetDir);

bool CreateZip(bool (*callback)(wxString message), const wxString aZipFile, const wxString aTargetDir, const wxArrayString &files);


#endif

