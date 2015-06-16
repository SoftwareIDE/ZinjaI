#include "mxGotoFunctionDialog.h"
#include "Parser.h"
#include "parserData.h"
#include "mxMainWindow.h"
#include "mxSource.h"
#include "mxGotoFileDialog.h"
#include "MenusAndToolsConfig.h"

	
/**
* @param direct_goto   if empty shows the regular dialog, else it searchs for the text, and if theres only one result automatically goes to it, if there are more than one shows the dialog, but trying to preselect and option in the current_file, wich is given in this argument
**/
mxGotoFunctionDialog::mxGotoFunctionDialog(wxString text, wxWindow* parent, wxString direct_goto) 
	: mxGotoListDialog( parent,
		LANG(GOTOFUNCTION_CAPTION,"Ir a..."), 
		LANG(GOTOFUNCTION_ENTER_FUNC_NAME,"Ingrese el nombre de la funcion, clase o metodo:"),
		"Buscar archivo...")
{
	SetExtraButtonAccelerator(wxACCEL_CTRL|wxACCEL_SHIFT,'f');
	case_sensitive->SetValue(true);
	SetInputValue(text);
	if (direct_goto.Len()) {
		if (list_ctrl->GetCount()==1 && goto_button->IsEnabled()) {
			GotoNow();
#ifdef __WIN32__
			main_window->SetFocusToSourceAfterEvents();
#endif
			return;
		} else {
			HideInput();
			for(int i=0;i<m_results.GetSize();i++) {
				if (wxFileName(m_results[i].get_file()).GetName()==direct_goto) {
					list_ctrl->SetSelection(list_ctrl->FindString(m_results[i].get_label()));
					break;
				}
			}
		}
	}
	Show();
}

void mxGotoFunctionDialog::OnGoto(int pos, wxString key) {
	// find selected item (list is alphabetically sorted, so order does not match m_results)
	int i=0;
	while (i<m_results.GetSize() && m_results[i].get_label()!=key) i++;
	if (i==m_results.GetSize()) return; // no debería pasar, pero a veces pasa!!!
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

int mxGotoFunctionDialog::OnSearch(wxString key, bool case_sensitive) {
	if (!key.Len()) return -1;
	int n = FillResults(key,!case_sensitive,strict_compare);
	for(int i=0;i<n;i++) list_ctrl->Append(m_results[i].get_label());
	if (n) {
		return 0; 
	} else {
		list_ctrl->Append(LANG(GOTOFUNCTION_NO_RESULTS,"<<no se encontraron coincidencias>>"));
		return -1;
	}
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


void mxGotoFunctionDialog::OnExtraButton() {
	Hide();
	new mxGotoFileDialog(text_ctrl->GetValue(),main_window);
	Close();
}

