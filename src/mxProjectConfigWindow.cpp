#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/notebook.h>
#include <wx/panel.h>

#include "mxProjectConfigWindow.h"
#include "ConfigManager.h"
#include "mxUtils.h"
#include "mxHelpWindow.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include "mxMessageDialog.h"
#include "mxLongTextEditor.h"
#include "mxEnumerationEditor.h"
#include "mxSizers.h"
#include "mxExtraStepWindow.h"
#include "mxOutputView.h"
#include "Language.h"
#include "mxLibToBuildWindow.h"
#include "ProjectManager.h"
#include "mxTextDialog.h"
#include "Toolchain.h"
#include "mxToolchainOptions.h"
#include "mxThreeDotsUtils.h"
#include "mxCommonConfigControls.h"

int mxProjectConfigWindow::last_page_index=0;

BEGIN_EVENT_TABLE(mxProjectConfigWindow, wxDialog)
	EVT_BUTTON(wxID_OK,mxProjectConfigWindow::OnOkButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxProjectConfigWindow::OnHelpButton)
	EVT_BUTTON(wxID_CANCEL,mxProjectConfigWindow::OnCancelButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_MANIFEST_DIR,mxProjectConfigWindow::OnManifestDirButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_ICON_DIR,mxProjectConfigWindow::OnIconDirButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_TEMP_DIR,mxProjectConfigWindow::OnTempDirButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_ENV_VARS,mxProjectConfigWindow::OnEnvVarsButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_WORKING_DIR,mxProjectConfigWindow::OnWorkingDirButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_ADD,mxProjectConfigWindow::OnAddConfigButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_SELECT,mxProjectConfigWindow::OnSelectConfigButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_REMOVE,mxProjectConfigWindow::OnRemoveConfigButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_RENAME,mxProjectConfigWindow::OnRenameConfigButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_TOOLCHAIN_OPTIONS,mxProjectConfigWindow::OnToolchainOptionsButton)
	EVT_COMBOBOX(mxID_PROJECT_CONFIG_TOOLCHAIN_COMBO,mxProjectConfigWindow::OnComboToolchainChange)
	EVT_COMBOBOX(mxID_PROJECT_CONFIG_NAME,mxProjectConfigWindow::OnSelectConfigInCombo)
	EVT_COMBOBOX(mxID_PROJECT_CONFIG_EXEC_METHOD,mxProjectConfigWindow::OnComboExecutionScript)
	EVT_MENU(mxID_PROJECT_CONFIG_EXEC_SCRIPT,mxProjectConfigWindow::OnExecutionMethodButton)
	
	EVT_BUTTON(mxID_PROJECT_GENERAL_EXE_PATH,mxProjectConfigWindow::OnGeneralExePathButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_ARGS_BUTTON,mxProjectConfigWindow::OnGeneralArgsButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_COMPILE_MACROS_BUTTON,mxProjectConfigWindow::OnCompilingMacrosButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_COMPILE_EXTRA_BUTTON,mxProjectConfigWindow::OnCompilingExtraOptionsButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_COMPILE_DIRS_BUTTON,mxProjectConfigWindow::OnCompilingHeadersDirsButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_LINK_EXTRA_BUTTON,mxProjectConfigWindow::OnLinkingExtraOptionsButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_LINK_LIBS_BUTTON,mxProjectConfigWindow::OnLinkingLibrariesButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_LINK_DIRS_BUTTON,mxProjectConfigWindow::OnLinkingLibrariesDirsButton)
	
	EVT_BUTTON(mxID_PROJECT_CONFIG_STEPS_UP,mxProjectConfigWindow::OnStepsUp)
	EVT_BUTTON(mxID_PROJECT_CONFIG_STEPS_DOWN,mxProjectConfigWindow::OnStepsDown)
	EVT_BUTTON(mxID_PROJECT_CONFIG_STEPS_ADD,mxProjectConfigWindow::OnStepsAdd)
	EVT_BUTTON(mxID_PROJECT_CONFIG_STEPS_EDIT,mxProjectConfigWindow::OnStepsEdit)
	EVT_BUTTON(mxID_PROJECT_CONFIG_STEPS_RUN,mxProjectConfigWindow::OnStepsRun)
	EVT_BUTTON(mxID_PROJECT_CONFIG_STEPS_DEL,mxProjectConfigWindow::OnStepsDel)

	EVT_BUTTON(mxID_PROJECT_CONFIG_LIBS_ADD,mxProjectConfigWindow::OnLibsAdd)
	EVT_BUTTON(mxID_PROJECT_CONFIG_LIBS_EDIT,mxProjectConfigWindow::OnLibsEdit)
	EVT_BUTTON(mxID_PROJECT_CONFIG_LIBS_DEL,mxProjectConfigWindow::OnLibsDel)
	
	EVT_CHECKBOX(mxID_PROJECT_CONFIG_LIBS_DONT_EXE,mxProjectConfigWindow::OnLibsNoExe)
	
//	EVT_BUTTON(mxID_PROJECT_CONFIG_IMPORT_LIBS,mxProjectConfigWindow::OnImportLibsButton)
	
	EVT_CLOSE(mxProjectConfigWindow::OnClose)
END_EVENT_TABLE()

wxString mxExtraStepWindow::new_name;
	
mxProjectConfigWindow::mxProjectConfigWindow(wxWindow* parent) 
	: mxDialog(parent, LANG(PROJECTCONFIG_CAPTION,"Opciones de Compilación y Ejecución") ),
	  discard(true), configuration(project->active_configuration)
{
	CreateSizer sizer(this);
	
	wxArrayString values;
	for (int i=0;i<project->configurations_count;i++)
		values.Add(project->configurations[i]->name);
	sizer.BeginLine()
		.BeginCombo().Add(values).Select(configuration->name).Id(mxID_PROJECT_CONFIG_NAME).EndCombo(configuration_name)
		.BeginButton( LANG(PROJECTCONFIG_USE_PROFILE,"Utilizar") ).Id(mxID_PROJECT_CONFIG_SELECT).EndButton()
		.BeginButton( LANG(PROJECTCONFIG_NEW_PROFILE,"Nuevo") ).Id(mxID_PROJECT_CONFIG_ADD).EndButton()
		.BeginButton( LANG(PROJECTCONFIG_RENAME_PROFILE,"Renombrar") ).Id(mxID_PROJECT_CONFIG_RENAME).EndButton()
		.BeginButton( LANG(PROJECTCONFIG_DELETE_PROFILE,"Eliminar") ).Id(mxID_PROJECT_CONFIG_REMOVE).EndButton()
		.EndLine();
	
	// crear las pestañas con las opciones de la configuracion
	sizer.BeginNotebook()
		.AddPageIf(config->Help.show_extra_panels,this,&mxProjectConfigWindow::CreateQuickHelpPanel, LANG(PROJECTCONFIG_HELP,"Ayuda"))
		.AddPage(this,&mxProjectConfigWindow::CreateGeneralPanel, LANG(PROJECTCONFIG_GENERAL,"General"))
		.AddPage(this,&mxProjectConfigWindow::CreateCompilingPanel, LANG(PROJECTCONFIG_COMPILING,"Compilacion"))
		.AddPage(this,&mxProjectConfigWindow::CreateLinkingPanel, LANG(PROJECTCONFIG_LINKING,"Enlazado"))
		.AddPage(this,&mxProjectConfigWindow::CreateStepsPanel, LANG(PROJECTCONFIG_SEQUENCE,"Secuencia"))
		.AddPage(this,&mxProjectConfigWindow::CreateLibsPanel, LANG(PROJECTCONFIG_LIBRARIES,"Bibliotecas"))
		.EndNotebook(notebook);

	wx_noexe.EnableAll(!configuration->dont_generate_exe);
	wx_extern.EnableAll(!Toolchain::GetInfo(toolchains_combo->GetStringSelection()).IsExtern());
	wx_noscript.EnableAll(configuration->exec_method);
	
	sizer.BeginBottom().Help().Ok().Cancel().EndBottom(this).SetAndFit();
	notebook->SetSelection(last_page_index);
	SetFocus();
	Show();
}

wxPanel *mxProjectConfigWindow::CreateQuickHelpPanel (wxNotebook *notebook) {
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	wxHtmlWindow *html = new wxHtmlWindow(panel,wxID_ANY);
	sizer->Add(html,sizers->Exp1);
	html->LoadFile(DIR_PLUS_FILE(config->Help.guihelp_dir,wxString("proy_help_")<<config->Init.language_file<<".html"));
	panel->SetSizerAndFit(sizer);
	return panel;	
}

wxPanel *mxProjectConfigWindow::CreateLinkingPanel (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	sizer.BeginText( LANG(PROJECTCONFIG_LINKING_EXTRA_ARGS,"Parametros extra para el enlazado") )
		.Value(configuration->linking_extra).Button(mxID_PROJECT_CONFIG_LINK_EXTRA_BUTTON)
		.RegisterIn(wx_noexe).RegisterIn(wx_extern).EndText(linking_extra_options);
			
	sizer.BeginText( LANG(PROJECTCONFIG_LINKING_EXTRA_PATHS,"Directorios adicionales para buscar bibliotecas") )
		.Value(configuration->libraries_dirs).Button(mxID_PROJECT_CONFIG_LINK_DIRS_BUTTON)
		.RegisterIn(wx_noexe).RegisterIn(wx_extern).EndText(linking_libraries_dirs);
			
	sizer.BeginText( LANG(PROJECTCONFIG_LINKING_EXTRA_LIBS,"Bibliotecas a enlazar") )
		.Value(configuration->libraries).Button(mxID_PROJECT_CONFIG_LINK_LIBS_BUTTON)
		.RegisterIn(wx_noexe).RegisterIn(wx_extern).EndText(linking_libraries);

	sizer.BeginCombo( LANG(PROJECTCONFIG_DEBUG_INFO,"Información para depuración") )
		.Add(LANG(PROJECTCONFIG_DEBUG_INFO_KEEP,"Mantener en el binario"))
		.Add(LANG(PROJECTCONFIG_DEBUG_INFO_COPY,"Extraer a un archivo separado"))
		.Add(LANG(PROJECTCONFIG_DEBUG_INFO_STRIP,"Eliminar del binario"))
		.Select(configuration->strip_executable)
		.RegisterIn(wx_noexe).RegisterIn(wx_extern)
		.EndCombo(linking_strip_executable);
			
	
	sizer.BeginCheck( LANG(PROJECTCONFIG_LTO,"Habilitar optimizaciones durante el enlazado (LTO)") )
		.RegisterIn(wx_noexe).RegisterIn(wx_extern)
		.Value(configuration->enable_lto).EndCheck(compiling_enable_lto);

	sizer.BeginCheck( LANG(PROJECTCONFIG_LINKING_IS_CONSOLE_PROGRAM,"Es un programa de consola") )
		.RegisterIn(wx_noexe) _if_win32( .RegisterIn(wx_extern), )
		.Value(configuration->console_program).EndCheck(linking_console_program);
				
	sizer.BeginCheck( LANG(PROJECTCONFIG_LINKING_FORCE_RELINK,"Reenlazar obligatoriamente en la proxima compilacion/ejecucion") )
		.RegisterIn(wx_noexe).RegisterIn(wx_extern)
		.Value(project->force_relink).EndCheck(linking_force_relink);
			
	sizer.BeginText( LANG(PROJECTCONFIG_GENERAL_ICON_PATH,"Icono del ejecutable (solo Windows)") )
		.Value(configuration->icon_file).Button(mxID_PROJECT_CONFIG_ICON_DIR)
		.RegisterIn(wx_noexe).RegisterIn(wx_extern).EndText(linking_icon);
			
	sizer.BeginText( LANG(PROJECTCONFIG_GENERAL_MANIFEST_PATH,"Archivo manifest.xml (solo Windows)") )
		.Value(configuration->manifest_file).Button(mxID_PROJECT_CONFIG_MANIFEST_DIR)
		.RegisterIn(wx_noexe).RegisterIn(wx_extern).EndText(linking_manifest);
	
	sizer.SetAndFit();
	return sizer.GetPanel();
}

wxPanel *mxProjectConfigWindow::CreateGeneralPanel (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
		
	sizer.BeginText( LANG(PROJECTCONFIG_GENERAL_EXE_PATH,"Ubicacion del ejecutable") )
		.Value(configuration->output_file).Button(mxID_PROJECT_GENERAL_EXE_PATH)
		.RegisterIn(wx_noexe).EndText(general_output_file);
					
	sizer.BeginCombo( LANG(PROJECTCONFIG_GENERAL_EXEC_METHOD,"Mecanismo de ejecución") )
		.Add(LANG(PROJECTCONFIG_GENERAL_EXEC_METHOD_REGULAR,"Regular (se lanza directamente el ejecutable)"))
		.Add(LANG(PROJECTCONFIG_GENERAL_EXEC_METHOD_WRAPPER,"Mediante un wrapper (otro programa lanza al ejecutable)"))
		.Add(LANG(PROJECTCONFIG_GENERAL_EXEC_METHOD_INI,"Con inicialización (se ejecuta un script antes)"))
		.Add(LANG(PROJECTCONFIG_GENERAL_EXEC_METHOD_SCRIPT,"Solo script (el script deberá lanzar el ejecutable)"))
		.Select(configuration->exec_method).Id(mxID_PROJECT_CONFIG_EXEC_METHOD).EndCombo(general_exec_method);
					
	sizer.BeginText( LANG(PROJECTCONFIG_GENERAL_SCRIPT,"Script para ejecución/comando wrapper") )
		.Value(configuration->exec_script).Button(mxID_PROJECT_CONFIG_EXEC_SCRIPT)
		.RegisterIn(wx_noscript).EndText(general_exec_script);
					
	sizer.BeginText( LANG(PROJECTCONFIG_GENERAL_WORKDIR,"Directorio de trabajo") )
		.Value(configuration->working_folder).Button(mxID_PROJECT_CONFIG_WORKING_DIR)
		.RegisterIn(wx_noexe).EndText(general_working_folder);
	
	sizer.BeginText( LANG(PROJECTCONFIG_GENERAL_RUNNING_ARGS,"Argumentos para la ejecucion") )
		.Value(configuration->args).Button(mxID_PROJECT_CONFIG_ARGS_BUTTON)
		.RegisterIn(wx_noexe).EndText(general_args);
	
	sizer.BeginCheck( LANG(PROJECTCONFIG_GENERAL_ASK_ARGS,"Siempre pedir argumentos al ejecutar") )
		.Value(configuration->always_ask_args).RegisterIn(wx_noexe).EndCheck(general_always_ask_args);
	

	sizer.BeginCombo( LANG(PROJECTCONFIG_GENERAL_WAIT_KEY,"Esperar una tecla luego de la ejecucion") )
		.Add(LANG(PROJECTCONFIG_GENERAL_WAIT_KEY_NEVER,"Nunca"))
		.Add(LANG(PROJECTCONFIG_GENERAL_WAIT_KEY_ERROR,"En caso de error"))
		.Add(LANG(PROJECTCONFIG_GENERAL_WAIT_KEY_ALWAYS,"Siempre"))
		.Select(configuration->wait_for_key).RegisterIn(wx_noexe)
		.EndCombo(general_wait_for_key);
					
	sizer.BeginText( LANG(PROJECTCONFIG_GENERAL_ENV_VARS,"Variables de entorno") )
		.Value(configuration->env_vars).Button(mxID_PROJECT_CONFIG_ENV_VARS)
		.RegisterIn(wx_noexe).EndText(general_env_vars);
					
	sizer.SetAndFit();
	return sizer.GetPanel();
}


wxPanel *mxProjectConfigWindow::CreateCompilingPanel (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
		
	sizer.BeginText( LANG(PROJECTCONFIG_COMPILING_EXTRA_ARGS,"Parametros extra para la compilacion") )
		.Value(configuration->compiling_extra).Button(mxID_PROJECT_CONFIG_COMPILE_EXTRA_BUTTON)
		.RegisterIn(wx_extern).EndText(compiling_extra_options);
						
		
	sizer.BeginText( LANG(PROJECTCONFIG_COMPILING_MACROS,"Macros a definir") )
		.Value(configuration->macros).Button(mxID_PROJECT_CONFIG_COMPILE_MACROS_BUTTON)
		.RegisterIn(wx_extern).EndText(compiling_macros);
							
		
	sizer.BeginText( LANG(PROJECTCONFIG_COMPILING_EXTRA_PATHS,"Directorios adicionales para buscar cabeceras") )
		.Value(configuration->headers_dirs).Button(mxID_PROJECT_CONFIG_COMPILE_DIRS_BUTTON)
		.RegisterIn(wx_extern).EndText(compiling_headers_dirs);
							

	sizer.BeginLine()
		.BeginLabel( LANG(PROJECTCONFIG_COMPILING_STD,"Estandar:") ).RegisterIn(wx_extern).EndLabel()
		.Space(15)
		.BeginCombo( "C" )
			.Add(LANG(PROJECTCONFIG_COMPILING_STD_DEFAULT,"<predeterminado>"))
			.Add("c90").Add("gnu90").Add("c99").Add("gnu99").Add("c11").Add("gnu11")
			.Select(configuration->std_c).RegisterIn(wx_extern)
			.EndCombo(compiling_std_c)
		.Space(15)
		.BeginCombo( "C++" )
			.Add(LANG(PROJECTCONFIG_COMPILING_STD_DEFAULT,"<predeterminado>"))
			.Add("c++98").Add("gnu++98").Add("c++03").Add("gnu++03")
			.Add("c++11").Add("gnu++11").Add("c++14").Add("gnu++14")  
			.Add("c++1z").Add("gnu++1z")
			.Select(configuration->std_cpp).RegisterIn(wx_extern)
			.EndCombo(compiling_std_cpp)
		.Space(15)
		.BeginCheck(LANG(PROJECTCONFIG_COMPILING_ONLY_ANSI,"estricto"))
			.Value(configuration->pedantic_errors).RegisterIn(wx_extern)
			.EndCheck(compiling_pedantic)
		.EndLine();
		
	sizer.BeginLine()
		.BeginCombo(LANG(PROJECTCONFIG_COMPILING_WARNINGS,"Nivel de advertencias"))
			.Add(LANG(PROJECTCONFIG_COMPILING_WARNINGS_NONE,"Ninguna"))
			.Add(LANG(PROJECTCONFIG_COMPILING_WARNINGS_DEFAULT,"Predeterminadas"))
			.Add(LANG(PROJECTCONFIG_COMPILING_WARNINGS_ALL,"Todas"))
			.Add(LANG(PROJECTCONFIG_COMPILING_WARNINGS_ALL,"Extra"))
			.Select(configuration->warnings_level).RegisterIn(wx_extern)
			.EndCombo(compiling_warnings_level)
		.Space(15)
		.BeginCheck(LANG(PROJECTCONFIG_WARNINGS_AS_ERRORS,"como errores"))
			.Value(configuration->warnings_as_errors).RegisterIn(wx_extern)
			.EndCheck(compiling_warnings_as_errors)
		.EndLine();
							
	sizer.BeginCombo(LANG(PROJECTCONFIG_COMPILING_DEBUG,"Informacion de depuracion"))
		.Add(LANG(PROJECTCONFIG_COMPILING_DEBUG_NONE,"Ninguna"))
		.Add(LANG(PROJECTCONFIG_COMPILING_DEBUG_LEVEL_1,"Nivel 1"))
		.Add(LANG(PROJECTCONFIG_COMPILING_DEBUG_LEVEL_2,"Nivel 2"))
		.Add(LANG(PROJECTCONFIG_COMPILING_DEBUG_LEVEL_3,"Nivel 3"))
		.Select(configuration->debug_level).RegisterIn(wx_extern)
		.EndCombo(compiling_debug_level);
							
	sizer.BeginCombo(LANG(PROJECTCONFIG_COMPILING_OPTIM,"Nivel de optimizacion"))
		.Add(LANG(PROJECTCONFIG_COMPILING_OPTIM_NONE,"Ninguna"))
		.Add(LANG(PROJECTCONFIG_COMPILING_OPTIM_LEVEL_1,"Nivel 1"))
		.Add(LANG(PROJECTCONFIG_COMPILING_OPTIM_LEVEL_2,"Nivel 2"))
		.Add(LANG(PROJECTCONFIG_COMPILING_OPTIM_LEVEL_3,"Nivel 3"))
		.Add(LANG(PROJECTCONFIG_COMPILING_OPTIM_SIZE,"Reducir Tamaño"))
		.Add(LANG(PROJECTCONFIG_COMPILING_OPTIM_DEBUG,"Depuración"))
		.Add(LANG(PROJECTCONFIG_COMPILING_OPTIM_FAST,"Velocidad"))
		.Select(configuration->optimization_level).RegisterIn(wx_extern)
		.EndCombo(compiling_optimization_level);
							
	sizer.BeginText( LANG(PROJECTCONFIG_GENERAL_TEMP_FOLDER,"Directorio para archivos temporales e intermedios") )
		.Value(configuration->temp_folder).Button(mxID_PROJECT_CONFIG_TEMP_DIR)
		.RegisterIn(wx_extern).EndText(compiling_temp_folder);
			
	sizer.SetAndFit();
	return sizer.GetPanel();
}

void mxProjectConfigWindow::OnCancelButton(wxCommandEvent &event){
	Close();
}

void mxProjectConfigWindow::OnClose(wxCloseEvent &event){
	if (discard) DiscardChanges();
	last_page_index=notebook->GetSelection();
	for (int i=0;i<project->configurations_count;i++)
		if (project->configurations[i]->bakup) {
			delete project->configurations[i]->bakup;
			project->configurations[i]->bakup=nullptr;
		}
	Destroy();
}

void mxProjectConfigWindow::OnIconDirButton(wxCommandEvent &event){
	wxFileDialog dlg(this,"Icono:",DIR_PLUS_FILE(project->path,linking_icon->GetValue()));
	dlg.SetWildcard("Iconos|*.ico;*.ICO");
	if (wxID_OK==dlg.ShowModal()) 
		linking_icon->SetValue(mxUT::Relativize(dlg.GetPath(),project->path));
}

void mxProjectConfigWindow::OnManifestDirButton(wxCommandEvent &event){
	wxFileDialog dlg(this,"Manifest:",DIR_PLUS_FILE(project->path,linking_manifest->GetValue()));
	dlg.SetWildcard("XMLs|*.xml;*.XML");
	if (wxID_OK==dlg.ShowModal()) 
		linking_manifest->SetValue(mxUT::Relativize(dlg.GetPath(),project->path));
}

void mxProjectConfigWindow::OnTempDirButton(wxCommandEvent &event){
	wxDirDialog dlg(this,"Carpeta para archivos temporales e intermedios:",DIR_PLUS_FILE(project->path,compiling_temp_folder->GetValue()));
	if (wxID_OK==dlg.ShowModal())
		compiling_temp_folder->SetValue(mxUT::Relativize(dlg.GetPath(),project->path));
}

void mxProjectConfigWindow::OnEnvVarsButton(wxCommandEvent &event){
	new mxEnumerationEditor(this,"Parametros extra para el compilador",general_env_vars,false);
}

void mxProjectConfigWindow::OnWorkingDirButton(wxCommandEvent &event) {
	mxThreeDotsUtils::ReplaceAllWithDirectory(this,general_working_folder,project->path,LANG(PROJECTCONFIG_GENERAL_WORKDIR,"Directorio de trabajo"));
}


void mxProjectConfigWindow::OnAddConfigButton(wxCommandEvent &event) {
	// ask for a name for the new configuration
	wxString new_name = mxGetTextFromUser(LANG(PROJECTCONFIG_PROFILE_NAME,"Nombre:"), LANG(PROJECTCONFIG_ADD_PROFILE,"Agregar configuracion"), "", this);
	if (new_name.IsEmpty()) return;
	// check if the name is unique
	for (int i=0;i<project->configurations_count;i++)
		if (project->configurations[i]->name==new_name) {
			mxMessageDialog(this,LANG(PROJECTCONFIG_PROFILE_NAME_REPEATED,"Ya existe otra configuracion con ese nombre"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
			return;
		}
	// ask for an old configuration to copy settings from
	wxArrayString copy_from_arr; copy_from_arr.Add(LANG(PROJECTCONFIG_PROFILE_NULL,"<ninguna>"));
	for(int i=0;i<project->configurations_count;i++) copy_from_arr.Add(project->configurations[i]->name);
	wxString copy_from_name = wxGetSingleChoice(LANG(PROJECTCONFIG_COPY_PROFILE_FROM,"Copiar de:"),LANG(PROJECTCONFIG_ADD_PROFILE,"Agregar configuracion:"),copy_from_arr);
	if (copy_from_name.IsEmpty()) return;
	int copy_from_idx = copy_from_arr.Index(copy_from_name);
	// create the new configuration
	project_configuration *new_conf;
	if (copy_from_idx==0) new_conf = new project_configuration(wxFileName(project->filename).GetName(),new_name);
	else new_conf = new project_configuration(new_name,project->configurations[copy_from_idx-1]);
	configuration = project->configurations[project->configurations_count++] = new_conf;
	configuration_name->Append(new_name);
	// select the new configuration
	for (unsigned int i=0;i<configuration_name->GetCount();i++) {
		if (configuration_name->GetString(i)==new_name) {
			configuration_name->SetSelection(i);
			configuration_name->SetValue(new_name);
			LoadValues();
		}
	}
}

void mxProjectConfigWindow::OnSelectConfigButton(wxCommandEvent &event) {
	project->SetActiveConfiguration(configuration);
}

void mxProjectConfigWindow::OnRemoveConfigButton(wxCommandEvent &event) {
	if (project->configurations_count==1) 
		mxMessageDialog(this,LANG(PROJECTCONFIG_CANT_DELETE_LAST_PROFILE,"No puedes eliminar la unica configuracion existente"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
	else if (mxMD_YES==mxMessageDialog(this,wxString(LANG(PROJECTCONFIG_CONFIRM_DELETE_PRE,"Seguro que desea eliminar la configuracion "))+configuration->name,LANG(GENERAL_WARNING,"Advertencia"),mxMD_YES_NO|mxMD_WARNING).ShowModal()) {
		int i=0;
		// buscar la que hay que eliminar
		while (project->configurations[i]!=configuration)
			i++;
		// quitarla de memoria y el la lista del proyecto
		if (project->configurations[i]->bakup)
			delete project->configurations[i]->bakup;
		delete project->configurations[i];
		project->configurations_count--;
		int sel=i;
		while (i<project->configurations_count) {
			project->configurations[i]=project->configurations[i+1];
			i++;
		}
		// si es la activa, cambiar la activa
		if (configuration==project->active_configuration) {
			if (sel==0)
				project->SetActiveConfiguration(configuration=project->configurations[0]);
			else
				project->SetActiveConfiguration(configuration=project->configurations[sel-1]);
		} 
		if (sel==0)
			configuration = project->configurations[0];
		else
			configuration = project->configurations[sel-1];
		// rehacer la lista de la ventana
		configuration_name->Clear();
		for (int i=0;i<project->configurations_count;i++)
			configuration_name->Append(project->configurations[i]->name);
		for (unsigned int i=0;i<configuration_name->GetCount();i++) {
			if (configuration_name->GetString(i)==configuration->name) {
				configuration_name->SetSelection(i);
				configuration_name->SetValue(configuration->name);
			}
		}
		LoadValues();
	}
}

void mxProjectConfigWindow::OnRenameConfigButton(wxCommandEvent &event) {
	wxString res = mxGetTextFromUser(LANG(PROJECTCONFIG_PROFILE_NEW_NAME,"Nuevo nombre:"), LANG(PROJECTCONFIG_RENAME_PROFILE,"Renombrar configuracion:") , configuration->name, this);
	if (res!="") {
		for (int i=0;i<project->configurations_count;i++)
			if (project->configurations[i]!=configuration && project->configurations[i]->name==res) {
				mxMessageDialog(this,LANG(PROJECTCONFIG_PROFILE_NAME_REPEATED,"Ya existe otra configuracion con ese nombre"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
				return;
			}
		configuration->name=res;
		if (configuration->bakup) configuration->bakup->name=res;
		configuration_name->SetString(configuration_name->GetSelection(),res);
		configuration_name->SetValue(res);
	}
}

/**
* Copia los valores de la configuración actual a los controles del dialogo
**/
void mxProjectConfigWindow::LoadValues() {

	if (configuration->bakup) configuration->bakup = new project_configuration(*configuration);
	
	linking_extra_options->SetValue(configuration->linking_extra);
	linking_libraries_dirs->SetValue(configuration->libraries_dirs );
	linking_libraries->SetValue(configuration->libraries);
	linking_strip_executable->SetSelection(configuration->strip_executable);
	linking_console_program->SetValue(configuration->console_program);

	general_output_file->SetValue(configuration->output_file);
	general_working_folder->SetValue(configuration->working_folder);
	general_always_ask_args->SetValue(configuration->always_ask_args);
	general_args->SetValue(configuration->args);
	general_exec_script->SetValue(configuration->exec_script);
	general_exec_method->SetSelection(configuration->exec_method);
	general_wait_for_key->SetSelection(configuration->wait_for_key);
	general_env_vars->SetValue(configuration->env_vars);
	linking_icon->SetValue(configuration->icon_file);
	linking_manifest->SetValue(configuration->manifest_file);
	
	compiling_temp_folder->SetValue(configuration->temp_folder);
	compiling_macros->SetValue(configuration->macros);
	compiling_extra_options->SetValue(configuration->compiling_extra);
	compiling_headers_dirs->SetValue(configuration->headers_dirs);
	compiling_pedantic->SetValue(configuration->pedantic_errors);
	if (configuration->std_c.Len() && compiling_std_c->FindString(configuration->std_c)!=wxNOT_FOUND)
		compiling_std_c->SetValue(configuration->std_c); else compiling_std_c->SetSelection(0);
	if (configuration->std_cpp.Len() && compiling_std_cpp->FindString(configuration->std_cpp)!=wxNOT_FOUND)
		compiling_std_cpp->SetValue(configuration->std_cpp); else compiling_std_cpp->SetSelection(0);
	compiling_warnings_level->SetSelection(configuration->warnings_level);
	compiling_warnings_as_errors->SetValue(configuration->warnings_as_errors);
	compiling_debug_level->SetSelection(configuration->debug_level);
	compiling_optimization_level->SetSelection(configuration->optimization_level);
	compiling_enable_lto->SetValue(configuration->enable_lto);
	
	ReloadSteps();
	ReloadLibs();
	libtobuild_noexec->SetValue(configuration->dont_generate_exe);
	// set toolchains combo value
	toolchains_combo->SetSelection(0);
	if (configuration->toolchain.Len()) 
		for(unsigned int i=1;i<toolchains_combo->GetCount();i++) 
			if (toolchains_combo->GetString(i)==configuration->toolchain) 
			{ toolchains_combo->SetSelection(i); break; }
	
	wx_noexe.EnableAll(!configuration->dont_generate_exe);
	wx_noscript.EnableAll(configuration->exec_method);
}

void mxProjectConfigWindow::OnSelectConfigInCombo(wxCommandEvent &event) {
	if (SaveValues()) { // guardar los cambios de la conf actual en vista anterior
		configuration=project->GetConfig(configuration_name->GetValue());
		LoadValues(); // carga los valores de la conf nueva
	} else {
		configuration_name->SetValue(configuration->name);
	}
}

void mxProjectConfigWindow::OnOkButton(wxCommandEvent &event){
	if (!project) {Close(); return;}
//	if (project->project_name!=project_name->GetValue()) {
//		project->project_name=project_name->GetValue();
//		main_window->SetOpenedFileName(project->project_name);
//	}
	project->force_relink=linking_force_relink->GetValue();
	discard=false; // evitar que al cerrar revierta los cambios
	if (!SaveValues()) return; // guardar los cambios de la conf actual en vista
	if (project->active_configuration != configuration && mxMD_YES==mxMessageDialog(this,wxString()<<LANG1(PROJECTCONFIG_ASK_FOR_SETTING_CURRENT_PROFILE,"Desea establecer la configuracion \"<{1}>\" como la configuracion a utilizar?",configuration->name),LANG(PROJECTCONFIG_CURRENT_PROFILE,"Configuracion activa"),mxMD_YES_NO|mxMD_QUESTION).ShowModal() )
		project->SetActiveConfiguration(configuration);
	else
		main_window->SetToolchainMode(Toolchain::SelectToolchain().IsExtern());
	Close();
}

/**
* Guarda los valores de los controles del dialogo en la configuración actual
**/
bool mxProjectConfigWindow::SaveValues() {
	
	if (!mxUT::LeftTrim(general_output_file->GetValue()).Len()) {
		mxMessageDialog(this,LANG(PROJECTCONFIG_EXE_NAME_MISSING,"No se ha definido ubicacion (primer campo en \"General\") para el ejecutable.\n Este campo no puede quedar vacio."),LANG(GENERAL_ERROR,"Error"),mxMD_WARNING|mxMD_OK).ShowModal();
		return false;
	}
	
	project->modified=true;
	
	configuration->linking_extra=linking_extra_options->GetValue();
	configuration->libraries_dirs=linking_libraries_dirs->GetValue();
	configuration->libraries=linking_libraries->GetValue();
	if (configuration->strip_executable!=linking_strip_executable->GetSelection()) {
		configuration->strip_executable=linking_strip_executable->GetSelection();
		project->force_relink=true;
	}
#ifdef __WIN32__
	if (configuration->console_program!=linking_console_program->GetValue()) {
		configuration->console_program=linking_console_program->GetValue();
		project->force_relink=true;
	}
#else
	configuration->console_program=linking_console_program->GetValue();
#endif
	configuration->output_file=general_output_file->GetValue();
	configuration->working_folder=general_working_folder->GetValue();
	configuration->always_ask_args=general_always_ask_args->GetValue();
	configuration->args=general_args->GetValue();
	configuration->exec_script=general_exec_script->GetValue();
	configuration->exec_method=general_exec_method->GetSelection();
	configuration->wait_for_key=general_wait_for_key->GetSelection();
	configuration->icon_file=linking_icon->GetValue();
	configuration->manifest_file=linking_manifest->GetValue();
	configuration->env_vars=general_env_vars->GetValue();
	
	configuration->temp_folder=compiling_temp_folder->GetValue();
	configuration->macros=compiling_macros->GetValue();
	configuration->compiling_extra=compiling_extra_options->GetValue();
	configuration->headers_dirs=compiling_headers_dirs->GetValue();
	configuration->pedantic_errors=compiling_pedantic->GetValue();
	configuration->std_c=compiling_std_c->GetValue();
	if (compiling_std_c->GetSelection()==0) configuration->std_c="";
	configuration->std_cpp=compiling_std_cpp->GetValue();
	if (compiling_std_cpp->GetSelection()==0) configuration->std_cpp="";
	configuration->warnings_level=compiling_warnings_level->GetSelection();
	configuration->warnings_as_errors=compiling_warnings_as_errors->GetValue();
	configuration->debug_level=compiling_debug_level->GetSelection();
	configuration->optimization_level=compiling_optimization_level->GetSelection();
	configuration->enable_lto=compiling_enable_lto->GetValue();
	
	configuration->dont_generate_exe = libtobuild_noexec->GetValue();
	
	configuration->toolchain=toolchains_combo->GetStringSelection();
	if (configuration->toolchain=="<default>") configuration->toolchain="";

	return true;
	
}

void mxProjectConfigWindow::OnHelpButton(wxCommandEvent &event){
	mxHelpWindow::ShowHelp("project_config.html");
}

void mxProjectConfigWindow::OnCompilingExtraOptionsButton(wxCommandEvent &evt) {
	CommonPopup(compiling_extra_options).CommaSplit(false).BasePath(project->path)
		.Caption( LANG(PROJECTCONFIG_COMPILING_EXTRA_ARGS,"Parametros extra para la compilacion") )
		.AddEditAsText().AddEditAsList().AddFilename().AddPath().AddMinGWDir().Run(this);
}

void mxProjectConfigWindow::OnCompilingHeadersDirsButton(wxCommandEvent &evt) {
	CommonPopup(compiling_headers_dirs).CommaSplit(true).BasePath(project->path)
		.Caption( LANG(PROJECTCONFIG_COMPILING_EXTRA_PATHS,"Directorios adicionales para buscar cabeceras") )
		.AddEditAsText().AddEditAsList().AddPath().AddMinGWDir().Run(this);
}

void mxProjectConfigWindow::OnLinkingExtraOptionsButton(wxCommandEvent &evt) {
	CommonPopup(linking_extra_options).CommaSplit(false).BasePath(project->path)
		.Caption( LANG(PROJECTCONFIG_LINKING_EXTRA_ARGS,"Parametros extra para el enlazado") )
		.AddEditAsText().AddEditAsList().AddFilename().AddPath().AddMinGWDir().Run(this);
}

void mxProjectConfigWindow::OnLinkingLibrariesDirsButton(wxCommandEvent &evt) {
	CommonPopup(linking_libraries_dirs).CommaSplit(true).BasePath(project->path)
		.Caption( LANG(PROJECTCONFIG_LINKING_EXTRA_PATHS,"Directorios adicionales para buscar bibliotecas") )
		.AddEditAsText().AddEditAsList().AddPath().AddMinGWDir().Run(this);
}

void mxProjectConfigWindow::OnLinkingLibrariesButton(wxCommandEvent &evt) {
	CommonPopup(linking_libraries).CommaSplit(true)
		.Caption( LANG(PROJECTCONFIG_LINKING_EXTRA_LIBS,"Bibliotecas a enlazar") )
		.AddEditAsText().AddEditAsList().Run(this);
}

void mxProjectConfigWindow::OnGeneralArgsButton(wxCommandEvent &evt) {
	CommonPopup(general_args).CommaSplit(false).BasePath(DIR_PLUS_FILE(project->path,general_working_folder->GetValue()))
		.Caption( LANG(PROJECTCONFIG_GENERAL_RUNNING_ARGS,"Argumentos para la ejecucion") )
		.AddEditAsText().AddEditAsList().AddFilename().AddPath().Run(this);
}

void mxProjectConfigWindow::OnGeneralExePathButton(wxCommandEvent &evt) {
	wxString sel = general_output_file->GetStringSelection();
	wxFileDialog dlg(this,"Ubicacion del ejecutable:",DIR_PLUS_FILE(project->path,general_output_file->GetValue()));
	if (wxID_OK==dlg.ShowModal())
		general_output_file->SetValue(mxUT::Relativize(dlg.GetPath(),project->path));
}

wxPanel *mxProjectConfigWindow::CreateStepsPanel (wxNotebook *notebook) {
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *sizer_h= new wxBoxSizer(wxHORIZONTAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	wxArrayString toolchain_cmb;
	toolchain_cmb.Add("<default>");
	Toolchain::GetNames(toolchain_cmb,false);
	
	wxBoxSizer *sizer_i = new wxBoxSizer(wxHORIZONTAL);
	sizer_i->Add(new wxStaticText(panel,wxID_ANY,"Toolchain:"),sizers->BA5_Center);
	sizer_i->Add(toolchains_combo=new wxComboBox(panel,mxID_PROJECT_CONFIG_TOOLCHAIN_COMBO,"<default>",wxDefaultPosition,wxDefaultSize,toolchain_cmb,wxCB_READONLY),wxSizerFlags().Proportion(1).Center());
	wxButton *button_toolchain = new wxButton(panel,mxID_PROJECT_CONFIG_TOOLCHAIN_OPTIONS,"Opciones...");
	sizer_i->Add(button_toolchain,sizers->BA10);
	sizer->Add(sizer_i,sizers->BB5_Exp0);
	// set toolchains combo value
	if (configuration->toolchain.Len()) 
		for(unsigned int i=1;i<toolchains_combo->GetCount();i++) 
			if (toolchains_combo->GetString(i)==configuration->toolchain) 
				{ toolchains_combo->SetSelection(i); break; }

	wxStaticText *steps_label=new wxStaticText(panel,wxID_ANY,LANG(PROJECTCONFIG_STEPS_COMPILATION_SEQUENCE,"Secuencia de compilacion:"));
	sizer->Add(steps_label,sizers->BLRT5);
	wx_extern.Add(steps_label);
	
	steps_list = new wxListBox(panel,wxID_ANY);
	ReloadSteps();
	sizer_h->Add(steps_list,sizers->BA5_Exp1);
	wx_extern.Add(steps_list);
	
	wxBoxSizer *buttons = new wxBoxSizer(wxVERTICAL);
	wxButton *button_add = new wxButton(panel,mxID_PROJECT_CONFIG_STEPS_ADD,LANG(PROJECTCONFIG_STEPS_ADD,"Agregar"));
	buttons->Add(button_add,sizers->BLRT5); wx_extern.Add(button_add);
	wxButton *button_edit = new wxButton(panel,mxID_PROJECT_CONFIG_STEPS_EDIT,LANG(PROJECTCONFIG_STEPS_EDIT,"Editar"));
	buttons->Add(button_edit,sizers->BLRT5); wx_extern.Add(button_edit);
	wxButton *button_run = new wxButton(panel,mxID_PROJECT_CONFIG_STEPS_RUN,LANG(PROJECTCONFIG_STEPS_RUN,"Ejecutar"));
	buttons->Add(button_run,sizers->BLRT5); wx_extern.Add(button_run);
	wxButton *button_delete = new wxButton(panel,mxID_PROJECT_CONFIG_STEPS_DEL,LANG(PROJECTCONFIG_STEPS_DELETE,"Quitar"));
	buttons->Add(button_delete,sizers->BLRT5); wx_extern.Add(button_delete);
	wxButton *button_up = new wxButton(panel,mxID_PROJECT_CONFIG_STEPS_UP,LANG(PROJECTCONFIG_STEPS_MOVE_UP,"Subir"));
	buttons->Add(button_up,sizers->BLRT5); wx_extern.Add(button_up);
	wxButton *button_down = new wxButton(panel,mxID_PROJECT_CONFIG_STEPS_DOWN,LANG(PROJECTCONFIG_STEPS_MOVE_DOWN,"Bajar"));
	buttons->Add(button_down,sizers->BA5); wx_extern.Add(button_down);
	sizer_h->Add(buttons,sizers->BA5_Center);
	sizer->Add(sizer_h,sizers->BA5_Exp1);
	

	wxStaticText *st_warn1 = new wxStaticText(panel,wxID_ANY,LANG(PROJECTCONFIG_STEPS_WARNING_APPLY_NOW_LINE_1,"Nota: estos cambios se aplican inmediatamente,"));
	sizer->Add(st_warn1,sizers->Center); wx_extern.Add(st_warn1);
	wxStaticText *st_warn2 = new wxStaticText(panel,wxID_ANY,LANG(PROJECTCONFIG_STEPS_WARNING_APPLY_NOW_LINE_2,"aunque luego seleccione cancelar"));
	sizer->Add(st_warn2,sizers->Center); wx_extern.Add(st_warn2);
	
	panel->SetSizerAndFit(sizer);
	
	return panel;
}

wxPanel *mxProjectConfigWindow::CreateLibsPanel (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook); wxPanel *panel = sizer.GetPanel();
	

//#warning LA FUNCIONALIDAD DEL BOTON PARA IMPORTAR BIBLIOTECAS NO ESTA IMPLEMENTADA
//	wxBoxSizer *sizer_i = new wxBoxSizer(wxHORIZONTAL);
//	sizer_i->Add(new wxStaticText(panel,mxID_PROJECT_CONFIG_IMPORT_LIBS,"Bibliotecas a utilizar:"),sizers->BA5_Center);
//	sizer_i->Add(new wxButton(panel,wxID_ANY," Importar configuración desde plantilla... "),sizers->BA10);
//	sizer->Add(sizer_i,sizers->BB5_Exp0);
	
	sizer.BeginLabel( LANG(PROJECTCONFIG_LIBS_TO_GENERATE,"Bibliotecas a generar*") ).RegisterIn(wx_extern).EndLabel();
	
	libtobuild_list = new wxListBox(panel,wxID_ANY);
	wx_extern.Add(libtobuild_list);
	
	ReuseSizer sizer_h (panel,new wxBoxSizer(wxHORIZONTAL));
	sizer_h.Add(libtobuild_list,sizers->BA5_Exp1);
	
	CreateSizer sizer_buttons (panel);
	sizer_buttons.BeginButton( LANG(PROJECTCONFIG_LIBS_ADD,"Agregar") )
		.Id(mxID_PROJECT_CONFIG_LIBS_ADD).RegisterIn(wx_extern).Expand().EndButton();
	sizer_buttons.BeginButton( LANG(PROJECTCONFIG_LIBS_EDIT,"Editar") )
		.Id(mxID_PROJECT_CONFIG_LIBS_EDIT).RegisterIn(wx_extern).Expand().EndButton();
	sizer_buttons.BeginButton( LANG(PROJECTCONFIG_LIBS_DELETE,"Quitar") )
		.Id(mxID_PROJECT_CONFIG_LIBS_DEL).RegisterIn(wx_extern).Expand().EndButton();
	sizer_h.Add(sizer_buttons,sizers->BA5_Center);
	
	sizer.Add(sizer_h,sizers->BA5_Exp1);
	
	sizer.BeginCheck( LANG(PROJECTCONFIG_LIBS_NOEXEC,"Generar solo bibliotecas (no generar ejecutable)") )
		.Value(configuration->dont_generate_exe).Id(mxID_PROJECT_CONFIG_LIBS_DONT_EXE)
		.RegisterIn(wx_extern).EndCheck(libtobuild_noexec);
	
	sizer.BeginLabel( LANG(PROJECTCONFIG_STEPS_WARNING_APPLY_NOW,"* estos cambios se aplican inmediatamente,\n"
						                                               "aunque luego seleccione cancelar") )
					 .Center().EndLabel();
	
	ReloadLibs();
	sizer.Set();
	return sizer.GetPanel();
}

void mxProjectConfigWindow::OnStepsUp(wxCommandEvent &evt) {
	bool orig_dir=true;
	unsigned int i = steps_list->GetSelection();
	wxString sel = steps_list->GetString(i);
	compile_extra_step *step = project->GetExtraStep(configuration,steps_list->GetString(i));
	while (!step && i>0) {
		orig_dir=false;
		step = project->GetExtraStep(configuration,steps_list->GetString(--i));
	}
	if (step) {
		project->MoveExtraSteps(configuration,step,orig_dir?-1:1);
		ReloadSteps(sel);
	}
}

void mxProjectConfigWindow::OnStepsDown(wxCommandEvent &evt) {
	bool orig_dir=true;
	unsigned int i = steps_list->GetSelection();
	wxString sel = steps_list->GetString(i);
	compile_extra_step *step = project->GetExtraStep(configuration,steps_list->GetString(i));
	while (!step && i<steps_list->GetCount()-1) {
		orig_dir=false;
		step = project->GetExtraStep(configuration,steps_list->GetString(++i));
	}
	if (step) {
		project->MoveExtraSteps(configuration,step,orig_dir?1:-1);
		ReloadSteps(sel);
	}
}

void mxProjectConfigWindow::OnStepsAdd(wxCommandEvent &evt) {
	new mxExtraStepWindow(this,configuration,nullptr);
	if (mxExtraStepWindow::new_name.Len())
		ReloadSteps(mxExtraStepWindow::new_name);
}

void mxProjectConfigWindow::OnStepsRun(wxCommandEvent &evt) {
	compile_extra_step *step = project->GetExtraStep(configuration,steps_list->GetString(steps_list->GetSelection()));
	if (!step) return;
	wxString temp_folder_short = configuration->temp_folder;
	wxString command = project->GetCustomStepCommand(step);
	if (step->hide_window) {
		mxOutputView *outwin= new mxOutputView(step->name,mxOV_EXTRA_NULL,"","",mxVO_NULL,"");
		outwin->Launch(project->path,command);
	} else {
		mxUT::Execute(project->path,command,wxEXEC_ASYNC|wxEXEC_NOHIDE);
	}
}

void mxProjectConfigWindow::OnStepsDel(wxCommandEvent &evt) {
	int sel=steps_list->GetSelection(), count=steps_list->GetCount();
	wxString sname = steps_list->GetString(sel), snext;
	if (sel<count-1) snext=steps_list->GetString(sel+1);
	else if (sel>0) snext=steps_list->GetString(sel-1);
	if (mxMD_NO==mxMessageDialog(this,wxString(LANG(PROJECTCONFIG_ASK_DELETE_STEP,"Eliminar el paso: "))<<sname,LANG(GENERAL_CONFIRM,"Confirmacion"),(mxMD_YES_NO|mxMD_WARNING)).ShowModal())
		return;
	if (project->DeleteExtraStep(configuration,project->GetExtraStep(configuration,sname))) {
		steps_list->Delete(sel);
		ReloadSteps(snext);
	}
}

void mxProjectConfigWindow::OnStepsEdit(wxCommandEvent &evt) {
	compile_extra_step *step = project->GetExtraStep(configuration,steps_list->GetString(steps_list->GetSelection()));
	if (step) {
		new mxExtraStepWindow(this,configuration,step);
		if (mxExtraStepWindow::new_name.Len())
			ReloadSteps(mxExtraStepWindow::new_name);
	}
}

void mxProjectConfigWindow::ReloadSteps(wxString selection) {
	int count=0, sel=0, last_count=steps_list->GetCount();
	compile_extra_step *step = configuration->extra_steps;
	while (step && step->pos==CES_BEFORE_SOURCES) {
		if (count<last_count)
			steps_list->SetString(count,step->name);
		else
			steps_list->Append(step->name);
		if (step->name==selection) sel=count;
		count++;
		step = step->next;
	}
	if (count<last_count)
		steps_list->SetString(count,LANG(PROJECTCONFIG_STEPS_COMPILE_SOURCES,"**Compilar Fuentes del Proyecto**"));
	else
		steps_list->Append(LANG(PROJECTCONFIG_STEPS_COMPILE_SOURCES,"**Compilar Fuentes del Proyecto**"));
	if (selection==LANG(PROJECTCONFIG_STEPS_COMPILE_SOURCES,"**Compilar Fuentes del Proyecto**")) sel=count;
	count++;
	while (step && step->pos==CES_BEFORE_LIBS) {
		if (count<last_count)
			steps_list->SetString(count,step->name);
		else
			steps_list->Append(step->name);
		if (step->name==selection) sel=count;
		count++;
		step = step->next;
	}
//	if (configuration->libs_to_build) {
		if (count<last_count)
			steps_list->SetString(count,LANG(PROJECTCONFIG_STEPS_LINK_LIBS,"**Generar Bibliotecas**"));
		else
			steps_list->Append(LANG(PROJECTCONFIG_STEPS_LINK_LIBS,"**Generar Bibliotecas**"));
		if (selection==LANG(PROJECTCONFIG_STEPS_LINK_LIBS,"**Generar Bibliotecas**")) sel=count;
		count++;
//	}
	while (step && step->pos==CES_BEFORE_EXECUTABLE) {
		if (count<last_count)
			steps_list->SetString(count,step->name);
		else
			steps_list->Append(step->name);
		if (step->name==selection) sel=count;
		count++;
		step = step->next;
	}
	if (count<last_count)
		steps_list->SetString(count,LANG(PROJECTCONFIG_STEPS_LINK_EXE,"**Enlazar Ejecutable**"));
	else
		steps_list->Append(LANG(PROJECTCONFIG_STEPS_LINK_EXE,"**Enlazar Ejecutable**"));
	if (selection==LANG(PROJECTCONFIG_STEPS_LINK_EXE,"**Enlazar Ejecutable**")) sel=count;
	count++;
	while (step && step->pos==CES_AFTER_LINKING) {
		if (count<last_count)
			steps_list->SetString(count,step->name);
		else
			steps_list->Append(step->name);
		if (step->name==selection) sel=count;
		count++;
		step = step->next;
	}
	while (count<last_count) steps_list->Delete(--last_count);
	if (count) steps_list->SetSelection(sel);
}

void mxProjectConfigWindow::DiscardChanges() {
	for (int i=0;i<project->configurations_count;i++)
		if (project->configurations[i]->bakup) {
			project->configurations[i]->bakup->extra_steps=project->configurations[i]->extra_steps;
			*(project->configurations[i])=*(project->configurations[i]->bakup);
		}
}

void mxProjectConfigWindow::ReloadLibs(wxString selection) {
	int n=libtobuild_list->GetCount(), i=0;
	project_library *lib = configuration->libs_to_build;
	while (lib) {
		if (i<n)
			libtobuild_list->SetString(i,lib->libname);
		else
			libtobuild_list->Append(lib->libname);
		lib = lib->next;
		i++;
	}
	if (i<n) while (i<n) libtobuild_list->Delete(--n);
	if (selection=="" && i) selection=libtobuild_list->GetString(0);
	libtobuild_list->SetSelection(libtobuild_list->FindString(selection));
}

void mxProjectConfigWindow::OnLibsAdd(wxCommandEvent &evt) {
	new mxLibToBuildWindow(this,configuration,nullptr);
	if (mxLibToBuildWindow::new_name.Len())
		ReloadLibs(mxLibToBuildWindow::new_name);
}

void mxProjectConfigWindow::OnLibsEdit(wxCommandEvent &evt) {
	project_library *lib = project->GetLibToBuild(configuration,libtobuild_list->GetString(libtobuild_list->GetSelection()));
	if (lib) {
		new mxLibToBuildWindow(this,configuration,lib);
		if (mxLibToBuildWindow::new_name.Len())
			ReloadLibs(mxLibToBuildWindow::new_name);
	}	
}

void mxProjectConfigWindow::OnLibsDel(wxCommandEvent &evt) {
	int sel=libtobuild_list->GetSelection(), count=libtobuild_list->GetCount();
	wxString lname = libtobuild_list->GetString(sel), snext;
	if (sel<count-1) snext=libtobuild_list->GetString(sel+1);
	else if (sel>0) snext=libtobuild_list->GetString(sel-1);
	if (mxMD_NO==mxMessageDialog(this,wxString(LANG(PROJECTCONFIG_ASK_DELETE_LIB,"Eliminar la biblioteca: "))<<lname,LANG(GENERAL_CONFIRM,"Confirmacion"),(mxMD_YES_NO|mxMD_WARNING)).ShowModal())
		return;
	if (project->DeleteLibToBuild(configuration,project->GetLibToBuild(configuration,lname))) {
		libtobuild_list->Delete(sel);
		ReloadLibs(snext);
	}
}

void mxProjectConfigWindow::OnLibsNoExe(wxCommandEvent &evt) {
	evt.Skip();
	wx_noexe.EnableAll(!libtobuild_noexec->GetValue());
}

void mxProjectConfigWindow::OnCompilingMacrosButton (wxCommandEvent & evt) {
	CommonPopup(compiling_macros).CommaSplit(true)
		.Caption( LANG(PROJECTCONFIG_COMPILING_MACROS,"Macros a definir") )
		.AddEditAsText().AddEditAsList().Run(this);
}

void mxProjectConfigWindow::OnComboToolchainChange(wxCommandEvent &evt) {
	wx_extern.EnableAll(!Toolchain::GetInfo(toolchains_combo->GetStringSelection()).IsExtern());
}

void mxProjectConfigWindow::OnToolchainOptionsButton (wxCommandEvent & evt) {
	mxToolchainOptions(this,toolchains_combo->GetStringSelection(),configuration).ShowModal();
}

//void mxProjectConfigWindow::OnImportLibsButton (wxCommandEvent & evt) {
//	
// }

void mxProjectConfigWindow::OnExecutionMethodButton (wxCommandEvent & evt) {
	
}

void mxProjectConfigWindow::OnComboExecutionScript (wxCommandEvent & evt) {
	wx_noscript.EnableAll(general_exec_method->GetSelection()!=0);
}

void mxProjectConfigWindow::SelectCustomStep(const wxString &custom_step_name) {
	notebook->SetSelection(config->Help.show_extra_panels?4:3);
	for(unsigned int i=0;i<steps_list->GetCount();i++) { 
		if (steps_list->GetString(i)==custom_step_name) {
			steps_list->Select(i);
			break;
		}
	}
}
