#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include "mxGotoFileDialog.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "ProjectManager.h"
#include "mxMainWindow.h"
#include "ids.h"
#include "Language.h"
#include "mxSource.h"

BEGIN_EVENT_TABLE(mxGotoFileDialog, wxDialog)
	
	EVT_BUTTON(wxID_OK,mxGotoFileDialog::OnGotoButton)
	EVT_BUTTON(wxID_CANCEL,mxGotoFileDialog::OnCancelButton)
	EVT_CLOSE(mxGotoFileDialog::OnClose)
	EVT_TEXT(wxID_ANY,mxGotoFileDialog::OnTextChange)
	EVT_TIMER(mxID_TIMER_INPUT, mxGotoFileDialog::OnTimerInput)
	EVT_CHAR_HOOK(mxGotoFileDialog::OnCharHook)
	EVT_LISTBOX_DCLICK(wxID_ANY,mxGotoFileDialog::OnGotoButton)
	EVT_CHECKBOX(wxID_ANY,mxGotoFileDialog::OnCaseCheck)
END_EVENT_TABLE()
	
	
mxGotoFileDialog::mxGotoFileDialog(wxString text, wxWindow* parent, int _goto_line) : wxDialog( parent, wxID_ANY, LANG(GOTOFILE_CAPTION,"Ir a..."), wxDefaultPosition, wxDefaultSize, wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER ) {
	goto_line=_goto_line;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar")); 
	goto_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_GOTO_BUTTON,"  &Ir  "));
	goto_button->SetMinSize(wxSize(goto_button->GetSize().GetWidth()<80?80:goto_button->GetSize().GetWidth(),goto_button->GetSize().GetHeight()));
	goto_button->SetDefault(); 
	
	text_ctrl = new wxTextCtrl(this,wxID_ANY,_T(""));
	list_ctrl = new wxListBox(this,wxID_ANY,wxDefaultPosition, wxSize(450,300),0,NULL,wxLB_SINGLE|wxLB_SORT);
	case_sensitive = new wxCheckBox(this,wxID_ANY,LANG(GOTOFILE_CASE_SENSITIVE,"&Distinguir mayusculas y minusculas"));
	
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(goto_button,sizers->BA5);
	
	mySizer->Add(new wxStaticText(this,wxID_ANY,LANG(GOTOFILE_ENTER_FILENAME,"Ingrese parte del nombre del archivo que desea abrir:")),sizers->BLRT5_Exp0);
	mySizer->Add(text_ctrl,sizers->BA5_Exp0);
	mySizer->Add(list_ctrl,sizers->BA5_Exp0);
	mySizer->Add(case_sensitive,sizers->BA5_Exp0);
	mySizer->Add(bottomSizer,sizers->BA5_Right);
	SetSizerAndFit(mySizer);
	
	text_ctrl->SetSelection(-1,-1);
	text_ctrl->SetFocus();
	
	timer = new wxTimer(GetEventHandler(),mxID_TIMER_INPUT);
	
	if (text.Len()) {
		text_ctrl->SetValue(text);
		wxTimerEvent evt;
		OnTimerInput(evt);
	} else 
		goto_button->Enable(false);
	text_ctrl->SetSelection(0,-1);
	Show();
}

mxGotoFileDialog::~mxGotoFileDialog() {
	
}

void mxGotoFileDialog::OnGotoButton(wxCommandEvent &event) {
	if (timer->IsRunning()) {
		timer->Stop();
		wxTimerEvent evt;
		OnTimerInput(evt);
	}
	wxString key = list_ctrl->GetString(list_ctrl->GetSelection());
	if (key.Len()) {
		if (project) {
			Close();
			mxSource *src=main_window->OpenFile(DIR_PLUS_FILE(project->GetPath(),key),false);
			if (goto_line!=-1 && src && src!=EXTERNAL_SOURCE) src->MarkError(goto_line); /// for errors with incomplete or relative path from external building tools
		} else {
			mxSource *source;
			for (unsigned int i=0;i<main_window->notebook_sources->GetPageCount();i++) {
				source = (mxSource*)(main_window->notebook_sources->GetPage(i));
				if (source->sin_titulo) {
					if (source->page_text==key) {
						Close();
						main_window->notebook_sources->SetSelection(i);
						main_window->notebook_sources->GetPage(i)->SetFocus();
						break;
					}
				} else {
					if (source->source_filename.GetFullName()==key) {
						Close();
						main_window->notebook_sources->SetSelection(i);
						main_window->notebook_sources->GetPage(i)->SetFocus();
						break;
					}
				}
			}
		}
	}
}


void mxGotoFileDialog::OnCancelButton(wxCommandEvent &event) {
	Close();
}

void mxGotoFileDialog::OnClose(wxCloseEvent &event) {
	if (timer->IsRunning())
		timer->Stop();
	Destroy();
}

void mxGotoFileDialog::OnTextChange(wxCommandEvent &event) {
	timer->Start(500,true);
}

void mxGotoFileDialog::OnTimerInput(wxTimerEvent &event) {
	bool ignore_case = !case_sensitive->IsChecked();
	list_ctrl->Freeze();
	list_ctrl->Clear();
	wxString key = text_ctrl->GetValue();
	if (ignore_case) key.MakeUpper();
	if (project) {
		
		wxString path = project->GetPath();
		wxArrayString array;
		project->GetFileList(array,'*',true);
		for (unsigned int i=0; i<array.GetCount();i++) {
//			wxFileName fn(array[i]);
//			fn.Normalize();
//			wxString file(fn.GetFullPath());
//			if (file.StartsWith(path)) {
//				fn.MakeRelativeTo(path);
//				file=fn.GetFullPath();
//			}
			if ((ignore_case?array[i].Upper():array[i]).Find(key)!=wxNOT_FOUND)
				list_ctrl->Append(array[i]);
		}
	} else {
		mxSource *source;
		for (unsigned int i=0; i<main_window->notebook_sources->GetPageCount();i++) {
			source = (mxSource*)(main_window->notebook_sources->GetPage(i));
			wxString fullname = source->sin_titulo?source->page_text:source->source_filename.GetFullPath();
			if ((ignore_case?fullname.Upper():fullname).Find(key)!=wxNOT_FOUND)
					list_ctrl->Append(source->page_text);
		}
	}
	list_ctrl->Thaw();
	if (list_ctrl->GetCount()) {
		goto_button->Enable(true);
		list_ctrl->SetSelection(0);
	} else {
		list_ctrl->Append(LANG(GOTOFILE_NO_RESULTS,"<<no se encontraron coincidencias>>"));
		goto_button->Enable(false);
	}
}

void mxGotoFileDialog::OnCharHook (wxKeyEvent &event) {
	if (!list_ctrl->GetCount())  {
		event.Skip();
	} else if ( event.GetKeyCode() == WXK_UP ) {
		int sel = list_ctrl->GetSelection();
		sel--;
		if (sel==-1)
			sel=list_ctrl->GetCount()-1;
		list_ctrl->SetSelection(sel);
		list_ctrl->SetFirstItem(sel);
	} else if (event.GetKeyCode()==WXK_DOWN) {
		int sel = list_ctrl->GetSelection();
		sel++;
		if (sel==int(list_ctrl->GetCount()))
			sel=0;
		list_ctrl->SetSelection(sel);
		list_ctrl->SetFirstItem(sel);
	} else event.Skip();
}

void mxGotoFileDialog::OnCaseCheck(wxCommandEvent &event) {
	timer->Stop();
	wxTimerEvent evt;
	OnTimerInput(evt);
	text_ctrl->SetFocus();
}
