#include <wx/choicdlg.h>
#include <wx/button.h>

#include "mxWelcomePanel.h"

#include "mxMainWindow.h"
#include "ConfigManager.h"
#include "mxUtils.h"
#include "version.h"
#include "mxNewWizard.h"
#include "mxTipsWindow.h"
#include "mxArt.h"
#include "mxOpenRecentDialog.h"
#include "Language.h"
#include "mxApplication.h" // SHOW_MILLIS
#include "mxHelpWindow.h"
#include <wx/settings.h>

mxWelcomePanel *g_welcome_panel = nullptr;

BEGIN_EVENT_TABLE(mxWelcomePanel, wxHtmlWindow)
	EVT_HTML_LINK_CLICKED(wxID_ANY, mxWelcomePanel::OnLinkClicked)
END_EVENT_TABLE()

mxWelcomePanel::mxWelcomePanel(wxWindow *parent):wxHtmlWindow(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxHW_NO_SELECTION|wxHW_DEFAULT_STYLE) {
	is_visible=false;
	wxString tips_file=DIR_PLUS_FILE(config->Help.guihelp_dir,wxString(_T("tips_"))<<config->Init.language_file);
	if (wxFileName::FileExists(tips_file)) file.Open(tips_file);
}

void mxWelcomePanel::Reload() {
	static wxString source;
	bool skin_file=wxFileName::FileExists(DIR_PLUS_FILE(config->Files.skin_dir,wxString(_T("welcome_panel_"))<<config->Init.language_file<<_T(".html")));
	if (!source.Len()) {
		wxTextFile fil(SKIN_FILE(wxString(_T("welcome_panel_"))<<config->Init.language_file<<_T(".html")));
		fil.Open();
		for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
			source<<str;
		}
		fil.Close();
	}
	wxString text=source;
	if (skin_file)
		text.Replace(_T("src=\""),wxString(_T("src=\""))<<DIR_PLUS_FILE(config->Files.skin_dir,""));
	else
		text.Replace(_T("src=\""),wxString(_T("src=\""))<<DIR_PLUS_FILE(_T("imgs"),""));
	text.Replace(_T("${ZVERSION}"),wxString()<<VERSION);
	wxColour background_colour=wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE );
	wxColour foreground_colour=wxSystemSettings::GetColour( wxSYS_COLOUR_BTNTEXT );
	text.Replace(_T("${BUTTON_COLOR}"),background_colour.GetAsString(wxC2S_HTML_SYNTAX));
	text.Replace(_T("${LABEL_COLOR}"),foreground_colour.GetAsString(wxC2S_HTML_SYNTAX));
	text.Replace(_T("${FILE:01}"),config->Files.last_source[0]);
	text.Replace(_T("${FILE:02}"),config->Files.last_source[1]);
	text.Replace(_T("${FILE:03}"),config->Files.last_source[2]);
	text.Replace(_T("${FILE:04}"),config->Files.last_source[3]);
	text.Replace(_T("${FILE:05}"),config->Files.last_source[4]);
	text.Replace(_T("${PROJECT:01}"),config->Files.last_project[0]);
	text.Replace(_T("${PROJECT:02}"),config->Files.last_project[1]);
	text.Replace(_T("${PROJECT:03}"),config->Files.last_project[2]);
	text.Replace(_T("${PROJECT:04}"),config->Files.last_project[3]);
	text.Replace(_T("${PROJECT:05}"),config->Files.last_project[4]);
	if (file.IsOpened()) {
		wxString tip;
		while (!tip.Len())
			tip=file[TIPS_TO_SKIP+1+rand()%(file.GetLineCount()-TIPS_TO_SKIP-1)];
//			tip=file[file.GetLineCount()-1];
		text.Replace(_T("${TIP}"),tip);
	} else {
		text.Replace(_T("${TIP}"),LANG(TIPS_DB_NOT_FOUND,"No se pudo abrir correctamente el archivo de tips y sugerencias."));
	}
	SetPage(text);
}

void mxWelcomePanel::OnLinkClicked (wxHtmlLinkEvent &event) {
	wxString action(event.GetLinkInfo().GetHref().BeforeFirst(':'));
	if (action==_T("http")) {
		mxUT::OpenInBrowser(event.GetLinkInfo().GetHref());
	} else if (action==_T("help")) {
		mxHelpWindow::ShowHelp(event.GetLinkInfo().GetHref().AfterFirst(':'));
	} else if (action==_T("open")) {
		main_window->OpenFileFromGui(event.GetLinkInfo().GetHref().AfterFirst(':') );
	} else if (action==_T("new_simple")) {
		if (!g_wizard) g_wizard = new mxNewWizard(main_window);
		g_wizard->RunWizard(_T("templates"));
	} else if (action==_T("new_project")) {
		if (!g_wizard) g_wizard = new mxNewWizard(main_window);
		g_wizard->RunWizard(_T("new_project"));
	} else if (action==_T("tip")) {
		Freeze();
		int x,y;
		GetViewStart(&x,&y);
		Reload();
		Scroll(x,y);
		Thaw();
	} else if (action==_T("never_again")) {
		config->Init.show_welcome=false;
		main_window->ShowWelcome(false);
		main_window->aui_manager.DetachPane(g_welcome_panel);
		g_welcome_panel=nullptr;
		Destroy();
		if (config->Init.new_file==0)
			main_window->NewFileFromText("");
		else
			main_window->NewFileFromTemplate(mxUT::WichOne(config->Files.default_template,"templates",true));
	} else if (action==_T("open_file")) {
		wxCommandEvent evt;
		main_window->OnFileOpen(evt);
	} else if (action==_T("recent_files")) {
		new mxOpenRecentDialog(main_window,false);
//		int n=0; unsigned int max_len=0;
//		for (int i=0;i<CM_HISTORY_MAX_LEN;i++) 
//			if (config->Files.last_source[i][0]!=0) {
//				n++; 
//				if (config->Files.last_source[i].Len()>max_len)
//					max_len=config->Files.last_source[i].Len();
//			} else break;
//		wxString file=wxGetSingleChoice(wxString(' ',max_len*2),_T("Abrir"),n,config->Files.last_source,this,wxDefaultCoord,wxDefaultCoord,true,450,250);
//		if (file.Len()) main_window->OpenFileFromGui(file);
	} else if (action==_T("recent_projects")) {
		new mxOpenRecentDialog(main_window,true);
//		int n=0; unsigned int max_len=0;
//		for (int i=0;i<CM_HISTORY_MAX_LEN;i++) 
//			if (config->Files.last_project[i][0]!=0) {
//				n++; 
//				if (config->Files.last_project[i].Len()>max_len)
//					max_len=config->Files.last_project[i].Len();
//			} else break;
//		wxString file=wxGetSingleChoice(wxString(' ',max_len*2),_T("Abrir"),n,config->Files.last_project,this,wxDefaultCoord,wxDefaultCoord,true,450,250);
//		if (file.Len()) main_window->OpenFileFromGui(file);
	}
}

bool mxWelcomePanel::IsVisible ( ) {
	return is_visible;
}

