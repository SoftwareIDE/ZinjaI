#include <wx/txtstrm.h>

#include "mxCompiler.h"

#include "ids.h"
#include "Language.h"
#include "mxMainWindow.h"
#include "Parser.h"
#include "mxMessageDialog.h"
#include "ProjectManager.h"
#include "ConfigManager.h"
#include "DebugManager.h"
#include "mxSource.h"
#include "mxUtils.h"

#define EN_NSTANTIATED_FROM "nstantiated from "
//#define EN_INSTANTIATED_FROM_HERE "nstantiated from here"
#define EN_IN_INSTANTIATION_OF "In instantiation of "
#define EN_COMPOUT_IN_FILE_INCLUDED_FROM "In file included from "
#define EN_COMPOUT_IN_FILE_INCLUDED_FROM_FROM "                 from "
#define EN_COMPOUT_LINKER_WARNING "Warning: "
#define EN_COMPOUT_WARNING ": warning: "
#define EN_COMPOUT_NOTE ": note: "
#define EN_COMPOUT_AT_THIS_POINT_IN_FILE ": at this point in file"
#define EN_COMPOUT_WITHIN_THIS_CONTEXT ": within this context"
#define EN_COMPOUT_IN_PASSING_ARGUMENT ": in passing argument"

#define ES_NSTANTIATED_FROM "nstantiated from "
//#define ES_INSTANTIATED_FROM_HERE "instantiated from here"
#define ES_IN_INSTANTIATION_OF "In instantiation of "
#define ES_COMPOUT_IN_FILE_INCLUDED_FROM "En el fichero incluído de "
#define ES_COMPOUT_IN_FILE_INCLUDED_FROM_FROM "                 de "
#define ES_COMPOUT_WARNING ": aviso: "
#define ES_COMPOUT_NOTE ": nota: "
#define ES_COMPOUT_AT_THIS_POINT_IN_FILE ": en este punto en el fichero"
#define ES_COMPOUT_WITHIN_THIS_CONTEXT ": en este contexto"
#define ES_COMPOUT_IN_PASSING_ARGUMENT ": en el paso del argumento"
#include "Toolchain.h"


wxMutex mxMuteCompiler;

mxCompiler *compiler=NULL;

compile_and_run_struct_single::compile_and_run_struct_single(const compile_and_run_struct_single *o) {
	*this=*o;
#ifdef DEBUG
	cerr<<"compile_and_run: *"<<mname<<endl;
	count++;
#endif
	mxMuteCompiler.Lock();
	prev=NULL;
	next=compiler->compile_and_run_single;
	if (next) next->prev=this;
	compiler->compile_and_run_single=this;
	mxMuteCompiler.Unlock();
}

compile_and_run_struct_single::compile_and_run_struct_single(const char *name) {
#ifdef DEBUG
	mname=name;
	cerr<<"compile_and_run: +"<<name<<endl;
	count++;
#endif
	killed=for_debug=compiling=special_output=linking=run_after_compile=last_error_item_IsOk=false;
	process=NULL;
	pid=parsing_flag=0;
	last_item_data=NULL;
	valgrind_cmd=compiler->valgrind_cmd;
	mxMuteCompiler.Lock();
	prev=NULL;
	next=compiler->compile_and_run_single;
	if (next) next->prev=this;
	compiler->compile_and_run_single=this;
	mxMuteCompiler.Unlock();
}

compile_and_run_struct_single::~compile_and_run_struct_single() {
	mxMuteCompiler.Lock();
	if (next) next->prev=prev;
	if (prev)
		prev->next=next;
	else
		compiler->compile_and_run_single=next;
	
	if (full_output.GetCount()) {
		for (unsigned int i=0;i<full_output.GetCount();i++)
			compiler->full_output.Add(full_output[i]);
	}
#ifdef DEBUG
	cerr<<"compile_and_run: -"<<mname<<endl;
	count--;
#endif
	mxMuteCompiler.Unlock();
}

wxString compile_and_run_struct_single::GetInfo() {
	wxString info;
	info<<pid<<" ";
	if (compiling) info<<"c";
	if (linking) info<<"l";
	if (for_debug) info<<"d";
	if (run_after_compile) info<<"r";
#ifdef DEBUG
	info<<" "<<mname;
	info<<" "<<step_label;
#endif
	return info;
}

#ifdef DEBUG
int compile_and_run_struct_single::count=0;
#endif

mxCompiler::mxCompiler(wxTreeCtrl *atree, wxTreeItemId s, wxTreeItemId e, wxTreeItemId w, wxTreeItemId a) {
	tree=atree; all=a; errors=e; warnings=w; state=s;
	last_runned=NULL;
	last_compiled=NULL;
	compile_and_run_single=NULL;
}

void mxCompiler::BuildOrRunProject(bool run, bool debug, bool prepared) {
	tree->SetItemText(state,LANG(GENERAL_PREPARING_BUILDING,"Preparando compilacion..."));
	main_window->SetStatusText(LANG(GENERAL_PREPARING_BUILDING,"Preparando compilacion..."));
	wxYield();
	ABORT_IF_PARSING;
	if (prepared || project->PrepareForBuilding()) { // si hay que compilar/enlazar
//		project->AnalizeConfig(project->path,true,config->mingw_real_path);
		wxString current;
		compile_and_run_struct_single *compile_and_run=new compile_and_run_struct_single("BuildOrRunProject 1");
		tree->DeleteChildren(all);
		full_output.Clear();
		project->compile_startup_time = time(NULL);
		ResetCompileData();
		for (unsigned int i=0;i<project->warnings.GetCount();i++)
			tree->AppendItem(warnings,project->warnings[i],7);
		num_warnings=project->warnings.GetCount();
		compile_and_run->run_after_compile=run; // hay que setearlo antes de CompileNext porque se va a duplicar ahi si hay paralelismo
		compile_and_run->compiling=true;
		compile_and_run->for_debug=debug;
		compile_and_run->pid=project->CompileNext(compile_and_run,current); // mandar a compilar el primero
		if (compile_and_run->pid) // si se puso a compilar algo
			main_window->StartExecutionStuff(true,run,compile_and_run,current);
		else {
			main_window->SetStatusText(LANG(MAINW_COMPILATION_INTERRUPTED,"Compilacion interrumpida."));
			delete compile_and_run;
		}
	} else {
		compiler->CheckForExecutablePermision(project->GetExePath());
		main_window->SetStatusText(LANG(MAINW_BINARY_ALREADY_UPDATED,"El binario ya esta actualizado."));
		if (run) { // si ya esta actualizado, ejecutar si se debe
			compile_and_run_struct_single *compile_and_run=new compile_and_run_struct_single("BuildOrRunProject 2");;
			project->Run(compile_and_run);
			main_window->StartExecutionStuff(false,true,compile_and_run,LANG(GENERAL_RUNNING_DOTS,"Ejecutando..."));
		} else {
			tree->SetItemText(state,LANG(MAINW_BINARY_ALREADY_UPDATED,"El binario ya esta actualizado"));
		}
	}
	
}

void mxCompiler::UnSTD(wxString &line) {
	int p;
	while ((p=line.find("[with "))!=wxNOT_FOUND) {
		int p5=p; p+=6;
		int p2,p3,p4,p6=p,lev=0;
		while (p6<int(line.Len())&&(lev>0||line[p6]!=']')) {
			if (line[p6]=='[') lev++;
			else if (line[p6]==']') lev--;
			p6++;
		}
		while ((p2=line.Mid(p).Find("="))!=wxNOT_FOUND&&p2+p<p6) {
			p3=(p2+=p);
			p2--; while (p2>0&&line[p2]==' ') p2--; p4=p2;
			lev=0; // no deberia ser necesario
			while (p2>=p&&(lev>0||(line[p2]!='['&&line[p2]!=','))) {
				if (line[p2]=='>'||line[p2]==')') lev++;
				else if (line[p2]=='<'||line[p2]=='(') lev--;
				p2--;
			}
			p2++; while (p2<int(line.Len())&&line[p2]==' ') p2++;
			wxString type_in=line.Mid(p2,p4-p2+1);
			lev=0; // no deberia ser necesario
			p3++; while (p3<int(line.Len())&&line[p3]==' ') p3++; p4=p3;
			while (lev>0||(line[p3]!=']'&&line[p3]!=',')) {
				if (line[p3]=='('||line[p3]=='<') lev++;
				else if (line[p3]==')'||line[p3]=='>') lev--;
				p3++;
			}
			p3--; while (p3>0&&line[p3]==' ') p3--;
			wxString type_out=line.Mid(p4,p3-p4+1);
			p=p4; p4=0;
			while ((p2=line.Mid(p4).Find(type_in))!=wxNOT_FOUND && p4+p2<p5) {
				p4+=p2;
				int diff;
				line.replace(p4,type_in.size(),type_out);
				diff=type_out.size()-type_in.size();
				p4+=diff;
				p+=diff;
				p5+=diff;
				p6+=diff;
			}
			
		}
		line.erase(p5,p6-p5+1);
	}
	
	// quitar los allocators
	while ((p=line.Find("std::allocator<"))!=wxNOT_FOUND) { 
		int p2=p+15, lev=0;
		// buscar donde empieza (puede haber un const y espacios antes)
		while (p>1&&line[p-1]==' ') p--;
		if (p>5 && line.Mid(p-5,5)=="const") {
			p-=5;
			while (p>1 && line[p-1]==' ') p--;
		}
		// buscar donde termina el argumento de su template
		while (p2<int(line.Len())&&(line[p2]!='>'||lev>0)) {
			if (line[p2]=='<') lev++;
			else if (line[p2]=='>') lev--;
			p2++;
		}
		// saltar espacios y &
		while (p2<int(line.Len())&&(line[p2+1]==' '||line[p2+1]=='&')) p2++; 
		// quitar una coma si estaba en una lista de argumentos
		if (line[p-1]==',') { p--; while (p>1&&line[p-1]==' ') p--; }
		else if (line[p2]==',') { p2++; while (p2<int(line.Len())&&line[p2]==' ') p2++; }
		line=line.Mid(0,p)+line.Mid(p2+1);
	}
	while ((p=line.Find("std::basic_string<"))!=wxNOT_FOUND) { 
		int p2=p+18, lev=0;
		while (p>1&&line[p-1]==' ') p--;
		if (p>5 && line.Mid(p-5,5)=="const") {
			p-=5;
			while (p>1 && line[p-1]==' ') p--;
		}
		while (p2<int(line.Len())&&(line[p2]!='>'||lev>0)) {
			if (line[p2]=='<') lev++;
			else if (line[p2]=='>') lev--;
			p2++;
		}
		line=line.Mid(0,p)+"string"+line.Mid(p2+1);
	}
	// cambiar los "basic_strig" por "string"
	p=0; int p2;
	while ((p2=line.Mid(p).Find("basic_string"))!=wxNOT_FOUND) { 
		p+=p2;
		if (p>0&&((line[p-1]|32)<'a'||(line[p-1]|32)>'z')&&((line[p+12]|32)<'a'||(line[p+12]|32)>'z'))
			line.erase(p,6);
		else
			p+=12;
	}
	// quitar los "std::"
	p=0;
	while ((p2=line.Mid(p).Find("std::"))!=wxNOT_FOUND) { 
		p+=p2;
		if (p>0&&((line[p-1]|32)<'a'||(line[p-1]|32)>'z'))
			line.erase(p,5);
		else
			p+=5;
	}
	line.Replace("typename::rebind<char>::other::size_type","size_t");
}


void mxCompiler::ParseSomeErrors(compile_and_run_struct_single *compile_and_run) {
	wxProcess *process = compile_and_run->process;
	static wxString error_line, nice_error_line;
	int p,l;
//	char c;
	wxTextInputStream input(*(process->GetErrorStream()));	
	while ( process->IsErrorAvailable() ) {
//		error_line.Clear();
//		while (process->IsErrorAvailable()) {
//			c=input.GetChar();
//			if (c!='\r' && c!='\n') error_line<<c;
//			else break;
//		}
//		if (!(c=='\r'||c=='\n')) return;
		
		error_line=input.ReadLine();
		
		// acortar el nombre de archivo
		if ( error_line[1]==':' ) p=2; else p=0;
		l=error_line.Len();
		while (p<l && error_line[p]!=':') p++;
		if (p<l) {
			while (p>=0 && error_line[p]!='/' && error_line[p]!='\\') p--;
			nice_error_line=error_line.Mid(p+1);
		} else 
			nice_error_line=error_line;
		
		compile_and_run->full_output.Add(error_line);
		if (compile_and_run->special_output || error_line.Len()==0) 
			continue;
		num_all++;
		tree->AppendItem(compile_and_run->last_all_item,nice_error_line,6,-1,new mxCompilerItemData(error_line));
		if (num_all<config->Init.max_errors) {
//		if (compile_and_run.parsing_flag==2) {
//			compile_and_run.parsing_flag=0;
//			if (wxNOT_FOUND!=error_line.Find(_T(EN_COMPOUT_WARNING)) || wxNOT_FOUND!=error_line.Find(_T(ES_COMPOUT_WARNING)))
//				compile_and_run.last_error_item=tree->AppendItem(warnings,error_line,3);
//			else
//				compile_and_run.last_error_item=tree->AppendItem(errors,error_line,4);
//		}
			
			// reemplazar templates para que sea más legible
			if (config->Init.beautify_compiler_errors) UnSTD(nice_error_line);
			
			if (error_line.Last()!=',' && ( error_line.Last()!=':' || error_line.Find(_T(": error: "))!=wxNOT_FOUND || wxNOT_FOUND!=error_line.Find(_T(EN_COMPOUT_WARNING)) || wxNOT_FOUND!=error_line.Find(_T(ES_COMPOUT_WARNING)) || error_line.StartsWith(EN_COMPOUT_LINKER_WARNING)) ) {
				bool flag;
				if ( (flag=(wxNOT_FOUND!=error_line.First(_T(ES_NSTANTIATED_FROM)) || wxNOT_FOUND!=error_line.First(_T(EN_NSTANTIATED_FROM)))) || wxNOT_FOUND!=error_line.First(_T(EN_COMPOUT_AT_THIS_POINT_IN_FILE)) || wxNOT_FOUND!=error_line.Find(_T(ES_COMPOUT_AT_THIS_POINT_IN_FILE)) || wxNOT_FOUND!=error_line.First(_T(EN_COMPOUT_WITHIN_THIS_CONTEXT)) || wxNOT_FOUND!=error_line.Find(_T(ES_COMPOUT_WITHIN_THIS_CONTEXT)) ) {
					if (!flag && error_line.Right(4)!=_T("here")) {
						if (compile_and_run->parsing_flag==1) {
							compile_and_run->parsing_flag=2;
							continue;
//						compile_and_run.parsing_aux=error_line;
						} else {
							if (compile_and_run->last_error_item_IsOk) {
								wxString last=tree->GetItemText(compile_and_run->last_error_item);
								last.Replace(last.Left(last.Find(_T(": "))),error_line.Left(error_line.Find(_T(": "))));
								compile_and_run->last_item_data->Set(last);
								tree->SetItemText(compile_and_run->last_error_item,last);
							} else {
								mxMessageDialog(main_window,LANG(MAINW_COMPILER_OUTPUT_PARSING_ERROR,"ZinjaI ha intentado reacomodar la salida del compilador de forma incorrecta.\n"
									"Puede que algun error no se muestre correctamente en el arbol. Para ver la salida\n"
									"completa haga click con el boton derecho del raton en cualquier elemento del arbol\n"
									"y seleccione \"abrir ultima salida\". Para contribuir al desarrollo de ZinjaI puede\n"
									"enviar esta salida, o el codigo que la ocasiono a zaskar_84@yahoo.com.ar"
									),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
							}
						}
					}
//			} else if (wxNOT_FOUND!=error_line.First(_T(ES_INSTANTIATED_FROM_HERE)) || wxNOT_FOUND!=error_line.First(_T(EN_INSTANTIATED_FROM_HERE)) || wxNOT_FOUND!=error_line.First(_T(EN_COMPOUT_AT_THIS_POINT_IN_FILE)) || wxNOT_FOUND!=error_line.Find(_T(ES_COMPOUT_AT_THIS_POINT_IN_FILE)) || wxNOT_FOUND!=error_line.First(_T(EN_COMPOUT_WITHIN_THIS_CONTEXT)) || wxNOT_FOUND!=error_line.Find(_T(ES_COMPOUT_WITHIN_THIS_CONTEXT)) ){
//				wxString last=tree->GetItemText(compile_and_run.last_error_item);
//				last.Replace(last.Left(last.Find(": ")),error_line.Left(error_line.Find(": ")));
//				tree->SetItemText(compile_and_run.last_error_item,last);
				} else if (
					wxNOT_FOUND!=error_line.First(_T(EN_COMPOUT_NOTE)) || wxNOT_FOUND!=error_line.Find(_T(ES_COMPOUT_NOTE)) 
					|| wxNOT_FOUND!=error_line.First(_T(EN_COMPOUT_IN_PASSING_ARGUMENT)) || wxNOT_FOUND!=error_line.Find(_T(ES_COMPOUT_IN_PASSING_ARGUMENT)) 
					|| (wxNOT_FOUND!=error_line.Find(_T("(")) && error_line[error_line.Find(_T("("))-2]==':' && error_line[error_line.Find(_T("("))-3]!=':') 
					) {
						if (compile_and_run->last_error_item_IsOk) {
							tree->AppendItem(compile_and_run->last_error_item,nice_error_line,5,-1,new mxCompilerItemData(error_line));
						} else {
							mxMessageDialog(main_window,LANG(MAINW_COMPILER_OUTPUT_PARSING_ERROR,"ZinjaI ha intentado reacomodar la salida del compilador de forma incorrecta.\n"
								"Puede que algun error no se muestre correctamente en el arbol. Para ver la salida\n"
								"completa haga click con el boton derecho del raton en cualquier elemento del arbol\n"
								"y seleccione \"abrir ultima salida\". Para contribuir al desarrollo de ZinjaI puede\n"
								"enviar esta salida, o el codigo que la ocasiono a zaskar_84@yahoo.com.ar"
								),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
						}
					} else if ( wxNOT_FOUND!=error_line.Find(_T(EN_COMPOUT_WARNING)) || wxNOT_FOUND!=error_line.Find(_T(ES_COMPOUT_WARNING)) || error_line.StartsWith(EN_COMPOUT_LINKER_WARNING) ) {
						num_warnings++;
						compile_and_run->last_error_item=tree->AppendItem(warnings,nice_error_line,3,-1,compile_and_run->last_item_data=new mxCompilerItemData(error_line));
						compile_and_run->last_error_item_IsOk=true;
					} else {
						num_errors++;
						compile_and_run->last_error_item=tree->AppendItem(errors,nice_error_line,4,-1,compile_and_run->last_item_data=new mxCompilerItemData(error_line));
						compile_and_run->last_error_item_IsOk=true;
					}
			} else {
				if (wxNOT_FOUND!=error_line.Find(_T(EN_IN_INSTANTIATION_OF)) || wxNOT_FOUND!=error_line.Find(_T(ES_IN_INSTANTIATION_OF)))
					compile_and_run->parsing_flag = 1;
			}
		}
	}
}

void mxCompiler::ParseCompilerOutput(compile_and_run_struct_single *compile_and_run, bool success) {
	
	// poner los errores/warnings/etc en el arbol
	ParseSomeErrors(compile_and_run); 
	tree->SetItemText(errors,wxString(LANG(MAINW_CT_ERRORS,"Errores"))<<_T(" (")<<num_errors<<(num_all>config->Init.max_errors?_T("+"):_T(""))<<_T(")"));
	tree->SetItemText(warnings,wxString(LANG(MAINW_CT_WARNINGS,"Advertencias"))<<_T(" (")<<num_warnings<<(num_all>config->Init.max_errors?_T("+"):_T(""))<<_T(")"));
	
	delete compile_and_run->process; compile_and_run->process=NULL; // liberar el proceso
	
	// ver como sigue
	if (success) { // si el proceso termino correctamente (no hay errores/problemas)
		if (project && (compile_and_run->compiling||compile_and_run->linking)) { // si es proyecto
			
//			compile_and_run->compiling=false; // para que se pone en falso?
			
			wxString current;
			if ((project->compile_was_ok||!config->Init.stop_compiling_on_error) && project->CompileNext(compile_and_run,current)) { // si se puso a compilar algo
				main_window->StartExecutionStuff(true,compile_and_run->run_after_compile,compile_and_run,current);
			} else {
				if (compiler->IsCompiling()) { // si queda otro en paralelo
					delete compile_and_run; return;
				}
				if (project->compile_was_ok) { // si esta todo listo, informar el resultado
					if (compile_and_run->linking && !project->active_configuration->dont_generate_exe) CheckForExecutablePermision(project->GetExePath());
					main_window->SetStatusProgress(0);
					time_t elapsed_time = time(NULL)-project->compile_startup_time;
					if (elapsed_time>5) {
						wxString status_text(LANG(MAINW_COMPILING_DONE_TIME_PRE,"Compilacion finalizada ( tiempo transcurrido: "));
						if (elapsed_time/60==0)
							status_text<<elapsed_time%60<<LANG(MAINW_COMPILING_DONE_POST_SECONDS," segundos ).");
						else if (elapsed_time/60==1)
							status_text<<LANG(MAINW_COMPILING_DONE_MID_ONE_MINUTE,"un minuto y ")<<elapsed_time%60<<_T(" segundos ).");
						else
							status_text<<elapsed_time/60<<LANG(MAINW_COMPILING_DONE_MID_MINUTES," minutos y ")<<elapsed_time%60<<LANG(MAINW_COMPILING_DONE_POST_SECONDS," segundos ).");
							main_window->SetStatusText(status_text);
					} else {
						main_window->SetStatusText(LANG(MAINW_COMPILING_DONE,"Compilacion finalizada."));
					}
					tree->Expand(warnings);
					tree->SetItemText(state,LANG(MAINW_COMPILING_DONE,"Compilacion Finalizada"));
					// ejecutar o depurar
					if (compile_and_run->run_after_compile) {
						if (compile_and_run->for_debug) {
							delete compile_and_run;
							debug->Start(false);
						} else {
							compile_and_run->pid = project->Run(compile_and_run);
							main_window->StartExecutionStuff(false,true,compile_and_run,LANG(GENERAL_RUNNING_DOTS,"Ejecutando..."));
						}
					} else
						delete compile_and_run;
				} else {
					compile_and_run->compiling=false;
					tree->SetItemText(state,LANG(MAINW_COMPILATION_INTERRUPTED,"Compilacion interrumpida!"));
					main_window->SetStatusProgress(0);
					main_window->SetStatusText(LANG(MAINW_COMPILATION_INTERRUPTED,"Compilacion interrumpida!"));
					main_window->ShowCompilerTreePanel();
					delete compile_and_run;
				}
			}
		} else { // si era un ejercicio simple, ver si hay que ejecutar
			CheckForExecutablePermision(last_compiled->binary_filename.GetFullPath());
			main_window->SetStatusProgress(0);
			main_window->SetStatusText(LANG(GENERAL_READY,"Listo"));
			tree->SetItemText(state,LANG(MAINW_COMPILING_DONE,"Compilacion Finalizada"));
			tree->Expand(warnings);
			if (compile_and_run->run_after_compile && last_compiled) { // ejecutar o depurar
				if (compile_and_run->for_debug)
					debug->Start(false,last_compiled);
				else {
					valgrind_cmd=compile_and_run->valgrind_cmd;
					main_window->RunSource(last_compiled);
					valgrind_cmd="";
				}
			}
			delete compile_and_run;
		}
	} else { // si fallo la compilacion
		// informar y no seguir
		if (!project || project->compile_was_ok) {
			main_window->SetStatusProgress(0);
			main_window->SetStatusText(LANG(MAINW_COMPILATION_INTERRUPTED,"Compilacion interrumpida!"));
			compile_and_run->compiling=false;
			wxBell();
			if (compile_and_run->special_output)
				tree->SetItemText(state,wxString(_T("Error en: "))<<compile_and_run->step_label);
			else
				tree->SetItemText(state,LANG(MAINW_COMPILATION_INTERRUPTED,"Compilacion interrumpida!"));
			tree->Expand(errors);
			main_window->ShowCompilerTreePanel();
		}
		if (project) {
			project->compile_was_ok=false;
			if (!config->Init.stop_compiling_on_error) {
				wxString current;
				 // mandar a compilar el que sigue
				if (project->CompileNext(compile_and_run,current)) { // si se puso a compilar algo
					main_window->StartExecutionStuff(true,compile_and_run->run_after_compile,compile_and_run,current);
					return;
				} else {
					main_window->SetStatusProgress(0);
					main_window->SetStatusText(LANG(MAINW_COMPILATION_INTERRUPTED,"Compilacion interrumpida!"));
				}
			}
		}
		delete compile_and_run;
	}
}

void mxCompiler::CompileSource (mxSource *source, bool run, bool debug) {
	compile_and_run_struct_single *compile_and_run=new compile_and_run_struct_single("CompileSource");;
	compile_and_run->for_debug=debug;
	compile_and_run->special_output=false;
	parser->ParseSource(source,true);
	last_compiled=source;
	wxString z_opts(wxString(_T(" ")));
	bool cpp = source->sin_titulo || (source->source_filename.GetExt()!=_T("C") && source->source_filename.GetExt()!=_T("c"));
	if (config->Debug.format.Len()) z_opts<<config->Debug.format<<_T(" ");
	z_opts<<(cpp?current_toolchain.cpp_compiling_options:current_toolchain.c_compiling_options)<<" "; // forced compiler arguments
	z_opts<<current_toolchain.linker_options<<" "; // forced linker arguments
	z_opts<<" -g "; // always include debugging information
	wxString ext=source->source_filename.GetExt();
	if (!source->sin_titulo && (!ext.Len()||(ext[0]>='0'&&ext[0]<='9')))
		z_opts<<_T("-x c++ "); // avoid not recognizing files without extension
	// prepare command line
	wxString comp_opts = source->GetParsedCompilerOptions();
	wxString command = wxString(cpp?current_toolchain.cpp_compiler:current_toolchain.c_compiler)+z_opts+_T("\"")+(main_window->GetCurrentSource()->sin_titulo?source->temp_filename:source->source_filename).GetFullPath()+_T("\" ")+comp_opts+_T(" -o \"")+source->binary_filename.GetFullPath()<<_T("\"");
	
	// lanzar la ejecucion
	compile_and_run->process=new wxProcess(main_window->GetEventHandler(),mxPROCESS_COMPILE);
	compile_and_run->process->Redirect();
	ResetCompileData();
	if (!source->sin_titulo) {
		compile_and_run->pid = utils->Execute(source->source_filename.GetPath(),command,wxEXEC_ASYNC,compile_and_run->process);
		main_window->StartExecutionStuff(true,run,compile_and_run,LANG(MAINW_COMPILING_DOTS,"Compilando..."));
	} else {
		compile_and_run->pid = wxExecute(command, wxEXEC_ASYNC,compile_and_run->process);
		main_window->StartExecutionStuff(true,run,compile_and_run,LANG(MAINW_COMPILING_DOTS,"Compilando..."));
	}
	compile_and_run->full_output.Add(_T(""));
	compile_and_run->full_output.Add(wxString(_T("> "))+command);
	compile_and_run->full_output.Add(_T(""));
}

// incializar el arbol de compilacion y sus auxiliares
void mxCompiler::ResetCompileData() {
	full_output.Clear();
	num_errors=num_warnings=num_all=0;
	tree->SetItemText(errors,LANG(MAINW_CT_ERRORS,"Errores"));
	tree->SetItemText(warnings,LANG(MAINW_CT_WARNINGS,"Advertencias"));
	tree->DeleteChildren(errors);
	tree->DeleteChildren(warnings);
	if (!project)  {
		tree->DeleteChildren(all);
	} else {
		project->compile_startup_time=time(NULL);
	}
	tree->Collapse(errors);
	tree->Collapse(warnings);
	if (project)
		tree->Expand(all);
	else
		tree->Collapse(all);
}

int mxCompiler::NumCompilers() {
	int n=0;
	mxMuteCompiler.Lock();
	compile_and_run_struct_single *item = compile_and_run_single;
	while (item) {
		if (item->compiling && item->pid!=0) n++;
		item=item->next;
	}
	mxMuteCompiler.Unlock();
	return n;
}
bool mxCompiler::IsCompiling() {
	mxMuteCompiler.Lock();
	compile_and_run_struct_single *item = compile_and_run_single;
	while (item) {
		if (item->compiling && item->pid!=0) {
			mxMuteCompiler.Unlock();
			return true;
		}
		item=item->next;
	}
	mxMuteCompiler.Unlock();
	return false;
}

wxString mxCompiler::GetCompilingStatusText() {
	wxString text=LANG(PROJMNGR_COMPILING,"Compilando");
	char sep=' ',com='\"';
	compile_and_run_struct_single *item = compile_and_run_single;
	while (item) {
		if (item->compiling && item->pid!=0) {
			text<<sep<<com<<item->step_label<<com;
			sep=',';
		}
		item=item->next;
	}
	text<<_T("\"...");
	return text;
}

bool mxCompiler::CheckForExecutablePermision(wxString file) {
#ifndef __WIN32__
	if (!wxFileName::IsFileExecutable(file)) {
		tree->AppendItem(errors,LANG(MAINW_WARNING_NO_EXCUTABLE_PERMISSION,"El binario no tiene permisos de ejecución."),7);
		main_window->ShowCompilerTreePanel();
		tree->Expand(errors);
		return false;
	}
#endif
	return true;
}
