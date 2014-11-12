#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include "mxGotoFunctionDialog.h"
#include "mxBitmapButton.h"
#include "Parser.h"
#include "parserData.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxSource.h"
#include "mxSizers.h"
#include "Language.h"
#include "mxGotoFileDialog.h"
#include "MenusAndToolsConfig.h"

BEGIN_EVENT_TABLE(mxGotoFunctionDialog, wxDialog)
	EVT_BUTTON(mxID_EDIT_GOTO_FILE,mxGotoFunctionDialog::OnGotoFileButton)
	EVT_BUTTON(wxID_OK,mxGotoFunctionDialog::OnGotoButton)
	EVT_BUTTON(wxID_CANCEL,mxGotoFunctionDialog::OnCancelButton)
	EVT_CLOSE(mxGotoFunctionDialog::OnClose)
	EVT_TEXT(wxID_ANY,mxGotoFunctionDialog::OnTextChange)
	EVT_TIMER(mxID_TIMER_INPUT, mxGotoFunctionDialog::OnTimerInput)
	EVT_CHAR_HOOK(mxGotoFunctionDialog::OnCharHook)
	EVT_LISTBOX_DCLICK(wxID_ANY,mxGotoFunctionDialog::OnGotoButton)
	EVT_CHECKBOX(wxID_ANY,mxGotoFunctionDialog::OnCaseCheck)
END_EVENT_TABLE()
	
/**
* @param direct_goto   if empty shows the regular dialog, else it searchs for the text, and if theres only one result automatically goes to it, if there are more than one shows the dialog, but trying to preselect and option in the current_file, wich is given in this argument
**/
mxGotoFunctionDialog::mxGotoFunctionDialog(wxString text, wxWindow* parent, wxString direct_goto) : wxDialog(parent, wxID_ANY, LANG(GOTOFUNCTION_CAPTION,"Ir a..."),wxDefaultPosition, wxDefaultSize, wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	strict_compare=direct_goto.Len();
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar")); 
	goto_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_GOTO_BUTTON,"  &Ir  "));
	goto_button->SetMinSize(wxSize(goto_button->GetSize().GetWidth()<80?80:goto_button->GetSize().GetWidth(),goto_button->GetSize().GetHeight()));
	goto_button->SetDefault(); 
	
	text_ctrl = new wxTextCtrl(this,wxID_ANY,"");
	if (strict_compare) text_ctrl->Hide();
	list_ctrl = new wxListBox(this,wxID_ANY,wxDefaultPosition, wxSize(450,300),0,NULL,wxLB_SINGLE|wxLB_SORT);
	case_sensitive = new wxCheckBox(this,wxID_ANY,LANG(GOTOFUNCTION_CASE_SENSITIVE,"&Distinguir mayusculas y minusculas"));
	case_sensitive->SetValue(true);
	if (strict_compare) case_sensitive->Hide();
	
	bottomSizer->Add(new wxButton(this,mxID_EDIT_GOTO_FILE,"Buscar archivo...",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT),sizers->Center);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(goto_button,sizers->BA5);
	wxAcceleratorEntry accel(wxACCEL_CTRL|wxACCEL_SHIFT,'f',mxID_EDIT_GOTO_FILE);
	SetAcceleratorTable(wxAcceleratorTable(1,&accel));
	
	if (!strict_compare) 
		mySizer->Add(new wxStaticText(this,wxID_ANY,LANG(GOTOFUNCTION_ENTER_FUNC_NAME,"Ingrese el nombre de la funcion, clase o metodo:")),sizers->BLRT5_Exp0);
	mySizer->Add(text_ctrl,sizers->BA5_Exp0);
	mySizer->Add(list_ctrl,sizers->BA5_Exp0);
	mySizer->Add(case_sensitive,sizers->BA5_Exp0);
	mySizer->Add(bottomSizer,sizers->BA5_Exp0);
	SetSizerAndFit(mySizer);
	
	text_ctrl->SetSelection(-1,-1);
	text_ctrl->SetFocus();
	
	if (!strict_compare) timer = new wxTimer(GetEventHandler(),mxID_TIMER_INPUT); else timer=NULL;
	
	if (text.Len()) {
		text_ctrl->SetValue(text);
		wxTimerEvent evt;
		OnTimerInput(evt);
	} else 
		goto_button->Enable(false);
	text_ctrl->SetSelection(0,-1);
	if (strict_compare && list_ctrl->GetCount()==1 && goto_button->IsEnabled()) {
		wxCommandEvent evt;
		OnGotoButton(evt);
#if defined(_WIN32) || defined(__WIN32__)
		main_window->SetFocusToSourceAfterEvents();
#endif
	} else {
		if (direct_goto.Len()) {
			list_ctrl->SetFocus();
			for(int i=0;i<m_results.GetSize();i++) { 
				if (wxFileName(m_results[i].get_file()).GetName()==direct_goto) {
					list_ctrl->SetSelection(list_ctrl->FindString(m_results[i].get_label()));
					break;
				}
			}
		}
		Show();
	}
}

mxGotoFunctionDialog::~mxGotoFunctionDialog() {
	
}

void mxGotoFunctionDialog::OnGotoButton(wxCommandEvent &event) {
	// update results if needed
	if (timer && timer->IsRunning()) {
		timer->Stop();
		wxTimerEvent evt;
		OnTimerInput(evt);
	}
	if (list_ctrl->IsEmpty()) return;
	// find selected item (list is alphabetically sorted, so order does not match m_results)
	wxString key = list_ctrl->GetString(list_ctrl->GetSelection());
	int i=0;
	while (i<m_results.GetSize() && m_results[i].get_label()!=key) i++;
	if (i==-1) return;
	gotoff_result &r=m_results[i];
	Close(); 
	// process results (simulate double click on parser tree)
	wxTreeItemId tree_item=r.get_item();
	parser->popup_file_def = r.get_file();
	parser->popup_line_def = r.get_line();
	parser->OnGotoDef(main_window->notebook_sources);
	// select the item in symbols tree and ensure definition/declaration visibility in its mxSource (at this point should be the current one)
	if (main_window->left_panels) {
		if (_menu_item(mxID_VIEW_LEFT_PANELS)->IsChecked()) {
			main_window->left_panels->SetSelection(1);
			main_window->symbols_tree.treeCtrl->EnsureVisible(tree_item);
			main_window->symbols_tree.treeCtrl->ScrollTo(tree_item);
			main_window->symbols_tree.treeCtrl->SelectItem(tree_item);
		}
	} else {
		if (_menu_item(mxID_VIEW_SYMBOLS_TREE)->IsChecked()) {
			main_window->symbols_tree.treeCtrl->EnsureVisible(tree_item);
			main_window->symbols_tree.treeCtrl->ScrollTo(tree_item);
			main_window->symbols_tree.treeCtrl->SelectItem(tree_item);
		}
	}
	mxSource *source=(mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
	source->EnsureVisibleEnforcePolicy(source->GetCurrentLine());
	if (!source->GetFoldExpanded(source->GetCurrentLine()))	source->ToggleFold(source->GetCurrentLine());
	source->SetFocus();
}


void mxGotoFunctionDialog::OnCancelButton(wxCommandEvent &event) {
	Close();
}

void mxGotoFunctionDialog::OnClose(wxCloseEvent &event) {
	if (timer && timer->IsRunning()) timer->Stop();
	Destroy();
}

void mxGotoFunctionDialog::OnTextChange(wxCommandEvent &event) {
	if (timer) timer->Start(500,true);
}

void mxGotoFunctionDialog::OnTimerInput(wxTimerEvent &event) {
	list_ctrl->Freeze(); list_ctrl->Clear();
	int n = FillResults(text_ctrl->GetValue(),!case_sensitive->IsChecked(),strict_compare);
	for(int i=0;i<n;i++) 
		list_ctrl->Append(m_results[i].get_label());
	list_ctrl->Thaw();
	if (n) {
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
	if (timer) timer->Stop();
	wxTimerEvent evt;
	OnTimerInput(evt);
	text_ctrl->SetFocus();
}

int mxGotoFunctionDialog::FillResults (wxString key, bool ignore_case, bool strict_compare) {
	m_results.Clear();
	if (ignore_case && !strict_compare) key.MakeUpper();
	// search in preprocessor functions
	pd_macro *aux_macro = parser->last_macro->next;
	while (aux_macro) {
		if (!aux_macro->file->hide_symbols) {
			if (aux_macro->props&PD_CONST_MACRO_FUNC) {
				if (strict_compare) {
					if (aux_macro->name==key) m_results.Add(aux_macro);
				} else if (ignore_case) {
					if (aux_macro->name.Upper().Contains(key)) m_results.Add(aux_macro);
				} else {
					if (aux_macro->name.Contains(key)) m_results.Add(aux_macro);
				}
			}
		}
		aux_macro = aux_macro->next;
	}

	// search in class names and their methods
	pd_class *aux_class = parser->last_class->next;
	while (aux_class) {
		if (!aux_class->file || !aux_class->file->hide_symbols) {
			// class name
			if (strict_compare) {
				if (aux_class->name==key) m_results.Add(aux_class);
			} else if (ignore_case) {
				if (aux_class->name.Upper().Contains(key)) m_results.Add(aux_class);
			} else {
				if (aux_class->name.Contains(key)) m_results.Add(aux_class);
			}
			// methods names
			pd_func *aux_func = aux_class->first_method;
			while (aux_func) {
				if ((aux_func->file_def && !aux_func->file_def->hide_symbols) || (aux_func->file_dec && !aux_func->file_dec->hide_symbols)) {
					if (strict_compare) {
						if (aux_func->name==key) m_results.Add(aux_func);
					} else if (aux_func->file_def) {
						if (ignore_case) {
							if (aux_func->name.Upper().Contains(key)) m_results.Add(aux_func);
						} else {
							if (aux_func->name.Contains(key)) m_results.Add(aux_func);
						}
					}
				}
				aux_func = aux_func->next;
			}
		}
		aux_class = aux_class->next;
	}
	// search in global functions and methos
	pd_func *aux_func = parser->last_function->next;
	while (aux_func) {
		if ((aux_func->file_def && !aux_func->file_def->hide_symbols) || (aux_func->file_dec && !aux_func->file_dec->hide_symbols)) {
			if (strict_compare) {
				if (aux_func->name==key) m_results.Add(aux_func);
			} else if (aux_func->file_def) {
				if (ignore_case) {
					if (aux_func->name.Upper().Contains(key)) m_results.Add(aux_func);
				} else {
					if (aux_func->name.Contains(key)) m_results.Add(aux_func);
				}
			}
		}
		aux_func = aux_func->next;
	}
	return m_results.GetSize();
}


void mxGotoFunctionDialog::OnGotoFileButton (wxCommandEvent & event) {
	Hide();
	new mxGotoFileDialog(text_ctrl->GetValue(),main_window);
	Close();
}

