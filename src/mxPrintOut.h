#ifndef MXPRINTOUT_H
#define MXPRINTOUT_H
#include<wx/print.h>

class mxSource;

extern wxPrintDialogData *printDialogData;
//extern wxPageSetupDialogData *pageSetupData;

/**
* @brief Gestiona la impresi�n de un fuente
* 
* Gestiona la impresi�n de un fuente utilizando la infraestructura de impresi�n
* de wxWidgets. Se encarga de calcular la cantidad de paginas, donde comienza
* cada una, y dialogar con el mxSource para renderizarlas en el dc.
* el codigo de esta clase esta basado en el ejemplo de stc que viene con 
* wxWidgets, aunque tiene varias modificaciones.
**/
class mxPrintOut: public wxPrintout {
	
public:
	
	//! constructor
	mxPrintOut (mxSource *source, wxString title = "");
	//! destructor
	~mxPrintOut();
	
	//! evento: imprimir una pagina
	bool OnPrintPage (int page);
	//! evento: inicializar la impresi�n
	bool OnBeginDocument (int startPage, int endPage);
	
	//! determinar si existe una p�gina (fake)
	bool HasPage (int page);
	//! evento: recuperar informaci�n de la impresi�n
	void GetPageInfo (int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
	
	//! guarda en que indice del texto fuente comienza una determinada pagina
	void SetPageStart(int page, int start);
	
private:
	mxSource *source; ///< fuente a imprimir
	int m_printed; ///< variable auxiliar para determinar hasta donde se "imprimi�" en GetPageInfo
	int *pages; ///< guarda en que indice del texto fuente comienza cada pagina
	int pages_len; ///< tama�o en memoria del arreglo de indices de comienzo para cada pagina
	wxRect m_pageRect; ///< area de la p�gina completa
	wxRect m_printRect; ///< area de impresi�n
	
	bool PrintScaling (wxDC *dc);
};

#endif

