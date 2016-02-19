#ifndef MXCOMMONPOPUP_H
#define MXCOMMONPOPUP_H
#include <wx/menu.h>
#include <wx/string.h>
#include "ids.h"
#include "Language.h"

class mxCommonPopup {
	bool m_registered_in_event_handler;
	wxWindow *m_parent;
	wxTextCtrl *m_text_ctrl;
	wxString m_caption;
	wxString m_base_path;
	bool m_comma_split;
	void RegisterEvents();
	wxMenu m_menu;
	void ReplaceSelectionWith(wxString text) {
		long f,t;
		m_text_ctrl->GetSelection(&f,&t);
		m_text_ctrl->Replace(f,t,text);
		m_text_ctrl->SetFocus();
	}
public:
	mxCommonPopup() : m_registered_in_event_handler(false), m_text_ctrl(nullptr), m_comma_split(false) {}
	
	mxCommonPopup &ResetMenu() { 
		m_base_path = m_caption = ""; m_comma_split = false; m_text_ctrl = nullptr; 
		while(m_menu.GetMenuItemCount()) m_menu.Remove(m_menu.FindItemByPosition(0)); return *this;
	}
	mxCommonPopup &CommonPopup(wxTextCtrl *text_ctrl) { ResetMenu(); m_text_ctrl = text_ctrl; return *this; }
	template<typename wxDialog_t> void Run(wxDialog_t *dlg) {
		if (!m_registered_in_event_handler) {
			dlg->Connect(mxID_POPUPS_ID_BEGIN,mxID_POPUPS_ID_END,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(wxDialog_t::OnCommonPopup),nullptr,dlg);
			m_registered_in_event_handler = true;
		}
		m_parent = dlg;
		dlg->PopupMenu(&m_menu); 
	}
	
	mxCommonPopup &SelectAll() { if (m_text_ctrl) m_text_ctrl->SetSelection(0,m_text_ctrl->GetValue().Len()); return *this; }
	mxCommonPopup &CommaSplit(bool comma_split) { m_comma_split = comma_split; return *this; }
	mxCommonPopup &Caption(const wxString &caption) { m_caption = caption; return *this; }
	mxCommonPopup &BasePath(const wxString &base_path) { m_base_path = base_path; return *this; }

	mxCommonPopup &AddEditAsText() { m_menu.Append(mxID_POPUPS_EDIT_AS_TEXT,LANG(GENERAL_POPUP_INSERT_TEXT,"editar como texto...")); return *this; }
	mxCommonPopup &AddEditAsList() { m_menu.Append(mxID_POPUPS_EDIT_AS_LIST,LANG(GENERAL_POPUP_INSERT_LIST,"editar como lista...")); return *this; }
	mxCommonPopup &AddFilename() { m_menu.Append(mxID_POPUPS_INSERT_FILE,LANG(GENERAL_POPUP_INSERT_FILE,"insertar archivo...")); return *this; }
	mxCommonPopup &AddPath() { m_menu.Append(mxID_POPUPS_INSERT_DIR,LANG(GENERAL_POPUP_REPLACE_INSERT_DIR,"insertar directorio...")); return *this; }
	mxCommonPopup &AddOutFile() { m_menu.Append(mxID_POPUPS_INSERT_OUTPUT,LANG(GENERAL_POPUP_INSERT_OUTPUT,"insertar archivo de salida")); return *this; }
	mxCommonPopup &AddDeps() { m_menu.Append(mxID_POPUPS_INSERT_DEPS,LANG(GENERAL_POPUP_INSERT_DEPS,"insertar lista de dependencias")); return *this; }
	mxCommonPopup &AddTempDir() { m_menu.Append(mxID_POPUPS_INSERT_TEMP_DIR,LANG(GENERAL_POPUP_INSERT_TEMP_DIR,"insertar directorio de temporales")); return *this; }
#ifdef __WIN32__
	mxCommonPopup &AddMinGWDir() { m_menu.Append(mxID_POPUPS_INSERT_MINGW_DIR,LANG(GENERAL_POPUP_INSERT_MINGW_DIR,"insertar directorio MinGW")); return *this; }
#else
	mxCommonPopup &AddMinGWDir() { m_menu.Append(mxID_POPUPS_INSERT_MINGW_DIR,LANG(GENERAL_POPUP_INSERT_MINGW_DIR,"insertar directorio del compilador")); return *this; }
#endif
	mxCommonPopup &AddProjectDir() { m_menu.Append(mxID_POPUPS_INSERT_PROJECT_PATH,LANG(GENERAL_POPUP_INSERT_PROJECT_PATH,"insertar directorio del proyecto")); return *this; }
	mxCommonPopup &AddProjectBin() { m_menu.Append(mxID_POPUPS_INSERT_PROJECT_BIN,LANG(GENERAL_POPUP_INSERT_PROJECT_BIN,"insertar ruta del ejecutable")); return *this; }
	mxCommonPopup &AddArgs() { m_menu.Append(mxID_POPUPS_INSERT_ARGS,LANG(GENERAL_POPUP_INSERT_ARGS,"argumentos para la ejecución")); return *this; }
	mxCommonPopup &AddCurrentFile() { m_menu.Append(mxID_POPUPS_INSERT_CURRENT_FILE,LANG(GENERAL_POPUP_INSERT_CURRENT_FILE,"insertar archivo actual")); return *this; }
	mxCommonPopup &AddCurrentDir() { m_menu.Append(mxID_POPUPS_INSERT_CURRENT_DIR,LANG(GENERAL_POPUP_INSERT_CURRENT_DIR,"insertar directorio del archivo actual")); return *this; }
	mxCommonPopup &AddWorkDir() { m_menu.Append(mxID_POPUPS_INSERT_WORKDIR,LANG(GENERAL_POPUP_INSERT_WORKDIR,"insertar el directorio de trabajo")); return *this; }
	mxCommonPopup &AddZinjaiDir() { m_menu.Append(mxID_POPUPS_INSERT_ZINJAI_DIR,LANG(GENERAL_POPUP_INSERT_ZINJAI_DIR,"insertar el directorio de instalación de ZinjaI")); return *this; }
	mxCommonPopup &AddBrowerCommand() { m_menu.Append(mxID_POPUPS_INSERT_BROWSER,LANG(GENERAL_POPUP_INSERT_BROWSER,"insertar comando del navegador")); return *this; }
	mxCommonPopup &AddShellCommand() { m_menu.Append(mxID_POPUPS_INSERT_SHELL_EXECUTE,LANG(GENERAL_POPUP_INSERT_SHELL_EXECUTE,"insertar comando para abrir con el programa asociado")); return *this; }

	void ProcessCommandEvent(wxCommandEvent &evt);
};

/**
* @brief Workaround para insertar la funcionalidad de mxCommonPopup en cualquier dialog con una sola linea
*
* la idea era usar la clase mxCommonPopup como base del dialogo que necesite estos popups menus,
* pero si lo hago así el método ProcessCommandEvent no es del diálogo, y entonces no es de un
* eventhandler, y entonces el Connect lo rechaza... esta macro es un parche temporal para seguir
* usando esto con una sola linea en la clase cliente, hasta que solucione lo del event handler...
**/
#define _use_common_popup \
	mxCommonPopup m_common_popup; \
	void OnCommonPopup(wxCommandEvent &evt) { m_common_popup.ProcessCommandEvent(evt); } \
	mxCommonPopup &CommonPopup(wxTextCtrl *text_ctrl) { return m_common_popup.CommonPopup(text_ctrl); } \
	friend class mxCommonPopup

/**
* Prototipo de función que no existe, es solo para engañar al autocompletado
*
* Como el autocompletado de ZinjaI no resuelve bien la macro _use_common_popup, en las
* clases donde se uso, no reconoce el método CommonPopup... pero va a usar en su lugar
* esta función para autocompletar
**/
mxCommonPopup &CommonPopup(wxTextCtrl*); // si esto genera un "undefined reference...", falta "_use_common_popup;" en algún .h

#endif

