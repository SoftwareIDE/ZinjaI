#include "CodeExporter.h"
#include "mxSource.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include <wx/ffile.h>
#include "Language.h"

CodeExporter::CodeExporter() {
	
}

bool CodeExporter::ExportHtml(mxSource *src, wxString title, wxString fname) {
	
	wxFFile fil(fname,_T("w+"));
	if (!fil.IsOpened()) 
		return false;
	
	main_window->SetStatusText(LANG(HTMLEXPORT_STATUS,"Exportando..."));
	
	wxString header;
	header<<_T("<HTML><HEAD>\n\t<TITLE>")<<title<<_T("</TITLE>\n\t<STYLE type=\"text/css\">\n");
	fil.Write(header);
	
	header=_T("\t\t.SSD { color:#FFFFFF; background-color:#000000; }\n");
	fil.Write(header);

	
	DefineStyles(src->lexer);
	for (int i=0;i<128;i++) {
		if (styles[i].used) {
			header=_T("\t\t.SS");
			header<<i<<_T(" { ");
			if(styles[i].bold)
				header<<_T("font-weight: bold; ");
			if(styles[i].italic)
				header<<_T("font-style: italic; ");
			if(styles[i].underline)
				header<<_T("text-decoration: underline; ");
			header<<_T("color:")<<styles[i].fgcolor<<_T("; ")<<_T("background-color:")<<styles[i].bgcolor<<_T("; }\n");
			fil.Write(header);
		}
	}
	header=_T("\t</STYLE>\n</HEAD><BODY><CODE>\n\t");
	fil.Write(header);
	
	
	int len = src->GetLength();
	src->Colourise(0,len);
	int p0=0,p1=1;
	while (true) {
		int s=src->GetStyleAt(p0);
		while (p1<len && src->GetStyleAt(p1)==s)
			p1++;
			
		if (styles[s].used) {
			wxString code(_T("<SPAN class=\"SS"));
			code<<s<<_T("\">")<<mxUT::ToHtml(src->GetTextRange(p0,p1))<<_T("</SPAN>");
			fil.Write(code);
		} else {
			wxString code(_T("<SPAN class=\"SSD"));
			code<<s<<_T("\">")<<mxUT::ToHtml(src->GetTextRange(p0,p1))<<_T("</SPAN>");
			fil.Write(code);
		}
		
		if (p1==len) 
			break;
		p0=p1++;
	}

	wxString footer(_T("\n</CODE></BODY></HTML>"));
	fil.Write(footer);

	fil.Close();
	
	mxUT::OpenInBrowser(fname);
	
	main_window->SetStatusText(LANG(GENERAL_READY,"Listo"));
	
	return true;
}

void CodeExporter::DefineStyles(int lexer) {
	switch (lexer) {
	case wxSTC_LEX_CPP:
		SetStyle(wxSTC_C_DEFAULT,config->Styles.font_size,_T("BLACK"),_T("WHITE"),0); // default
		SetStyle(wxSTC_C_COMMENT,config->Styles.font_size,_T("Z DARK GRAY"),_T("WHITE"),mxSOURCE_ITALIC); // comment
		SetStyle(wxSTC_C_COMMENTLINE,config->Styles.font_size,_T("Z DARK GRAY"),_T("WHITE"),mxSOURCE_ITALIC); // comment line
		SetStyle(wxSTC_C_COMMENTDOC,config->Styles.font_size,_T("Z DOXY BLUE"),_T("WHITE"),mxSOURCE_ITALIC); // comment doc
		SetStyle(wxSTC_C_NUMBER,config->Styles.font_size,_T("SIENNA"),_T("WHITE"),0); // number
		SetStyle(wxSTC_C_WORD,config->Styles.font_size,_T("Z DARK BLUE"),_T("WHITE"),mxSOURCE_BOLD); // keywords
		SetStyle(wxSTC_C_STRING,config->Styles.font_size,_T("RED"),_T("WHITE"),0); // string
		SetStyle(wxSTC_C_CHARACTER,config->Styles.font_size,_T("MAGENTA"),_T("WHITE"),0); // character
		SetStyle(wxSTC_C_UUID,config->Styles.font_size,_T("ORCHID"),_T("WHITE"),0); // uuid
		SetStyle(wxSTC_C_PREPROCESSOR,config->Styles.font_size,_T("FOREST GREEN"),_T("WHITE"),0); // preprocessor
		SetStyle(wxSTC_C_OPERATOR,config->Styles.font_size,_T("BLACK"),_T("WHITE"),mxSOURCE_BOLD); // operator 
		SetStyle(wxSTC_C_IDENTIFIER,config->Styles.font_size,_T("BLACK"),_T("WHITE"),0); // identifier 
		SetStyle(wxSTC_C_STRINGEOL,config->Styles.font_size,_T("RED"),_T("LIGHT GRAY"),0); // string eol
		SetStyle(wxSTC_C_VERBATIM,config->Styles.font_size,_T("BLACK"),_T("WHITE"),0); // default verbatim
		SetStyle(wxSTC_C_REGEX,config->Styles.font_size,_T("ORCHID"),_T("WHITE"),0); // regexp  
		SetStyle(wxSTC_C_COMMENTLINEDOC,config->Styles.font_size,_T("Z DOXY BLUE"),_T("WHITE"),0); // special comment 
		SetStyle(wxSTC_C_WORD2,config->Styles.font_size,_T("Z DARK BLUE"),_T("WHITE"),0); // extra words
		SetStyle(wxSTC_C_COMMENTDOCKEYWORD,config->Styles.font_size,_T("CORNFLOWER BLUE"),_T("WHITE"),0); // doxy keywords
		SetStyle(wxSTC_C_COMMENTDOCKEYWORDERROR,config->Styles.font_size,_T("RED"),_T("WHITE"),0); // keywords errors
		SetStyle(wxSTC_C_GLOBALCLASS,config->Styles.font_size,_T("BLACK"),_T("WHITE"),0); // keywords errors
		SetStyle(wxSTC_STYLE_BRACELIGHT,config->Styles.font_size,_T("RED"),_T("Z LIGHT BLUE"),mxSOURCE_BOLD); 
		SetStyle(wxSTC_STYLE_BRACEBAD,config->Styles.font_size,_T("Z DARK RED"),_T("WHITE"),mxSOURCE_BOLD); 
		break;
	case wxSTC_LEX_HTML: case wxSTC_LEX_XML:
		SetStyle(wxSTC_H_DEFAULT,config->Styles.font_size,_T("BLACK"),_T("WHITE"),0); // default
		SetStyle(wxSTC_H_TAG,config->Styles.font_size,_T("Z DARK BLUE"),_T("WHITE"),mxSOURCE_BOLD); // keywords
		SetStyle(wxSTC_H_TAGUNKNOWN,config->Styles.font_size,_T("Z DARK BLUE"),_T("WHITE"),mxSOURCE_BOLD); // keywords
		SetStyle(wxSTC_H_ATTRIBUTE,config->Styles.font_size,_T("BLUE"),_T("WHITE"),0); // comment doc
		SetStyle(wxSTC_H_ATTRIBUTEUNKNOWN,config->Styles.font_size,_T("BLUE"),_T("WHITE"),0); // comment doc
		SetStyle(wxSTC_H_COMMENT,config->Styles.font_size,_T("Z DARK GRAY"),_T("WHITE"),mxSOURCE_ITALIC); // comment doc
		SetStyle(wxSTC_H_SINGLESTRING,config->Styles.font_size,_T("RED"),_T("WHITE"),0); // string
		SetStyle(wxSTC_H_DOUBLESTRING,config->Styles.font_size,_T("RED"),_T("WHITE"),0); // string
		SetStyle(wxSTC_H_NUMBER,config->Styles.font_size,_T("SIENNA"),_T("WHITE"),0); // number
		SetStyle(wxSTC_H_VALUE,config->Styles.font_size,_T("FOREST GREEN"),_T("WHITE"),0); // preprocessor
		SetStyle(wxSTC_H_SCRIPT,config->Styles.font_size,_T("FOREST GREEN"),_T("WHITE"),0); // preprocessor
		SetStyle(wxSTC_H_ENTITY,config->Styles.font_size,_T("BLUE"),_T("WHITE"),0); // &nbsp;
		break;
	case wxSTC_LEX_MAKEFILE:
		SetStyle(wxSTC_MAKE_DEFAULT,config->Styles.font_size,_T("BLACK"),_T("WHITE"),0); // default
		SetStyle(wxSTC_MAKE_IDENTIFIER,config->Styles.font_size,_T("Z DARK BLUE"),_T("WHITE"),mxSOURCE_BOLD); // keywords
		SetStyle(wxSTC_MAKE_COMMENT,config->Styles.font_size,_T("Z DARK GRAY"),_T("WHITE"),mxSOURCE_ITALIC); // comment doc
		SetStyle(wxSTC_MAKE_PREPROCESSOR,config->Styles.font_size,_T("FOREST GREEN"),_T("WHITE"),0); // preprocessor
		SetStyle(wxSTC_MAKE_OPERATOR,config->Styles.font_size,_T("BLACK"),_T("WHITE"),mxSOURCE_BOLD); // operator 
		SetStyle(wxSTC_MAKE_TARGET,config->Styles.font_size,_T("BLACK"),_T("WHITE"),mxSOURCE_BOLD); // string
		SetStyle(wxSTC_MAKE_IDEOL,config->Styles.font_size,_T("SIENNA"),_T("WHITE"),0); // string
		break;
	}
}

void CodeExporter::SetStyle(int idx, int fontSize, const wxChar *foreground, const wxChar *background, int fontStyle){
	if (foreground) 
		styles[idx].fgcolor = wxColour(foreground).GetAsString(wxC2S_HTML_SYNTAX);
	if (background)
		styles[idx].bgcolor = wxColour(background).GetAsString(wxC2S_HTML_SYNTAX);
	styles[idx].bold = fontStyle&mxSOURCE_BOLD;
	styles[idx].italic = fontStyle&mxSOURCE_ITALIC;
	styles[idx].underline = fontStyle&mxSOURCE_UNDERL;
	styles[idx].used=true;
}
