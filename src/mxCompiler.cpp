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
#include "DebugPatcher.h"
#include "mxSource.h"
#include "mxUtils.h"
#include "Toolchain.h"
#include "CodeHelper.h"
#include "mxExternCompilerOutput.h"


#define EN_COMPOUT_FATAL_ERROR ": fatal error: "
#define EN_COMPOUT_ERROR ": error: "
#define EN_COMPOUT_ERROR_CHILD ": error:   "
#define EN_COMPOUT_WARNING ": warning: "
#define EN_COMPOUT_WARNING_CHILD ": warning:   "
#define EN_COMPOUT_LINKER_WARNING "Warning: "

#define EN_COMPOUT_COMPILATION_TERMINATED "compilation terminated."
#define EN_COMPOUT_REQUIRED_FROM_HERE "required from "
//#define EN_COMPOUT__NSTANTIATED_FROM "nstantiated from "
#define EN_COMPOUT_IN_INSTANTIATION_OF "In instantiation of "
#define EN_COMPOUT_AT_THIS_POINT_IN_FILE ": at this point in file"
#define EN_COMPOUT_WITHIN_THIS_CONTEXT ": within this context"
#define EN_COMPOUT_NOTE ": note: "
#define EN_COMPOUT_IN_FILE_INCLUDED_FROM "In file included from "
#define EN_COMPOUT_IN_EXPANSION_OF_MACRO "in expansion of macro"

//#define EN_COMPOUT_IN_PASSING_ARGUMENT ": in passing argument"
//#define EN_COMPOUT_FORWARD_DECLARATION_OF ": forward declaration of "

#define ES_COMPOUT_FATAL_ERROR ": error fatal: "
#define ES_COMPOUT_ERROR ": error: "
#define ES_COMPOUT_ERROR_CHILD ": error:   "
#define ES_COMPOUT_WARNING ": aviso: "
#define ES_COMPOUT_WARNING_CHILD ": aviso:   "
#define ES_COMPOUT_LINKER_WARNING "Advertencia: "


#define ES_COMPOUT_COMPILATION_TERMINATED "compilaci�n terminada."
#define ES_COMPOUT_REQUIRED_FROM_HERE "se requiere desde "
//#define ES_COMPOUT__NSTANTIATED_FROM "nstantiated from "
#define ES_COMPOUT_IN_INSTANTIATION_OF "In instantiation of "
#define ES_COMPOUT_AT_THIS_POINT_IN_FILE ": en este punto en el fichero"
#define ES_COMPOUT_WITHIN_THIS_CONTEXT ": en este contexto"
#define ES_COMPOUT_NOTE ": nota: "
#define ES_COMPOUT_IN_FILE_INCLUDED_FROM "En el fichero inclu�do de "
//#define ES_COMPOUT_WARNING ": aviso: "
//#define ES_COMPOUT_IN_PASSING_ARGUMENT ": en el paso del argumento"
//#define ES_COMPOUT_FORWARD_DECLARATION_OF ": forward declaration of "

#ifdef _ZINJAI_DEBUG
#include<iostream>
#include <cstdlib>
using namespace std;
#endif

wxMutex mxMutexCompiler;

mxCompiler *compiler=nullptr;

static bool EnsureCompilerNotRunning() {
	compile_and_run_struct_single *compile_and_run=compiler->compile_and_run_single;
	while (compile_and_run && !compile_and_run->compiling && !compile_and_run->linking) 
		compile_and_run=compile_and_run->next;
	if (compile_and_run) return false;
	return true;
}

compile_and_run_struct_single::compile_and_run_struct_single(const compile_and_run_struct_single *o) {
	*this=*o; 
	on_end=nullptr; // para evitar doble-delete, igual nunca deberia tener un on_end porque esto solo se usa para fuentes de un proyecto
#ifdef _ZINJAI_DEBUG
	cerr<<"compile_and_run: *"<<mname<<endl;
	count++;
#endif
	mxMutexCompiler.Lock();
	prev=nullptr;
	next=compiler->compile_and_run_single;
	if (next) next->prev=this;
	compiler->compile_and_run_single=this;
	mxMutexCompiler.Unlock();
}

compile_and_run_struct_single::compile_and_run_struct_single(const char *name) {
	error_line_flag=CAR_LL_NULL;
#ifdef _ZINJAI_DEBUG
	mname=name;
	cerr<<"compile_and_run: +"<<name<<endl;
	count++;
#endif
	on_end=nullptr;
	killed=/*for_debug=*/compiling=linking=/*run_after_compile=*/last_error_item_IsOk=false;
	parsing_errors_was_ok=true;
	output_type=MXC_NULL;
	process=nullptr;
	pid=0;
	valgrind_cmd=compiler->valgrind_cmd;
	mxMutexCompiler.Lock();
	prev=nullptr;
	next=compiler->compile_and_run_single;
	if (next) next->prev=this;
	compiler->compile_and_run_single=this;
	mxMutexCompiler.Unlock();
}

compile_and_run_struct_single::~compile_and_run_struct_single() {
	delete on_end;
	mxMutexCompiler.Lock();
	if (next) next->prev=prev;
	if (prev)
		prev->next=next;
	else
		compiler->compile_and_run_single=next;
	
	if (full_output.GetCount()) {
		for (unsigned int i=0;i<full_output.GetCount();i++)
			compiler->full_output.Add(full_output[i]);
	}
#ifdef _ZINJAI_DEBUG
	cerr<<"compile_and_run: -"<<mname<<endl;
	count--;
#endif
	mxMutexCompiler.Unlock();
}

wxString compile_and_run_struct_single::GetInfo() {
	wxString info;
	info<<pid<<" ";
	if (compiling) info<<"c";
	if (linking) info<<"l";
//	if (for_debug) info<<"d";
//	if (run_after_compile) info<<"r";
#ifdef _ZINJAI_DEBUG
	info<<" "<<mname;
	info<<" "<<step_label;
#endif
	return info;
}

#ifdef _ZINJAI_DEBUG 
int compile_and_run_struct_single::count=0;
#endif

mxCompiler::mxCompiler(wxTreeCtrl *atree, wxTreeItemId s, wxTreeItemId e, wxTreeItemId w, wxTreeItemId a) {
	tree=atree; all=a; errors=e; warnings=w; state=s;
	last_runned=nullptr;
	last_compiled=nullptr;
	compile_and_run_single=nullptr;
}

/**
* @param on_end En cualquier caso se hacer cargo de esta accion. Si hay que 
*               compilar se la transfiere al proyecto, si est� todo listo
*               la ejecuta y le hace luego el delete.
**/
void mxCompiler::BuildOrRunProject(bool prepared, GenericAction *on_end) {
	RaiiDelete<GenericAction> ga_del(on_end);
	if (project->GetWxfbActivated() && project->GetWxfbConfiguration()->working) return;
	main_window->extern_compiler_output->Clear();
	main_window->SetCompilingStatus(LANG(GENERAL_PREPARING_BUILDING,"Preparando compilacion..."));
DEBUG_INFO("wxYield:in  mxCompiler::BuildOrRunProject");
	wxYield();
DEBUG_INFO("wxYield:out mxCompiler::BuildOrRunProject");
	if (prepared || project->PrepareForBuilding(nullptr)) { // si hay que compilar/enlazar
		if (!EnsureCompilerNotRunning()) return;
		fms_move( fms_delete(project->post_compile_action), on_end );
//		project->AnalizeConfig(project->path,true,config->mingw_real_path);
		wxString current;
		compile_and_run_struct_single *compile_and_run=new compile_and_run_struct_single("BuildOrRunProject 1");
		tree->DeleteChildren(all);
		full_output.Clear();
		project->compile_startup_time = time(nullptr);
		ResetCompileData();
		for (unsigned int i=0;i<project->warnings.GetCount();i++)
			tree->AppendItem(warnings,project->warnings[i],7);
		num_warnings=project->warnings.GetCount();
		compile_and_run->compiling=true;
		compile_and_run->pid=project->CompileNext(compile_and_run,current); // mandar a compilar el primero
		if (compile_and_run->pid) // si se puso a compilar algo
			main_window->StartExecutionStuff(compile_and_run,current);
		else {
			main_window->SetStatusText(LANG(MAINW_COMPILATION_INTERRUPTED,"Compilacion interrumpida."));
			delete compile_and_run;
		}
	} else {
		compiler->CheckForExecutablePermision(project->GetExePath());
		main_window->SetStatusText(LANG(MAINW_BINARY_ALREADY_UPDATED,"El binario ya esta actualizado."));
		if (on_end) { // si ya esta actualizado, ejecutar si se debe
			on_end->Do(); 
		} else {
			tree->SetItemText(state,LANG(MAINW_BINARY_ALREADY_UPDATED,"El binario ya esta actualizado"));
		}
	}	
}

void mxCompiler::UnSTD(wxString &line) {
	int p;
	while ((p=line.find("[with "))!=wxNOT_FOUND) { // buscar un argumento actual para plantilla
		int p0=p;  // pos donde inicia el argumento actual "[with booga=...."
		int p1=p;  // pos donde termina el argumento actual "...]"
		int lev=0; // auxiliar para ignorar corchetes anidados
		p+=6; // salteat el "[with " y buscar el corchete que cierra
		while (p1<int(line.Len())&&(lev>0||line[p1]!=']')) {
			if (line[p1]=='[') lev++;
			else if (line[p1]==']') lev--;
			p1++;
		}
		int pos_last_igual=p0; 
		int pos_igual=0; // posici�n donde encuentra un = que corresponda a esa lista de argumentos
		while (pos_last_igual<p1 && (pos_igual=line.SubString(pos_last_igual+1,p1).Find("="))!=wxNOT_FOUND) {
			pos_igual+=pos_last_igual+1; // para que quede como pos absoluta
			pos_last_igual=pos_igual;
			
			// buscar que hay a la izquierda del igual
			int pre_fin=pos_igual-1;  // pos donde termina el termino de la izquierda
			while (pre_fin>0&&line[pre_fin]==' ') pre_fin--;
			int pre_ini=pre_fin; // pos donde empieza el termino de la izquierda
			lev=0; // no deberia ser necesario
			while (pre_ini>=p0+6&&(lev>0||(line[pre_ini]!='['&&line[pre_ini]!=','))) {
				if (line[pre_ini]=='>'||line[pre_ini]==')') lev++;
				else if (line[pre_ini]=='<'||line[pre_ini]=='(') lev--;
				pre_ini--;
			}
			pre_ini++; 
			while (pre_ini<pre_fin&&line[pre_ini]==' ') pre_ini++;
			wxString type_in=line.SubString(pre_ini,pre_fin); // tipo que sera reemplazado
			
			// buscar que hay a la derecha del igual
			int pos_ini=pos_igual+1; // pos donde empieza el termino de la derecha
			while (pos_ini<p1&&line[pos_ini]==' ') pos_ini++;
			int pos_fin=pos_ini; // pos donde termina el termino de la derecha
			lev=0; // no deberia ser necesario
			while (pos_fin<p1&&(lev>0||(line[pos_fin]!=']'&&line[pos_fin]!=','))) {	
				if (line[pos_fin]=='<'||line[pos_fin]=='(') lev++;
				else if (line[pos_fin]==')'||line[pos_fin]=='>') lev--;
				pos_fin++;
			}
			pos_fin--;
			while (pos_fin>pos_ini&&line[pos_fin]==' ') pos_fin--;
			wxString type_out=line.SubString(pos_ini,pos_fin); // tipo con el cual se reemplazar�
			
			int p_in=0, diff=type_out.size()-type_in.size();
			while ((p=line.SubString(p_in,p0).Find(type_in))!=wxNOT_FOUND && p+p_in<p0) {
				p+=p_in; // pos absoluta donde reemplazar
				line.replace(p,type_in.size(),type_out);
				p0+=diff; p1=diff; pos_last_igual+=diff; // corregir las pos posteriores que interesen mantener
				p_in=p+type_out.size(); // para que la proxima busqueda empiece despues (porque puede aparecer "[with t=t]")
			}
			
		}
		line.erase(p0,p1-p0+1);
	}
	
	// quitar los allocators y otras cosas
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

static inline bool ErrorLineIsChild(const wxString &error_line) {
	// si el mensaje de error empieza con 3 espacios, es porque en realidad sigue del mensaje anterior
	int p=2,l=error_line.Len();
	while (p<l && error_line[p]!=':') p++; // saltear el nombre del archivo
	if (p==l) return false; else p++;
	while (p<l && error_line[p]!=':') p++; // saltear el numero de linea
	if (p==l) return false; else p++;
	while (p<l && error_line[p]!=':') p++; // saltear el la columna
	if (p==l) return false; else p++;
	if (p+2<l && error_line[p]==' '&&error_line[p+1]==' '&&error_line[p+2]==' ') return true; // puede venir directo el mensaje
	if (p+5<l && error_line[p+1]=='n'&&error_line[p+2]=='o'&&error_line[p+3]=='t'&&error_line[p+5]==':') return true; // o la palabra note
	while (p<l && error_line[p]!=':') p++; // o la palabra error/warning/
	if (p==l) return false; else p++;
	if (p+2<l && error_line[p]==' '&&error_line[p+1]==' '&&error_line[p+2]==' ') return true; // puede venir directo el mensaje
	return false;
}


CAR_ERROR_LINE mxCompiler::ParseSomeErrorsOneLine(compile_and_run_struct_single *compile_and_run, const wxString &error_line) {
	
	// if it starts or continues an "In included file from ....., \n from ...., \n from .....:" list, ignore
	if (compile_and_run->error_line_flag==CAR_LL_IN_INCLUDED_FILE || error_line.Contains(EN_COMPOUT_IN_FILE_INCLUDED_FROM) || error_line.Contains(ES_COMPOUT_IN_FILE_INCLUDED_FROM) ) {
		if (error_line.Last()==',') compile_and_run->error_line_flag=CAR_LL_IN_INCLUDED_FILE;
		else compile_and_run->error_line_flag=CAR_LL_NULL;
		return CAR_EL_IGNORE;
	}
	compile_and_run->error_line_flag=CAR_LL_NULL;
	
	// if there was an error within the compiler stop
	if (error_line==EN_COMPOUT_COMPILATION_TERMINATED || error_line==ES_COMPOUT_COMPILATION_TERMINATED) return CAR_EL_ERROR;
	
	// if it starts or continue a list of template instantiation chain: "In instantiation of ....: \n required from ...\n required from ..."
	if (error_line.Contains(EN_COMPOUT_IN_INSTANTIATION_OF) || error_line.Contains(ES_COMPOUT_IN_INSTANTIATION_OF)
		|| error_line.Contains(EN_COMPOUT_REQUIRED_FROM_HERE) || error_line.Contains(ES_COMPOUT_REQUIRED_FROM_HERE)) {
		return CAR_EL_CHILD_NEXT;
	}
	
	// esto va antes de testear si es error o warning, porque ambos tests (por ejemplo, este y el de si es error) pueden dar verdaderos, y en ese caso debe primar este
	if (ErrorLineIsChild(error_line)) {
		if (error_line.Contains(EN_COMPOUT_IN_EXPANSION_OF_MACRO)) return CAR_EL_CHILD_SWAP;
		return CAR_EL_CHILD_LAST;
	}
	
	// errores "fatales" (como cuando no encuentra los archivos que tiene que compilar)
	if (error_line.Contains(EN_COMPOUT_FATAL_ERROR) || error_line.Contains(ES_COMPOUT_FATAL_ERROR)) {
		return CAR_EL_ERROR;
	}
	
	if (error_line.Contains(EN_COMPOUT_ERROR) || error_line.Contains(ES_COMPOUT_ERROR)) {
		// el "within this contex" avisa el punto dentro de una plantilla, para un error previo que se marco en la llamada a la plantilla, por eso va como hijo
		if (error_line.Contains(EN_COMPOUT_WITHIN_THIS_CONTEXT) || error_line.Contains(ES_COMPOUT_WITHIN_THIS_CONTEXT))
			return CAR_EL_CHILD_LAST;
		if (error_line.Contains(EN_COMPOUT_AT_THIS_POINT_IN_FILE) || error_line.Contains(EN_COMPOUT_AT_THIS_POINT_IN_FILE)) // estos errores aparecian en gcc 4.4, ahora cambiaron y ya no los veo
			return CAR_EL_CHILD_LAST;
		else return CAR_EL_ERROR;
	}
	
	// warnings del compilador
	if (error_line.Contains(EN_COMPOUT_WARNING) || error_line.Contains(ES_COMPOUT_WARNING)) {
		return CAR_EL_WARNING;
	}
	
	// warnings del linker
	if (error_line.Contains(EN_COMPOUT_LINKER_WARNING) || error_line.Contains(ES_COMPOUT_LINKER_WARNING)) {
		return CAR_EL_WARNING;
	}
	
//	// notas (detectado antes en ErrorLineIsChild(error_line))
//	if (error_line.Contains(EN_COMPOUT_NOTE) || error_line.Contains(ES_COMPOUT_NOTE)) {
//		return CAR_EL_CHILD_LAST;
//	}
	
	// error inside functions has a line before saying "In function 'foo()':"
	if (error_line.Last()==':') return CAR_EL_IGNORE;
	
	// anything else? probably linker error
	return CAR_EL_ERROR;
	
//	compile_and_run->parsing_errors_was_ok=false;
//	return CAR_EL_UNKNOWN;
}

void mxCompiler::ParseSomeErrors(compile_and_run_struct_single *compile_and_run) {
	if (compile_and_run->output_type==MXC_EXTERN) { ParseSomeExternErrors(compile_and_run); return; }
	
	wxProcess *process = compile_and_run->process;
	static wxString error_line, nice_error_line;
	int p;
//	char c;
//cerr<<"wxTextInputStream input(*(process->GetErrorStream()));"<<endl;
	wxTextInputStream input(*(process->GetErrorStream()));	
//cerr<<"while ( process->IsErrorAvailable() ) {"<<endl;
	while ( process->IsErrorAvailable() ) {
		
//cerr<<"error_line=input.ReadLine();"<<endl;
		error_line=input.ReadLine();
		
		// acortar el nombre de archivo
		int l=error_line.Len();
		if ( l>2 && error_line[1]==':' ) p=2; else p=0;
		while (p<l && error_line[p]!=':') p++;
		if (p<l) {
			while (p>=0 && error_line[p]!='/' && error_line[p]!='\\') p--;
			nice_error_line=error_line.Mid(p+1);
		} else 
			nice_error_line=error_line;
		
		compile_and_run->full_output.Add(error_line);
		if (compile_and_run->output_type==MXC_EXTRA || error_line.Len()==0) 
			continue;
		tree->AppendItem(compile_and_run->last_all_item,error_line,6,-1/*,new mxCompilerItemData(error_line)*/);
		
		// reemplazar templates para que sea m�s legible
		if (config->Init.beautify_compiler_errors && !current_toolchain.IsExtern()) UnSTD(nice_error_line);
		
		// averiguar si es error, warning, o parte de un error/warning anterior/siguiente
		CAR_ERROR_LINE action=ParseSomeErrorsOneLine(compile_and_run,error_line);
		
		// no mostrar taaantos errores/warnings	
		if (action==CAR_EL_ERROR) {
			num_errors++; 
			if (num_errors>config->Init.max_errors) {
				compile_and_run->process->Kill(compile_and_run->pid,wxSIGKILL); // matar el proceso, por si entra en loop infinito, podr�a colgar zinjai
				compile_and_run->last_error_item_IsOk=false;
				continue;
			}
			compile_and_run->last_error_item_IsOk=false;
		} else if (action==CAR_EL_WARNING) { 
			num_warnings++; 
			if (num_warnings>config->Init.max_errors) {
				compile_and_run->last_error_item_IsOk=false;
				continue;
			}
		} else if (action==CAR_EL_CHILD_LAST||action==CAR_EL_CHILD_SWAP) {
			if (!compile_and_run->last_error_item_IsOk && (num_warnings>config->Init.max_errors||num_errors>config->Init.max_errors)) {
				compile_and_run->process->Kill(compile_and_run->pid,wxSIGKILL); // matar el proceso, por si entra en loop infinito, podr�a colgar zinjai
				continue;
			}
		}
		
		if (action==CAR_EL_ERROR||action==CAR_EL_WARNING) { // nuevo error o warning
			if (action==CAR_EL_ERROR) { // nuevo error
				compile_and_run->last_error_item=tree->AppendItem(errors,nice_error_line,4,-1,new mxCompilerItemData(error_line));
				compile_and_run->last_error_item_IsOk=true;
			} else { // nuevo warning
				compile_and_run->last_error_item=tree->AppendItem(warnings,nice_error_line,3,-1,new mxCompilerItemData(error_line));
				compile_and_run->last_error_item_IsOk=true;
			}
			if (!compile_and_run->pending_error_lines.IsEmpty()) { // agregar los hijos que estaban pendientes
				wxTreeItemId tree_item = tree->AppendItem(compile_and_run->last_error_item,compile_and_run->pending_error_nices[0],5,-1,new mxCompilerItemData(compile_and_run->pending_error_lines[0]));
				for(unsigned int i=1;i<compile_and_run->pending_error_lines.GetCount();i++) { 
					tree->AppendItem(tree_item,compile_and_run->pending_error_nices[i],5,-1,new mxCompilerItemData(compile_and_run->pending_error_lines[i]));
				}
				compile_and_run->pending_error_nices.Clear();
				compile_and_run->pending_error_lines.Clear();
			}
		} else if (action==CAR_EL_CHILD_LAST||action==CAR_EL_CHILD_SWAP) { // continua el �ltimo error
			if (compile_and_run->last_error_item_IsOk) {
				wxTreeItemId it = tree->AppendItem(compile_and_run->last_error_item,nice_error_line,5,-1,new mxCompilerItemData(error_line));
				if (action==CAR_EL_CHILD_SWAP) {
					wxString schld=main_window->compiler_tree.treeCtrl->GetItemText(it);
					wxString sprnt=main_window->compiler_tree.treeCtrl->GetItemText(compile_and_run->last_error_item);
					main_window->compiler_tree.treeCtrl->SetItemText(it,sprnt);
					main_window->compiler_tree.treeCtrl->SetItemText(compile_and_run->last_error_item,schld);
					mxCompilerItemData &ichld=*((mxCompilerItemData*)main_window->compiler_tree.treeCtrl->GetItemData(it));
					mxCompilerItemData &iprnt=*((mxCompilerItemData*)main_window->compiler_tree.treeCtrl->GetItemData(compile_and_run->last_error_item));
					mxCompilerItemData iaux(ichld); ichld=iprnt; iprnt=iaux;
				}
			} else
				compile_and_run->parsing_errors_was_ok=false;
		} else if (action==CAR_EL_CHILD_NEXT) { // parte (hijos) del siguiente error
			compile_and_run->last_error_item_IsOk=false;
			compile_and_run->pending_error_lines.Add(error_line);
			compile_and_run->pending_error_nices.Add(nice_error_line);
		}
			
//cerr<<"while ( process->IsErrorAvailable() ) {"<<endl;
	}
//cerr<<"}"<<endl;
}

void mxCompiler::ParseSomeExternErrors(compile_and_run_struct_single *compile_and_run) {
	wxProcess *process = compile_and_run->process;
	static wxString error_line, nice_error_line;
	wxTextInputStream input1(*(process->GetInputStream()));	
	while ( process->IsInputAvailable() ) {
		error_line=input1.ReadLine();
		if (error_line.Len()==0) continue;
		compile_and_run->full_output.Add(error_line);
		main_window->extern_compiler_output->AddLine("< ",error_line);
	}
	wxTextInputStream input2(*(process->GetErrorStream()));	
	while ( process->IsErrorAvailable() ) {
		error_line=input2.ReadLine();
		if (error_line.Len()==0) continue;
		compile_and_run->full_output.Add(error_line);
		main_window->extern_compiler_output->AddLine("!! ",error_line);
	}
}

void mxCompiler::ParseCompilerOutput(compile_and_run_struct_single *compile_and_run, bool success) {
	
	// poner los errores/warnings/etc en el arbol
	ParseSomeErrors(compile_and_run); 
	
	SetWarningsAndErrorsNumbersOnTree();
	
	if (!compile_and_run_single->killed && (!compile_and_run->parsing_errors_was_ok || !compile_and_run->pending_error_lines.IsEmpty())) {
		mxMessageDialog(main_window,LANG(MAINW_COMPILER_OUTPUT_PARSING_ERROR,"ZinjaI ha intentado reacomodar la salida del compilador de forma incorrecta.\n"
			"Puede que algun error no se muestre correctamente en el arbol. Para ver la salida\n"
			"completa haga click con el boton derecho del raton en cualquier elemento del arbol\n"
			"y seleccione \"abrir ultima salida\". Para contribuir al desarrollo de ZinjaI puede\n"
			"enviar esta salida, o el codigo que la ocasiono a zaskar_84@yahoo.com.ar"
			),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
	}
	
	delete compile_and_run->process; compile_and_run->process=nullptr; // liberar el proceso
	
	// ver como sigue
	if (success) { // si el proceso termino correctamente (no hay errores/problemas)
		if (project && (compile_and_run->compiling||compile_and_run->linking)) { // si es proyecto
			
//			compile_and_run->compiling=false; // para que se pone en falso?
			
			wxString current;
			if ((project->compile_was_ok||!config->Init.stop_compiling_on_error) && project->CompileNext(compile_and_run,current)) { // si se puso a compilar algo
				main_window->StartExecutionStuff(compile_and_run,current);
			} else {
				if (compiler->IsCompiling()) { // si queda otro en paralelo
					delete compile_and_run; return;
				}
				if (project->compile_was_ok) { // si esta todo listo, informar el resultado
					if (compile_and_run->linking && !project->active_configuration->dont_generate_exe) CheckForExecutablePermision(project->GetExePath());
					main_window->SetStatusProgress(0);
					time_t elapsed_time = time(nullptr)-project->compile_startup_time;
					if (elapsed_time>5) {
						if (elapsed_time/60==0)
							main_window->SetCompilingStatus(LANG1(MAINW_COMPILING_DONE_SECONDS,"Compilacion finalizada ( tiempo transcurrido: <{1}> segundos ).",wxString()<<elapsed_time));
						if (elapsed_time/60==1)
							main_window->SetCompilingStatus(LANG1(MAINW_COMPILING_DONE_ONE_MINUTE,"Compilacion finalizada ( tiempo transcurrido: un minuto y <{1}> segundos ).",wxString()<<(elapsed_time%60)));
						else
							main_window->SetCompilingStatus(LANG2(MAINW_COMPILING_DONE_MINUTES_AND_SECONDS,"Compilacion finalizada ( tiempo transcurrido: <{1}> minutos y <{2}> segundos ).",wxString()<<(elapsed_time/60),wxString()<<(elapsed_time%60)));
					} else {
						main_window->SetCompilingStatus(LANG(MAINW_COMPILING_DONE,"Compilacion finalizada."));
					}
					if (num_warnings) {
						tree->Expand(warnings);
						main_window->ShowCompilerTreePanel();
					}
					tree->SetItemText(state,LANG(MAINW_COMPILING_DONE,"Compilacion Finalizada"));
					// ejecutar o depurar
					if (compile_and_run->on_end) compile_and_run->on_end->Do();
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
			CheckForExecutablePermision(last_compiled->GetBinaryFileName().GetFullPath());
			main_window->SetStatusProgress(0);
			main_window->SetStatusText(LANG(GENERAL_READY,"Listo"));
			tree->SetItemText(state,LANG(MAINW_COMPILING_DONE,"Compilacion Finalizada"));
			if (num_warnings) {
				tree->Expand(warnings);
				main_window->ShowCompilerTreePanel();
			}
			valgrind_cmd=compile_and_run->valgrind_cmd;
			if (compile_and_run->on_end) compile_and_run->on_end->Do();
			valgrind_cmd="";
			delete compile_and_run;
		}
	} else { // si fallo la compilacion
		// informar y no seguir
		if (!project || project->compile_was_ok) {
			main_window->SetStatusProgress(0);
			main_window->SetCompilingStatus(LANG(MAINW_COMPILATION_INTERRUPTED,"Compilacion interrumpida!"));
			compile_and_run->compiling=false;
			wxBell();
			if (compile_and_run->output_type==MXC_EXTRA) {
				tree->AppendItem(errors,wxString(LANG(MAINW_COMPILATION_CUSTOM_STEP_ERROR,"Error al ejecutar paso de compilaci�n personalizado: "))<<compile_and_run->step_label,5);
				num_errors++; SetWarningsAndErrorsNumbersOnTree();
			}
			tree->SetItemText(state,LANG(MAINW_COMPILATION_INTERRUPTED,"Compilacion interrumpida!"));
			tree->Expand(errors);
			main_window->ShowCompilerTreePanel();
			main_window->SetFocusToSourceAfterEvents();
			if (!project) code_helper->TryToSuggestTemplateSolutionForLinkingErrors(compile_and_run->full_output,compile_and_run->on_end);
		}
		if (project) {
			project->compile_was_ok=false;
			if (!config->Init.stop_compiling_on_error) {
				wxString current;
				 // mandar a compilar el que sigue
				if (project->CompileNext(compile_and_run,current)) { // si se puso a compilar algo
					main_window->StartExecutionStuff(compile_and_run,current);
					return; // avoid final delete
				} else {
					main_window->SetStatusProgress(0);
					main_window->SetStatusText(LANG(MAINW_COMPILATION_INTERRUPTED,"Compilacion interrumpida!"));
				}
			}
		}
		delete compile_and_run;
	}
}

/**
* @param on_end Si puede ponerse a compilar, le pasa esta accion al proceso de 
*               compilacion, sino la destruye, pero en cualquier caso ya no le
*               pertenece a quien invoque a esta funcion.
**/
void mxCompiler::CompileSource (mxSource *source, GenericAction *on_end) {
	RaiiDelete<GenericAction> oe_del(on_end);
	if (!EnsureCompilerNotRunning()) return;
	compile_and_run_struct_single *compile_and_run=new compile_and_run_struct_single("CompileSource");;
	fms_move(compile_and_run->on_end,on_end);
	compile_and_run->output_type=MXC_GCC;
	parser->ParseSource(source,true);
	last_compiled=source;
	wxString z_opts(wxString(" "));
	bool cpp = source->IsCppOrJustC();
	if (config->Debug.format.Len()) z_opts<<config->Debug.format<<" ";
	z_opts<<(cpp?current_toolchain.cpp_compiling_options:current_toolchain.c_compiling_options)<<" "; // forced compiler arguments
	z_opts<<(cpp?current_toolchain.cpp_linker_options:current_toolchain.c_linker_options)<<" "; // forced linker arguments
	z_opts<<current_toolchain.GetExtraCompilingArguments(cpp);
	z_opts<<" -g "; // always include debugging information
	if (!source->sin_titulo) { // avoid not recognizing files without extension
		eFileType ftype = mxUT::GetFileType(source->source_filename.GetFullName(),false);
		if (ftype!=FT_HEADER&&ftype!=FT_SOURCE) z_opts<<(source->IsCppOrJustC()?"-x c++ ":"-x c "); 
	}
	// prepare command line
	wxString comp_opts = source->GetCompilerOptions();
	wxString output_file = source->GetBinaryFileName().GetFullPath();
	if (debug->IsDebugging()) debug->GetPatcher()->AlterOutputFileName(output_file);
	wxString command = wxString(cpp?current_toolchain.cpp_compiler:current_toolchain.c_compiler)+z_opts+"\""+source->GetFullPath()+"\" "+comp_opts+_T(" -o \"")+output_file+"\"";
	
	// lanzar la ejecucion
	compile_and_run->process=new wxProcess(main_window->GetEventHandler(),mxPROCESS_COMPILE);
	compile_and_run->process->Redirect();
	ResetCompileData();
	compile_and_run->last_all_item = main_window->compiler_tree.treeCtrl->AppendItem(main_window->compiler_tree.all,command,2);
	compile_and_run->pid = mxUT::Execute(source->GetPath(false),command,wxEXEC_ASYNC,compile_and_run->process);
	compile_and_run->compiling=true;
	main_window->StartExecutionStuff(compile_and_run,LANG(MAINW_COMPILING_DOTS,"Compilando..."));
	compile_and_run->full_output.Add("");
	if (!project) compile_and_run->full_output.Add(wxString(_T("> "))+command); // lo hace el proyecto si hay
	compile_and_run->full_output.Add("");
}

// incializar el arbol de compilacion y sus auxiliares
void mxCompiler::ResetCompileData() {
	full_output.Clear();
	num_errors=num_warnings=0;
	tree->SetItemText(errors,LANG(MAINW_CT_ERRORS,"Errores"));
	tree->SetItemText(warnings,LANG(MAINW_CT_WARNINGS,"Advertencias"));
	tree->DeleteChildren(errors);
	tree->DeleteChildren(warnings);
	if (!project)  {
		tree->DeleteChildren(all);
	} else {
		project->compile_startup_time=time(nullptr);
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
	mxMutexCompiler.Lock();
	compile_and_run_struct_single *item = compile_and_run_single;
	while (item) {
		if (item->compiling && item->pid!=0) n++;
		item=item->next;
	}
	mxMutexCompiler.Unlock();
	return n;
}
bool mxCompiler::IsCompiling() {
	mxMutexCompiler.Lock();
	compile_and_run_struct_single *item = compile_and_run_single;
	while (item) {
		if (item->compiling && item->pid!=0) {
			mxMutexCompiler.Unlock();
			return true;
		}
		item=item->next;
	}
	mxMutexCompiler.Unlock();
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
		system((string("chmod a+x ")+mxUT::Quotize(file).c_str()).c_str());
		if (!wxFileName::IsFileExecutable(file)) {
			tree->AppendItem(errors,LANG(MAINW_WARNING_NO_EXCUTABLE_PERMISSION,"El binario no tiene permisos de ejecuci�n."),7);
			main_window->ShowCompilerTreePanel();
			tree->Expand(errors);
			return false;
		} else {
			tree->AppendItem(warnings,LANG(MAINW_WARNING_EXCUTABLE_PERMISSION_CHANGED,"Se cambiaron los permisos del binario (chmod a+x)."),7);
			tree->Expand(warnings);
		}
	}
#endif
	return true;
}

void mxCompiler::SetWarningsAndErrorsNumbersOnTree() {
	wxString serrors=LANG(MAINW_CT_ERRORS,"Errores"); serrors<<" (";
	if (num_errors<=config->Init.max_errors) serrors<<num_errors;
	else serrors<<config->Init.max_errors<<"+";
	serrors<<")"; tree->SetItemText(errors,serrors);

	wxString swarnings=LANG(MAINW_CT_WARNINGS,"Advertencias"); swarnings<<" (";
	if (num_warnings<=config->Init.max_errors) swarnings<<num_warnings;
	else swarnings<<config->Init.max_errors<<"+";
	swarnings<<")"; tree->SetItemText(warnings,swarnings);
}
