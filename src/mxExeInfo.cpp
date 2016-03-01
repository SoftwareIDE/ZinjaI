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
#include "mxCommonConfigControls.h"
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxExeInfo, wxDialog)
	EVT_BUTTON(wxID_OK,mxExeInfo::OnCloseButton)
	EVT_BUTTON(mxID_EXEINFO_STRIP,mxExeInfo::OnStripButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxExeInfo::OnHelpButton)
	EVT_BUTTON(wxID_FIND,mxExeInfo::OnLocationButton)
	EVT_TIMER(wxID_ANY,mxExeInfo::OnTimer)
END_EVENT_TABLE()
	
mxExeInfo::mxExeInfo(wxWindow *parent, ei_mode mode, wxFileName fname, mxSource *src) 
	: mxDialog(parent, LANG(EXEINFO_CAPTION,"Propiedades del Ejecutable")), 
	  m_mode(mode), m_source(src), m_fname(fname), m_wait_for_parser(GetEventHandler(),wxID_ANY)
{
	if (mode==mxEI_PROJECT||mode==mxEI_SIMPLE) {
		if (!m_fname.FileExists() || (src&&src->sin_titulo&&compiler->last_compiled!=src) ) {
			mxMessageDialog(LANG(EXEINFO_NOT_FOUND,"No se encontro ejecutable. Compile el programa para crearlo."),LANG(GENERAL_WARNING,"Aviso"),mxMD_OK).ShowModal();
			Close();
			return;
		}
	}
	SetSize(wxSize(450,400));
	CreateSizer sizer(this);
	sizer.BeginNotebook()
		.AddPage(this,&mxExeInfo::CreateGeneralPanel, LANG(EXEINFO_GENERAL,"General") )
		.AddPageIf(m_mode!=mxEI_SOURCE,this,&mxExeInfo::CreateDependPanel, LANG(EXEINFO_DEPS,"Dependencias") )
		.EndNotebook();
	
	if (mode==mxEI_PROJECT||mode==mxEI_SIMPLE) {
		sizer.BeginBottom()
			.Extra(mxID_EXEINFO_STRIP,LANG(EXEINFO_STRIP,"&Strip"),bitmaps->buttons.cancel)
			.Help().Close().EndBottom(this);
	} else {
		sizer.BeginBottom().Close().EndBottom(this);
		ldd_ctrl = nullptr;
	}
	
	if (!parser->working) UpdateTypeAndDeps();
	else m_wait_for_parser.Start(1000,false);
	
	sizer.Set();
	SetFocus();
	Show();
}

wxPanel *mxExeInfo::CreateGeneralPanel (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	sizer.BeginText( LANG(EXEINFO_LOCATION,"Ubicacion") )
		.Button(wxID_FIND).Value(m_fname.GetFullPath()).ReadOnly().EndText();

	double fsize = m_fname.GetSize().ToDouble();
	wxString tsize;
	if (fsize>1024) {
		if (fsize>1024*1024) {
			tsize<<int((fsize/1024/1024))<<"."<<int(fsize/1024/1.024)%1000<<" MB = ";
		} else
			tsize<<int(fsize/1024)<<"."<<int(fsize/1.024)%1000<<" KB = ";
	}
	tsize<<m_fname.GetSize()<<" B";
	
	sizer.BeginText( LANG(EXEINFO_SIZE,"Tamaño") ).Value(tsize).Short().ReadOnly().EndText(text_size);
	sizer.BeginText( LANG(EXEINFO_LAST_MOD_DATE,"Fecha última modifiación") ).Short()
		.Value(m_fname.GetModificationTime().Format("%H:%M:%S - %d/%B/%Y")).ReadOnly().EndText(text_time);

	bool show_updated = false, updated;
	if (m_mode==mxEI_PROJECT) {
		show_updated = true;
		updated = !project->PrepareForBuilding();
	} else if (m_mode==mxEI_SIMPLE) {
		wxFileName bin = m_source->GetBinaryFileName();
		show_updated = !m_source->sin_titulo;
		updated = show_updated && !m_source->GetModify() && bin.FileExists() &&
			      bin.GetModificationTime()>=m_fname.GetModificationTime();
		if (updated && config->Running.check_includes)
			updated =  mxUT::AreIncludesUpdated(bin.GetModificationTime(),m_fname);
	} else if (m_mode==mxEI_SOURCE) {
		sizer.BeginText( LANG(EXEINFO_IS_OPEN,"Abierto") ).Short()
			.Value(m_source?LANG(EXEINFO_YES,"Si"):LANG(EXEINFO_NO,"No")).ReadOnly().Short().EndText();
		if (project) {
			project_file_item *fitem = project->FindFromFullPath(m_fname.GetFullPath());
			sizer.BeginText( LANG(EXEINFO_IN_PROJECT,"En proyecto") )
				.Value( fitem
					      ? (fitem->where==FT_SOURCE
						       ? LANG(EXEINFO_IN_SOURCE,"en Fuentes")
							   : ( fitem->where==FT_HEADER
								     ? LANG(EXEINFO_IN_HEADER,"en Cabeceras")
								     : LANG(EXEINFO_IN_OTHER,"en Otros") ) )
					      : LANG(EXEINFO_NO,"No") )
				.Short().ReadOnly().Short().EndText();
		}
		if (m_source && !m_source->sin_titulo) 
			sizer.BeginText( LANG(EXEINFO_MODIFIED,"Modificado") )
				.Value(m_source->GetModify()?LANG(EXEINFO_YES,"Si"):LANG(EXEINFO_NO,"No"))
				.Short().ReadOnly().Short().EndText();
	}
	if (show_updated)
		sizer.BeginText( LANG(EXEINFO_UPDATED,"Actualizado") )
			.Value(updated?LANG(EXEINFO_YES,"Si"):LANG(EXEINFO_NO,"No"))
			.Short().ReadOnly().EndText();
	
	wxString file_type = LANG(EXEINFO_WAIT_FOR_PARSER,"No se puede determinar el tipo mientras el parser esta analizando fuentes");
	
	sizer.BeginText( LANG(EXEINFO_FILE_TYPE,"Tipo de Archivo") ).Value(file_type).MultiLine().ReadOnly().EndText(text_type);
	
	sizer.Set();
	return sizer.GetPanel();
}

wxPanel *mxExeInfo::CreateDependPanel (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
	sizer.BeginText( _if_win32("lsdeps","ldd") )
		.Value( LANG(EXEINFO_WAIT_FOR_PARSER,"No se puede determinar esta informacion mientras el parser esta analizando fuentes") )
		.ReadOnly().MultiLine().EndText(ldd_ctrl);
	sizer.Set();
	return sizer.GetPanel();
}

void mxExeInfo::OnCloseButton(wxCommandEvent &evt) {
	Close();
}

void mxExeInfo::OnStripButton(wxCommandEvent &evt) {
	mxUT::GetOutput(wxString("strip \"")<<m_fname.GetFullPath()<<"\"");
	wxString tsize;
	double fsize = m_fname.GetSize().ToDouble();
	if (fsize>1024) {
		if (fsize>1024*1024) {
			tsize<<int((fsize/1024/1024))<<"."<<int(fsize/1024/1.024)%1000<<" MB = ";
		} else
			tsize<<int(fsize/1024)<<"."<<int(fsize/1.024)%1000<<" KB = ";
	}
	tsize<<m_fname.GetSize()<<" B";
	text_size->SetValue(tsize);	
	text_type->SetValue(mxUT::GetFileTypeDescription(m_fname.GetFullPath()));
	text_time->SetValue(m_fname.GetModificationTime().Format("%H:%M:%S - %d/%B/%Y"));
}

void mxExeInfo::OnHelpButton(wxCommandEvent &event){
	mxHelpWindow::ShowHelp("exe_info.html");
}

void mxExeInfo::OnTimer(wxTimerEvent &evt) {
	if (parser->working) { return; }
	m_wait_for_parser.Stop();  
	UpdateTypeAndDeps();
}

void mxExeInfo::UpdateTypeAndDeps ( ) {
	if (ldd_ctrl) {
		wxString ldd_cmd = OSDEP_VAL( DIR_PLUS_FILE(config->zinjai_bin_dir,"lsdeps.exe"), "ldd" );
		ldd_ctrl->SetValue(mxUT::GetOutput(ldd_cmd+" "+mxUT::Quotize(m_fname.GetFullPath()),true,false));
	}
	text_type->SetValue(mxUT::GetFileTypeDescription(m_fname.GetFullPath()));
}

void mxExeInfo::RunForProject (wxWindow * parent) {
	new mxExeInfo(parent,mxEI_PROJECT,project->GetExePath(false));
}

void mxExeInfo::RunForSimpleProgram (wxWindow * parent, mxSource * source) {
	new mxExeInfo(parent,mxEI_SIMPLE,source->GetBinaryFileName(),source);
}

void mxExeInfo::RunForSource (wxWindow * parent, mxSource * source) {
	new mxExeInfo(parent,mxEI_SOURCE,source->GetFullPath(),source);
}

void mxExeInfo::RunForSource (wxWindow * parent, wxFileName fname) {
	new mxExeInfo(parent,mxEI_SOURCE,fname,main_window->FindSource(fname));
}

void mxExeInfo::OnLocationButton (wxCommandEvent & evt) {
	
}

