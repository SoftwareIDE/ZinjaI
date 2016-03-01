#include "mxMainWindow.h"
#include "mxSourceProperties.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "mxUtils.h"
#include "mxSource.h"
#include "Parser.h"
#include "mxCommonConfigControls.h"

BEGIN_EVENT_TABLE(mxSourceProperties, wxDialog)
	EVT_BUTTON(wxID_OK,mxSourceProperties::OnOkButton)
END_EVENT_TABLE()

mxSourceProperties::mxSourceProperties(wxString path, mxSource *src)
	: mxDialog(main_window, "Propiedades del Archivo" ), m_fname(path)
{
	SetSize(wxSize(450,400));
#warning FALTA TRADUCIR
#warning AGREGAR BOTON PARA ABRIR CARPETA
#warning AGREGAR DEPENDENCIAS???
	
	CreateSizer sizer(this);

	wxFileName filename(m_fname);
	filename.Normalize();
	
	double fsize = filename.GetSize().ToDouble();
	wxString tsize;
	if (fsize>1024) {
		if (fsize>1024*1024) {
			tsize<<int((fsize/1024/1024))<<"."<<int(fsize/1024/1.024)%1000<<" MB = ";
		} else
			tsize<<int(fsize/1024)<<"."<<int(fsize/1.024)%1000<<" KB = ";
	}
	tsize<<filename.GetSize()<<" B";
	
	sizer.BeginText( "Ubicacion" ).Value(filename.GetFullPath()).ReadOnly().EndText();
	sizer.BeginText( "Tamaño" ).Short().Value(tsize).ReadOnly().EndText();
	sizer.BeginText( "Fecha ultima modifiacion" )
		.Value(filename.GetModificationTime().Format("%H:%M:%S - %d/%B/%Y"))
		.Short().ReadOnly().EndText();
	
	if (!src) src=main_window->FindSource(m_fname);
	sizer.BeginText( "Abierto" ).Value(src?"Si":"No").ReadOnly().Short().EndText();
	if (src) sizer.BeginText( "Modificado" ).Value(src->GetModify()?"Si":"No").ReadOnly().Short().EndText();
	
	if (!m_source || !m_source->sin_titulo) {
		bool updated=true;
		if (m_source) {
			if (m_source->GetModify() || !m_source->GetBinaryFileName().FileExists() || m_source->GetBinaryFileName().GetModificationTime()<m_source->source_filename.GetModificationTime())
				updated=false;
			else if (config->Running.check_includes && mxUT::AreIncludesUpdated(m_source->GetBinaryFileName().GetModificationTime(),m_source->source_filename))
				updated=false;
		} else {
			if (project->PrepareForBuilding())
				updated=false;
		}
		sizer.BeginText( LANG(EXEINFO_UPDATED,"Actualizado") )
			.Value(updated?LANG(EXEINFO_YES,"Si"):LANG(EXEINFO_NO,"No"))
			.ReadOnly().EndText();
	}
	
	
	wxString file_type = mxUT::GetFileTypeDescription(m_fname);
	
	sizer.BeginText( "Tipo de Archivo" ).Value(file_type).ReadOnly().MultiLine().EndText();
	
	sizer.BeginBottom().Close().EndBottom(this).Set();
	SetFocus();
	Show();
}

void mxSourceProperties::OnOkButton(wxCommandEvent &event) {
	Close();
}
