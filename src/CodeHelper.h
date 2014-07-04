#ifndef CODE_HELPER_H
#define CODE_HELPER_H 

#include <wx/string.h>
#include <wx/arrstr.h>

//#define CODE_HELP_MAP(c) (c>='a' && c<='z')?c-'a':((c>='A' && c<='Z')?c-'A':26)

class mxSource;

#include <iostream>
using namespace std;


class MyAutocompList {
	wxArrayString keywords;
	wxArrayString helps;
	unsigned int max_len, count;
public:
	bool Empty() { 
		return count==0;
	}
	void Init() {
		max_len=count=0;
		keywords.Clear();
		helps.Clear();
	}
	void Add(const wxString &keyword, const wxString &icon, const wxString &help);
	wxString GetResult(bool sort=true);
	wxString GetHelp(unsigned int sel);
	unsigned int GetMaxLen() { 
		return max_len;
	}
};
extern MyAutocompList autocomp_list;


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
	struct CodeHelperSpecialArray {
		wxArrayString keywords;
		wxString icon, help;
	};
	CodeHelperSpecialArray reserved_words;
	CodeHelperSpecialArray preproc_directives;
	CodeHelperSpecialArray doxygen_directives;
public:
	CodeHelper(int ml);
	~CodeHelper();	
	bool LoadData(wxString index);
	void ResetStdData();
	wxString GetInclude(wxString path, wxString key);
	wxString GetIncludeForClass(wxString path, wxString key);
	bool AutocompleteAutocode(mxSource *source, wxString key, int max_str_dist=3);
	bool AutocompleteGeneral(mxSource *source, wxString scope, wxString key, wxString *args=NULL, int max_str_dist=3);
	bool AutocompleteScope(mxSource *source, wxString &key, wxString typed, bool consider_inherit, bool add_reserved_words, int max_str_dist=3);
	wxString GetCalltip(wxString scope, wxString key, bool onlyScope, bool only_type=false);
	bool ShowFunctionCalltip(int p, mxSource *source, wxString scope, wxString key, bool onlyScope=true);
	bool ShowConstructorCalltip(int p, mxSource *source, wxString name);
	wxString GetAttribType(wxString &scope, wxString &key, int &dims);
	wxString GetGlobalType(wxString &key, int &dims);
	wxString UnMacro(wxString name);
	wxString UnMacro(wxString name,int &dims);
	void UnTemplate(wxString &var);
	void AddReservedWords(wxString &typed, int max_str_dist=3);
	bool AutocompleteFromArray(mxSource *source, CodeHelperSpecialArray &words, wxString typed);
	bool AutocompletePreprocesorDirective(mxSource *source, wxString typed="");
	bool AutocompleteDoxygen(mxSource *source, wxString typed="");
	bool GenerateAutocompletionIndex(wxString path, wxString filename);
	
	//! Recarga los indices de autocompletado
	void ReloadIndexes(wxString indexes);
	
	//! Agrega indices de autocompletado
	void AppendIndexes(wxString indexes);
	
	void TryToSuggestTemplateSolutionForLinkingErrors(const wxArrayString &full_output, bool for_running);
	
};

extern CodeHelper *code_helper;

#endif
