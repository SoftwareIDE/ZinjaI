#ifndef MXCOMPILERARGENABLER_H
#define MXCOMPILERARGENABLER_H
#include "mxCommonConfigControls.h"

class wxCheckBox;

class mxCompilerArgEnabler : public mxDialog {
	wxCheckBox *m_enable_arg, *m_recompile;
	wxString m_comp_arg, m_link_arg;
public:
	mxCompilerArgEnabler(wxWindow *parent, wxString title, wxString help_text, wxString comp_arg, wxString link_arg);

	void OnArgCheck(wxCommandEvent &evt);
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	bool GetUserSelection();
	DECLARE_EVENT_TABLE();
};

#endif

