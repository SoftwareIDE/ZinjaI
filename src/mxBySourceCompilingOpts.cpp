#include "mxBySourceCompilingOpts.h"
#include <wx/listctrl.h>
#include "mxUtils.h"
#include <wx/sizer.h>
#include "mxSizers.h"
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include "ProjectManager.h"

BEGIN_EVENT_TABLE(mxBySourceCompilingOpts,wxDialog)
	EVT_BUTTON(wxID_OK,mxBySourceCompilingOpts::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxBySourceCompilingOpts::OnButtonCancel)
	EVT_BUTTON(wxID_HELP,mxBySourceCompilingOpts::OnButtonHelp)
	EVT_CLOSE(mxBySourceCompilingOpts::OnClose)
END_EVENT_TABLE()

#define _CAPTION LANG(BYSRCOPTS_CAPTION,"Opciones de compilacion por fuente")
	
mxBySourceCompilingOpts::mxBySourceCompilingOpts(wxWindow *parent, project_file_item *item) 
	: wxDialog(parent,wxID_ANY,_CAPTION,wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *top_sizer = new wxBoxSizer(wxHORIZONTAL);
	list = new wxListBox(this,wxID_ANY);
	top_sizer->Add(list,sizers->BA5_Exp1);
	
	wxBoxSizer *right_sizer = new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer *fromprof_sizer = new wxStaticBoxSizer(wxVERTICAL,this,LANG(BYSRCOPTS_FROM_PROFILE,"Tomar desde el perfil"));
	right_sizer->Add(fromprof_sizer,sizers->BA5_Exp0);
	mxUT::AddCheckBox(fromprof_sizer,this,LANG(PROJECTCONFIG_COMPILING_EXTRA_ARGS,"Parametros extra para la compilacion"));
	mxUT::AddCheckBox(fromprof_sizer,this,LANG(PROJECTCONFIG_COMPILING_MACROS,"Macros a definir"));
	mxUT::AddCheckBox(fromprof_sizer,this,LANG(PROJECTCONFIG_COMPILING_EXTRA_PATHS,"Directorios adicionales para buscar cabeceras"));
	mxUT::AddCheckBox(fromprof_sizer,this,LANG(PROJECTCONFIG_COMPILING_STD,"Estandar"));
	mxUT::AddCheckBox(fromprof_sizer,this,LANG(PROJECTCONFIG_COMPILING_WARNINGS,"Nivel de advertencias"));
	mxUT::AddCheckBox(fromprof_sizer,this,LANG(PROJECTCONFIG_COMPILING_DEBUG,"Informacion de depuracion"));
	mxUT::AddCheckBox(fromprof_sizer,this,LANG(PROJECTCONFIG_COMPILING_OPTIM,"Nivel de optimizacion"));
	mxUT::AddLongTextCtrl(right_sizer,this,LANG(BYSRCOPTS_ADDITIONAL_ARGS,"Argumentos de compilación adicionales"));

	wxArrayString profiles;
	for(int i=0;i<project->configurations_count;i++) profiles.Add(project->configurations[i]->name);
	profiles.Add(LANG(BYSRCOPT_ALL_PROFILES,"<<todos los perfiles>>"));
	int cur_profile = profiles.Index(project->active_configuration->name);
	
	mxUT::AddComboBox(right_sizer,this,LANG(BYSRCOPT_PROFILE,"Aplicar para el perfil: "),profiles,cur_profile);
	right_sizer->Add(new wxButton(this,wxID_APPLY,LANG(BYSRCOPT_APPLY_TO_SELECTED_FILES,"Aplicar cambios")),sizers->Center);

	top_sizer->Add(right_sizer,wxSizerFlags().Proportion(2).Expand());
	
	main_sizer->Add(top_sizer,sizers->Exp1);
	main_sizer->Add(mxUT::MakeGenericButtonsSizer(this,true),sizers->Exp0);
	SetEscapeId(wxID_CANCEL);
	SetSizerAndFit(main_sizer);
	Show();
	
}

void mxBySourceCompilingOpts::OnButtonOk (wxCommandEvent & evt) {
	Close();
}

void mxBySourceCompilingOpts::OnButtonCancel (wxCommandEvent & evt) {
	Close();
}

void mxBySourceCompilingOpts::OnButtonHelp (wxCommandEvent & evt) {
	
}

void mxBySourceCompilingOpts::OnClose (wxCloseEvent & evt) {
	Destroy();
}

