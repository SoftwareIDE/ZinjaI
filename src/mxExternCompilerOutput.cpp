#include "mxExternCompilerOutput.h"
#include "mxMainWindow.h"
#include "ids.h"
#include "mxHidenPanel.h"

BEGIN_EVENT_TABLE(mxExternCompilerOutput,wxListBox)
	EVT_LISTBOX_DCLICK(wxID_ANY,mxExternCompilerOutput::OnDClick)
	EVT_MENU(mxID_EXTERN_COMPILER_OUTPUT,mxExternCompilerOutput::OnOnlyErrors)
	EVT_MENU(mxID_COMPILER_POPUP_FULL,mxExternCompilerOutput::OnViewFullOutput)
	EVT_RIGHT_DOWN(mxExternCompilerOutput::OnPopup)
END_EVENT_TABLE();

mxExternCompilerOutput::mxExternCompilerOutput(wxWindow *compiler_panel):wxListBox(compiler_panel,mxID_EXTERN_COMPILER_OUTPUT,wxDefaultPosition,wxDefaultSize) {
	showing_only_errors = false;
}

void mxExternCompilerOutput::AddLine (const wxString &pre, const wxString &message) {
	Append(pre+message);
	SetFirstItem(GetCount()-1);
}


void mxExternCompilerOutput::OnDClick(wxCommandEvent & evt) {
	wxString str=GetStringSelection().AfterFirst(' ');
	if (str.Len()) main_window->OnSelectErrorCommon(str,true);
}

void mxExternCompilerOutput::OnPopup (wxMouseEvent & evt) {
	mxHidenPanel::ignore_autohide=true;
	wxMenu menu("");
	menu.Append(mxID_COMPILER_POPUP_FULL, "Ver salida completa");
	menu.AppendCheckItem(mxID_EXTERN_COMPILER_OUTPUT, "Mostrar solo errores")->Check(showing_only_errors);
	PopupMenu(&menu);
	mxHidenPanel::ignore_autohide=false;
}

void mxExternCompilerOutput::OnOnlyErrors (wxCommandEvent & evt) {
	if (showing_only_errors) {
		showing_only_errors = false;
		wxListBox::Clear();
		for(unsigned int i=0;i<lines.GetCount();i++) {
			Append(lines[i]);
		}
	} else {
		PopulateArray();
		for(unsigned int i=0;i<GetCount();i++) {
			while (i<GetCount() && !IsErrorLine(i))
				Delete(i);
		}
		showing_only_errors = true;
	}
}

void mxExternCompilerOutput::OnViewFullOutput (wxCommandEvent & evt) {
	PopulateArray();
	main_window->ShowSpecilaUnnamedSource("<ultima_compilacion>",lines);
}


void mxExternCompilerOutput::PopulateArray ( ) {
	if(lines.GetCount()) return;
	for(unsigned int i=0;i<GetCount();i++) 
		lines.Add(GetString(i));
}

void mxExternCompilerOutput::Clear ( ) {
	showing_only_errors = false;
	lines.Clear();
	wxListBox::Clear();
}

void mxExternCompilerOutput::OnErrorNext ( ) {
	int p = GetSelection(); if (p<0||p>=GetCount()) p=-1; p++;
	while (p<int(GetCount()) && !IsErrorLine(p)) p++;
	if (p==int(GetCount())) return;
	main_window->ShowCompilerTreePanel();
	Select(p); wxCommandEvent evt; OnDClick(evt);
}

void mxExternCompilerOutput::OnErrorPrev ( ) {
	int p = GetSelection(); if (p<0||p>=GetCount()) p=GetCount(); p--;
	while (p>=0 && !IsErrorLine(p)) p--;
	if (p<0) return;
	main_window->ShowCompilerTreePanel();
	Select(p); wxCommandEvent evt; OnDClick(evt);
}

