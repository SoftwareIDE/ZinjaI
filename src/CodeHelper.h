#ifndef CODE_HELPER_H
#define CODE_HELPER_H 

#include <wx/string.h>
#include <wx/arrstr.h>

//#define CODE_HELP_MAP(c) (c>='a' && c<='z')?c-'a':((c>='A' && c<='Z')?c-'A':26)

class mxSource;
class GenericAction;

#include <iostream>
#include "Cpp11.h"
using namespace std;


class MyAutocompList {
	wxArrayString keywords;
	wxArrayString helps;
	wxString result;
	unsigned int max_len/*, count*/;
public:
	bool Empty() { 
		return keywords.GetCount()==0;
	}
	void Init() {
		max_len=0;
		keywords.Clear();
		helps.Clear();
	}
	void Add(const wxString &keyword, const wxString &icon, const wxString &help);
	const wxString &GetResult(bool sort=true, int n=-1);
	wxString GetHelp(int sel);
	unsigned int GetMaxLen() { 
		return max_len;
	}
	const wxString &GetLastResult() { return result; }
	wxString GetFiltered(wxString keyword);
	bool Contains(const wxString &key) {
		for(unsigned int i=0;i<keywords.GetCount();i++) 
			if (keywords[i].StartsWith(key) && (keywords[i].Len()==key.Len()||keywords[i][key.Len()]=='$'))
				return true;
		return false;
	}
};
extern MyAutocompList g_autocomp_list; /// @todo: move this inside CodeHelper


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
	CodeHelper(int ml, int mode);
public:
	static void Initialize();
	bool LoadData(wxString index);
	void ResetStdData();
	wxString GetInclude(wxString path, wxString key);
	wxString GetIncludeForClass(wxString path, wxString key);
	bool AutocompleteAutocode(mxSource *source, wxString key/*, int max_str_dist=3*/);
	bool AutocompleteGeneral(mxSource *source, wxString scope, wxString key, wxString *args=nullptr, int scope_start=-1);
	void AutocompleteLocals(mxSource *source, wxString key, int scope_start);
	bool AutocompleteScope(mxSource *source, wxString &key, wxString typed, bool consider_inherit, bool add_reserved_words/*, int max_str_dist=3*/);
	wxString GetCalltip(wxString scope, wxString key, bool onlyScope, bool only_type=false);
	bool ShowFunctionCalltip(int p, mxSource *source, wxString scope, wxString key, bool onlyScope=true);
	bool ShowConstructorCalltip(int p, mxSource *source, wxString name);
	wxString GetAttribType(wxString &scope, wxString &key, int &dims);
	wxString GetGlobalType(wxString &key, int &dims);
	wxString UnMacro(wxString name);
	wxString UnMacro(wxString name,int &dims);
	void UnTemplate(wxString &var);
	void AddReservedWords(wxString &typed/*, int max_str_dist=3*/);
	bool AutocompleteFromArray(mxSource *source, CodeHelperSpecialArray &words, wxString typed);
	bool AutocompletePreprocesorDirective(mxSource *source, wxString typed="");
	bool AutocompleteDoxygen(mxSource *source, wxString typed="");
	bool GenerateAutocompletionIndex(wxString path, wxString filename);
	
	void FilterAutocomp(mxSource *source, const wxString &key, bool force_show=false);
	
	//! Recarga los indices de autocompletado
	void ReloadIndexes(wxString indexes);
	
	//! Agrega indices de autocompletado
	void AppendIndexes(wxString indexes);
	
	void TryToSuggestTemplateSolutionForLinkingErrors(const wxArrayString &full_output, GenericAction *&on_end);
	
	/// @param mode  0=case-sensitive start, 1=case-insesitive start, 2=case-insensitive any, 3=case-insensitive Levenshtein
	void SetAutocompletionMatchingMode(int mode);
	
	class RAIAutocompModeChanger {
	public:
		RAIAutocompModeChanger();
		void Change(int mode);
		~RAIAutocompModeChanger();
	};
	
};

extern CodeHelper *g_code_helper;

#endif


