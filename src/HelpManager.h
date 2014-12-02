#ifndef HELP_MANAGER_H
#define HELP_MANAGER_H

#include "mxUtils.h" // HashStringString

class pd_func;
class pd_var;
class pd_macro;
class pd_class;

/**
* @brief Encapsula la generación de ayuda rápida.
* 
* Encapsula la generación de ayuda rápida. Carga y almacena los índices estándar, 
* y dialoga con el parser para generar sobre la marcha los items relacionados a 
* los componentes del código del usuario.
**/
class HelpManager {
private:
	unsigned int index_ref_counter;
	HashStringString doxy_index;
	wxDateTime doxy_index_stamp;
	bool ReloadDoxyIndex();
	wxString GetDoxyInfo(pd_class *aclass, wxString &desc); ///< busca info en la doc de doxygen para una clase dada
	wxString GetDoxyInfo(pd_func *afunc, wxString &desc); ///< busca info en la doc de doxygen para un prototipo dado
	wxString ParseDoxyText(wxString link, wxString &desc); ///< arma el prototipo y extrae la descripcion desde el html
	HelpManager();
public:
	static void Initialize();
	wxString GetQuickHelp(wxString keyword);
	bool IsHelpForType(wxString what, wxString &link);
	void HelpFor(pd_func *afunc, wxString &content, wxString &index);
	void HelpFor(pd_var *avar, wxString &content, wxString &index);
	void HelpForEnum(pd_var *avar, wxString &content, wxString &index);
	void HelpFor(pd_macro *amacro, wxString &content, wxString &index);
	void HelpFor(pd_class *aclass, wxString &content, wxString &index);
	int GetParserHelp(wxString keyword, wxString &content);
	wxString MakeClassLinks(wxString what);
};

extern HelpManager *help;

#endif
