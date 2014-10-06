#include <wx/sizer.h>
#include <wx/arrstr.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include "mxOpenRecentDialog.h"
#include "mxBitmapButton.h"
#include "ProjectManager.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxSource.h"
#include "mxSizers.h"
#include "mxMessageDialog.h"
#include "Language.h"
#include <wx/listctrl.h>
#include "MenusAndToolsConfig.h"

BEGIN_EVENT_TABLE(mxOpenRecentDialog, wxDialog)
	EVT_BUTTON(wxID_OK,mxOpenRecentDialog::OnGotoButton)
	EVT_BUTTON(wxID_FIND,mxOpenRecentDialog::OnClear)
	EVT_BUTTON(wxID_REPLACE,mxOpenRecentDialog::OnDelButton)
	EVT_BUTTON(wxID_CANCEL,mxOpenRecentDialog::OnCancelButton)
	EVT_CLOSE(mxOpenRecentDialog::OnClose)
	EVT_TEXT(wxID_ANY,mxOpenRecentDialog::OnTextChange)
	EVT_CHAR_HOOK(mxOpenRecentDialog::OnCharHook)
	EVT_SIZE(mxOpenRecentDialog::OnResize)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY,mxOpenRecentDialog::OnListDClick)
	EVT_CHECKBOX(wxID_ANY,mxOpenRecentDialog::OnShowDates)
END_EVENT_TABLE()
	
mxOpenRecentDialog::mxOpenRecentDialog(wxWindow* parent, bool aprj) : wxDialog(parent, wxID_ANY, LANG(RECENT_CAPTION,"Archivos Recientes..."), wxDefaultPosition, wxDefaultSize, wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	projects=aprj;
	
//	wxString *array = projects?config->Files.last_project:config->Files.last_source;
//	int n=0; unsigned int max_len=0;
//	for (int i=0;i<CM_HISTORY_MAX_LEN;i++) 
//		if (array[i].Len()) {
//			n++; 
//			if (array[i].Len()>max_len)
//				max_len=array[i].Len();
//		} else break;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	clear_button = new mxBitmapButton (this, wxID_FIND, bitmaps->buttons.cancel, LANG(RECENT_CLEAN,"&Limpiar Lista")); 
	del_button = new mxBitmapButton (this, wxID_REPLACE, bitmaps->buttons.cancel, LANG(RECENT_DELETE,"&Quitar de la Lista")); 
	cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar")); 
	goto_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_GOTO_BUTTON,"  &Ir  "));
	goto_button->SetMinSize(wxSize(goto_button->GetSize().GetWidth()<80?80:goto_button->GetSize().GetWidth(),goto_button->GetSize().GetHeight()));
	goto_button->SetDefault(); 
	
	text_ctrl = new wxTextCtrl(this,wxID_ANY,"");
	list_ctrl = new wxListCtrl(this,wxID_ANY,wxDefaultPosition, wxSize(450,200),wxLC_REPORT|wxLC_SINGLE_SEL);
	list_ctrl->InsertColumn(0,LANG(RECENT_COLUMN_FILE,"Archivo"));
	list_ctrl->InsertColumn(1,LANG(RECENT_COLUMN_DATE,"Fecha Modificacion"));
	
	bottomSizer->Add(clear_button,sizers->BA5);
	bottomSizer->Add(del_button,sizers->BA5);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(goto_button,sizers->BA5);
	
	mySizer->Add(new wxStaticText(this,wxID_ANY,LANG(RECENT_ENTER_FILENAME,"Ingrese parte del nombre del archivo que desea abrir:")),sizers->BLRT5_Exp0);
	mySizer->Add(text_ctrl,sizers->BA5_Exp0);
	mySizer->Add(list_ctrl,sizers->BA5_Exp1);
//	mySizer->Add(new wxStaticText(this,wxID_ANY,wxString(' ',max_len*2)),sizers->BLRT5_Exp0);
	mySizer->Add(show_mod_date = new wxCheckBox(this,wxID_ANY,"Mostrar fecha de modificacion",wxDefaultPosition,wxDefaultSize),sizers->BA5_Exp0);
	mySizer->Add(bottomSizer,sizers->BA5_Exp0);
	SetSizer(mySizer);
	this->SetSize(wxSize(main_window->GetSize().GetWidth()/2,main_window->GetSize().GetHeight()/2));
	
	CenterOnParent();
	Show();
	
	UpdateList();
	text_ctrl->SetSelection(-1,-1);
	text_ctrl->SetFocus();
}

void mxOpenRecentDialog::OnGotoButton(wxCommandEvent &event) {
	wxString key = list_ctrl->GetItemText(GetListSelection());
	Close();
	main_window->OpenFileFromGui(key);
}


void mxOpenRecentDialog::OnCancelButton(wxCommandEvent &event) {
	Close();
}

void mxOpenRecentDialog::OnDelButton(wxCommandEvent &event) {
	int sel = GetListSelection();
	if (sel<0||sel>=int(list_ctrl->GetItemCount())) return;
	wxString todel = list_ctrl->GetItemText(sel);
	list_ctrl->DeleteItem(sel);

	wxString *array = projects?config->Files.last_project:config->Files.last_source;
	bool found=false;
	for (int i=0;i<CM_HISTORY_MAX_LEN-1;i++) {
		if (!found&&array[i]==todel) found=true;
		if (found) array[i]=array[i+1];
	}
	array[CM_HISTORY_MAX_LEN-1]="";
	
	if (array[0].Len()) main_window->UpdateInHistory(array[0],projects);
}

void mxOpenRecentDialog::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxOpenRecentDialog::OnTextChange(wxCommandEvent &event) {
	UpdateList();
}

void mxOpenRecentDialog::UpdateList() {
	list_ctrl->Freeze();
	list_ctrl->DeleteAllItems();
	wxString key = text_ctrl->GetValue();
	key.MakeUpper();
	wxString *array = projects?config->Files.last_project:config->Files.last_source;
	for (unsigned int i=0,count=0;i<CM_HISTORY_MAX_LEN;i++) {
		if (array[i].Len()==0) break;
		wxFileName fn(array[i]);
		fn.Normalize();
		wxString file(fn.GetFullPath());
		if (file.Upper().Find(key)!=wxNOT_FOUND) {
			list_ctrl->InsertItem(count,file);
			if (show_mod_date->GetValue()) {
				if (fn.FileExists())
					list_ctrl->SetItem(count,1,fn.GetModificationTime().Format(LANG(RECENT_DATE_FORMAT,"%d/%m/%Y %H:%M:%S")));
				else
					list_ctrl->SetItem(count,1,LANG(RECENT_DATE_NOT_FOUND,"no encontrado"));
			}
			count++;
		}
	}
	list_ctrl->Thaw();
	if (list_ctrl->GetItemCount()) {
		goto_button->Enable(true);
		SetListSelection(0);
	} else {
		list_ctrl->InsertItem(0,LANG(RECENT_NO_RESULTS,"<<no se encontraron coincidencias>>"));
		goto_button->Enable(false);
	}
	if (show_mod_date->GetValue()) ResizeColums();
}

void mxOpenRecentDialog::OnCharHook (wxKeyEvent &event) {
	if (!list_ctrl->GetItemCount())  {
		event.Skip();
	} else if ( event.GetKeyCode() == WXK_DELETE && FindFocus()!=text_ctrl) {
		wxCommandEvent evt;
		OnDelButton(evt);
	} else if ( event.GetKeyCode() == WXK_UP ) {
		int sel = GetListSelection();
		sel--;
		if (sel==-1)
			sel=list_ctrl->GetItemCount()-1;
		SetListSelection(sel);
//		list_ctrl->SetFirstItem(sel); // para que estaba esto?
	} else if (event.GetKeyCode()==WXK_DOWN) {
		int sel = GetListSelection();
		sel++;
		if (sel==int(list_ctrl->GetItemCount()))
			sel=0;
		SetListSelection(sel);
//		list_ctrl->SetFirstItem(sel); // para que estaba esto?
	} else event.Skip();
}

void mxOpenRecentDialog::OnClear(wxCommandEvent &event) {
	if (projects) {
		wxMenu *menu_recent = menu_data->GetMenu(mxID_FILE_PROJECT_RECENT);
		if (mxMD_YES==mxMessageDialog(this,LANG(RECENT_CONFIRM_CLEAR_PROJECT_HISTORY,"Desea eliminar la lista de proyectos recientes?"), LANG(GENERAL_CONFIRM,"Confirmacion"), mxMD_YES_NO|mxMD_WARNING).ShowModal()) {
			for (unsigned int i=0;i<CM_HISTORY_MAX_LEN;i++) {
				config->Files.last_project[i]="";
				if (menu_data->file_project_history[i])
					menu_recent->Remove(menu_data->file_project_history[i]);
			}
			list_ctrl->DeleteAllItems();
		}
	} else {
		wxMenu *menu_recent = menu_data->GetMenu(mxID_FILE_SOURCE_RECENT);
		if (mxMD_YES==mxMessageDialog(this,LANG(RECENT_CONFIRM_CLEAR_FILE_HISTORY,"Desea eliminar la lista de archivos recientes?"), LANG(GENERAL_CONFIRM,"Confirmacion"), mxMD_YES_NO|mxMD_QUESTION).ShowModal()) {
			for (unsigned int i=0;i<CM_HISTORY_MAX_LEN;i++) {
				config->Files.last_source[i]="";
				if (menu_data->file_source_history[i])
					menu_recent->Remove(menu_data->file_source_history[i]);
			}
			list_ctrl->DeleteAllItems();
		}
	}
}

int mxOpenRecentDialog::GetListSelection ( ) {
	long item=-1;
	return list_ctrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}

void mxOpenRecentDialog::SetListSelection (int i) {
	list_ctrl->SetItemState(i,wxLIST_STATE_SELECTED,wxLIST_STATE_SELECTED);	
}

void mxOpenRecentDialog::OnResize (wxSizeEvent & event) {
	event.Skip(); ResizeColums();
}

void mxOpenRecentDialog::OnListDClick (wxListEvent & event) {
	wxCommandEvent e; OnGotoButton(e);
}

void mxOpenRecentDialog::ResizeColums ( ) {
	Layout();
	list_ctrl->SetColumnWidth(1,show_mod_date->GetValue()?wxLIST_AUTOSIZE:0);
	list_ctrl->SetColumnWidth(1,list_ctrl->GetColumnWidth(1)+5);
	list_ctrl->SetColumnWidth(0,list_ctrl->GetClientSize().GetWidth()-list_ctrl->GetColumnWidth(1)-wxSystemSettings::GetMetric(wxSYS_VSCROLL_X)-1);
}

void mxOpenRecentDialog::OnShowDates (wxCommandEvent & evt) {
	evt.Skip();
	UpdateList();
}

