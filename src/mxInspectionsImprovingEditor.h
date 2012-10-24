#ifndef MXINSPECTIONSIMPROVINGEDITOR_H
#define MXINSPECTIONSIMPROVINGEDITOR_H
#include <wx/dialog.h>

class wxListBox;
class wxTextCtrl;

class mxInspectionsImprovingEditor : public wxDialog {
private:
	int selected;
	wxListBox *list;
	wxTextCtrl *type_from, *expression_to;
	wxArrayString array_from, array_to;
public:
	mxInspectionsImprovingEditor(wxWindow *parent);
	void OnFrom(wxCommandEvent &evt);
	void OnTo(wxCommandEvent &evt);
	void OnAdd(wxCommandEvent &evt);
	void OnDel(wxCommandEvent &evt);
	void OnOk(wxCommandEvent &evt);
	void OnCancel(wxCommandEvent &evt);
	void OnList(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

