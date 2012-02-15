#ifndef CODE_HELPER_H
#define CODE_HELPER_H 

#include <wx/string.h>
#include <wx/textfile.h>

//#define CODE_HELP_MAP(c) (c>='a' && c<='z')?c-'a':((c>='A' && c<='Z')?c-'A':26)

class mxSource;

/**
* @brief encapsula las funciones relacionadas al autocompletado y los calltips
* 
* Esta clase funciona como interfáz para la información relacionada al autocompletado, 
* los calltips, etc. Dicha información, la tiene en su mayoría el parser, pero
* esta clase encapsula la comunicación con el parser para estos fines particulares
* y genera ella los menues de autocompletado y los globos de calltips. Se encarga
* además de cargar y procesar los cachés de autocompletados de funciones "estándar"
* definidos por el usuario.
**/
class CodeHelper {
private:
	int min_len;
	wxArrayString actual_indexes; ///< la lista de indices de autocompletado cargados actualmente
	wxArrayString comp_array;
	wxArrayString reserved_words;
	wxArrayString preproc_directives;
	wxArrayString doxygen_directives;
public:
	CodeHelper(int ml);
	~CodeHelper();	
	bool LoadData(wxString index);
	void ResetStdData();
	wxString GetInclude(wxString path, wxString key);
	wxString GetIncludeForClass(wxString path, wxString key);
	bool AutocompleteAutocode(mxSource *source, wxString key, int max_str_dist=3);
	bool AutocompleteGeneral(mxSource *source, wxString scope, wxString key, int max_str_dist=3);
	bool AutocompleteScope(mxSource *source, wxString &key, wxString typed, bool consider_inherit, bool add_reserved_words, int max_str_dist=3);
	bool ShowCalltip(int p, mxSource *source, wxString scope, wxString key, bool onlyScope=true);
	bool ShowConstructorCalltip(int p, mxSource *source, wxString name);
	wxString GetAttribType(wxString &scope, wxString &key, int &dims);
	wxString GetGlobalType(wxString &key, int &dims);
	wxString UnMacro(wxString name);
	wxString UnMacro(wxString name,int &dims);
	void UnTemplate(wxString &var);
	int AddReservedWords(wxArrayString &comp_list, wxString &typed, int max_str_dist=3);
	bool AutocompleteFromArray(mxSource *source, wxArrayString &words, wxString typed);
	bool AutocompletePreprocesorDirective(mxSource *source, wxString typed=_T(""));
	bool AutocompleteDoxygen(mxSource *source, wxString typed=_T(""));
	bool GenerateCacheFile(wxString path, wxString filename);
	
	//! Recarga los indices de autocompletado
	void ReloadIndexes(wxString indexes);
	
	//! Agrega indices de autocompletado
	void AppendIndexes(wxString indexes);
	
};

extern CodeHelper *code_helper;

#endif
