#include "mxToolchainConfig.h"
#include "Toolchain.h"
#include "ids.h"
#include "Language.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "mxHelpWindow.h"
#include <wx/notebook.h>
#include "mxMessageDialog.h"
#include "ConfigManager.h"
#include "mxCommonConfigControls.h"
#include "mxThreeDotsUtils.h"

BEGIN_EVENT_TABLE(mxToolchainConfig, wxDialog)
	EVT_BUTTON(wxID_OK,mxToolchainConfig::OnButtonOk)
//	EVT_BUTTON(mxID_CUSTOM_TOOLS_RUN,mxToolchainConfig::OnButtonTest)
	EVT_BUTTON(wxID_CANCEL,mxToolchainConfig::OnButtonCancel)
	EVT_BUTTON(mxID_HELP_BUTTON,mxToolchainConfig::OnButtonHelp)
	EVT_BUTTON(mxID_TOOLCHAINS_BIN_PATHS,mxToolchainConfig::OnButtonCompilerBinPaths)
	EVT_BUTTON(mxID_TOOLCHAINS_BASE_PATH,mxToolchainConfig::OnButtonCompilerBasePath)
	EVT_COMBOBOX(mxID_TOOLCHAINS_TYPE_COMBO,mxToolchainConfig::OnComboChange)
END_EVENT_TABLE()

mxToolchainConfig::mxToolchainConfig(wxWindow *parent, const wxString &tc_name) 
	: mxDialog(parent, LANG(TOOLCHAINS_CAPTION,"Personalizar herramientas de compilación") ),
	  m_toolchain(Toolchain::GetToolchain(tc_name))
{
	CreateSizer(this)
		.BeginNotebook()
			.AddPage(this,&mxToolchainConfig::CreatePanelGeneral, LANG(TOOLCHAINS_GENERAL_SETTINGS,"General"))
			.AddPage(this,&mxToolchainConfig::CreatePanelFixed, LANG(TOOLCHAINS_FIXED_SETTINGS,"Configuración fija"))
			.AddPage(this,&mxToolchainConfig::CreatePanelArgs, LANG(TOOLCHAINS_USER_ARGUMENTS,"Argumentos configurables"))
		.EndNotebook()
		.BeginBottom().Help().Cancel().Ok().EndBottom(this)
		.SetAndFit();
	wxCommandEvent evt;
	OnComboChange(evt);
}

void mxToolchainConfig::OnButtonOk (wxCommandEvent & event) {
	Toolchain tc;
	tc.file = name->GetValue();
	if (!tc.file.Len()) { mxMessageDialog(LANG(TOOLCHAINS_ERROR_EMPTY_NAME,"Debe completar el nombre."),LANG(TOOLCHAINS_CAPTION,"Personalizar herramientas de compilación"),mxMD_ERROR|mxMD_OK).ShowModal(); return; }
//	tc.desc = description->GetValue();
	tc.base_path = base_path->GetValue();
	tc.bin_path = bin_path->GetValue();
	tc.build_command = build_command->GetValue();
	tc.clean_command = clean_command->GetValue();
	for(int i=0;i<TOOLCHAIN_MAX_ARGS;i++) { 
		tc.arguments[i][0] = arguments[i][0]->GetValue();
		tc.arguments[i][1] = arguments[i][1]->GetValue();
	}
	tc.type = (TC_TYPE) type->GetSelection();
	tc.c_compiler = c_compiler->GetValue();
	tc.c_compiling_options = c_compiling_options->GetValue();
	tc.cpp_compiler = cpp_compiler->GetValue();
	tc.cpp_compiling_options = cpp_compiling_options->GetValue();
	tc.linker = linker->GetValue();
	tc.c_linker_options = c_linker_options->GetValue();
	tc.cpp_linker_options = cpp_linker_options->GetValue();
	tc.dynamic_lib_linker = dynamic_lib_linker->GetValue();
	tc.static_lib_linker = static_lib_linker->GetValue();
	wxString dirname = DIR_PLUS_FILE(config->config_dir,"toolchains");
	if (!wxFileName::DirExists(dirname)) wxMkdir(dirname);
	tc.Save(DIR_PLUS_FILE(dirname,tc.file));
	Toolchain::LoadToolchains();
	Toolchain::SelectToolchain();
	Close();
}

void mxToolchainConfig::OnButtonCancel (wxCommandEvent & event) {
	Close();
}

void mxToolchainConfig::OnButtonHelp (wxCommandEvent & event) {
	mxHelpWindow::ShowHelp("toolchains.html",this);
}

void mxToolchainConfig::OnComboChange (wxCommandEvent & event) {
	bool is_extern=type->GetSelection()==TC_EXTERN;
	for_extern.EnableAll(is_extern);
	for_gcc.EnableAll(!is_extern);
}

wxPanel * mxToolchainConfig::CreatePanelGeneral(wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	wxArrayString array; for(int i=0;i<TC_COUNT;i++) array.Add("");
	array[TC_CLANG] = "clang"; array[TC_EXTERN] = "extern"; array[TC_GCC] = "gcc"; array[TC_GCC_LIKE] = "gcc-like";
	//	description = AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_DESCRIPTION,"Descripción"),tc->desc);
	sizer.BeginText( LANG(TOOLCHAINS_NAME,"Nombre de archivo") )
		.Value(m_toolchain->file).Short().EndText(name);
	sizer.BeginCombo( LANG(TOOLCHAINS_TYPE,"Tipo de herramienta") )
		.Add(array).Select(m_toolchain->type).Id(mxID_TOOLCHAINS_TYPE_COMBO).EndCombo(type);
	sizer.BeginText( LANG(TOOLCHAINS_BASE_PATH,"Directorio base del compilador") )
		.Value(m_toolchain->base_path).Short().Button(mxID_TOOLCHAINS_BASE_PATH).EndText(base_path);
	sizer.BeginText( LANG(TOOLCHAINS_BIN_PATH,"Directorios con ejecutables") )
		.Value(m_toolchain->bin_path).Short().Button(mxID_TOOLCHAINS_BIN_PATHS).EndText(bin_path);
	sizer.BeginText( LANG(TOOLCHAINS_BUILD_COMMAND,"Comando de construcción") )
		.Value(m_toolchain->build_command).Short().RegisterIn(for_extern).EndText(build_command);
	sizer.BeginText( LANG(TOOLCHAINS_CLEAN_COMMAND,"Comando de limpieza") )
		.Value(m_toolchain->clean_command).Short().RegisterIn(for_extern).EndText(clean_command);
	
	sizer.Set();
	return sizer.GetPanel();
}

wxPanel * mxToolchainConfig::CreatePanelFixed (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	sizer.BeginText( LANG(TOOLCHAINS_C_COMMAND,"Comando del compilador C") )
		.Value(m_toolchain->c_compiler).Short().RegisterIn(for_gcc).EndText(c_compiler);
	sizer.BeginText( LANG(TOOLCHAINS_C_OPTIONS,"Argumentos para la compilación C") )
		.Value(m_toolchain->c_compiling_options).Short().RegisterIn(for_gcc).EndText(c_compiling_options);
	sizer.BeginText( LANG(TOOLCHAINS_CPP_COMMAND,"Comando del compilador C++") )
		.Value(m_toolchain->cpp_compiler).Short().RegisterIn(for_gcc).EndText(cpp_compiler);
	sizer.BeginText( LANG(TOOLCHAINS_CPP_OPTIONS,"Argumentos para la compilación C++") )
		.Value(m_toolchain->cpp_compiling_options).Short().RegisterIn(for_gcc).EndText(cpp_compiling_options);
	sizer.BeginText( LANG(TOOLCHAINS_LINKER_COMMAND,"Comando del enlazador") )
		.Value(m_toolchain->linker).Short().RegisterIn(for_gcc).EndText(linker);
	sizer.BeginText( LANG(TOOLCHAINS_LINKER_C_OPTIONS,"Argumentos para el enlazado C") )
		.Value(m_toolchain->c_linker_options).Short().RegisterIn(for_gcc).EndText(c_linker_options);
	sizer.BeginText( LANG(TOOLCHAINS_LINKER_CPP_OPTIONS,"Argumentos para el enlazado C++") )
		.Value(m_toolchain->cpp_linker_options).Short().RegisterIn(for_gcc).EndText(cpp_linker_options);
	sizer.BeginText( LANG(TOOLCHAINS_DYNAMIC_LIB_LINKER,"Comando para enlazar libs. dinámicas") )
		.Value(m_toolchain->dynamic_lib_linker).Short().RegisterIn(for_gcc).EndText(dynamic_lib_linker);
	sizer.BeginText( LANG(TOOLCHAINS_STATIC_LIB_LINKER,"Comando para enlazar libs. estáticas") )
		.Value(m_toolchain->static_lib_linker).Short().RegisterIn(for_gcc).EndText(static_lib_linker);
	
	sizer.Set();
	return sizer.GetPanel();
}

wxPanel * mxToolchainConfig::CreatePanelArgs (wxNotebook * notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	for(int i=0;i<TOOLCHAIN_MAX_ARGS;i++) {
		sizer.BeginText( LANG1(TOOLCHAINS_ARGUMENT_NAME,"Nombre (${ARG<{1}>})",wxString()<<i+1) )
			.Short().Value(m_toolchain->arguments[i][0]).EndText(arguments[i][0]);
		sizer.BeginText( LANG1(TOOLCHAINS_ARGUMENT_DEFAULT,"   Valor por defecto (${ARG<{1}>})",wxString()<<i+1) )
			.Short().Value(m_toolchain->arguments[i][1]).EndText(arguments[i][1]);
	}

	sizer.Set();
	return sizer.GetPanel();
}

void mxToolchainConfig::OnButtonCompilerBinPaths (wxCommandEvent & event) {
	mxThreeDotsUtils::ReplaceSelectionWithDirectory(this,bin_path,config->zinjai_dir,LANG(TOOLCHAINS_BIN_PATH,"Directorios con ejecutables"));
}

void mxToolchainConfig::OnButtonCompilerBasePath (wxCommandEvent & event) {
	mxThreeDotsUtils::ReplaceAllWithDirectory(this,base_path,config->zinjai_dir,LANG(TOOLCHAINS_BASE_PATH,"Directorio base del compilador"));
}

