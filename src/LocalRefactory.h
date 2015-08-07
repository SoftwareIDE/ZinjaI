#ifndef LOCALREFACTORY_H
#define LOCALREFACTORY_H
#include <wx/string.h>

class mxSource;

/// class grouping helper functions for code-generation and refactory operations
class LocalRefactory {
public:
	static void GenerateFunction(mxSource *src, int pos);
	static wxString GetLiteralType(wxString literal_expression);
};

#endif

