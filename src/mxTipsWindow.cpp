#include <wx/wx.h>
#include <wx/checkbox.h>
#include <wx/html/htmlwin.h>
#include <wx/textfile.h>
#include <wx/statbmp.h>

#include "mxTipsWindow.h"

#include "ids.h"
#include "ConfigManager.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include "version.h"
#include "mxHelpWindow.h"
#include "mxSizers.h"
#include "Language.h"

#define TIP_INITIAL_TEXT "<B>Sabias que...?</B><BR><BR>&nbsp;&nbsp;&nbsp;"
mxTipsWindow *tips_window=nullptr;

BEGIN_EVENT_TABLE(mxTipsWindow, wxDialog)
	EVT_BUTTON(mxID_TIP_OTHERONE,mxTipsWindow::OnOtherOne)
	EVT_BUTTON(mxID_TIP_CLOSE,mxTipsWindow::OnButtonClose)
	EVT_CHECKBOX(wxID_ANY, mxTipsWindow::OnCheckbox)
	EVT_CLOSE(mxTipsWindow::OnClose)
	EVT_KEY_UP(mxTipsWindow::OnKeyDown)
	EVT_CHAR_HOOK(mxTipsWindow::OnKeyDown)
	EVT_HTML_LINK_CLICKED(wxID_ANY, mxTipsWindow::OnQuickHelpLink)
END_EVENT_TABLE()

void mxTipsWindow::OnButtonClose(wxCommandEvent &event){
	tips_window=nullptr;
	Destroy();
}
void mxTipsWindow::OnClose(wxCloseEvent &event){
	tips_window=nullptr;
	Destroy();
}

mxTipsWindow::~mxTipsWindow() {
	tips_window=nullptr;
	file.Close();
}

void mxTipsWindow::OnCheckbox (wxCommandEvent &event){
	config->Init.show_tip_on_startup=showtip_checkbox->GetValue();
}

void mxTipsWindow::OnOtherOne(wxCommandEvent &event){
	ShowAnyTip();
}
void mxTipsWindow::ShowAnyTip() {
	if (!file.IsOpened()) return;
	if (config->Init.version!=VERSION) {
		wxString tip(wxString("<B>")<<LANG(TIPS_WELCOME,"Bienvenido a ZinjaI ")<<VERSION<<"</B><BR><BR>");
#ifdef __APPLE__
		tip<<"[EN] Important notice: due to the lack of propper hardware, ZinjaI's Mac binaries are released without being properly tested, so you should expect to find some bugs. You can report them and ask for help in ZinjaI's forums.<BR><BR>";
		tip<<"[ES] Importante: por falta de hardware adecuado, las versiones para Mac de ZinjaI se publican sin haber sido testeadas, por lo que seguramente encontrará bugs. Puede reportarlos y solicitar ayuda en los foros del sitio web de ZinjaI.<BR><BR>";
#else
		tip<<file[config->Init.version?1:0];
#endif
		changelog = config->Init.version;
		tip<<"<BR><BR><CENTER>http://zinjai.sourceforge.net<BR>"<<LANG(TIP_EMAIL,"Correo")<<": zaskar_84@yahoo.com.ar</CENTER>";
		tooltip->SetPage(tip);
		config->Init.version=VERSION;
	} else {
		changelog=false;
		wxString tip(TIP_INITIAL_TEXT);
		if (retry_num==42) {
			tip<<"Exploradores de una raza de seres pandimensionales e hiperinteligentes construyen <I>Pensamiento Profundo</I>, el segundo mejor ordenador del universo, para calcular el sentido de la vida, el universo y todo lo demás. Después de siete millones y medio de años meditando la pregunta, <I>Pensamiento Profundo</I> revela la respuesta:"
				"<BR>- ¡<B>Cuarenta y dos</B>!<BR>- ¿Es eso todo lo que tienes que mostrar tras siete millones y medio de años de trabajo?<BR>- Lo he comprobado muy minuciosamente -dijo el ordenador-, y ésa es casi definitivamente la respuesta.<BR><BR>de <I>La Guia del Autoestopista Galáctico</I>, por <I>Douglas Adams</I><BR>";
		} else {
			while (tip==TIP_INITIAL_TEXT)
				tip<<file[TIPS_TO_SKIP+rand()%(file.GetLineCount()-TIPS_TO_SKIP)];
//				tip<<file[file.GetLineCount()-1];
		}
		tooltip->SetPage(tip);
	}
	retry_num++;
}
	

mxTipsWindow::mxTipsWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, LANG(TIPS_CAPTION,"Sugerencias del dia"), pos, size, style) {
	tips_window=this;
	retry_num=0;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	tooltip = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(300,175));
	
	wxString tips_file=DIR_PLUS_FILE(config->Help.guihelp_dir,wxString("tips_")<<config->Init.language_file);
	if (wxFileName::FileExists(tips_file)) file.Open(tips_file);

	if (file.IsOpened())	
		ShowAnyTip();
	else
		tooltip->SetPage(wxString("<B>")<<LANG(TIPS_DB_NOT_FOUND,"Error: No se pudo abrir el archivo de sugerencias")<<"</B>");	

	showtip_checkbox = new wxCheckBox(this, wxID_ANY, LANG(TIPS_SHOW_AT_START,"Mostrar sugerencias al inicio   "));
	showtip_checkbox->SetValue(config->Init.show_tip_on_startup);
	wxButton *otherone_button = new mxBitmapButton (this,mxID_TIP_OTHERONE,bitmaps->buttons.ok,LANG(TIPS_OTHER,"Otra")); 
	wxButton *close_button = new mxBitmapButton (this,mxID_TIP_CLOSE,bitmaps->buttons.cancel,LANG(TIPS_CLOSE,"Cerrar")); 
	SetAffirmativeId(mxID_TIP_OTHERONE);
	SetEscapeId(mxID_TIP_CLOSE);

	bottomSizer->Add(showtip_checkbox, sizers->BA5_Exp1);
	bottomSizer->Add(otherone_button,sizers->BA5);
	bottomSizer->Add(close_button,sizers->BA5);

	topSizer->Add(new wxStaticBitmap(this,wxID_ANY, bitmaps->GetBitmap("bigtip.png")), sizers->Center);
	topSizer->Add(tooltip, sizers->Exp1);

	mySizer->Add(topSizer, sizers->Exp1);
	mySizer->Add(bottomSizer, sizers->Exp0);
	SetSizerAndFit(mySizer);

	tooltip->SetFocus();
	Show(); close_button->SetFocus();
	Raise();
}

void mxTipsWindow::OnKeyDown(wxKeyEvent &evt) {
	if (evt.GetKeyCode()==WXK_ESCAPE) {
		Close();
	} else if (evt.GetKeyCode()==WXK_RETURN || evt.GetKeyCode()==WXK_NUMPAD_ENTER) {
		ShowAnyTip();
	} else if (evt.GetKeyCode()==WXK_F1) {
		Close();
		mxHelpWindow::ShowHelp(changelog?"ChangeLog.html":"index.html");
	} else evt.Skip();
}

void mxTipsWindow::OnQuickHelpLink (wxHtmlLinkEvent &event) {
	wxString action(event.GetLinkInfo().GetHref().BeforeFirst(':'));
	if (action=="http") {
		mxUT::OpenInBrowser(event.GetLinkInfo().GetHref());
	} else if (action=="help") {
		mxHelpWindow::ShowHelp(event.GetLinkInfo().GetHref().AfterFirst(':'));
	}
}
