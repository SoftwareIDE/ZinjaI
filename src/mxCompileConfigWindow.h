#ifndef MX_COMPILE_CONFIG_WINDOW_H
#define MX_COMPILE_CONFIG_WINDOW_H

#include "mxCommonConfigControls.h"
#include "widgetDisabler.h"
#include "mxCommonPopup.h"

class mxSource;
class wxAuiNotebook;
class wxTextCtrl;
class wxCheckBox;

class mxCompileConfigWindow : public mxDialog {
	_use_common_popup;
	widgetBinder m_binder;
	wxArrayString opts_list;
 	mxSource *m_source;
	wxTextCtrl *args_ctrl;
	wxTextCtrl *compiler_options_ctrl;
	wxTextCtrl *working_folder_ctrl;
	
public:
	mxCompileConfigWindow(wxWindow* parent, mxSource *a_source);
	void OnOkButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnButtonFolder(wxCommandEvent &event);
	void OnButtonCompilerOptions(wxCommandEvent &event);
	void OnHelpButton(wxCommandEvent &event);
	void OnArgsDefault(wxCommandEvent &evt);
	void OnArgsFromTemplate(wxCommandEvent &evt);
	void OnArgsButton(wxCommandEvent &evt);

	DECLARE_EVENT_TABLE()
};

#endif
