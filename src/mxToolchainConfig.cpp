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

BEGIN_EVENT_TABLE(mxToolchainConfig, wxDialog)
	EVT_BUTTON(wxID_OK,mxToolchainConfig::OnButtonOk)
//	EVT_BUTTON(mxID_CUSTOM_TOOLS_RUN,mxToolchainConfig::OnButtonTest)
	EVT_BUTTON(wxID_CANCEL,mxToolchainConfig::OnButtonCancel)
	EVT_BUTTON(mxID_HELP_BUTTON,mxToolchainConfig::OnButtonHelp)
	EVT_COMBOBOX(mxID_TOOLCHAINS_TYPE_COMBO,mxToolchainConfig::OnComboChange)
	EVT_CLOSE(mxToolchainConfig::OnClose)
END_EVENT_TABLE()

mxToolchainConfig::mxToolchainConfig(wxWindow *parent, const wxString &tc_name) :wxDialog(parent,wxID_ANY,LANG(TOOLCHAINS_CAPTION,"Personalizar herramientas de compilación"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
	
	Toolchain *tc = Toolchain::GetToolchain(tc_name);
	wxNotebook *notebook = new wxNotebook(this,wxID_ANY);
	
	wxPanel *panel_1 = new wxPanel(notebook, wxID_ANY );
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxVERTICAL);
	wxArrayString array; for(int i=0;i<TC_COUNT;i++) array.Add("");
	array[TC_CLANG] = "clang";
	array[TC_EXTERN] = "extern";
	array[TC_GCC] = "gcc";
	array[TC_GCC_LIKE] = "gcc-like";
//	description = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_DESCRIPTION,"Descripción"),tc->desc);
	name = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_NAME,"Nombre de archivo"),tc->file);
	type = utils->AddComboBox(sizer_1,panel_1,LANG(TOOLCHAINS_TYPE,"Tipo de herramienta"),array,tc->type,mxID_TOOLCHAINS_TYPE_COMBO);
	base_path = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_BASE_PATH,"Directorio del compilador"),tc->base_path);
	bin_path = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_BIN_PATH,"Directorios con ejecutables"),tc->bin_path);
	build_command = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_BUILD_COMMAND,"Comando de construcción"),tc->build_command);
	for_extern.Add(build_command,true);
	clean_command = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_CLEAN_COMMAND,"Comando de limpieza"),tc->clean_command);
	for_extern.Add(clean_command,true);
	c_compiler = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_C_COMMAND,"Comando del compilador C"),tc->c_compiler);
	for_gcc.Add(c_compiler,true);
	c_compiling_options = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_C_OPTIONS,"Argumentos para la compilación C"),tc->c_compiling_options);
	for_gcc.Add(c_compiling_options,true);
	cpp_compiler = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_CPP_COMMAND,"Comando del compilador C++"),tc->cpp_compiler);
	for_gcc.Add(cpp_compiler,true);
	cpp_compiling_options = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_CPP_OPTIONS,"Argumentos para la compilación C++"),tc->cpp_compiling_options);
	for_gcc.Add(cpp_compiling_options,true);
	linker = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_LINKER_COMMAND,"Comando del enlazador"),tc->linker);
	for_gcc.Add(linker,true);
	c_linker_options = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_LINKER_C_OPTIONS,"Argumentos para el enlazado C"),tc->c_linker_options);
	for_gcc.Add(c_linker_options,true);
	cpp_linker_options = utils->AddShortTextCtrl(sizer_1,panel_1,LANG(TOOLCHAINS_LINKER_CPP_OPTIONS,"Argumentos para el enlazado C++"),tc->cpp_linker_options);
	for_gcc.Add(cpp_linker_options,true);
	panel_1->SetSizer(sizer_1);
	
	wxPanel *panel_2 = new wxPanel(notebook, wxID_ANY );
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxVERTICAL);
	for(int i=0;i<TOOLCHAIN_MAX_ARGS;i++) { 
		arguments[i][0] = utils->AddShortTextCtrl(sizer_2,panel_2,LANG1(TOOLCHAINS_ARGUMENT_NAME,"Nombre (${ARG<{1}>})",wxString()<<i+1),tc->arguments[i][0]);
		arguments[i][1] = utils->AddShortTextCtrl(sizer_2,panel_2,LANG1(TOOLCHAINS_ARGUMENT_DEFAULT,"   Valor por defecto (${ARG<{1}>})",wxString()<<i+1),tc->arguments[i][1]);
	}
	panel_2->SetSizer(sizer_2);
	
	notebook->AddPage(panel_1, LANG(TOOLCHAINS_FIXED_SETTINGS,"Configuración fija"));
	notebook->AddPage(panel_2, LANG(TOOLCHAINS_USER_ARGUMENTS,"Argumentos configurables"));
	
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *cancel_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	SetEscapeId(wxID_CANCEL);
	wxButton *ok_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
//	wxButton *run_button = new mxBitmapButton (this,mxID_CUSTOM_TOOLS_RUN,bitmaps->buttons.ok,LANG(TOOLCHAINS_TEST,"&Ejecutar"));
	ok_button->SetDefault(); 
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	
	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
//	bottomSizer->Add(run_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(notebook,sizers->Exp1);
	sizer->Add(bottomSizer,sizers->Exp0);
	SetSizerAndFit(sizer);
	
	wxCommandEvent evt;
	OnComboChange(evt);
	
	ShowModal();
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
	wxString dirname = DIR_PLUS_FILE(config->home_dir,"toolchains");
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
	mxHelpWindow::ShowHelp("toolchains.html");
}

void mxToolchainConfig::OnClose (wxCloseEvent & event) {
	Destroy();
}

void mxToolchainConfig::OnComboChange (wxCommandEvent & event) {
	bool is_extern=type->GetSelection()==TC_EXTERN;
	for_extern.EnableAll(is_extern);
	for_gcc.EnableAll(!is_extern);
}

