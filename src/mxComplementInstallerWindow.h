#ifndef MXCOMPLEMENTINSTALLERWINDOW_H
#define MXCOMPLEMENTINSTALLERWINDOW_H
#include <wx/dialog.h>

/**
* @brief Dialogo para instalar complementos
* 
* Este dialogo permite seleccionar un complemento para instalar en ZinjaI.
* No es en realidad este di�logo quien lo instala, ya que ZinjaI podr�a no 
* tener permisos suficientes para modificar su instalaci�n, o hasta podr�a ser
* necesario modificar el propio binario que se est� ejecutando. Por eso, el
* di�logo permite elegir el complemento y invoca a otro ejecutable para
* que complete la instalaci�n pas�ndole los datos que necesita.
**/
class mxComplementInstallerWindow : wxDialog {
private:
protected:
public:
	/// @brief Constuctor
	mxComplementInstallerWindow(wxWindow *parent);
	/// @brief Callback del bot�n "Instalar...", lleva a elegir el archivo del complemento y lanzar la instalaci�n
	void OnOkButton(wxCommandEvent &evt);
	/// @brief Callback del bot�n de ayuda
	void OnHelpButton(wxCommandEvent &evt);
	/// @brief Callback del bot�n "Descargar...", abre en un navegador el sitio para descargar complementos
	void OnDownloadButton(wxCommandEvent &evt);
	/// @brief Callback del bot�n "Cerrar", cierra la ventana sin hacer nada m�s
	void OnCancelButton(wxCommandEvent &evt);
	/// @brief Destruye (quita de la memoria) la ventana cuando de cierra
	void OnClose(wxCloseEvent &evt);
	/// @brief Instala el complemento invocanto al instalador externo
	void Install(wxString fname);
	DECLARE_EVENT_TABLE();
};

#endif

