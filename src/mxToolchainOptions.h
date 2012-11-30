#ifndef MXTOOLCHAINOPTIONS_H
#define MXTOOLCHAINOPTIONS_H
#include <wx/dialog.h>
#include "Toolchain.h" // for TOOLCHAIN_MAX_ARGS

class wxTextCtrl;
class project_configuration;
	
class mxToolchainOptions : public wxDialog {
private:
	project_configuration *configuration;
	wxTextCtrl *texts[TOOLCHAIN_MAX_ARGS];
public:
	mxToolchainOptions(wxWindow *parent, wxString toolchain_fname, project_configuration *conf);
	void OnOkButton(wxCommandEvent &evnt);
	void OnCancelButton(wxCommandEvent &evnt);
	DECLARE_EVENT_TABLE();
};

#endif

