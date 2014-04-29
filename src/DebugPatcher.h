#ifndef DEBUGPATCHER_H
#define DEBUGPATCHER_H
#include <wx/string.h>


class DebugPatcher {
	
	struct mem_seg {
		int mini,mend,fini,size; // dato tomados del "info proc mappings" de gdb, los carga el constructor mem_seg::mem_seg(wxString)
		int cambios_ok, cambios_error; // cantidad de cambios que si lograron aplicar y cantidad que no (lo completa DebugPatcher::Patch(mem_seg&))
		bool extract(wxString &s, int &where) ; ///< método auxiliar para el constructor
		mem_seg() {}
		mem_seg(wxString line);
	};
	
	wxString exe_file; ///< archivo que está ejecutando el depurador (importa solo el nombre para buscar sus segmentos en gdb)
	wxString patched_file; ///< archivo con los cambios (partes se van a comparar con lo tire el dump de memoria de gdb)
	wxString temp_file; ///< archivo donde gdb hace el dump con lo que tiene "su" binario para comparar con el nuevo
	
	unsigned int max_cambios;
public:
	DebugPatcher():max_cambios(100) {}
	~DebugPatcher();
		
	int Patch(mem_seg &ms);
public:
	void Patch();
	void Init(wxString bin_name) { exe_file=bin_name; }
	void AlterOuputFileName(wxString &bin_name) {
//#ifdef __WIN32__
//		if (bin_name==exe_file) bin_name+=".for_patch";
//#endif
		patched_file=bin_name;
	}
};

#endif

