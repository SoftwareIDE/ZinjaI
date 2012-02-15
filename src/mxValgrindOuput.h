#ifndef MXVALGRINDOUPUT_H
#define MXVALGRINDOUPUT_H
#include <wx/treectrl.h>

class mxValgrindOuput:public wxTreeCtrl {
private:
	wxString filename;
	wxTreeItemId root;
	wxTreeItemId last;
	bool is_last;
	wxTreeItemId sel;
	char mode;
public:
	mxValgrindOuput(wxWindow* parent, char amode, wxString afilename);
	void SetMode(char amode, wxString afilename);
	void LoadOutput();
	void LoadOutputValgrind();
	void LoadOutputCppCheck();
	void OnPopup(wxTreeEvent &evt);
	void OnSelect(wxTreeEvent &evt);
	void OnDelete(wxCommandEvent &evt);
	void OnReload(wxCommandEvent &evt);
	void OnOpen(wxCommandEvent &evt);
	void OnKey(wxKeyEvent &evt);
	~mxValgrindOuput();
	DECLARE_EVENT_TABLE();
};

#endif

