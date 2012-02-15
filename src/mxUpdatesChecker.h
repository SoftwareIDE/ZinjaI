#ifndef MXUPDATESCHECKER_H
#define MXUPDATESCHECKER_H
#include <wx/dialog.h>
#include <wx/timer.h>
#include <wx/process.h>
class wxStaticText;
class wxCheckBox;
class wxButton;
class wxTextCtrl;

/**
* @brief Dialogo para verificar si existen actualizaciones.
* 
* Dialogo para verificar si existen actualizaciones. Para ello buscar el archivo
* "version" en la pagina (http://zinjai.sourceforge.net/version) y lo compara su
* contenido (una linea que dice ZVERSION=AAAAMMDD) con la version actual del
* entorno (config->Init.version). En caso de encontrar actualizaciones solo
* provee enlaces al sitio de descarga y al registro de cambios, pero no gestiona
* la descarga e instalación de la actualización
**/
class mxUpdatesChecker : public wxDialog {
private:
	bool shown;
	wxStaticText *text;
	wxCheckBox *check;
	wxTextCtrl *proxy_dir;
	wxTextCtrl *proxy_port;
	wxButton *close_button;
	wxButton *changes_button;
	wxButton *proxy_button;
	static wxProcess *process;
public:
	bool show,done;
	mxUpdatesChecker(bool show=true);
	static void BackgroundCheck();
	void CheckNow();
	void OnClose(wxCloseEvent &evt);
	void OnProxyButton(wxCommandEvent &evt);
	void OnChangesButton(wxCommandEvent &evt);
	void OnCloseButton(wxCommandEvent &evt);
	void OnProcessEnds(wxProcessEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

