#ifndef GDBPARSER_H
#define GDBPARSER_H
#include <wx/string.h>


// de inspection matrix

/**
* @brief Saltea una cadena completa
*
* Dada una posición (i) donde comienza una cadena (donde hay un \' o \")
* avanza la posición hasta llegar al final de la misma (hasta llegar al 
* \' o \" que cierre la cadena, allí queda i)
**/
template<typename TString>
void GdbParse_SkipString(const TString &s, int &i, int l) {
	wxChar c=s[i++];
	while (i<l && s[i]!=c) {
		if (s[i]=='\\') i++;
		i++;
	}
}

/**
* @brief Saltea la lista de argumentos del template
*
* Dada una posición(i) donde comienza una lista de argumentos de un template
* (donde hay un <) avanza la posición hasta llegar al final de la misma
* (hasta llegar al > que cierre la cadena, allí queda i)
**/
template<typename TString>
void GdbParse_SkipTemplate(const TString &s, int &i, int l) {
	int template_level=1; ++i;
	while (i+1<l && template_level>0) {
		i++;
		if (s[i]=='<') template_level++;
		else if (s[i]=='>') template_level--;
	}
}

/**
* @brief Saltea una lista completa
*
* Dada una posición donde comienza una lista (donde hay un '{')
* avanza la posición hasta llegar al final de la misma.
* Para determinar los limites de la lista (y sublistas dentro
* de esta), los delimitares validos son {..}, [..] y (..), y
* son todos equivalentes.
**/
void GdbParse_SkipList(const wxString &s, int &i, int l);

/**
* @brief Saltea espacios en blanco
*
* Avanza i mientras sea menor a la longitud de la cadena (l), y 
* los caracteres sean enter, espacio o tab.
**/
void GdbParse_SkipEmpty(const wxString &s, int &i, int l);

/**
* @brief Determina si el siguiente elemento es el comienzo de una lista
*
* A partir de la posición i, busca el primer caracter que no nulo (nulo=enter,
* tab,espacio) y returna true si este es una llave que abre una lisat ('{'), o
* false si es otra cosa, o si se llegó al final de la cadena
**/
bool GdbParse_IsList(const wxString &s, int &i, int l=-1);

bool GdbParse_GetPair(const wxString &s, int &i, int &pos_st, int &pos_eq, int &pos_end, int l=-1);


// de locals grid

bool GdbParse_GetNext(const wxString &s, int &i, wxString &key, wxString &val);

#endif

