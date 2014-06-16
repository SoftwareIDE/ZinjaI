#ifndef MX_NEW_WIZARD_H
#define MX_NEW_WIZARD_H

#include <wx/dialog.h>
#include <wx/textfile.h>
#include <wx/html/htmlwin.h>

class wxCheckBox;
class wxHtmlWindow;
class wxTextFile;

#define TIPS_TO_SKIP 2

/** 
* @brief Dialogo para sugerencias (tips).
* 
* Diálogo de sugerencias. Muestra la sugerencia del día, o el mensaje que indica
* las mejoras de la versión, si es la primera vez que se ejecuta una versión.
**/
class mxTipsWindow : public wxDialog {
public:
	wxCheckBox *showtip_checkbox;
	wxHtmlWindow *tooltip; 
	bool changelog;
	int retry_num;
 
	wxTextFile file;

	mxTipsWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);

	void OnButtonClose(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	~mxTipsWindow();
	void OnCheckbox (wxCommandEvent &event);
	void OnOtherOne(wxCommandEvent &event);
	void OnKeyDown(wxKeyEvent &evt);
	void ShowAnyTip();
	void OnQuickHelpLink (wxHtmlLinkEvent &event);

private:
	DECLARE_EVENT_TABLE()

};

extern mxTipsWindow *tips_window;

#endif
