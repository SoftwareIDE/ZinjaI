#ifndef AUTOCODER_H
#define AUTOCODER_H
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/hashmap.h>

/// @brief Handles information about one autocode template, to be used in Autocoder hash table
struct auto_code {
	wxString description; ///< one line description
	wxString code; ///< template code
	wxArrayString args; ///< arguments
	void Clear() { args.Clear(); description.Clear(); code.Clear(); }
};

class mxSource;

WX_DECLARE_STRING_HASH_MAP( auto_code, HashStringAutoCode );

/**
* @brief This class contains methods for loading, parsing, replacing and 
*        whatevering autocodes.
*
* Autocodes works very similar to C++ macros, but are replaced when typing.
* Use TAB key in ZinjaI to invoke the replacement
**/
class Autocoder {
private:
	wxString m_description;
	HashStringAutoCode m_list;
	friend class CodeHelper;
public:
	
	/// @brief intenta cargar las plantillas del usuario, si el archivo no existe lo crea con los defaults
	Autocoder();
	
	/// @brief define default autocode templates
	void SetDefaults();
	
	/// @brief erase all data about autocodes (current ones, description, ...)
	void Clear();
	
	/// @brief clean all data and reload general autocodes (the ones from config, to dismiss project's specific ones)
	void Reset(wxString pfile);
	
	/// @brief loads definitions from a text file, without erasing current ones (but they might be replaced)
	bool LoadFromFile(wxString filename="");
	
	/// @brief saves current autocodes definitions to a text file
	void SaveToFile(wxString filename="");
	
	/// @brief realiza el reemplazo, el texto a reemplazar viene dado por TargetStart y TargetEnd del mxSource
	bool Apply(mxSource *src, auto_code *ac, bool args);
	
	/// @brief analiza si se debe reemplazar algo o no, en caso afirmativo llama al otro Apply para hacer el reemplazo
	bool Apply(mxSource *src);
	
};

extern Autocoder *g_autocoder;

#endif

