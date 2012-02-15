#ifndef MXWXFBINHERITER_H
#include <wx/dialog.h>
#include <wx/arrstr.h>
#define MXWXFBINHERITER_H

class wxComboBox;
class wxTextCtrl;

class mxWxfbInheriter : public wxDialog {
	wxArrayString base_array,father_array;
	bool new_class;
private:
	wxComboBox *base_class;
	wxTextCtrl *child_class;
public:
	mxWxfbInheriter(wxWindow *parent, bool a_new_class);
	~mxWxfbInheriter();
	void FillBaseArray();
	void OnClose(wxCloseEvent &evt);
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OkNewClass();
	void OkUpdateClass();
//	void GetFatherMethods(wxString base_header, wxString class_name, wxArrayString &methods);
	DECLARE_EVENT_TABLE();
};

#endif

