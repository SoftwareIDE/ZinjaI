#ifndef LOCALREFACTORY_H
#define LOCALREFACTORY_H
#include <wx/string.h>

class mxSource;

/// class grouping helper functions for code-generation and refactory operations
class LocalRefactory {
	static int GetScopeStart(mxSource *src, int pos);
	static wxString GetFunctionProto(mxSource *src, int pos);
	static void Surround(mxSource *src, int lmin, int lmax, wxString spre, wxString spos, bool fix_indent=true);
	static int InsertText(mxSource *src, int line, wxString content, bool fix_indent=true);
	static void ExtractFunctionPost (mxSource * src, int line_proto, int line_call);
public:
	static void GenerateFunctionDec(mxSource *src, int pos);
	static void GenerateFunctionDef(mxSource *src, int pos);
	static void ExtractFunction(mxSource *src, int pos);
	static void SurroundIf(mxSource *src, int pos);
	static void SurroundWhile(mxSource *src, int pos);
	static void SurroundDo(mxSource *src, int pos);
	static void SurroundFor(mxSource *src, int pos);
	static void SurroundIfdef(mxSource *src, int pos);
	static wxString GetLiteralType(wxString literal_expression);
};

#endif

