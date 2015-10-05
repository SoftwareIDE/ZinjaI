#include "Toolchain.h"
#include "mxUtils.h"
#include <wx/arrstr.h>
#include <wx/textfile.h>
#include "ConfigManager.h"
#include "ProjectManager.h"
#include <wx/msgdlg.h>
#include <iostream>
using namespace std;

Toolchain *Toolchain::toolchains=nullptr;
int Toolchain::toolchains_count=0;
Toolchain current_toolchain;

void Toolchain::LoadToolchains ( ) {
	if (toolchains) delete [] toolchains;
	wxArrayString toolchain_files;
	mxUT::GetFilesFromBothDirs(toolchain_files,"toolchains");
	toolchains_count=toolchain_files.GetCount();
	toolchains=new Toolchain[toolchains_count];
	for (int i=0; i<toolchains_count;i++) {
		toolchains[i].file = toolchains[i].file = toolchain_files[i];
		wxString filename = mxUT::WichOne(toolchain_files[i],"toolchains",true);
		wxTextFile file(filename); file.Open();
		if (file.IsOpened()) {
			wxString key, value;
			for (wxString line = file.GetFirstLine();!file.Eof();line = file.GetNextLine()) {
				key=line.BeforeFirst('=');
				value=line.AfterFirst('=');
				if (key=="base_path") toolchains[i].base_path = value;
//				else if (key=="desc") toolchains[i].desc = value;
				else if (key=="bin_path") toolchains[i].bin_path = value;
				else if (key=="build_command") toolchains[i].build_command = value;
				else if (key=="clean_command") toolchains[i].clean_command = value;
				else if (key.StartsWith("argument")) { 
					long l; 
					if (key.Mid(8).ToLong(&l) && l>0 && l<=TOOLCHAIN_MAX_ARGS) {
						toolchains[i].arguments[l-1][0]=value.BeforeFirst('=');
						toolchains[i].arguments[l-1][1]=value.AfterFirst('=');
					}
				}
				else if (key=="type") {
					if (value=="gcc") toolchains[i].type = TC_GCC;
					else if (value=="gcc-like") toolchains[i].type = TC_GCC_LIKE;
					else if (value=="clang") toolchains[i].type = TC_CLANG;
					else if (value=="extern") toolchains[i].type = TC_EXTERN;
					else toolchains[i].type = TC_COUNT;
				}
				else if (key=="c_compiler") toolchains[i].c_compiler = value;
				else if (key=="c_compiling_options") toolchains[i].c_compiling_options = value;
				else if (key=="cpp_compiler") toolchains[i].cpp_compiler = value;
				else if (key=="cpp_compiling_options") toolchains[i].cpp_compiling_options = value;
				else if (key=="linker") toolchains[i].linker = value;
				else if (key=="c_linker_options") toolchains[i].c_linker_options = value;
				else if (key=="cpp_linker_options") toolchains[i].cpp_linker_options = value;
				else if (key=="dynamic_lib_linker") toolchains[i].dynamic_lib_linker = value;
				else if (key=="static_lib_linker") toolchains[i].static_lib_linker = value;
			}
		}
	}
}

void Toolchain::Save(const wxString &fname) {
	wxTextFile fil(fname);
	if (fil.Exists())
		fil.Open();
	else
		fil.Create();
	fil.Clear();
	
	switch(type) {
	case TC_GCC: fil.AddLine("type=gcc"); break;
	case TC_GCC_LIKE: fil.AddLine("type=gcc-like"); break;
	case TC_CLANG: fil.AddLine("type=clang"); break;
	case TC_EXTERN: fil.AddLine("type=extern"); break;
	case TC_COUNT:;
	}
	
	fil.AddLine(wxString("base_path=")+base_path);
	fil.AddLine(wxString("bin_path=")+bin_path);
	if (type==TC_EXTERN) {
		fil.AddLine(wxString("build_command=")+build_command);
		fil.AddLine(wxString("clean_command=")+clean_command);
	} else {
		fil.AddLine(wxString("c_compiler=")+c_compiler);
		fil.AddLine(wxString("cpp_compiler=")+cpp_compiler);
		fil.AddLine(wxString("linker=")+linker);
		fil.AddLine(wxString("c_compiling_options=")+c_compiling_options);
		fil.AddLine(wxString("cpp_compiling_options=")+cpp_compiling_options);
		fil.AddLine(wxString("c_linker_options=")+c_linker_options);
		fil.AddLine(wxString("cpp_linker_options=")+cpp_linker_options);
		fil.AddLine(wxString("dynamic_lib_linker=")+dynamic_lib_linker);
		fil.AddLine(wxString("static_lib_linker=")+static_lib_linker);
	}
	for(int unsigned i=0;i<TOOLCHAIN_MAX_ARGS;i++) {  
		if (arguments[i][0].Len())
			fil.AddLine(wxString("argument")<<(i+1)<<"="<<arguments[i][0]<<"="<<arguments[i][1]);
	}
	
	fil.Write();
	fil.Close();
	
}

Toolchain::Toolchain () {
	version_c=version_cpp=-1;
	file="<null>";
//	desc="<null>";
	type=TC_GCC;
#ifdef __WIN32__
	linker="mingw32-g++";
	cpp_compiler="mingw32-g++";
	c_compiler="mingw32-gcc";
	c_linker_options="-static-libgcc";
	cpp_linker_options="-static-libgcc -static-libstdc++";
	base_path="MinGW";
	bin_path="MinGW\\bin";
#else
	linker="g++";
	cpp_compiler="g++";
	c_compiler="gcc";
#endif
	c_compiling_options="--show-column";
	cpp_compiling_options="--show-column";
	dynamic_lib_linker=cpp_compiler+" -shared -o";
	static_lib_linker="ar cr";
}

Toolchain &Toolchain::SelectToolchain ( ) {
	wxString fname=config->Files.toolchain;
	if (project && project->active_configuration->toolchain.Len())
		fname=project->active_configuration->toolchain;
	for(int i=0;i<toolchains_count;i++) { 
		if (toolchains[i].file==fname) {
			current_toolchain=toolchains[i];
			current_toolchain.SetArgumets();
			current_toolchain.SetPaths();
			if (toolchains[i].CheckVersion(false,0,0)) current_toolchain.version_c=toolchains[i].version_c;
			if (toolchains[i].CheckVersion(true,0,0)) current_toolchain.version_cpp=toolchains[i].version_cpp;
			return current_toolchain;
		}
	}
	current_toolchain=Toolchain();
	current_toolchain.SetArgumets();
	current_toolchain.SetPaths();
	return current_toolchain;
}

void Toolchain::SetPaths() {
	static wxString old_compiler_bin_path; 
	if (old_compiler_bin_path!=current_toolchain.bin_path) {
		old_compiler_bin_path = current_toolchain.bin_path;
		static wxString original_path;
		if (!original_path.Len()) wxGetEnv("PATH",&original_path);
		if (current_toolchain.bin_path.Len()) {
			wxString new_path=original_path;
			wxArrayString array; mxUT::Split(current_toolchain.bin_path,array,true,false);
			for(unsigned int i=0;i<array.GetCount();i++) 
				new_path=wxFileName(DIR_PLUS_FILE(config->zinjai_dir,array[i])).GetShortPath()+";"+new_path;
			wxSetEnv("PATH",new_path);
		} else {
			wxSetEnv("PATH",original_path);
		}
	}
	mingw_dir=DIR_PLUS_FILE(config->zinjai_dir,base_path);
	if (mingw_dir.EndsWith("\\")||mingw_dir.EndsWith("/")) mingw_dir.RemoveLast();
}

const Toolchain &Toolchain::GetInfo (const wxString &fname) {
	for(int i=0;i<toolchains_count;i++) { 
		if (toolchains[i].file==fname) {
			return toolchains[i];
		}
	}
	if (fname!=config->Files.toolchain) // if not found, returns default one
		return GetInfo(config->Files.toolchain);
#ifdef _ZINJAI_DEBUG
		wxMessageBox(wxString("invalid toolchain fname: ")<<fname);
#endif
	return current_toolchain; // this should never reach this point
}

void Toolchain::GetNames (wxArrayString & names, bool exclude_extern) {
	for(int i=0;i<toolchains_count;i++) { 
		if (!exclude_extern || !toolchains[i].IsExtern())
			names.Add(toolchains[i].file);
	}
}

static void aux1_SetProjectArguments(wxString &_tool, wxString _project) {
	_project.Replace("${DEFAULT}",_tool);
	_tool=_project;
}

static void aux2_SetProjectArguments(wxString &command, const wxString array[][2]) {
	for(int i=0;i<TOOLCHAIN_MAX_ARGS;i++) command.Replace(wxString("${ARG")<<i+1<<"}",array[i][1],true);
}

void Toolchain::SetArgumets ( ) {
	if (project) {
		for(int i=0;i<TOOLCHAIN_MAX_ARGS;i++) // replace arguments with values from project
			aux1_SetProjectArguments(arguments[i][1],project->active_configuration->toolchain_arguments[i]);
	}
	// if its extern, ProjectManager will apply arguments, but if its not we must do it now so they became transparent for clients
	if (!IsExtern()) {
		aux2_SetProjectArguments(c_compiler,arguments);
		aux2_SetProjectArguments(cpp_compiler,arguments);
		aux2_SetProjectArguments(linker,arguments);
		aux2_SetProjectArguments(c_compiling_options,arguments);
		aux2_SetProjectArguments(cpp_compiling_options,arguments);
		aux2_SetProjectArguments(c_linker_options,arguments);
		aux2_SetProjectArguments(cpp_linker_options,arguments);
		aux2_SetProjectArguments(dynamic_lib_linker,arguments);
		aux2_SetProjectArguments(static_lib_linker,arguments);
	}
}

wxString Toolchain::GetExtraCompilingArguments (bool cpp) {
	if (type==TC_GCC) {
		if (CheckVersion(cpp,4,8)) return "-fshow-column -fno-diagnostics-show-caret";
		else return "-fshow-column";
	} else if (type==TC_CLANG) {
		return "-fno-caret-diagnostics";
	} else 
		return "";
}

bool Toolchain::CheckVersion(bool cpp, int _v, int _s) {
	if (cpp) {
		if (version_cpp<0) {
			wxString str=mxUT::GetOutput(cpp_compiler+" -dumpversion");
			if (str.Len()==0) return false;
			long v,s;
			str.BeforeFirst('.').ToLong(&v);
			str.AfterFirst('.').BeforeFirst('.').ToLong(&s);
			version_cpp=v*1000+s;
			DEBUG_INFO("Toolchain::CheckVersion cpp: "<<version_cpp);
		}
		return version_cpp>=_v*1000+_s;
	} else {
		if (version_c<0) {
			wxString str=mxUT::GetOutput(c_compiler+" -dumpversion");
			if (str.Len()==0) return false;
			long v,s;
			str.BeforeFirst('.').ToLong(&v);
			str.AfterFirst('.').BeforeFirst('.').ToLong(&s);
			version_c=v*1000+s;
			DEBUG_INFO("Toolchain::CheckVersion c: "<<version_c);
		}
		return version_c>=_v*1000+_s;
	}
}

wxString Toolchain::FixArgument (bool cpp, wxString arg) {
	if (type!=TC_GCC) return arg;
	if (arg=="-Og" && !CheckVersion(cpp,4,8)) return "-O0";
	if (arg=="-std=c++11" && !CheckVersion(cpp,4,7)) return "-std=c++0x";
	if (arg=="-std=gnu++11" && !CheckVersion(cpp,4,7)) return "-std=gnu++0x";
	return arg;
}

Toolchain *Toolchain::GetToolchain (const wxString & name) {
	for(int i=0;i<toolchains_count;i++) 
		if (toolchains[i].file==name) return toolchains+i;
	return nullptr;
}

