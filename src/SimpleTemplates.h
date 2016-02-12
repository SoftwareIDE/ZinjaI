#ifndef SIMPLETEMPLATES_H
#define SIMPLETEMPLATES_H
#include <wx/string.h>
#include <wx/arrstr.h>
#include <map>
#include "SingleList.h"

class SimpleTemplates {
	struct TemplateInfo {
		wxString file_name; ///< just filename (no dir, can be in zinjai/templates, or in $HOME/.zinjai/templates)
		wxString userf_name; ///< user-friendly name for that template (Name field in template's header)
		bool cpp; ///< true (default) if it is a cpp template, false if it is a C one (Type field in template's header)
		wxString options; ///< compiler arguments (without ${DEFAULT}, Options field in template's header)
		int caret_pos; ///< initial position of the caret, (Caret field in template's header)
	};
	SingleList<TemplateInfo> m_templates;
	SimpleTemplates();
public:
	static void Initialize(); ///< singleton

	/** 
	* @brief given a path for a template, retrieves its user-friendly name, 
	*        or an empty string if the path is not valid
	**/
	wxString GetNameFromFile(wxString file);
	
	/** 
	* @brief retrieves the list of templates files, the bool arguments
	*        can be used to filter only cpp o only c templates
	**/
	void GetFilesList(wxArrayString &list, bool c, bool cpp);
	
	/** 
	* @brief retrieves the list of user-friendly names, the bool arguments
	*        can be used to filter only cpp o only c templates
	**/
	void GetNamesList(wxArrayString &list, bool c, bool cpp);
	
	/**
	* @brief returns the options member from the TemplateInfo matching the given
	*        file_name with "${DEFAULT}" replaced
	**/
	wxString GetParsedCompilerArgs(wxString file_name);
	
	/**
	* @brief overload for GetParsedCompilerArgs(wxString) that calls that first 
	*        one with the default c/cpp template
	**/
	wxString GetParsedCompilerArgs(bool for_cpp);
	
	/**
	* @brief parses a template's header, fills a map with the defined options,
	*        and return the number of header's lines
	**/
	int GetOptions(std::map<wxString,wxString> &opts, wxString full_path, bool replace_default=true);
	
	/**
	* @brief given a template file_name returns true if corresponds to a 
	*        C++ template, false if it is a C template
	**/
	bool IsCpp(wxString file_name);
};

extern SimpleTemplates *g_templates;

#endif

