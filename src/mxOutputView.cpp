#include <wx/file.h>
#include <wx/textctrl.h>
#include <wx/process.h>
#include <wx/txtstrm.h>
#include "mxOutputView.h"
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "ConfigManager.h"
#include "ids.h"
#include "mxUtils.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxOutputView, wxDialog)
	EVT_BUTTON(wxID_OK,mxOutputView::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxOutputView::OnExtraButton)
	EVT_CLOSE(mxOutputView::OnClose)
	EVT_TIMER(mxID_TIMER_OUTPUT, mxOutputView::OnTimer)
	EVT_END_PROCESS(wxID_ANY, mxOutputView::OnProcessTerminate)
END_EVENT_TABLE()
	
/**
* @param caption       title for this dialog
* @param extra_mode    mode for the additional button (mxOV_EXTRA_*)
* @param extra_label   if extra_mode!=mxOV_EXTRA_NULL, label for the additional button
* @param extra_command if extra_mode!=mxOV_EXTRA_NULL, file/URL to be opened/launched by the additional button
* @param output_mode   mode for openning results pannel in main_window (mxOV_OUTPUT_*)
* @param output_file   if output_mode!=mxVO_NULL, the full path for the output file
**/
mxOutputView::mxOutputView(wxString caption, mxOVmode extra_mode, wxString extra_label, wxString extra_command, mxVOmode output_mode, wxString output_file) : wxDialog(main_window, wxID_ANY, caption, wxDefaultPosition, wxSize(600,500) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	process = NULL;
	working = false;
	this->extra_mode=extra_mode;
	this->extra_label=extra_label;
	this->extra_command=extra_command;
	this->output_mode=output_mode;
	this->output_file=output_file;
	
	if (output_mode!=mxVO_NULL) {
		textfile=new wxFile(output_file,wxFile::write);
	} else textfile=NULL;
	
	timer = new wxTimer(GetEventHandler(),mxID_TIMER_OUTPUT);
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	
	mySizer->Add(new wxStaticText(this,wxID_ANY,LANG(LAUNCH_STANDAR_OUTPUT,"Salida estandar")),sizers->BLRT5_Exp0);
	ctrl_std = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
	mySizer->Add(ctrl_std,sizers->BA5_Exp1);
	
	mySizer->Add(new wxStaticText(this,wxID_ANY,LANG(LAUNCH_ERROR_OUTPUT,"Salida de errores")),sizers->BLRT5_Exp0);
	ctrl_err = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
	mySizer->Add(ctrl_err,sizers->BA5_Exp1);
	
	mySizer->Add(state = new wxStaticText(this,wxID_ANY,LANG(LAUNCH_STATUS_STARTING,"Estado: Comenzando")),sizers->BA5_Exp0);
	
	
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	
	close_button= new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_CLOSE_BUTTON,"&Cerrar"));
	SetEscapeId(wxID_OK);
	close_button->SetDefault(); 
	close_button->Enable(false);
	
	extra_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERA_STOP_BUTTON,"Detener"));
	buttonSizer->Add(extra_button,sizers->BA5_Right);
	extra_button->Enable(false);
	
	buttonSizer->Add(close_button,sizers->BA5_Right);
	
	mySizer->Add(buttonSizer,sizers->Center);
	
	SetSizer(mySizer);
//	close_button->SetFocus();
	
	Show();
	
}

mxOutputView::~mxOutputView() {
	delete textfile;
}

void mxOutputView::OnOkButton(wxCommandEvent &evt) {
	Close();
}

void mxOutputView::OnClose(wxCloseEvent &event) {
	timer->Stop();
	if (process) process->Detach();
	Destroy();
}

void mxOutputView::AppendError(wxString text) {
	ctrl_err->AppendText(text);
}

void mxOutputView::AppendOutput(wxString text) {
	ctrl_std->AppendText(text);
}

// cppcheck-suppress publicAllocationError
void mxOutputView::Launch(wxString path, wxString command) {
	process = new wxProcess(GetEventHandler(),wxID_ANY);
	process->Redirect();
	wxSetWorkingDirectory(path);
//	cerr<<command<<endl<<endl<<command.Len();
	command=command.Mid(0,4000);
	pid = wxExecute(command,wxEXEC_ASYNC,process);
	wxSetWorkingDirectory(config->zinjai_dir);
	if (pid) { 
		working=true;
		if (extra_button) extra_button->Enable(true);
		timer->Start(500,true);
		state->SetLabel(LANG(LAUNCH_STATUS_RUNNING,"Estado: Ejecutando"));
	}
}

void mxOutputView::Launched(wxProcess *_process, int _pid) {
	process = _process;
	pid = _pid;
	if (pid) { 
		working=true;
		if (extra_button) extra_button->Enable(true);
		timer->Start(500,true);
		state->SetLabel(LANG(LAUNCH_STATUS_RUNNING,"Estado: Ejecutando"));
	}
}

void mxOutputView::OnProcessTerminate(wxProcessEvent &evt) {
	OnProcessTerminate(evt.GetExitCode());
	delete process;
	process=NULL;
}

void mxOutputView::OnProcessTerminate(int exit_code) {
	close_button->Enable(true);
	working=false;
	if (extra_mode!=mxOV_EXTRA_NULL) {
		if (extra_label.IsEmpty()) {
			extra_button->Disable();
		} else {
			extra_button->SetThings(bitmaps->buttons.next,extra_label);
			GetSizer()->Layout();
			extra_button->SetFocus();
		}
	} else {
		extra_button->Enable(false);
		close_button->SetFocus();
	}
	timer->Stop();
	GetProcessOutput();
	if (exit_code)
		state->SetLabel(wxString(LANG(LAUNCH_STATUS_EXITCODE,"Estado: Codigo de salida: "))<<exit_code);
	else {
		state->SetLabel(LANG(LAUNCH_STATUS_FINISH,"Estado: Terminado"));
		if (textfile) {
			textfile->Close();
			main_window->ShowValgrindPanel(output_mode,output_file,output_mode!=mxVO_CPPCHECK);
			if (extra_mode==mxOV_EXTRA_NULL) Close();
		}
	}
}

void mxOutputView::OnTimer(wxTimerEvent &evt) {
	GetProcessOutput();
}

void mxOutputView::GetProcessOutput() {
	if (!process) return;
	wxTextInputStream input(*(process->GetInputStream()));
	wxTextInputStream input2(*(process->GetErrorStream()));	
	static wxString line;
	static char c;
	line.Clear();
	while (process->IsInputAvailable()) {
		c=input.GetChar();
		if (c!='\r') line<<c;
	}
	ctrl_std->AppendText(line);
	ctrl_std->ShowPosition(ctrl_std->GetLastPosition());
	line.Clear();
	while (process->IsErrorAvailable()) {
		c=input2.GetChar();
		if (c!='\r') line<<c;
	}
	if (textfile) textfile->Write(line);
	ctrl_err->AppendText(line);
	ctrl_err->ShowPosition(ctrl_err->GetLastPosition());
	if (working) timer->Start(500,true);
}

void mxOutputView::OnExtraButton(wxCommandEvent &evt) {
	if (working) process->Kill(pid,wxSIGKILL);
	else if (extra_mode==mxOV_EXTRA_URL) { utils->OpenInBrowser(extra_command); Close(); }
	else if (extra_mode==mxOV_EXTRA_COMMAND) wxExecute(extra_command);
}
