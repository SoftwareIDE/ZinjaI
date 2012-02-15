#ifndef MXOPENRECENTDIALOG_H
#define MXOPENRECENTDIALOG_H
#include <wx/dialog.h>
#include <wx/timer.h>

class wxListBox;
class wxTextCtrl;
class wxCheckBox;
class wxButton;

class mxOpenRecentDialog: public wxDialog {
private:
	wxTextCtrl *text_ctrl;
	wxListBox *list_ctrl;
	wxButton *clear_button;
	wxButton *del_button;
	wxButton *goto_button;
	wxButton *cancel_button;
	bool projects; ///< determina si muestra proyectos o fuentes recientes
public:
	mxOpenRecentDialog(wxWindow* parent, bool aprj);
	void OnDelButton(wxCommandEvent &event);
	void OnGotoButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnClear(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnTextChange(wxCommandEvent &event);
	void OnCharHook (wxKeyEvent &event);
	void UpdateList();
private:
	DECLARE_EVENT_TABLE();
};

#endif
