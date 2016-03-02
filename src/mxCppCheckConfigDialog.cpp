#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
#include <wx/notebook.h>
#include "mxCppCheckConfigDialog.h"
#include "ids.h"
#include "mxUtils.h"
#include "mxSizers.h"
#include "mxArt.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "ProjectManager.h"
#include "mxEnumerationEditor.h"
#include "mxMultipleChoiceEditor.h"
#include "mxHelpWindow.h"
#include "mxCommonConfigControls.h"

BEGIN_EVENT_TABLE(mxCppCheckConfigDialog,wxDialog)
	EVT_CHECKBOX(mxID_CPPCHECK_COPYCONFIG,mxCppCheckConfigDialog::OnCheckCopyConfig)
	EVT_BUTTON(mxID_CPPCHECK_CONFIG_D,mxCppCheckConfigDialog::OnButtonConfigD)
	EVT_BUTTON(mxID_CPPCHECK_CONFIG_U,mxCppCheckConfigDialog::OnButtonConfigU)
	EVT_BUTTON(mxID_CPPCHECK_STYLE,mxCppCheckConfigDialog::OnButtonStyle)
	EVT_BUTTON(mxID_CPPCHECK_PLATFORM,mxCppCheckConfigDialog::OnButtonPlatform)
	EVT_BUTTON(mxID_CPPCHECK_STANDARD,mxCppCheckConfigDialog::OnButtonStandard)
	EVT_BUTTON(mxID_CPPCHECK_SUPPRESS_FILE,mxCppCheckConfigDialog::OnButtonSuppressFile)
	EVT_BUTTON(mxID_CPPCHECK_SUPPRESS_IDS,mxCppCheckConfigDialog::OnButtonSuppressIds)
	EVT_BUTTON(mxID_CPPCHECK_INCLUDE_FILE,mxCppCheckConfigDialog::OnButtonIncludeFile)
	EVT_BUTTON(mxID_CPPCHECK_EXCLUDE_FILE,mxCppCheckConfigDialog::OnButtonExcludeFile)
	EVT_BUTTON(mxID_CPPCHECK_ADDITIONAL_FILES,mxCppCheckConfigDialog::OnButtonAdditionalFiles)
	EVT_CHECKBOX(mxID_CPPCHECK_EXCLUDE_HEADERS,mxCppCheckConfigDialog::OnButtonExcludeHeaders)
	EVT_BUTTON(wxID_OK,mxCppCheckConfigDialog::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxCppCheckConfigDialog::OnButtonCancel)
	EVT_BUTTON(mxID_HELP_BUTTON,mxCppCheckConfigDialog::OnButtonHelp)
END_EVENT_TABLE()

mxCppCheckConfigDialog::mxCppCheckConfigDialog(wxWindow *parent)
	: mxDialog(parent, LANG(CPPCHECK_CONFIG_CAPTION,"Configuración de CppCheck") )
{
	if (!project->cppcheck) { 
		project->cppcheck = new cppcheck_configuration();
		project->cppcheck->save_in_project=true;
	}
	ccc = project->cppcheck;
	CreateSizer(this)
		.BeginNotebook()
			.AddPage(this,&mxCppCheckConfigDialog::CreateGeneralPanel, "General")
			.AddPage(this,&mxCppCheckConfigDialog::CreateFilesPanel, "Files")
		.EndNotebook()
		.BeginBottom().Help().Ok().Cancel().EndBottom(this)
		.SetAndFit();
	SetFocus();
	Show();
}

wxPanel *mxCppCheckConfigDialog::CreateGeneralPanel(wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	sizer.BeginCheck( LANG(CPPCHECK_COPY_FROM_CONFIG,"Copiar configuración (macros definidas) de las opciones del proyecto") )
		.Value(ccc->copy_from_config).Id(mxID_CPPCHECK_COPYCONFIG).EndCheck(copy_from_config);
		
	sizer.BeginText( LANG(CPPCHECK_CONFIG_D,"  Configuraciones a verificar") )
		.Value(ccc->config_d).Button(mxID_CPPCHECK_CONFIG_D).EndText(config_d);
	
	sizer.BeginText( LANG(CPPCHECK_CONFIG_U,"  Configuraciones a saltear") )
		.Value(ccc->config_u).Button(mxID_CPPCHECK_CONFIG_U).EndText(config_u);
	
	config_u->Enable(!copy_from_config->GetValue()); config_d->Enable(!copy_from_config->GetValue());
	
	sizer.BeginText( LANG(CPPCHECK_STYLE,"Verificaciones adicionales") )
		.Value(ccc->style).Button(mxID_CPPCHECK_STYLE).EndText(style);
	
	sizer.BeginText( LANG(CPPCHECK_PLATFORM,"Verificaciones especificas de una plataforma") )
		.Value(ccc->platform).Button(mxID_CPPCHECK_PLATFORM).EndText(platform);
	
	sizer.BeginText( LANG(CPPCHECK_STANDARD,"Verificaciones especificas de un estándar") )
		.Value(ccc->standard).Button(mxID_CPPCHECK_STANDARD).EndText(standard);
	
	sizer.BeginText( LANG(CPPCHECK_SUPPRESS_IDS,"Supresiones") )
		.Value(ccc->suppress_ids).Button(mxID_CPPCHECK_SUPPRESS_IDS).EndText(suppress_ids);
									 
	sizer.BeginText( LANG(CPPCHECK_SUPPRESS_FILE,"Archivo con lista de supresiones") )
		.Value(ccc->suppress_file).Button(mxID_CPPCHECK_SUPPRESS_FILE).EndText(suppress_file);
									  
	sizer.BeginCheck( LANG(CPPCHECK_INLINE_SUPPR,"Habilitar supresiones inline") )
		.Value(ccc->inline_suppr).EndCheck(inline_suppr);
	
	sizer.BeginCheck( LANG(CPPCHECK_SAVE,"Guardar esta configuración con el proyecto") )
		.Value(ccc->save_in_project).EndCheck(save_in_project);
	
	sizer.Set();
	return sizer.GetPanel();
}

wxPanel *mxCppCheckConfigDialog::CreateFilesPanel (wxNotebook * notebook) {
	CreatePanelAndSizer sizer(notebook); wxPanel *panel = sizer.GetPanel();
	
	wxSizer *src_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *szsrc_buttons = new wxBoxSizer(wxVERTICAL);
	szsrc_buttons->Add(new wxButton(panel,mxID_CPPCHECK_INCLUDE_FILE,">>>",wxDefaultPosition,wxSize(50,-1)),sizers->BA10_Exp0);
	szsrc_buttons->Add(new wxButton(panel,mxID_CPPCHECK_EXCLUDE_FILE,"<<<",wxDefaultPosition,wxSize(50,-1)),sizers->BA10_Exp0);
	wxSizer *szsrc_in = new wxBoxSizer(wxVERTICAL);
	szsrc_in->Add(new wxStaticText(panel,wxID_ANY,LANG(CPPCHECK_SOURCES_IN,"Files to analize")),sizers->Exp0);
	sources_in = new wxListBox(panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,nullptr,wxLB_SORT|wxLB_EXTENDED|wxLB_NEEDED_SB);
	szsrc_in->Add(sources_in,sizers->Exp1);
	wxSizer *szsrc_out = new wxBoxSizer(wxVERTICAL);
	szsrc_out->Add(new wxStaticText(panel,wxID_ANY,LANG(CPPCHECK_SOURCES_OUT,"Files to exclude")),sizers->Exp0);
	sources_out = new wxListBox(panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,nullptr,wxLB_SORT|wxLB_EXTENDED|wxLB_NEEDED_SB);
	szsrc_out->Add(sources_out,sizers->Exp1);
	src_sizer->Add(szsrc_out,sizers->Exp1);
	src_sizer->Add(szsrc_buttons,sizers->Center);
	src_sizer->Add(szsrc_in,sizers->Exp1);
	
	wxArrayString files,exclude_list;
	mxUT::Split(project->cppcheck->additional_files,files,true,false);
	mxUT::Split(project->cppcheck->exclude_list,exclude_list,true,false);
	project->GetFileList(files,FT_SOURCE,true);
	if (!project->cppcheck->exclude_headers) project->GetFileList(files,FT_HEADER,true);
	for (unsigned int i=0;i<files.GetCount();i++) {
		if (exclude_list.Index(files[i])==wxNOT_FOUND) {
			sources_in->Append(files[i]);
		} else {
			sources_out->Append(files[i]);
		}
	}
	
	sizer.GetSizer()->Add(src_sizer,sizers->Exp1);
	
	sizer.BeginText( LANG(CPPCHECK_ADDITIONAL_FILES,"Archivos adicionales a analizar") )
		.Value(ccc->additional_files).Button(mxID_CPPCHECK_ADDITIONAL_FILES).EndText(additional_files);
	
	sizer.BeginCheck( LANG(CPPCHECK_EXCLUDE_HEADERS,"Omitir archivos de cabeceras") )
		.Value(ccc->exclude_headers).Id(mxID_CPPCHECK_EXCLUDE_HEADERS).EndCheck(exclude_headers);
	
	sizer.SetAndFit();
	return sizer.GetPanel();
}

void mxCppCheckConfigDialog::OnCheckCopyConfig (wxCommandEvent & evt) {
	evt.Skip();
	config_u->Enable(!copy_from_config->GetValue());
	config_d->Enable(!copy_from_config->GetValue());
}

void mxCppCheckConfigDialog::OnButtonConfigD (wxCommandEvent & evt) {
	new mxEnumerationEditor(this,LANG(CPPCHECK_CONFIG_D,"Configurations to analize"),config_d,true);
}

void mxCppCheckConfigDialog::OnButtonConfigU (wxCommandEvent & evt) {
	new mxEnumerationEditor(this,LANG(CPPCHECK_CONFIG_U,"Configurations to analize"),config_u,true);
}

void mxCppCheckConfigDialog::OnButtonStyle (wxCommandEvent & evt) {
	wxArrayString array;
	array.Add("style");
	array.Add("performance");
	array.Add("portability");
	array.Add("information");
	array.Add("unusedFunction");
	array.Add("missingInclude");
	if (style->GetValue()=="all") style->SetValue("information missingInclude performance portability style unusedFunction");
	new mxMultipleChoiceEditor(this,"CppCheck",LANG(CPPCHECK_STYLE,"Additional checks"),style,array);
	if (style->GetValue()=="information missingInclude performance portability style unusedFunction") style->SetValue("all");
}

void mxCppCheckConfigDialog::OnButtonPlatform (wxCommandEvent & evt) {
	wxArrayString array;
	array.Add("unix32");
	array.Add("unix64");
	array.Add("win32A");
	array.Add("win32W");
	array.Add("win64");
	new mxMultipleChoiceEditor(this,"CppCheck",LANG(CPPCHECK_PLATFORM,"Specific platforms"),platform,array);
}

void mxCppCheckConfigDialog::OnButtonStandard (wxCommandEvent & evt) {
	wxArrayString array;
	array.Add("posix");
	array.Add("c99");
	array.Add("c++11");	
	new mxMultipleChoiceEditor(this,"CppCheck",LANG(CPPCHECK_STANDARD,"Standard related checks"),standard,array);
}

void mxCppCheckConfigDialog::OnButtonSuppressFile (wxCommandEvent & evt) {
	wxFileDialog dlg(this,_T("Ubicacion del ejecutable:"),DIR_PLUS_FILE(project->path,suppress_file->GetValue()));
	if (wxID_OK==dlg.ShowModal())
		suppress_file->SetValue(mxUT::Relativize(dlg.GetPath(),project->path));
}

void mxCppCheckConfigDialog::OnButtonSuppressIds (wxCommandEvent & evt) {
	new mxEnumerationEditor(this,LANG(CPPCHECK_SUPPRESS_IDS,"Suppressions"),suppress_ids,true);
}

void mxCppCheckConfigDialog::OnButtonIncludeFile (wxCommandEvent & evt) {
	sources_in->SetSelection(wxNOT_FOUND);
	for (int i=sources_out->GetCount()-1;i>=0;i--)
		if (sources_out->IsSelected(i)) {
			sources_in->Append(sources_out->GetString(i));
			sources_in->Select(sources_in->FindString(sources_out->GetString(i)));
			sources_out->Delete(i);
		}
}

void mxCppCheckConfigDialog::OnButtonExcludeFile (wxCommandEvent & evt) {
	sources_out->SetSelection(wxNOT_FOUND);
	for (int i=sources_in->GetCount()-1;i>=0;i--)
		if (sources_in->IsSelected(i)) {
			sources_out->Append(sources_in->GetString(i));
			sources_out->Select(sources_out->FindString(sources_in->GetString(i)));
			sources_in->Delete(i);
		}
}

void mxCppCheckConfigDialog::OnButtonOk (wxCommandEvent & evt) {
	if (!project) return;
	ccc->exclude_list.Clear();
	for (unsigned int i=0;i<sources_out->GetCount();i++)
		ccc->exclude_list<<mxUT::Quotize(sources_out->GetString(i))<<" ";
	ccc->copy_from_config=copy_from_config->GetValue();
	ccc->config_d=config_d->GetValue();
	ccc->config_u=config_u->GetValue();
	ccc->style=style->GetValue();
	ccc->platform=platform->GetValue();
	ccc->standard=standard->GetValue();
	ccc->suppress_file=suppress_file->GetValue();
	ccc->suppress_ids=suppress_ids->GetValue();
	ccc->inline_suppr=inline_suppr->GetValue();
	ccc->save_in_project=save_in_project->GetValue();
	ccc->exclude_headers=exclude_headers->GetValue();
	ccc->additional_files=additional_files->GetValue();
	Close();
}

void mxCppCheckConfigDialog::OnButtonCancel (wxCommandEvent & evt) {
	Close();
}

void mxCppCheckConfigDialog::OnButtonHelp (wxCommandEvent & evt) {
	mxHelpWindow::ShowHelp("cppcheck_config.html");
}

void mxCppCheckConfigDialog::OnButtonExcludeHeaders (wxCommandEvent & evt) {
	wxArrayString files;
	project->GetFileList(files,FT_HEADER,true);
	if (exclude_headers->GetValue()) {
		for(unsigned int i=0;i<files.GetCount();i++) { 
			int p1 = sources_in->FindString(files[i]);
			if (p1!=wxNOT_FOUND) { sources_in->Delete(p1); continue; }
			int p2 = sources_out->FindString(files[i]);
			if (p2!=wxNOT_FOUND) { sources_out->Delete(p2); }
		}
	} else {
		wxArrayString exclude_list;
		mxUT::Split(project->cppcheck->exclude_list,exclude_list,true,false);
		for (unsigned int i=0;i<files.GetCount();i++) {
			if (exclude_list.Index(files[i])==wxNOT_FOUND) {
				sources_in->Append(files[i]);
			} else {
				sources_out->Append(files[i]);
			}
		}
	}
}

void mxCppCheckConfigDialog::OnButtonAdditionalFiles (wxCommandEvent & evt) {
	mxEnumerationEditor(this,LANG(CPPCHECK_ADDITIONAL_FILES,"Archivos adicionales a analizar"),additional_files,true);
}

