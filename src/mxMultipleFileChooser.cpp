#include <wx/checkbox.h> // for wxCheckListBox
#include <wx/sizer.h>
#include <wx/dirdlg.h>
#include <wx/dir.h>
#include "ids.h"
#include "mxMultipleFileChooser.h"
#include "mxMainWindow.h"
#include "Language.h"
#include "mxUtils.h"
#include "ConfigManager.h"
#include "ProjectManager.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "mxMessageDialog.h"
#include "mxHidenPanel.h"
#include "Parser.h"
#include "mxOSD.h"

BEGIN_EVENT_TABLE(mxMultipleFileChooser, wxDialog)
	EVT_BUTTON(mxID_MULTIPLEFILE_ADD,mxMultipleFileChooser::OnButtonOk)
	EVT_BUTTON(mxID_MULTIPLEFILE_CANCEL,mxMultipleFileChooser::OnButtonCancel)
	EVT_MENU(mxID_MULTIPLEFILE_MARK_ALL,mxMultipleFileChooser::OnButtonMarkAll)
	EVT_MENU(mxID_MULTIPLEFILE_MARK_NONE,mxMultipleFileChooser::OnButtonMarkNone)
	EVT_MENU(mxID_MULTIPLEFILE_MARK_INVERT,mxMultipleFileChooser::OnButtonMarkInvert)
	EVT_BUTTON(mxID_MULTIPLEFILE_DIR,mxMultipleFileChooser::OnButtonDir)
	EVT_BUTTON(mxID_MULTIPLEFILE_FIND,mxMultipleFileChooser::OnButtonFind)
	EVT_CLOSE(mxMultipleFileChooser::OnClose)
END_EVENT_TABLE()

mxMultipleFileChooser::mxMultipleFileChooser(wxString apath, bool modal) : wxDialog(main_window, wxID_ANY, LANG(MULTIFILE_CAPTION,"Agregar Archivos al Proyecto"), wxDefaultPosition, wxDefaultSize ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	
	basedir = mxUT::AddDirCtrl(sizer,this,LANG(MULTIFILE_DIR,"1) Seleccione el directorio donde buscar los archivos"),apath.Len()?apath:project->last_dir,mxID_MULTIPLEFILE_DIR);
	subdirs = mxUT::AddCheckBox(sizer,this,LANG(MULTIFILE_SUBDIRS,"buscar tambien en subdirectorios"),true);
	filter  = mxUT::AddDirCtrl(sizer,this,LANG(MULTIFILE_FILTER,"2) Ingrese un filtro para los nombres de archivos"),"*",mxID_MULTIPLEFILE_FIND,LANG(MULTIFILE_FIND," Buscar "));
	
	mxUT::AddStaticText(sizer,this,LANG(MULTIFILE_LIST,"Seleccione los archivos a agregar"));
	list = new wxCheckListBox(this,wxID_ANY,wxDefaultPosition,wxSize(320,230));
	list->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( mxMultipleFileChooser::OnListRightClick ), nullptr, this );
	sizer->Add(list,sizers->BA5_Exp1);

	wxArrayString awhere;
	awhere.Add(LANG(MAINW_PT_SOURCES,"Fuentes"));
	awhere.Add(LANG(MAINW_PT_HEADERS,"Cabeceras"));
	awhere.Add(LANG(MAINW_PT_OTHERS,"Otros"));
	awhere.Add(LANG(MAINW_PT_AUTO,"<determinar por extension>"));
	cmb_where = mxUT::AddComboBox(sizer,this,LANG(MULTIFILE_WHERE,"Agregar en categor�a"),awhere,3);
	
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxButton *cancel_button = new mxBitmapButton (this,mxID_MULTIPLEFILE_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	SetEscapeId(mxID_MULTIPLEFILE_CANCEL);
	wxButton *ok_button = new mxBitmapButton (this,mxID_MULTIPLEFILE_ADD,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	ok_button->SetDefault(); 
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	
	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	
	sizer->Add(bottomSizer,sizers->Exp0);
	
	SetSizerAndFit(sizer);
	
	if (apath.Len()) {
		wxCommandEvent evt;
		OnButtonFind(evt);
	}
	
	if (modal)
		ShowModal();
	else
		Show();
	
	basedir->SetFocus();
}

void mxMultipleFileChooser::OnButtonOk(wxCommandEvent &event) {
	int n=list->GetCount(); if (n==0) return;
	int nn=0; for (int i=0;i<n;i++) if (list->IsChecked(i)) nn++;
	int x=mxMessageDialog(this,LANG1(MULTIFILE_CONFIRM_ADD,"�Desea agregar <{1}> archivos al proyecto?",wxString()<<nn),LANG(GENERAL_CONFIRM,"Confirmacion"),mxMD_YES_NO|mxMD_QUESTION).ShowModal();
	if (mxMD_YES!=x) return;
	mxOSD osd(main_window,LANG(OSD_ADDING_FILES,"Agregando archivos..."));
	int iw=cmb_where->GetSelection();
	eFileType where=FT_NULL;
	if (iw==0) where=FT_SOURCE;
	else if (iw==1) where=FT_HEADER;
	else if (iw==2) where=FT_OTHER;
	for (int i=0;i<n;i++) {
		wxString fname=DIR_PLUS_FILE(search_base,list->GetString(i));
		if (list->IsChecked(i) && !project->HasFile(fname)) {
			eFileType aux_where = where==FT_NULL?mxUT::GetFileType(fname,false):where;
			project->AddFile(aux_where,fname);
			if (aux_where==FT_SOURCE||aux_where==FT_HEADER) parser->ParseFile(fname);
		}
	}
	if (config->Init.autohiding_panels)
		main_window->autohide_handlers[ATH_PROJECT]->ForceShow(false);
	Close();
}

void mxMultipleFileChooser::OnButtonCancel(wxCommandEvent &event) {
	Close();
}

void mxMultipleFileChooser::OnButtonMarkAll(wxCommandEvent &event) {
	for (int i=0,n=list->GetCount();i<n;i++)
		list->Check(i,true);
}

void mxMultipleFileChooser::OnButtonMarkNone(wxCommandEvent &event) {
	for (int i=0,n=list->GetCount();i<n;i++)
		list->Check(i,false);
}

void mxMultipleFileChooser::OnButtonMarkInvert(wxCommandEvent &event) {
	for (int i=0,n=list->GetCount();i<n;i++)
		list->Check(i,!list->IsChecked(i));
}

void mxMultipleFileChooser::OnButtonDir(wxCommandEvent &event) {
	wxString text=basedir->GetValue();
	wxDirDialog dlg(this,LANG(MULTIFILE_DIR_CAPTION,"Directorio donde buscar"),text.Len()?text:project->last_dir);
	if (wxID_OK!=dlg.ShowModal()) return;
	wxString dir=project->last_dir =dlg.GetPath();
	basedir->SetValue(mxUT::Relativize(dir,project->path));
}

void mxMultipleFileChooser::OnButtonFind(wxCommandEvent &event) {
	list->Freeze();
	list->Clear();
	
	wxArrayString filter_array;
	mxUT::Split(filter->GetValue(),filter_array,true,false);
	
	FindFiles(search_base=DIR_PLUS_FILE(project->path,basedir->GetValue()),"",filter_array,subdirs->GetValue());
	for (int i=0,n=list->GetCount();i<n;i++) 
		if (list->GetString(i).AfterLast('.').Lower()!=PROJECT_EXT)
			list->Check(i,true);
	list->Thaw();
}

void mxMultipleFileChooser::FindFiles(wxString where, wxString sub, wxArrayString &filter_array, bool rec) {
	
	for(unsigned int i=0;i<filter_array.GetCount();i++) { 
		wxString &what=filter_array[i];
		wxDir fil(sub.Len()?DIR_PLUS_FILE(where,sub):where);
		if ( fil.IsOpened() ) {
			wxString filename;
			bool cont = fil.GetFirst(&filename, what , wxDIR_FILES);
			while ( cont ) {
				wxString fname=DIR_PLUS_FILE(sub,filename);
				if (list->FindString(fname)==wxNOT_FOUND) list->Append(fname);
				cont = fil.GetNext(&filename);
			}	
		}
	}
	
	if (!rec) return;
	
	wxDir dir(sub.Len()?DIR_PLUS_FILE(where,sub):where);
	if ( dir.IsOpened() ) {
		wxString filename, spec;
		bool cont = dir.GetFirst(&filename, spec , wxDIR_DIRS);
		while ( cont ) {
			FindFiles(where, DIR_PLUS_FILE(sub,filename),filter_array,true);
			cont = dir.GetNext(&filename);
		}	
	}
	
}

void mxMultipleFileChooser::OnClose(wxCloseEvent &evt) {
	Destroy();
}

void mxMultipleFileChooser::OnListRightClick(wxMouseEvent &event) {
	wxMenu menu;
	menu.Append(mxID_MULTIPLEFILE_MARK_ALL,LANG(MULTIFILE_POPUP_ALL,"marcar todos"));
	menu.Append(mxID_MULTIPLEFILE_MARK_NONE,LANG(MULTIFILE_POPUP_NONE,"desmarcar todos"));
	menu.Append(mxID_MULTIPLEFILE_MARK_INVERT,LANG(MULTIFILE_POPUP_INVERT,"invertir marcados"));
	PopupMenu(&menu);
}
