#include "mxWxfbConfigDialog.h"
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include "ids.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "mxSizers.h"
#include "mxUtils.h"
#include "ProjectManager.h"
#include "mxHelpWindow.h"
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxWxfbConfigDialog, wxDialog)
	EVT_BUTTON(wxID_OK,mxWxfbConfigDialog::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxWxfbConfigDialog::OnCancelButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxWxfbConfigDialog::OnHelpButton)
	EVT_CLOSE(mxWxfbConfigDialog::OnClose)
END_EVENT_TABLE()

mxWxfbConfigDialog::mxWxfbConfigDialog():wxDialog(main_window, wxID_ANY, LANG(WXFBSETUP_CAPTION,"Configuracion wxFormBuilder"), wxDefaultPosition, wxDefaultSize ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
//	if (!project->doxygen) project->doxygen = new doxygen_configuration(project->project_name);
//	dox = project->doxygen;
//	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	
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
	
	m_activate_integration=utils->AddCheckBox(mySizer,this,LANG(WXFBSETUP_ACTIVATE,"Activar integración con wxFormBuilder"),false);
	m_autoupdate_project=utils->AddCheckBox(mySizer,this,LANG(WXFBSETUP_AUTOUPDATE,"Regenerar proyectos wxFormBuilder automáticamente"),false);
	m_update_class_list=utils->AddCheckBox(mySizer,this,LANG(WXFBSETUP_UPDATE_CLASSES,"Crear/eliminar clases heredadas luego de regenerar"),false);
	m_update_methods=utils->AddCheckBox(mySizer,this,LANG(WXFBSETUP_UPDATE_METHODS,"Actualizar clases heredadas automáticamente"),false);
	m_set_wxfb_sources_as_readonly=utils->AddCheckBox(mySizer,this,LANG(WXFBSETUP_READONLY,"Marcar fuentes generados por wxfb como solo-lectura"),false);
	m_dont_show_base_classes_in_goto=utils->AddCheckBox(mySizer,this,LANG(WXFBSETUP_HIDEONGOTO,"Ocultar métodos y clases generadas por wxfb en el cuadro \"Ir a Función/Clase/Método\""),false);
	
	mySizer->Add(bottomSizer,sizers->Exp0);
	
	SetSizerAndFit(mySizer);
	
	SetFocus();
	Show();
	
}

mxWxfbConfigDialog::~mxWxfbConfigDialog() {
	
}

void mxWxfbConfigDialog::OnClose (wxCloseEvent & event) {
	Destroy();
}

void mxWxfbConfigDialog::OnOkButton (wxCommandEvent & evt) {
	if (!project) { return; }
//	Close();
}

void mxWxfbConfigDialog::OnCancelButton (wxCommandEvent & evt) {
	Close();
}

void mxWxfbConfigDialog::OnHelpButton (wxCommandEvent & evt) {
	SHOW_HELP("wxfb_config.html");
}

