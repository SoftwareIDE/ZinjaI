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

BEGIN_EVENT_TABLE(mxOpenRecentDialog, wxDialog)
	EVT_BUTTON(wxID_OK,mxOpenRecentDialog::OnGotoButton)
	EVT_BUTTON(wxID_FIND,mxOpenRecentDialog::OnClear)
	EVT_BUTTON(wxID_REPLACE,mxOpenRecentDialog::OnDelButton)
	EVT_BUTTON(wxID_CANCEL,mxOpenRecentDialog::OnCancelButton)
	EVT_CLOSE(mxOpenRecentDialog::OnClose)
	EVT_TEXT(wxID_ANY,mxOpenRecentDialog::OnTextChange)
	EVT_CHAR_HOOK(mxOpenRecentDialog::OnCharHook)
	EVT_LISTBOX_DCLICK(wxID_ANY,mxOpenRecentDialog::OnGotoButton)
END_EVENT_TABLE()
	
	
mxOpenRecentDialog::mxOpenRecentDialog(wxWindow* parent, bool aprj) : wxDialog(parent, wxID_ANY, LANG(RECENT_CAPTION,"Archivos Recientes..."), wxDefaultPosition, wxDefaultSize, wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	projects=aprj;
	
	wxString *array = projects?config->Files.last_project:config->Files.last_source;
	int n=0; unsigned int max_len=0;
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++) 
		if (array[i].Len()) {
			n++; 
			if (array[i].Len()>max_len)
				max_len=array[i].Len();
		} else break;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	clear_button = new mxBitmapButton (this, wxID_FIND, bitmaps->buttons.cancel, LANG(RECENT_CLEAN,"&Limpiar Lista")); 
	del_button = new mxBitmapButton (this, wxID_REPLACE, bitmaps->buttons.cancel, LANG(RECENT_DELETE,"&Quitar de la Lista")); 
	cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar")); 
	goto_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_GOTO_BUTTON,"  &Ir  "));
	goto_button->SetMinSize(wxSize(goto_button->GetSize().GetWidth()<80?80:goto_button->GetSize().GetWidth(),goto_button->GetSize().GetHeight()));
	goto_button->SetDefault(); 
	
	text_ctrl = new wxTextCtrl(this,wxID_ANY,_T(""));
	list_ctrl = new wxListBox(this,wxID_ANY,wxDefaultPosition, wxSize(450,300),0,NULL,wxLB_SINGLE|wxLB_SORT);
	
	bottomSizer->Add(clear_button,sizers->BA5);
	bottomSizer->Add(del_button,sizers->BA5);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(goto_button,sizers->BA5);
	
	mySizer->Add(new wxStaticText(this,wxID_ANY,LANG(RECENT_ENTER_FILENAME,"Ingrese parte del nombre del archivo que desea abrir:")),sizers->BLRT5_Exp0);
	mySizer->Add(text_ctrl,sizers->BA5_Exp0);
	mySizer->Add(list_ctrl,sizers->BA5_Exp0);
	mySizer->Add(new wxStaticText(this,wxID_ANY,wxString(' ',max_len*2)),sizers->BLRT5_Exp0);
	mySizer->Add(bottomSizer,sizers->BA5_Exp0);
	SetSizerAndFit(mySizer);
	
	text_ctrl->SetSelection(-1,-1);
	text_ctrl->SetFocus();
	
	UpdateList();
	CenterOnParent();
	Show();
	
}

void mxOpenRecentDialog::OnGotoButton(wxCommandEvent &event) {
	wxString key = list_ctrl->GetString(list_ctrl->GetSelection());
	Close();
	main_window->OpenFileFromGui(key);
}


void mxOpenRecentDialog::OnCancelButton(wxCommandEvent &event) {
	Close();
}

void mxOpenRecentDialog::OnDelButton(wxCommandEvent &event) {
	int sel = list_ctrl->GetSelection();
	if (sel<0||sel>=int(list_ctrl->GetCount())) return;
	wxString todel = list_ctrl->GetString(sel);
	list_ctrl->Delete(sel);

	wxString *array = projects?config->Files.last_project:config->Files.last_source;
	bool found=false;
	for (int i=0;i<CM_HISTORY_MAX_LEN-1;i++) {
		if (!found&&array[i]==todel) found=true;
		if (found) array[i]=array[i+1];
	}
	array[CM_HISTORY_MAX_LEN-1]="";
	
	if (array[0].Len()) main_window->UpdateInHistory(array[0]);
	
}

void mxOpenRecentDialog::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxOpenRecentDialog::OnTextChange(wxCommandEvent &event) {
	UpdateList();
}

void mxOpenRecentDialog::UpdateList() {
	list_ctrl->Freeze();
	list_ctrl->Clear();
	wxString key = text_ctrl->GetValue();
	key.MakeUpper();
	wxString *array = projects?config->Files.last_project:config->Files.last_source;
	for (unsigned int i=0;i<CM_HISTORY_MAX_LEN;i++) {
		if (array[i].Len()==0) break;
		wxFileName fn(array[i]);
		fn.Normalize();
		wxString file(fn.GetFullPath());
		if (file.Upper().Find(key)!=wxNOT_FOUND)
			list_ctrl->Append(file);
	}
	list_ctrl->Thaw();
	if (list_ctrl->GetCount()) {
		goto_button->Enable(true);
		list_ctrl->SetSelection(0);
	} else {
		list_ctrl->Append(LANG(RECENT_NO_RESULTS,"<<no se encontraron coincidencias>>"));
		goto_button->Enable(false);
	}
}

void mxOpenRecentDialog::OnCharHook (wxKeyEvent &event) {
	if (!list_ctrl->GetCount())  {
		event.Skip();
	} else if ( event.GetKeyCode() == WXK_DELETE && FindFocus()!=text_ctrl) {
		wxCommandEvent evt;
		OnDelButton(evt);
	} else if ( event.GetKeyCode() == WXK_UP ) {
		int sel = list_ctrl->GetSelection();
		sel--;
		if (sel==-1)
			sel=list_ctrl->GetCount()-1;
		list_ctrl->SetSelection(sel);
//		list_ctrl->SetFirstItem(sel); // para que estaba esto?
	} else if (event.GetKeyCode()==WXK_DOWN) {
		int sel = list_ctrl->GetSelection();
		sel++;
		if (sel==int(list_ctrl->GetCount()))
			sel=0;
		list_ctrl->SetSelection(sel);
//		list_ctrl->SetFirstItem(sel); // para que estaba esto?
	} else event.Skip();
}

void mxOpenRecentDialog::OnClear(wxCommandEvent &event) {
	if (projects) {
		if (mxMD_YES==mxMessageDialog(this,LANG(RECENT_CONFIRM_CLEAR_PROJECT_HISTORY,"Desea eliminar la lista de proyectos recientes?"), LANG(GENERAL_CONFIRM,"Confirmacion"), mxMD_YES_NO|mxMD_WARNING).ShowModal()) {
			for (unsigned int i=0;i<CM_HISTORY_MAX_LEN;i++) {
				config->Files.last_project[i]=_T("");
				if (main_window->menu.file_project_history[i])
					main_window->menu.file_project_recent->Remove(main_window->menu.file_project_history[i]);
			}
			list_ctrl->Clear();
		}
	} else {
		if (mxMD_YES==mxMessageDialog(this,LANG(RECENT_CONFIRM_CLEAR_FILE_HISTORY,"Desea eliminar la lista de archivos recientes?"), LANG(GENERAL_CONFIRM,"Confirmacion"), mxMD_YES_NO|mxMD_QUESTION).ShowModal()) {
			for (unsigned int i=0;i<CM_HISTORY_MAX_LEN;i++) {
				config->Files.last_source[i]=_T("");
				if (main_window->menu.file_source_history[i])
					main_window->menu.file_source_recent->Remove(main_window->menu.file_source_history[i]);
			}
			list_ctrl->Clear();
		}
	}
}
