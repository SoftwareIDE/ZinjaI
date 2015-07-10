#include <wx/notebook.h>
#include <wx/panel.h>
#include "mxExeInfo.h"
#include "mxSource.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include "mxMessageDialog.h"
#include "ProjectManager.h"
#include "ids.h"
#include "mxSizers.h"
#include "mxHelpWindow.h"
#include "Parser.h"
#include "Language.h"
#include "mxCompiler.h"

BEGIN_EVENT_TABLE(mxExeInfo, wxDialog)
	EVT_BUTTON(wxID_OK,mxExeInfo::OnCloseButton)
	EVT_BUTTON(mxID_EXEINFO_STRIP,mxExeInfo::OnStripButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxExeInfo::OnHelpButton)
	EVT_CLOSE(mxExeInfo::OnClose)
	EVT_TIMER(wxID_ANY,mxExeInfo::OnTimer)
END_EVENT_TABLE()
	
mxExeInfo::mxExeInfo(wxWindow *parent, mxSource *src) : wxDialog(parent, wxID_ANY, LANG(EXEINFO_CAPTION,"Propiedades del Ejecutable"), wxDefaultPosition, wxSize(450,400) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	fname = src?src->GetBinaryFileName().GetFullPath():wxFileName(DIR_PLUS_FILE(project->path,project->active_configuration->output_file));
	if (!fname.FileExists() || (src&&src->sin_titulo&&compiler->last_compiled!=src) ) {
		mxMessageDialog(LANG(EXEINFO_NOT_FOUND,"No se encontro ejecutable. Compile el programa para crearlo."),LANG(GENERAL_WARNING,"Aviso"),mxMD_OK).ShowModal();
		Destroy();
		return;
	}
	
	source=src;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxNotebook *notebook = new wxNotebook(this,wxID_ANY);
	
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	wxButton *close_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.ok,LANG(GENERAL_CLOSE_BUTTON,"&Cerrar"));
	wxButton *strip_button = new mxBitmapButton (this,mxID_EXEINFO_STRIP,bitmaps->buttons.cancel,LANG(EXEINFO_STRIP,"&Strip"));
	SetEscapeId(wxID_CANCEL);
	close_button->SetDefault(); 
	
	buttonSizer->Add(help_button,sizers->BA5_Exp0);
	buttonSizer->AddStretchSpacer(1	);
	buttonSizer->Add(strip_button,sizers->BA5_Right);
	buttonSizer->Add(close_button,sizers->BA5_Right);
	
	notebook->AddPage(CreateGeneralPanel(notebook), LANG(EXEINFO_GENERAL,"General"));
	notebook->AddPage(CreateDependPanel(notebook), LANG(EXEINFO_DEPS,"Dependencias"));
	
	if (!parser->working) {
		wait_for_parser = nullptr;
		UpdateTypeAndDeps();
	} else {
		wait_for_parser = new wxTimer(GetEventHandler(),wxID_ANY); 
		wait_for_parser->Start(1000,false);
	}
	
	mySizer->Add(notebook,sizers->Exp1);
	mySizer->Add(buttonSizer,sizers->Exp0);
	
	SetSizer(mySizer);
	
	SetFocus();
	Show();
	
}

wxPanel *mxExeInfo::CreateGeneralPanel (wxNotebook *notebook) {
	
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	mxUT::AddTextCtrl(sizer,panel,LANG(EXEINFO_LOCATION,"Ubicacion"),fname.GetFullPath())->SetEditable(false);

	double fsize = fname.GetSize().ToDouble();
	wxString tsize;
	if (fsize>1024) {
		if (fsize>1024*1024) {
			tsize<<int((fsize/1024/1024))<<"."<<int(fsize/1024/1.024)%1000<<" MB = ";
		} else
			tsize<<int(fsize/1024)<<"."<<int(fsize/1.024)%1000<<" KB = ";
	}
	tsize<<fname.GetSize()<<" B";
	
	text_size = mxUT::AddTextCtrl(sizer,panel,LANG(EXEINFO_SIZE,"Tama�o"),tsize);
	text_size->SetEditable(false);
	
	text_time = mxUT::AddTextCtrl(sizer,panel,LANG(EXEINFO_LAST_MOD_DATE,"Fecha ultima modifiacion"),fname.GetModificationTime().Format("%H:%M:%S - %d/%B/%Y"));
	text_time->SetEditable(false);

	if (!source || !source->sin_titulo) {
		bool updated=true;
		if (source) {
			if (source->GetModify() || !source->GetBinaryFileName().FileExists() || source->GetBinaryFileName().GetModificationTime()<source->source_filename.GetModificationTime())
				updated=false;
			else if (config->Running.check_includes && mxUT::AreIncludesUpdated(source->GetBinaryFileName().GetModificationTime(),source->source_filename))
				updated=false;
		} else {
			if (project->PrepareForBuilding())
				updated=false;
		}
		mxUT::AddShortTextCtrl(sizer,panel,LANG(EXEINFO_UPDATED,"Actualizado"),updated?LANG(EXEINFO_YES,"Si"):LANG(EXEINFO_NO,"No"))->SetEditable(false);
	}
	
	wxString file_type = LANG(EXEINFO_WAIT_FOR_PARSER,"No se puede determinar el tipo mientras el parser esta analizando fuentes");
	
	text_type = mxUT::AddLongTextCtrl(sizer,panel,LANG(EXEINFO_FILE_TYPE,"Tipo de Archivo"),file_type);
	text_type ->SetEditable(false);
	
	panel->SetSizer(sizer);
	return panel;
	
}

wxPanel *mxExeInfo::CreateDependPanel (wxNotebook *notebook) {
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	wxString ldd = LANG(EXEINFO_WAIT_FOR_PARSER,"No se puede determinar esta informacion mientras el parser esta analizando fuentes");
	
#if !defined(_WIN32) && !defined(__WIN32__)
	ldd_ctrl = mxUT::AddLongTextCtrl(sizer,panel,_T("ldd"),ldd);
#else
	ldd_ctrl = mxUT::AddLongTextCtrl(sizer,panel,_T("lsdeps"),ldd);
#endif
	
	panel->SetSizer(sizer);
	return panel;
}

void mxExeInfo::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxExeInfo::OnCloseButton(wxCommandEvent &evt) {
	Close();
}

void mxExeInfo::OnStripButton(wxCommandEvent &evt) {
	mxUT::GetOutput(wxString("strip \"")<<fname.GetFullPath()<<"\"");
	wxString tsize;
	double fsize = fname.GetSize().ToDouble();
	if (fsize>1024) {
		if (fsize>1024*1024) {
			tsize<<int((fsize/1024/1024))<<"."<<int(fsize/1024/1.024)%1000<<" MB = ";
		} else
			tsize<<int(fsize/1024)<<"."<<int(fsize/1.024)%1000<<" KB = ";
	}
	tsize<<fname.GetSize()<<" B";
	text_size->SetValue(tsize);	
	text_type->SetValue(mxUT::GetFileTypeDescription(fname.GetFullPath()));
	text_time->SetValue(fname.GetModificationTime().Format("%H:%M:%S - %d/%B/%Y"));
}

void mxExeInfo::OnHelpButton(wxCommandEvent &event){
	mxHelpWindow::ShowHelp("exe_info.html");
}

void mxExeInfo::OnTimer(wxTimerEvent &evt) {
	if (parser->working) { return; }
	wait_for_parser->Stop();  UpdateTypeAndDeps();
}

void mxExeInfo::UpdateTypeAndDeps ( ) {
	wxString ldd_cmd = OSDEP_VAL( DIR_PLUS_FILE(config->zinjai_bin_dir,"lsdeps.exe"), "ldd" );
	ldd_ctrl->SetValue(mxUT::GetOutput(ldd_cmd+" "+mxUT::Quotize(fname.GetFullPath()),true,false));
	text_type->SetValue(mxUT::GetFileTypeDescription(fname.GetFullPath()));
}

