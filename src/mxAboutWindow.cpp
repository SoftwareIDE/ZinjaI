
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/html/htmlwin.h>

#include "mxAboutWindow.h"
#include "ConfigManager.h"
#include "mxBitmapButton.h"
#include "mxUtils.h"
#include "version.h"
#include "mxSizers.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxAboutWindow, wxDialog)

	EVT_BUTTON(wxID_OK,mxAboutWindow::OnCloseButton)
	EVT_HTML_LINK_CLICKED(wxID_ANY, mxAboutWindow::OnLink)
	EVT_CLOSE(mxAboutWindow::OnClose)

END_EVENT_TABLE()

mxAboutWindow::mxAboutWindow(wxWindow* parent) : wxDialog(parent, wxID_ANY, LANG(ABOUT_CAPTION,"Acerca de..."), wxDefaultPosition, wxDefaultSize, wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
		
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(ABOUT_CLOSE,"Cerrar"));
	ok_button->SetDefault(); 
	SetEscapeId(wxID_OK);
	
	wxString version(wxString(_T("ZinjaI-"))<<ARCHITECTURE
#ifdef DEBUG
		<<_T("-debug")
#endif
		<<_T("-")<<VERSION);
	
	bottomSizer->Add(new wxStaticText(this,wxID_ANY,version),sizers->BA5_Center);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(ok_button,sizers->BA5);

	mySizer->Add(
		html = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(450,350))
		,sizers->Exp1);
	mySizer->Add(bottomSizer,sizers->Exp0);
	SetSizerAndFit(mySizer);
	html->LoadPage(DIR_PLUS_FILE(config->Help.guihelp_dir,wxString(_T("about_"))<<config->Init.language_file<<_T(".html")));
	ShowModal();

}

void mxAboutWindow::OnCloseButton(wxCommandEvent &event){
	Destroy();
}

void mxAboutWindow::OnClose(wxCloseEvent &event){
	Destroy();
}


void mxAboutWindow::OnLink (wxHtmlLinkEvent &event) {
	if (event.GetLinkInfo().GetHref().StartsWith(_T("http://"))) {
		utils->OpenInBrowser(event.GetLinkInfo().GetHref());
	} else {
		event.Skip();
	}
}
