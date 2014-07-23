#include "mxLibToBuildWindow.h"
#include "ids.h"
#include "ProjectManager.h"
#include "Language.h"
#include "mxSizers.h"
#include "mxArt.h"
#include "mxBitmapButton.h"
#include "mxHelpWindow.h"
#include "mxMessageDialog.h"
#include "mxProjectConfigWindow.h"

BEGIN_EVENT_TABLE(mxLibToBuildWindow, wxDialog)
	
	EVT_BUTTON(mxID_LIBS_IN,mxLibToBuildWindow::OnInButton)
	EVT_BUTTON(mxID_LIBS_OUT,mxLibToBuildWindow::OnOutButton)
	EVT_BUTTON(wxID_CANCEL,mxLibToBuildWindow::OnCancelButton)
	EVT_BUTTON(wxID_OK,mxLibToBuildWindow::OnOkButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxLibToBuildWindow::OnHelpButton)
	EVT_TEXT(wxID_ANY,mxLibToBuildWindow::OnCombo)
	EVT_CLOSE(mxLibToBuildWindow::OnClose)
	
END_EVENT_TABLE()
	
wxString mxLibToBuildWindow::new_name;
	
mxLibToBuildWindow::mxLibToBuildWindow(mxProjectConfigWindow *aparent, project_configuration *conf, project_library *alib) : wxDialog(aparent,wxID_ANY,LANG(LIBTOBUILD_CAPTION,"Generar biblioteca"),wxDefaultPosition,wxDefaultSize,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

	parent=aparent;
	constructed=false;
	configuration=conf;
	lib=alib;
	
	wxBoxSizer *mySizer= new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *butSizer = new wxBoxSizer(wxHORIZONTAL);
	
	name = mxUT::AddTextCtrl(mySizer,this,LANG(LIBTOBUILD_NAME,"Nombre"),lib?lib->libname:"");
	path = mxUT::AddTextCtrl(mySizer,this,LANG(LIBTOBUILD_DIR,"Directorio de salida"),lib?lib->path:configuration->temp_folder);
	filename = mxUT::AddTextCtrl(mySizer,this,LANG(LIBTOBUILD_FILE,"Archivo de salida"),"");
	filename->SetEditable(false);
	extra_link = mxUT::AddTextCtrl(mySizer,this,LANG(LIBTOBUILD_EXTRA_LINK,"Opciones de enlazado"),lib?lib->extra_link:"");
	
	wxArrayString tipos;
	tipos.Add(LANG(LIBTOBUILD_DYNAMIC,"Dinamica"));
	tipos.Add(LANG(LIBTOBUILD_STATIC,"Estatica"));
	type = mxUT::AddComboBox(mySizer,this,LANG(LIBTOBUILD_TYPE,"Tipo de biblioteca"),tipos,0);
	if (lib) type->SetSelection(lib->is_static?1:0);
		
	wxSizer *src_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *szsrc_buttons = new wxBoxSizer(wxVERTICAL);
	szsrc_buttons->Add(new wxButton(this,mxID_LIBS_IN,">>>",wxDefaultPosition,wxSize(50,-1)),sizers->BA10_Exp0);
	szsrc_buttons->Add(new wxButton(this,mxID_LIBS_OUT,"<<<",wxDefaultPosition,wxSize(50,-1)),sizers->BA10_Exp0);
	wxSizer *szsrc_in = new wxBoxSizer(wxVERTICAL);
	szsrc_in->Add(new wxStaticText(this,wxID_ANY,LANG(LIBTOBUILD_SOURCES_IN,"Fuentes a incluir")),sizers->Exp0);
	sources_in = new wxListBox(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,NULL,wxLB_SORT|wxLB_EXTENDED|wxLB_NEEDED_SB);
	szsrc_in->Add(sources_in,sizers->Exp1);
	wxSizer *szsrc_out = new wxBoxSizer(wxVERTICAL);
	szsrc_out->Add(new wxStaticText(this,wxID_ANY,LANG(LIBTOBUILD_SOURCES_OUT,"Fuentes a excluir")),sizers->Exp0);
	sources_out = new wxListBox(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,NULL,wxLB_SORT|wxLB_EXTENDED|wxLB_NEEDED_SB);
	szsrc_out->Add(sources_out,sizers->Exp1);
	src_sizer->Add(szsrc_out,sizers->Exp1);
	src_sizer->Add(szsrc_buttons,sizers->Center);
	src_sizer->Add(szsrc_in,sizers->Exp1);
	mySizer->Add(src_sizer,sizers->BA10_Exp1);
	
	default_lib = mxUT::AddCheckBox(mySizer,this,LANG(LIBTOBUILD_DEFAULT,"Biblioteca por defecto para nuevos fuentes"),lib?lib->default_lib:false);
	
	wxBitmapButton *help_button = new wxBitmapButton(this,mxID_HELP_BUTTON,*bitmaps->buttons.help);
	butSizer->Add(help_button,sizers->BA5);
	butSizer->AddStretchSpacer();
	wxBitmapButton *cancel_button = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	butSizer->Add(cancel_button,sizers->BA5);
	wxBitmapButton *ok_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	butSizer->Add(ok_button,sizers->BA5);
	
	mySizer->Add(butSizer,sizers->BA5_Exp0);
	
	mySizer->SetMinSize(400,400);
	SetSizerAndFit(mySizer);
	ok_button->SetDefault();
	SetEscapeId(wxID_CANCEL);
	
	project->AssociateLibsAndSources(configuration);

	LocalListIterator<project_file_item*> item(&project->files_sources);
	while (item.IsValid()) {
		if (lib && item->lib==lib)
			sources_in->Append(item->name);
		else
			sources_out->Append(item->name);
		item.Next();
	}
	
	constructed=true;
	SetFName();
	name->SetFocus();
	ShowModal();
}

mxLibToBuildWindow::~mxLibToBuildWindow() {
	
}

void mxLibToBuildWindow::OnClose(wxCloseEvent &evt) {
	Destroy();
}

void mxLibToBuildWindow::OnOkButton(wxCommandEvent &evt) {
	new_name = name->GetValue();
	if (new_name.Len()==0) {
		mxMessageDialog(this,LANG(LIBTOBUILD_NAME_MISSING,"Debe completar el nombre."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
		return;
	}
	if (!lib) {
		if (project->GetLibToBuild(configuration,name->GetValue())) {
			mxMessageDialog(this,LANG(LIBTOBUILD_NAME_REPEATED,"Ya existe una biblioteca con ese nombre."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
			return;
		}
		lib = project->AppendLibToBuild(configuration);
	}
	lib->libname = new_name;
	lib->path = path->GetValue();
//	lib->filename = filename->GetValue();
	lib->extra_link = extra_link->GetValue();
	lib->is_static = type->GetSelection()==1;
	lib->default_lib = default_lib->GetValue();
	if (lib->default_lib) {
		project_library *aux = configuration->libs_to_build;
		while (aux) {
			if (aux!=lib) aux->default_lib=false;
			aux = aux->next;
		}
	}
	LocalListIterator<project_file_item*> fi(&project->files_sources);
	while(fi.IsValid()) {
		if (sources_in->FindString(fi->name)!=wxNOT_FOUND) {
#if !defined(_WIN32) && !defined(__WIN32__)
			if (!fi->lib) fi->force_recompile=true; // por el fPIC
#endif
			fi->lib = lib;
		} else if (fi->lib == lib) {
#if !defined(_WIN32) && !defined(__WIN32__)
			if (!fi->lib) fi->force_recompile=true; // por el fPIC
			fi->lib=NULL;
#endif
		}
		fi.Next();
	}
	project->SaveLibsAndSourcesAssociation(configuration);
	lib->need_relink=true;
	parent->linking_force_relink->SetValue(true);
	Close();
}

void mxLibToBuildWindow::OnCancelButton(wxCommandEvent &evt) {
	new_name = "";
	Close();
}

void mxLibToBuildWindow::OnHelpButton(wxCommandEvent &evt) {
	mxHelpWindow::ShowHelp("lib_to_build.html",this);
}

void mxLibToBuildWindow::OnOutButton(wxCommandEvent &evt) {
	sources_out->SetSelection(wxNOT_FOUND);
	for (int i=sources_in->GetCount();i>=0;i--)
		if (sources_in->IsSelected(i)) {
			sources_out->Append(sources_in->GetString(i));
			sources_out->Select(sources_out->FindString(sources_in->GetString(i)));
			sources_in->Delete(i);
		}
}

void mxLibToBuildWindow::OnInButton(wxCommandEvent &evt) {
	sources_in->SetSelection(wxNOT_FOUND);
	for (int i=sources_out->GetCount();i>=0;i--)
		if (sources_out->IsSelected(i)) {
			sources_in->Append(sources_out->GetString(i));
			sources_in->Select(sources_in->FindString(sources_out->GetString(i)));
			sources_out->Delete(i);
		}
}

void mxLibToBuildWindow::OnCombo(wxCommandEvent &evt) {
	wxObject *w = evt.GetEventObject();
	if (w==name || w==type || w==path)
		SetFName();
}
void mxLibToBuildWindow::SetFName() {
	if (!constructed) return;
	bool is_static = type->GetSelection()==1;
	wxString fname = DIR_PLUS_FILE(path->GetValue(),wxString("lib")<<name->GetValue());
#if defined(_WIN32) || defined(__WIN32__)
	if (is_static)
		fname<<".a";
	else
		fname<<".dll";
#else
	if (is_static)
		fname<<".a";
	else
		fname<<".so";
#endif
	filename->SetValue(fname);
}
