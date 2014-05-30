/**
 * La clase mxUtils define funciones varias de utileria para ser usadas
 * desde otras clases. Su instancia es "utils". Ademas define algunas
 * macros comunes para el manejo de listas enlazadas baratas, y alguna
 * que otra constante.
 **/


#ifndef MXUTILS_H
#define MXUTILS_H

#include <wx/string.h>
#include <wx/filename.h>
#include <wx/treebase.h>

#if !defined(_WIN32) && !defined(__WIN32__)
#define WILDCARD_SOURCE "*.cpp;*.c;*.c++;*.cxx;*.CPP;*.C;*.C++;*.CXX;*.cc;*.CC"
#define WILDCARD_HEADER "*.h;*.hpp;*.hxx;*.H;*.HPP;*.HXX;*.hh;*.HH"
#define WILDCARD_PROJECT "*.zpr;*.ZPR"
#define WILDCARD_WXFB "*.fbp;*.FBP"
#else
#define WILDCARD_SOURCE "*.cpp;*.c;*.c++;*.cxx;*.cc"
#define WILDCARD_HEADER "*.h;*.hpp;*.hxx;*.hh"
#define WILDCARD_PROJECT "*.zpr"
#define WILDCARD_WXFB "*.fbp"
#endif
#define WILDCARD_CPP WILDCARD_SOURCE";"WILDCARD_HEADER
#define WILDCARD_CPP_EXT WILDCARD_SOURCE";"WILDCARD_HEADER";"WILDCARD_PROJECT
#define WILDCARD_ALL "*"
#include "enums.h"
#include "Language.h"

class wxBoxSizer;
class wxTextCtrl;
class wxStaticText;
class wxMenuItem;
class wxMenu;
class wxWindow;
class wxCheckBox;
class wxToolBar;
class wxComboBox;
class wxButton;
class wxBitmapButton;

#ifdef DEBUG
	extern wxString debug_string;
	#define DEBUG_QH_SET(lala) debug_string=""; debug_string<<utils->ToHtml(wxString()<<lala)<<"<BR>"; main_window->ShowInQuickHelpPanel(debug_string);
	#define DEBUG_QH_ADD(lala) debug_string<<utils->ToHtml(wxString()<<lala)<<"  "; main_window->ShowInQuickHelpPanel(debug_string);
	#define DEBUG_QH_ADDNL(lala) debug_string<<utils->ToHtml(wxString()<<lala)<<"<BR> "; main_window->ShowInQuickHelpPanel(debug_string);
//	#if defined(_WIN32) || defined(__WIN32__)
//		#include <wx/msgdlg.h>
//		#define DEBUG_INFO(info) wxMessageBox(wxString()<<info);
//	#else
		#include <iostream>
		using namespace std;
		#define DEBUG_INFO(info) cerr<<info<<endl;
//	#endif
#else
	#define DEBUG_INFO(info)
#endif

#define DIR_PLUS_FILE utils->JoinDirAndFile
	
#define LANG1(key,text,arg1) 			utils->ReplaceLangArgs(LANG(key,text),arg1)
#define LANG2(key,text,arg1,arg2) 		utils->ReplaceLangArgs(LANG(key,text),arg1,arg2)
#define LANG3(key,text,arg1,arg2,arg3) 	utils->ReplaceLangArgs(LANG(key,text),arg1,arg2,arg3)

#if defined(_WIN32) || defined(__WIN32__)
#define BINARY_EXTENSION ".exe"
#else
#define BINARY_EXTENSION ".bin"
#endif

WX_DECLARE_STRING_HASH_MAP( wxString, HashStringString );
WX_DECLARE_STRING_HASH_MAP( wxTreeItemId, HashStringTreeItem );

/*
* macros auxiliares para trabajar con mis listas enlazadas made in home
**/
// /** @brief eliminar un item de una lista doblemente enlazada **/
//#define ML_REMOVE(item) if ((item->prev->next=item->next)!=NULL) item->next->prev=item->prev;
// /** @brief while (hay un item mas) **/
//#define ML_WHILE(item) while(item->next!=NULL)
/** @brief avanzar al siguiente item **/
#define ML_NEXT(item) item=item->next;
/** @brief iterar por una lista (de primer elemento ficticio) **/
#define ML_ITERATE(item) while(item->next && (item=item->next))
// /** @brief vaciar una lista (dejando al primer item ficticio) **/
//#define ML_CLEAN(first) if (first->next) { typeof(first) ml_aifc; typeof(first) item=first->next; first->next=NULL; while(item->next) { ml_aifc=item; item=item->next; delete ml_aifc; } delete item; }
// /** @brief eliminar una lista (incluyendo al primer item ficticio) **/
//#define ML_FREE(item) { typeof(item) ml_aifc; while(item->next) { ml_aifc=item; item=item->next; delete ml_aifc; } delete item; }

/**
* @brief Clase para contener funciones varias de uso general
**/
class mxUtils {
public:
	
	/**
	* @name funciones varias
	**/
	/*@{*/
	
	//! Agrega comillas si la cadena tiene un espacio, sino no
	wxString Quotize(const wxString &what);
	
	//! Agrega comillas si la cadena tiene un espacio, coma o punto y coma, pero ademas mira que comilla usar (simple, doble o ambas)
	wxString QuotizeEx(const wxString &what);
	
	//! Agrega comillas simples a una linea de comandos, respetando las comillas anteriores si había
	wxString SingleQuotes(wxString what);
	
	wxString EscapeString(wxString str, bool add_comillas=false);
	
	wxString UnEscapeString(wxString str);
	
	//! Elimina los elementos repetidos de un wxArrayString
	void Purgue(wxArrayString &array, bool sort=true);
	
	//! Compara dos wxArrayString elemento a elemento
	bool Compare(const wxArrayString &array1, const wxArrayString &array2, bool check_case=true);
	
	/** @brief Compara el final de una cadena con otra **/
	bool EndsWithNC(wxString s1, wxString s2);
	
	/** @brief Quita espacios del comienzo de una cadena **/
	wxString LeftTrim(wxString str);
	
	/** @brief Concatena una ruta y un nombre de archivo **/
	wxString JoinDirAndFile(wxString dir, wxString fil);
	
	/** @brief Convierte un path absoluto en relativo **/
	wxString Relativize(wxString fname, wxString path);
	
	/// @brief Concatena una cadena en una linea reemplazando saltos de linea por "\\n" (y escapando las demás '\\')
	wxString Text2Line(const wxString &text);
	/// @brief Divide una linea generada con Text2Line en una cadena en una cadena con varias lineas
	wxString Line2Text(const wxString &line);
	
	// copia una cadena wx en una cadena c
	void strcpy(wxChar *cstr, wxString wstr);
	void strcpy(wxChar *cstr, wxChar *wstr);
	//! Ordena una wxArrayString mediante QuickSort
	void SortArrayString(wxArrayString &array, int inf=-1, int sup=-1);
	// devuelve verdadero si el dato value empieza con '1','V','v','T','t','S' o 's'
	bool ToInt(wxString &value, int &what);
	bool IsTrue(wxString &value);
	wxString UnSplit(wxArrayString &array); ///< joins all strins in array into a single string using a single space as separator
	wxString Split(wxString str, wxString pre);
	int Split(wxString str, wxArrayString &array, bool coma_splits=true,bool keep_quotes=true);
	int Execute(wxString path, wxString command, int sync);
	int Execute(wxString path, wxString command, int sync, wxProcess *&process);
	bool XCopy(wxString src, wxString dst, bool ask=true, bool replace=false);
	//! Ejecuta los comandos entre acentos de una cadena y los reemplaza por su salida (simi Makefile)
	wxString ExecComas(wxString where, wxString line);
	//! Ejecuta un comando de forma sincrónica y devuelve en una cadena su salida
	wxString GetOutput(wxString command, bool also_error=false);
	//! Devuelve una cadena convertida a HTML
	wxString ToHtml(wxString text, bool full=false);
	// reemplaza una cadena por otra, pero agregando comillas si necesita para mantener el parametro como uno solo
	void ParameterReplace(wxString &str, wxString from, wxString to);
//	void MakeDesktopIcon(wxString desk_dir);
	
	/** @brief Distancia entre cadenas **/
	int Levenshtein(const char *s1, int N1, const char *s2, int N2);

	/** @brief Busca el archivo complementario(cpp<->h) **/
	wxString GetComplementaryFile(wxFileName the_one, eFileType force_ext=FT_NULL);
	/*@}*/
	
	
	/**
	* @name para colocar controles en los cuadros de configuracion
	**/
	/*@{*/
	wxStaticText *last_label; ///< guarda la ultima etiqueta que se uso en alguno de los AddAlgo
	wxButton *last_button; ///< guarda el ultimo boton colocado por AddDirCtrl
	wxCheckBox *AddCheckBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool value=false, wxWindowID id = wxID_ANY,bool margin=false);
	wxTextCtrl *AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value="");
	wxTextCtrl *AddLongTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value="");
	wxTextCtrl *AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value="", bool margin=false);
	wxTextCtrl *AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int n, wxString tail, bool margin=false);
	wxTextCtrl *AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int value=0,bool margin=false);
	wxTextCtrl *AddDirCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, wxWindowID id, wxString button_text = "...",bool margin=false);
	wxComboBox *AddComboBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxArrayString &values, int def, wxWindowID = wxID_ANY,bool margin=false);
	wxStaticText* AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text);
	wxStaticText* AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, bool margin=false);
	/*@}*/
	
	/**
	* @name para agregar items a los menues y barras de herramientas
	**/
	/*@{*/
	wxMenuItem *AddItemToMenu(wxMenu *menu, wxWindowID id,wxString caption, wxString accel, wxString help, wxString filename, int where=-1);
	wxMenuItem *AddCheckToMenu(wxMenu *menu, wxWindowID id,wxString caption, wxString accel, wxString help, bool value);
	void AddTool(wxToolBar *toolbar, wxWindowID id, wxString caption, wxString filename, wxString status_text, wxItemKind=wxITEM_NORMAL);
	wxMenuItem *AddSubMenuToMenu(wxMenu *menu, wxMenu *menu_h, wxString caption, wxString help, wxString filename);
	/*@}*/
	
public:
	/**
	* @name para el manejo de "dependencias" (que hs incluye un cpp)
	**/
	/*@{*/
	/** @brief funcion interna que busca "inclusiones" (\#include...) en un fuente **/
	void FindIncludes(wxString path, wxString filename, wxArrayString &already_processed, wxArrayString &header_dirs, bool recursive=true);
	/** @brief Busca las dependencias de un fuente (en realidad del objeto que genera al compilarse, ya que incluye al propio fuente, para el makefile)**/
	wxString FindObjectDeps(wxFileName filename, wxString ref_path, wxArrayString &header_dirs);
	/// @brief Devuelve una lista de includes directos (no recursivo) como wxArrayString (para el grafo del proyecto)
	void FindIncludes(wxArrayString &dest, wxFileName filename, wxString path, wxArrayString &header_dirs);

	wxString GetOnePath(wxString orig_path, wxString project_path, wxString fname, wxArrayString &dirs);
	/// devuelve verdadero si alguno de los archivos incluidos por filename es mas nuevo que bin_date
	bool AreIncludesUpdated(wxDateTime bin_date, wxFileName filename);
	/// devuelve verdadero si alguno de los archivos incluidos por filename es mas nuevo que bin_date
	bool AreIncludesUpdated(wxDateTime bin_date, wxFileName filename, wxArrayString &header_dirs);
	
	/// Busca los programas ejecutados desde zinjai
	void GetRunningChilds(wxArrayString &childs);
	
	/// Abre una pagina en el navegador configurado
	void OpenInBrowser(wxString url);
	
	void OpenZinjaiSite(wxString page="");
	
	/// Abre una carpeta en el explorador de archivos configurado
	void OpenFolder(wxString path);
	
	wxString UrlEncode(wxString str);
	
	/// popups para cuadros de texto con boton de "..."
	void ShowTextPopUp(wxWindow *parent, wxString title, wxTextCtrl *text, wxString options, wxString path="");
	void ProcessTextPopup(int id, wxWindow *parent=NULL, wxTextCtrl *t=NULL, wxString path="", wxString title="", bool replace=false, bool comma_splits=false);
	
	/// determina el tipo de archivo segun su extension
	eFileType GetFileType(wxString name, bool recognize_projects=true);
	
	/// returns the list of files/subdirs from a directory (not recursive, add to the array without clearing first)
	int GetFilesFromDir(wxArrayString &array, wxString path, bool files=true);
	/// populate the array with files from home/.zinjai/dir_name install_dir/dir_name (sorted) and adds extra_element at the in if it is not empty
	void GetFilesFromBothDirs(wxArrayString &array, wxString dir_name, bool is_file=true, wxString extra_element="");
	
	/// returns the list of files subdirectories from a directory (not recursive, add to the array without clearing first)
	int Unique(wxArrayString &array, bool do_sort=false);
	/// looks for file in path1 and path2, and returns the first ocurrence, or and empty string if not found
	wxString WichOne(wxString file, wxString path1, wxString path2, bool is_file=true);
	/// wrapper for looking as in the other WichOne with paths home/.zinjai/dir_name and install_dir/dir_name
	wxString WichOne(wxString file, wxString dir_name, bool is_file=true);
	/// determines whether the argument "arg" is prensent in the arguments list "full"
	bool IsArgumentPresent(const wxString &full, const wxString &arg);
	/// adds/removes the argument "arg" is prensent to/from the arguments list "full" (modifies full)
	void SetArgument(wxString &full, const wxString &arg, bool add);
	
	/// takes a graph description, process it with graphviz and shows results (output=="") or save it to an image file(output)
	int ProcessGraph(wxString graph_file, bool use_fdp, wxString output, wxString title="");
	
	/// for replacing aguments in translate strings for gui messages (version for 1 argument)
	wxString ReplaceLangArgs(wxString src, wxString arg1);
	/// for replacing aguments in translate strings for gui messages (version for 2 arguments)
	wxString ReplaceLangArgs(wxString src, wxString arg1, wxString arg2);
	/// for replacing aguments in translate strings for gui messages (version for 3 arguments)
	wxString ReplaceLangArgs(wxString src, wxString arg1, wxString arg2, wxString arg3);

};

extern mxUtils *utils;

extern bool zinjai_debug_mode; ///< cuando se pone en true zinjai muestra con wxMessageBox información interna, (ver _IF_DEBUGMODE), para ayudar a encontrar un error en version release, se activa desde Do That con "set verbose"

#define _IF_DEBUGMODE(x) if (zinjai_debug_mode) wxMessageBox(wxString()<<x)

#endif

