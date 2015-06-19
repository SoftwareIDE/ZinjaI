#ifndef MXTOOLBAREDITOR_H
#define MXTOOLBAREDITOR_H
#include <wx/dialog.h>

class wxBoxSizer;
class wxScrolledWindow;
class wxCheckBox;
class wxToolBar;

#define MAX_TOOLBAR_BUTTONS 100

class mxToolbarEditor : public wxDialog {
private:
	wxString ipre;
	wxBoxSizer *sizer;
	wxScrolledWindow *scroll;
	bool *booleans[MAX_TOOLBAR_BUTTONS];
	wxCheckBox *checkboxs[MAX_TOOLBAR_BUTTONS];
	int count, toolbar_id;
public:
	mxToolbarEditor(wxWindow *parent, int toolbar_id, wxString name);
	void Add(wxString name, wxString file, bool &config_entry);
	void ShowUp();
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

