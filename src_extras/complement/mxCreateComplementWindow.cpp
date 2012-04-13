#include "mxCreateComplementWindow.h"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include "ComplementArchive.h"
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/filesys.h>
#include <wx/fs_arc.h>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/wfstream.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/sstream.h>
#include <iostream>
#include "Application.h"
using namespace std;

BEGIN_EVENT_TABLE(mxCreateComplementWindow,wxFrame)
	EVT_BUTTON(wxID_OK,mxCreateComplementWindow::OnButtonCreate)
	EVT_BUTTON(wxID_CANCEL,mxCreateComplementWindow::OnButtonCancel)
	EVT_BUTTON(wxID_FIND,mxCreateComplementWindow::OnButtonDest)
	EVT_BUTTON(wxID_REPLACE,mxCreateComplementWindow::OnButtonFolder)
	EVT_CLOSE(mxCreateComplementWindow::OnClose)
END_EVENT_TABLE()

mxCreateComplementWindow *create_win=NULL;

#ifdef __WIN32__
#define sep "\\"
#else
#define sep "/"
#endif

	
enum STEPS_CREATE {STEP_ASKING,STEP_ANALYSING,STEP_BUILDING,STEP_DONE,STEP_ABORTING};

// devuelve true si el compresor puede seguir, false si debe abortar
bool callback_create(wxString message, int progress) {
	if (message.Len()) create_win->Notify(message);
	return create_win->GetStep()!=STEP_ABORTING || wxYES!=wxMessageBox(spanish?"¿Desea interrumpir el proceso?":"Abort the process?",spanish?"Generación de Complementos":"Complement Generation",wxYES_NO);
}
	

void FindFiles(wxArrayString &array, wxString where, wxString sub, wxArrayString &bins) {
	
	if (where.EndsWith(sep)) where.RemoveLast();
	if (sub.EndsWith(sep)) sub.RemoveLast();
	
	wxDir fil(sub.Len()?where+sep+sub:where);
	if ( fil.IsOpened() ) {
		wxString filename;
		bool cont = fil.GetFirst(&filename, "*" , wxDIR_FILES);
		while ( cont ) {
			array.Add(sub+sep+filename);
			if (wxFileName::IsFileExecutable(where+array.Last()))
				bins.Add(array.Last());
			cont = fil.GetNext(&filename);
		}	
	}
	
	wxDir dir(sub.Len()?where+sep+sub:where);
	if ( dir.IsOpened() ) {
		wxString filename, spec;
		bool cont = dir.GetFirst(&filename, spec , wxDIR_DIRS);
		while ( cont ) {
			FindFiles(array,where,sub+sep+filename,bins);
			cont = dir.GetNext(&filename);
		}	
	}
	
}
	
mxCreateComplementWindow::mxCreateComplementWindow(wxString path):wxFrame(NULL,wxID_ANY,spanish?"Generación de complementos":"Complement Generation",wxDefaultPosition,wxDefaultSize) {
	create_win=this; step=STEP_ASKING;
	wxFileSystem::AddHandler(new wxArchiveFSHandler);
	
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	
	wxBoxSizer *folder_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *sfolder = new wxStaticText(this,wxID_ANY,spanish?"Directorio con archivos a incluir:":"Directory with files to be included:");
	sizer->Add(sfolder,wxSizerFlags().Proportion(0).Expand().Border(wxTOP|wxLEFT|wxRIGHT,5));
	folder = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize);
	folder_sizer->Add(folder,wxSizerFlags().Proportion(1).Expand());
	wxButton *but_folder = new wxButton(this,wxID_REPLACE,"...",wxDefaultPosition,wxSize(30,folder->GetSize().GetHeight()));
	folder_sizer->Add(but_folder);
	sizer->Add(folder_sizer,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	
	wxStaticText *stext_en = new wxStaticText(this,wxID_ANY,spanish?"Descripción del complemento (ingles):":"Complement's description (english):");
	sizer->Add(stext_en,wxSizerFlags().Proportion(0).Expand().Border(wxTOP|wxLEFT|wxRIGHT,5));
	text_en = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_WORDWRAP);
	text_en->SetMinSize(wxSize(400,150));
	sizer->Add(text_en,wxSizerFlags().Proportion(1).Expand().Border(wxALL,5));
	
	wxStaticText *stext_es = new wxStaticText(this,wxID_ANY,spanish?"Descripción del complemento (español):":"Complement's description (spanish):");
	sizer->Add(stext_es,wxSizerFlags().Proportion(0).Expand().Border(wxTOP|wxLEFT|wxRIGHT,5));
	text_es = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_WORDWRAP);
	text_es->SetMinSize(wxSize(400,150));
	sizer->Add(text_es,wxSizerFlags().Proportion(1).Expand().Border(wxALL,5));
	
	text = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
	text->SetMinSize(wxSize(400,200)); sizer->Add(text,wxSizerFlags().Proportion(1).Expand().Border(wxALL,5));
	text->Hide();
	
	wxBoxSizer *version_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *sversion = new wxStaticText(this,wxID_ANY,spanish?"Versión de ZinjaI requerida: ":"Required ZinjaI version: ");
	version_sizer->Add(sversion,wxSizerFlags().Center());
	version = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize);
	version_sizer->Add(version,wxSizerFlags().Proportion(1).Expand());
	sizer->Add(version_sizer,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	
	close = new wxCheckBox(this,wxID_ANY,spanish?"Requiere cerrar ZinjaI durante la instalación":"Requires closing ZinjaI for correct installation");
	sizer->Add(close,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	
	reset = new wxCheckBox(this,wxID_ANY,spanish?"Requiere reiniciar ZinjaI para completar la instalación":"Requires restarting ZinjaI after installation");
	sizer->Add(reset,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	
	
	wxBoxSizer *dest_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *sdest= new wxStaticText(this,wxID_ANY,spanish?"Archivo de complemento a generar:":"Ouput file:");
	sizer->Add(sdest,wxSizerFlags().Proportion(0).Expand().Border(wxTOP|wxLEFT|wxRIGHT,5));
	dest = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize);
	dest_sizer->Add(dest,wxSizerFlags().Proportion(1).Expand());
	wxButton *but_dest = new wxButton(this,wxID_FIND,"...",wxDefaultPosition,wxSize(30,dest->GetSize().GetHeight()));
	dest_sizer->Add(but_dest);
	sizer->Add(dest_sizer,wxSizerFlags().Proportion(0).Expand().Border(wxALL,5));
	
	
	wxBoxSizer *but_sizer = new wxBoxSizer(wxHORIZONTAL);
	but_create = new wxButton(this,wxID_OK,spanish?"Generar...":"Generate...");
	but_cancel = new wxButton(this,wxID_CANCEL,spanish?"Cancelar":"Cancel");
	but_sizer->Add(but_cancel,wxSizerFlags().Border(wxALL,5));
	but_sizer->AddStretchSpacer();
	but_sizer->Add(but_create,wxSizerFlags().Border(wxALL,5));
	sizer->Add(but_sizer,wxSizerFlags().Proportion(0).Expand());
	SetSizerAndFit(sizer);

	Show(); 
	
	if (path.Len()) SetFolder(path);
	text->SetFocus();

}

#define _return \
but_create->Enable(true);\
step=STEP_ASKING;\
text_es->Show();\
text_en->Show();\
text->Hide();\
but_create->SetLabel(spanish?"Generar...":"Generate..."); \
but_cancel->SetLabel(spanish?"Cancelar":"Cancel"); \
but_create->Enable(); \
Layout();\
return

void mxCreateComplementWindow::OnButtonCreate (wxCommandEvent & evt) {
	if (step==STEP_DONE) { _return; }
	if (step==STEP_ASKING) {
		wxString desc_en=text_en->GetValue(), desc_es=text_es->GetValue();
		if (!desc_es.Len() && !desc_en.Len()) { wxMessageBox(spanish?"La descripción no debe quedar en blanco.":"You must complete at least one description.");  _return; }
		if (!desc_es.Len()) desc_es=desc_en;
		else if (!desc_en.Len()) desc_en=desc_es;
		if (!folder->GetValue().Len()) { wxMessageBox(spanish?"Falta especificar la carpeta de donde tomar los archivos.":"You must specify the origin folder");  _return; }
		if (!dest->GetValue().Len()) { wxMessageBox(spanish?"Falta especificar el archivo de destino.":"You must specify the complement filename.");  _return; }
		text->SetValue(spanish?"Generando...\n":"Generating...\n");
		but_create->Enable(false);
		text->Show(); text_en->Hide(); text_es->Hide(); Layout();
		step=STEP_ANALYSING;
		info.desc_english=desc_en;
		info.desc_spanish=desc_es;
		if (!version->GetValue().Len()) {
			info.reqver=0;
		} else if (!version->GetValue().ToLong(&info.reqver)) {
			wxMessageBox(spanish?"El número de versión requerida no es correcto.":"Wrong version number."); 
			_return;
		}
		info.closereq=close->GetValue();

		wxArrayString files,bins;
		info.bins.Clear();
		FindFiles(files,folder->GetValue(),"",info.bins);
		
		wxString desc_text;
		desc_merge(info,desc_text);
		wxString where=folder->GetValue();
		if (!wxFileName::DirExists(where+sep+"temp"))
			wxFileName::Mkdir(where+sep+"temp");
		if (where.EndsWith(sep)) where.RemoveLast();
		wxFile file(where+sep+"temp"+sep+"desc.ini",wxFile::write);
		if (!file.IsOpened()) {
			wxMessageBox(spanish?"No se pudo crear el archivo desc.ini (debe tener\npermisos de escritura en el directorio seleccionado).":"Couldn't create desc.ini file (you must\nhave write privilege in the origin folder)."); 
			_return;
		}
		wxFileOutputStream fos(file);
		fos.Write(desc_text.c_str(),desc_text.Len());
		fos.Close();
		file.Close();

		step=STEP_BUILDING;
		if (!CreateZip(callback_create,dest->GetValue(),folder->GetValue(),files)) { 
			wxMessageBox(spanish?"Ha ocurrido un error durante la compresión (CreateZip).":"There's been a error while creating the complement file (CreateZip)."); 
			_return;
		}
		if (step==STEP_ABORTING) { _return; }
		
		Notify(spanish?"Generación Finalizada.":"Generation completed.");
		step=STEP_DONE; but_create->SetLabel(spanish?"Aceptar":"Ok"); but_cancel->SetLabel(spanish?"Cerrar":"Close"); but_create->Enable();
	}
}

void mxCreateComplementWindow::OnButtonCancel (wxCommandEvent & evt) {
	if (step==STEP_ASKING || step==STEP_DONE) Close();
	else step=STEP_ABORTING;
}

void mxCreateComplementWindow::OnClose (wxCloseEvent & evt) {
	Destroy();
}

void mxCreateComplementWindow::Notify(const wxString &message) {
	text->AppendText(message+"\n");
	int i=text->GetValue().Len()-1;
	text->SetSelection(i,i);
	wxYield();
}

void mxCreateComplementWindow::OnButtonFolder (wxCommandEvent & evt) {
	wxDirDialog dlg(this,_T(spanish?"Directorio con archivos para el complemento:":"Directory with files to be included:"),folder->GetValue());
	if (wxID_OK==dlg.ShowModal()) SetFolder(dlg.GetPath());
}

void mxCreateComplementWindow::OnButtonDest (wxCommandEvent & evt) {
	wxFileDialog dlg(this,spanish?"Archivo a generar:":"File to generate:","",dest->GetValue(),"*.zcp;*.ZCP",wxFD_SAVE);
	if (wxID_OK==dlg.ShowModal()) { dest->SetValue(dlg.GetPath()); }
}

int mxCreateComplementWindow::GetStep ( ) {
	return step;
}

void mxCreateComplementWindow::SetFolder (wxString where) {
	info=complement_info();
	folder->SetValue(where);
	if (where.EndsWith(sep)) where.RemoveLast();
	if (wxFileName::FileExists(where+sep+"temp"+sep+"desc.ini")) {
		wxFile file(where+sep+"temp"+sep+"desc.ini");
		wxFileInputStream fis(file);
		wxString desc;
		wxStringOutputStream sos(&desc);
		fis.Read(sos);
		desc_split(desc,info);
		text_en->SetValue(info.desc_english);
		text_es->SetValue(info.desc_spanish);
		close->SetValue(info.closereq);
		reset->SetValue(info.resetreq);
		version->SetValue(wxString()<<info.reqver);
		dest->SetValue(folder->GetValue()+".zcp");
	}
}

