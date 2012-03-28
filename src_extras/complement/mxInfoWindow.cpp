#include "mxInfoWindow.h"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include "ComplementArchive.h"
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/filesys.h>
#include <wx/fs_arc.h>
#include "Application.h"

BEGIN_EVENT_TABLE(mxInfoWindow,wxFrame)
	EVT_BUTTON(wxID_OK,mxInfoWindow::OnButtonOk)
	EVT_BUTTON(wxID_CANCEL,mxInfoWindow::OnButtonCancel)
	EVT_CLOSE(mxInfoWindow::OnClose)
END_EVENT_TABLE()

mxInfoWindow *info_win=NULL;
	
enum STEPS_INFO {STEP_READING_INFO,STEP_SHOWING_INFO,STEP_CREATING_DIRS,STEP_CREATING_FILES,STEP_DONE,STEP_ABORTING};

// devuelve true si el descompresor puede seguir, false si debe abortar
bool callback_info(wxString message, int progress) {
	if (progress) info_win->Progress(progress);
	if (message.Len()) info_win->Notify(message);
	return info_win->GetStep()!=STEP_ABORTING || wxYES!=wxMessageBox(spanish?"¿Desea interrumpir la instalación?":"Abort instalation?",spanish?"Instalación de Complementos":"Complement installer",wxYES_NO);
}
	
	
mxInfoWindow::mxInfoWindow(wxString _dest, wxString _file):wxFrame(NULL,wxID_ANY,spanish?"Instalación de complementos":"Complement installer",wxDefaultPosition,wxDefaultSize) {
	if (!_dest.Len()) _dest=".";
	if (_dest.Last()!='\\' && _dest.Last()!='/') {
#ifdef __WIN32__
		_dest<<"\\";
#else
		_dest<<"/";
#endif
	}
	dest=_dest; file=_file; step=0; info_win=this;
	
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	
	wxStaticText *stext = new wxStaticText(this,wxID_ANY,spanish?"Información del complemento:":"Complement information:");
	sizer->Add(stext,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	
	text = new wxTextCtrl(this,wxID_ANY,(spanish?"Leyendo ":"Reading ")+file,wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
	text->SetMinSize(wxSize(400,200));
	sizer->Add(text,wxSizerFlags().Proportion(1).Expand().Border(wxALL,5));
	
	gauge = new wxGauge(this,wxID_ANY,1);
	sizer->Add(gauge,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	
	wxBoxSizer *but_sizer = new wxBoxSizer(wxHORIZONTAL);
	but_ok = new wxButton(this,wxID_OK,spanish?"Instalar...":"Install...");
	but_cancel = new wxButton(this,wxID_CANCEL,spanish?"Cancelar":"Cancel");
	but_sizer->Add(but_cancel,wxSizerFlags().Border(wxALL,5));
	but_sizer->AddStretchSpacer();
	but_sizer->Add(but_ok,wxSizerFlags().Border(wxALL,5));
	sizer->Add(but_sizer,wxSizerFlags().Proportion(0).Expand());
	SetSizerAndFit(sizer);

	Show(); 
	but_ok->Enable(false);
	wxYield();

	wxFileSystem::AddHandler(new wxArchiveFSHandler);
	wxString desc; int fcount,dcount;
	if (!GetFilesAndDesc(callback_info,file,fcount,dcount,desc)) { wxMessageBox(spanish?"Ha ocurrido un error durante la instalación (GetFilesAndDesc).":"There was a error installing the complement (GetFilesAndDesc)"); Close(); return; }
	if (step==STEP_ABORTING) { Close(); return; }
	desc_split(desc,info);
	text->SetValue(spanish?info.desc_spanish:info.desc_english);
	gauge->SetRange(fcount*2+dcount);
	gauge->SetValue(0);
	but_ok->Enable(true);
	but_ok->SetFocus(); 
	step=STEP_SHOWING_INFO;
	
}

void mxInfoWindow::OnButtonOk (wxCommandEvent & evt) {
	if (step==STEP_SHOWING_INFO) {
		if (info.closereq && wxNO==wxMessageBox(spanish?"Debe cerrar todas las instancias abiertas de ZinjaI antes de continuar. ¿Instalar ahora?.":"You must close all ZinjaI's instances before continuing. Install now?",spanish?"Advertencia":"Warning",wxYES_NO)) 
			return;
		but_ok->Enable(false);
		text->SetValue(spanish?"Instalando...\n":"Installing...\n");
		step=STEP_CREATING_DIRS;
		if (!CreateDirectories(callback_info,file,dest)) 
		if (step==STEP_ABORTING) { Close(); return; }
		if (!ExtractFiles(callback_info,file,dest)) { wxMessageBox(spanish?"Ha ocurrido un error durante la instalación (ExtractFiles).":"There was a error installing the complement (ExtractFiles)"); Close(); return; }
		if (step==STEP_ABORTING) { Close(); return; }
		// falta setear permisos a ejecutables
#ifndef __WIN32__
		if (info.bins.GetCount()) {
			Notify(spanish?"Corrigiendo permisos...":"Setting permissions...");
			SetBins(info.bins,dest);
		}
#endif
		Progress(1);
		Notify(spanish?"Instalación Finalizada":"Instalation completed.");
		but_cancel->Enable(false);
		but_ok->Enable(true);
		but_ok->SetLabel(spanish?"Cerrar":"Close");
		step=STEP_DONE;
	} else if (step==STEP_DONE) Close();
}

void mxInfoWindow::OnButtonCancel (wxCommandEvent & evt) {
	if (step==STEP_SHOWING_INFO) Close();
	else step=STEP_ABORTING;
}

void mxInfoWindow::OnClose (wxCloseEvent & evt) {
	Destroy();
}

void mxInfoWindow::Notify(const wxString &message) {
	text->AppendText(message+"\n");
	int i=text->GetValue().Len()-1;
	text->SetSelection(i,i);
	wxYield();
}

void mxInfoWindow::Progress (int progress) {
	gauge->SetValue(info_win->gauge->GetValue()+progress);
}

int mxInfoWindow::GetStep ( ) {
	return step;
}

