#include "mxExeInfo.h"
#include "mxSource.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include <wx/notebook.h>
#include <wx/panel.h>
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
	
mxExeInfo::mxExeInfo(mxSource *src) : wxDialog(main_window, wxID_ANY, LANG(EXEINFO_CAPTION,"Propiedades del Ejecutable"), wxDefaultPosition, wxSize(450,400) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	wait_for_parser = NULL;
	
	fname = src?src->binary_filename:wxFileName(DIR_PLUS_FILE(project->path,project->active_configuration->output_file));
	if (!fname.FileExists() || (src&&src->sin_titulo&&compiler->last_compiled!=src) ) {
		mxMessageDialog(main_window,LANG(EXEINFO_NOT_FOUND,"No se encontro ejecutable. Compile el programa para crearlo."),LANG(GENERAL_WARNING,"Aviso"),mxMD_OK).ShowModal();
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
	
	mySizer->Add(notebook,sizers->Exp1);
	mySizer->Add(buttonSizer,sizers->Exp0);
	
	SetSizer(mySizer);
	
	SetFocus();
	Show();
	
}

wxPanel *mxExeInfo::CreateGeneralPanel (wxNotebook *notebook) {
	
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	utils->AddTextCtrl(sizer,panel,LANG(EXEINFO_LOCATION,"Ubicacion"),fname.GetFullPath())->SetEditable(false);

	double fsize = fname.GetSize().ToDouble();
	wxString tsize;
	if (fsize>1024) {
		if (fsize>1024*1024) {
			tsize<<int((fsize/1024/1024))<<_T(".")<<int(fsize/1024/1.024)%1000<<_T(" MB = ");
		} else
			tsize<<int(fsize/1024)<<_T(".")<<int(fsize/1.024)%1000<<_T(" KB = ");
	}
	tsize<<fname.GetSize()<<_T(" B");
	
	text_size = utils->AddTextCtrl(sizer,panel,LANG(EXEINFO_SIZE,"Tamaño"),tsize);
	text_size->SetEditable(false);
	
	text_time = utils->AddTextCtrl(sizer,panel,LANG(EXEINFO_LAST_MOD_DATE,"Fecha ultima modifiacion"),fname.GetModificationTime().Format(_T("%H:%M:%S - %d/%B/%Y")));
	text_time->SetEditable(false);

	if (!source || !source->sin_titulo) {
		bool updated=true;
		if (source) {
			if (source->GetModify() || !wxFileName::FileExists(source->binary_filename.GetFullPath()) || source->binary_filename.GetModificationTime()<source->source_filename.GetModificationTime())
				updated=false;
			else if (config->Running.check_includes && utils->AreIncludesUpdated(source->binary_filename.GetModificationTime(),source->source_filename))
				updated=false;
		} else {
			if (project->PrepareForBuilding())
				updated=false;
		}
		utils->AddShortTextCtrl(sizer,panel,LANG(EXEINFO_UPDATED,"Actualizado"),updated?LANG(EXEINFO_YES,"Si"):LANG(EXEINFO_NO,"No"))->SetEditable(false);
	}
	
	wxString file_type = LANG(EXEINFO_WAIT_FOR_PARSER,"No se puede determinar el tipo mientras el parser esta analizando fuentes");
	if (!parser->working) 
		file_type = utils->GetOutput(wxString(_T("file -b \""))<<fname.GetFullPath()<<_T("\""));
	else if (!wait_for_parser) {
		wait_for_parser = new wxTimer(GetEventHandler(),wxID_ANY); 
		wait_for_parser->Start(1000,true);
	}
	text_type = utils->AddLongTextCtrl(sizer,panel,LANG(EXEINFO_FILE_TYPE,"Tipo de Archivo"),file_type);
	text_type ->SetEditable(false);
	
	panel->SetSizer(sizer);
	return panel;
	
}

wxPanel *mxExeInfo::CreateDependPanel (wxNotebook *notebook) {
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	wxString ldd = LANG(EXEINFO_WAIT_FOR_PARSER,"No se puede determinar esta informacion mientras el parser esta analizando fuentes");
	
	if (!parser->working) {
#if !defined(_WIN32) && !defined(__WIN32__)
		ldd = utils->GetOutput(wxString(_T("ldd \""))<<fname.GetFullPath()<<_T("\""));
#else
		ldd = utils->GetOutput(wxString(_T("lsdeps \""))<<fname.GetFullPath()<<_T("\""));
#endif
	} else if (!wait_for_parser) {
		wait_for_parser = new wxTimer(GetEventHandler(),wxID_ANY); 
		wait_for_parser->Start(1000,true);
	}
	
#if !defined(_WIN32) && !defined(__WIN32__)
	ldd_ctrl = utils->AddLongTextCtrl(sizer,panel,_T("ldd"),ldd);
#else
	ldd_ctrl = utils->AddLongTextCtrl(sizer,panel,_T("lsdeps"),ldd);
#endif
	
	panel->SetSizer(sizer);
	return panel;
}

mxExeInfo::~mxExeInfo() {
	
}

void mxExeInfo::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxExeInfo::OnCloseButton(wxCommandEvent &evt) {
	Close();
}

void mxExeInfo::OnStripButton(wxCommandEvent &evt) {
	utils->GetOutput(wxString(_T("strip \""))<<fname.GetFullPath()<<_T("\""));
	wxString tsize;
	double fsize = fname.GetSize().ToDouble();
	if (fsize>1024) {
		if (fsize>1024*1024) {
			tsize<<int((fsize/1024/1024))<<_T(".")<<int(fsize/1024/1.024)%1000<<_T(" MB = ");
		} else
			tsize<<int(fsize/1024)<<_T(".")<<int(fsize/1.024)%1000<<_T(" KB = ");
	}
	tsize<<fname.GetSize()<<_T(" B");
	text_size->SetValue(tsize);	
	text_type->SetValue(utils->GetOutput(wxString(_T("file -b \""))<<fname.GetFullPath()<<_T("\"")));
	text_time->SetValue(fname.GetModificationTime().Format(_T("%H:%M:%S - %d/%B/%Y")));
}

void mxExeInfo::OnHelpButton(wxCommandEvent &event){
	SHOW_HELP(_T("exe_info.html"));
}

void mxExeInfo::OnTimer(wxTimerEvent &evt) {
	if (parser->working) { wait_for_parser->Start(1000,true); return; }
	ldd_ctrl->SetValue(utils->GetOutput(wxString(_T("lsdeps \""))<<fname.GetFullPath()<<_T("\"")));
	text_type->SetValue(utils->GetOutput(wxString(_T("file -b \""))<<fname.GetFullPath()<<_T("\"")));
}
