#ifndef MXMULTIPLEFILECHOOSER_H
#define MXMULTIPLEFILECHOOSER_H
#include <wx/dialog.h>

class wxTextCtrl;
class wxComboBox;
class wxCheckListBox;
class wxCheckBox;

class mxMultipleFileChooser : public wxDialog {
private:
	wxString search_base;
	wxTextCtrl *basedir, *filter;
	wxCheckBox *subdirs;
	wxCheckListBox *list;
	wxComboBox *cmb_where;
public:
	mxMultipleFileChooser(wxString apath="", bool modal=false);
	void OnButtonOk(wxCommandEvent &event);
	void OnButtonCancel(wxCommandEvent &event);
	void OnListRightClick(wxMouseEvent &event);
	void OnButtonMarkAll(wxCommandEvent &event);
	void OnButtonMarkNone(wxCommandEvent &event);
	void OnButtonMarkInvert(wxCommandEvent &event);
	void OnButtonDir(wxCommandEvent &event);
	void OnButtonFind(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void FindFiles(wxString where, wxString sub, wxString what, bool rec);
	DECLARE_EVENT_TABLE();
};

#endif

