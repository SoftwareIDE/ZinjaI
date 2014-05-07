#ifndef MX_COMPILE_CONFIG_WINDOW_H
#define MX_COMPILE_CONFIG_WINDOW_H

#include <wx/dialog.h>

class mxSource;
class wxAuiNotebook;
class wxTextCtrl;
class wxCheckBox;

class mxCompileConfigWindow : public wxDialog {
	
	wxTextCtrl *text_for_edit;
	wxArrayString opts_list;
	
public:

 	mxSource *source;

	wxTextCtrl *args_ctrl;
	wxTextCtrl *compiler_options_ctrl;
	wxTextCtrl *working_folder_ctrl;
	wxCheckBox *always_ask_args_ctrl;
	wxCheckBox *wait_for_key_ctrl;
	
	wxString last_dir;

	mxCompileConfigWindow(mxSource *a_source, wxWindow* parent, wxWindowID id=wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	void OnOkButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnButtonFolder(wxCommandEvent &event);
	void OnButtonCompilerOptions(wxCommandEvent &event);
	void OnHelpButton(wxCommandEvent &event);
	void OnArgsAddFile(wxCommandEvent &evt);
	void OnArgsReplaceFile(wxCommandEvent &evt);
	void OnArgsDefault(wxCommandEvent &evt);
	void OnArgsFromTemplate(wxCommandEvent &evt);
	void OnArgsAddDir(wxCommandEvent &evt);
	void OnArgsReplaceDir(wxCommandEvent &evt);
	void OnArgsButton(wxCommandEvent &evt);
	void OnArgsEditList(wxCommandEvent &evt);
	void OnArgsEditText(wxCommandEvent &evt);

private:
	DECLARE_EVENT_TABLE()

};

#endif
