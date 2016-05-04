#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/textfile.h>
#include <wx/filename.h>
using namespace std;
#include <set>
#include "mxValgrindOuput.h"
#include "ids.h"
#include "Language.h"
#include "mxMessageDialog.h"
#include "mxMainWindow.h"
#include "ProjectManager.h"
#include "mxSource.h"
#include "mxCompiler.h"
#include "mxArt.h"

BEGIN_EVENT_TABLE(mxValgrindOuput,wxTreeCtrl)
	EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY,mxValgrindOuput::OnPopup)
	EVT_MENU(mxID_VALGRIND_OPEN_OUTPUT_FILE, mxValgrindOuput::OnOpen)
	EVT_MENU(mxID_VALGRIND_RELOAD_TREE, mxValgrindOuput::OnReload)
	EVT_MENU(mxID_VALGRIND_DELETE_FROM_TREE, mxValgrindOuput::OnDelete)
	EVT_TREE_ITEM_ACTIVATED(wxID_ANY, mxValgrindOuput::OnSelect)
	EVT_KEY_DOWN(mxValgrindOuput::OnKey)
END_EVENT_TABLE()

mxValgrindOuput::mxValgrindOuput(wxWindow *parent, mxVOmode mode, wxString afilename)
	: wxTreeCtrl(parent, wxID_ANY, wxPoint(0,0), wxSize(160,100), wxTR_DEFAULT_STYLE | wxNO_BORDER | wxTR_HIDE_ROOT) 
{
	SetMode(mode,afilename);
}

bool mxValgrindOuput::LoadOutput() {
	if (mode==mxVO_VALGRIND) return LoadOutputValgrind();
	else if (mode==mxVO_CPPCHECK) return LoadOutputCppCheck();
	else if (mode==mxVO_DOXYGEN) return LoadOutputDoxygen();
	return false;
}

bool mxValgrindOuput::LoadOutputCppCheck() {
	
	set<wxString> all;
		
	DeleteAllItems();
	root = AddRoot(_T("Salida"), -1);
	
	wxTreeItemId error=AppendItem(root,"Error (0)");
	wxTreeItemId warning=AppendItem(root,"Warning (0)");
	wxTreeItemId performance=AppendItem(root,"Performance (0)");
	wxTreeItemId information=AppendItem(root,"Information (0)");
	wxTreeItemId style=AppendItem(root,"Style (0)");
	wxTreeItemId other=AppendItem(root,"Other (0)");
	int cerr=0,cperf=0,cwar=0,csty=0,coth=0,cinf=0;
		
	wxTextFile fil(filename);
	if (!fil.Exists()) {
		mxMessageDialog(this,LANG(VALGRIND_OUTPUT_MISSING,"No se encontro el archivo de salida.\n"
														  "Para saber como generarlo consulte la ayuda."))
			.Title(LANG(GENERAL_ERROR,"Error")).IconError().Run();
		return false;
	}
	fil.Open();
	wxArrayString last_files; // para compatibilidad hacia atras, guarda el historial unificado y despues lo divide
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		if (all.find(str)!=all.end()) continue;
		all.insert(str);
		wxString tag=str.AfterFirst(']').AfterFirst('(').BeforeFirst(',');
		wxString s_str=str.BeforeFirst(']')+"]"+str.AfterFirst(']').BeforeFirst('(')+"("+str.AfterFirst(']').AfterFirst('(').AfterFirst(',');
		if (tag=="error") { AppendItem(error,s_str); cerr++; }
		else if (tag=="performance") { AppendItem(performance,s_str); cperf++; }
		else if (tag=="warning") { AppendItem(warning,s_str); cwar++; }
		else if (tag=="information") { AppendItem(information,s_str); cinf++; }
		else if (tag=="style") { AppendItem(style,s_str); csty++; }
		else { AppendItem(other,str); coth++; }
	}
	
	SetItemText(error,wxString("Error (")<<cerr<<")");
	SetItemText(performance,wxString("Performance (")<<cperf<<")");
	SetItemText(warning,wxString("Warning (")<<cwar<<")");
	SetItemText(style,wxString("Style (")<<csty<<")");
	SetItemText(information,wxString("Information(")<<cinf<<")");
	SetItemText(other,wxString("Other (")<<coth<<")");
	
//	Expand(sel=root);
	return true;
}	

bool mxValgrindOuput::LoadOutputValgrind() {
	DeleteAllItems();
	root = AddRoot(_T("Salida"), -1);
	is_last=false;
	if (!wxFileName(filename).FileExists()) {
		mxMessageDialog(this,LANG(VALGRIND_OUTPUT_MISSING,"No se encontro el archivo de salida.\n"
								                          "Para saber como generarlo consulte la ayuda."))
			.Title(LANG(GENERAL_ERROR,"Error")).IconError().Run();
		return false;
	}
	wxFFileInputStream finput(filename);
	wxTextInputStream input(finput);
	wxString line;
	while (finput.CanRead()) {
		line=input.ReadLine();
		int i=0, l=line.Len();
		while (i<l && ( line[i]==' ' || line[i]=='\t') ) i++;
		while (i<l && line[i]!=' ' && line[i]!='\t') i++;
		i++;
		line=line.Mid(i);
		i=0; l=line.Len();
		while (i<l && ( line[i]==' ' || line[i]=='\t') ) i++;
		if (i<l) {
			if (is_last) {
				AppendItem(last,line);
			} else {
				last = AppendItem(root,line);
				is_last=true;
			}
		} else 
			is_last=false;
	}
//	Expand(sel=root);
	return true;
}

bool mxValgrindOuput::LoadOutputDoxygen() {
	DeleteAllItems();
	root = AddRoot(_T("Salida"), 0);
	is_last=false;
	if (!wxFileName(filename).FileExists()) {
		mxMessageDialog(this,LANG(VALGRIND_OUTPUT_MISSING,"No se encontro el archivo de salida.\n"
								                          "Para saber como generarlo consulte la ayuda."))
			.Title(LANG(GENERAL_ERROR,"Error")).IconError().Run();
		return false;
	}
	wxFFileInputStream finput(filename);
	wxTextInputStream input(finput);
	wxString line;
	int count=0;
	while (finput.CanRead()) {
		line=input.ReadLine();
		if (!line.Len()) { is_last=false; continue; }
		int i=0, l=line.Len();
		while (i<l && ( line[i]==' ' || line[i]=='\t') ) i++;
		line=line.Mid(i);
		if (i && is_last) {
			AppendItem(last,line);
		} else {
			last = AppendItem(root,line);
			is_last=true;
		}
		count++;
	}
//	Expand(sel=root);
	return count;
}

void mxValgrindOuput::OnOpen(wxCommandEvent  &evt) {
	if (config->Init.show_welcome) main_window->ShowWelcome(false);
	wxString name="<results>";
	switch (mode) { 
	case mxVO_VALGRIND: name="<results_valgrind>"; break;
	case mxVO_CPPCHECK: name="<results_cppcheck>"; break;
	case mxVO_DOXYGEN: name="<results_doxygen>"; break;
	default:;
	}
	mxSource* source = new mxSource(main_window->notebook_sources, main_window->AvoidDuplicatePageText(name));
	source->SetStyle(false); source->LoadFile(filename);
	main_window->notebook_sources->AddPage(source, name ,true, *bitmaps->files.other);
	if (!project) source->treeId = main_window->AddToProjectTreeSimple(name,FT_OTHER);
	source->SetModify(false);
	source->SetReadOnlyMode(ROM_SPECIAL);
	source->SetFocus();
}

void mxValgrindOuput::OnReload(wxCommandEvent  &evt) {
	LoadOutput();
}

void mxValgrindOuput::OnDelete(wxCommandEvent &evt) {
	if (sel!=root && sel.IsOk()) {
		wxTreeItemId next=GetNextSibling(sel);
		if (!next.IsOk()) next=GetPrevSibling(sel);
		if (next.IsOk()) SelectItem(next);
		Delete(sel);
	}
}


void mxValgrindOuput::OnPopup(wxTreeEvent &evt) {
	sel = evt.GetItem();
	
	wxMenu menu("");
	menu.Append(mxID_VALGRIND_RELOAD_TREE, LANG(VALGRIND_RELOAD_TREE,"Recargar"));
	menu.Append(mxID_VALGRIND_OPEN_OUTPUT_FILE, LANG(VALGRIND_OPEN_OUTPUT_FILE,"Abrir salida completa"));
	menu.AppendSeparator();
	menu.Append(mxID_VALGRIND_DELETE_FROM_TREE, LANG(VALGRIND_DELETE_FROM_TREE,"Eliminar Items"));
	PopupMenu(&menu,evt.GetPoint());
	
}

void mxValgrindOuput::OnSelect(wxTreeEvent &evt) {
	wxString text=GetItemText(evt.GetItem());
	if (mode==mxVO_VALGRIND) {
		if (text.Last()==')') {
			text = text.AfterLast('(');
			text = text.Mid(0,text.Len()-1);
			int i=text.Len()-1;
			while (i>0 && text[i]>='0'&&text[i]<='9') i--;
			if (i<1 || text[i]!=':') return;
			long line;
			text.Mid(i+1).ToLong(&line);
			text = text.Mid(0,i);
			if (project) {
				project_file_item *fi = project->FindFromName(text);
				if (fi) {
					mxSource *source = main_window->OpenFile(DIR_PLUS_FILE(project->path,fi->name),false);
					if (source && source!=EXTERNAL_SOURCE) source->MarkError(line-1);
					return;
				}
			}
			for (int i=0,j=main_window->notebook_sources->GetPageCount();i<j;i++) {
				mxSource *src = ((mxSource*)(main_window->notebook_sources->GetPage(i)));
				if (src && ((!src->sin_titulo && src->source_filename.GetFullName()==text) || (src->temp_filename.GetFullName()==text && src==compiler->last_compiled)) ) {
					main_window->notebook_sources->SetSelection(i);
					src->MarkError(line-1);
					main_window->SetFocusToSourceAfterEvents();
					return;
				}
			}
		}
	} else if (mode==mxVO_CPPCHECK) {
		if (text.Len()&&text[0]=='[') {
			wxString file=text.AfterFirst('[').BeforeFirst(']');
			long line;
			file.AfterLast(':').ToLong(&line);
			file=file.BeforeLast(':');
			if (!file.Len()) return;
			mxSource *src=main_window->IsOpen(file);
			if (!src) src=main_window->OpenFile(project?DIR_PLUS_FILE(project->path,file):file);
			if (src) { src->MarkError(line-1); main_window->SetFocusToSourceAfterEvents(); }
		}
	} else if (mode==mxVO_DOXYGEN) {
		if ( text.Len()>1 && ( (text[0]=='/'&&text.Find(':')!=wxNOT_FOUND) || (text[1]==':'&&text.Mid(2).Find(':')!=wxNOT_FOUND) ) ) {
			wxString file; long line=-1;
			if (text[0]=='/') {
				file = text.BeforeFirst(':'); 
				text.AfterFirst(':').BeforeFirst(':').ToLong(&line);
			} else {
				file = text.BeforeFirst(':')+":"+text.AfterFirst(':').BeforeFirst(':'); 
				text.AfterFirst(':').AfterFirst(':').BeforeFirst(':').ToLong(&line);
			}
			if (line<0 || !file.Len()) return;
			mxSource *src=main_window->IsOpen(file);
			if (!src) src=main_window->OpenFile(project?DIR_PLUS_FILE(project->path,file):file);
			if (src) { src->MarkError(line-1); main_window->SetFocusToSourceAfterEvents(); }
		}
	}
}

void mxValgrindOuput::OnKey(wxKeyEvent &evt) {
	if (evt.GetKeyCode()==WXK_DELETE) {
		sel=GetSelection();
		wxCommandEvent evt2;
		OnDelete(evt2);
	} else evt.Skip();
}

void mxValgrindOuput::SetMode (mxVOmode mode, wxString afilename) {
	filename=afilename;
	this->mode=mode;
}

	
