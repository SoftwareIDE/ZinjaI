#ifndef MXDIFFWINDOW_H
#define MXDIFFWINDOW_H
#include <wx/dialog.h>

class wxCheckBox;
class wxComboBox;
class mxSource;

class mxDiffWindow : public wxDialog {
private:
	wxComboBox *file1, *file2;
	wxCheckBox *ignore_case, *ignore_spaces, *ignore_empty_lines;
	wxCheckBox *show_tools, *show_sidebar;
	mxSource *source;
	wxString diff_to_name;
public:
	mxDiffWindow(mxSource *source=NULL, wxString fname="");
	void OnClose(wxCloseEvent &event);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void DiffTwoSources(mxSource *src1, mxSource *src2);
	void DiffSourceFile(mxSource *src, wxString fname);
	~mxDiffWindow();
	DECLARE_EVENT_TABLE();
};

#endif

