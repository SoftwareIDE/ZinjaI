#include "mxDoxyDialog.h"
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "ProjectManager.h"
#include "ids.h"
#include "ConfigManager.h"
#include "mxHelpWindow.h"
#include "Language.h"
using namespace std;

BEGIN_EVENT_TABLE(mxDoxyDialog, wxDialog)
	EVT_BUTTON(wxID_OK,mxDoxyDialog::OnOkButton)
	EVT_BUTTON(mxID_DOXYDIALOG_BASE,mxDoxyDialog::OnBaseDirButton)
	EVT_BUTTON(mxID_DOXYDIALOG_DEST,mxDoxyDialog::OnDestDirButton)
	EVT_BUTTON(wxID_CANCEL,mxDoxyDialog::OnCancelButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxDoxyDialog::OnHelpButton)
	EVT_CLOSE(mxDoxyDialog::OnClose)
END_EVENT_TABLE()

mxDoxyDialog::mxDoxyDialog() : wxDialog(main_window, wxID_ANY, LANG(DOXYCONF_CAPTION,"Configuracion Doxygen"), wxDefaultPosition, wxDefaultSize ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

	if (!project->doxygen) project->doxygen = new doxygen_configuration(project->project_name);
	dox = project->doxygen;
	
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
	
	notebook->AddPage(CreateGeneralPanel(notebook), LANG(DOXYCONF_BASIC_OPTIONS,"Opciones Basicas"));
	notebook->AddPage(CreateMorePanel(notebook), LANG(DOXYCONF_MORE_OPTIONS,"Mas Opciones"));
	notebook->AddPage(CreateExtraPanel(notebook), LANG(DOXYCONF_EXTRA_TAB,"Campos Adicionales"));
	
	mySizer->Add(notebook,sizers->Exp1);
	mySizer->Add(bottomSizer,sizers->Exp0);
	
	SetSizerAndFit(mySizer);
	
	SetFocus();
	Show();
	
}

mxDoxyDialog::~mxDoxyDialog() {
	
}

void mxDoxyDialog::OnOkButton(wxCommandEvent &evt) {
	if (!project) { return; }
	dox->destdir = destdir_ctrl->GetValue();
	dox->do_cpps = base_files_ctrl->GetSelection()==2 || base_files_ctrl->GetSelection()==3;
	dox->do_headers = base_files_ctrl->GetSelection()==1 || base_files_ctrl->GetSelection()==3;
	dox->base_path = base_path_ctrl->GetValue();
	dox->exclude_files = exclude_files_ctrl->GetValue();
	dox->extra_files = extra_files_ctrl->GetValue();
	dox->name = name_ctrl->GetValue();
	dox->version = version_ctrl->GetValue();
	dox->save = save_ctrl->GetValue();
	dox->lang = lang_ctrl->GetValue()==LANG(DOXYCONF_LANG_ES,"Espanol")?_T("Spanish"):_T("English");
	dox->html = html_ctrl->GetValue();
	dox->preprocess = preprocess_ctrl->GetValue();
	dox->extra_static = static_ctrl->GetValue();
	dox->extra_private = private_ctrl->GetValue();
	dox->use_in_quickhelp = use_in_quickhelp_ctrl->GetValue();
	dox->html_navtree = html_navtree_ctrl->GetValue();
	dox->latex = latex_ctrl->GetValue();
	dox->hideundocs = hideundocs_ctrl->GetValue();
	dox->extra_conf = extra_conf->GetValue();
	Close();
}

void mxDoxyDialog::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxDoxyDialog::OnClose(wxCloseEvent &event) {
	Destroy();
}




wxPanel *mxDoxyDialog::CreateGeneralPanel (wxNotebook *notebook) {
	
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	name_ctrl = utils->AddTextCtrl(sizer,panel,LANG(DOXYCONF_PROJECT_NAME,"Nombre del Proyecto"),dox->name);
	version_ctrl = utils->AddTextCtrl(sizer,panel,LANG(DOXYCONF_PROJECT_VER,"Version del proyecto"),dox->version);
	destdir_ctrl = utils->AddDirCtrl(sizer,panel,LANG(DOXYCONF_DEST_DIR,"Directorio destino"),dox->destdir,mxID_DOXYDIALOG_DEST);
	
	int idx;
	wxArrayString array1;
	array1.Add(LANG(DOXYCONF_FILES_NONE,"Ninguno"));
	array1.Add(wxString(LANG(DOXYCONF_FILES_CPP,"Fuentes"))<<_T(" (c,cpp,cxx...)"));
	array1.Add(wxString(LANG(DOXYCONF_FILES_HEADERS,"Cabeceras"))<<_T(" (h,hpp,hxx,...)"));
	array1.Add(LANG(DOXYCONF_FILES_H_AND_CPP,"Fuentes y Cabeceras"));
	if (dox) {
		if (dox->do_cpps && dox->do_headers) idx = 3;
		else if (dox->do_headers) idx = 2;
		else if (dox->do_cpps) idx = 1;
		else idx = 0;
	} else
		idx = 3;
	base_files_ctrl = utils->AddComboBox(sizer,panel,LANG(DOXYCONF_WICH_FILES,"Archivos a procesar"),array1,idx);
	
	extra_files_ctrl = utils->AddTextCtrl(sizer,panel,LANG(DOXYCONF_EXTRA_FILES,"Archivos adicionales a procesar"),dox->extra_files);
	exclude_files_ctrl = utils->AddTextCtrl(sizer,panel,LANG(DOXYCONF_FILES_TO_EXCLUDE,"Archivos a excluir al procesar"),dox->exclude_files);
	
	save_ctrl = utils->AddCheckBox(sizer,panel,LANG(DOXYCONF_SAVE_WITH_PROJECT,"Guardar con el proyecto"),dox->save);
	
	panel->SetSizer(sizer);
	return panel;
	
}


wxPanel *mxDoxyDialog::CreateExtraPanel (wxNotebook *notebook) {
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	extra_conf = new wxTextCtrl(panel,wxID_ANY,dox->extra_conf,wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE);
	utils->AddStaticText(sizer,panel,LANG(DOXYCONF_EXTRA_LABEL,"El texto de este campo se agregara sin cambios en el\n"
																 "Doxyfile. Puede utilizarlo para definir parametros\n"
																 "no contemplados en este cuadro de dialogo."));
	sizer->Add(extra_conf,sizers->BA10_Exp1);
	
	panel->SetSizer(sizer);
	return panel;
	
}

wxPanel *mxDoxyDialog::CreateMorePanel (wxNotebook *notebook) {
	
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	int idx;
	wxArrayString array2;
	array2.Add(LANG(DOXYCONF_LANG_ES,"Espanol"));
	array2.Add(LANG(DOXYCONF_LANG_EN,"Ingles"));
	if (!dox || dox->lang==_T("Spanish")) idx = 0; else idx=1;
	lang_ctrl = utils->AddComboBox(sizer,panel,LANG(DOXYCONF_LANG,"Idioma"),array2,idx);

	base_path_ctrl = utils->AddDirCtrl(sizer,panel,LANG(DOXYCONF_BASE_DIR,"Directorio base"),dox->base_path,mxID_DOXYDIALOG_BASE);
	
	preprocess_ctrl = utils->AddCheckBox(sizer,panel,LANG(DOXYCONF_ENABLE_PREPROC,"Habilitar preprocesado"),dox->preprocess);
	private_ctrl = utils->AddCheckBox(sizer,panel,LANG(DOXYCONF_EXTRA_PRIVATE,"Incluir metodos/atributos privados"),dox->extra_private);
	static_ctrl = utils->AddCheckBox(sizer,panel,LANG(DOXYCONF_EXTRA_STATIC,"Incluir funciones/variables static"),dox->extra_static);
	hideundocs_ctrl = utils->AddCheckBox(sizer,panel,LANG(DOXYCONF_ONLY_DOC_ENTITIES,"Extraer solo las entidades documentadas"),dox->hideundocs);
	latex_ctrl = utils->AddCheckBox(sizer,panel,LANG(DOXYCONF_GENERATE_LATEX,"Generar documentacion Latex"),dox->latex);
	html_ctrl = utils->AddCheckBox(sizer,panel,LANG(DOXYCONF_GENERATE_HTML,"Generar documentacion HTML"),dox->html);
	html_navtree_ctrl = utils->AddCheckBox(sizer,panel,LANG(DOXYCONF_SHOW_NAV_TREE,"Mostrar arbol de navegacion (para doc HTML)"),dox->html_navtree);
	html_searchengine_ctrl = utils->AddCheckBox(sizer,panel,LANG(DOXYCONF_INCLUDE_SEARCH_ENGINE,"Incluir motor de busquedas (para doc HTML, requiere PHP)"),dox->html_searchengine);
	use_in_quickhelp_ctrl = utils->AddCheckBox(sizer,panel,LANG(DOXYCONF_USE_IN_QUICKHELP,"Utilizar en ayuda rapida"),dox->use_in_quickhelp);
	
	panel->SetSizer(sizer);
	return panel;
	
}

void mxDoxyDialog::OnHelpButton(wxCommandEvent &event) {
	SHOW_HELP(_T("doxygen.html"));
}

void mxDoxyDialog::OnBaseDirButton(wxCommandEvent &event){
	wxDirDialog *dlg=new wxDirDialog(this,LANG(DOXYCONF_BASE_DIR,"Directorio base:"),DIR_PLUS_FILE(project->path,base_path_ctrl->GetValue()));
	if (wxID_OK==dlg->ShowModal()) {
		wxFileName fname(dlg->GetPath());
		fname.MakeRelativeTo(project->path);
		base_path_ctrl->SetValue(fname.GetFullPath());
	}
}

void mxDoxyDialog::OnDestDirButton(wxCommandEvent &event){
	wxDirDialog *dlg=new wxDirDialog(this,LANG(DOXYCONF_DEST_DIR,"Directorio destino:"),DIR_PLUS_FILE(project->path,destdir_ctrl->GetValue()));
	if (wxID_OK==dlg->ShowModal()) {
		wxFileName fname(dlg->GetPath());
		fname.MakeRelativeTo(project->path);
		destdir_ctrl->SetValue(fname.GetFullPath());
	}
}
