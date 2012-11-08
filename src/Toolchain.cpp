#include "Toolchain.h"
#include "mxUtils.h"
#include <wx/arrstr.h>
#include <wx/textfile.h>
#include "ConfigManager.h"
#include "ProjectManager.h"

Toolchain *Toolchain::toolchains=NULL;
int Toolchain::toolchains_count=0;
Toolchain current_toolchain;

void Toolchain::LoadToolchains ( ) {
	if (toolchains) delete [] toolchains;
	wxArrayString toolchain_files;
	utils->GetFilesFromDir(toolchain_files,DIR_PLUS_FILE(config->zinjai_dir,"toolchains"),true);
	utils->GetFilesFromDir(toolchain_files,DIR_PLUS_FILE(config->home_dir,"toolchains"),true);
	utils->Unique(toolchain_files,true);
	toolchains_count=toolchain_files.GetCount();
	toolchains=new Toolchain[toolchains_count];
	for (int i=0; i<toolchains_count;i++) {
		toolchains[i].file = toolchains[i].file = toolchain_files[i];
		wxString filename = utils->WichOne(toolchain_files[i],DIR_PLUS_FILE(config->home_dir,"toolchains"),DIR_PLUS_FILE(config->zinjai_dir,"toolchains"),true);
		wxTextFile file(filename); file.Open();
		if (file.IsOpened()) {
			wxString key, value, base_dir;
			for (wxString line = file.GetFirstLine();!file.Eof();line = file.GetNextLine()) {
				key=line.BeforeFirst('=');
				value=line.AfterFirst('=');
				if (key=="desc") toolchains[i].desc = value;
				else if (key=="base_dir") base_dir = value;
				else if (key=="build_command") toolchains[i].build_command = value;
				else if (key=="clean_command") toolchains[i].clean_command = value;
				else if (key.StartsWith("argument")) { 
					long l; 
					if (key.Mid(8).ToLong(&l) && l>0 && l<=TOOLCHAIN_MAX_ARGS) {
						toolchains[i].arguments[l-1][0]=value.BeforeFirst('=');
						toolchains[i].arguments[l-1][1]=value.AfterFirst('=');
					}
				}
				else if (key=="extern") toolchains[i].is_extern = utils->IsTrue(value);
				else if (key=="c_compiler") toolchains[i].c_compiler = value;
				else if (key=="c_compiling_options") toolchains[i].c_compiling_options = value;
				else if (key=="cpp_compiler") toolchains[i].cpp_compiler = value;
				else if (key=="cpp_compiling_options") toolchains[i].cpp_compiling_options = value;
				else if (key=="linker") toolchains[i].linker = value;
				else if (key=="c_linker_options") toolchains[i].c_linker_options = value;
				else if (key=="cpp_linker_options") toolchains[i].cpp_linker_options = value;
				
			}
		}
	}
	SelectToolchain();
}

Toolchain::Toolchain () {
	file="<null>";
	desc="<null>";
	is_extern=false;
#if defined(__WIN32__)
	linker=_T("mingw32-g++");
	cpp_compiler=_T("mingw32-g++");
	c_compiler=_T("mingw32-gcc");
	c_linker_options=_T("-static-libgcc");
	cpp_linker_options=_T("-static-libgcc -static-libstdc++");
#else
	linker=_T("g++");
	cpp_compiler=_T("g++");
	c_compiler=_T("gcc");
#endif
	c_compiling_options=_T("--show-column");
	cpp_compiling_options=_T("--show-column");
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
			return current_toolchain;
		}
	}
	current_toolchain=Toolchain();
	return current_toolchain;
}

void Toolchain::GetNames (wxArrayString & names, bool exclude_extern) {
	for(int i=0;i<toolchains_count;i++) { 
		if (!exclude_extern || !toolchains[i].is_extern)
			names.Add(toolchains[i].file);
	}
}

