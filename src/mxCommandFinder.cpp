#include "mxCommandFinder.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include "Language.h"
#include "mxMainWindow.h"
#include "MenusAndToolsConfig.h"
#include <wx/settings.h>
#include "Cpp11.h"

BEGIN_EVENT_TABLE(mxCommandFinderText,wxTextCtrl)
	EVT_TEXT(wxID_ANY,mxCommandFinderText::OnText)
	EVT_KEY_DOWN(mxCommandFinderText::OnKey)
END_EVENT_TABLE();

mxCommandFinder::mxCommandFinder ( ) 
	: wxDialog(main_window,wxID_ANY,LANG(FINDCOMMAND_CAPTION,"Buscar comando"),
			   wxPoint((3*main_window->GetScreenRect().GetLeft()+main_window->GetScreenRect().GetRight())/4,
					   (3*main_window->GetScreenRect().GetTop()+main_window->GetScreenRect().GetBottom())/4),
			   wxDefaultSize)
{
	wxSizer * sizer = new wxBoxSizer(wxVERTICAL);
	text = new mxCommandFinderText(this);
	sizer->Add(text,wxSizerFlags().Proportion(0).Expand());
	SetSizerAndFit(sizer);
	text->SetFocus();
}

static bool matches(const wxArrayString &array, const wxString &item){
	static char *map = nullptr;
	if (!map) {
		map = new char[256];
		for(int i=0;i<256;i++)
			map[i] = toupper(char((unsigned int)(i)));
		map[(unsigned char)('á')] = 'A';
		map[(unsigned char)('é')] = 'E';
		map[(unsigned char)('í')] = 'I';
		map[(unsigned char)('ó')] = 'O';
		map[(unsigned char)('ú')] = 'U';
		map[(unsigned char)('ü')] = 'U';
		map[(unsigned char)('Á')] = 'A';
		map[(unsigned char)('É')] = 'E';
		map[(unsigned char)('Í')] = 'I';
		map[(unsigned char)('Ó')] = 'O';
		map[(unsigned char)('Ú')] = 'U';
		map[(unsigned char)('Ü')] = 'U';
		map[(unsigned char)('ñ')] = 'N';
		map[(unsigned char)('Ñ')] = 'N';
	}
	int li = item.Len(), found = 0;
	for(unsigned int i=0;i<array.GetCount();i++) { 
		const wxString &key = array[i];
		int lk = key.Len();
		for(int j=0;j<=li-lk;j++) { 
			int ik = 0, ii = j;
			while (ik<lk&&ii<li) {
//				if (item[ii]=='&') { ++ii; continue; }
				char ci = item[ii], ck = key[ik];
				if (map[(unsigned char)(ci)]!=map[(unsigned char)(ck)]) break;
				++ii; ++ik;
			}
			if (ik==lk) { ++found; break; }
		}
	}
	return found == int(array.GetCount());
}

void mxCommandFinderList::SetPattern (wxString str, int x, int y) {
	list->Clear(); ids.clear();
	wxArrayString array;
	mxUT::Split(str,array,true,false);
	if (!array.GetCount()) { Hide(); return; }
	Show();
	for(unsigned int i=0;i<MenusAndToolsConfig::mnCOUNT;i++) { 
		MenusAndToolsConfig::myMenu &menu = menu_data->GetMyMenu(i);
		wxString label = menu.label + "->" , full_label = menu.label + "->";
		for(unsigned int j=0;j<menu.items.size();j++) { 
			int props = menu.items[j].properties;
			if (props&MenusAndToolsConfig::maSEPARATOR) {
				continue;
			} else if (props&MenusAndToolsConfig::maBEGIN_SUBMENU) { 
				if (full_label!=label) { while (!(menu.items[++j].properties&MenusAndToolsConfig::maEND_SUBMENU)); continue; } // gprof's sub-sub-menu
				full_label = label + menu.items[j].label + " -> "; continue; 
			} else if (props&MenusAndToolsConfig::maEND_SUBMENU) { 
				full_label = label; continue; 
			}
			wxString item_label = full_label + menu.items[j].label;
			item_label.Replace("&","",true);
			if (matches(array,item_label)) { list->Append(item_label); ids.push_back(menu.items[j].wx_id); }
		}
	}
	if (!list->GetCount()) list->Append("<<no results>>"); else list->SetSelection(0);
	list->Append("");
	pos_and_size = wxRect(x,y,list->GetBestSize().GetWidth()+10,list->GetBestSize().GetHeight()+wxSystemSettings::GetMetric(wxSYS_VSCROLL_X)+10);
	SetSize(pos_and_size); timer.Start(100,true);
	list->Delete(list->GetCount()-1);
}

void mxCommandFinderList::KeyUp ( ) {
	int sel = list->GetSelection();
	if (sel>0&&sel<int(list->GetCount()))
		list->SetSelection(sel-1);
}

void mxCommandFinderList::KeyDown ( ) {
	int sel = list->GetSelection();
	if (sel>=0&&sel<int(list->GetCount())-1)
		list->SetSelection(sel+1);
}

void mxCommandFinderList::Select ( ) {
	int sel = list->GetSelection();
	if (sel>=0&&sel<int(list->GetCount())) {
		GetParent()->Close();
		wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,ids[sel]);
		main_window->ProcessEvent(evt);
	}
}

void mxCommandFinderText::OnText (wxCommandEvent & evt) {
	int x = GetScreenRect().GetBottomLeft().x;
	int y = GetScreenRect().GetBottomLeft().y;
	list.SetPattern(GetValue(),x,y);
	GetParent()->Raise();
}

mxCommandFinderList::mxCommandFinderList (wxWindow * parent) 
	: wxDialog(parent,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxNO_BORDER|wxSTAY_ON_TOP)
	, timer(GetEventHandler())
{
	list = new wxListBox(this,wxID_ANY);
	wxSizer * sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(list,wxSizerFlags().Proportion(1).Expand());
	SetSizerAndFit(sizer);
	Connect(wxEVT_TIMER,wxTimerEventHandler(mxCommandFinderList::OnTimer),NULL,this);
}

void mxCommandFinderText::OnKey (wxKeyEvent &evt) {
	if (evt.GetKeyCode()==WXK_RETURN) list.Select();
	else if (evt.GetKeyCode()==WXK_DOWN) list.KeyDown();
	else if (evt.GetKeyCode()==WXK_UP) list.KeyUp();
	else if (evt.GetKeyCode()==WXK_ESCAPE) GetParent()->Close();
	else evt.Skip();
}

mxCommandFinderText::mxCommandFinderText (wxWindow * parent) 
	: wxTextCtrl(parent,wxID_ANY,"",wxDefaultPosition,wxSize(300,-1),wxTE_PROCESS_ENTER), list(parent) 
{
	SetToolTip(LANG(FINDCOMMAND_HELP,"Introduzca una o más palabras claves para buscar, y presione Enter para seleccionar un resultado"));
}

void mxCommandFinderList::OnTimer (wxTimerEvent & evt) {
	SetSize(pos_and_size);
}

