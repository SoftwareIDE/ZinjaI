#ifndef MXWINDOW_H
#define MXWINDOW_H
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/frame.h>
#include <wx/listbox.h>
#include <wx/listctrl.h>

WX_DECLARE_STRING_HASH_MAP( wxString, wxHashStringString );

class mxWindow : public wxFrame {
private:
	wxTextCtrl *curref_text;
	wxTextCtrl *curlang_text;
	wxTextCtrl *item_text;
	wxTextCtrl *ref_text;
	wxTextCtrl *trans_text;
	wxTextCtrl *status_text;
	wxCheckBox *doubt_check;
	wxListBox *list;
	
	wxString sel_item;
	wxString lang_file;
	wxArrayString enum_array;
	wxHashStringString lang_hash;
	wxHashStringString ref_hash;
public:
	void LoadEnum();
	void LoadLang(wxHashStringString &hash, wxString land);
	void OnShowAll(wxCommandEvent &evt);
	void OnShowDone(wxCommandEvent &evt);
	void OnShowPending(wxCommandEvent &evt);
	void OnApplyNext(wxCommandEvent &evt);
	void OnUndo(wxCommandEvent &evt);
	void OnSave(wxCommandEvent &evt);
	void OnSelect(wxCommandEvent &evt);
	void OnFind(wxCommandEvent &evt);
	void OnChangeRef(wxCommandEvent &evt);
	void OnChangeLang(wxCommandEvent &evt);
	void UpdateStatics();
	mxWindow();
	DECLARE_EVENT_TABLE();
};

#endif

