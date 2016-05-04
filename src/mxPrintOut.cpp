#include <wx/printdlg.h>
#include "mxPrintOut.h"
#include "mxSource.h"
#include "mxMessageDialog.h"
#include "mxMainWindow.h"

wxPrintDialogData *g_printDialogData = nullptr;
//wxPageSetupDialogData *pageSetupData = nullptr;

/**
* @brief Gestiona la impresión de un fuente
* 
* Gestiona la impresión de un fuente utilizando la infraestructura de impresión
* de wxWidgets. Se encarga de calcular la cantidad de paginas, donde comienza
* cada una, y dialogar con el mxSource para renderizarlas en el dc.
* el codigo de esta clase esta basado en el ejemplo de stc que viene con 
* wxWidgets, aunque tiene varias modificaciones.s
**/
mxPrintOut::mxPrintOut (mxSource *src, wxString title) : wxPrintout(title) {
//	if (!printData) printData=new wxPrintData;
//	if (!pageSetupData) {
//		pageSetupData=new wxPageSetupDialogData;
////		wxPageSetupDialog pageSetupDialog(nullptr);
////		*printData = pageSetupDialog.GetPageSetupData().GetPrintData();
////		*pageSetupData = pageSetupDialog.GetPageSetupData();	
//	}
	m_source = src;
	m_pages_len = m_printed = 0;
	m_pages=nullptr;
}

bool mxPrintOut::OnPrintPage (int page) {
	wxDC *dc = GetDC();
	if (!dc) return false;
	PrintScaling (dc);
	m_source->FormatRange (1, m_pages[page-1], m_source->GetLength(),
		dc, dc, m_printRect, m_pageRect);
	return true;
}

bool mxPrintOut::OnBeginDocument (int startPage, int endPage) {
	if (!wxPrintout::OnBeginDocument (startPage, endPage))
		return false;
	m_printed=0;
	return true;
}

void mxPrintOut::GetPageInfo (int *minPage, int *maxPage, int *selPageFrom, int *selPageTo) {
	
	// initialize values
	*minPage = *maxPage = *selPageFrom = *selPageTo = 0;
	
	// scale DC if possible
	wxDC *dc = GetDC();
	if (!dc) return;
	PrintScaling (dc);
	
	// get print page informations and convert to printer pixels
	wxSize ppiScr;
	GetPPIScreen (&ppiScr.x, &ppiScr.y);
//	wxSize page = pageSetupData->GetPaperSize();
//	wxSize page = dc->GetSize();
	wxSize page; GetPageSizeMM(&page.x,&page.y);
	page.x = static_cast<int> (page.x * ppiScr.x / 25.4);
	page.y = static_cast<int> (page.y * ppiScr.y / 25.4);
	m_pageRect = wxRect (0,0,page.x,page.y);
	
	// get margins informations and convert to printer pixels
//	wxPoint pt = pageSetupData->GetMarginTopLeft();
//	int left = pt.x;
//	int top = pt.y;
//	pt = pageSetupData->GetMarginBottomRight();
//	int right = pt.x;
//	int bottom = pt.y;
	int print_margin=10;
	int top=print_margin,left=print_margin,right=print_margin,bottom=print_margin;
	
	top = static_cast<int> (top * ppiScr.y / 25.4);
	bottom = static_cast<int> (bottom * ppiScr.y / 25.4);
	left = static_cast<int> (left * ppiScr.x / 25.4);
	right = static_cast<int> (right * ppiScr.x / 25.4);
	
	m_printRect = wxRect (left,
		top,
		page.x - (left + right),
		page.y - (top + bottom));
	
	// count pages
	while (HasPage (*maxPage)) {
		int last = m_printed;
		SetPageStart(*maxPage,m_printed);
		m_printed = m_source->FormatRange (0, m_printed, m_source->GetLength(), dc, dc, m_printRect, m_pageRect);
		*maxPage += 1;
		if (last==m_printed) {
			mxMessageDialog(main_window,"Debe configurar la pagina antes de imprimir")
				.Title(LANG(GENERAL_ERROR,"Error")).IconWarning().Run();
			break;
		}
	}
	if (*maxPage > 0) *minPage = 1;
	*selPageFrom = *minPage;
	*selPageTo = *maxPage;
}

bool mxPrintOut::HasPage (int page) {
	return (m_printed < m_source->GetLength());
}

bool mxPrintOut::PrintScaling (wxDC *dc){
	
	// check for dc, return if none
	if (!dc) return false;
	
	// get printer and screen sizing values
	wxSize ppiScr;
	GetPPIScreen (&ppiScr.x, &ppiScr.y);
	if (ppiScr.x == 0) { // most possible guess 96 dpi
		ppiScr.x = 96;
		ppiScr.y = 96;
	}
	wxSize ppiPrt;
	GetPPIPrinter (&ppiPrt.x, &ppiPrt.y);
	if (ppiPrt.x == 0) { // scaling factor to 1
		ppiPrt.x = ppiScr.x;
		ppiPrt.y = ppiScr.y;
	}
	wxSize dcSize = dc->GetSize();
	wxSize pageSize;
	GetPageSizePixels (&pageSize.x, &pageSize.y);
	
	// set user scale
	float scale_x = (float)(ppiPrt.x * dcSize.x) /
		(float)(ppiScr.x * pageSize.x);
	float scale_y = (float)(ppiPrt.y * dcSize.y) /
		(float)(ppiScr.y * pageSize.y);
	dc->SetUserScale (scale_x, scale_y);
	
	return true;
}

/**
* Recibe un numero de pagina y un entero con la posición dentro del fuente
* en donde comienza la impresión de esa página, y los alamcena en una arreglo
* para utilizarlo en OnPrintPage. Maneja además la memoria del arreglo donde
* se guarda esta información redimensionando e incializando cuando es necesario.
* Este método es llamado por GetPageInfo.
* @param page numero de la pagina (wx las maneja en base 1, y asi se guardan)
* @param start indice dentro del texto fuente (buffer del mxSource) donde 
*        comienza esta página
**/
void mxPrintOut::SetPageStart(int page, int start) {
	if (!m_pages) m_pages=new int[m_pages_len=10];
	if (page>=m_pages_len) {
		int *p2=new int[m_pages_len*2];
		for (int i=0;i<m_pages_len;i++)
			p2[i]=m_pages[i];
		delete [] m_pages;
		m_pages=p2;
		m_pages_len=m_pages_len*2;
	}
	m_pages[page]=start;
}

mxPrintOut::~mxPrintOut() {
	if (m_pages) delete []m_pages;
}
