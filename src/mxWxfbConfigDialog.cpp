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
#include "mxCommonConfigControls.h"

BEGIN_EVENT_TABLE(mxWxfbConfigDialog, wxDialog)
	EVT_BUTTON(wxID_OK,mxWxfbConfigDialog::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxWxfbConfigDialog::OnCancelButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxWxfbConfigDialog::OnHelpButton)
	EVT_CHECKBOX(wxID_FIND,mxWxfbConfigDialog::OnActivateClick)
	EVT_CLOSE(mxWxfbConfigDialog::OnClose)
END_EVENT_TABLE()

mxWxfbConfigDialog::mxWxfbConfigDialog():wxDialog(main_window, wxID_ANY, LANG(WXFBSETUP_CAPTION,"Configuracion wxFormBuilder"), wxDefaultPosition, wxDefaultSize ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	conf=project->GetWxfbConfiguration(false);
	
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
	
	m_activate_integration=mxCCC::AddCheckBox(mySizer,this,LANG(WXFBSETUP_ACTIVATE,"Activar integración con wxFormBuilder"),conf->activate_integration,wxID_FIND);
	m_autoupdate_projects=mxCCC::AddCheckBox(mySizer,this,LANG(WXFBSETUP_AUTOUPDATE,"Regenerar proyectos wxFormBuilder automáticamente"),conf->autoupdate_projects);
	m_disabler.Add(m_autoupdate_projects);
	m_update_class_list=mxCCC::AddCheckBox(mySizer,this,LANG(WXFBSETUP_UPDATE_CLASSES,"Crear/eliminar clases heredadas luego de regenerar"),conf->update_class_list);
	m_disabler.Add(m_update_class_list);
	m_update_methods=mxCCC::AddCheckBox(mySizer,this,LANG(WXFBSETUP_UPDATE_METHODS,"Actualizar clases heredadas automáticamente"),conf->update_methods);
	m_disabler.Add(m_update_methods);
	m_set_wxfb_sources_as_readonly=mxCCC::AddCheckBox(mySizer,this,LANG(WXFBSETUP_READONLY,"Marcar fuentes generados por wxfb como solo-lectura"),conf->set_wxfb_sources_as_readonly);
	m_disabler.Add(m_set_wxfb_sources_as_readonly);
	m_dont_show_base_classes_in_goto=mxCCC::AddCheckBox(mySizer,this,LANG(WXFBSETUP_HIDEONGOTO,"Ocultar métodos y clases generadas por wxfb en el cuadro \"Ir a Función/Clase/Método\""),conf->dont_show_base_classes_in_goto);
	m_disabler.Add(m_dont_show_base_classes_in_goto);
	
	mySizer->Add(bottomSizer,sizers->Exp0);
	
	SetSizerAndFit(mySizer);
	
	m_disabler.EnableAll(conf->activate_integration);
	
	Show();
	SetFocus();
	
}

void mxWxfbConfigDialog::OnClose (wxCloseEvent & event) {
	Destroy();
}

void mxWxfbConfigDialog::OnOkButton (wxCommandEvent & evt) {
	if (!project) { return; }
	bool old_hide_symbols=conf->dont_show_base_classes_in_goto, 
		 old_read_only=conf->set_wxfb_sources_as_readonly;
	if (conf->activate_integration!=m_activate_integration->GetValue())
		project->ActivateWxfb(m_activate_integration->GetValue());
	conf->autoupdate_projects=m_autoupdate_projects->GetValue();
	conf->update_class_list=m_update_class_list->GetValue();
	conf->update_methods=m_update_methods->GetValue();
	conf->set_wxfb_sources_as_readonly=m_set_wxfb_sources_as_readonly->GetValue();
	conf->dont_show_base_classes_in_goto=m_dont_show_base_classes_in_goto->GetValue();
	project->WxfbSetFileProperties(
		conf->set_wxfb_sources_as_readonly!=old_read_only,
		conf->set_wxfb_sources_as_readonly,
		conf->dont_show_base_classes_in_goto!=old_hide_symbols,
		conf->dont_show_base_classes_in_goto);
	Close();
}

void mxWxfbConfigDialog::OnCancelButton (wxCommandEvent & evt) {
	Close();
}

void mxWxfbConfigDialog::OnHelpButton (wxCommandEvent & evt) {
	mxHelpWindow::ShowHelp("wxfb_config.html");
}

void mxWxfbConfigDialog::OnActivateClick (wxCommandEvent & evt) {
	m_disabler.EnableAll(m_activate_integration->GetValue());
}

