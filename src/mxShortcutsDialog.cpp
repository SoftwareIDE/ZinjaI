#include "mxShortcutsDialog.h"

#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include "mxBitmapButton.h"
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include "Language.h"
#include "mxSizers.h"
#include "MenusAndToolsConfig.h"

#define _CAPTION LANG(SHORCUTS_CAPTION,"Atajos de teclado")
#include "mxHelpWindow.h"
#include "mxMessageDialog.h"
#include "ConfigManager.h"

class mxShortcutGrabber:public wxDialog {
	wxTextCtrl *destiny,*current;
	wxString key;
	bool alt,ctrl,shift;
public:
	mxShortcutGrabber(wxWindow *parent, wxTextCtrl *dest) : wxDialog(parent,wxID_ANY,_CAPTION,wxDefaultPosition,wxSize(300,100),wxDEFAULT_DIALOG_STYLE) {
		destiny = dest; key="...";
		alt=ctrl=shift=false;
		wxSizer *sz = new wxBoxSizer(wxVERTICAL);
		sz->AddStretchSpacer(1);
		sz->Add(current = new wxTextCtrl(this,wxID_ANY,""),sizers->Exp0);
		sz->AddStretchSpacer(1);
		SetSizer(sz);
		ShowCurrent();
		current->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( mxShortcutGrabber::OnKeyDown), nullptr, this );
		current->Connect( wxEVT_KEY_UP, wxKeyEventHandler( mxShortcutGrabber::OnKeyUp), nullptr, this );
	}
	void ShowCurrent() {
		wxString text;
		if (ctrl) text<<"Ctrl+";
		if (shift) text<<"Shift+";
		if (alt) text<<"Alt+";
		if (key!="..." || text.Len()) text<<key; 
		else text="Presione una combinación de teclas";
		current->SetValue(text);
		current->SetFocus();
	}
	wxString getName(int code) {
		if (code==WXK_BACK) return "Back";
		if (code==WXK_TAB) return "Tab";
		if (code==WXK_RETURN) return "Return";
		if (code==WXK_ESCAPE) return "Escape";
		if (code==WXK_SPACE) return "Space";
		if (code==WXK_DELETE) return "Delete";
		if (code==WXK_START) return "Start";
//		if (code==WXK_LBUTTON) return "Lbutton";
//		if (code==WXK_RBUTTON) return "Rbutton";
		if (code==WXK_CANCEL) return "Cancel";
//		if (code==WXK_MBUTTON) return "Mbutton";
		if (code==WXK_CLEAR) return "Clear";
//		if (code==WXK_SHIFT) return "Shift";
//		if (code==WXK_ALT) return "Alt";
//		if (code==WXK_CONTROL) return "Control";
		if (code==WXK_MENU) return "Menu";
		if (code==WXK_PAUSE) return "Pause";
		if (code==WXK_CAPITAL) return "Capital";
		if (code==WXK_END) return "End";
		if (code==WXK_HOME) return "Home";
		if (code==WXK_LEFT) return "Left";
		if (code==WXK_UP) return "Up";
		if (code==WXK_RIGHT) return "Right";
		if (code==WXK_DOWN) return "Down";
		if (code==WXK_SELECT) return "Select";
		if (code==WXK_PRINT) return "Print";
		if (code==WXK_EXECUTE) return "Execute";
		if (code==WXK_SNAPSHOT) return "Snapshot";
		if (code==WXK_INSERT) return "Insert";
		if (code==WXK_HELP) return "Help";
		if (code==WXK_NUMPAD0) return "Numpad0";
		if (code==WXK_NUMPAD1) return "Numpad1";
		if (code==WXK_NUMPAD2) return "Numpad2";
		if (code==WXK_NUMPAD3) return "Numpad3";
		if (code==WXK_NUMPAD4) return "Numpad4";
		if (code==WXK_NUMPAD5) return "Numpad5";
		if (code==WXK_NUMPAD6) return "Numpad6";
		if (code==WXK_NUMPAD7) return "Numpad7";
		if (code==WXK_NUMPAD8) return "Numpad8";
		if (code==WXK_NUMPAD9) return "Numpad9";
		if (code==WXK_MULTIPLY) return "Multiply";
		if (code==WXK_ADD) return "Add";
		if (code==WXK_SEPARATOR) return "Separator";
		if (code==WXK_SUBTRACT) return "Subtract";
		if (code==WXK_DECIMAL) return "Decimal";
		if (code==WXK_DIVIDE) return "Divide";
		if (code==WXK_F1) return "F1";
		if (code==WXK_F2) return "F2";
		if (code==WXK_F3) return "F3";
		if (code==WXK_F4) return "F4";
		if (code==WXK_F5) return "F5";
		if (code==WXK_F6) return "F6";
		if (code==WXK_F7) return "F7";
		if (code==WXK_F8) return "F8";
		if (code==WXK_F9) return "F9";
		if (code==WXK_F10) return "F10";
		if (code==WXK_F11) return "F11";
		if (code==WXK_F12) return "F12";
		if (code==WXK_F13) return "F13";
		if (code==WXK_F14) return "F14";
		if (code==WXK_F15) return "F15";
		if (code==WXK_F16) return "F16";
		if (code==WXK_F17) return "F17";
		if (code==WXK_F18) return "F18";
		if (code==WXK_F19) return "F19";
		if (code==WXK_F20) return "F20";
		if (code==WXK_F21) return "F21";
		if (code==WXK_F22) return "F22";
		if (code==WXK_F23) return "F23";
		if (code==WXK_F24) return "F24";
		if (code==WXK_NUMLOCK) return "NumLock";
		if (code==WXK_SCROLL) return "Scroll";
		if (code==WXK_PAGEUP) return "PageUp";
		if (code==WXK_PAGEDOWN) return "PageDown";
		if (code==WXK_NUMPAD_SPACE) return "Numpad_Space";
		if (code==WXK_NUMPAD_TAB) return "Numpad_Yab";
		if (code==WXK_NUMPAD_ENTER) return "Numpad_Enter";
//		if (code==WXK_NUMPAD_F1) return "Numpad_f1";
//		if (code==WXK_NUMPAD_F2) return "Numpad_f2";
//		if (code==WXK_NUMPAD_F3) return "Numpad_f3";
//		if (code==WXK_NUMPAD_F4) return "Numpad_f4";
		if (code==WXK_NUMPAD_HOME) return "Numpad_Home";
		if (code==WXK_NUMPAD_LEFT) return "Numpad_Left";
		if (code==WXK_NUMPAD_UP) return "Numpad_Up";
		if (code==WXK_NUMPAD_RIGHT) return "Numpad_Right";
		if (code==WXK_NUMPAD_DOWN) return "Numpad_Down";
		if (code==WXK_NUMPAD_PAGEUP) return "Numpad_PageUp";
		if (code==WXK_NUMPAD_PAGEDOWN) return "Numpad_PageDown";
		if (code==WXK_NUMPAD_END) return "Numpad_End";
		if (code==WXK_NUMPAD_BEGIN) return "Numpad_Begin";
		if (code==WXK_NUMPAD_INSERT) return "Numpad_Insert";
		if (code==WXK_NUMPAD_DELETE) return "Numpad_Delete";
		if (code==WXK_NUMPAD_EQUAL) return "Numpad_Equal";
		if (code==WXK_NUMPAD_MULTIPLY) return "Numpad_Multiply";
		if (code==WXK_NUMPAD_ADD) return "Numpad_Add";
		if (code==WXK_NUMPAD_SEPARATOR) return "Numpad_Separator";
		if (code==WXK_NUMPAD_SUBTRACT) return "Numpad_Subtract";
		if (code==WXK_NUMPAD_DECIMAL) return "Numpad_Decimal";
		if (code==WXK_NUMPAD_DIVIDE) return "Numpad_Divide";
		if (code==WXK_WINDOWS_LEFT) return "Windows_Left";
		if (code==WXK_WINDOWS_RIGHT) return "Windows_Right";
		if (code==WXK_WINDOWS_MENU) return "Windows_Menu";
		if (code==WXK_COMMAND) return "Command";
		if (code>0 && code<256) { wxString s(" "); s[0]=code; return s.Upper(); }
		return "...";
	}
	void OnKeyDown(wxKeyEvent &evt) {
		if (evt.GetKeyCode()==WXK_SHIFT) shift=true;
		else if (evt.GetKeyCode()==WXK_ALT) alt=true;
		else if (evt.GetKeyCode()==WXK_CONTROL) ctrl=true;
		else key=getName(evt.GetKeyCode());
		ShowCurrent();
		if (key!="...") { destiny->SetValue(current->GetValue()); EndModal(0); }
	}
	void OnKeyUp(wxKeyEvent &evt) {
		if (evt.GetKeyCode()==WXK_SHIFT) shift=false;
		else if (evt.GetKeyCode()==WXK_ALT) alt=false;
		else if (evt.GetKeyCode()==WXK_CONTROL) ctrl=false;
		ShowCurrent();
	}
};


static wxString normalize(wxString str) {
	str.MakeUpper();
	if (str.Contains("SHIFT+")) { str.Replace("SHIFT+","",true); str=wxString("SHIFT+")+str; }
	if (str.Contains("ALT+")) { str.Replace("ALT+","",true); str=wxString("ALT+")+str; }
	if (str.Contains("CTRL+")) { str.Replace("CTRL+","",true); str=wxString("CTRL+")+str; }
	return str;
}



BEGIN_EVENT_TABLE(mxShortcutsDialog,wxDialog)
	EVT_TEXT(wxID_FIND,mxShortcutsDialog::OnFilter)
	EVT_TIMER(wxID_ANY,mxShortcutsDialog::OnTimer)
	EVT_BUTTON(wxID_OPEN,mxShortcutsDialog::OnGrabButton)
	EVT_BUTTON(wxID_OK,mxShortcutsDialog::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxShortcutsDialog::OnCancelButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxShortcutsDialog::OnHelpButton)
END_EVENT_TABLE()

mxShortcutsDialog::mxShortcutsDialog(wxWindow *parent) : wxDialog(parent,wxID_ANY,_CAPTION,wxDefaultPosition,wxSize(350,400),wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER), timer(GetEventHandler(),wxID_ANY) {

	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*bitmaps->buttons.help);
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar")); 
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	ok_button->SetMinSize(wxSize(ok_button->GetSize().GetWidth()<80?80:ok_button->GetSize().GetWidth(),ok_button->GetSize().GetHeight()));
	ok_button->SetDefault(); 
	
	bottomSizer->Add(help_button,sizers->BA5);
	bottomSizer->AddStretchSpacer(1);
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	
	wxScrolledWindow *scroll = new wxScrolledWindow(this,wxID_ANY);
	sizer = new wxBoxSizer(wxVERTICAL);
	scroll->SetScrollRate(10,10);
	scroll->SetSizer(sizer);
	
	wxBoxSizer *filterSizer = new wxBoxSizer(wxHORIZONTAL);
	filterSizer->Add(new wxStaticText(this,wxID_ANY,LANG(SHORCUTS_FILTER,"Buscar:")),sizers->Center);
	filterSizer->Add(filter = new wxTextCtrl(this,wxID_FIND,""),sizers->Exp1);
	
	mySizer->Add(filterSizer,sizers->BA5_Exp0);
	
	mySizer->Add(scroll,sizers->BA5_Exp1);
	mySizer->Add(bottomSizer,sizers->BA5_Exp0);
	SetSizer(mySizer);
	
	for(unsigned int i=0;i<MenusAndToolsConfig::mnCOUNT;i++) { 
		MenusAndToolsConfig::myMenu &menu = menu_data->GetMyMenu(i);
		wxString label = menu.label + "->" , full_label = menu.label + "->";
		for(unsigned int j=0;j<menu.items.size();j++) { 
			int props=menu.items[j].properties;
			if (props&MenusAndToolsConfig::maSEPARATOR) continue;
			if (props&MenusAndToolsConfig::maBEGIN_SUBMENU) { full_label = label + menu.items[j].label + " -> "; continue; }
			if (props&MenusAndToolsConfig::maEND_SUBMENU) { full_label = label; continue; }
			wxBoxSizer *btsizer = new wxBoxSizer(wxHORIZONTAL);
			wxString item_label = full_label + menu.items[j].label;
			item_label.Replace("&","",true);
			entry e; e.search_text = item_label.AfterFirst('>');
			btsizer->Add(e.label = new wxStaticText(scroll,wxID_ANY,item_label),sizers->BA5_Exp1);
			btsizer->Add(e.text = new wxTextCtrl(scroll,wxID_ANY,menu.items[j].shortcut),sizers->Center);
			btsizer->Add(e.button = new wxButton(scroll,wxID_OPEN,"...",wxDefaultPosition,wxSize(30,10)),sizers->BA5_Exp0);
			e.text->SetMinSize(wxSize(e.text->GetSize().GetWidth()*2,e.text->GetSize().GetHeight()));
			e.menu_item=&(menu.items[j]);
			sizer->Add(btsizer,sizers->Exp0); actions.Add(e);
		}
	}
	
	sizer->Layout(); 
	SetSize(sizer->GetMinSize().GetWidth()+wxSystemSettings::GetMetric(wxSYS_VSCROLL_X)+10,GetSize().GetHeight());
	filter->SetFocus();
	ShowModal();
}

void mxShortcutsDialog::OnTimer (wxTimerEvent & evt) {
	wxString text = filter->GetValue().Upper();
	wxString keys = normalize(filter->GetValue());
	for(int i=0;i<actions.GetSize();i++) { 
		bool visible = text.Len()==0 || actions[i].search_text.Upper().Contains(text) || normalize(actions[i].text->GetValue()).Contains(keys);
		actions[i].text->Show(visible);
		actions[i].label->Show(visible);
		actions[i].button->Show(visible);
	}
	sizer->Layout();
	GetSizer()->Layout();
}

void mxShortcutsDialog::OnFilter (wxCommandEvent & evt) {
	timer.Start(500,true);
}

void mxShortcutsDialog::OnGrabButton (wxCommandEvent & evt) {
	for(int i=0;i<actions.GetSize();i++) { 
		if (actions[i].button==evt.GetEventObject()) {
			mxShortcutGrabber(this,actions[i].text).ShowModal();
			return;
		}
	}
}

void mxShortcutsDialog::OnOkButton (wxCommandEvent & evt) {
	// verificar que no se repitan
	for(int i=0;i<actions.GetSize();i++) {
		wxString norm = normalize(actions[i].text->GetValue());
		if (!norm.Len()) continue;
		for(int j=i+1;j<actions.GetSize();j++) {
			if (norm == normalize(actions[j].text->GetValue())) {
				mxMessageDialog(this,LANG2(SHORCUTS_COLLISION,"Dos acciones tienen el mismo atajo:\n     <{1}>\n     <{2}>",actions[i].label->GetLabel(),actions[j].label->GetLabel()),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
				filter->SetValue(actions[j].text->GetValue());
				wxTimerEvent t_evt;	OnTimer(t_evt);
				return;
			}
		}
	}
	
	for(int i=0;i<actions.GetSize();i++) 
		actions[i].menu_item->RedefineShortcut(actions[i].text->GetValue());
	menu_data->SaveShortcutsSettings(DIR_PLUS_FILE(config->home_dir,"shortcuts.zsc"));
	
	menu_data->CreateMenues();
	EndModal(1);
}

void mxShortcutsDialog::OnCancelButton (wxCommandEvent & evt) {
	EndModal(0);
}

void mxShortcutsDialog::OnHelpButton (wxCommandEvent & evt) {
	mxHelpWindow::ShowHelp("shortcuts_dialog.html",this);
}

