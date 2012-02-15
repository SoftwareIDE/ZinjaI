#ifndef MXTOOLBAREDITOR_H
#define MXTOOLBAREDITOR_H
#include <wx/dialog.h>

class wxBoxSizer;
class wxScrolledWindow;
class wxCheckBox;
class wxToolBar;

#define MAX_TOOLBAR_BUTTONS 50

class mxToolbarEditor : public wxDialog {
private:
	wxString ipre;
	wxToolBar *toolbar;
	wxBoxSizer *sizer;
	wxScrolledWindow *scroll;
	bool *booleans[MAX_TOOLBAR_BUTTONS];
	wxCheckBox *checkboxs[MAX_TOOLBAR_BUTTONS];
	int count;
public:
	mxToolbarEditor(wxToolBar *toolbar,wxString name);
	~mxToolbarEditor();
	void Add(wxString name, wxString file, bool &config_entry);
	void ShowUp();
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

