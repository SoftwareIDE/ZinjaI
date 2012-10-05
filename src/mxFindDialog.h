#ifndef MXFINDDIALOG_H
#define MXFINDDIALOG_H
#include <wx/dialog.h>
#include <wx/string.h>
#include <wx/arrstr.h>

class wxComboBox;
class wxCheckBox;
class mxSource;
class wxButton;
class wxStaticText;

class mxFindDialog : public wxDialog {
private:
	DECLARE_EVENT_TABLE();
	wxString GetHtmlEntry(wxString fname, int line, int pos, int len, wxString falias, wxString the_line);
public:
  bool replace_mode; ///< para que se abrio el cuadro: buscar o reemplazar
	bool only_selection;
	int last_flags;
	wxString last_search;
	wxString last_replace;
	wxComboBox *combo_find;
	wxComboBox *combo_replace;
	wxComboBox *combo_scope;
	wxCheckBox *check_case;
	wxCheckBox *check_word;
	wxCheckBox *check_start;
	wxCheckBox *check_regexp;
	wxCheckBox *check_close;
	wxCheckBox *check_nocomments;
	wxButton *replace_button;
	wxButton *next_button;
	wxButton *replace_all_button;
	wxStaticText *replace_static;
	mxFindDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	~mxFindDialog();
	void MyHide();
	void OnClose(wxCloseEvent &event);
	void OnCancel(wxCommandEvent &event);
	void ShowFind(mxSource *source);
	void ShowReplace(mxSource *source);
	void OnFindNextButton(wxCommandEvent &event);
	void OnFindPrevButton(wxCommandEvent &event);
	void OnReplaceButton(wxCommandEvent &event);
	void OnReplaceAllButton(wxCommandEvent &event);
	void OnHelpButton(wxCommandEvent &event);
	void OnComboScope(wxCommandEvent &event);
	bool FindNext();
	bool FindPrev();
	int FindInSource(mxSource *source,wxString &res);
	bool FindInSources();
	bool FindInProject(char where);
};

#endif

