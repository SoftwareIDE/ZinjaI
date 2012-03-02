#ifndef MXCOMPLEMENTINSTALLERWINDOW_H
#define MXCOMPLEMENTINSTALLERWINDOW_H
#include <wx/dialog.h>

/**
* @brief Dialogo para instalar complementos
* 
* Este dialogo permite seleccionar un complemento para instalar en ZinjaI.
* No es en realidad este diálogo quien lo instala, ya que ZinjaI podría no 
* tener permisos suficientes para modificar su instalación, o hasta podría ser
* necesario modificar el propio binario que se está ejecutando. Por eso, el
* diálogo permite elegir el complemento y invoca a otro ejecutable para
* que complete la instalación pasándole los datos que necesita.
**/
class mxComplementInstallerWindow : wxDialog {
private:
protected:
public:
	/// @brief Constuctor
	mxComplementInstallerWindow(wxWindow *parent);
	/// @brief Callback del botón "Instalar...", lleva a elegir el archivo del complemento y lanzar la instalación
	void OnOkButton(wxCommandEvent &evt);
	/// @brief Callback del botón de ayuda
	void OnHelpButton(wxCommandEvent &evt);
	/// @brief Callback del botón "Descargar...", abre en un navegador el sitio para descargar complementos
	void OnDownloadButton(wxCommandEvent &evt);
	/// @brief Callback del botón "Cerrar", cierra la ventana sin hacer nada más
	void OnCancelButton(wxCommandEvent &evt);
	/// @brief Destruye (quita de la memoria) la ventana cuando de cierra
	void OnClose(wxCloseEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

