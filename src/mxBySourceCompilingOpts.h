#ifndef MXBYSOURCECOMPILINGOPTS_H
#define MXBYSOURCECOMPILINGOPTS_H
#include <wx/dialog.h>
#include <wx/arrstr.h>
#include "mxUtils.h"
#include "SingleList.h"
#include "raii.h"

class wxListBox;
class wxTextCtrl;
class wxCheckBox;
class wxComboBox;
class project_file_item;

class mxBySourceCompilingOpts : public wxDialog {
private:
	wxArrayString sources_list;
	wxListBox *list;
	wxCheckBox *fp_extra;
	wxCheckBox *fp_macros;
	wxCheckBox *fp_includes;
	wxCheckBox *fp_std;
	wxCheckBox *fp_warnings;
	wxCheckBox *fp_debug;
	wxCheckBox *fp_optimizations;
	wxCheckBox *apply_to_all;
	wxComboBox *profiles;
	wxTextCtrl *additional_args;
	wxTextCtrl *filter_sources;
	SingleList<HashStringString> config;
	int active_config;
	wxString last_source;
	BoolFlag mask_list_selection_event;
public:
	mxBySourceCompilingOpts(wxWindow *parent, project_file_item *item);
protected:
	DECLARE_EVENT_TABLE();
	void OnList(wxCommandEvent &evt);
	void OnButtonApply(wxCommandEvent &evt);
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnButtonHelp(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void OnFilter(wxCommandEvent &evt);
	void OnProfile(wxCommandEvent &evt);
	void Load(int prof, wxString src="");
};

#endif

