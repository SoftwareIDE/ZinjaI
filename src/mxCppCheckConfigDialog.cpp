#include "mxCppCheckConfigDialog.h"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
#include "ids.h"
#include "mxUtils.h"
#include "mxSizers.h"
#include "mxArt.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "ProjectManager.h"
#include <wx/notebook.h>
#include "mxEnumerationEditor.h"
#include "mxMultipleChoiceEditor.h"
#include "mxHelpWindow.h"

BEGIN_EVENT_TABLE(mxCppCheckConfigDialog,wxDialog)
	EVT_CHECKBOX(mxID_CPPCHECK_COPYCONFIG,mxCppCheckConfigDialog::OnCheckCopyConfig)
	EVT_BUTTON(mxID_CPPCHECK_CONFIG_D,mxCppCheckConfigDialog::OnButtonConfigD)
	EVT_BUTTON(mxID_CPPCHECK_CONFIG_U,mxCppCheckConfigDialog::OnButtonConfigU)
	EVT_BUTTON(mxID_CPPCHECK_STYLE,mxCppCheckConfigDialog::OnButtonStyle)
	EVT_BUTTON(mxID_CPPCHECK_PLATFORM,mxCppCheckConfigDialog::OnButtonPlatform)
	EVT_BUTTON(mxID_CPPCHECK_STANDARD,mxCppCheckConfigDialog::OnButtonStandard)
	EVT_BUTTON(mxID_CPPCHECK_SUPPRESS_FILE,mxCppCheckConfigDialog::OnButtonSuppressFile)
	EVT_BUTTON(mxID_CPPCHECK_SUPPRESS_IDS,mxCppCheckConfigDialog::OnButtonSuppressIds)
	EVT_BUTTON(mxID_CPPCHECK_INCLUDE_FILE,mxCppCheckConfigDialog::OnButtonIncludeFile)
	EVT_BUTTON(mxID_CPPCHECK_EXCLUDE_FILE,mxCppCheckConfigDialog::OnButtonExcludeFile)
	EVT_BUTTON(wxID_OK,mxCppCheckConfigDialog::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxCppCheckConfigDialog::OnButtonCancel)
	EVT_BUTTON(mxID_HELP_BUTTON,mxCppCheckConfigDialog::OnButtonHelp)
	EVT_CLOSE(mxCppCheckConfigDialog::OnClose)
END_EVENT_TABLE()

mxCppCheckConfigDialog::mxCppCheckConfigDialog(wxWindow *parent) : wxDialog(parent,wxID_ANY,"Configuración de CppCheck",wxDefaultPosition,wxDefaultSize,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	if (!project->cppcheck) project->cppcheck = new cppcheck_configuration();
	ccc = project->cppcheck;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxNotebook *notebook = new wxNotebook(this,wxID_ANY);
	
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxButton *cancel_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	SetEscapeId(wxID_CANCEL);
	wxButton *ok_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	ok_button->SetDefault(); 
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	
	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	
	notebook->AddPage(CreateGeneralPanel(notebook), "General");
	notebook->AddPage(CreateFilesPanel(notebook), "Files");
	
	mySizer->Add(notebook,sizers->Exp1);
	mySizer->Add(bottomSizer,sizers->Exp0);
	
	SetSizerAndFit(mySizer);
	
	SetFocus();
	ShowModal();
	
}

wxPanel *mxCppCheckConfigDialog::CreateGeneralPanel(wxNotebook *notebook) {
	
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	wxBoxSizer *mySizer= new wxBoxSizer(wxVERTICAL);
	
	copy_from_config = utils->AddCheckBox(mySizer,panel,LANG(CPPCHECK_COPY_FROM_CONFIG,"Copiar configuración (macros definidas) de las opciones del proyecto"),ccc->copy_from_config,mxID_CPPCHECK_COPYCONFIG);
	config_d = utils->AddDirCtrl(mySizer,panel,LANG(CPPCHECK_CONFIG_D,"  Configuraciones a verificar"),ccc->config_d,mxID_CPPCHECK_CONFIG_D,"...",false);
	config_u = utils->AddDirCtrl(mySizer,panel,LANG(CPPCHECK_CONFIG_U,"  Configuraciones a saltear"),ccc->config_u,mxID_CPPCHECK_CONFIG_U,"...",false);
	
	config_u->Enable(!copy_from_config->GetValue()); config_d->Enable(!copy_from_config->GetValue());
	
	style = utils->AddDirCtrl(mySizer,panel,LANG(CPPCHECK_STYLE,"Verificaciones adicionales"),ccc->style,mxID_CPPCHECK_STYLE,"...");
	
	platform = utils->AddDirCtrl(mySizer,panel,LANG(CPPCHECK_PLATFORM,"Verificaciones especificas de una plataforma"),ccc->platform,mxID_CPPCHECK_PLATFORM);
	
	standard = utils->AddDirCtrl(mySizer,panel,LANG(CPPCHECK_STANDARD,"Verificaciones especificas de un estándar"),ccc->standard,mxID_CPPCHECK_STANDARD);
	
	suppress_ids = utils->AddDirCtrl(mySizer,panel,LANG(CPPCHECK_SUPPRESS_IDS,"Supresiones"),ccc->suppress_ids,mxID_CPPCHECK_SUPPRESS_IDS);
	suppress_file = utils->AddDirCtrl(mySizer,panel,LANG(CPPCHECK_SUPPRESS_FILE,"Archivo con lista de supresiones"),ccc->suppress_file,mxID_CPPCHECK_SUPPRESS_FILE);
	inline_suppr = utils->AddCheckBox(mySizer,panel,LANG(CPPCHECK_INLINE_SUPPR,"Habilitar supresiones inline"),ccc->inline_suppr);
	
	save_in_project = utils->AddCheckBox(mySizer,panel,LANG(CPPCHECK_SAVE,"Guardar esta configuración con el proyecto"),ccc->save_in_project);
	
	panel->SetSizer(mySizer);
	return panel;
	
}

wxPanel *mxCppCheckConfigDialog::CreateFilesPanel (wxNotebook * notebook) {
	
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	wxSizer *src_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *szsrc_buttons = new wxBoxSizer(wxVERTICAL);
	szsrc_buttons->Add(new wxButton(panel,mxID_CPPCHECK_INCLUDE_FILE,">>>",wxDefaultPosition,wxSize(50,-1)),sizers->BA10_Exp0);
	szsrc_buttons->Add(new wxButton(panel,mxID_CPPCHECK_EXCLUDE_FILE,"<<<",wxDefaultPosition,wxSize(50,-1)),sizers->BA10_Exp0);
	wxSizer *szsrc_in = new wxBoxSizer(wxVERTICAL);
	szsrc_in->Add(new wxStaticText(panel,wxID_ANY,LANG(CPPCHECK_SOURCES_IN,"Files to analize")),sizers->Exp0);
	sources_in = new wxListBox(panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,NULL,wxLB_SORT|wxLB_EXTENDED|wxLB_NEEDED_SB);
	szsrc_in->Add(sources_in,sizers->Exp1);
	wxSizer *szsrc_out = new wxBoxSizer(wxVERTICAL);
	szsrc_out->Add(new wxStaticText(panel,wxID_ANY,LANG(CPPCHECK_SOURCES_OUT,"Files to exclude")),sizers->Exp0);
	sources_out = new wxListBox(panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,NULL,wxLB_SORT|wxLB_EXTENDED|wxLB_NEEDED_SB);
	szsrc_out->Add(sources_out,sizers->Exp1);
	src_sizer->Add(szsrc_out,sizers->Exp1);
	src_sizer->Add(szsrc_buttons,sizers->Center);
	src_sizer->Add(szsrc_in,sizers->Exp1);
	
	wxArrayString array;
	utils->Split(ccc->exclude_list,array,true,false);
	file_item *fi=project->first_source;
	ML_ITERATE(fi) {
		if (array.Index(fi->name)==wxNOT_FOUND)
			sources_in->Append(fi->name);
		else 
			sources_out->Append(fi->name);
	}
	
	panel->SetSizer(src_sizer);
	return panel;
	
}

void mxCppCheckConfigDialog::OnCheckCopyConfig (wxCommandEvent & evt) {
	evt.Skip();
	config_u->Enable(!copy_from_config->GetValue());
	config_d->Enable(!copy_from_config->GetValue());
}

void mxCppCheckConfigDialog::OnButtonConfigD (wxCommandEvent & evt) {
	new mxEnumerationEditor(this,LANG(CPPCHECK_CONFIG_D,"Configurations to analize"),config_d,true);
}

void mxCppCheckConfigDialog::OnButtonConfigU (wxCommandEvent & evt) {
	new mxEnumerationEditor(this,LANG(CPPCHECK_CONFIG_U,"Configurations to analize"),config_u,true);
}

void mxCppCheckConfigDialog::OnButtonStyle (wxCommandEvent & evt) {
	wxArrayString array;
	array.Add("style");
	array.Add("performance");
	array.Add("portability");
	array.Add("information");
	array.Add("unusedFunction");
	array.Add("missingInclude");
	if (style->GetValue()=="all") style->SetValue("information missingInclude performance portability style unusedFunction");
	new mxMultipleChoiceEditor(this,"CppCheck",LANG(CPPCHECK_STYLE,"Additional checks"),style,array);
	if (style->GetValue()=="information missingInclude performance portability style unusedFunction") style->SetValue("all");
}

void mxCppCheckConfigDialog::OnButtonPlatform (wxCommandEvent & evt) {
	wxArrayString array;
	array.Add("unix32");
	array.Add("unix64");
	array.Add("win32A");
	array.Add("win32W");
	array.Add("win64");
	new mxMultipleChoiceEditor(this,"CppCheck",LANG(CPPCHECK_PLATFORM,"Specific platforms"),platform,array);
}

void mxCppCheckConfigDialog::OnButtonStandard (wxCommandEvent & evt) {
	wxArrayString array;
	array.Add("posix");
	array.Add("c99");
	array.Add("c++11");	
	new mxMultipleChoiceEditor(this,"CppCheck",LANG(CPPCHECK_STANDARD,"Standard related checks"),standard,array);
}

void mxCppCheckConfigDialog::OnButtonSuppressFile (wxCommandEvent & evt) {
	
}

void mxCppCheckConfigDialog::OnButtonSuppressIds (wxCommandEvent & evt) {
	new mxEnumerationEditor(this,LANG(CPPCHECK_SUPPRESS_IDS,"Suppressions"),suppress_ids,true);
}

void mxCppCheckConfigDialog::OnButtonIncludeFile (wxCommandEvent & evt) {
	sources_in->SetSelection(wxNOT_FOUND);
	for (int i=sources_out->GetCount();i>=0;i--)
		if (sources_out->IsSelected(i)) {
			sources_in->Append(sources_out->GetString(i));
			sources_in->Select(sources_in->FindString(sources_out->GetString(i)));
			sources_out->Delete(i);
		}
}

void mxCppCheckConfigDialog::OnButtonExcludeFile (wxCommandEvent & evt) {
	sources_out->SetSelection(wxNOT_FOUND);
	for (int i=sources_in->GetCount();i>=0;i--)
		if (sources_in->IsSelected(i)) {
			sources_out->Append(sources_in->GetString(i));
			sources_out->Select(sources_out->FindString(sources_in->GetString(i)));
			sources_in->Delete(i);
		}
}

void mxCppCheckConfigDialog::OnClose (wxCloseEvent & evt) {
	Destroy();
}

void mxCppCheckConfigDialog::OnButtonOk (wxCommandEvent & evt) {
	ccc->exclude_list.Clear();
	for (unsigned int i=0;i<sources_out->GetCount();i++)
		ccc->exclude_list<<utils->Quotize(sources_out->GetString(i))<<" ";
	ccc->copy_from_config=copy_from_config->GetValue();
	ccc->config_d=config_d->GetValue();
	ccc->config_u=config_u->GetValue();
	ccc->style=style->GetValue();
	ccc->platform=platform->GetValue();
	ccc->standard=standard->GetValue();
	ccc->suppress_file=suppress_file->GetValue();
	ccc->suppress_ids=suppress_ids->GetValue();
	ccc->inline_suppr=inline_suppr->GetValue();
	ccc->save_in_project=save_in_project->GetValue();
	Close();
}

void mxCppCheckConfigDialog::OnButtonCancel (wxCommandEvent & evt) {
	Close();
}

void mxCppCheckConfigDialog::OnButtonHelp (wxCommandEvent & evt) {
	SHOW_HELP("cppcheck_config.html");
}

