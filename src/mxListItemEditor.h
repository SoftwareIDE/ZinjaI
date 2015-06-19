#ifndef MXLISTITEMEDITOR_H
#define MXLISTITEMEDITOR_H
#include <wx/dialog.h>

class wxListBox;
class wxTextCtrl;

class mxListItemEditor : public wxDialog {
private:
	wxListBox *list;
	wxTextCtrl *my_text;
	wxString last_dir;
	int sel;
public:
	mxListItemEditor(wxWindow *parent, wxString title, wxListBox *list, int sel);
	void OnClose(wxCloseEvent &evt);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnReplaceDir(wxCommandEvent &evt);
	void OnAddDir(wxCommandEvent &evt);
	void OnReplaceFile(wxCommandEvent &evt);
	void OnAddFile(wxCommandEvent &evt);
	void OnArgsButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

