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
	wxCheckBox *reset;
	wxButton *but_create;
	wxButton *but_cancel;
	complement_info info;
protected:
public:
	mxCreateComplementWindow(wxString path);
	void OnButtonCreate(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnButtonFolder(wxCommandEvent &evt);
	void SetFolder(wxString path);
	void OnButtonDest(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void Notify(const wxString &message);
	int GetStep();
	DECLARE_EVENT_TABLE();
};

#endif

