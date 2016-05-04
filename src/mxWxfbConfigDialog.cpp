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
	EVT_CHECKBOX(wxID_FIND,mxWxfbConfigDialog::OnActivateClick)
END_EVENT_TABLE()

mxWxfbConfigDialog::mxWxfbConfigDialog()
	: mxDialog(main_window, LANG(WXFBSETUP_CAPTION,"Configuracion wxFormBuilder") ) 
{
	CreateSizer sizer(this);
	
	conf=project->GetWxfbConfiguration(false);
	
	sizer.BeginCheck( LANG(WXFBSETUP_ACTIVATE,"Activar integración con wxFormBuilder") )
		.Value(conf->activate_integration).Id(wxID_FIND).EndCheck(m_activate_integration);
	
	sizer.BeginCheck( LANG(WXFBSETUP_AUTOUPDATE,"Regenerar proyectos wxFormBuilder automáticamente") )
		.Value(conf->autoupdate_projects).RegisterIn(m_disabler).EndCheck(m_autoupdate_projects);
	
	sizer.BeginLine().Space(15).
		BeginCheck( LANG(WXFBSETUP_AUTOUPDATE_TEMP_DISABLED,"Deshabilitar temporalmente") )
			.Value(conf->autoupdate_projects_temp_disabled).RegisterIn(m_disabler).EndCheck(m_autoupdate_projects_temp_disabled)
		.EndLine();
		
	sizer.BeginCheck( LANG(WXFBSETUP_UPDATE_CLASSES,"Crear/eliminar clases heredadas luego de regenerar") )
		.Value(conf->update_class_list).RegisterIn(m_disabler).EndCheck(m_update_class_list);
	
	sizer.BeginCheck( LANG(WXFBSETUP_UPDATE_METHODS,"Actualizar clases heredadas automáticamente") )
		.Value(conf->update_methods).RegisterIn(m_disabler).EndCheck(m_update_methods);
	
	sizer.BeginCheck( LANG(WXFBSETUP_READONLY,"Marcar fuentes generados por wxfb como solo-lectura") )
		.Value(conf->set_wxfb_sources_as_readonly).RegisterIn(m_disabler).EndCheck(m_set_wxfb_sources_as_readonly);
	
	sizer.BeginCheck( LANG(WXFBSETUP_HIDEONGOTO,"Ocultar métodos y clases generadas por wxfb en el cuadro \"Ir a Función/Clase/Método\"") )
		.Value(conf->dont_show_base_classes_in_goto).RegisterIn(m_disabler).EndCheck(m_dont_show_base_classes_in_goto);
	
	sizer.BeginBottom().Help().Ok().Cancel().EndBottom(this).SetAndFit();
	
	m_disabler.EnableAll(conf->activate_integration);
	
	Show();
	SetFocus();
}

void mxWxfbConfigDialog::OnOkButton (wxCommandEvent & evt) {
	if (!project) { return; }
	bool old_hide_symbols=conf->dont_show_base_classes_in_goto, 
		 old_read_only=conf->set_wxfb_sources_as_readonly;
	if (conf->activate_integration!=m_activate_integration->GetValue())
		project->ActivateWxfb(m_activate_integration->GetValue());
	conf->autoupdate_projects=m_autoupdate_projects->GetValue();
	conf->autoupdate_projects_temp_disabled = 
		conf->autoupdate_projects && m_autoupdate_projects_temp_disabled->GetValue();
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

