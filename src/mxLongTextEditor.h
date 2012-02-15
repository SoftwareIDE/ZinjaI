#ifndef MXLONGTEXTEDITOR_H
#define MXLONGTEXTEDITOR_H
#include <wx/dialog.h>

class wxTextCtrl;
class wxComboBox;

class mxLongTextEditor : public wxDialog {
private:
	wxTextCtrl *my_text, *text;
	wxComboBox *combo;
	wxString last_dir;
public:
	mxLongTextEditor(wxWindow *parent, wxString title, wxTextCtrl *text);
	mxLongTextEditor(wxWindow *parent, wxString title, wxComboBox *combo);
	void CreateCommonStuff(wxString value);
	~mxLongTextEditor();
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

