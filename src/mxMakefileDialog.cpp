#include "mxMakefileDialog.h"
#include <wx/wx.h>
#include <wx/sizer.h>
#include "ids.h"
#include "ProjectManager.h"
#include "ConfigManager.h"
#include "mxHelpWindow.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxMakefileDialog, wxDialog)
	EVT_BUTTON(wxID_OK,mxMakefileDialog::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxMakefileDialog::OnCancelButton)
	EVT_BUTTON(mxID_MAKEFILE_EXPLORE,mxMakefileDialog::OnExploreButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxMakefileDialog::OnHelpButton)
	EVT_CLOSE(mxMakefileDialog::OnClose)
END_EVENT_TABLE()
	
mxMakefileDialog::mxMakefileDialog(wxWindow* parent, wxWindowID id, const wxPoint& pos , const wxSize& size , long style) : wxDialog(parent, id, LANG(MAKEFILE_CAPTION,"Generar Makefile"), pos, size, style) {
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer= new wxBoxSizer(wxHORIZONTAL);
		
	wxArrayString values;
	int sel=0;
	for (int i=0;i<project->configurations_count;i++) {
		values.Add(project->configurations[i]->name);
		if (project->configurations[i]==project->active_configuration)
			sel=i;
	}
	configuration_name = utils->AddComboBox(mySizer,this,LANG(MAKEFILE_PROFILE,"Perfil"),values,sel);
	mingw_dir = utils->AddTextCtrl(mySizer,this,LANG(MAKEFILE_REPLACE_MINGW,"Reemplazar \"${MINGW_DIR}\" con"),config->mingw_real_path);
#if defined(__WIN32__)
	expand_comas = utils->AddCheckBox(mySizer,this,LANG(MAKEFILE_EXPAND_SUBCOMMANDS,"Expandir subcomandos (comandos entre acentos)."),true);
#else
	expand_comas = utils->AddCheckBox(mySizer,this,LANG(MAKEFILE_EXPAND_SUBCOMMANDS,"Expandir subcomandos (comandos entre acentos)."),false);
#endif
	
	file_path = utils->AddDirCtrl(mySizer,this,LANG(MAKEFILE_DESTINATION_FILE,"Destino"),DIR_PLUS_FILE(project->path,"Makefile"),mxID_MAKEFILE_EXPLORE);
	
	wxArrayString split_values;
	split_values.Add(LANG(MAKEFILE_TYPE_FULL,"Completo"));
	split_values.Add(LANG(MAKEFILE_TYPE_CONFIG,"Por partes: configuracion"));
	split_values.Add(LANG(MAKEFILE_TYPE_OBJECTS,"Por partes: objetos"));
	split_type = utils->AddComboBox(mySizer,this,LANG(MAKEFILE_TYPE,"Formato"),split_values,0);
	
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancel"));
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(MAKEFILE_GENERATE,"Generar"));
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));

	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	
	mySizer->Add(bottomSizer,sizers->BA5_Exp0);

	
	SetSizerAndFit(mySizer);
	
	file_path->SetFocus();

	SetEscapeId(wxID_CANCEL);
	ok_button->SetDefault();
	
	ShowModal();

}


void mxMakefileDialog::OnOkButton(wxCommandEvent &event) {
	if (file_path->GetValue().Len()) {
		Close();
		project_configuration *orig = project->active_configuration;
		project->active_configuration = project->GetConfig(configuration_name->GetValue());
		makefile_type mktype=MKTYPE_FULL;
		if (split_type->GetSelection()==1) mktype=MKTYPE_CONFIG;
		if (split_type->GetSelection()==2) mktype=MKTYPE_OBJS;
		wxString fpath=file_path->GetValue();
		if (mktype==MKTYPE_OBJS) fpath=DIR_PLUS_FILE(wxFileName(fpath).GetPath(),"Makefile.common");
		project->ExportMakefile(fpath,expand_comas->GetValue(),mingw_dir->GetValue(),mktype);
		project->active_configuration = orig;
	}
}

void mxMakefileDialog::OnCancelButton(wxCommandEvent &event) {
	Close();
}

void mxMakefileDialog::OnExploreButton(wxCommandEvent &event) {
	wxFileDialog dlg (this, LANG(MAKEFILE_CAPTION,"Generar Makefile"),project->path,file_path->GetValue(), _T("Any file (*)|*"), wxFD_SAVE);
	if (dlg.ShowModal() == wxID_OK)
		file_path->SetValue(dlg.GetPath());
}

void mxMakefileDialog::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxMakefileDialog::OnHelpButton(wxCommandEvent &event){
	SHOW_HELP("makefile.html");
}
