#ifndef MXGENERICINSPECTIONCTRL_H
#define MXGENERICINSPECTIONCTRL_H
#include "SingleList.h"

/**
* @brief Clase base para todo control que muestre inspecciones
*
* Todas las ventanas y paneles que muestren o usen de alguna manera 
* inspecciones deben heredar de esta clase para que se les avise automáticamente
* cuando los resultados de las inspecciones pueden haber cambiado para que
* se actulicen.
**/
class mxGenericInspectionCtrl {
	
	/// lista de ventanas que usan/muestran inspecciones, , para que se les avise cuando deben actualizarse
	static SingleList<mxGenericInspectionCtrl*> list; 
	
public:
	
	/// Constructor, registra la ventana en la lista de ventanas que usan inspecciones
	mxGenericInspectionCtrl();
	
	/// Constructor, desregistra la ventana en la lista de ventanas que usan inspecciones
	~mxGenericInspectionCtrl();
	
	/**
	* @brief Método a llamar cuando el depurador actuliza las inspecciones 
	*
	* Cuando el depurador actualiza las inspecciones (es decir, averigua los nuevos valores, 
	* actualiza los datos en las DebuggerInspection), esta ventana debe mostrar esos cambios.
	**/
	virtual void OnInspectionsUpdated()=0;
	
	/// Invoca a OnInspectionsUpdated para todas las ventanas de este tipo
	static void UpdateAll();
};

#endif

