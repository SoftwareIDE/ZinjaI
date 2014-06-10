#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/textfile.h>
#include "mxErrorRecovering.h"
#include "ids.h"
#include "mxBitmapButton.h"
#include "ConfigManager.h"
#include "mxSizers.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include "ProjectManager.h"
#include "mxSource.h"
#include "mxMessageDialog.h"
#include "Parser.h"
#include "mxWelcomePanel.h"
#include "Language.h"
#include "MenusAndToolsConfig.h"

BEGIN_EVENT_TABLE(mxErrorRecovering, wxDialog)
	
	EVT_BUTTON(wxID_YES,mxErrorRecovering::OnAllButton)
	EVT_BUTTON(wxID_CANCEL,mxErrorRecovering::OnNoneButton)
	
	EVT_BUTTON(wxID_NO,mxErrorRecovering::OnSomeButton)
//	EVT_BUTTON(mxID_HELP_BUTTON,mxErrorRecovering::OnHelpButton)
	
	EVT_CLOSE(mxErrorRecovering::OnClose)
	
END_EVENT_TABLE()

mxErrorRecovering::mxErrorRecovering() : wxDialog(main_window, wxID_ANY, LANG(ERRORRECOVERY_CAPTION,"Recuperacion ante errores"), wxDefaultPosition, wxSize(400,300) ,wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	wxBoxSizer *mid_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottom_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	list = new wxListBox(this,wxID_ANY);
	
	wxButton *button_all = new mxBitmapButton(this,wxID_YES,bitmaps->buttons.ok,LANG(ERRORRECOVERY_ALL,"Todos"));
	wxButton *button_some = new mxBitmapButton(this,wxID_NO,bitmaps->buttons.ok,LANG(ERRORRECOVERY_SELECTED,"Seleccionados"));
	wxButton *button_none = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(ERRORRECOVERY_NONE,"Ninguno"));
	
	bottom_sizer->Add(button_none,sizers->BA5);
	bottom_sizer->AddStretchSpacer();
	bottom_sizer->Add(button_some,sizers->BA5);
	bottom_sizer->Add(button_all,sizers->BA5);
	
	mid_sizer->Add(new wxStaticText(this,wxID_ANY,LANG(ERRORRECOVERY_PAR1_LINE1,"ZinjaI no se cerro correctamente durante su ultima ejecucion.")),sizers->BA5);
	mid_sizer->Add(new wxStaticText(this,wxID_ANY,LANG(ERRORRECOVERY_PAR1_LINE2,"Algunos archivos en los que trabajaba fueron guardados")),sizers->BA5);
	mid_sizer->Add(new wxStaticText(this,wxID_ANY,LANG(ERRORRECOVERY_PAR1_LINE3,"automaticamente, y ahora puede recuperarlos.")),sizers->BA5);
	mid_sizer->Add(list,sizers->BA5_Exp1);
	mid_sizer->Add(new wxStaticText(this,wxID_ANY,LANG(ERRORRECOVERY_PAR2_LINE1,"Seleccione que archivos desea recuperar (se abriran como")),sizers->BA5);
	mid_sizer->Add(new wxStaticText(this,wxID_ANY,LANG(ERRORRECOVERY_PAR2_LINE2,"archivos sin titulo, utilice guardar como para guardarlos")),sizers->BA5);
	mid_sizer->Add(new wxStaticText(this,wxID_ANY,LANG(ERRORRECOVERY_PAR2_LINE3,"efectivamente).")),sizers->BA5);
	mid_sizer->Add(bottom_sizer,sizers->Exp0);
	
	button_all->SetDefault(); 
	
	SetSizerAndFit(mid_sizer);
	
	wxTextFile fil(DIR_PLUS_FILE(config->home_dir,_T("recovery_log")));	
	fil.Open();
	fil.GetFirstLine();
	wxString str;
	while (!fil.Eof()) {
		str = fil.GetNextLine();
		if (str.Len() && !fil.Eof()) {
			list->Append(str);
			fil.GetNextLine();
			names.Add(str);
			files.Add(fil.GetNextLine());
		}
	}
	fil.Close();
	
	Show();
	
	if (!names.GetCount())
		Close();
	
}

void mxErrorRecovering::OnAllButton(wxCommandEvent &evt) {
	for (unsigned int i=0;i<files.GetCount();i++) {
		mxSource *src =	main_window->OpenFile(DIR_PLUS_FILE(config->home_dir,files[i]));
		if (src!=EXTERNAL_SOURCE) {
			src->SetPageText(names[i]);
			src->SetModify(true);
			src->sin_titulo=true;
		}
	}
	Close();
}

void mxErrorRecovering::OnSomeButton(wxCommandEvent &evt) {
	for (unsigned int i=0;i<files.GetCount();i++) {
		if (list->IsSelected(i)) {
			mxSource *src =	main_window->OpenFile(DIR_PLUS_FILE(config->home_dir,files[i]));
			if (src && src!=EXTERNAL_SOURCE) {
				src->SetPageText(names[i]);
				src->SetModify(true);
				src->sin_titulo=true;
			}
		}
	}
	Close();
}

void mxErrorRecovering::OnNoneButton(wxCommandEvent &evt) {
	Close();
}

void mxErrorRecovering::OnClose(wxCloseEvent &evt) {
	wxRemoveFile(DIR_PLUS_FILE(config->home_dir,_T("recovery_log")));
	Destroy();
}

void mxErrorRecovering::OnHelpButton(wxCommandEvent &evt) {
	
}


bool mxErrorRecovering::RecoverSomething() {
	if (!wxFileName::FileExists(DIR_PLUS_FILE(config->home_dir,"recovery_log"))) return false;
	if (wxFileName::FileExists(DIR_PLUS_FILE(config->home_dir,"kboom.zpr"))) {
		if (mxMD_YES == mxMessageDialog(main_window,LANG(ERRORRECOVERY_PROJECT_MESSAGE,""
			"ZinjaI no se cerro correctamente durante su ultima ejecucion.\n"
			"El proyecto en el que trabajaba fue guardado automaticamente.\n"
			"Desea recuperarlo? (Atencion: al abrir el proyecto recuperado\n"
			"podria sobreescribir la version en disco del proyecto y sus\n"
			"archivos. Se guardara una copia de su archivo de proyecto actual\n"
			"antes de reescribirlo con la version restaurada)"
			),LANG(ERRORRECOVERY_CAPTION,"Recuperacion ante fallos"), mxMD_YES_NO|mxMD_WARNING).ShowModal()) {
				wxTextFile tf(DIR_PLUS_FILE(config->home_dir,_T("kboom.zpr")));
				tf.Open();
				if (wxFileName::FileExists(tf[0])) wxCopyFile(tf[0],tf[0]+".bakcup");
				if (!wxFileName::FileExists(tf[0]+".kaboom")) {
					mxMessageDialog(main_window,LANG(ERRORRECOVERY_PROJECT_PROBLEM,"Ha ocurrido un error al intentar recuperar el proyecto."),LANG(ERRORRECOVERY_CAPTION,"Recuperacion ante fallos"),mxMD_OK|mxMD_ERROR).ShowModal();
					return true;
				}
				project = new ProjectManager(tf[0]+".kaboom");
				project->filename=tf[1];
				tf.Close();
				// abrir los archivos recuperados
				wxTextFile fil(DIR_PLUS_FILE(config->home_dir,"recovery_log"));
				fil.Open();
				fil.GetFirstLine();
				wxString str;
				while (!fil.Eof()) {
					str = fil.GetNextLine();
					if (str.Len() && !fil.Eof()) {
						str = fil.GetNextLine();
						mxSource *the_one=NULL;
						for (unsigned int i=0;i<main_window->notebook_sources->GetPageCount();i++) {
							mxSource *src = ((mxSource*)(main_window->notebook_sources->GetPage(i)));
							if (src->source_filename==str) { the_one=src; break; }
						}
						str=fil.GetNextLine();
						if (the_one && wxFileName::FileExists(str)) {
							the_one->wxStyledTextCtrl::LoadFile(str);
							the_one->SetModify(true);
						}
					}
				}
			fil.Close();
			if (welcome_panel) main_window->ShowWelcome(false);
			
			if (main_window->left_panels) {
				_menu_item(mxID_VIEW_LEFT_PANELS)->Check(true);
				main_window->aui_manager.GetPane(main_window->left_panels).Show();
				main_window->left_panels->SetSelection(config->Init.prefer_explorer_tree?2:0);
			} else {
				_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(config->Init.prefer_explorer_tree);
				_menu_item(mxID_VIEW_PROJECT_TREE)->Check(!config->Init.prefer_explorer_tree);
				if (config->Init.prefer_explorer_tree) {
					main_window->aui_manager.GetPane(main_window->explorer_tree.treeCtrl).Show();
					main_window->aui_manager.GetPane(main_window->project_tree.treeCtrl).Hide();
				} else {
					main_window->aui_manager.GetPane(main_window->explorer_tree.treeCtrl).Hide();
					main_window->aui_manager.GetPane(main_window->project_tree.treeCtrl).Show();
					main_window->project_tree.treeCtrl->ExpandAll();
				}
			}
			// parsear los archivos recuperados
			parser->ParseProject();
		}
		wxRemoveFile(DIR_PLUS_FILE(config->home_dir,"kboom.zpr"));
		wxRemoveFile(DIR_PLUS_FILE(config->home_dir,"recovery_log"));
	} else new mxErrorRecovering;
	return true;
}
