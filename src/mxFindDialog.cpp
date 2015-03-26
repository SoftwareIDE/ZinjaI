#include <wx/sizer.h>
#include <wx/combobox.h>
#include <wx/arrstr.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textfile.h>
#include "mxFindDialog.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include "mxSource.h"
#include "mxMainWindow.h"
#include "mxMessageDialog.h"
#include "ids.h"
#include "ProjectManager.h"
#include "mxHelpWindow.h"
#include "mxSizers.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxFindDialog, wxDialog)
	EVT_BUTTON(mxID_HELP_BUTTON,mxFindDialog::OnHelpButton)
	EVT_BUTTON(mxID_FIND_FIND_NEXT,mxFindDialog::OnFindNextButton)
	EVT_BUTTON(mxID_FIND_FIND_PREV,mxFindDialog::OnFindPrevButton)
	EVT_BUTTON(mxID_FIND_REPLACE,mxFindDialog::OnReplaceButton)
	EVT_BUTTON(mxID_FIND_REPLACE_ALL,mxFindDialog::OnReplaceAllButton)
	EVT_BUTTON(mxID_FIND_CANCEL, mxFindDialog::OnCancel)
	EVT_COMBOBOX(mxID_FIND_SCOPE, mxFindDialog::OnComboScope)
	EVT_CLOSE(mxFindDialog::OnClose)
END_EVENT_TABLE()

mxFindDialog::mxFindDialog(wxWindow* parent, wxWindowID id, const wxPoint& pos , const wxSize& size , long style) : wxDialog(parent, id, LANG(FIND_CAPTION,"Buscar"), pos, size, style) {
	
	wxBoxSizer *mySizer= new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *optSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *butSizer = new wxBoxSizer(wxVERTICAL);
	
	optSizer->Add(new wxStaticText(this, wxID_ANY, LANG(FIND_SEARCH_FOR,"Texto a buscar:"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5);
	combo_find = new wxComboBox(this, wxID_ANY);
	optSizer->Add(combo_find,sizers->BLB5_Exp0);
	
	optSizer->Add(replace_static = new wxStaticText(this, wxID_ANY, LANG(REPLACE_WITH,"Reemplazar por:"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5);
	combo_replace = new wxComboBox(this, wxID_ANY);
	optSizer->Add(combo_replace,sizers->BLB5_Exp0);
	
	check_word = mxUT::AddCheckBox(optSizer,this,LANG(FIND_WHOLE_WORD,"Solo palabras completas"),false);
	check_start = mxUT::AddCheckBox(optSizer,this,LANG(FIND_BEGINNING_OF_WORD,"Solo al comienzo de la palabra"),false);
	check_case = mxUT::AddCheckBox(optSizer,this,LANG(FIND_MATCH_CASE,"Distinguir mayusculas y minusculas"),false);
	check_nocomments = mxUT::AddCheckBox(optSizer,this,LANG(FIND_IGNORE_COMMENTS,"Ignorar comentarios"),false);
	check_regexp = mxUT::AddCheckBox(optSizer,this,LANG(FIND_USE_REGULAR_EXPRESSIONS,"Es una expresion regular"),false);
	wxArrayString scopes;
	scopes.Add(LANG(FIND_SELECTION,"Seleccion"));
	scopes.Add(LANG(FIND_CURRENT_FILE,"Archivo actual"));
	scopes.Add(LANG(FIND_ALL_OPENED_FILES,"Todos los Archivos Abiertos"));
	scopes.Add(LANG(FIND_ALL_PROJECT_SOURCES,"Todos los Fuentes del Proyecto"));
	scopes.Add(LANG(FIND_ALL_PROJECT_HEADERS,"Todas las Cabeceras del Proyecto"));
	scopes.Add(LANG(FIND_ALL_PROJECT_OTHERS,"Todos los Otros Archivos del Proyecto"));
	scopes.Add(LANG(FIND_ALL_PROJECT_FILES,"Todos los Archivos del Proyecto"));
	combo_scope = mxUT::AddComboBox(optSizer,this,LANG(FIND_FIND_SCOPE,"Buscar en"),scopes,1,mxID_FIND_SCOPE);
	check_close = mxUT::AddCheckBox(optSizer,this,LANG(FIND_CLOSE_AFTER_FIND,"Cerrar este dialogo después de encontrar"),true);
	
	replace_button = new mxBitmapButton (this, mxID_FIND_REPLACE, bitmaps->buttons.replace, LANG(FIND_REPLACE,"Reemplazar"));
	replace_all_button = new mxBitmapButton (this, mxID_FIND_REPLACE_ALL, bitmaps->buttons.replace, LANG(FIND_REPLACE_ALL,"Reemplazar Todo"));
	next_button = new mxBitmapButton (this, mxID_FIND_FIND_NEXT, bitmaps->buttons.find, LANG(FIND_FIND_NEXT,"Buscar Siguiente"));
	wxButton *prev_button = new mxBitmapButton (this, mxID_FIND_FIND_PREV, bitmaps->buttons.find, LANG(FIND_FIND_PREVIOUS,"Buscar Anterior"));
	wxButton *cancel_button = new mxBitmapButton (this, mxID_FIND_CANCEL, bitmaps->buttons.cancel, LANG(FIND_CANCEL,"Cancelar"));
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	
	butSizer->Add(replace_button,sizers->BA5_Exp0);
	butSizer->Add(replace_all_button,sizers->BA5_Exp0);
	butSizer->Add(next_button,sizers->BA5_Exp0);
	butSizer->Add(prev_button,sizers->BA5_Exp0);
	butSizer->Add(cancel_button,sizers->BA5_Exp0);
	butSizer->AddStretchSpacer();
	butSizer->Add(help_button,sizers->BA5_Right);
	
	mySizer->Add(optSizer,sizers->BA5_Exp1);
	mySizer->Add(butSizer,sizers->BA5_Exp0);
	
	SetSizerAndFit(mySizer);

	combo_find->Append(wxString());
	combo_replace->Append(wxString());
	
	SetEscapeId(mxID_FIND_CANCEL);
	
}

mxFindDialog::~mxFindDialog() {
	
}

void mxFindDialog::ShowFind(mxSource *source) {
	replace_mode=false;
	check_close->SetValue(true);
	if (source && !check_regexp->GetValue()) {
		int i=source->GetSelectionStart();
		int f=source->GetSelectionEnd();
//		if (i==f) {
//			int s=source->WordStartPosition(i,true);
//			int e=source->WordEndPosition(i,true);
//			if (s!=e)
//				combo_find->SetValue(source->GetTextRange(s,e));
//		} else if (source->LineFromPosition(i)==source->LineFromPosition(f)) {
		if (i!=f && source->LineFromPosition(i)==source->LineFromPosition(f)) {
			combo_find->SetSelection(combo_find->GetCount()-1);
			if (i<f)
				combo_find->SetValue(source->GetTextRange(i,f));
			else
				combo_find->SetValue(source->GetTextRange(f,i));
		} else {
			if (combo_scope->GetSelection()==0) combo_scope->SetSelection(1);
			if (combo_find->GetCount()>1)
				combo_find->SetSelection(combo_find->GetCount()-2);
		}
	}
	replace_static->Hide();
	combo_replace->Hide();
	replace_all_button->Hide();
	replace_button->Hide();
	next_button->SetDefault();
	SetTitle(LANG(FIND_FIND,"Buscar"));
	GetSizer()->SetSizeHints(this);
	Fit();
	combo_find->SetFocus();
	Show();
	Raise();
}

void mxFindDialog::ShowReplace(mxSource *source) {
  replace_mode=true;
  check_close->SetValue(false);
	if (combo_scope->GetSelection()>1) {
		combo_scope->SetSelection(1);
		wxCommandEvent cmd; OnComboScope(cmd);
	}
	if (combo_replace->GetCount()>1)
		combo_replace->SetSelection(combo_find->GetCount()-2);
	if (source && !check_regexp->GetValue()) {
		int i=source->GetSelectionStart();
		int f=source->GetSelectionEnd();
//		if (i==f) {
//			int s=source->WordStartPosition(i,true);
//			int e=source->WordEndPosition(i,true);
//			if (s!=e)
//				combo_find->SetValue(source->GetTextRange(s,e));
//		} else if (source->LineFromPosition(i)==source->LineFromPosition(f)) {
		if (i!=f && source->LineFromPosition(i)==source->LineFromPosition(f)) {
			combo_find->SetSelection(combo_find->GetCount()-1);
			if (i<f)
				combo_find->SetValue(source->GetTextRange(i,f));
			else
				combo_find->SetValue(source->GetTextRange(f,i));
		} else {
			if (combo_find->GetCount()>1)
				combo_find->SetSelection(combo_find->GetCount()-2);
		}
	}
	replace_static->Show();
	combo_replace->Show();
	replace_all_button->Show();
	replace_button->Show();
	replace_button->SetDefault();
	SetTitle(LANG(FIND_REPLACE,"Reemplazar"));
	GetSizer()->SetSizeHints(this);
	Fit();
	combo_find->SetFocus();
	Show();
	Raise();
}

bool mxFindDialog::FindPrev() {
	
	if (main_window->notebook_sources->GetPageCount()!=0) {
		mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
		int f,t,p;
		if (only_selection) { // seleccion
			f = source->GetSelectionStart();
			p = t = source->GetSelectionEnd();
			if (t>f) { // acomodar el sentido de la seleccion
				t=f;
				f=p;
				p=t;
			}
			if (f==t || (f-t==int(last_search.Len()) && source->GetTextRange(t,f).CmpNoCase(last_search)==0) ) { // si no hay seleccion tomar el fuente
				t=0;
				p=f;
				f=source->GetLength();
			}
		} else { // fuente
			p=source->GetSelectionStart();
			t=0;
			f=source->GetLength();
		}
		
		// buscar
		source->SetSearchFlags(last_flags);
		source->SetTargetStart(p);
		source->SetTargetEnd(t);
		int ret = source->SearchInTarget(last_search);
		// si hay que ignorar los comentarios, y esta en comentario, buscar otra
		while (check_nocomments->GetValue() && ret!=wxSTC_INVALID_POSITION && source->IsComment(ret)) {
			source->SetTargetStart(ret);
			source->SetTargetEnd(t);
			ret = source->SearchInTarget(last_search);
		}
		if (ret==wxSTC_INVALID_POSITION && p!=f) {
			source->SetTargetStart(f);
			if (last_flags&wxSTC_FIND_REGEXP)
				source->SetTargetEnd(t);
			else
				source->SetTargetEnd(p-last_search.Len());
			ret=source->SearchInTarget(last_search);
			// si hay que ignorar los comentarios, y esta en comentario, buscar otra
			while (check_nocomments->GetValue() && ret!=wxSTC_INVALID_POSITION && source->IsComment(ret)) {
				source->SetTargetStart(ret);
				source->SetTargetEnd(t);
				ret = source->SearchInTarget(last_search);
			}
		}
		if (ret>=0) {
			source->EnsureVisibleEnforcePolicy(source->LineFromPosition(ret));
			source->SetSelection(source->GetTargetStart(),source->GetTargetEnd());
			return true;
		} else { 
			return false;
		}
	}
	return false;
}

bool mxFindDialog::FindNext() {
	if (main_window->notebook_sources->GetPageCount()!=0) {
		mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
		int f,t,p;
		if (only_selection) { // seleccion
			p = f = source->GetSelectionStart();
			t = source->GetSelectionEnd();
			if (t<f) { // acomodar el sentido de la seleccion
				f=t;
				t=p;
				p=f;
			}
			if (f==t || (t-f==int(last_search.Len()) && source->GetTextRange(f,t).CmpNoCase(last_search)==0) ) { // si no hay seleccion tomar el fuente
				f=0;
				p=t;
				t=source->GetLength();
			}
		} else { // fuente
			p=source->GetSelectionEnd();
			f=0;
			t=source->GetLength();
		}
		
		// buscar
		source->SetSearchFlags(last_flags);
		source->SetTargetStart(p);
		source->SetTargetEnd(t);
		int ret = source->SearchInTarget(last_search);
		// si hay que ignorar los comentarios, y esta en comentario, buscar otra
		while (check_nocomments->GetValue() && ret!=wxSTC_INVALID_POSITION && source->IsComment(ret)) {
			source->SetTargetStart(ret+last_search.Len());
			source->SetTargetEnd(t);
			ret = source->SearchInTarget(last_search);
		}
		if (ret==wxSTC_INVALID_POSITION && p!=f) {
			source->SetTargetStart(f);
			if (last_flags&wxSTC_FIND_REGEXP)
				source->SetTargetEnd(t);
			else
				source->SetTargetEnd(p+last_search.Len());
			ret = source->SearchInTarget(last_search);
			// si hay que ignorar los comentarios, y esta en comentario, buscar otra
			while (check_nocomments->GetValue() && ret!=wxSTC_INVALID_POSITION && source->IsComment(ret)) {
				source->SetTargetStart(ret+last_search.Len());
				source->SetTargetEnd(p+last_search.Len());
				ret = source->SearchInTarget(last_search);
			}
		}
		if (ret!=wxSTC_INVALID_POSITION) {
			source->EnsureVisibleEnforcePolicy(source->LineFromPosition(ret));
			source->SetSelection(source->GetTargetStart(),source->GetTargetEnd());
			return true;
		} else { 
			return false;
		}
	}
	return false;
}

void mxFindDialog::OnFindNextButton(wxCommandEvent &event) {
	num_results=0;
	if (combo_find->GetValue().Len()==0) {
		combo_find->SetFocus();
		return;
	}

	if (combo_scope->GetSelection()>2) {
		if (!project) {
			mxMessageDialog(this,LANG(FIND_NO_PROJECT,"No hay ningun proyecto abierto actualmente."),LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal();
			return;
		}
	} else {
		if (!main_window->notebook_sources->GetPageCount()) {
			return;
		}
	}

	last_search = combo_find->GetValue();
	if (last_search!=combo_find->GetString(combo_find->GetCount()-1)) {
		combo_find->SetString(combo_find->GetCount()-1,last_search);
		combo_find->Append(wxString());
	}
	int scope = combo_scope->GetSelection();
	only_selection = (scope==0);
	last_flags = 
		(check_case->GetValue()?wxSTC_FIND_MATCHCASE:0) |
		(check_word->GetValue()?wxSTC_FIND_WHOLEWORD:0) |
		(check_start->GetValue()?wxSTC_FIND_WORDSTART:0) |
		(check_regexp->GetValue()?wxSTC_FIND_REGEXP:0) ;

	if (scope<2) {
		if (FindNext()) {
			if (check_close->GetValue()) {
				MyHide(); /*main_window->Raise();*/
			} else {
				Raise(); combo_find->SetFocus();
			}
		} else {
			mxMessageDialog(main_window,LANG1(FIND_NOT_FOUND,"La cadena \"<{1}>\" no se encontro.",last_search), LANG(FIND_FIND_CAPTION,"Buscar"), mxMD_OK|mxMD_INFO).ShowModal();
			Raise();
		}
	} else if (scope==2) {
		if (FindInSources() && check_close->GetValue()) {
			MyHide();
		} else {
			Raise();
			combo_find->SetFocus();
		}
	} else {
		switch (scope) {
		case 3:
			if (FindInProject(FT_SOURCE) && check_close->GetValue()) {
				MyHide(); /*main_window->Raise();*/
			} else {
				Raise();
				combo_find->SetFocus();
			}
			break;
		case 4:
			if (FindInProject(FT_HEADER) && check_close->GetValue()) {
				MyHide(); /*main_window->Raise();*/
			} else {
				Raise();
				combo_find->SetFocus();
			}
			break;
		case 5:
			if (FindInProject(FT_OTHER) && check_close->GetValue()) {
				MyHide(); /*main_window->Raise();*/
			} else {
				Raise();
				combo_find->SetFocus();
			}
			break;
		default:
			if (FindInProject(FT_NULL) && check_close->GetValue()) {
				MyHide(); /*main_window->Raise();*/
			} else {
				Raise();
				combo_find->SetFocus();
			}
			break;
		}
	}
	
}

void mxFindDialog::OnFindPrevButton(wxCommandEvent &event) {
	num_results=0;
	if (combo_find->GetValue().Len()==0) {
		combo_find->SetFocus();
		return;
	}
	
	if (combo_scope->GetSelection()>2) {
		if (!project) {
			mxMessageDialog(this,LANG(FIND_NO_PROJECT,"No hay ningun proyecto abierto actualmente."),LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal();
			return;
		}
	} else {
		if (!main_window->notebook_sources->GetPageCount()) {
			mxMessageDialog(this,LANG(FIND_NO_FILE,"No hay ningun archivo abierto actualmente."),LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal();
			return;
		}
	}

	last_search = combo_find->GetValue();
	if (last_search!=combo_find->GetString(combo_find->GetCount()-1)) {
		combo_find->SetString(combo_find->GetCount()-1,last_search);
		combo_find->Append(wxString());
	}
	
	int scope = combo_scope->GetSelection();
	only_selection = (scope==0);
	last_flags = 
		(check_case->GetValue()?wxSTC_FIND_MATCHCASE:0) |
		(check_word->GetValue()?wxSTC_FIND_WHOLEWORD:0) |
		(check_start->GetValue()?wxSTC_FIND_WORDSTART:0) |
		(check_regexp->GetValue()?wxSTC_FIND_REGEXP:0) ;

	if (scope<2) {
		if (FindPrev()) {
			if (check_close->GetValue()) {
				MyHide(); /*main_window->Raise();*/
			} else {
				Raise();
				combo_find->SetFocus();
			}
		} else
			mxMessageDialog(main_window,LANG1(FIND_NOT_FOUND,"La cadena \"<{1}>\" no se encontro.",last_search), LANG(FIND_FIND_CAPTION,"Buscar"), mxMD_OK|mxMD_INFO).ShowModal();
	} else if (scope==2) {
		if (FindInSources() && check_close->GetValue()) {
			MyHide(); /*main_window->Raise();*/
		} else {
			Raise();
			combo_find->SetFocus();
		}
	} else {
		switch (scope) {
		case 3:
			if (FindInProject(FT_SOURCE) && check_close->GetValue()) {
				MyHide(); /*main_window->Raise();*/
			} else {
				Raise();
				combo_find->SetFocus();
			}
			break;
		case 4:
			if (FindInProject(FT_HEADER) && check_close->GetValue()) {
				MyHide(); /*main_window->Raise();*/
			} else {
				Raise();
				combo_find->SetFocus();
			}
			break;
		case 5:
			if (FindInProject(FT_OTHER) && check_close->GetValue()) {
				MyHide(); /*main_window->Raise();*/
			} else {
				Raise();
				combo_find->SetFocus();
			}
			break;
		default:
			if (FindInProject(FT_NULL) && check_close->GetValue()) {
				MyHide(); /*main_window->Raise();*/
			} else {
				Raise();
				combo_find->SetFocus();
			}
			break;
		}
	}
	
}

void mxFindDialog::OnReplaceButton(wxCommandEvent &event) {

	if (combo_find->GetValue().Len()==0) {
		combo_find->SetFocus();
		return;
	}

	if (combo_scope->GetSelection()>2) {
		mxMessageDialog(this,LANG1(FIND_CANT_REPLACE_IN,"No es posible realizar reemplazos en \"<{1}>\",\nesta opcion aun no esta disponible.",combo_scope->GetValue()),LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal();
		return;
	}

	if (!main_window->notebook_sources->GetPageCount()) {
		mxMessageDialog(this,LANG(FIND_NO_FILE,"No hay ningun archivo abierto actualmente."),LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal();
		return;
	}
	
	last_search = combo_find->GetValue();
	if (last_search!=combo_find->GetString(combo_find->GetCount()-1)) {
		combo_find->SetString(combo_find->GetCount()-1,last_search);
		combo_find->Append(wxString());
	}
	last_replace = combo_replace->GetValue();
	if (last_replace.Len() && last_replace!=combo_replace->GetString(combo_replace->GetCount()-1)) {
		combo_replace->SetString(combo_replace->GetCount()-1,last_replace);
		combo_replace->Append(wxString());
	}
	
	int scope = combo_scope->GetSelection();
	only_selection = (scope==0);

	last_flags = 
		(check_case->GetValue()?wxSTC_FIND_MATCHCASE:0) |
		(check_word->GetValue()?wxSTC_FIND_WHOLEWORD:0) |
		(check_start->GetValue()?wxSTC_FIND_WORDSTART:0) |
		(check_regexp->GetValue()?wxSTC_FIND_REGEXP:0) ;
	
	mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
	int f,t;
	source->SetTargetStart(f=source->GetSelectionStart());
	source->SetTargetEnd(t=source->GetSelectionEnd());
	if (source->SearchInTarget(last_search)!=wxSTC_INVALID_POSITION && ( (source->GetTargetStart()==f && source->GetTargetEnd()==t) || (source->GetTargetStart()==t && source->GetTargetEnd()==f) ) ) {
		if (last_flags&wxSTC_FIND_REGEXP)
			source->ReplaceTargetRE(last_replace);
		else
			source->ReplaceTarget(last_replace);
		source->SetSelection(source->GetTargetEnd(),source->GetTargetEnd());
	}
	FindNext();
	
}

void mxFindDialog::OnReplaceAllButton(wxCommandEvent &event) {

	if (combo_find->GetValue().Len()==0) {
		combo_find->SetFocus();
		return;
	}

	if (combo_scope->GetSelection()>2) {
		mxMessageDialog(this,LANG1(FIND_CANT_REPLACE_IN,"No es posible realizar reemplazos en \"<{1}>\",\nesta opcion aun no esta disponible.",combo_scope->GetValue()),LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal();
		return;
	}

	if (!main_window->notebook_sources->GetPageCount()) {
		mxMessageDialog(this,LANG(FIND_NO_FILE,"No hay ningun archivo abierto actualmente."),LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal();
		return;
	}
	
	last_search = combo_find->GetValue();
	if (last_search!=combo_find->GetString(combo_find->GetCount()-1)) {
		combo_find->SetString(combo_find->GetCount()-1,last_search);
		combo_find->Append(wxString());
	}
	last_replace = combo_replace->GetValue();
	if (last_replace.Len() && last_replace!=combo_replace->GetString(combo_replace->GetCount()-1)) {
		combo_replace->SetString(combo_replace->GetCount()-1,last_replace);
		combo_replace->Append(wxString());
	}

	int scope = combo_scope->GetSelection();
	only_selection = (scope==0);

	last_flags = 
		(check_case->GetValue()?wxSTC_FIND_MATCHCASE:0) |
		(check_word->GetValue()?wxSTC_FIND_WHOLEWORD:0) |
		(check_start->GetValue()?wxSTC_FIND_WORDSTART:0) |
		(check_regexp->GetValue()?wxSTC_FIND_REGEXP:0) ;
	
	mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
	
	int f,t;
	if (only_selection) {
		f=source->GetSelectionStart();
		t=source->GetSelectionEnd();
		if (t<f) {
			int a=f;
			f=t;
			t=a;
		}
		source->SetSelection(f,t);
	} else {
		f=0;
		t=source->GetLength();
	}
	
	int c=0; // contador de reemplazos
	
	// primera busqueda
	source->SetSearchFlags(last_flags);
	source->SetTargetStart(f);
	source->SetTargetEnd(t);
	int ret = source->SearchInTarget(last_search);
	mxSource::UndoActionGuard undo_action(source,false);
	while (ret!=wxSTC_INVALID_POSITION) {
		undo_action.Begin();
		int l = source->GetTargetEnd()-source->GetTargetStart(); // para saber si cambio el largo de la seleccion despues de reemplazar
		if (last_flags&wxSTC_FIND_REGEXP) // el remplazo propiamente dicho
			source->ReplaceTargetRE(last_replace);
		else
			source->ReplaceTarget(last_replace);
		t+=(source->GetTargetEnd()-source->GetTargetStart())-l; // actualizar el largo del bloque donde se busca
		c++; // contar
		// buscar otra vez
		source->SetTargetStart(source->GetTargetEnd());
		source->SetTargetEnd(t);
		ret = source->SearchInTarget(last_search);
	}
	undo_action.End();
	if (only_selection) source->SetSelection(f,t);

	if (c==0)
		mxMessageDialog(this,LANG(FIND_NO_REPLACE_DONE,"No se realizo ningun reemplazo."), LANG(FIND_REPLACE_CAPTION,"Reemplazar"), mxMD_OK|mxMD_INFO).ShowModal();
	else if (c==1)
		mxMessageDialog(this,LANG(FIND_ONE_REPLACE_DONE,"Se realizo un remplazo."), LANG(FIND_REPLACE_CAPTION,"Reemplazar"), mxMD_OK|mxMD_INFO).ShowModal();
	else
		mxMessageDialog(this,LANG1(FIND_MANY_REPLACES,"Se realizaron <{1}> reemplazos.",wxString()<<c), LANG(FIND_REPLACE_CAPTION,"Reemplazar"), mxMD_OK|mxMD_INFO).ShowModal();
	
	Raise();
}

bool mxFindDialog::FindInSources() {
	wxString res; int count=0;
	for (unsigned int j=0;j<main_window->notebook_sources->GetPageCount();j++) {
		count+=FindInSource((mxSource*)(main_window->notebook_sources->GetPage(j)),res);
	}
	return MultifindAux(count,res);
}
	
	
bool mxFindDialog::MultifindAux(int count, const wxString &res) {
	if (count==0) {
		mxMessageDialog(this,LANG(FIND_IN_FILES_NOT_FOUND,"No se encontraron coincidencias"),LANG(FIND_IN_FILES_CAPTION,"Buscar en archivos"),mxMD_WARNING|mxMD_OK).ShowModal();
		return false;
	} else {
		wxString html("<HTML><HEAD><TITLE>");
		html<<LANG(FIND_FIND_IN_FILES,"Buscar en archivos")<<"</TITLE></HEAD><BODY><B>";
		html<<LANG2(FIND_IN_FILES_RESULT,"Resultados para la busqueda \"<I><{1}></I>\" (<{2}> resultados):",mxUT::ToHtml(last_search),wxString()<<count);
		html<<"</B><BR><TABLE>"<<res<<"</TABLE><BR><BR></BODY></HTML>";
		main_window->ShowInQuickHelpPanel(html);
		return true;
	}
}

/**
* @brief Auxiliar function for generaten the html (result for searchs in multiple files)
*
* This function will handle the results limit, if the count exceeds 500 only the first
* 500 will be shown (trying to show a huge html will virtually freeze the application).
*
* @param fname    the full path to de filename, to be included in href in the link
* @param line     the base 0 line number for the coincidence
* @param pos      the base 0 position for the coincidence within that line
* @param len      the len of the coincidence
* @param falias   the friendly short filename to display in the text
* @param the_line the content of the line, to display trimmed and with the coincidence in bold
**/
wxString mxFindDialog::GetHtmlEntry(wxString fname, int line, int pos, int len, wxString falias, wxString the_line) {
	wxString res;
	if (++num_results==500) {
		return wxString("<TR><TD><B>...</B></TD>")<<LANG(FIND_TOO_MANY_RESULTS,"demasiados resultados, solo se muestran los primeros 500")<<"<TD></TD></TR>";
	} else if (num_results>500) return "";
	res<<"<TR><TD><A href=\"gotolinepos:"<<fname<<":"<<line<<":"<<pos<<":"<<len<<"\">"<<falias<<": "<<LANG(FIND_LINE,"linea")<<" "<<line+1<<"</A></TD>";
	res<<"<TD>"<<mxUT::ToHtml(the_line.Mid(0,pos).Trim(false))+"<B>"+mxUT::ToHtml(the_line.Mid(pos,len))+"</B>"+mxUT::ToHtml(the_line.Mid(pos+len).Trim(true))<<"</TD></TR>";
	return res;
}

int mxFindDialog::FindInSource(mxSource *source,wxString &res) {
	int count=0;
	wxString file_name = source->GetFullPath();
	wxString page_text = mxUT::ToHtml(source->page_text);
	source->SetSearchFlags(last_flags);
	int l = source->GetLength();
	source->SetTargetStart(0);
	source->SetTargetEnd(l);
	int i = source->SearchInTarget(last_search);
	while (i!=wxSTC_INVALID_POSITION) {
		count++;
		int line=source->LineFromPosition(i); i-=source->PositionFromLine(line);
		res<<GetHtmlEntry(file_name,line,i,source->GetTargetEnd()-source->GetTargetStart(),page_text,source->GetLine(line));
		source->SetTargetStart(source->GetTargetEnd());
		source->SetTargetEnd(l);
		i = source->SearchInTarget(last_search);
	}
	return count;
}

bool mxFindDialog::FindInProject(eFileType where) {
	last_flags = wxSTC_FIND_MATCHCASE;
	wxArrayString array;
	project->GetFileList(array,where);
	wxString res;
	int p,count=0;
	last_flags = check_case->GetValue()?wxSTC_FIND_MATCHCASE:0;
	wxString what=last_search;
	if (!check_case->GetValue()) what.MakeUpper();
	for (unsigned int i=0;i<array.GetCount();i++) {
		mxSource *src=main_window->IsOpen(array[i]);
		if (src) {
			count+=FindInSource(src,res);
		} else {
			wxTextFile fil(array[i]);
			if (fil.Exists()) {
				fil.Open();
				int l=0;
				for ( wxString str_orig, str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine()) {
					int ac=0; str_orig=str;
					if (!check_case->GetValue()) str.MakeUpper();
					while (wxNOT_FOUND!=(p=str.Find(what))) {
						count++;
						res<<GetHtmlEntry(array[i],l,ac+p,what.Len(),wxFileName(array[i]).GetFullName(),str_orig);
						int todel=p+what.Len(); ac+=todel; str.Remove(0,todel);
					}
					l++;
				}
				fil.Close();
			}
		}
	}
	return MultifindAux(count,res);
}

void mxFindDialog::OnComboScope(wxCommandEvent &event) {
	int scope = combo_scope->GetSelection();
	if (scope>2) {
//		check_case->Enable(false);
		check_regexp->Enable(false);
		check_start->Enable(false);
		check_word->Enable(false);
		check_nocomments->Enable(false);
		replace_button->Enable(false);
		replace_all_button->Enable(false);
	} else if (scope==2) {
//		check_case->Enable(true);
		check_regexp->Enable(true);
		check_start->Enable(true);
		check_word->Enable(true);
		check_nocomments->Enable(true);
		replace_button->Enable(false);
		replace_all_button->Enable(false);
	} else {
//		check_case->Enable(true);
		check_regexp->Enable(true);
		check_start->Enable(true);
		check_word->Enable(true);
		check_nocomments->Enable(true);
		replace_button->Enable(true);
		replace_all_button->Enable(true);
	}
}

void mxFindDialog::OnHelpButton(wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("search_dialog.html");
}

void mxFindDialog::OnCancel(wxCommandEvent &event) {
	MyHide(); /*main_window->Raise();*/
}

void mxFindDialog::OnClose(wxCloseEvent &event) {
	MyHide();
}

void mxFindDialog::MyHide() {
	Hide(); wxYield();
	if (config->Init.autohiding_panels) main_window->Raise();
	main_window->FocusToSource();
}
