#ifndef MXTOOLCHAINOPTIONS_H
#define MXTOOLCHAINOPTIONS_H

#include "mxCommonConfigControls.h"
#include "Toolchain.h" // for TOOLCHAIN_MAX_ARGS

class wxTextCtrl;
class project_configuration;
	
class mxToolchainOptions : public mxDialog {
private:
	project_configuration *m_configuration;
	wxTextCtrl *texts[TOOLCHAIN_MAX_ARGS];
public:
	mxToolchainOptions(wxWindow *parent, wxString toolchain_fname, project_configuration *conf);
	void OnOkButton(wxCommandEvent &evnt);
	void OnCancelButton(wxCommandEvent &evnt);
	DECLARE_EVENT_TABLE();
};

#endif

