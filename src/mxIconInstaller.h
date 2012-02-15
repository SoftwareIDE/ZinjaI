#if !defined(__WIN32__) && !defined(__APPLE__)

#ifndef MXICONINSTALLER_H
#define MXICONINSTALLER_H
#include <wx/dialog.h>

class wxCheckBox;

class mxIconInstaller : public wxDialog {
private:
	wxCheckBox *desktop;
	wxCheckBox *menu;
//	wxCheckBox *zpr;
//	wxCheckBox *cpp;
	bool icon_installed;
	bool xdg_not_found;
public:
	mxIconInstaller(bool first_run=false);
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void InstallIcons();
	void InstallDesktop(bool menu);
	void InstallMime ( wxString mime_type, wxString mime_desc, wxString icon, wxArrayString exts );
	void InstallMimeZpr();
	void InstallMimeSource();
	void MakeDesktopIcon();
	DECLARE_EVENT_TABLE();
};

#endif

#endif
