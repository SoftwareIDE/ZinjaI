#include "mxDoxyDialog.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "ProjectManager.h"
#include "ids.h"
#include "ConfigManager.h"
#include "mxHelpWindow.h"
#include "Language.h"
#include "mxCommonConfigControls.h"
#include <wx/notebook.h>
#include "mxThreeDotsUtils.h"
using namespace std;

BEGIN_EVENT_TABLE(mxDoxyDialog, wxDialog)
	EVT_BUTTON(wxID_OK,mxDoxyDialog::OnOkButton)
	EVT_BUTTON(mxID_DOXYDIALOG_BASE,mxDoxyDialog::OnBaseDirButton)
	EVT_BUTTON(mxID_DOXYDIALOG_DEST,mxDoxyDialog::OnDestDirButton)
	EVT_BUTTON(wxID_CANCEL,mxDoxyDialog::OnCancelButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxDoxyDialog::OnHelpButton)
	EVT_CLOSE(mxDoxyDialog::OnClose)
END_EVENT_TABLE()

mxDoxyDialog::mxDoxyDialog(wxWindow *parent) 
	: wxDialog(parent, wxID_ANY, LANG(DOXYCONF_CAPTION,"Configuracion Doxygen"), wxDefaultPosition,
			   wxDefaultSize, wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER),
	  m_doxygen_config(project->GetDoxygenConfiguration())
{
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxNotebook *notebook = new wxNotebook(this,wxID_ANY);
	notebook->AddPage(CreateGeneralPanel(notebook), LANG(DOXYCONF_BASIC_OPTIONS,"Opciones Básicas"));
	notebook->AddPage(CreateMorePanel(notebook), LANG(DOXYCONF_MORE_OPTIONS,"Más Opciones"));
	notebook->AddPage(CreateExtraPanel(notebook), LANG(DOXYCONF_EXTRA_TAB,"Campos Adicionales"));
	mySizer->Add(notebook,sizers->Exp1);
	mxCCC::MainSizer(this,mySizer)
		.BeginBottom().Help().Ok().Cancel().EndBottom(this).SetAndFit();
	SetFocus();
	Show();
}

void mxDoxyDialog::OnOkButton(wxCommandEvent &evt) {
	if (!project) { return; }
	m_doxygen_config->destdir = destdir_ctrl->GetValue();
	m_doxygen_config->do_cpps = base_files_ctrl->GetSelection()==1 || base_files_ctrl->GetSelection()==3;
	m_doxygen_config->do_headers = base_files_ctrl->GetSelection()==2 || base_files_ctrl->GetSelection()==3;
	m_doxygen_config->base_path = base_path_ctrl->GetValue();
	m_doxygen_config->exclude_files = exclude_files_ctrl->GetValue();
	m_doxygen_config->extra_files = extra_files_ctrl->GetValue();
	m_doxygen_config->name = name_ctrl->GetValue();
	m_doxygen_config->version = version_ctrl->GetValue();
	m_doxygen_config->save = save_ctrl->GetValue();
	m_doxygen_config->lang = lang_ctrl->GetValue()==LANG(DOXYCONF_LANG_ES,"Español")?"Spanish":"English";
	m_doxygen_config->html = html_ctrl->GetValue();
	m_doxygen_config->preprocess = preprocess_ctrl->GetValue();
	m_doxygen_config->extra_static = static_ctrl->GetValue();
	m_doxygen_config->extra_private = private_ctrl->GetValue();
	m_doxygen_config->use_in_quickhelp = use_in_quickhelp_ctrl->GetValue();
	m_doxygen_config->html_navtree = html_navtree_ctrl->GetValue();
	m_doxygen_config->latex = latex_ctrl->GetValue();
	m_doxygen_config->hideundocs = hideundocs_ctrl->GetValue();
	m_doxygen_config->extra_conf = extra_conf->GetValue();
	Close();
}

void mxDoxyDialog::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxDoxyDialog::OnClose(wxCloseEvent &event) {
	Destroy();
}

wxPanel *mxDoxyDialog::CreateGeneralPanel (wxNotebook *notebook) {
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	mxCCC::MainSizer sizer = mxCCC::CreateMainSizer(panel);
	
	sizer.BeginText( LANG(DOXYCONF_PROJECT_NAME,"Nombre del Proyecto") )
		.Value(m_doxygen_config->name).EndText(name_ctrl);
	
	sizer.BeginText( LANG(DOXYCONF_PROJECT_VER,"Versión del proyecto") )
		.Value(m_doxygen_config->version).EndText(version_ctrl);
	
	sizer.BeginText( LANG(DOXYCONF_DEST_DIR,"Directorio destino") )
		.Value(m_doxygen_config->destdir).Button(mxID_DOXYDIALOG_DEST).EndText(destdir_ctrl);
	
	sizer.BeginCombo( LANG(DOXYCONF_WICH_FILES,"Archivos a procesar") )
		.Add(LANG(DOXYCONF_FILES_NONE,"Ninguno"))
		.Add(wxString(LANG(DOXYCONF_FILES_CPP,"Fuentes"))<<_T(" (c,cpp,cxx...)"))
		.Add(wxString(LANG(DOXYCONF_FILES_HEADERS,"Cabeceras"))<<_T(" (h,hpp,hxx,...)"))
		.Add(LANG(DOXYCONF_FILES_H_AND_CPP,"Fuentes y Cabeceras"))
		.Select((m_doxygen_config->do_cpps?1:0)+(m_doxygen_config->do_headers?2:0))
		.EndCombo(base_files_ctrl);
	
	sizer.BeginText( LANG(DOXYCONF_EXTRA_FILES,"Archivos adicionales a procesar") )
		.Value(m_doxygen_config->extra_files).EndText(extra_files_ctrl);
	
	sizer.BeginText( LANG(DOXYCONF_FILES_TO_EXCLUDE,"Archivos a excluir al procesar") )
		.Value(m_doxygen_config->exclude_files).EndText(exclude_files_ctrl);
	
	sizer.BeginCheck( LANG(DOXYCONF_SAVE_WITH_PROJECT,"Guardar con el proyecto") )
		.Value(m_doxygen_config->save).EndCheck(save_ctrl);
	
	sizer.Set();
	return panel;
}


wxPanel *mxDoxyDialog::CreateExtraPanel (wxNotebook *notebook) {
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	mxCCC::MainSizer sizer = mxCCC::CreateMainSizer(panel);
	
	sizer.BeginLabel( LANG(DOXYCONF_EXTRA_LABEL,"El texto de este campo se agregará sin cambios en el\n"
											    "Doxyfile. Puede utilizarlo para definir parámetros\n"
											    "no contemplados en este cuadro de diálogo.") ).Center().EndLabel();
	
	extra_conf = new wxTextCtrl(panel,wxID_ANY,m_doxygen_config->extra_conf,wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE);
	sizer.GetSizer()->Add(extra_conf,sizers->BA10_Exp1);
	
	sizer.Set();
	return panel;
	
}

wxPanel *mxDoxyDialog::CreateMorePanel (wxNotebook *notebook) {
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	mxCCC::MainSizer sizer = mxCCC::CreateMainSizer(panel);
	
	sizer.BeginCombo( LANG(DOXYCONF_LANG,"Idioma") )
		.Add(LANG(DOXYCONF_LANG_ES,"Español"))
		.Add(LANG(DOXYCONF_LANG_EN,"Ingles"))
		.Select(m_doxygen_config->lang=="Spanish"?0:1)
		.EndCombo(lang_ctrl);

	sizer.BeginText( LANG(DOXYCONF_BASE_DIR,"Directorio base") )
		.Value(m_doxygen_config->base_path).Button(mxID_DOXYDIALOG_BASE).EndText(base_path_ctrl);
	
	sizer.BeginCheck( LANG(DOXYCONF_ENABLE_PREPROC,"Habilitar preprocesado") )
		.Value(m_doxygen_config->preprocess).EndCheck(preprocess_ctrl);
	
	sizer.BeginCheck( LANG(DOXYCONF_EXTRA_PRIVATE,"Incluir métodos/atributos privados") )
		.Value(m_doxygen_config->extra_private).EndCheck(private_ctrl);
	
	sizer.BeginCheck( LANG(DOXYCONF_EXTRA_STATIC,"Incluir funciones/variables static") )
		.Value(m_doxygen_config->extra_static).EndCheck(static_ctrl);
	
	sizer.BeginCheck( LANG(DOXYCONF_ONLY_DOC_ENTITIES,"Extraer solo las entidades documentadas") )
		.Value(m_doxygen_config->hideundocs).EndCheck(hideundocs_ctrl);
	
	sizer.BeginCheck( LANG(DOXYCONF_GENERATE_LATEX,"Generar documentación Latex") )
		.Value(m_doxygen_config->latex).EndCheck(latex_ctrl);
	
	sizer.BeginCheck( LANG(DOXYCONF_GENERATE_HTML,"Generar documentación HTML") )
		.Value(m_doxygen_config->html).EndCheck(html_ctrl);
	
	sizer.BeginCheck( LANG(DOXYCONF_SHOW_NAV_TREE,"Mostrar arbol de navegación (para doc HTML)") )
		.Value(m_doxygen_config->html_navtree).EndCheck(html_navtree_ctrl);
	
	sizer.BeginCheck( LANG(DOXYCONF_INCLUDE_SEARCH_ENGINE,"Incluir motor de búsquedas (para doc HTML)") )
		.Value(m_doxygen_config->html_searchengine).EndCheck(html_searchengine_ctrl);
	
	sizer.BeginCheck( LANG(DOXYCONF_USE_IN_QUICKHELP,"Utilizar en ayuda rápida") )
		.Value(m_doxygen_config->use_in_quickhelp).EndCheck(use_in_quickhelp_ctrl);
	
	sizer.Set();
	return panel;
}

void mxDoxyDialog::OnHelpButton(wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("doxygen.html");
}

void mxDoxyDialog::OnBaseDirButton(wxCommandEvent &event) {
	mxThreeDotsUtils::ReplaceSelectionWithDirectory(this,base_path_ctrl,project->GetPath(),LANG(DOXYCONF_BASE_DIR,"Directorio base"));
}

void mxDoxyDialog::OnDestDirButton(wxCommandEvent &event) {
	mxThreeDotsUtils::ReplaceSelectionWithDirectory(this,destdir_ctrl,project->GetPath(),LANG(DOXYCONF_DEST_DIR,"Directorio destino:"));
}
