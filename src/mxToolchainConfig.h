#ifndef MXTOOLCHAINCONFIG_H
#define MXTOOLCHAINCONFIG_H
#include <wx/dialog.h>
#include "Toolchain.h"
#include "widgetDisabler.h"

class wxTextCtrl;
class wxComboBox;

class mxToolchainConfig : public wxDialog {
private:
	widgetDisabler for_extern, for_gcc;
	wxTextCtrl *name;
//	wxTextCtrl *description;
	wxTextCtrl *base_path;
	wxTextCtrl *bin_path;
	wxTextCtrl *build_command;
	wxTextCtrl *clean_command;
	wxTextCtrl *arguments[TOOLCHAIN_MAX_ARGS][2];
	wxComboBox *type;
	wxTextCtrl *c_compiler;
	wxTextCtrl *c_compiling_options;
	wxTextCtrl *cpp_compiler;
	wxTextCtrl *cpp_compiling_options;
	wxTextCtrl *linker;
	wxTextCtrl *c_linker_options;
	wxTextCtrl *cpp_linker_options;
	wxTextCtrl *dynamic_lib_linker;
	wxTextCtrl *static_lib_linker;
public:
	mxToolchainConfig(wxWindow *parent, const wxString &tc_name);
	void OnButtonOk(wxCommandEvent &event);
	void OnButtonCancel(wxCommandEvent &event);
	void OnButtonHelp(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnComboChange(wxCommandEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

