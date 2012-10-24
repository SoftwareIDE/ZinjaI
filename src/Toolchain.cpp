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
		toolchains[i].name = toolchain_files[i];
		wxString filename = utils->WichOne(toolchain_files[i],DIR_PLUS_FILE(config->home_dir,"toolchains"),DIR_PLUS_FILE(config->zinjai_dir,"toolchains"),true);
		wxTextFile file(filename); file.Open();
		if (file.IsOpened()) {
			wxString key, value, base_dir;
			for (wxString line = file.GetFirstLine();!file.Eof();line = file.GetNextLine()) {
				key=line.BeforeFirst('=');
				value=line.AfterFirst('=');
				if (key=="name") toolchains[i].name = value;
				else if (key=="base_dir") base_dir = value;
				else if (key=="extern") toolchains[i].is_extern = utils->IsTrue(value);
				else if (key=="c_compiler") toolchains[i].c_compiler = value;
				else if (key=="c_compiling_options") toolchains[i].c_compiling_options = value;
				else if (key=="cpp_compiler") toolchains[i].cpp_compiler = value;
				else if (key=="cpp_compiling_options") toolchains[i].cpp_compiling_options = value;
				else if (key=="linker") toolchains[i].linker = value;
				else if (key=="linker_options") toolchains[i].linker_options = value;
				
			}
		}
	}
	SelectToolchain();
}

Toolchain::Toolchain () {
	is_extern=false;
#if defined(__WIN32__)
	linker=_T("mingw32-g++");
	cpp_compiler=_T("mingw32-g++");
	c_compiler=_T("mingw32-gcc");
	linker_options=_T("-static-libgcc -static-libstdc++");
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

bool Toolchain::SelectToolchain ( ) {
	wxString name=project?project->active_configuration->toolchain:config->Files.toolchain;
	for(int i=0;i<toolchains_count;i++) { 
		if (toolchains[i].name==name) {
			current_toolchain=toolchains[i];
			return true;
		}
	}
	return false;
}

void Toolchain::GetNames (wxArrayString & names, bool exclude_extern) {
	for(int i=0;i<toolchains_count;i++) { 
		if (!exclude_extern || !toolchains[i].is_extern)
			names.Add(toolchains[i].name);
	}
}

