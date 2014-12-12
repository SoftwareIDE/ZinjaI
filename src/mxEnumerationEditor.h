#ifndef MXENUMERATIONEDITOR_H
#define MXENUMERATIONEDITOR_H
#include <wx/dialog.h>
#include <wx/string.h>

class wxTextCtrl;
class wxListBox;
class wxComboBox;

class mxEnumerationEditor : public wxDialog {
private:
	wxListBox *m_list;
	wxTextCtrl *m_text;
	wxComboBox *m_combo;
	wxArrayString *m_array;
	bool m_comma_splits;
public:
	mxEnumerationEditor(wxWindow *parent, wxString title, wxTextCtrl *text, bool comma_splits);
	mxEnumerationEditor(wxWindow *parent, wxString title, wxComboBox *combo, bool comma_splits);
	mxEnumerationEditor(wxWindow *parent, wxString title, wxArrayString *combo);
	void CreateCommonStuff(wxString value, bool comma_splits);
	void CreateCommonStuff(const wxArrayString &value);
	~mxEnumerationEditor();
	void OnClose(wxCloseEvent &evt);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnAdd(wxCommandEvent &evt);
	void OnDelete(wxCommandEvent &evt);
	void OnEdit(wxCommandEvent &evt);
	void OnUp(wxCommandEvent &evt);
	void OnDown(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

