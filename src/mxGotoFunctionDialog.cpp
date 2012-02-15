#include "mxGotoFunctionDialog.h"
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include "mxBitmapButton.h"
#include <wx/stattext.h>
#include "Parser.h"
#include "parserData.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxSource.h"
#include "mxSizers.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxGotoFunctionDialog, wxDialog)
	EVT_BUTTON(wxID_OK,mxGotoFunctionDialog::OnGotoButton)
	EVT_BUTTON(wxID_CANCEL,mxGotoFunctionDialog::OnCancelButton)
	EVT_CLOSE(mxGotoFunctionDialog::OnClose)
	EVT_TEXT(wxID_ANY,mxGotoFunctionDialog::OnTextChange)
	EVT_TIMER(mxID_TIMER_INPUT, mxGotoFunctionDialog::OnTimerInput)
	EVT_CHAR_HOOK(mxGotoFunctionDialog::OnCharHook)
	EVT_LISTBOX_DCLICK(wxID_ANY,mxGotoFunctionDialog::OnGotoButton)
	EVT_CHECKBOX(wxID_ANY,mxGotoFunctionDialog::OnCaseCheck)
END_EVENT_TABLE()
	

mxGotoFunctionDialog::mxGotoFunctionDialog(wxString text, wxWindow* parent, bool direct_goto) : wxDialog(parent, wxID_ANY, LANG(GOTOFUNCTION_CAPTION,"Ir a..."),wxDefaultPosition, wxDefaultSize, wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	strict_compare=direct_goto;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar")); 
	goto_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_GOTO_BUTTON,"  &Ir  "));
	goto_button->SetMinSize(wxSize(goto_button->GetSize().GetWidth()<80?80:goto_button->GetSize().GetWidth(),goto_button->GetSize().GetHeight()));
	goto_button->SetDefault(); 
	
	text_ctrl = new wxTextCtrl(this,wxID_ANY,_T(""));
	if (direct_goto) text_ctrl->Hide();
	list_ctrl = new wxListBox(this,wxID_ANY,wxDefaultPosition, wxSize(450,300),0,NULL,wxLB_SINGLE|wxLB_SORT);
	case_sensitive = new wxCheckBox(this,wxID_ANY,LANG(GOTOFUNCTION_CASE_SENSITIVE,"&Distinguir mayusculas y minusculas"));
	case_sensitive->SetValue(true);
	if (direct_goto) case_sensitive->Hide();
	
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(goto_button,sizers->BA5);
	
	if (!direct_goto) 
		mySizer->Add(new wxStaticText(this,wxID_ANY,LANG(GOTOFUNCTION_ENTER_FUNC_NAME,"Ingrese el nombre de la funcion, clase o metodo:")),sizers->BLRT5_Exp0);
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
	if (direct_goto && list_ctrl->GetCount()==1 && goto_button->IsEnabled()) {
		wxCommandEvent evt;
		OnGotoButton(evt);
#if defined(_WIN32) || defined(__WIN32__)
		main_window->focus_timer->Start(333,true);
#endif
	} else
		Show();
	if (direct_goto) list_ctrl->SetFocus();
}

mxGotoFunctionDialog::~mxGotoFunctionDialog() {
	
}

void mxGotoFunctionDialog::OnGotoButton(wxCommandEvent &event) {
	if (timer->IsRunning()) {
		timer->Stop();
		wxTimerEvent evt;
		OnTimerInput(evt);
	}
	wxString key = list_ctrl->GetString(list_ctrl->GetSelection());
	if (key.Mid(0,8)==_T("#define ")) {
		key=key.Mid(8);
		pd_macro *aux_macro = parser->last_macro->next;
		while (aux_macro) {
			if (aux_macro->has_args && aux_macro->proto==key) {
				Close();
				parser->popup_file_def = aux_macro->file->name;
				parser->popup_line_def = aux_macro->line;
				parser->OnGotoDef(main_window->notebook_sources);
				if (main_window->symbols_tree.menuItem->IsChecked()) {
					main_window->symbols_tree.treeCtrl->EnsureVisible(aux_macro->item);
					main_window->symbols_tree.treeCtrl->ScrollTo(aux_macro->item);
					main_window->symbols_tree.treeCtrl->SelectItem(aux_macro->item);
				}
				mxSource *source=(mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
				source->EnsureVisibleEnforcePolicy(source->GetCurrentLine());
				if (!source->GetFoldExpanded(source->GetCurrentLine()))	source->ToggleFold(source->GetCurrentLine());
				source->SetFocus();
				return;
			}
			aux_macro = aux_macro->next;
		}
	} else if (key.Mid(0,6)==_T("class ")) {
		key=key.Mid(6);
		pd_class *aux_class = parser->last_class->next;
		while (aux_class) {
			if (aux_class->name==key) {
				Close();
				parser->popup_file_def = aux_class->file->name;
				parser->popup_line_def = aux_class->line;
				parser->OnGotoDef(main_window->notebook_sources);
				if (main_window->symbols_tree.menuItem->IsChecked()) {
					main_window->symbols_tree.treeCtrl->EnsureVisible(aux_class->item);
					main_window->symbols_tree.treeCtrl->ScrollTo(aux_class->item);
					main_window->symbols_tree.treeCtrl->SelectItem(aux_class->item);
				}
				mxSource *source=(mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
				source->EnsureVisibleEnforcePolicy(source->GetCurrentLine());
				if (!source->GetFoldExpanded(source->GetCurrentLine()))	source->ToggleFold(source->GetCurrentLine());
				source->SetFocus();
				return;
			}
			aux_class = aux_class->next;
		}
	} else {
		pd_func *aux_func = parser->last_function->next;
		while (aux_func) {
			if (aux_func->file_def && aux_func->proto==key) {
				Close();
				parser->popup_file_def = aux_func->file_def->name;
				parser->popup_line_def = aux_func->line_def;
				parser->OnGotoDef(main_window->notebook_sources);
				if (main_window->left_panels) {
					if (main_window->menu.view_left_panels->IsChecked()) {
						main_window->left_panels->SetSelection(1);
						main_window->symbols_tree.treeCtrl->EnsureVisible(aux_func->item);
						main_window->symbols_tree.treeCtrl->ScrollTo(aux_func->item);
						main_window->symbols_tree.treeCtrl->SelectItem(aux_func->item);
					}
				} else {
					if (main_window->symbols_tree.menuItem->IsChecked()) {
						main_window->symbols_tree.treeCtrl->EnsureVisible(aux_func->item);
						main_window->symbols_tree.treeCtrl->ScrollTo(aux_func->item);
						main_window->symbols_tree.treeCtrl->SelectItem(aux_func->item);
					}
				}
				mxSource *source=(mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
				source->EnsureVisibleEnforcePolicy(source->GetCurrentLine());
				if (!source->GetFoldExpanded(source->GetCurrentLine()))	source->ToggleFold(source->GetCurrentLine());
				source->SetFocus();
				return;
			}
			aux_func = aux_func->next;
		}

		pd_class *aux_class = parser->last_class->next;
		while (aux_class) {
			pd_func *aux_func = aux_class->first_method;
			while (aux_func) {
				if (aux_func->file_def && aux_func->full_proto==key) {
					Close();
					parser->popup_file_def = aux_func->file_def->name;
					parser->popup_line_def = aux_func->line_def;
					parser->OnGotoDef(main_window->notebook_sources);
					if (main_window->symbols_tree.menuItem->IsChecked()) {
						main_window->symbols_tree.treeCtrl->EnsureVisible(aux_func->item);
						main_window->symbols_tree.treeCtrl->ScrollTo(aux_func->item);
						main_window->symbols_tree.treeCtrl->SelectItem(aux_func->item);
						main_window->symbols_tree.treeCtrl->SelectItem(aux_func->item);
					}
					mxSource *source=(mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
					source->EnsureVisibleEnforcePolicy(source->GetCurrentLine());
					if (!source->GetFoldExpanded(source->GetCurrentLine()))	source->ToggleFold(source->GetCurrentLine());
					source->SetFocus();
					return;
				}
				aux_func = aux_func->next;
			}
			aux_class = aux_class->next;
		}
	}
}


void mxGotoFunctionDialog::OnCancelButton(wxCommandEvent &event) {
	Close();
}

void mxGotoFunctionDialog::OnClose(wxCloseEvent &event) {
	if (timer->IsRunning())
		timer->Stop();
	Destroy();
}

void mxGotoFunctionDialog::OnTextChange(wxCommandEvent &event) {
	timer->Start(500,true);
}

void mxGotoFunctionDialog::OnTimerInput(wxTimerEvent &event) {
	bool ignore_case=!case_sensitive->IsChecked();
	list_ctrl->Freeze();
	list_ctrl->Clear();
	wxString key = text_ctrl->GetValue();
	if (ignore_case && !strict_compare) key.MakeUpper();
	
	pd_macro *aux_macro = parser->last_macro->next;
	while (aux_macro) {
		if (aux_macro->has_args) {
			if (strict_compare) {
				if (aux_macro->name==key)
					list_ctrl->Append(wxString(_T("#define "))<<aux_macro->proto);
			} else {
				if ((ignore_case?aux_macro->name.Upper():aux_macro->name).Contains(key))
					list_ctrl->Append(wxString(_T("#define "))<<aux_macro->proto);
			}
		}
		aux_macro = aux_macro->next;
	}
	
	pd_class *aux_class = parser->last_class->next;
	while (aux_class) {
		if (strict_compare && aux_class->name==key)
			list_ctrl->Append(wxString(_T("class "))<<aux_class->name);
		else if (!strict_compare && (ignore_case?aux_class->name.Upper():aux_class->name).Contains(key))
			list_ctrl->Append(wxString(_T("class "))<<aux_class->name);
		pd_func *aux_func = aux_class->first_method;
		while (aux_func) {
			if (strict_compare && aux_func->name==key)
				list_ctrl->Append(aux_func->full_proto);
			else if (!strict_compare && aux_func->file_def && (ignore_case?aux_func->name.Upper():aux_func->name).Contains(key))
				list_ctrl->Append(aux_func->full_proto);
			aux_func = aux_func->next;
		}
		aux_class = aux_class->next;
	}
	
	pd_func *aux_func = parser->last_function->next;
	while (aux_func) {
		if (strict_compare && aux_func->name==key)
			list_ctrl->Append(aux_func->proto);
		else if (!strict_compare && aux_func->file_def && (ignore_case?aux_func->name.Upper():aux_func->name).Contains(key))
			list_ctrl->Append(aux_func->proto);
		aux_func = aux_func->next;
	}
	list_ctrl->Thaw();
	if (list_ctrl->GetCount()) {
		goto_button->Enable(true);
		list_ctrl->SetSelection(0);
	} else {
		list_ctrl->Append(LANG(GOTOFUNCTION_NO_RESULTS,"<<no se encontraron coincidencias>>"));
		goto_button->Enable(false);
	}
}

void mxGotoFunctionDialog::OnCharHook (wxKeyEvent &event) {
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

void mxGotoFunctionDialog::OnCaseCheck(wxCommandEvent &event) {
	timer->Stop();
	wxTimerEvent evt;
	OnTimerInput(evt);
	text_ctrl->SetFocus();
}
