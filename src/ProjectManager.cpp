/** 
* @file ProjectManager.cpp
* @brief Implementaci�n de los m�todos de la clase ProjectManager
**/

#include "ProjectManager.h"
#include "mxUtils.h"
#include "BreakPointInfo.h"

#include <wx/textfile.h>
#include <wx/treectrl.h>
#include "mxSource.h"
#include "mxMainWindow.h"
#include "mxStatusBar.h"
#include "mxMessageDialog.h"
#include "Parser.h"
#include "ids.h"
#include "DebugManager.h"
#include "version.h"
#include "mxArgumentsDialog.h"
#include "ConfigManager.h"
#include <iostream>
#include <wx/msgdlg.h>
#include "Language.h"
#include "mxSingleton.h"
#include "mxOSD.h"
#include "parserData.h"
#include "mxCompiler.h"
#include <algorithm>
#include "Autocoder.h"
#include "Toolchain.h"
using namespace std;

#define ICON_LINE(filename) (wxString(_T("0 ICON \""))<<filename<<_T("\""))
#define MANIFEST_LINE(filename) (wxString(_T("1 RT_MANIFEST \""))<<filename<<_T("\""))

ProjectManager *project=NULL;
extern char path_sep;

wxString doxygen_configuration::get_tag_index() { 
	if (project && project->doxygen && project->doxygen->use_in_quickhelp)
		return DIR_PLUS_FILE(DIR_PLUS_FILE(project->path,project->doxygen->destdir),_T("index-for-zinjai.tag"));
	else return _T("");
}

	
// abrir un proyecto existente
ProjectManager::ProjectManager(wxFileName name) {
	loading=true; generating_wxfb=false;
	mxOSD osd(main_window,wxString(LANG(OSD_LOADING_PROJECT_PRE,"Abriendo "))<<name.GetName()<<LANG(OSD_LOADING_PROJECT_POST,"..."));
	
	singleton->Stop();
	
	first_compile_step=NULL;
	
	project=this;
	parser->CleanAll();
	cppcheck=NULL;
	doxygen=NULL;
	auto_wxfb=true; use_wxfb=false;
	version_required=0;
	custom_tabs=0;
	tab_use_spaces=false;
	
	int version_saved=0; // vesion del zinjai que guardo al proyecto (suele diferir de la requerida)
	int files_to_open=0, num_files_opened=0; // para la barra de progreso
	
	inspectlist current_inspections(_("<current_inspections>")), *current_inspectlist=NULL;
	
	main_window->SetStatusText(wxString(LANG(PROJMNGR_OPENING,"Abriendo"))<<_T(" \"")+name.GetFullPath()+_T("\"..."));
	main_window->notebook_sources->Freeze();
	
	debug->ClearSavedInspectionTables();
	
	name.MakeAbsolute();
	wxString conf_name=_T("Debug");
	wxString current_source=_T("");
	configurations_count=0;
	
	// inicializar listas con el elemento ficticio begin
	first_source = new file_item;
	first_other = new file_item;
	first_header = new file_item;
	// inicializar el arbol de proyecto
	main_window->project_tree.treeCtrl->DeleteChildren(main_window->project_tree.sources);
	main_window->project_tree.treeCtrl->DeleteChildren(main_window->project_tree.headers);
	main_window->project_tree.treeCtrl->DeleteChildren(main_window->project_tree.others);
	// setear nombre y path
	project_name=name.GetName();
	name.MakeAbsolute();
	filename=name.GetFullName();
	path=name.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
	last_dir=path;
	wxTextFile fil(DIR_PLUS_FILE(path,filename));
	executable_name=DIR_PLUS_FILE(path,name.GetName()+_T(BINARY_EXTENSION));
	if (!fil.Exists()) return;
	fil.Open();
#if defined(__WIN32__)
	wxChar file_path_char='\\';
	wxChar real_path_char='\\';
#else
	wxChar file_path_char='/';
	wxChar real_path_char='/';
#endif
	wxString section, key, value;
	file_item *last_file = NULL;
	BreakPointInfo *last_breakpoint = NULL;
	long l;
	compile_extra_step *extra_step=NULL;
	project_library *lib_to_build=NULL;
	for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() ) {
		//wxYield();
		if (str[0]=='#') {
			continue;
		} else if (str[0]=='[') {
			last_file=NULL;
			section=str.AfterFirst('[').BeforeFirst(']');
			if (section==_T("config")) { // agregar una configuracion en blanco a la lista
				active_configuration=configurations[configurations_count++]=new project_configuration(name.GetName(),_T(""));
				extra_step = NULL;
				lib_to_build = NULL;
			} else if (section==_T("lib_to_build")) { // agregar un paso de compilaci�n adicional a la configuraci�n actual del proyecto
				if (lib_to_build) {
					lib_to_build->next = new project_library;
					lib_to_build->next->prev=lib_to_build;
					lib_to_build=lib_to_build->next;
				} else if (active_configuration) {
					lib_to_build = new project_library;
					active_configuration->libs_to_build = lib_to_build;
				}
			} else if (section==_T("extra_step")) { // agregar un paso de compilaci�n adicional a la configuraci�n actual del proyecto
				if (extra_step) {
					extra_step->next = new compile_extra_step;
					extra_step->next->prev=extra_step;
					extra_step=extra_step->next;
				} else if (active_configuration) {
					extra_step = new compile_extra_step;
					active_configuration->extra_steps=extra_step;
				}
			} else if (section==_T("cppcheck") && !cppcheck) { 
				cppcheck=new cppcheck_configuration();
				cppcheck->save_in_project=true;
			} else if (section==_T("doxygen") && !doxygen) { 
				doxygen=new doxygen_configuration(project_name);
			}
		} else {
			// separar clave y valor, y arreglar valor si es un path
			key=str.BeforeFirst('=');
			value=str.AfterFirst('=');
			// interpretar clave y valor
			if (section==_T("general")) {
				if (key=="files_to_open") {
					utils->ToInt(value,files_to_open);
					if(files_to_open>0) {
						files_to_open++;
						main_window->SetStatusProgress((100*num_files_opened)/files_to_open);
					}
				}
				else CFG_GENERIC_READ_DN("active_configuration",conf_name);
				else CFG_GENERIC_READ_DN("current_source",current_source);
				else CFG_CHAR_READ_DN("path_char",file_path_char);
				else CFG_GENERIC_READ_DN("project_name",project_name);
				else CFG_GENERIC_READ_DN("autocodes_file",autocodes_file);
				else CFG_GENERIC_READ_DN("macros_file",macros_file);
				else CFG_GENERIC_READ_DN("autocomp_extra",autocomp_extra);
				else CFG_INT_READ_DN("version_saved",version_saved);
				else CFG_INT_READ_DN("version_required",version_required);
				else CFG_INT_READ_DN("custom_tabs",custom_tabs); 
				else CFG_BOOL_READ_DN("tab_use_spaces",tab_use_spaces);
				else CFG_BOOL_READ_DN("use_wxfb",use_wxfb);
				else CFG_BOOL_READ_DN("auto_wxfb",auto_wxfb);
				else CFG_GENERIC_READ_DN("explorer_path",last_dir);
			} else if (section==_T("lib_to_build") && lib_to_build) {
				CFG_GENERIC_READ_DN("path",lib_to_build->path);
				else CFG_GENERIC_READ_DN("libname",lib_to_build->libname);
				else CFG_GENERIC_READ_DN("sources",lib_to_build->sources);
				else CFG_GENERIC_READ_DN("extra_link",lib_to_build->extra_link);
				else CFG_BOOL_READ_DN("is_static",lib_to_build->is_static);
				else CFG_BOOL_READ_DN("default_lib",lib_to_build->default_lib);
			} else if (section==_T("extra_step") && extra_step) {
				CFG_GENERIC_READ_DN("name",extra_step->name);
				else CFG_GENERIC_READ_DN("output",extra_step->out);
				else CFG_GENERIC_READ_DN("deps",extra_step->deps);
				else CFG_GENERIC_READ_DN("command",extra_step->command);
				else CFG_BOOL_READ_DN("check_retval",extra_step->check_retval);
				else CFG_BOOL_READ_DN("hide_win",extra_step->hide_window);
				else CFG_BOOL_READ_DN("delete_on_clean",extra_step->delete_on_clean);
				else CFG_BOOL_READ_DN("link_output",extra_step->link_output);
				else if (key==_T("position")) {
					if (value==_T("before_sources"))
						extra_step->pos=CES_BEFORE_SOURCES;
					else if (value==_T("before_executable") || value==_T("before_linking"))
						extra_step->pos=CES_BEFORE_EXECUTABLE;
					else if (value==_T("before_libs"))
						extra_step->pos=CES_BEFORE_LIBS;
					else if (value==_T("after_linking"))
						extra_step->pos=CES_AFTER_LINKING;
				} 
			} else if (section==_T("config")) {
				if (key.StartsWith("toolchain_argument_")) {
					long l=-1; key.Mid(19).ToLong(&l);
					if (l>=0&&l<TOOLCHAIN_MAX_ARGS) 
						active_configuration->toolchain_arguments[l]=value;
				}
				else CFG_GENERIC_READ_DN("name",active_configuration->name);
				else CFG_GENERIC_READ_DN("toolchain",active_configuration->toolchain);
				else CFG_GENERIC_READ_DN("working_folder",active_configuration->working_folder);
				else CFG_BOOL_READ_DN("always_ask_args",active_configuration->always_ask_args);
				else CFG_GENERIC_READ_DN("args",active_configuration->args);
				else CFG_INT_READ_DN("wait_for_key",active_configuration->wait_for_key);
				else CFG_GENERIC_READ_DN("temp_folder",active_configuration->temp_folder);
				else CFG_GENERIC_READ_DN("output_file",active_configuration->output_file);
				else CFG_GENERIC_READ_DN("manifest_file",active_configuration->manifest_file);
				else CFG_GENERIC_READ_DN("icon_file",active_configuration->icon_file);
				else CFG_GENERIC_READ_DN("macros",active_configuration->macros);
				else CFG_GENERIC_READ_DN("compiling_extra",active_configuration->compiling_extra);
				else CFG_GENERIC_READ_DN("headers_dirs",active_configuration->headers_dirs);
				else CFG_INT_READ_DN("warnings_level",active_configuration->warnings_level);
				else CFG_BOOL_READ_DN("ansi_compliance",active_configuration->ansi_compliance);
				else CFG_INT_READ_DN("debug_level",active_configuration->debug_level);
				else CFG_INT_READ_DN("optimization_level",active_configuration->optimization_level);
				else CFG_GENERIC_READ_DN("linking_extra",active_configuration->linking_extra);
				else CFG_GENERIC_READ_DN("libraries_dirs",active_configuration->libraries_dirs);
				else CFG_GENERIC_READ_DN("libraries",active_configuration->libraries);
				else CFG_BOOL_READ_DN("strip_executable",active_configuration->strip_executable);
				else CFG_BOOL_READ_DN("enable_profiling",active_configuration->enable_profiling);
				else CFG_BOOL_READ_DN("console_program",active_configuration->console_program);
				else CFG_BOOL_READ_DN("dont_generate_exe",active_configuration->dont_generate_exe);

			} else if ( section==_T("source") || section==_T("header") || section==_T("other") ) {
				if (key==_T("path")) {
					if (real_path_char!=file_path_char)
						for (unsigned int i=0;i<value.Len();i++)
							if (value[i]==file_path_char)
								value[i]=real_path_char;
					if (section==_T("source"))
						last_file = AddFile('s',value,false);
					else if (section==_T("header"))
						last_file = AddFile('h',value,false);
					else if (section==_T("other"))
						last_file = AddFile('o',value,false);
				} else if (key==_T("cursor")) {
					value.ToLong(&l);
					if (last_file)
						last_file->cursor = l;
				} else if (key==_T("marker")) {
					value.ToLong(&l);
					if (last_file)
						last_file->markers = new marked_line_item(l,last_file->markers);
				} else if (key==_T("breakpoint")) {
					value.ToLong(&l);
					if (last_file && l>=0) {
						last_breakpoint=new BreakPointInfo(last_file,l);
					} else last_breakpoint=NULL;
				} else if (key==_T("breakpoint_ignore")) {
					value.ToLong(&l);
					if (last_breakpoint) last_breakpoint->ignore_count=l;
				} else if (key==_T("breakpoint_only_once")) {
					if (last_breakpoint) last_breakpoint->only_once=utils->IsTrue(value);
				} else if (key==_T("enabled")) {
					if (last_breakpoint) last_breakpoint->enabled=utils->IsTrue(value);
				} else if (key==_T("breakpoint_condition")) {
					if (last_breakpoint) last_breakpoint->cond=value;
				} else if (key==_T("open") && utils->IsTrue(value)) {
					if (files_to_open>0)
						main_window->SetStatusProgress((100*(++num_files_opened))/files_to_open);
					if (last_file) {
						mxSource *src = main_window->OpenFile(DIR_PLUS_FILE(path,last_file->name),false);
						if (src && src!=EXTERNAL_SOURCE) src->MoveCursorTo(last_file->cursor,true);
					}
				}
			} else if (section==_T("doxygen")) {
				CFG_GENERIC_READ_DN("name",doxygen->name);
				else CFG_GENERIC_READ_DN("version",doxygen->version);
				else CFG_GENERIC_READ_DN("destdir",doxygen->destdir);
				else CFG_GENERIC_READ_DN("extra_files",doxygen->extra_files);
				else CFG_TEXT_READ_DN("extra_conf",doxygen->extra_conf);
				else CFG_GENERIC_READ_DN("base_path",doxygen->base_path);
				else CFG_GENERIC_READ_DN("exclude_files",doxygen->exclude_files);
				else CFG_GENERIC_READ_DN("lang",doxygen->lang);
				else CFG_BOOL_READ_DN("do_headers",doxygen->do_headers);
				else CFG_BOOL_READ_DN("do_cpps",doxygen->do_cpps);
				else CFG_BOOL_READ_DN("hideundocs",doxygen->hideundocs);
				else CFG_BOOL_READ_DN("latex",doxygen->latex);
				else CFG_BOOL_READ_DN("html",doxygen->html);
				else CFG_BOOL_READ_DN("html_searchengine",doxygen->html_searchengine);
				else CFG_BOOL_READ_DN("html_navtree",doxygen->html_navtree);
				else CFG_BOOL_READ_DN("use_in_quickhelp",doxygen->use_in_quickhelp);
				else CFG_BOOL_READ_DN("preprocess",doxygen->preprocess);
				else CFG_BOOL_READ_DN("extra_static",doxygen->extra_static);
				else CFG_BOOL_READ_DN("extra_private",doxygen->extra_private);
			} else if (section==_T("cppcheck")) {
				CFG_BOOL_READ_DN("copy_from_config",cppcheck->copy_from_config);
				else CFG_GENERIC_READ_DN("config_d",cppcheck->config_d);
				else CFG_GENERIC_READ_DN("config_u",cppcheck->config_u);
				else CFG_GENERIC_READ_DN("style",cppcheck->style);
				else CFG_GENERIC_READ_DN("platform",cppcheck->platform);
				else CFG_GENERIC_READ_DN("standard",cppcheck->standard);
				else CFG_GENERIC_READ_DN("suppress_file",cppcheck->suppress_file);
				else CFG_GENERIC_READ_DN("suppress_ids",cppcheck->suppress_ids);
				else CFG_GENERIC_READ_DN("exclude_list",cppcheck->exclude_list);
				else CFG_BOOL_READ_DN("inline_suppr",cppcheck->inline_suppr);
			} else if (section==_T("inspections")) {
				if (key==_T("name")) {
					if (value==_T("<current_inspections>"))
						current_inspectlist = &current_inspections;
					else
						current_inspectlist = debug->GetInspectionsTable(value,true);
				} else if (key==_T("expr") && current_inspectlist) {
					current_inspectlist->vars.Add(value);
				}
			}
		}
	}
	fil.Close();
	modified=false;
	force_relink=false;

	main_window->project_tree.treeCtrl->SortChildren(main_window->project_tree.sources);
	main_window->project_tree.treeCtrl->SortChildren(main_window->project_tree.others);
	main_window->project_tree.treeCtrl->SortChildren(main_window->project_tree.headers);
	
	last_dir=DIR_PLUS_FILE(path,last_dir);
	wxFileName explorer_fname(last_dir);
	if (explorer_fname.DirExists()) {
		explorer_fname.Normalize(wxPATH_NORM_DOTS);
		main_window->SetExplorerPath(explorer_fname.GetFullPath());
	}
	
	// agregar los indices de autocompletado nuevos
	code_helper->AppendIndexes(autocomp_extra);
	
	// cargar las inspecciones en la tabla
	code_helper->AppendIndexes(autocomp_extra);
	debug->LoadInspectionsTable(&current_inspections);
	
	if (configurations_count==0) { // si no tenia definida ninguna configuracion crear las dos predeterminadas
		// crear configuracion Debug
		configurations[0] = new project_configuration(name.GetName(),_T("Debug"));
		configurations[0]->debug_level=2;
		configurations[0]->optimization_level=0;
		configurations[0]->strip_executable=false;
		configurations[0]->macros=configurations[0]->old_macros="_DEBUG";
		// crear configuracion Release
		configurations[1] = new project_configuration(name.GetName(),_T("Release"));
		configurations[1]->debug_level=0;
		configurations[1]->optimization_level=2;
		configurations[1]->strip_executable=true;
		// Debug es la predeterminada
		active_configuration = configurations[0];
		configurations_count = 2;
	
	} else { // seleccionar la que corresponde si existe
		active_configuration=GetConfig(conf_name);
		if (active_configuration==NULL) // si no tenia seleccionada ninguna, seleccionar la primera
			active_configuration = configurations[0];
		
		for (int i=0;i<configurations_count;i++)
			configurations[i]->old_macros=configurations[i]->macros;
#ifdef __WIN32__
		wxString cur1="win",cur2="w32";
#else
#ifdef __APPLE__
		wxString cur1="mac",cur2="apple";
#else
		wxString cur1="lnx",cur2="linux";
#endif
#endif
		if ( (
				active_configuration->name.Lower().Contains("lnx")||
				active_configuration->name.Lower().Contains("linux")||
				active_configuration->name.Lower().Contains("win")||
				active_configuration->name.Lower().Contains("w32")||
				active_configuration->name.Lower().Contains("mac")||
				active_configuration->name.Lower().Contains("apple")
			) && !(
				active_configuration->name.Lower().Contains(cur1)||
				active_configuration->name.Lower().Contains(cur2)
			) ) {
				
				project_configuration *suggested_configuration=NULL;
				for (int i=0;i<configurations_count;i++) {
					if (configurations[i]->name.Lower().Contains(cur1)||configurations[i]->name.Lower().Contains(cur2)) {
						if (!suggested_configuration || 
							(!(suggested_configuration->name.Lower().Contains("debug")||suggested_configuration->name.Lower().Contains("dbg"))
							&&(configurations[i]->name.Lower().Contains("debug")||configurations[i]->name.Lower().Contains("dbg"))) )
								suggested_configuration=configurations[i];
					}
				}
				if (suggested_configuration) {
					int res = mxMessageDialog(main_window,wxString(
						LANG(PROJMNGR_CHANGE_PROFILE_OPENNING_PRE,"Parece que esta abriendo un proyecto que tiene seleccionado un perfil\n"
						"de compilacion y ejecucion para otro sistema operativo: \"")
						)<<active_configuration->name<<
						LANG(PROJMNGR_CHANGE_PROFILE_OPENNING_MID,"\"\n �Desea cambiar el perfil activo por: \"")
						<<suggested_configuration->name<<
						LANG(PROJMNGR_CHANGE_PROFILE_OPENNING_POST,"\"?")
						,LANG(PROJMNGR_CHANGE_PROFILE_OPENNING_POST,"Perfil de Compilacion y Ejecucion"),mxMD_YES_NO|mxMD_WARNING,
						LANG(PROJMNGR_CHANGE_PROFILE_OPENNING_CHECK,"Seleccionar otro perfil de la lista")
						,false).ShowModal();
					if (mxMD_YES&res) {
						if (res&mxMD_CHECKED) {
							wxArrayString choices;
							for (int i=0;i<configurations_count;i++)
								choices.Add(configurations[i]->name);
							wxString res=wxGetSingleChoice(
								LANG(PROJMNGR_CHANGE_PROFILE_OPENNING_CHOICE,"Seleccione el perfil a activar"),
								LANG(PROJMNGR_CHANGE_PROFILE_OPENNING_POST,"Perfil de Compilacion y Ejecucion"),
								choices,main_window);
							if (res.Len())
								for (int i=0;i<configurations_count;i++)
									if (configurations[i]->name==res)
										active_configuration=configurations[i];
						} else {
							active_configuration=suggested_configuration;
						}
					}
				}
			}
	}
 
	SetActiveConfiguration(active_configuration);
	
	// configurar interface de la ventana principal para modo proyecto
	if (current_source!=_T("")) {
		if (real_path_char!=file_path_char)
			for (unsigned int i=0;i<current_source.Len();i++)
				if (current_source[i]==file_path_char)
					current_source[i]=real_path_char;
		main_window->OpenFile(DIR_PLUS_FILE(path,current_source),false);
	}
	
	main_window->notebook_sources->Thaw();
	main_window->notebook_sources->Fit();
	
	if (autocodes_file.Len()) autocoder->LoadFromFile(DIR_PLUS_FILE(path,autocodes_file));
	
	if (version_saved<20100518) { // arreglar cambios de significado
		for (int i=0;i<configurations_count;i++)
			if (configurations[i]->wait_for_key)
				configurations[i]->wait_for_key=2;
	}
	
	if (version_required>VERSION) {
		mxMessageDialog(main_window,
			wxString(_T("El proyecto que esta abriendo requiere una version superior de ZinjaI\n"))<<
			_T("Su version es ")<<VERSION<<_T(" y se requiere al menos ")<<version_required<<_T(" para cargar\n")<<
			_T("todas las opciones de proyecto correctamente. El proyecto se abrira\n")<<
			_T("de todas formas, pero podria encontrar problemas posteriormente.\n"),
			_T("Version del Proyecto"),mxMD_WARNING|mxMD_OK).ShowModal();
	}
	
	if (files_to_open>0) main_window->SetStatusProgress(-1);
	
	loading=false;

	main_window->PrepareGuiForProject(true);
	if (project->use_wxfb) project->ActivateWxfb(); // para que marque en el menu y verifique si esta instalado
	main_window->SetStatusText(wxString(LANG(GENERAL_READY,"Listo")));
}

// liberar memoria al destruir el proyecto
ProjectManager::~ProjectManager(){

	parser->CleanAll();
	autocoder->Reset("");
	debug->ClearSavedInspectionTables();
	
	// configurar interface para modo proyecto
	main_window->PrepareGuiForProject(false);

	// vaciar las listas
	ML_FREE(first_source);
	ML_FREE(first_header);
	ML_FREE(first_other);
	
	// limipiar las configuraciones
	for (int i=0;i<configurations_count;i++)
		delete configurations[i];
	
	// restaurar los indices de autocompletado
	code_helper->ReloadIndexes(config->Help.autocomp_indexes);
	
	singleton->Start();
	
	project=NULL;
	
	Toolchain::SelectToolchain(); // keep after project=NULL
	
}

// agregar un archivo al proyecto: where = 's':fuente 'h':cabecera, 'o':otros
// devuelve verdadero si lo inserta, falso si ya estaba
file_item *ProjectManager::AddFile (char where, wxFileName filename, bool sort_tree) {
	if (filename.IsAbsolute())
		filename.MakeRelativeTo(path);
	wxString name=filename.GetFullPath();
	file_item *first;
	wxTreeItemId item;
	// elegir la lista adecuada y agregarlo al arbol de proyecto
	switch (where) {
		case 's':
			item = main_window->AddToProjectTreeProject(name,'s',sort_tree);
			first = first_source;
			for (int i=0;i<configurations_count;i++) {
				project_library *lib = configurations[i]->libs_to_build;
				while (lib) {
					if (lib->default_lib) {
						lib->sources<<_T(" ")<<utils->Quotize(name);
						break;
					}
					lib = lib->next;
				}
			}
			break;
		case 'h':
			item = main_window->AddToProjectTreeProject(name,'h',sort_tree);
			first = first_header;
			break;
		default:
			item = main_window->AddToProjectTreeProject(name,'o',sort_tree);
			first = first_other;
			break;
	};
	// insertar el elemento al final de la lista si aun no existe
	ML_ITERATE(first)
		if (first->name==name) return first;
	first->next = new file_item (first,name,item);
	modified=true;
	return first->next;
}

bool ProjectManager::Save () {
	
	if (version_required>VERSION) {
		int res = mxMessageDialog(main_window,
			wxString(_T("El proyecto que esta por guardar fue creado con una version de ZinjaI superior\n"))<<
			_T("a la que esta utilizando. Si graba el proyecto ahora se convertira a su su\n")<<
			_T("version. Algunas opciones de configuracion del mismo podrian perderse en la\n")<<
			_T("conversion. Desea guardarlo de todas formas?"),_T("Version del Proyecto"),mxMD_WARNING|mxMD_YES_NO/*,_T("Guardar con otro nombre"),false*/).ShowModal();
		if (res&mxMD_NO) return false;
//		if (res&mxMD_CHECKED) {		
//			wxFileDialog dlg (this, _T("Guardar Proyecto"),source->sin_titulo?wxString(wxFileName::GetHomeDir()):wxFileName(source->source_filename).GetPath(),source->sin_titulo?wxString(wxEmptyString):wxFileName(source->source_filename).GetFullName(), _T("Any file (*)|*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
//			dlg.SetDirectory(source->sin_titulo?wxString(project?project->last_dir:config->Files.last_dir):wxFileName(source->source_filename).GetPath());
//			dlg.SetWildcard(_T("Todos los archivos|*|Proyectos ZinjaI|"WILDCARD_PROJECT));
//			if (dlg.ShowModal() != wxID_OK) return;
//			wxFileName fn(dlg.GetPath());
//			path=fn.
//		}
		version_required=GetRequiredVersion();
	}
	
	
	// abrir el archivo (crear o pisar)
	wxTextFile fil(DIR_PLUS_FILE(path,filename));
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	
	file_item *item;
	mxSource *source;
	
#if defined(_WIN32) || defined(__WIN32__)
	fil.AddLine(wxString(_T("# generated by ZinjaI-w32-"))<<VERSION);
#else
	fil.AddLine(wxString(_T("# generated by ZinjaI-lnx-"))<<VERSION);
#endif
	
	// guardar cosas varias
	fil.AddLine(_T("[general]"));
	if (main_window->notebook_sources->GetPageCount()>2)
		CFG_GENERIC_WRITE_DN("files_to_open",main_window->notebook_sources->GetPageCount());
	CFG_GENERIC_WRITE_DN("project_name",project_name);
	CFG_GENERIC_WRITE_DN("autocodes_file",autocodes_file);
	CFG_GENERIC_WRITE_DN("macros_file",macros_file);
	CFG_GENERIC_WRITE_DN("autocomp_extra",autocomp_extra);
	CFG_GENERIC_WRITE_DN("active_configuration",active_configuration->name);
	CFG_GENERIC_WRITE_DN("version_saved",VERSION);
	CFG_GENERIC_WRITE_DN("version_required",version_required);
	CFG_GENERIC_WRITE_DN("custom_tabs",custom_tabs);
	CFG_BOOL_WRITE_DN("tab_use_spaces",tab_use_spaces);
	CFG_BOOL_WRITE_DN("use_wxfb",use_wxfb);
	CFG_GENERIC_WRITE_DN("explorer_path",utils->Relativize(main_window->explorer_tree.path,path));
	
	if (main_window->notebook_sources->GetPageCount()>0) {
		mxSource *source=(mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
		if (!source->sin_titulo) {
			wxFileName fn(source->source_filename.GetFullPath());
			fn.MakeRelativeTo(path);
			CFG_GENERIC_WRITE_DN("current_source",fn.GetFullPath());
		}
	}
#if defined(_WIN32) || defined(__WIN32__)
	fil.AddLine(_T("path_char=\\"));
#else
	fil.AddLine(_T("path_char=/"));
#endif
	// agregar la lista de fuentes pertenecientes al proyecto
	marked_line_item *marker;
	BreakPointInfo *breakpoint;
	wxString section;
	for(int i=0;i<3;i++) { 
		if (i==0) { item = first_source; section="[source]"; }
		else if (i==1) { item = first_header; section="[header]"; }
		else if (i==2) { item = first_other; section="[other]"; }
		ML_ITERATE(item) {
			fil.AddLine(section);
			CFG_GENERIC_WRITE_DN("path",item->name);
			source=main_window->IsOpen(DIR_PLUS_FILE(path,item->name));
			if (source) GetSourceExtras(source,item);
			CFG_GENERIC_WRITE_DN("cursor",item->cursor);
			breakpoint = item->breaklist;
			while (breakpoint) {
				if (breakpoint->line_number>=0) { 
					CFG_GENERIC_WRITE_DN("breakpoint",breakpoint->line_number);
					if (!breakpoint->enabled) CFG_BOOL_WRITE_DN("breakpoint_enabled",false);
					if (breakpoint->only_once) CFG_BOOL_WRITE_DN("breakpoint_only_once",true);
					if (breakpoint->ignore_count) CFG_GENERIC_WRITE_DN("breakpoint_ignore",breakpoint->ignore_count);
					if (breakpoint->cond.Len()) CFG_GENERIC_WRITE_DN("breakpoint_condition",breakpoint->cond);
	//				breakpoint->enabled=true; // que hace esto aca?
				}
				breakpoint = breakpoint->Next();
			}
			marker = item->markers;
			while (marker) {
				CFG_GENERIC_WRITE_DN("marker",marker->line);
				marker = marker->next;
			}
			if (source)
				fil.AddLine(_T("open=true"));
		}
	}
	
	for (int i=0;i<configurations_count;i++) {
		fil.AddLine(_T("[config]"));
		CFG_GENERIC_WRITE_DN("name",configurations[i]->name);
		CFG_GENERIC_WRITE_DN("toolchain",configurations[i]->toolchain);
		for(int j=0;j<TOOLCHAIN_MAX_ARGS;j++)
			fil.AddLine(wxString("toolchain_argument_")<<j<<"="<<configurations[i]->toolchain_arguments[j]);
		CFG_GENERIC_WRITE_DN("working_folder",configurations[i]->working_folder);
		CFG_BOOL_WRITE_DN("always_ask_args",configurations[i]->always_ask_args);
		CFG_GENERIC_WRITE_DN("args",configurations[i]->args);
		CFG_GENERIC_WRITE_DN("wait_for_key",configurations[i]->wait_for_key);
		CFG_GENERIC_WRITE_DN("temp_folder",configurations[i]->temp_folder);
		CFG_GENERIC_WRITE_DN("output_file",configurations[i]->output_file);
		CFG_GENERIC_WRITE_DN("icon_file",configurations[i]->icon_file);
		CFG_GENERIC_WRITE_DN("manifest_file",configurations[i]->manifest_file);
		CFG_GENERIC_WRITE_DN("compiling_extra",configurations[i]->compiling_extra);
		CFG_GENERIC_WRITE_DN("macros",configurations[i]->macros);
		CFG_GENERIC_WRITE_DN("warnings_level",configurations[i]->warnings_level);
		CFG_BOOL_WRITE_DN("ansi_compliance",configurations[i]->ansi_compliance);
		CFG_GENERIC_WRITE_DN("debug_level",configurations[i]->debug_level);
		CFG_GENERIC_WRITE_DN("optimization_level",configurations[i]->optimization_level);
		CFG_GENERIC_WRITE_DN("headers_dirs",configurations[i]->headers_dirs);
		CFG_GENERIC_WRITE_DN("linking_extra",configurations[i]->linking_extra);
		CFG_GENERIC_WRITE_DN("libraries_dirs",configurations[i]->libraries_dirs);
		CFG_GENERIC_WRITE_DN("libraries",configurations[i]->libraries);
		CFG_BOOL_WRITE_DN("strip_executable",configurations[i]->strip_executable);
		CFG_BOOL_WRITE_DN("console_program",configurations[i]->console_program);
		CFG_BOOL_WRITE_DN("dont_generate_exe",configurations[i]->dont_generate_exe);
		CFG_BOOL_WRITE_DN("enable_profiling",configurations[i]->enable_profiling);
		project_library *lib_to_build = configurations[i]->libs_to_build;
		while (lib_to_build) {
			fil.AddLine(_T("[lib_to_build]"));
			CFG_GENERIC_WRITE_DN("libname",lib_to_build->libname);
			CFG_GENERIC_WRITE_DN("path",lib_to_build->path);
			CFG_GENERIC_WRITE_DN("sources",lib_to_build->sources);
			CFG_GENERIC_WRITE_DN("extra_link",lib_to_build->extra_link);
			CFG_BOOL_WRITE_DN("is_static",lib_to_build->is_static);
			CFG_BOOL_WRITE_DN("default_lib",lib_to_build->default_lib);
			lib_to_build = lib_to_build->next;
		}
		compile_extra_step *step=configurations[i]->extra_steps;
		while (step) {
			fil.AddLine(_T("[extra_step]"));
			CFG_GENERIC_WRITE_DN("name",step->name);
			CFG_GENERIC_WRITE_DN("deps",step->deps);
			CFG_GENERIC_WRITE_DN("output",step->out);
			CFG_GENERIC_WRITE_DN("command",step->command);
			wxString spos=_T("unknown");
			switch (step->pos) {
			case CES_BEFORE_SOURCES:
				spos=_T("before_sources");
				break;
			case CES_BEFORE_LIBS:
				spos=_T("before_libs");
				break;
			case CES_BEFORE_EXECUTABLE:
				spos=_T("before_executable");
				break;
			case CES_AFTER_LINKING:
				spos=_T("after_linking");
				break;
			}
			CFG_GENERIC_WRITE_DN("position",spos);
			CFG_BOOL_WRITE_DN("check_retval",step->check_retval);
			CFG_BOOL_WRITE_DN("hide_win",step->hide_window);
			CFG_BOOL_WRITE_DN("delete_on_clean",step->delete_on_clean);
			CFG_BOOL_WRITE_DN("link_output",step->link_output);
			step = step->next;
		}
	}

	// configuracion Doxygen
	if (doxygen && doxygen->save) {
		fil.AddLine(_T("[doxygen]"));
		CFG_GENERIC_WRITE_DN("name",doxygen->name);
		CFG_GENERIC_WRITE_DN("version",doxygen->version);
		CFG_GENERIC_WRITE_DN("destdir",doxygen->destdir);
		CFG_GENERIC_WRITE_DN("lang",doxygen->lang);
		CFG_GENERIC_WRITE_DN("extra_files",doxygen->extra_files);
		CFG_GENERIC_WRITE_DN("base_path",doxygen->base_path);
		CFG_TEXT_WRITE_DN("extra_conf",doxygen->extra_conf);
		CFG_GENERIC_WRITE_DN("exclude_files",doxygen->exclude_files);
		CFG_BOOL_WRITE_DN("do_headers",doxygen->do_headers);
		CFG_BOOL_WRITE_DN("do_cpps",doxygen->do_cpps);
		CFG_BOOL_WRITE_DN("hideundocs",doxygen->hideundocs);
		CFG_BOOL_WRITE_DN("html",doxygen->html);
		CFG_BOOL_WRITE_DN("use_in_quickhelp",doxygen->use_in_quickhelp);
		CFG_BOOL_WRITE_DN("preprocess",doxygen->preprocess);
		CFG_BOOL_WRITE_DN("extra_private",doxygen->extra_private);
		CFG_BOOL_WRITE_DN("extra_static",doxygen->extra_static);
		CFG_BOOL_WRITE_DN("html_searchengine",doxygen->html_searchengine);
		CFG_BOOL_WRITE_DN("html_navtree",doxygen->html_navtree);
		CFG_BOOL_WRITE_DN("latex",doxygen->latex);
	}
	
	if (cppcheck && cppcheck->save_in_project) {
		fil.AddLine(_T("[cppcheck]"));
		CFG_BOOL_WRITE_DN("copy_from_config",cppcheck->copy_from_config);
		CFG_GENERIC_WRITE_DN("config_d",cppcheck->config_d);
		CFG_GENERIC_WRITE_DN("config_u",cppcheck->config_u);
		CFG_GENERIC_WRITE_DN("style",cppcheck->style);
		CFG_GENERIC_WRITE_DN("platform",cppcheck->platform);
		CFG_GENERIC_WRITE_DN("standard",cppcheck->standard);
		CFG_GENERIC_WRITE_DN("suppress_file",cppcheck->suppress_file);
		CFG_GENERIC_WRITE_DN("suppress_ids",cppcheck->suppress_ids);
		CFG_GENERIC_WRITE_DN("inline_suppr",cppcheck->inline_suppr);
		CFG_GENERIC_WRITE_DN("exclude_list",cppcheck->exclude_list);
	}
	
	// guardar inspecciones actuales y tablas guardadas
	fil.AddLine(_T("[inspections]"));
	fil.AddLine(_T("name=<current_inspections>"));
	inspectlist il(_T("<current_inspections>")); debug->SaveInspectionsTable(&il);
	for (unsigned int j=0;j<il.vars.GetCount();j++) 
		CFG_GENERIC_WRITE_DN("expr",il.vars[j]);
	
	for (unsigned int i=0;i<debug->inspections_tables.size();i++) {
		fil.AddLine(_T("[inspections]"));
		inspectlist &ii = *debug->inspections_tables[i];
		CFG_GENERIC_WRITE_DN("name",ii.name);
		for (unsigned int j=0;j<ii.vars.GetCount();j++) 
			CFG_GENERIC_WRITE_DN("expr",ii.vars[j]);
	}
	
	// sellar, escribir, cerrar y terminar
	fil.AddLine(_T("[end]"));
	fil.Write();
	fil.Close();
	modified=false;
	return true;
	
}

wxString ProjectManager::GetFileName() {
	return filename.Mid(0,filename.Len()-4);
}

file_item *ProjectManager::FindFromItem(wxTreeItemId &tree_item) {
	file_item *item;
	item = first_source;
	ML_ITERATE(item) {
		if (item->item==tree_item)
			return item;
	}
	item = first_header;
	ML_ITERATE(item) {
		if (item->item==tree_item)
			return item;
	}
	item = first_other;
	ML_ITERATE(item) {
		if (item->item==tree_item)
			return item;
	}
	return NULL;
}

file_item *ProjectManager::FindFromName(wxString name, char *where) {
	file_item *item;
	item = first_source;
	if (where) *where='s';
	ML_ITERATE(item) {
		if (wxFileName(item->name).GetFullName()==name || DIR_PLUS_FILE(path,item->name)==name)
			return item;
	}
	if (where) *where='h';
	item = first_header;
	ML_ITERATE(item) {
		if (wxFileName(item->name).GetFullName()==name || DIR_PLUS_FILE(path,item->name)==name)
			return item;
	}
	if (where) *where='o';
	item = first_other;
	ML_ITERATE(item) {
		if (wxFileName(item->name).GetFullName()==name || DIR_PLUS_FILE(path,item->name)==name)
			return item;
	}
	return NULL;
}

wxString ProjectManager::GetNameFromItem(wxTreeItemId &tree_item, bool relative) {
	file_item *item=FindFromItem(tree_item);
	if (item)
		if (relative) 
			return item->name;
		else
			return DIR_PLUS_FILE(path,item->name);
	else
		return _T("");
}

/**
* @return puntero al file_item del archivo si lo encuenctra, NULL si no lo encuentra
**/
file_item *ProjectManager::HasFile(wxFileName file) {
	file_item *item;
	item = first_source;
	ML_ITERATE(item) {
		if (wxFileName(DIR_PLUS_FILE(path,item->name))==file)
			return item;
	}
	item = first_header;
	ML_ITERATE(item) {
		if (wxFileName(DIR_PLUS_FILE(path,item->name))==file)
			return item;
	}
	item = first_other;
	ML_ITERATE(item) {
		if (wxFileName(DIR_PLUS_FILE(path,item->name))==file)
			return item;
	}
	return NULL;
}


bool ProjectManager::RenameFile(wxTreeItemId &tree_item, wxString new_name) {
	file_item *item = FindFromItem(tree_item);
	if (item) {
		wxFileName fname=new_name;
		fname.MakeRelativeTo(path);
		new_name=fname.GetFullPath();
		wxString src = DIR_PLUS_FILE(path,item->name);
		wxString dst = DIR_PLUS_FILE(path,new_name);
		if (!HasFile(new_name) && !wxFileName::DirExists(dst) && (!wxFileName::FileExists(dst) || mxMD_YES == mxMessageDialog(main_window,LANG(PROJMNGR_CONFIRM_REPLACE,"Ya existe un archivo con ese nombre. Desea Reemplazarlo?"),LANG(GENERAL_WARNING,"Advertencia"),mxMD_YES_NO|mxMD_WARNING).ShowModal()) ) { 
			parser->RenameFile(DIR_PLUS_FILE(path,item->name),DIR_PLUS_FILE(path,new_name));
			item->name=new_name;
			modified=true;
			wxRenameFile(src,dst,true);
			main_window->project_tree.treeCtrl->SetItemText(item->item,config->Init.fullpath_on_project_tree?item->name:wxFileName(new_name).GetFullName());
			main_window->project_tree.treeCtrl->SortChildren(main_window->project_tree.sources);
			main_window->project_tree.treeCtrl->SortChildren(main_window->project_tree.others);
			main_window->project_tree.treeCtrl->SortChildren(main_window->project_tree.headers);
			mxSource *source;
			for (int i=0,j=main_window->notebook_sources->GetPageCount();i<j;i++)
				if (((mxSource*)(main_window->notebook_sources->GetPage(i)))->source_filename==src) {
					source = ((mxSource*)(main_window->notebook_sources->GetPage(i)));
					source->SaveSource(dst);
					source->SetPageText(wxFileName(dst).GetFullName());
					parser->ParseSource(source,true);
					return true;
				} 
			return true;
		}
	}
	return false;
}

bool ProjectManager::MoveFile(wxTreeItemId &tree_item, char where) {
	modified=true;
	// eliminar el item de la lista
	file_item *item=FindFromItem(tree_item);
	ML_REMOVE(item);
	// eliminar del arbol	
	main_window->project_tree.treeCtrl->Delete(item->item);
	file_item *first;
	// elegir la lista adecuada y agregarlo al arbol de proyecto
	switch (where) {
		case 's':
			item->item = main_window->AddToProjectTreeProject(item->name,'s');
			first = first_source;
			break;
		case 'h':
			item->item = main_window->AddToProjectTreeProject(item->name,'h');
			first = first_header;
			break;
		default:
			item->item = main_window->AddToProjectTreeProject(item->name,'o');
			first = first_other;
			break;
	};
	
	if (where=='s' || where=='h') {
		wxString name = DIR_PLUS_FILE(path,item->name);
		mxSource *source;
		if ((source=main_window->IsOpen(name)) && source->GetModify()) {
			parser->ParseSource(source);
		} else {
			parser->ParseFile(name);
		}
	} else {
		wxString name = DIR_PLUS_FILE(path,item->name);
		parser->RemoveFile(name);
	}
	
	main_window->project_tree.treeCtrl->SelectItem(item->item);
	
	// insertar el elemento al final de la lista
	ML_ITERATE(first);
	first->next = item;
	item->prev=first;
	item->next=NULL;
	modified=true;
	return true;

}

bool ProjectManager::DeleteFile(wxTreeItemId &tree_item, bool also) {
	modified=true;
	file_item *item=FindFromItem(tree_item);
	if (item) {
		mxSource *src = main_window->IsOpen(tree_item);
		if (src) { // para que no reviente cuando al cerrarlo intente limpiar la list de breaks
			src->breaklist = new BreakPointInfo*(item->breaklist); src->own_breaks=true;
			item->breaklist=NULL;
		}
		int ans;
		if (also)
			ans=mxMessageDialog(main_window,wxString(LANG(PROJMNGR_CONFIRM_DETTACH_ALSO_PRE,"�Desea quitar tambien el archivo \""))<<item->name<<LANG(PROJMNGR_CONFIRM_DETTACH_ALSO_POST,"\" del proyecto?"),item->name,mxMD_QUESTION|mxMD_YES_NO,LANG(PROJMNGR_DELETE_FROM_DISK,"Eliminar el archivo del disco"),false).ShowModal();
		else
			ans=mxMessageDialog(main_window,LANG(PROJMNGR_CONFIRM_DETTACH_FILE,"�Desea quitar el archivo del proyecto?"),item->name,mxMD_QUESTION|mxMD_YES_NO,LANG(PROJMNGR_DELETE_FROM_DISK,"Eliminar el archivo del disco"),false).ShowModal();
		if (ans&mxMD_CANCEL || ans&mxMD_NO)
			return false;
		// eliminar el archivo del disco
		if (ans&mxMD_CHECKED)
			wxRemoveFile(DIR_PLUS_FILE(path,item->name));
		parser->RemoveFile(DIR_PLUS_FILE(path,item->name));
		// quitar el archivo del arbol de proyecto
		main_window->project_tree.treeCtrl->Delete(item->item);
		// eliminar el item de la lista
		wxString comp = utils->GetComplementaryFile(DIR_PLUS_FILE(project->path,item->name));
		ML_REMOVE(item);
		delete item;
		if (comp.Len() && HasFile(comp)) DeleteFile(HasFile(comp)->item,true);
		return true;
	}
	return false;
}


bool ProjectManager::DependsOnMacro(file_item *item, wxArrayString &macros) {
	wxTextFile fil(DIR_PLUS_FILE(path,item->name));
	if (fil.Exists()) {
		fil.Open();
		unsigned int i,l=macros.GetCount();
		for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine()) {
			for (i=0;i<l;i++) {
				int j=str.Find(macros[i]), n=macros[i].Len();
				if ( j!=wxNOT_FOUND // si estaba...
					// ...y antes no habia letra/numero/guion_bajo...
					&& ( j==0 || ( !(str[j-1]>='a'&&str[j-1]<='z')&&!(str[j-1]>='A'&&str[j-1]<='Z')&&!(str[j-1]>='0'&&str[j-1]<='9')&&str[j-1]!='_' ) ) 
					// ...y despues no habia letra/numero/guion_bajo...
					&& ( j+n==int(str.Len()) || ( !(str[j+n]>='a'&&str[j+n]<='z')&&!(str[j+n]>='A'&&str[j+n]<='Z')&&!(str[j+n]>='0'&&str[j+n]<='9')&&str[j+n]!='_' ) ) 
					)
						return true;
			}
		}
		fil.Close();
	}
	return false;
}


/**
* Si only_one es NULL se analiza todo el proyecto (fuentes, bibliotecas, pasos 
* adicionales, ejecutable, etc). Si no es NULL se analiza solo lo relacionado 
* a ese archivo en particular, para recompilar un solo objeto manualmente desde
* el men� contextual del arbol de proyecto.
**/
bool ProjectManager::PrepareForBuilding(file_item *only_one) {
	
	// borrar los pasos que hayan quedado incompletos de compilaciones anteriores
	// para preparar la nueva lista, se pone un nodo ficticio para facilitar la 
	// "insersion", que siempre sera al final, se evita preguntar si es el primero
	while (first_compile_step) {
		compile_step *s = first_compile_step->next;
		delete first_compile_step;
		first_compile_step=s;
	}
	warnings.Clear();

	compile_was_ok=true; // inicialmente no hay errores, se va a activar si un paso falla
	
	// si se encarga otro....
	if (current_toolchain.is_extern) {
		current_step=0;
		steps_count=1;
		SaveAll(false);
		first_compile_step = new compile_step(CNS_EXTERN,NULL);
		return true;
	}
	
	bool retval=false, relink_exe=false;
	config_analized=false;
	
	
	// prepara la info de las bibliotecas
	AssociateLibsAndSources(active_configuration);
	
	compile_step *step=first_compile_step=new compile_step(CNS_VOID,NULL);
	compile_extra_step *extra_step = only_one?NULL:active_configuration->extra_steps;
	current_step=steps_count=0;

	file_item *item = first_source;
	wxString full_path;
	wxDateTime bin_date, youngest_bin;
	youngest_bin.SetYear(1900);
	wxFileName bin_name;
	// crear el directorio para los objetos si no existe
	temp_folder_short=active_configuration->temp_folder;
	temp_folder=wxFileName(DIR_PLUS_FILE(path,active_configuration->temp_folder)).GetFullPath();
	if (temp_folder.Len() && !wxFileName::DirExists(temp_folder))
		wxFileName::Mkdir(temp_folder,0777,wxPATH_MKDIR_FULL);
	
	wxString extra_step_for_link;
		
	// guardar todos los fuentes abiertos
	SaveAll(false);
	if (!only_one) {
		if (use_wxfb) WxfbGenerate();
		
		// agregar los items extra previos a los fuentes
		while (extra_step && extra_step->pos==CES_BEFORE_SOURCES) {
			if (ShouldDoExtraStep(extra_step)) {
				steps_count++;
				step = step->next = new compile_step(CNS_EXTRA,extra_step);
				if (extra_step->out.Len()) {
					file_item *fitem = project->HasFile(DIR_PLUS_FILE(path,extra_step->out));
					if (fitem) fitem->force_recompile=true;
					if (extra_step->link_output) force_relink=true;
				}
				retval=true;
			}
			extra_step = extra_step->next;
		}
	}
	
	// marcar los headers que cambiaron por culpa de las macros
	wxArrayString dif_mac;
	if (!only_one && active_configuration->old_macros!=active_configuration->macros) {
		wxArrayString omac,nmac; 
		utils->Split(active_configuration->old_macros,omac,true,true);
		utils->Split(active_configuration->macros,nmac,true,true);
		int j;
		for (unsigned int i=0;i<omac.GetCount();i++)
			if ((j=nmac.Index(omac[i]))==wxNOT_FOUND)
				dif_mac.Add(omac[i]);
			else
				nmac.RemoveAt(j);
		for (unsigned int i=0;i<nmac.GetCount();i++)
			dif_mac.Add(nmac[i]);
		for (unsigned int i=0;i<dif_mac.GetCount();i++)
			if (dif_mac[i].Contains('='))
				dif_mac[i]=dif_mac[i].BeforeFirst('=');
		file_item *item = first_header;
		ML_ITERATE(item) {
			if (!item->force_recompile && DependsOnMacro(item,dif_mac))
				item->force_recompile=true;
		}
	}
	
	// ver si cambiaron 
	wxArrayString header_dirs_array;
	utils->Split(active_configuration->headers_dirs,header_dirs_array,true,false);
	for (unsigned int i=0;i<header_dirs_array.GetCount();i++) 
		header_dirs_array[i]=DIR_PLUS_FILE(path,header_dirs_array[i]);
	
	compile_step *prev_to_sources=step;
	wxDateTime now=wxDateTime::Now();
	bool flag;
	ML_ITERATE(item) {
		flag=false;
		if ( ( only_one && only_one==item ) || item->force_recompile) {
			flag=true;
		} else if (!only_one && (!active_configuration->dont_generate_exe || item->lib)) {
			full_path = DIR_PLUS_FILE(path,item->name);
			bin_name = DIR_PLUS_FILE(temp_folder,wxFileName(item->name).GetName()+_T(".o"));
			wxDateTime src_date=wxFileName(full_path).GetModificationTime();
			// nota: se usa getseconds porque comparar con < anda mal en windows (al menos en xp, funciona como <=)
			if ((now-src_date).GetSeconds().ToLong()<-3) { // si el fuente es del futuro, touch
				wxFileName(full_path).Touch();
				mxSource *src = main_window->IsOpen(item->item);
				if (src) src->Reload();
				warnings.Add(wxString(LANG(PROJMNGR_FUTURE_SOURCE_PRE,"El fuente "))<<full_path<<LANG(PROJMNGR_FUTURE_SOURCE_POST," tenia fecha de modificacion en el futuro. Se reemplazo por la fecha actual."));
				utils->AreIncludesUpdated(bin_date,full_path,header_dirs_array);
				flag=true;
			} else 
			if (bin_name.FileExists()) {
				bin_date = bin_name.GetModificationTime();
				if (bin_date>youngest_bin) // guardar el objeto mas nuevo para comparar con el ejecutable
					youngest_bin=bin_date;
				if (src_date>bin_date) // si el objeto esta desactualizado respecto al fuente 
					flag=true;
				else if (bin_date.IsLaterThan(now)) { // si el objeto es del futuro (por las dudas)
					warnings.Add(wxString(LANG(PROJMNGR_FUTURE_OBJECT_PRE,"El objeto "))<<full_path<<LANG(PROJMNGR_FUTURE_OBJECT_POST," tenia fecha de modificacion en el futuro. Se recompilara el fuente."));
					flag=true;
				} else { // ver si el objeto esta desactualizado respecto a los includes del fuente
					flag=utils->AreIncludesUpdated(bin_date,full_path,header_dirs_array);
					if (!flag && dif_mac.GetCount()) flag=DependsOnMacro(item,dif_mac); // si cambiaron las macros y el fuente las usa
				}
			} else { // si el objeto no existe, hay que compilar
				flag=true;
			}
		} else if (active_configuration->dont_generate_exe && !item->lib) {
			warnings.Add(wxString(LANG(PROJMNGR_NO_LIB_FOR_THAT_SOURCE_PRE,"El fuente \""))<<item->name<<LANG(PROJMNGR_NO_LIB_FOR_THAT_SOURCE_POST,"\" no esta asociado a ninguna biblioteca."));
		}
		if (flag) {
			step = step->next = new compile_step(CNS_SOURCE,item);
			if (item->lib)
				item->lib->need_relink=true;
			else
				relink_exe=true;
			steps_count++;
		}
	}
	if (prev_to_sources->next) prev_to_sources->next->start_parallel=true;
	step = step->next = new compile_step(CNS_BARRIER,NULL);
	if (!only_one) {
		// agregar los items extra previos al enlazado de bibliotecas
		while (extra_step && extra_step->pos==CES_BEFORE_LIBS) {
			if (ShouldDoExtraStep(extra_step)) {
				steps_count++; relink_exe=true;
				step = step->next = new compile_step(CNS_EXTRA,extra_step);
			}
			extra_step = extra_step->next;
		}
		
		// agregar el enlazado de bibliotecas
		project_library *lib = active_configuration->libs_to_build;
		while (lib) {
			if (lib->path.Len() && !wxFileName::DirExists(DIR_PLUS_FILE(path,lib->path)))
				wxFileName::Mkdir(DIR_PLUS_FILE(path,lib->path),0777,wxPATH_MKDIR_FULL);
			if (lib->need_relink || !wxFileName(DIR_PLUS_FILE(path,lib->filename)).FileExists()) {
				AnalizeConfig(path,true,config->mingw_real_path,false);
				if (lib->objects_list.Len()) {
					step = step->next = new compile_step(CNS_LINK,new linking_info(
						lib->is_static?current_toolchain.static_lib_linker:current_toolchain.dynamic_lib_linker,
						DIR_PLUS_FILE(path,lib->filename),lib->objects_list,lib->parsed_extra,&(lib->need_relink)));
					steps_count++;
					relink_exe=true;
				} else {
					warnings.Add(wxString(LANG(PROJMNGR_LIB_WITHOUT_SOURCES_PRE,"La biblioteca \""))<<lib->libname<<LANG(PROJMNGR_LIB_WITHOUT_SOURCES_POST,"\" no tiene ningun fuente asociado."));
				}
			}
			lib = lib->next;
		}
		
		// agregar los items extra posteriores al enlazado de blibliotecas
		while (extra_step && extra_step->pos==CES_BEFORE_EXECUTABLE) {
			if (ShouldDoExtraStep(extra_step)) {
				steps_count++; relink_exe=true;
				step = step->next = new compile_step(CNS_EXTRA,extra_step);
			}
			extra_step = extra_step->next;
		}
		

#if defined(_WIN32) || defined(__WIN32__)
		bool rc_redo=false;
		wxArrayString rc_text;
		if (active_configuration->icon_file.Len()) { // ver si hay que compilar el recurso del icono
			wxFileName ficon_in(DIR_PLUS_FILE(path,active_configuration->icon_file));
			if (ficon_in.FileExists()) {
				wxFileName ficon_out(DIR_PLUS_FILE(temp_folder,_T("zpr_resource.o")));
				if (ficon_in.FileExists() && (!ficon_out.FileExists() || ficon_in.GetModificationTime()>ficon_out.GetModificationTime() ) )
					rc_redo=true;
				wxString icon_name=active_configuration->icon_file;
				icon_name.Replace(_T("\\"),_T("\\\\"),true);
				rc_text.Add(ICON_LINE(icon_name));
			} else {
				warnings.Add(LANG(PROJMNGR_ICON_NOT_FOUND,"No se ha encontrado el icono del ejecutable."));
			}
		}
		if (active_configuration->manifest_file.Len()) { // ver si hay que compilar el recurso del manifest
			wxFileName ficon_in(DIR_PLUS_FILE(path,active_configuration->manifest_file));
			if (ficon_in.FileExists()) {
				wxFileName ficon_out(DIR_PLUS_FILE(temp_folder,_T("zpr_resource.o")));
				if (ficon_in.FileExists() && (!ficon_out.FileExists() || ficon_in.GetModificationTime()>ficon_out.GetModificationTime() ) )
					rc_redo=true;
				wxString icon_name=active_configuration->manifest_file;
				icon_name.Replace(_T("\\"),_T("\\\\"),true);
				rc_text.Insert("#include \"winuser.h\"",0);
				rc_text.Add(MANIFEST_LINE(icon_name));
			} else {
				warnings.Add(LANG(PROJMNGR_MANIFEST_NOT_FOUND,"No se ha encontrado el archivo manifest.xml."));
			}
		}
		wxFileName rc_file(DIR_PLUS_FILE(temp_folder,_T("zpr_resource.rc")));
		if (rc_text.GetCount()) { 
			// ver si cambio o falta el archivo .rc
			if (!rc_file.FileExists())
				rc_redo=true;
			else if (!rc_redo) {
				wxTextFile fil(rc_file.GetFullPath());
				fil.Open();
				wxString cont; 
				rc_redo=rc_text.GetCount()!=fil.GetLineCount();
				if (!rc_redo) {
					int i=0;
					for ( wxString str = fil.GetFirstLine(); !fil.Eof(); str = fil.GetNextLine() )
						if (rc_text[i++]!=str) rc_redo=true;
				}
				fil.Close();
			}
			if (rc_redo) {
				wxTextFile fil(rc_file.GetFullPath());
				if (fil.Exists())
					fil.Open();
				else
					fil.Create();
				fil.Clear();
				for (unsigned int i=0;i<rc_text.GetCount();i++)
					fil.AddLine(rc_text[i]);
				fil.Write();
				fil.Close();
				
				steps_count++; relink_exe=true;
				step = step->next = new compile_step(CNS_ICON,new wxString(DIR_PLUS_FILE(path,active_configuration->icon_file)));
			}
		} 
#endif
	
		// si ningun objeto esta desactualizado, ver si hay que relinkar
		if (!active_configuration->dont_generate_exe) {
			wxFileName fn_output(DIR_PLUS_FILE(path,active_configuration->output_file));
			if (!wxFileName::DirExists(fn_output.GetPath()) ) // si el directorio del exe no existe, crearl
				wxFileName::Mkdir(fn_output.GetPath(),0777,wxPATH_MKDIR_FULL);
			if (!relink_exe) { // si no hay que actualizar ningun objeto, preguntar por el exe
				executable_name=fn_output.GetFullPath();
				bin_name=executable_name;
				if (force_relink || !bin_name.FileExists() || bin_name.GetModificationTime()<youngest_bin)
					relink_exe=true;
			} else {
				relink_exe=true;
			}

			if (relink_exe) {
				AnalizeConfig(path,true,config->mingw_real_path,false);
				step = step->next = new compile_step(CNS_LINK,
					new linking_info(current_toolchain.linker+_T(" -o"),
					executable_name,objects_list,linking_options,&force_relink));
				steps_count++;
			}
		} else if (relink_exe) {
			AnalizeConfig(path,true,config->mingw_real_path,false);
		}
	} else  { // si only_one
		// configurar si hay que recompilar un solo fuente
		AnalizeConfig(path,true,config->mingw_real_path,false);
		relink_exe=true;
	}
	
	if (!only_one) {
		// agregar los items extra posteriores al reenlazado
		while (extra_step && extra_step->pos==CES_AFTER_LINKING) {
			if (ShouldDoExtraStep(extra_step)) {
				steps_count++;
				step = step->next = new compile_step(CNS_EXTRA,extra_step);
			}
			extra_step = extra_step->next;
		}
		// reestablecer la bandera force_recompile de los headers y guardar lista de macros
		active_configuration->old_macros=active_configuration->macros;
		ML_ITERATE(item) { item->force_recompile=false; }
	}
	
	// calcular la cantidad de pasos y el progreso en cada uno
	if (steps_count)
		progress_step=100.0/(steps_count);
	actual_progress = -progress_step/2;
	
	// eliminar el primer nodo ficticio de la lista de pasos
	step = first_compile_step->next;
	delete first_compile_step;
	first_compile_step = step;
	
	retval|=relink_exe;
	
	return retval;
}

/** 
* Guarda todos los archivos del proyecto que esten abiertos y eventualmente tambien
* el archivo de configuraci�n de proyecto. Si se indica guardar tambien este archivo,
* pero el proyecto no ha cambiado, no lo reescribe.
* @param save_project determina si tambi�n se debe guardar el archivo de configuraci�n de proyecto
**/
void ProjectManager::SaveAll(bool save_project) {
	file_item *item;
	mxSource *source;
	// guardar los fuentes
	item = first_source;
	ML_ITERATE(item) {
		if ((source=main_window->IsOpen(DIR_PLUS_FILE(path,item->name))) && source->GetModify()) {
			source->SaveSource();
			parser->ParseSource(source,true);
		}
	}
	// guardar las cabeceras
	item = first_header;
	ML_ITERATE(item) {
		if ( (source=main_window->IsOpen(DIR_PLUS_FILE(path,item->name))) && source->GetModify()) {
			source->SaveSource();
			parser->ParseSource(source,true);
		}
	}
	// guardar los otros
	item = first_other;
	ML_ITERATE(item) {
		if ( (source=main_window->IsOpen(DIR_PLUS_FILE(path,item->name))) && source->GetModify()) {
			source->SaveSource();
//			parser->ParseSource(source,true);
		}
	}
	// guardar la configuracion del proyecto
	if (save_project && modified)
		Save();
}


long int ProjectManager::CompileFile(compile_and_run_struct_single *compile_and_run, file_item *item) {
	compile_and_run->step_label=item->name;
	compile_and_run->compiling=true;
	item->force_recompile=false;
	
	// preparar la linea de comando 
	wxFileName bin_name = DIR_PLUS_FILE(temp_folder,wxFileName(item->name).GetName()+_T(".o"));
	bool cpp = (item->name[item->name.Len()-1]|32)!='c' || item->name[item->name.Len()-2]!='.';
	wxString command = wxString(cpp?current_toolchain.cpp_compiler:current_toolchain.c_compiler)+
#if !defined(_WIN32) && !defined(__WIN32__)
		(item->lib?_T(" -fPIC "):_T(" "))+
#endif
		compiling_options+_T(" \"")+DIR_PLUS_FILE(path,item->name)+_T("\" -c -o \"")+bin_name.GetFullPath()+_T("\"");
	
	compile_and_run->process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_COMPILE);
	compile_and_run->process->Redirect();
	
	// ejecutar
	compile_and_run->output_type=MXC_GCC;
	compile_and_run->full_output.Add(_T(""));
	compile_and_run->full_output.Add(wxString(_T("> "))+command);
	compile_and_run->full_output.Add(_T(""));
	compile_and_run->last_all_item = main_window->compiler_tree.treeCtrl->AppendItem(main_window->compiler_tree.all,command,2);
	return utils->Execute(path,command, wxEXEC_ASYNC|wxEXEC_MAKE_GROUP_LEADER,compile_and_run->process);	
}


long int ProjectManager::CompileFile(compile_and_run_struct_single *compile_and_run, wxFileName filename) {
	AnalizeConfig(path,true,config->mingw_real_path);
	// crear el directorio para los objetos si no existe
	if (!wxFileName::DirExists(temp_folder))
		wxFileName::Mkdir(temp_folder,0777,wxPATH_MKDIR_FULL);
	file_item *item;
	item = first_source;
	ML_ITERATE(item) {
		if (wxFileName(DIR_PLUS_FILE(path,item->name))==filename)
			return CompileFile(compile_and_run,item);
	}
	return 0;

}

/** 
* Compila un fuente o enlaza el ejecutable si ya estan todos los objetos listos.
* El paso que ejecuta es el primero de la lista (first_compile_step), y adem�s 
* lo saca de esta lista. Puede fallar si ya no quedan pasos o si hay otros pasos
* en paralelos que deben finalizar antes de continuar (esto es cuando el pr�ximo
* paso es de tipo CNS_BARRIER). Se encarga adem�s de lanzar otros pasos en paralelo 
* si es el primero de un conjunto paralelizable (lo cual se indica en la propiedad 
* start_parallel del paso). Devuelve el pid del proceso que lanzo para 
* compilar/enlazar o 0 en caso contrario. Coloca el nombre corto del paso que 
* est� ejecutando en caption para que se muestre en la interfaz.
**/
long int ProjectManager::CompileNext(compile_and_run_struct_single *compile_and_run, wxString &caption) {

	if (first_compile_step && first_compile_step->type==CNS_BARRIER) {
		if (compiler->NumCompilers()>1) {
			return 0;
		} else {
			compile_step *step = first_compile_step;
			first_compile_step = step->next;
			delete step;
			return CompileNext(compile_and_run,caption);
		}
	}

	compile_and_run->pid=0;
	
	main_window->SetStatusProgress( int(actual_progress+=progress_step) );
	main_window->SetStatusText(wxString(LANG(PROJMNGR_COMPILING,"Compilando"))<<_T("... ( ")<<LANG(PROJMNGR_STEP_FROM_PRE,"paso ")<<++current_step<<LANG(PROJMNGR_STEP_FROM_PORT," de ")<<steps_count<<_T(" - ")<<int(actual_progress)<<_T("% )"));
	
	if (first_compile_step) {
		compile_step *step = first_compile_step;
		switch (step->type) {
		case CNS_EXTERN: // for custom extern toolchains
			caption="";
			compile_and_run->pid = CompileWithExternToolchain(compile_and_run);
			break;
		case CNS_ICON:
			caption=wxString(LANG(PROJMNGR_COMPILING,"Compilando"))<<_T(" \"")<<((file_item*)step->what)->name<<_T("\"...");
			compile_and_run->pid = CompileIcon(compile_and_run,*((wxString*)step->what));
			delete ((wxString*)step->what);
			break;
		case CNS_SOURCE:
			compile_and_run->pid = CompileFile(compile_and_run,(file_item*)step->what);
			// ver si hay que lanzar otra en paralelo
			if (step->start_parallel) {
				int nc=1;
				while (step->next && step->next->type==CNS_SOURCE && nc<config->Init.max_jobs) {
					first_compile_step = step->next; delete step; step=first_compile_step; current_step++;
					compile_and_run_struct_single *compile_and_run_2=new compile_and_run_struct_single(compile_and_run);
					compile_and_run_2->pid=CompileFile(compile_and_run_2,(file_item*)step->what); nc++;
					if (compile_and_run_2->pid<=0) {
						delete compile_and_run_2; break;
					}
				}
			}
			caption=compiler->GetCompilingStatusText();
			break;
		case CNS_EXTRA:
			caption=wxString(LANG(PROJMNGR_RUNNING,"Ejecutando"))<<_T(" \"")<<((compile_extra_step*)step->what)->name<<_T("\"...");
			compile_and_run->pid = CompileExtra(compile_and_run,(compile_extra_step*)step->what);
			break;
		case CNS_LINK:
			if (!compile_was_ok) return 0;
			caption=wxString(LANG(PROJMNGR_LINKING,"Enlazando \""))<<((linking_info*)step->what)->output_file<<_T("\"...");
			compile_and_run->pid = Link(compile_and_run,(linking_info*)step->what);
			delete ((linking_info*)step->what);
			break;
		default:
			;
		}
		first_compile_step = step->next;
		delete step;
	}
	return compile_and_run->pid;
}

long int ProjectManager::Link(compile_and_run_struct_single *compile_and_run, linking_info *info) {
	(*info->flag_relink)=false;
	wxString command = info->command;
	command<<_T(" ")+utils->Quotize(info->output_file)+_T(" ");
	command<<info->objects_list+_T(" ")+info->extra_args;
	compile_and_run->process=new wxProcess(main_window->GetEventHandler(),mxPROCESS_COMPILE);
	compile_and_run->process->Redirect();
	compile_and_run->output_type=MXC_SIMIL_GCC;
	compile_and_run->linking=true; // para que cuando termine sepa que enlazo, para verificar llamar a compiler->CheckForExecutablePermision
	compile_and_run->last_all_item = main_window->compiler_tree.treeCtrl->AppendItem(main_window->compiler_tree.all,command,2);
	compile_and_run->full_output.Add(_T(""));
	compile_and_run->full_output.Add(wxString(_T("> "))+command);
	compile_and_run->full_output.Add(_T(""));
	return utils->Execute(path, command, wxEXEC_ASYNC,compile_and_run->process);
}

long int ProjectManager::Run(compile_and_run_struct_single *compile_and_run) {
	// ver que no sea un proyecto sin ejecutable
	if (active_configuration->dont_generate_exe) {
		mxMessageDialog(main_window,LANG(PROJMNGR_RUNNING_NO_EXE,"Este proyecto no puede ejecutarse porque esta configurado\npara generar solo bibliotecas."),LANG(GENERAL_WARNING,"Aviso"),mxMD_OK|mxMD_WARNING).ShowModal();
		return 0;
	}
	compile_and_run->linking=compile_and_run->compiling=false;
	
	// agregar el prefijo para valgrind
	wxString exe_pref;
#if !defined(_WIN32) && !defined(__WIN32__)
	if (compile_and_run->valgrind_cmd.Len())
		exe_pref = compile_and_run->valgrind_cmd+_T(" ");
#endif
	// armar la linea de comando para ejecutar
	executable_name=wxFileName(DIR_PLUS_FILE(path,active_configuration->output_file)).GetFullPath();
	wxString command(config->Files.terminal_command);
	command.Replace("${TITLE}",LANG(GENERA_CONSOLE_CAPTION,"ZinjaI - Consola de Ejecucion"));
	if (command.Len()!=0) {
		if (command==_T(" ") )
			command=_T("");
		else if (command[command.Len()-1]!=' ') 
			command<<_T(" ");
	}
	wxString working_path = (active_configuration->working_folder==_T(""))?path:DIR_PLUS_FILE(path,active_configuration->working_folder);
	if (working_path.Last()==path_sep)
		working_path.RemoveLast();
	if (active_configuration->console_program) { // si es de consola, ver si hay que esperar y pasar la ruta de trabajo
		command<<_T("\"")<<config->Files.runner_command<<_T("\" ");
		command<<_T("-lang \"")<<config->Init.language_file<<_T("\" ");
		if (active_configuration->wait_for_key==2)
			command<<_T("-waitkey ");
		else if (active_configuration->wait_for_key==1)
			command<<_T("-waitkey-onerror ");
		command<<_T("\"")<<working_path<<_T("\" ")<<exe_pref<<_T("\"")<<wxFileName(executable_name).GetShortPath()<<_T("\"");
	} else { // si no es de consola, se ejecuta derecho, sin el runner
		command=exe_pref<<wxString(_T("\""))<<wxFileName(executable_name).GetFullPath()<<_T("\"");
	}
	
//	utils->ParameterReplace(command,_T("${ZINJAI_DIR}"),wxGetCwd());
	
	// agregar los argumentos de ejecucion
	if (active_configuration->always_ask_args) {
		int res = mxArgumentsDialog(main_window,active_configuration->args).ShowModal();
		if (res&AD_CANCEL) return 0;
		if (res&AD_ARGS) {
			active_configuration->args = mxArgumentsDialog::last_arguments;;
			command<<' '<<active_configuration->args;
		}
		if (res&AD_REMEMBER) {
			active_configuration->always_ask_args=false;
			if (res&AD_EMPTY) active_configuration->args=_T("");
		}
	} else if (active_configuration->args.Len())
		command<<' '<<active_configuration->args;	
	
	// lanzar la ejecucion
#if defined(__WIN32__)
	wxString ldlp_vname="PATH";
	wxString ldlp_sep=";";
#else
	wxString ldlp_vname="LD_LIBRARY_PATH";
	wxString ldlp_sep=":";
#endif
	
	wxString ld_library_path;
	wxGetEnv(ldlp_vname,&ld_library_path);
	wxString old_ld_library_path=ld_library_path;
	project_library *lib = active_configuration->libs_to_build;
	while (lib) {
		if (!lib->is_static) {
			if (ld_library_path.Len())
				ld_library_path<<ldlp_sep;
			ld_library_path<<utils->Quotize(DIR_PLUS_FILE(path,lib->path));
		}
		lib = lib->next;
	}
	wxSetEnv(ldlp_vname,ld_library_path);
	
	int pid;
	compile_and_run->process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_COMPILE);
	if (active_configuration->console_program)
		pid = wxExecute(command, wxEXEC_NOHIDE|wxEXEC_MAKE_GROUP_LEADER, compile_and_run->process);
	else
		pid = utils->Execute(working_path,command,wxEXEC_ASYNC|wxEXEC_MAKE_GROUP_LEADER,compile_and_run->process);

#if !defined(_WIN32) && !defined(__WIN32__)
	wxSetEnv(_T("LD_LIBRARY_PATH"),old_ld_library_path);
#endif
	return compile_and_run->pid=pid;
}


project_configuration *ProjectManager::GetConfig(wxString name) {
	for (int i=0;i<configurations_count;i++)
		if (configurations[i]->name==name)
			return configurations[i];
	return NULL;
}

void ProjectManager::ExportMakefile(wxString make_file, bool exec_comas, wxString mingw_dir,makefile_type mktype) {
	
	wxString old_temp_folder = active_configuration->temp_folder;
	if (mktype!=MKTYPE_FULL) active_configuration->temp_folder="${OBJS_DIR}";
	
	AnalizeConfig(_T(""),exec_comas,mingw_dir);
	if (executable_name.Contains(_T(" ")))
		executable_name=wxString(_T("\""))+executable_name+_T("\"");
	
	wxFileName bin_name;
	wxString tab(_T("\t"));
	
	// abrir el archivo (crear o pisar)
	wxTextFile fil(make_file);
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	
	// definir las variables
	if (mktype==MKTYPE_CONFIG) fil.AddLine(wxString(_T("OBJS_DIR="))+old_temp_folder);
	if (mktype!=MKTYPE_OBJS) {
		if (mingw_dir==_T("${MINGW_DIR}"))
			fil.AddLine(wxString(_T("MINGW_DIR="))+config->mingw_real_path);
		fil.AddLine(wxString(_T("GPP="))+current_toolchain.cpp_compiler);
		fil.AddLine(wxString(_T("GCC="))+current_toolchain.c_compiler);
		fil.AddLine(wxString(_T("FLAGS="))+compiling_options);
		fil.AddLine(wxString(_T("LIBS="))+linking_options);
	}
	
	wxString libs_deps;
	project_library *lib = active_configuration->libs_to_build;
	while (lib) {
		if (lib->objects_list.Len()) libs_deps<<" "<<lib->objects_list;
		lib = lib->next;
	}
	
	if (mktype!=MKTYPE_CONFIG) fil.AddLine(wxString(_T("OBJS="))+exe_deps);
	if (libs_deps.Len()&&mktype!=MKTYPE_CONFIG) fil.AddLine(wxString(_T("LIBS_OBJS="))+libs_deps);
	fil.AddLine(_T(""));
	
	if (mktype!=MKTYPE_OBJS) {
		// agregar las secciones all, clean, y la del ejecutable
		fil.AddLine(wxString(_T("all: "))+temp_folder+_T(" ")+executable_name);
		fil.AddLine(_T(""));
		
		if (mktype==MKTYPE_CONFIG) { 
			fil.AddLine("include Makefile.common");
			fil.AddLine(_T(""));
		}

		compile_extra_step *estep=active_configuration->extra_steps;
		wxString extra_pre, extra_post;
		while (estep) {
			if (estep->out.Len() && estep->delete_on_clean) {
				if (estep->pos!=CES_AFTER_LINKING) {
					extra_pre<<_T(" ");
					extra_pre<<estep->out;
				} else {
					extra_post<<_T(" ");
					extra_post<<estep->out;
				}
			}
			estep=estep->next;
		}
		utils->ParameterReplace(extra_post,_T("${TEMP_DIR}"),temp_folder);
		utils->ParameterReplace(extra_pre,_T("${TEMP_DIR}"),temp_folder);
		utils->ParameterReplace(extra_post,_T("${PROJECT_PATH}"),project->path);
		utils->ParameterReplace(extra_pre,_T("${PROJECT_PATH}"),project->path);
		utils->ParameterReplace(extra_post,_T("${PROJECT_BIN}"),executable_name);
		utils->ParameterReplace(extra_pre,_T("${PROJECT_BIN}"),executable_name);
		
		fil.AddLine(_T("clean:"));
	#if defined(_WIN32) || defined(__WIN32__)
		fil.AddLine(wxString(_T("\tdel ${OBJS} "))+executable_name+extra_pre+extra_post+(libs_deps.Len()?" ${LIBS_OBJS}":""));
	#else
		fil.AddLine(wxString(_T("\trm -rf ${OBJS} "))+executable_name+extra_pre+extra_post+(libs_deps.Len()?" ${LIBS_OBJS}":""));
	#endif
		fil.AddLine(_T(""));

//		compile_extra_step *estep=active_configuration->extra_steps;
//		wxString extra_deps;
//		while (estep) {
//			if (estep->out.Len()) {
//				extra_deps<<_T(" ");
//				extra_deps<<estep->out;
//			}
//			estep=estep->next;
//		}
//		utils->ParameterReplace(extra_deps,_T("${TEMP_DIR}"),temp_folder);
		
		fil.AddLine(executable_name+_T(": ${OBJS}")/*+extra_deps*/+extra_pre);
		fil.AddLine(tab+_T("${GPP} ${OBJS} ${LIBS} -o $@"));
		fil.AddLine(_T(""));

		if (temp_folder.Len()!=0) {
			fil.AddLine(temp_folder+_T(":"));
			fil.AddLine(_T("\tmkdir ")+temp_folder);
			fil.AddLine(_T(""));
		}
	
		// agregar las secciones de los pasos extra 
		/// @todo: falta ver como forzar la ejecucion y el orden
		
		estep=active_configuration->extra_steps;
		while (estep) {
			if (estep->out.Len()) {
				wxString command = estep->command;
				wxString output = estep->out;
				wxString deps = estep->deps;
				utils->ParameterReplace(deps,"${TEMP_DIR}",project->temp_folder);
				utils->ParameterReplace(deps,"${PROJECT_PATH}",project->path);
				utils->ParameterReplace(deps,"${PROJECT_BIN}",project->executable_name);
				utils->ParameterReplace(output,"${TEMP_DIR}",project->temp_folder);
				utils->ParameterReplace(output,"${PROJECT_PATH}",project->path);
				utils->ParameterReplace(output,"${PROJECT_BIN}",project->executable_name);
				utils->ParameterReplace(command,"${TEMP_DIR}",project->temp_folder);
				utils->ParameterReplace(command,"${PROJECT_PATH}",project->path);
				utils->ParameterReplace(command,"${PROJECT_BIN}",project->executable_name);
#if defined(_WIN32) || defined(__WIN32__)
				utils->ParameterReplace(output,"${MINGW_DIR}",mingw_dir);
				utils->ParameterReplace(deps,"${MINGW_DIR}",mingw_dir);
				utils->ParameterReplace(command,"${MINGW_DIR}",mingw_dir);
#endif
				utils->ParameterReplace(command,"${DEPS}",deps);
				utils->ParameterReplace(command,"${OUTPUT}",output);
				fil.AddLine(output+_T(": ")+deps);
				fil.AddLine(_T("\t")+command);
				fil.AddLine(_T(""));
			}
			estep=estep->next;
		}
	
		// agregar las bibliotecas
		project_library *lib = active_configuration->libs_to_build;
		while (lib) {
			wxString libdep = utils->Quotize(lib->filename);
			wxString objs;
			libdep<<_T(":");
			file_item *item=first_source;
			ML_ITERATE(item) {
				if (item->lib==lib) {
					wxString bin_name = DIR_PLUS_FILE(temp_folder,wxFileName(item->name).GetName()+_T(".o"));
					objs<<_T(" ")<<utils->Quotize(bin_name);
				}
			}
			fil.AddLine(libdep+objs);
			fil.AddLine(tab+(lib->is_static?current_toolchain.static_lib_linker:current_toolchain.dynamic_lib_linker)+_T(" ")+utils->Quotize(lib->filename)+objs+_T(" ")+lib->extra_link);
			fil.AddLine(_T(""));
			lib = lib->next;
		}
	}

	if (mktype!=MKTYPE_CONFIG) {
		// agregar las secciones de los objetos
		wxArrayString header_dirs_array;
		utils->Split(active_configuration->headers_dirs,header_dirs_array,true,false);
		for (unsigned int i=0;i<header_dirs_array.GetCount();i++) 
			header_dirs_array[i]=DIR_PLUS_FILE(path,header_dirs_array[i]);
		
		file_item *item=first_source;
		wxString bin_full_path;
		ML_ITERATE(item) {
			bin_name = DIR_PLUS_FILE(temp_folder,wxFileName(item->name).GetName()+_T(".o"));
			bin_full_path=utils->Quotize(bin_name.GetFullPath());
			fil.AddLine(bin_full_path+_T(": ")+utils->FindIncludes(DIR_PLUS_FILE(path,item->name),path,header_dirs_array));
			bool cpp = (item->name[item->name.Len()-1]|32)!='c' || item->name[item->name.Len()-2]!='.';
			fil.AddLine(tab+(cpp?_T("${GPP}"):_T("${GCC}"))+_T(" ${FLAGS} ")+
	#if !defined(_WIN32) && !defined(__WIN32__)
				(item->lib?_T("-fPIC "):_T(""))+
	#endif
				_T("-c ")+item->name+_T(" -o $@"));
			fil.AddLine(_T(""));
		}
	}
	
	// guardar y cerrar
	fil.Write();
	fil.Close();
	
	active_configuration->temp_folder=old_temp_folder;
	
}

void ProjectManager::Clean() {
	
	if (current_toolchain.is_extern) {
		compile_and_run_struct_single *compile_and_run=new compile_and_run_struct_single("clean");
		compile_and_run->killed=true; // para que no lo procese el evento main_window->ProcessKilled
		compile_and_run->pid=CompileWithExternToolchain(compile_and_run,false);
		return;
	}
	
	wxString file;
	// preparar las rutas y nombres adecuados
	AnalizeConfig(path,true,config->mingw_real_path);
	// borrar los objetos
	file_item *item=first_source;
	ML_ITERATE(item) {
		file=DIR_PLUS_FILE(temp_folder,wxFileName(item->name).GetName()+_T(".o"));
		if (wxFileName::FileExists(file))
			wxRemoveFile(file);
	}
	// borrar las salidas de los pasos adicionales
	compile_extra_step *step = active_configuration->extra_steps;
	while (step) {
		if (step->delete_on_clean&&step->out.Len()){
			wxString out=step->out;
			utils->ParameterReplace(out,"${TEMP_DIR}",temp_folder);
			utils->ParameterReplace(out,"${PROJECT_BIN}",executable_name);
			utils->ParameterReplace(out,"${PROJECT_PATH}",path);
#ifdef __WIN32__
			utils->ParameterReplace(out,"${MINGW_DIR}",config->mingw_real_path);
#endif
			file=DIR_PLUS_FILE(path,out);
			if (wxFileName::FileExists(file))
				wxRemoveFile(file);
		}
		step = step->next;
	}
	// borrar temporales del icono
	if (wxFileName::FileExists(DIR_PLUS_FILE(temp_folder,_T("zpr_resource.rc"))))
		wxRemoveFile(DIR_PLUS_FILE(temp_folder,_T("zpr_resource.rc")));
	if (wxFileName::FileExists(DIR_PLUS_FILE(temp_folder,_T("zpr_resource.o"))))
		wxRemoveFile(DIR_PLUS_FILE(temp_folder,_T("zpr_resource.o")));

	// borrar las bibliotecas
	project_library *lib = active_configuration->libs_to_build;
	while (lib) {
		file=DIR_PLUS_FILE(path,lib->filename);
		if (wxFileName::FileExists(file))
			wxRemoveFile(file);
		lib = lib->next;
	}
	
	// borrar el ejecutable
	if (wxFileName::FileExists(executable_name))
		wxRemoveFile(executable_name);
}


/**
* Cuando se invoca al compilador se requiere una lista de argumentos que dependen
* directa o indirectamente de varios campos del cuadro de Opciones de Compilaci�n 
* y Ejecuci�n del Proyecto. No es c�modo convertir estos campos en argumentos
* para gcc cada vez que se los necesita, y adem�s tampoco es eficiente, ya que por
* ejemplo puede requerir ejecutar comandos externos como pkg-config, y se los
* necesita al menos una vez por cada objeto a generar en el proyecto. Esta funci�n
* se encarga de hacerlo una vez antes de compilar y guardar los resultados en los
* atributos de esta clase marcados como "temporales".
* @param path        es el directorio de base a utilizar para todas las rutas
*                    relativas. �n la salida de esta funci�n se utiliza este path
*                    para convertirlas en absolutas. Usualmente ser� la carpeta
*                    del proyecto, pero a la hora de generar el makefile se deja 
*                    en blanco, para que se mantengan relativas.
* @param exec_comas  indica si se deben reemplazar las cadenas entre acentos
*                    en los campos de argumentos adicionales o si se dejan 
*                    como estan (al compilar en zinjai se ejecutan, al generar
*                    el makefile suele ser convieniente dejarlos para que 
*                    los ejecute make).
* @param mignw_dir   indica con que valor se deben reemplazar las ocurrencias de
*                    la variable ${MINGW_DIR} en las opciones. Es el valor real
*                    al compilar en zinjai, pero conviene queda como variable
*                    al generar el makefile.
* @param force       hay un bool config_analized que indica que la configuraci�n
*                    ya est� actualizada, cuyo caso no hace nada. Este bool sirve
*                    para omitir esa comprobaci�n y hacer el proceso obligatoriamente.
**/
void ProjectManager::AnalizeConfig(wxString path, bool exec_comas, wxString mingw_dir, bool force) { // parse active_configuration to generate the parts of the command line
	
	if (!force && config_analized) return;
	
	Toolchain::SelectToolchain();
	
	temp_folder_short=active_configuration->temp_folder;
	temp_folder=wxFileName(DIR_PLUS_FILE(path,active_configuration->temp_folder)).GetFullPath();
	compiling_options=" ";
	compiling_options<<current_toolchain.cpp_compiling_options<<" ";
	
	// debug_level
	if (active_configuration->debug_level==1) 
		compiling_options<<config->Debug.format<<_T(" -g1 ");
	else if (active_configuration->debug_level==2) 
		compiling_options<<config->Debug.format<<_T(" -g2 ");
	else if (active_configuration->debug_level==3) 
		compiling_options<<config->Debug.format<<_T(" -g3 ");
	// enable profiling information
	if (active_configuration->enable_profiling) 
		compiling_options<<_T("-pg ");
	// warnings_level
	if (active_configuration->warnings_level==0) 
		compiling_options<<_T("-w ");
	else if (active_configuration->debug_level==2) 
		compiling_options<<_T("-Wall ");
	// warnings_level
	if (active_configuration->optimization_level==0) 
		compiling_options<<_T("-O0 ");
	else if (active_configuration->optimization_level==1) 
		compiling_options<<_T("-O1 ");
	else if (active_configuration->optimization_level==2) 
		compiling_options<<_T("-O2 ");
	else if (active_configuration->optimization_level==3) 
		compiling_options<<_T("-O3 ");
	else if (active_configuration->optimization_level==4) 
		compiling_options<<_T("-Os ");
	// ansi_compliance
	if (active_configuration->ansi_compliance)
		compiling_options<<_T("-pedantic-errors ");
	// headers_dirs
	compiling_options<<utils->Split(active_configuration->headers_dirs,_T("-I"));
	// parametros variables
	wxString compiling_extra = active_configuration->compiling_extra;
	utils->ParameterReplace(compiling_extra,_T("${MINGW_DIR}"),mingw_dir);
	utils->ParameterReplace(compiling_extra,_T("${TEMP_DIR}"),temp_folder_short);
	utils->ParameterReplace(compiling_options,_T("${MINGW_DIR}"),mingw_dir);
	utils->ParameterReplace(compiling_options,_T("${TEMP_DIR}"),temp_folder_short);
	// reemplazar subcomandos y agregar extras
	if (exec_comas)
		compiling_options<<utils->ExecComas(path, compiling_extra);
	else
		compiling_options<<compiling_extra;
	// macros predefinidas
	if (active_configuration->macros.Len())
		compiling_options<<_T(" ")<<utils->Split(active_configuration->macros,_T("-D"));
		
	linking_options=" ";
	linking_options<<current_toolchain.cpp_linker_options<<" ";
	// mwindows
#if defined(_WIN32) || defined(__WIN32__)
	if (!active_configuration->console_program)
		linking_options<<_T("-mwindows ");
#endif
	// enable profiling information
	if (active_configuration->enable_profiling) 
		linking_options<<_T("-pg ");
	// strip
	if (active_configuration->strip_executable)
		linking_options<<_T("-s ");
	// directorios para bibliotecas
	linking_options<<utils->Split(active_configuration->libraries_dirs,_T("-L"));
	// bibliotecas
	linking_options<<utils->Split(active_configuration->libraries,_T("-l"));
	// reemplazar variables
	wxString linking_extra = active_configuration->linking_extra;
	utils->ParameterReplace(linking_extra,_T("${MINGW_DIR}"),mingw_dir);
	utils->ParameterReplace(linking_extra,_T("${TEMP_DIR}"),temp_folder_short);
	utils->ParameterReplace(linking_options,_T("${MINGW_DIR}"),mingw_dir);
	utils->ParameterReplace(linking_options,_T("${TEMP_DIR}"),temp_folder_short);
	// reemplazar subcomandos y agregar extras
	if (exec_comas)
		linking_options<<_T(" ")<<utils->ExecComas(path,linking_extra);
	else
		linking_options<<_T(" ")<<linking_extra;

	executable_name=wxFileName(DIR_PLUS_FILE(path,active_configuration->output_file)).GetFullPath();
	
	// bibliotecas
	project_library *lib = active_configuration->libs_to_build;
	while (lib) {
		lib->objects_list.Clear();
		lib = lib->next;
	}	
	
	AssociateLibsAndSources(active_configuration);
	
	objects_list=_T("");
#if defined(_WIN32) || defined(__WIN32__)
	if (
		( active_configuration->icon_file.Len() && wxFileName::FileExists(DIR_PLUS_FILE(path,active_configuration->icon_file)) )
		||
		( active_configuration->manifest_file.Len() && wxFileName::FileExists(DIR_PLUS_FILE(path,active_configuration->manifest_file)) ) 
		)
			objects_list<<utils->Quotize(DIR_PLUS_FILE(temp_folder,_T("zpr_resource.o")))<<_T(" ");
#endif
	
	wxString extra_step_objs;
	compile_extra_step *step = active_configuration->extra_steps;
	while (step) {
		if (step->pos!=CES_AFTER_LINKING && step->out.Len() && step->link_output) {
			extra_step_objs<<step->out<<" ";
		}
		step = step->next;
	}
	if (extra_step_objs.Len()) {
		utils->ParameterReplace(extra_step_objs,_T("${TEMP_DIR}"),temp_folder);
		utils->ParameterReplace(extra_step_objs,_T("${TEMP_DIR}"),temp_folder);
		utils->ParameterReplace(extra_step_objs,_T("${PROJECT_PATH}"),project->path);
		utils->ParameterReplace(extra_step_objs,_T("${PROJECT_PATH}"),project->path);
		utils->ParameterReplace(extra_step_objs,_T("${PROJECT_BIN}"),executable_name);
		utils->ParameterReplace(extra_step_objs,_T("${PROJECT_BIN}"),executable_name);
		objects_list<<" "<<extra_step_objs;
	}
	
	file_item *item = first_source;
	wxFileName bin_name;
	ML_ITERATE(item) {
		bin_name = DIR_PLUS_FILE(temp_folder,wxFileName(item->name).GetName()+_T(".o"));
		wxString *olist = item->lib?&(item->lib->objects_list):&objects_list;
		(*olist)<<utils->Quotize(bin_name.GetFullPath())<<_T(" ");
	}
	
	exe_deps = objects_list;

	lib = active_configuration->libs_to_build;
	while (lib) {
		if (lib->objects_list.Len()) {
			lib->objects_list.RemoveLast();
			lib->parsed_extra = utils->ExecComas(path,lib->extra_link);
			utils->ParameterReplace(lib->parsed_extra,_T("${MINGW_DIR}"),mingw_dir);
			utils->ParameterReplace(lib->parsed_extra,_T("${TEMP_DIR}"),temp_folder_short);
			bin_name = DIR_PLUS_FILE(path,lib->filename);
			wxString libfile = lib->is_static ? bin_name.GetFullPath():wxString(_T("-l"))<<lib->libname;
				objects_list<<utils->Quotize(libfile)<<_T(" ");
			if (!lib->is_static) {
				bin_name.MakeRelativeTo(path);
				if (bin_name.GetPath().Len())
					linking_options<<_T(" -L")<<utils->Quotize(bin_name.GetPath());
				else
					linking_options<<_T(" -L./");
			}
			exe_deps<<utils->Quotize(lib->filename)<<_T(" ");
		}
		lib = lib->next;
	}	
	objects_list.RemoveLast();
	
}

void ProjectManager::GetSourceExtras(mxSource *source, file_item *item) {
	if (!item) item = FindFromItem(source->treeId);
	if (!item) return;
	// reacomodar los nros de linea de los pts de interrupcion
	BreakPointInfo *bitem=item->breaklist;
	while (bitem) {	bitem->UpdateLineNumber(); bitem=bitem->Next(); }
	// guardar los bookmarks
	int l = source->GetLineCount();
	item->ClearExtras();
	int m;
	for (int i=0;i<l;i++) {
		m = source->MarkerGet(i);
		if (m&1<<mxSTC_MARK_USER)
			item->markers = new marked_line_item (i,item->markers);
	}
	// guardar otras cosas
	item->cursor = source->GetCurrentPos();
}

void ProjectManager::SetSourceExtras(mxSource *source, file_item *item) {
	if (!item) {
		wxFileName fname = source->source_filename;
		fname.MakeRelativeTo(path);
		wxString name = fname.GetFullPath();
		if (!item) {
			item = first_source->next;
			while (item) {
				if (item->name == name) {
					break;
				}
				item=item->next;
			}
		}
		if (!item) {
			item = first_header->next;
			while (item) {
				if (item->name == name) {
					break;
				}
				item=item->next;
			}
		}
		if (!item) {
			item = first_other->next;
			while (item) {
				if (item->name == name) {
					break;
				}
				item=item->next;
			}
		}
		if (item)
			source->treeId = item->item;
	}
	if (item) {
		if (source->own_breaks) delete source->breaklist;
		source->breaklist = &item->breaklist;
		source->own_breaks = false;
		BreakPointInfo *bpi=item->breaklist;
		while (bpi) {
			bpi->SetSource(source);
			bpi=bpi->Next();
		}
		marked_line_item *marker = item->markers;
		while (marker) {
			source->MarkerAdd(marker->line, mxSTC_MARK_USER);
			marker = marker->next;
		}
		source->GotoPos(item->cursor);
	}
}

/**
* Agrega los archivos del proyecto al arreglo array. El arreglo no se vacia
* antes de comenzar. Cuales puede ser 's' para fuentes, 'h' para cabeceras,
* 'o' para otros, y '*' para todos.
**/
int ProjectManager::GetFileList(wxArrayString &array, char cuales, bool relative_paths) { // cuales='o','s','h','*'
	int i=0;

	file_item *item;
	if (cuales=='*' || cuales=='s') {
		item = first_source;
		ML_ITERATE(item) {
			i++;
			if (relative_paths)
				array.Add(item->name);
			else
				array.Add(DIR_PLUS_FILE(path,item->name));
		}
	}
	if (cuales=='*' || cuales=='h') {
		item = first_header;
		ML_ITERATE(item) {
			i++;
			if (relative_paths)
				array.Add(item->name);
			else
				array.Add(DIR_PLUS_FILE(path,item->name));
		}
	}
	if (cuales=='*' || cuales=='o') {
		item = first_other;
		ML_ITERATE(item) {
			i++;
			if (relative_paths)
				array.Add(item->name);
			else
				array.Add(DIR_PLUS_FILE(path,item->name));
		}
	}
	
	return i;
}

bool ProjectManager::Debug() {
	if (active_configuration->dont_generate_exe) {
		mxMessageDialog(main_window,LANG(PROJMNGR_RUNNING_NO_EXE,"Este proyecto no puede ejecutarse porque esta configurado\npara generar solo bibliotecas."),LANG(GENERAL_WARNING,"Aviso"),mxMD_OK|mxMD_WARNING).ShowModal();
		return false;
	}
	wxString command ( wxFileName(DIR_PLUS_FILE(path,active_configuration->output_file)).GetShortPath() );
	wxString working_path = (active_configuration->working_folder==_T(""))?path:DIR_PLUS_FILE(path,active_configuration->working_folder);
	if (working_path.Last()==path_sep)
		working_path.RemoveLast();
	wxString args = active_configuration->args;
	if (active_configuration->always_ask_args) {
		int res = mxArgumentsDialog(main_window,active_configuration->args).ShowModal();
		if (res&AD_CANCEL) return false;
		if (res&AD_ARGS) {
			active_configuration->args = mxArgumentsDialog::last_arguments;;
			args=active_configuration->args;
		}
		if (res&AD_REMEMBER) {
			active_configuration->always_ask_args=false;
			if (res&AD_EMPTY) active_configuration->args=_T("");
		}
	}
	
#if !defined(_WIN32) && !defined(__WIN32__)
	wxString ld_library_path;
	wxGetEnv(_T("LD_LIBRARY_PATH"),&ld_library_path);
	wxString old_ld_library_path=ld_library_path;
	project_library *lib = active_configuration->libs_to_build;
	while (lib) {
		if (!lib->is_static) {
			if (ld_library_path.Len())
				ld_library_path<<_T(";");
			ld_library_path<<utils->Quotize(DIR_PLUS_FILE(path,lib->path));
		}
		lib = lib->next;
	}
	wxSetEnv(_T("LD_LIBRARY_PATH"),ld_library_path);
#endif
	bool ret = debug->Start(working_path,command,args,active_configuration->console_program,active_configuration->wait_for_key);
#if !defined(_WIN32) && !defined(__WIN32__)
	wxSetEnv(_T("LD_LIBRARY_PATH"),old_ld_library_path);
#endif
	return ret;
}

/**
* Una vez inicializado gdb, esta funci�n invoca a trav�s de DebugManager los 
* comandos  necesarios para cargar todos los breakpoints definidos en todos los 
* archivos del proyecto. Setea adem�s las propiedades de cada uno. Se debe 
* llamar antes de comenzar la ejecuci�n.
**/
void ProjectManager::SetBreakpoints() {
	BreakPointInfo *bpi=NULL;
	while ((bpi=BreakPointInfo::GetGlobalNext(bpi))) {
		bpi->UpdateLineNumber();
		debug->SetBreakPoint(bpi);
	}
}

bool ProjectManager::GenerateDoxyfile(wxString fname) {
	
	if (!doxygen) doxygen=new doxygen_configuration(project_name);

	wxTextFile fil(DIR_PLUS_FILE(path,fname));
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	
	fil.AddLine(wxString(_T("PROJECT_NAME = "))<<doxygen->name);
	fil.AddLine(wxString(_T("PROJECT_NUMBER = "))<<doxygen->version);
	fil.AddLine(wxString(_T("OUTPUT_DIRECTORY = "))<<doxygen->destdir);
	if (doxygen->use_in_quickhelp)
		fil.AddLine(wxString(_T("GENERATE_TAGFILE = "))<<DIR_PLUS_FILE(doxygen->destdir,_T("index-for-zinjai.tag")));
	fil.AddLine(_T("INPUT_ENCODING = ISO-8859-15"));
	fil.AddLine(wxString(_T("OUTPUT_LANGUAGE = "))<<doxygen->lang);
	if (doxygen->base_path.Len()) fil.AddLine(wxString(_T("STRIP_FROM_PATH = "))<<doxygen->base_path);
		
	if (doxygen->hideundocs) {
		fil.AddLine(_T("EXTRACT_ALL = NO"));
		fil.AddLine(_T("HIDE_UNDOC_MEMBERS = YES"));
		fil.AddLine(_T("HIDE_UNDOC_CLASSES = YES"));
	} else {
		fil.AddLine(_T("EXTRACT_ALL = YES"));
		fil.AddLine(_T("HIDE_UNDOC_MEMBERS = NO"));
		fil.AddLine(_T("HIDE_UNDOC_CLASSES = NO"));
		
	}
	
	wxArrayString input;
	
	if (doxygen->do_headers) {
		file_item *item = first_header;
		ML_ITERATE(item) {
			input.Add(item->name);
		}
	}
	if (doxygen->do_cpps) {
		file_item *item = first_source;
		ML_ITERATE(item) {
			input.Add(item->name);
		}
	}
	
	wxArrayString array;
	utils->Split(doxygen->extra_files,array,true,false);
	for (unsigned int i=0;i<array.GetCount();i++)
		input.Add(array[i]);
	
	array.Clear();
	utils->Split(doxygen->exclude_files,array,true,false);
	for (unsigned int i=0;i<array.GetCount();i++) {
		for (unsigned int j=0;i<input.GetCount();j++) {
#if defined(_WIN32) || defined(__WIN32__)
			if (array[i].CmpNoCase(input[j])==0) 
#else
			if (array[i]==input[j]) 
#endif
			{
				input.RemoveAt(j);
				j--;
			}
		}
	}
	
	if (input.GetCount()) {
		fil.AddLine(wxString(_T("INPUT = "))<<input[0]);
		for (unsigned int i=1;i<input.GetCount();i++)
			fil.AddLine(wxString(_T("INPUT += \""))<<input[i]<<_T("\""));
	}
	
	array.Clear();
	
	fil.AddLine(wxString(_T("GENERATE_LATEX = "))<<(doxygen->latex?_T("YES"):_T("NO")));
	fil.AddLine(wxString(_T("GENERATE_HTML = "))<<(doxygen->html?_T("YES"):_T("NO")));
	fil.AddLine(wxString(_T("GENERATE_TREEVIEW = "))<<(doxygen->html_navtree?_T("YES"):_T("NO")));
	fil.AddLine(wxString(_T("SEARCHENGINE = "))<<(doxygen->html_searchengine?_T("YES"):_T("NO")));
	fil.AddLine(_T("SEARCH_INCLUDES = YES"));
	utils->Split(active_configuration->headers_dirs,array,true,false);
	if (array.GetCount()) {
		wxString defs(_T("INCLUDE_PATH = "));
		for (unsigned int i=0;i<array.GetCount();i++)
			defs<<_T("\"")<<array[i]<<_T("\" ");
		fil.AddLine(defs);
	}
	fil.AddLine(wxString(_T("ENABLE_PREPROCESSING = "))<<(doxygen->preprocess?_T("YES"):_T("NO")));
	fil.AddLine(wxString(_T("EXTRACT_PRIVATE = "))<<(doxygen->extra_private?_T("YES"):_T("NO")));
	fil.AddLine(wxString(_T("EXTRACT_STATIC = "))<<(doxygen->extra_static?_T("YES"):_T("NO")));
	if (doxygen->preprocess) {
		array.Clear();
		utils->Split(active_configuration->macros,array,true,false);
		if (array.GetCount()) {
			wxString defs(_T("PREDEFINED = "));
			for (unsigned int i=0;i<array.GetCount();i++)
				defs<<_T("\"")<<array[i]<<_T("\" ");
			fil.AddLine(defs);
		}
	}
	fil.AddLine(_T("HTML_FILE_EXTENSION = .html"));
	
	fil.AddLine(doxygen->extra_conf);
	
	fil.Write();
	fil.Close();
	return true;
}

wxString ProjectManager::GetExePath() {
	return DIR_PLUS_FILE(path,active_configuration->output_file);
}

wxString ProjectManager::GetPath() {
	return path;
}

void ProjectManager::MoveFirst(wxTreeItemId &tree_item) {
	file_item *item=FindFromItem(tree_item);
	if (item) {
		if (item->prev!=first_source) {
			item->prev->next=item->next;
			if (item->next) item->next->prev=item->prev;
			item->next=first_source->next;
			item->prev=first_source;
			first_source->next=item;
		}
	}
}


wxString ProjectManager::WxfbGetSourceFile(wxString fbp_file) {
	wxString str;
	wxTextFile tf(fbp_file);
	tf.Open();
	int found=0;
	bool generates_code=true;
	wxString out_name,out_path;
	for (str = tf.GetFirstLine(); !tf.Eof(); str = tf.GetNextLine() ) {
		if (str.Contains(_T("<property name=\"code_generation\">"))) {
			generates_code = !str.Contains(_T("XRC"));
			found++;
		} else if (str.Contains(_T("property name=\"path\""))) {
			out_path=str.AfterFirst('>').BeforeLast('<');
			found++;
		} else if (str.Contains(_T("property name=\"file\""))) {
			out_name=str.AfterFirst('>').BeforeLast('<');
			found++;
		}
		if (found==3) break;
	}
	tf.Close();
	if (!generates_code) return "";
	wxFileName fn(fbp_file);
	wxString in_name = _T("noname");
	wxString in_path = fn.GetPathWithSep();
	if (out_name.Len()) in_name = out_name;
	if (out_path.Len()) in_name = DIR_PLUS_FILE(out_path,in_name);
	return DIR_PLUS_FILE(fn.GetPath(),in_name);
}

bool ProjectManager::WxfbGenerate(bool show_osd, file_item *cual) {

	if (!config->CheckWxfbPresent()) return false;
	
	wxString old_compiler_tree_text = main_window->compiler_tree.treeCtrl->GetItemText(main_window->compiler_tree.state);
//	wxString old_status_text = main_window->GetStatusBar()->GetStatusText();

	mxOSD *osd=NULL;
	
	main_window->SetCompilingStatus(LANG(PROJMNGR_REGENERATING_WXFB,"Regenerando proyecto wxFormBuilder..."));
	
	wxfbHeaders.Clear();
	bool something_changed=false;
	file_item *item;
	item = first_other;
	ML_ITERATE(item) { // por cada archivo .fbp (deberian estar en "otros")
		if ( (cual&&item==cual) || (!cual && item->name.Len()>4 && item->name.Mid(item->name.Len()-4).CmpNoCase(_T(".fbp"))==0) ) {
			
			wxString fbp_file=DIR_PLUS_FILE(path,item->name);
			wxString fbase = WxfbGetSourceFile(fbp_file);
			
			// ver si hay que regenerar (comparando con fflag)
			wxString fflag = fbp_file.Mid(0,fbp_file.Len()-4)+_T(".flg");
			wxDateTime dp = wxFileName(fbp_file).GetModificationTime();
			bool regen = cual||(!wxFileName::FileExists(fflag) || dp>wxFileName(fflag).GetModificationTime());
			
			if (fbase.Len()) {
				wxFileName fn_header(fbase+_T(".h"));
				fn_header.Normalize();
				if (HasFile(fn_header.GetFullPath())) wxfbHeaders.Add(fn_header.GetFullPath());
			}
			if (regen) { // regenerar, reparsear y recargar si es necesario
				if (show_osd && osd==NULL) osd=new mxOSD(main_window,LANG(PROJMNGR_REGENERATING_WXFB,"Regenerando proyecto wxFormBuilder..."));
				if (fbase.Len()) {
					int ret = wxExecute(wxString(_T("\""))+config->Files.wxfb_command+_T("\" -g \"")+fbp_file+_T("\""), wxEXEC_NODISABLE|wxEXEC_SYNC);
					
					if (ret) {
						if (osd) delete osd;
						if (auto_wxfb) {
							if (mxMD_YES==mxMessageDialog(main_window,LANG(PROJMNGR_REGENERATING_ERROR_1,"No se pudieron actualizar correctamente los proyectos wxFormBuilder\n(probablemente la ruta al ejecutable de wxFormBuilder no este correctamente\ndefinida. Verifique esta propiedad en la pesta�a \"Rutas 2\" del cuadro de \"Preferencias\").\nSi el error se repite puede desactivar la actualizaci�n autom�tica.\n�Desea desactivar la actualizaci�n autom�tica ahora?"),_T("Error"),mxMD_YES_NO|mxMD_ERROR).ShowModal())
								auto_wxfb=false;
						} else {
							mxMessageDialog(main_window,LANG(PROJMNGR_REGENERATING_ERROR_2,"No se pudieron actualizar correctamente los proyectos wxFormBuilder\n(probablemente la ruta al ejecutable de wxFormBuilder no este correctamente\ndefinida. Verifique esta propiedad en la pesta�a \"Rutas 2\" del cuadro de \"Preferencias\")."),_T("Error"),mxMD_YES_NO|mxMD_ERROR).ShowModal();
						}
						return false;
					}
					
					something_changed=true;
					if (wxFileName::FileExists(fbase+_T(".cpp"))) {
						mxSource *src = main_window->FindSource(fbase+".cpp");
						if (src) src->Reload();
						if (project->HasFile(fbase+".cpp")) parser->ParseFile(fbase+".cpp");
					}
					if (wxFileName::FileExists(fbase+_T(".h"))) {
						mxSource *src = main_window->FindSource(fbase+_T(".h"));
						if (src) src->Reload();
						if (project->HasFile(fbase+".h")) parser->ParseFile(fbase+_T(".h"));
					}
				} else {
					wxString fxrc=fbase+_T(".xrc");
					wxExecute(wxString(_T("\""))+config->Files.wxfb_command+_T("\" -g \"")+fbp_file+_T("\""), wxEXEC_NODISABLE|wxEXEC_SYNC);
					mxSource *src=main_window->FindSource(fxrc);
					if (src) src->Reload();
				}
				
				wxTextFile fil(fflag);
				if (!fil.Exists()) fil.Create();
				fil.Open();
				if (!fil.IsOpened()) {
					if (auto_wxfb) {
						if (mxMD_YES==mxMessageDialog(main_window,LANG(PROJMNGR_REGENERATING_ERROR_3,"No se pudo actualizar correctamente los proyectos wxFormBuilder\n(probablemente no se puede escribir en la carpeta de proyecto).\nSi el error se repite puede desactivar la actualizaci�n autom�tica.\n�Desea desactivar la actualizaci�n autom�tica ahora?"),_T("Error"),mxMD_YES_NO|mxMD_ERROR).ShowModal())
							auto_wxfb=false;
					} else {
						mxMessageDialog(main_window,LANG(PROJMNGR_REGENERATING_ERROR_4,"No se pudieron actualizar correctamente los proyectos wxFormBuilder\n(probablemente no se puede escribir en la carpeta de proyecto)."),_T("Error"),mxMD_YES_NO|mxMD_ERROR).ShowModal();
					}
					break;
				}
				fil.Clear();
				fil.AddLine(_T("Este archivo se utiliza para determinar la fecha y hora de la ultima compilacion del proyecto wxFormBuilder homonimo."));
				fil.AddLine(_T("This is a dummy file to be used as timestamp for the generation of a wxFormBuilder project."));
				fil.Write();
				fil.Close();
			}
		}
	}
	
	// todo: reemplazar estas lineas por SetCompilingStatus, pero ver antes en que contexto se llega aca para saber que puede haber habido en status
	main_window->compiler_tree.treeCtrl->SetItemText(main_window->compiler_tree.state,old_compiler_tree_text);
	main_window->SetStatusText(wxString(LANG(GENERAL_READY,"Listo")));
	
	generating_wxfb=false;
	
	if (osd) delete osd;
	
	return something_changed;
}

compile_extra_step *ProjectManager::InsertExtraSteps(project_configuration *conf, wxString name, wxString cmd, int pos) {
	modified=true;
	if (!conf) conf=active_configuration;
	compile_extra_step *step = new compile_extra_step;
	step->command=cmd;
	step->name=name;
	step->pos=pos;
	if (conf->extra_steps) {
		compile_extra_step *s1=conf->extra_steps,*s1p=NULL;;
		while (s1 && s1->pos<=step->pos) {
			s1p=s1;
			s1=s1->next;
		}
		if (s1p) {
			if (s1p->next) s1p->next->prev=step;
			step->next=s1p->next;
			s1p->next=step;
			step->prev=s1p;
		} else {
			step->next=conf->extra_steps;
			if (step->next) step->next->prev=step;
			conf->extra_steps=step;
		}
	} else {
		conf->extra_steps=step;
		step->next=step->prev=NULL;
	}
	return step;
}

void ProjectManager::MoveExtraSteps(project_configuration *conf, compile_extra_step *step, int delta) {
	modified=true;
	if (!conf) conf=active_configuration;
	if (delta==-1) {
		if (!step->prev || step->prev->pos!=step->pos) {
			if (step->pos==CES_BEFORE_EXECUTABLE) {
				if (conf->libs_to_build)
					step->pos=CES_BEFORE_LIBS;
				else
					step->pos=CES_BEFORE_SOURCES;
			} else if (step->pos==CES_BEFORE_LIBS) {
				step->pos=CES_BEFORE_SOURCES;
			} else if (step->pos==CES_AFTER_LINKING)
				step->pos=CES_BEFORE_EXECUTABLE;
		} else if (step->prev) {
			// intercambia s2 y s3
			compile_extra_step *s1=step->prev->prev,*s2=step->prev,*s3=step,*s4=step->next;
			s2->next=s4;
			if (s4) s4->prev=s2;
			s2->prev=s3;
			s3->next=s2;
			s3->prev=s1;
			if (s1)
				s1->next=s3;
			else
				conf->extra_steps=s3;
		}
	} else if (delta==1) {
		if (!step->next || step->next->pos!=step->pos) {
			if (step->pos==CES_BEFORE_EXECUTABLE)
				step->pos=CES_AFTER_LINKING;
			else if (step->pos==CES_BEFORE_SOURCES) {
				if (conf->libs_to_build)
					step->pos=CES_BEFORE_LIBS;
				else
					step->pos=CES_BEFORE_EXECUTABLE;
			} else if (step->pos==CES_BEFORE_LIBS) {
				if (conf->libs_to_build)
					step->pos=CES_BEFORE_EXECUTABLE;
				else
					step->pos=CES_AFTER_LINKING;
			}
		} else if (step->next) {
			// intercambia s2 y s3
			compile_extra_step *s1=step->prev,*s2=step,*s3=step->next,*s4=step->next->next;
			s3->next=s2;
			s2->prev=s3;
			s2->next=s4;
			if (s4) s4->prev=s2;
			s3->prev=s1;
			if (s1)
				s1->next=s3;
			else
				conf->extra_steps=s3;
		}
	}
}

bool ProjectManager::DeleteExtraStep(project_configuration *conf, compile_extra_step *step) {
	if (!conf) conf=active_configuration;
	if (!step) return false;
	modified=true;
	if (step->next) step->next->prev = step->prev;
	if (step->prev)
		step->prev->next=step->next;
	else
		conf->extra_steps=step->next;
	delete step;
	return true;
}

/** 
* Busca informaci�n correspondiente a un paso de compilaci�n personalizado. La busqueda se hace
* por nombre.
* @param conf configuraci�n en la cual buscar; si recibe NULL utiliza active_configuration
* @param name nombre del paso a buscar, case sensitive
* @return puntero al paso, si es que existe, sino NULL
**/

compile_extra_step *ProjectManager::GetExtraStep(project_configuration *conf, wxString name) {
	if (!conf) conf=active_configuration;
	compile_extra_step *step = conf->extra_steps;
	while (step && step->name!=name)
		step = step->next;
	return step;
}

long int ProjectManager::CompileExtra(compile_and_run_struct_single *compile_and_run, compile_extra_step *step) {
	
	// preparar la linea de comando (OJO! esto tambien esta en mxProjectConfigWindow)
	wxString command = step->command;
	utils->ParameterReplace(command,_T("${OUTPUT}"),step->out);
	utils->ParameterReplace(command,_T("${DEPS}"),step->deps);
#if defined(_WIN32) || defined(__WIN32__)
	utils->ParameterReplace(command,_T("${MINGW_DIR}"),config->mingw_real_path);
#endif
	utils->ParameterReplace(command,_T("${PROJECT_PATH}"),project->path);
	utils->ParameterReplace(command,_T("${PROJECT_BIN}"),project->executable_name);
	utils->ParameterReplace(command,_T("${TEMP_DIR}"),temp_folder_short);
	compile_and_run->process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_COMPILE);
	compile_and_run->process->Redirect();
	
	// ejecutar
	compile_and_run->output_type=MXC_EXTRA;
	compile_and_run->step_label=step->name;
	compile_and_run->full_output.Add(_T(""));
	compile_and_run->full_output.Add(wxString(_T("> "))+command);
	compile_and_run->full_output.Add(_T(""));
	compile_and_run->last_all_item = main_window->compiler_tree.treeCtrl->AppendItem(main_window->compiler_tree.all,command,2);
	return utils->Execute(path,command, wxEXEC_ASYNC|(step->hide_window?0:wxEXEC_NOHIDE),compile_and_run->process);
}

long int ProjectManager::CompileWithExternToolchain(compile_and_run_struct_single *compile_and_run, bool build) {
	
	wxString command = build?current_toolchain.build_command:current_toolchain.clean_command;
	for(int i=0;i<TOOLCHAIN_MAX_ARGS;i++) 
		command.Replace(wxString("${ARG")<<i+1<<"}",current_toolchain.arguments[i][1],true);
	
#if defined(_WIN32) || defined(__WIN32__)
	utils->ParameterReplace(command,_T("${MINGW_DIR}"),config->mingw_real_path);
#endif
	utils->ParameterReplace(command,_T("${PROJECT_PATH}"),project->path);
	utils->ParameterReplace(command,_T("${PROJECT_BIN}"),project->executable_name);
	utils->ParameterReplace(command,_T("${TEMP_DIR}"),temp_folder_short);
	utils->ParameterReplace(command,_T("${NUM_PROCS}"),wxString()<<config->Init.max_jobs);
	
	compile_and_run->process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_COMPILE);
	compile_and_run->process->Redirect();
	
	// ejecutar
	compile_and_run->output_type=MXC_EXTERN;
	compile_and_run->full_output.Add(_T(""));
	compile_and_run->full_output.Add(wxString(_T("> "))+command);
	compile_and_run->full_output.Add(_T(""));
	main_window->AddExternCompilerOutput("> ",command);
	return utils->Execute(path,command, wxEXEC_ASYNC/*|(step->hide_window?0:wxEXEC_NOHIDE)*/,compile_and_run->process);
}


/** 
* Determina si se debe realizar o no un paso extra de compilaci�n de acuerdo a
* sus dependencias. Si el paso no tiene definido el archivo de salida, se realiza siempre.
* Si tiene definido el archivo de salida, pero no las dependencias, se realiza solo si
* el archivo de salida no existe. Si est� completamente definido, se realiza si alguno
* de los archivos de la lista de dependencias tiene fecha de modificaci�n mayor a la del
* archivo de salida. Se la invoca desde PrepareForBuilding.
* @param step puntero a la estructura que contiene la informaci�n del paso extra por el que se pregunta
* @retval true si es necesario ejecutar este paso
* @retval false si es no necesario ejecutar este paso
**/
bool ProjectManager::ShouldDoExtraStep(compile_extra_step *step) {
	if (step->out.Len()==0) return true;
	wxString str_out(step->out);
	utils->ParameterReplace(str_out,_T("${TEMP_DIR}"),temp_folder_short);
	utils->ParameterReplace(str_out,_T("${PROJECT_BIN}"),executable_name);
	utils->ParameterReplace(str_out,_T("${PROJECT_PATH}"),path);
	utils->ParameterReplace(str_out,_T("${MINGW_DIR}"),config->mingw_real_path);
	str_out=DIR_PLUS_FILE(path,str_out);
	wxFileName fn_out(str_out);
	if (!fn_out.FileExists()) return true;
	wxDateTime dt_out = fn_out.GetModificationTime();
	wxArrayString array;
	utils->Split(step->deps,array,true,false);
	for (unsigned int i=0;i<array.GetCount();i++) {
		wxString str_dep(array[i]);
		utils->ParameterReplace(str_dep,_T("${TEMP_DIR}"),temp_folder_short);
		utils->ParameterReplace(str_dep,_T("${PROJECT_BIN}"),executable_name);
		utils->ParameterReplace(str_dep,_T("${PROJECT_PATH}"),path);
		utils->ParameterReplace(str_dep,_T("${MINGW_DIR}"),config->mingw_real_path);
		str_dep = DIR_PLUS_FILE(path,str_dep);
		wxFileName fn_dep = wxFileName(str_dep);
		if (fn_dep.FileExists() && fn_dep.GetModificationTime()>dt_out)
			return true;
	}
	return false;
}

/**
* Cambia algunos parametros del proyecto para corregir nombres del template con
* el nombre de proyecto ingresado por el usuario. Se llama desde el asistente de
* nuevo proyecto, luego de crearlo y abrirlo.
*
* @param name nombre del proyecto (nombre del archivo sin ruta ni extensi�n)
**/
void ProjectManager::FixTemplateData(wxString name) {
	project_name=name;
	for (int i=0;i<configurations_count;i++) {
		utils->ParameterReplace(configurations[i]->output_file,_T("${FILENAME}"),name);
		project_library *lib = configurations[i]->libs_to_build;
		while (lib) {
			utils->ParameterReplace(lib->libname,_T("${FILENAME}"),name);
			lib = lib->next;
		}
	}
	main_window->SetOpenedFileName(project_name=name);
}

void ProjectManager::ActivateWxfb() {
	use_wxfb=true; wxfb_ask=true;
	main_window->menu.tools_wxfb_activate->Check(true);
	main_window->menu.tools_wxfb_auto->Check(auto_wxfb);
	main_window->menu.tools_wxfb_auto->Enable(true);
	main_window->menu.tools_wxfb_regen->Enable(true);
	main_window->menu.tools_wxfb_inherit->Enable(true);
	main_window->menu.tools_wxfb_update_inherit->Enable(true);
	config->CheckWxfbPresent();
}

long int ProjectManager::CompileIcon(compile_and_run_struct_single *compile_and_run, wxString icon_name) {
	// preparar la linea de comando 
	wxString command = _T("windres ");
	command<<_T(" -i \"")<<wxFileName(DIR_PLUS_FILE(temp_folder,_T("zpr_resource.rc"))).GetShortPath()<<_T("\"");
	command<<_T(" -o \"")<<wxFileName(DIR_PLUS_FILE(temp_folder,_T("zpr_resource.o"))).GetShortPath()<<_T("\"");
	compile_and_run->process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_COMPILE);
	compile_and_run->process->Redirect();
	// ejecutar
	compile_and_run->output_type=MXC_SIMIL_GCC;
	compile_and_run->full_output.Add(_T(""));
	compile_and_run->full_output.Add(wxString(_T("> "))+command);
	compile_and_run->full_output.Add(_T(""));
	compile_and_run->last_all_item = main_window->compiler_tree.treeCtrl->AppendItem(main_window->compiler_tree.all,command,2);
	return utils->Execute(path,command, wxEXEC_ASYNC,compile_and_run->process);
}

int ProjectManager::GetRequiredVersion() {
	bool have_macros=false,have_icon=false,have_temp_dir=false,builds_libs=false,have_extra_vars=false,have_manifest=false;
	for (int i=0;i<configurations_count;i++) {
		if (configurations[i]->compiling_extra.Contains("${TEMP_DIR}")) have_temp_dir=true;
		if (configurations[i]->linking_extra.Contains("${TEMP_DIR}")) have_temp_dir=true;
		if (configurations[i]->macros.Len()) have_macros=true;
		if (configurations[i]->manifest_file.Len()) have_manifest=true;
		if (configurations[i]->icon_file.Len()) have_icon=true;
		if (configurations[i]->libs_to_build) builds_libs=true;
		compile_extra_step *step=configurations[i]->extra_steps;
		while (step) {
			if (step->deps.Contains("${TEMP_DIR}")) have_temp_dir=true;
			if (step->out.Contains("${TEMP_DIR}")) have_temp_dir=true;
			wxString sum=step->out+step->deps+step->command;
			if (sum.Contains("${PROJECT_PATH}")) have_extra_vars=true;
			if (sum.Contains("${PROJECT_BIN}")) have_extra_vars=true;
			if (sum.Contains("${MINGW_DIR}")) have_extra_vars=true;
			if (sum.Contains("${DEPS}")) have_extra_vars=true;
			if (sum.Contains("${OUTPUT}")) have_extra_vars=true;
			if (!step->delete_on_clean) have_extra_vars=true;
			step = step->next;
		}
	}
	version_required=0;
	if (autocodes_file.Len()) version_required=20110814;
	else if (have_manifest) version_required=20110610;
	else if (have_extra_vars) version_required=20110401;
	else if (builds_libs) version_required=20100503;
	else if (doxygen && doxygen->base_path.Len()) version_required=20091127;
	else if (have_temp_dir) version_required=20091119;
	else if (have_icon) version_required=20091117;
	else if (macros_file.Len()) version_required=20091026;
	else if (have_macros) version_required=20090903;
	return version_required;
}

void ProjectManager::UpdateSymbols() {
	file_item *item;
	mxSource *source;
	item = first_source;
	ML_ITERATE(item) {
		wxString name = DIR_PLUS_FILE(path,item->name);
		if ((source=main_window->IsOpen(name)) && source->GetModify()) {
			parser->ParseSource(source);
		} else {
			parser->ParseIfUpdated(name);
		}
	}
	item = first_header;
	ML_ITERATE(item) {
		wxString name = DIR_PLUS_FILE(path,item->name);
		if ((source=main_window->IsOpen(name)) && source->GetModify()) {
			parser->ParseSource(source);
		} else {
			parser->ParseIfUpdated(name);
		}
	}
}


/** @brief Agrega una biblioteca a construir de una configuraci�n **/
project_library *ProjectManager::AppendLibToBuild(project_configuration *conf) {
	project_library *lib = conf->libs_to_build;
	if (!lib) {
		lib = conf->libs_to_build = new project_library;
	} else {
		while (lib->next)
			lib = lib->next;
		lib->next = new project_library(lib);
		lib = lib->next;
	}
	return lib;
}

/** @brief Elimina una biblioteca a construir de una configuraci�n **/
bool ProjectManager::DeleteLibToBuild(project_configuration *conf, project_library *lib_to_del) {
	if (conf->libs_to_build==lib_to_del) {
		conf->libs_to_build=lib_to_del->next;
		return true;
	}
	project_library *lib = conf->libs_to_build->next;
	while (lib && lib_to_del!=lib) {
		lib=lib->next;
	}
	if (lib) {
		lib->prev->next=lib->next;
		if (lib->next) 
			lib->next->prev=lib->prev;
		delete lib;
		return true;
	}
	return false;
}

/** @brief Elimina una biblioteca a construir de una configuraci�n **/
project_library *ProjectManager::GetLibToBuild(project_configuration *conf, wxString libname) {
	project_library *lib = conf->libs_to_build;
	while (lib) {
		if (lib->libname==libname)
			return lib;
		lib=lib->next;
	}
	return NULL;
}

void ProjectManager::SaveLibsAndSourcesAssociation(project_configuration *conf) {
	project_library *lib = conf->libs_to_build;
	while (lib) {
		lib->sources.Clear();
		lib=lib->next;
	}
	file_item *fi = first_source;
	ML_ITERATE(fi) {
		if (fi->lib) {
			if (fi->lib->sources.Len())
				fi->lib->sources<<_T(" ");
			if (fi->name.Contains(_T(" ")))
				fi->lib->sources<<_T("\"")<<fi->name<<_T("\"");
			else
				fi->lib->sources<<fi->name;
		}
	}
}

void ProjectManager::AssociateLibsAndSources(project_configuration *conf) {
	if (!conf) conf=active_configuration;
	file_item *fi = first_source;
	ML_ITERATE(fi) fi->lib=NULL;
	project_library *lib = conf->libs_to_build;
	wxArrayString srcs;
	while (lib) {
		srcs.Clear();
		utils->Split(lib->sources,srcs);
		for (unsigned int i=0;i<srcs.GetCount();i++) {
			fi = first_source;
			ML_ITERATE(fi) {
				if (!fi->lib && fi->name==srcs[i]) {
					fi->lib=lib;
					break;
				}
			}
		}
		// armar tambien el nombre del archivo
		lib->filename = DIR_PLUS_FILE(lib->path,wxString(_T("lib"))<<lib->libname);
#if defined(_WIN32) || defined(__WIN32__)
		if (lib->is_static)
			lib->filename<<_T(".lib");
		else
			lib->filename<<_T(".dll");
#else
		if (lib->is_static)
			lib->filename<<_T(".a");
		else
			lib->filename<<_T(".so");
#endif
		lib = lib->next;
	}
}

struct draw_graph_item {
	wxString name;
	wxFileName fullpath;
	unsigned long sz,sy,lc;
	bool operator<(const draw_graph_item &gi) const {
		return lc<gi.lc;
	}
};

void ProjectManager::DrawGraph() {
	
	unsigned long Ml=0, ml=0, mm=0; // maxima, minima, mediana
	mxOSD osd(main_window,LANG(OSD_GENERATING_GRAPH,"Generando grafo..."));
	
	wxArrayString header_dirs_array;
	utils->Split(active_configuration->headers_dirs,header_dirs_array,true,false);

	wxTextFile fil(DIR_PLUS_FILE(config->temp_dir,_T("graph.dot")));
	
	int c=0;
	file_item *fi = first_source;
	ML_ITERATE(fi) c++;
	fi = first_header;
	ML_ITERATE(fi) c++;
	
	draw_graph_item *dgi = new draw_graph_item[c];
	c=0;
	wxString tab(_T("\t"));
	
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	fil.AddLine(_T("digraph h {"));
	fil.AddLine(_T("\tsplines=true;"));
	
	fi = first_header;
	bool second=true;
	do {
		second=!second;
		ML_ITERATE(fi) {
			dgi[c].name = fi->name;
			wxString fullname = DIR_PLUS_FILE(path,fi->name);
			dgi[c].fullpath = fullname;
			dgi[c].sz=wxFileName::GetSize(fullname).ToULong();
			if (dgi[c].sz==wxInvalidSize) dgi[c].sz=0;
			wxTextFile tf(fullname);
			if (tf.Open()) {
				dgi[c].lc=tf.GetLineCount();
				tf.Close();
				if (c) {
					if (dgi[c].lc>Ml) Ml=dgi[c].lc;
					if (dgi[c].lc<ml) ml=dgi[c].lc;
				} else
					ml=Ml=dgi[c].lc;
			} else dgi[c].lc=0;
			dgi[c].sy=0;
			pd_file *pd = parser->GetFile(fullname);
			pd_ref *ref;
			if (pd) {
				ref=pd->first_class;
				ML_ITERATE(ref) dgi[c].sy++;
				ref=pd->first_func_dec;
				ML_ITERATE(ref) dgi[c].sy++;
				ref=pd->first_func_def;
				ML_ITERATE(ref) if (PD_UNREF(pd_func,ref)->file_dec!=pd) dgi[c].sy++;
				ref=pd->first_attrib;
				ML_ITERATE(ref) dgi[c].sy++;
				ref=pd->first_macro;
				ML_ITERATE(ref) dgi[c].sy++;
			}
			c++;
		}
		fi=first_source;
	} while (!second);
	
	sort(dgi,dgi+c);
	mm=dgi[c/2].lc;
	if (Ml==mm || mm==ml) mm=(ml+Ml)/2;
	
	for (int i=0;i<c;i++) {
		wxString line("\t");
		unsigned long x,lc=dgi[i].lc;
		wxColour col;
		if (ml==Ml) {
			col=wxColour(128,255,128);
		} else if (lc<mm) {
			x=(dgi[i].lc-ml)*100/(mm-ml);
			col=wxColour(156,155+x,255-x);
		} else {
			x=(dgi[i].lc-mm)*100/(Ml-mm);
			col=wxColour(155+x,255-x,156);
		}
		line<<_T("\"")<<dgi[i].name<<_T("\"")
			<<_T("[shape=note,style=filled,fillcolor=\"")
			<<col.GetAsString(wxC2S_HTML_SYNTAX)<<_T("\",label=\"")
			<<dgi[i].name
			<<_T("  \\l")<<dgi[i].sz<<LANG(PROJMNGR_PROJECT_GRAPH_BYTES," bytes")
			<<_T("\\r")<<dgi[i].lc<<LANG(PROJMNGR_PROJECT_GRAPH_LINES," linea(s)")
			<<_T("\\r")<<dgi[i].sy<<LANG(PROJMNGR_PROJECT_GRAPH_SYMBOLS," simbolo(s)")
			<<_T("\\r\"];");
		fil.AddLine(line);
	}
	
	wxArrayString deps;
	for (int i=0;i<c;i++) {
		utils->FindIncludes(deps,dgi[i].fullpath,path,header_dirs_array);
		for (unsigned int j=0;j<deps.GetCount();j++) {
			wxString header=DIR_PLUS_FILE(path,deps[j]);
			for (int k=0;k<c;k++) {
				if (dgi[k].fullpath==header) {
					fil.AddLine(tab+_T("\"")+dgi[i].name+_T("\"->\"")+dgi[k].name+_T("\";"));
					break;
				}
			}
		}
	}
	
	fil.AddLine(_T("}"));
	fil.Write();
	fil.Close();
	
	delete [] dgi;
	
#if defined(__WIN32__)	
	wxString command(DIR_PLUS_FILE(config->Files.graphviz_dir,_T("draw.exe")));
#else
	wxString command(DIR_PLUS_FILE(config->Files.graphviz_dir,_T("draw.bin")));
#endif
	command<<_T(" fdp");
//#if defined(__x86_64__) || defined(__WIN32__)
	command<<_T(" \"")<<DIR_PLUS_FILE(config->temp_dir,_T("graph.dot"))<<_T("\" -Tpng -o \"")<<DIR_PLUS_FILE(config->temp_dir,_T("graph.png"))<<_T("\"");
//#else
//	command<<_T(" \"")<<DIR_PLUS_FILE(config->temp_dir,_T("graph.dot"))<<_T("\" -Tbmp -o \"")<<DIR_PLUS_FILE(config->temp_dir,_T("graph.bmp"))<<_T("\"");
//#endif
	wxExecute(command,wxEXEC_SYNC);
	wxString command2 (config->Files.img_browser);
//#if defined(__x86_64__) || defined(__WIN32__)
	command2<<_T(" \"")<<DIR_PLUS_FILE(config->temp_dir,_T("graph.png"))<<_T("\" \"")<<LANG(PROJMNGR_PROJECT_GRAPH_TITLE,"Grafo de Proyecto")<<_T("\"");
//#else
//	command2<<_T(" \"")<<DIR_PLUS_FILE(config->temp_dir,_T("graph.bmp"))<<_T("\" \"")<<LANG(PROJMNGR_PROJECT_GRAPH_TITLE,"Grafo de Proyecto")<<_T("\"");
//#endif
	
	wxExecute(command2);
//			new mxPictureWindow(main_window,DIR_PLUS_FILE(config->temp_dir,_T("graph.png")));

}



/// Funcion auxiliar para ProjectManager::WxfbNewClass y ProjectManager::WxfbUpdateClass
void static GetFatherMethods(wxString base_header, wxString class_name, wxArrayString &methods) {
	// encontrar los metodos a heredar
	wxTextFile btf(base_header);
	btf.Open();
	wxString str, class_tag = wxString(_T("class "))+class_name;
	bool on_the_class=false;
	for ( str = btf.GetFirstLine(); !btf.Eof(); str = btf.GetNextLine() ) {
		if (str.Contains(_T("class ")) && utils->LeftTrim(str.Mid(0,2))!=_T("//")) {
			if (str.Contains(class_tag)) {
				on_the_class=true;
			} else if (on_the_class) 
				break;
		} else if (on_the_class) {
			if (str.Contains(_("virtual ")) && utils->LeftTrim(str.Mid(0,2))!=_T("//"))
				methods.Add(utils->LeftTrim(str.BeforeFirst('{')).Mid(8));
		}
	}
	btf.Close();
}

bool ProjectManager::WxfbUpdateClass(wxString fbp_file, wxString cname) {
	pd_class *pdc_son = parser->GetClass(cname);
	wxString cfile = utils->GetComplementaryFile(pdc_son->file->name,'h');
	if (cfile.Len()==0) { // si tampoco hay "public:", no hay caso
		mxMessageDialog(main_window,_T("No se pudo determinar donde definir los nuevos metodos.\nNo se encontr� el archivo fuente complementario."),_T("Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		return false;
	}
	wxTextFile fil(pdc_son->file->name);
	fil.Open();
	bool add_visibility=false;
	int i,lines=fil.GetLineCount(),curpos=pdc_son->line,inspos=-1, pubpos=-1; // linea en la cual agregar los metodos
	wxString str;
	wxString tabs_pro, tabs_pub;
	while (curpos<lines) {
		str = fil.GetLine(curpos);
		i=0; while (str[i]==' '||str[i]=='\t') i++;
		if (str.Len()-i>=10 && str.Mid(i,10)==_T("protected:")) {
			inspos=curpos;
			tabs_pro=str.Mid(0,i);
			break;
		} else
			if (str.Len()-i>=7 && str.Mid(i,7)==_T("public:")) {
				tabs_pub=str.Mid(0,i);
				pubpos=curpos;
			} else
				if (str.Len()-i>6 && str.Mid(i,6)==_T("class ")) break;
		curpos++;
	}
	if (inspos==-1) { // si no hay "protected:", se agrega antes de "public:"
		if (pubpos==-1) { // si tampoco hay "public:", no hay caso
			mxMessageDialog(main_window,_T("No se pudo determinar donde declarar los nuevos metodos.\nNo se encontraron la etiquetas public/protected en la clase."),_T("Error"),mxMD_OK|mxMD_ERROR).ShowModal();
			return false;
		}
		inspos = pubpos;
		tabs_pro = tabs_pub;
		add_visibility=true;
	} else inspos++; // linea siguiente a "protected:"
	if (add_visibility) {
		fil.InsertLine(tabs_pro+_T("protected:"),inspos++);
		fil.InsertLine(tabs_pro,inspos);
	}
	
	tabs_pro+=_T("\t");
	
	pd_class *pdc_father = parser->GetClass(fbp_file);
	wxArrayString methods;
	GetFatherMethods(pdc_father->file->name,fbp_file,methods);
	
	bool modified=false;
	
	wxTextFile fil2(cfile);
	fil2.Open();
	if (fil2.GetLastLine().Len())
		fil2.AddLine(_T(""));
	
	for (unsigned int i=0;i<methods.GetCount();i++) {
		wxString mname = methods[i].BeforeFirst('(');
		while (mname.Last()==' ') mname.RemoveLast();
		mname = mname.AfterLast(' ');
		pd_func *pdm_son = pdc_son->first_method;
		bool found=false;
		ML_ITERATE(pdm_son)	if (pdm_son->name==mname) { found=true; break; }
		if (!found) {
			modified=true;
			fil.InsertLine(tabs_pro+methods[i]+_T(";"),inspos++);
			fil2.AddLine(methods[i].BeforeFirst(' ')+_T(" ")+cname+_T("::")+methods[i].AfterFirst(' ')+_T(" {"));
			fil2.AddLine(_T("\tevent.Skip();"));
			fil2.AddLine(_T("}"));
			fil2.AddLine(_T(""));
		}
	}
	
	if (modified) {
		fil.Write();
		fil.Close();
		
		fil2.Write();
		fil2.Close();
		
		mxSource *src_h=main_window->IsOpen(pdc_son->file->name);
		mxSource *src_c=main_window->IsOpen(cfile);
		if (src_h) {
			src_h->Reload();
			parser->ParseSource(src_h,true);
		} else {
			parser->ParseFile(pdc_son->file->name);
		}
		if (src_c) {
			src_c->Reload();
			parser->ParseSource(src_c,true);
		} else {
			parser->ParseFile(cfile);
		}
	} else {
		fil.Close();
		fil2.Close();
	}
	
	return true;
}

void ProjectManager::WxfbAutoCheck() {
	if (loading || generating_wxfb) return;
	
	// primero una verificaci�n rapida para evitar que este evento moleste muy seguido
	bool something_changed=false;
	file_item *fitem = first_other;
	ML_ITERATE(fitem) { // por cada archivo .fbp (deberian estar en "otros")
		if (fitem->name.Len()>4 && fitem->name.Mid(fitem->name.Len()-4).CmpNoCase(_T(".fbp"))==0) {
			// ver si hay que regenerar (comparando con fflag)
			wxString fbp_file=DIR_PLUS_FILE(path,fitem->name);
			wxString fflag = fbp_file.Mid(0,fbp_file.Len()-4)+_T(".flg");
			if (!wxFileName::FileExists(fflag) || wxFileName(fbp_file).GetModificationTime()>wxFileName(fflag).GetModificationTime()) {
				something_changed=true;
				break;
			}
		}
	}
	if (!something_changed) return;
	
	SaveAll(false);
	if (parser->working) { parser->OnEnd(POE_AUTOUPDATE_WXFB); return; }
	generating_wxfb=true;
	if (!project->WxfbGenerate(true)) { generating_wxfb=false; return; }
	generating_wxfb=false;
	mxOSD osd(main_window,LANG(OSD_WXFB_AUTOREGEN,"Actualizando clases wxFormBuilder..."));
	
	parser->Parse();
	
	wxArrayString fathers;
	for (unsigned int i=0; i<project->wxfbHeaders.GetCount();i++) {
		pd_file *pdf=parser->GetFile(project->wxfbHeaders[i]);
		if (pdf) {
			pd_ref *cls_ref = pdf->first_class;
			ML_ITERATE(cls_ref)
				fathers.Add(PD_UNREF(pd_class,cls_ref)->name);
		}
	}

	wxArrayString to_update;
	pd_inherit *item=parser->first_inherit;
	while (item->next) {
		item=item->next;
		if (fathers.Index(item->father)!=wxNOT_FOUND) {
			to_update.Add(item->father);
			to_update.Add(item->son);
		}
	}
	for (unsigned int i=0;i<to_update.GetCount();i+=2)
		project->WxfbUpdateClass(to_update[i],to_update[i+1]);
	generating_wxfb=false;
}


bool ProjectManager::WxfbNewClass(wxString base_name, wxString name) {
	wxString base_header = parser->GetClass(base_name)->file->name;
	wxString folder;
	int pos1=name.Find('\\',true);
	int pos2=name.Find('/',true);
	if (pos1!=wxNOT_FOUND && pos2!=wxNOT_FOUND) {
		int pos = pos1>pos2?pos1:pos2;
		folder=name.Mid(0,pos);
		name=name.Mid(pos+1);
	} else if (pos1!=wxNOT_FOUND) {
		folder=name.Mid(0,pos1);
		name=name.Mid(pos1+1);
	} else if (pos2!=wxNOT_FOUND) {
		folder=name.Mid(0,pos2);
		name=name.Mid(pos2+1);
	}
	if (folder.Len()) {
		folder = DIR_PLUS_FILE(project->path,folder);
		if (!wxFileName::DirExists(folder)) {
			int ans = mxMessageDialog(main_window,wxString(LANG(PROJECT_DIRNOTFOUND_CREATE_PRE,"El directorio \""))<<folder<<LANG(PROJECT_DIRNOTFOUND_CREATE_POST,"\" no existe. Desea crearlo?"),_T("Error"),mxMD_YES|mxMD_NO|mxMD_QUESTION).ShowModal();
			if (ans==mxMD_YES) {
				wxFileName::Mkdir(folder);
				if (!wxFileName::DirExists(folder)) {
					mxMessageDialog(main_window,LANG(PROJECT_CANT_CREATE_DIR,"No se pudo crear el directorio."),LANG(GENERAL_ERROR,"Error"),mxMD_YES|mxMD_NO|mxMD_ERROR).ShowModal();
					return false;
				}
			} else {
				return false;
			}
		}
	} else
		folder=project->path;
	if (wxNOT_FOUND!=name.Find(' ') || wxNOT_FOUND!=name.Find('-') 
		|| wxNOT_FOUND!=name.Find('<') || wxNOT_FOUND!=name.Find('?') 
		|| wxNOT_FOUND!=name.Find('>') || wxNOT_FOUND!=name.Find('*') 
		|| wxNOT_FOUND!=name.Find('.') || wxNOT_FOUND!=name.Find(':') ) {
			mxMessageDialog(main_window,LANG(PROJECT_INVALID_CLASS_NAME,"El nombre de la clase no puede incluir ni espacios ni operadores"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
			return false;
		}
	// controlar que no exista
	wxString cpp_name = DIR_PLUS_FILE(project->path,(folder.Len()?DIR_PLUS_FILE(folder,name):name)+_T(".cpp"));
	wxString h_name = DIR_PLUS_FILE(project->path,(folder.Len()?DIR_PLUS_FILE(folder,name):name)+_T(".h"));
	if (wxFileName::FileExists(cpp_name)) {
		mxMessageDialog(main_window,LANG(PROJECT_WXFB_NEWFILE_EXISTS,"Ya existe un archivo con ese nombre. �Desea reemplazarlo?"),cpp_name,mxMD_YES_NO|mxMD_ERROR).ShowModal();
		return false;
	} else if (wxFileName::FileExists(h_name)) {
		mxMessageDialog(main_window,LANG(PROJECT_WXFB_NEWFILE_EXISTS,"Ya existe un archivo con ese nombre. �Desea reemplazarlo?"),h_name,mxMD_YES_NO|mxMD_ERROR).ShowModal();				
		return false;
	} else {
		wxArrayString methods;
		GetFatherMethods(base_header,base_name,methods);
		// crear el cpp
		wxTextFile cpp_file(cpp_name);
		cpp_file.Create();
		cpp_file.AddLine(wxString(_T("#include \""))+name+_T(".h\""));
		cpp_file.AddLine(_T(""));
		cpp_file.AddLine(name+_T("::")+name+_T("(wxWindow *parent) : ")+base_name+_T("(parent) {"));
		cpp_file.AddLine(_T("\t"));
		cpp_file.AddLine(_T("}"));
		cpp_file.AddLine(_T(""));
		for (unsigned int i=0;i<methods.GetCount();i++) {
			cpp_file.AddLine(methods[i].BeforeFirst(' ')+_T(" ")+name+_T("::")+methods[i].AfterFirst(' ')+_T(" {"));
			cpp_file.AddLine(_T("\tevent.Skip();"));
			cpp_file.AddLine(_T("}"));
			cpp_file.AddLine(_T(""));
		}
		cpp_file.AddLine(name+_T("::~")+name+_T("() {"));
		cpp_file.AddLine(_T("\t"));
		cpp_file.AddLine(_T("}"));
		cpp_file.AddLine(_T(""));
		cpp_file.Write();
		cpp_file.Close();
		// crear el h
		wxString def=name;
		def.MakeUpper();
		wxTextFile h_file(h_name);
		h_file.Create();
		h_file.AddLine(wxString(_T("#ifndef "))+def+_T("_H"));
		h_file.AddLine(wxString(_T("#define "))+def+_T("_H"));
		wxFileName fn_base_header(base_header);
		fn_base_header.MakeRelativeTo(folder);
		h_file.AddLine(wxString(_T("#include \""))+fn_base_header.GetFullPath()+_T("\""));
		h_file.AddLine(_T(""));
		h_file.AddLine(wxString(_T("class "))+name+_T(" : public ")+base_name+_T(" {"));
		h_file.AddLine(_T("\t"));
		h_file.AddLine(_T("private:"));
		h_file.AddLine(_T("\t"));
		h_file.AddLine(_T("protected:"));
		for (unsigned int i=0;i<methods.GetCount();i++)
			h_file.AddLine(wxString(_T("\t"))<<methods[i]+_T(";"));
		h_file.AddLine(_T("\t"));
		h_file.AddLine(_T("public:"));
		h_file.AddLine(wxString(_T("\t"))+name+_T("(wxWindow *parent=NULL);"));
		h_file.AddLine(wxString(_T("\t~"))+name+_T("();"));
		h_file.AddLine(_T("};"));
		h_file.AddLine(_T(""));
		h_file.AddLine(_T("#endif"));
		h_file.AddLine(_T(""));
		h_file.Write();
		h_file.Close();
		// abrir
		main_window->OpenFile(cpp_name,true);
		main_window->OpenFile(h_name,true);
		return true;
	}
}

void ProjectManager::SetActiveConfiguration (project_configuration * aconf) {
	active_configuration=aconf;
	main_window->SetToolchainMode(Toolchain::SelectToolchain().is_extern);
}

