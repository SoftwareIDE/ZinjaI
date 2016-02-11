#ifndef LANGUAGE_H
#define LANGUAGE_H
#include <string>
#include "lang_index_macros.h"
using namespace std;

#define LANG(key,text) (g_language_buffer?g_language_buffer+g_language_index[LANG_##key]:text)

/// Codigos de error para las funciones que administran el archivo y buffer de lenguaje
enum LANGUAGE_ERROR { 
	LANGERR_OK, ///< el lenguaje se cargó o regeneró sin problemas
	LANGERR_NO_CACHE_NAME, ///< no se especifico archivo de cache
	LANGERR_CACHE_NOT_FOUND, ///< no se encontro el archivo de cache
	LANGERR_SIGN_DIFFER, ///< las firmas del archivo de lenguaje y el archivo de cache no coinciden
	LANGERR_SIGN_NOT_FOUND, ///< no se encontro el archivo de firma
	LANGERR_PRE_NOT_FOUND, ///< no se encontro el archivo de entrada para construir buffer y cache
	LANGERR_PRE_WRONG_COUNT, ///< el conteo del archivo de entrada no coincide con el esperado
	LANGERR_RECOMPILED, ///< el cache se regenró con exito
	LANGERR_CACHE_OK ///< el archivo de cache se encuentra y su firma es correcta
};

extern char *g_language_buffer;
extern int *g_language_index;

LANGUAGE_ERROR load_language(string lang_in, string lang_cache);
LANGUAGE_ERROR compile_language(string lang_in, string lang_cache);
LANGUAGE_ERROR is_language_compiled(string lang_in, string lang_cache);

#endif

