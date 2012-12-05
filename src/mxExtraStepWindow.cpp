#include "mxExtraStepWindow.h"
#include "mxUtils.h"
#include "ids.h"
#include "ProjectManager.h"
#include "mxArt.h"
#include "mxSizers.h"
#include "mxBitmapButton.h"
#include "mxHelpWindow.h"
#include "mxMessageDialog.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxExtraStepWindow, wxDialog)
	EVT_BUTTON(wxID_CANCEL,mxExtraStepWindow::OnCancelButton)
	EVT_BUTTON(wxID_OK,mxExtraStepWindow::OnOkButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxExtraStepWindow::OnHelpButton)
	EVT_BUTTON(mxID_EXTRA_STEP_COMMAND,mxExtraStepWindow::OnCommandButton)
	EVT_BUTTON(mxID_EXTRA_STEP_OUTPUT,mxExtraStepWindow::OnOutputButton)
	EVT_BUTTON(mxID_EXTRA_STEP_DEPS,mxExtraStepWindow::OnDepsButton)
	EVT_MENU_RANGE(mxID_POPUPS_INSERT_FIRST, mxID_POPUPS_INSERT_LAST, mxExtraStepWindow::OnPopup)
	EVT_CLOSE(mxExtraStepWindow::OnClose)
END_EVENT_TABLE()

mxExtraStepWindow::mxExtraStepWindow(wxWindow *parent, project_configuration *conf, compile_extra_step *astep) : wxDialog(parent,wxID_ANY,_T("Paso de Compilacion Personalizable"),wxDefaultPosition,wxDefaultSize) {
	configuration=conf;
	step=astep;
	
	wxBoxSizer *mySizer= new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *butSizer = new wxBoxSizer(wxHORIZONTAL);
	
	name = utils->AddTextCtrl(mySizer,this,LANG(EXTRASTEP_NAME,"Nombre"),step?step->name:_T(""));
	command = utils->AddDirCtrl(mySizer,this,LANG(EXTRASTEP_COMMAND,"Comando"),step?step->command:_T(""),mxID_EXTRA_STEP_COMMAND);
	deps = utils->AddDirCtrl(mySizer,this,LANG(EXTRASTEP_DEPS,"Dependencias"),step?step->deps:_T(""),mxID_EXTRA_STEP_DEPS);
	output = utils->AddDirCtrl(mySizer,this,LANG(EXTRASTEP_OUTPUT,"Archivo de salida"),step?step->out:_T(""),mxID_EXTRA_STEP_OUTPUT);
	if (!step) {
		wxArrayString pos_array;
		pos_array.Add(_T(""),CES_COUNT);
		pos_array[CES_BEFORE_SOURCES]=LANG(EXTRASTEP_BEFORE_SOURCES,"Antes de compilar los fuentes");
		pos_array[CES_BEFORE_LIBS]=LANG(EXTRASTEP_BEFORE_LIBS,"Antes de generar las bibliotecas");
		pos_array[CES_BEFORE_EXECUTABLE]=LANG(EXTRASTEP_BEFORE_EXECUTABLE,"Antes de enlazar el ejecutable");
		pos_array[CES_AFTER_LINKING]=LANG(EXTRASTEP_AFTER_LINKING,"Despues de enlazar el ejecutable");
		position = utils->AddComboBox(mySizer,this,LANG(EXTRASTEP_LOCATION_ON_SEQ,"Ubicacion en la secuencia"),pos_array,0);
	} else
		position = NULL;
	link_output = utils->AddCheckBox(mySizer,this,LANG(EXTRASTEP_LINK_FORCE_RELINK,"Enlazar el archivo de salida en el ejecutable"),step?step->link_output:true);
	delclean = utils->AddCheckBox(mySizer,this,LANG(EXTRASTEP_DELETE_ON_CLEAN,"Eliminar al limpiar el proyecto"),step?step->delete_on_clean:true);
	check_rv = utils->AddCheckBox(mySizer,this,LANG(EXTRASTEP_CHECK_EXIT_CODE,"Verificar codigo de salida"),step?step->check_retval:false);
	hide_win = utils->AddCheckBox(mySizer,this,LANG(EXTRASTEP_HIDE_RUNNING_WINDOW,"Ocultar ventana de ejecucion"),step?step->hide_window:true);
	
	wxBitmapButton *help_button = new wxBitmapButton(this,mxID_HELP_BUTTON,*bitmaps->buttons.help);
	butSizer->Add(help_button,sizers->BA5);
	butSizer->AddStretchSpacer();
	wxBitmapButton *cancel_button = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	butSizer->Add(cancel_button,sizers->BA5);
	wxBitmapButton *ok_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	butSizer->Add(ok_button,sizers->BA5);
	
	mySizer->Add(butSizer,sizers->BA5_Exp0);
	
	mySizer->SetMinSize(400,100);
	SetSizerAndFit(mySizer);
	ok_button->SetDefault();
	SetEscapeId(wxID_CANCEL);
	name->SetFocus();
	ShowModal();
}

mxExtraStepWindow::~mxExtraStepWindow() {
	
}

void mxExtraStepWindow::OnClose(wxCloseEvent &evt) {
	Destroy();
}

void mxExtraStepWindow::OnOkButton(wxCommandEvent &evt) {
	wxString sname = name->GetValue().Trim(true).Trim(false);
	wxString scmd = command->GetValue().Trim(true).Trim(false);
	wxString sdeps = deps->GetValue().Trim(true).Trim(false);
	wxString sout = output->GetValue().Trim(true).Trim(false);
	if (!sname.Len()) {
		mxMessageDialog(this,LANG(EXTRASTEP_NAME_MISSING,"Debe completar el nombre del paso."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
		return;
	}
	if (!scmd.Len()) {
		mxMessageDialog(this,LANG(EXTRASTEP_COMMAND_MISSING,"Debe completar el comando del paso."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
		return;
	}
	compile_extra_step *aux = configuration->extra_steps;
	while (aux) {
		if (aux->name==sname && (!step || step!=aux)) {
			mxMessageDialog(this,LANG(EXTRASTEP_NAME_REPEATED,"Ya existe otro paso con ese nombre."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
			return;
		}
		aux=aux->next;
	}
	if (!step)
		step = project->InsertExtraSteps(configuration,sname,scmd,position->GetSelection());
	else {
		step->name=sname;
		step->command=scmd;
	}
	step->deps=sdeps;
	step->out=sout;
	step->check_retval=check_rv->GetValue();
	step->hide_window=hide_win->GetValue();
	step->link_output=link_output->GetValue();
	step->delete_on_clean=delclean->GetValue();
	new_name = step->name;
	Close();
}

void mxExtraStepWindow::OnCancelButton(wxCommandEvent &evt) {
	new_name = _T("");
	Close();
}

void mxExtraStepWindow::OnHelpButton(wxCommandEvent &evt) {
	SHOW_HELP(_T("compile_extra_steps.html"));
}

void mxExtraStepWindow::OnPopup(wxCommandEvent &evt) {
	utils->ProcessTextPopup(evt.GetId());
}

void mxExtraStepWindow::OnCommandButton(wxCommandEvent &evt) {
	utils->ShowTextPopUp(this,LANG(EXTRASTEP_COMMAND,"Comando"),command,"TEXT|LIST|FILE|DIR|DEPS|OUTPUT|MINGW_DIR|PROJECT_PATH|PROJECT_BIN|TEMP_DIR");
}

void mxExtraStepWindow::OnDepsButton(wxCommandEvent &evt) {
	utils->ShowTextPopUp(this,LANG(EXTRASTEP_DEPS,"Dependencias"),deps,"TEXT|LIST|FILE|DIR|PROJECT_PATH|PROJECT_BIN|TEMP_DIR");
}

void mxExtraStepWindow::OnOutputButton(wxCommandEvent &evt) {
	utils->ShowTextPopUp(this,LANG(EXTRASTEP_OUTPUT,"Archivo de salida"),output,"TEXT|LIST|FILE|DIR|PROJECT_PATH|PROJECT_BIN|TEMP_DIR");
}
