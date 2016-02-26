#include <wx/panel.h>
#include "mxProjectStatistics.h"
#include "mxUtils.h"
#include "mxBitmapButton.h"
#include "ProjectManager.h"
#include "ids.h"
#include "mxSizers.h"
#include "mxHelpWindow.h"
#include "Parser.h"
#include "parserData.h"
#include "Language.h"
#include <wx/textfile.h>
#include "mxCommonConfigControls.h"

BEGIN_EVENT_TABLE(mxProjectStatistics, wxDialog)
	EVT_BUTTON(wxID_CANCEL,mxProjectStatistics::OnCloseButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxProjectStatistics::OnHelpButton)
	EVT_CLOSE(mxProjectStatistics::OnClose)
	EVT_TIMER(wxID_ANY,mxProjectStatistics::OnTimer)
END_EVENT_TABLE()
	
mxProjectStatistics::mxProjectStatistics(wxWindow *parent) : wxDialog(parent, wxID_ANY, LANG(PROYSTATS_CAPTION,"Estadisticas del Proyecto"), wxDefaultPosition, wxSize(450,400) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	project->UpdateSymbols();
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	
	cant_total = mxCCC::AddStaticText(mySizer,this,LANG(PROYSTATS_NUM_TOTAL,"Archivos en el Proyecto"),"");
	cant_headers = mxCCC::AddStaticText(mySizer,this,LANG(PROYSTATS_NUM_HEADERS,"Cabeceras"),"",true);
	cant_sources = mxCCC::AddStaticText(mySizer,this,LANG(PROYSTATS_NUM_SOURCES,"Fuentes"),"",true);
	cant_others = mxCCC::AddStaticText(mySizer,this,LANG(PROYSTATS_NUM_OTHERS,"Otros"),"",true);
	total_size = mxCCC::AddStaticText(mySizer,this,LANG(PROYSTATS_DISK_USAGE,"Tamaño en Disco (*)"),"12");
	mxCCC::AddStaticText(mySizer,this,"");
	cant_lines = mxCCC::AddStaticText(mySizer,this,LANG(PROYSTATS_LINES,"Lineas de Codigo"),"");
	cant_class = mxCCC::AddStaticText(mySizer,this,LANG(PROYSTATS_NUM_CLASSES,"Clases Definidas"),"",true);
	cant_funcs = mxCCC::AddStaticText(mySizer,this,LANG(PROYSTATS_NUM_FUNCTIONS,"Funciones Definidas"),"",true);
	cant_globs = mxCCC::AddStaticText(mySizer,this,LANG(PROYSTATS_NUM_GLOBALS,"Objetos Globales"),"",true);
	cant_defs = mxCCC::AddStaticText(mySizer,this,LANG(PROYSTATS_NUM_MACROS,"Macros de Preprocesador"),"",true);
	mxCCC::AddStaticText(mySizer,this,"");
	mxCCC::AddStaticText(mySizer,this,LANG(PROYSTATS_NOTE_DISK_USAGE,"(*) no incluye binarios ni temporales"));
	
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	wxButton *close_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.ok,LANG(GENERAL_CANCEL_BUTTON,"&Cerrar"));
	SetEscapeId(wxID_CANCEL);
	close_button->SetDefault(); 
	
	buttonSizer->Add(help_button,sizers->BA5_Exp0);
	buttonSizer->AddStretchSpacer(1);
	buttonSizer->Add(close_button,sizers->BA5_Right);
	
	mySizer->Add(buttonSizer,sizers->Exp0);
	
	SetSizerAndFit(mySizer);
	
	SetFocus();
	Show();
	SetValues();
	
}

void mxProjectStatistics::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxProjectStatistics::OnCloseButton(wxCommandEvent &evt) {
	Close();
}

void mxProjectStatistics::OnHelpButton(wxCommandEvent &event){
	mxHelpWindow::ShowHelp("proy_stats.html");
}

void mxProjectStatistics::SetValues(bool all) {
	if (all) {
		unsigned long sz=0, tsz=wxFileName::GetSize(project->filename).ToULong();
		if (tsz!=wxInvalidSize) sz+=tsz;
		int ct=0, lc=0;
		
		{
			int ch=0;
			LocalListIterator<project_file_item*> fi(&project->files_headers);
			while (fi.IsValid()) { 
				ch++; ct++;
				tsz=wxFileName::GetSize(DIR_PLUS_FILE(project->path,fi->name)).ToULong();
				wxTextFile tf(DIR_PLUS_FILE(project->path,fi->name));
				if (tsz!=wxInvalidSize) sz+=tsz;
				if (tf.Open()) {
					lc+=tf.GetLineCount();
					tf.Close();
				}
				fi.Next();
			}
			cant_headers->SetLabel(wxString()<<ch);
		}
		
		{
			int cs=0;
			LocalListIterator<project_file_item*> fi(&project->files_sources);
			while(fi.IsValid()) { 
				ct++; cs++; 
				tsz=wxFileName::GetSize(DIR_PLUS_FILE(project->path,fi->name)).ToULong();
				if (tsz!=wxInvalidSize) sz+=tsz;
				wxTextFile tf(DIR_PLUS_FILE(project->path,fi->name));
				if (tf.Open()) {
					lc+=tf.GetLineCount();
					tf.Close();
				}
				fi.Next();
			}
			cant_sources->SetLabel(wxString()<<cs);
		}
		
		{
			int co=0;
			LocalListIterator<project_file_item*> fi(&project->files_others);
			while(fi.IsValid()) { 
				co++; ct++;
				tsz=wxFileName::GetSize(DIR_PLUS_FILE(project->path,fi->name)).ToULong();
				if (tsz!=wxInvalidSize) sz+=tsz;
				fi.Next();
			}
			cant_others->SetLabel(wxString()<<co);
		}
		
		cant_total->SetLabel(wxString()<<ct);
		
		cant_lines->SetLabel(wxString()<<lc);
	
		wxString ssz;
		if (sz>1024*1024) ssz<<double(sz*100/1024/1024)/100<<"MB";
		else if (sz>1024) ssz<<double(sz*100/1024)/100<<"kB";
		else ssz<<sz<<"B";
		total_size->SetLabel(wxString()<<ssz);
	}
	
	if (parser->working) {
		wait_for_parser = new wxTimer(GetEventHandler(),wxID_ANY); 
		wait_for_parser->Start(1000,true);
	} else {
	
		int c=0;
		pd_class *aux_class = parser->last_class;
		ML_ITERATE(aux_class) c++;
		cant_class->SetLabel(wxString()<<c);
		
		c=0;
		pd_func *aux_func = parser->last_function;
		ML_ITERATE(aux_func) c++;
		cant_funcs->SetLabel(wxString()<<c);
		
		c=0;
		pd_var *aux_var = parser->last_global;
		ML_ITERATE(aux_var) c++;
		cant_globs->SetLabel(wxString()<<c);
		
		c=0;
		pd_macro *aux_def = parser->last_macro;
		ML_ITERATE(aux_def) c++;
		cant_defs->SetLabel(wxString()<<c);
	}
	
	GetSizer()->Layout();
}

void mxProjectStatistics::OnTimer(wxTimerEvent &evt) {
	if (parser->working) { wait_for_parser->Start(1000,true); return; }
	SetValues(false);
}
