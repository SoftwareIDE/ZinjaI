#ifndef GDBPARSER_H
#define GDBPARSER_H
#include <wx/string.h>


// de inspection matrix

/**
* @brief Saltea una cadena completa
*
* Dada una posición donde comienza una cadena (donde hay un ' o ")
* avanza la posición hasta llegar al final de la misma
**/
void GdbParse_SkipString(const wxString &s, int &i, int l);

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

