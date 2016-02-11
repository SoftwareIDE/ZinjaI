#ifndef CODEEXPORTER_H
#define CODEEXPORTER_H
#include <wx/string.h>

class mxSource;

/**
* @brief se encarga de exportar un fuente a HTML, formateandolo correctamente
**/
class CodeExporter {
private:
	struct style_def {
		bool bold, italic, underline, used;
		wxString bgcolor, fgcolor;
		style_def() {
			bold=italic=underline=false;
			bgcolor="#000000";
			fgcolor="#000000";
			used=false;
		}
	} m_styles[128];
	void DefineStyles(int lexer);
public:
	CodeExporter();
	bool ExportHtml(mxSource *src, wxString title, wxString fname);
	void SetStyle(int idx, int fontSize, const wxChar *foreground, const wxChar *background, int fontStyle);
	
};

#endif

