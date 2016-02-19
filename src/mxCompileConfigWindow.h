#ifndef MX_COMPILE_CONFIG_WINDOW_H
#define MX_COMPILE_CONFIG_WINDOW_H

#include <wx/dialog.h>

class mxSource;
class wxAuiNotebook;
class wxTextCtrl;
class wxCheckBox;

class mxCompileConfigWindow : public wxDialog {
	
	// auxiliars for avoid repeating same code for every popup menu after a text ctrl
	wxTextCtrl *m_text_ctrl_for_popup; ///< ctrl that launched last popup
	wxString m_base_dir_for_popup; ///< base directory for relative paths for file/dir pickers
	wxString m_title_for_edit_helpers; ///< caption for mxListItemEditor and mxLongTextEditor
	
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
	void OnArgsDefault(wxCommandEvent &evt);
	void OnArgsFromTemplate(wxCommandEvent &evt);
	void OnArgsButton(wxCommandEvent &evt);
	
	void OnPopupAddFile(wxCommandEvent &evt);
	void OnPopupReplaceFile(wxCommandEvent &evt);
	void OnPopupAddDir(wxCommandEvent &evt);
	void OnPopupReplaceDir(wxCommandEvent &evt);
	void OnPopupEditList(wxCommandEvent &evt);
	void OnPopupEditText(wxCommandEvent &evt);

private:
	DECLARE_EVENT_TABLE()

};

#endif
