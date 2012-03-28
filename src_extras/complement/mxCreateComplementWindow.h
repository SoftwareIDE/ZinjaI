#ifndef MXCREATECOMPLEMENTWINDOW_H
#define MXCREATECOMPLEMENTWINDOW_H
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include "ComplementArchive.h"

class mxCreateComplementWindow : public wxFrame {
private:
	int step;
	wxTextCtrl *dest;
	wxTextCtrl *folder;
	wxTextCtrl *text_es;
	wxTextCtrl *text_en;
	wxTextCtrl *text;
	wxTextCtrl *version;
	wxCheckBox *close;
	wxButton *but_create;
	wxButton *but_cancel;
	complement_info info;
	friend bool callback_create(wxString message);
protected:
public:
	mxCreateComplementWindow();
	void OnButtonCreate(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnButtonFolder(wxCommandEvent &evt);
	void OnButtonDest(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void Notify(const wxString &message);
	DECLARE_EVENT_TABLE();
};

#endif

