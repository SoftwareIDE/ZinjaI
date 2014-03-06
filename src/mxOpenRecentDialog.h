#ifndef MXOPENRECENTDIALOG_H
#define MXOPENRECENTDIALOG_H
#include <wx/dialog.h>
#include <wx/timer.h>
#include <wx/listctrl.h>

class wxListCtrl;
class wxListBox;
class wxTextCtrl;
class wxCheckBox;
class wxButton;

class mxOpenRecentDialog: public wxDialog {
private:
	wxTextCtrl *text_ctrl;
	wxListCtrl *list_ctrl;
	wxButton *clear_button;
	wxButton *del_button;
	wxButton *goto_button;
	wxButton *cancel_button;
	bool projects; ///< determina si muestra proyectos o fuentes recientes
	int GetListSelection();
	void SetListSelection(int i);
public:
	mxOpenRecentDialog(wxWindow* parent, bool aprj);
	void OnDelButton(wxCommandEvent &event);
	void OnListDClick(wxListEvent &event);
	void OnGotoButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnClear(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnTextChange(wxCommandEvent &event);
	void OnCharHook (wxKeyEvent &event);
	void OnResize(wxSizeEvent &event);
	void UpdateList();
private:
	DECLARE_EVENT_TABLE();
};

#endif
