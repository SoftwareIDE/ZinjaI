#include "LocalRefactory.h"
#include "mxSource.h"
#include <wx/msgdlg.h>
#include "Language.h"
#include "mxMainWindow.h"
#include "mxMessageDialog.h"

void LocalRefactory::GenerateFunction (mxSource * src, int pos) {
#define GenerateFunction_error { \
	mxMessageDialog(main_window,LANG(CODE_CANNOT_GENERATE_FUNCTION,"No se pudo determinar el prototipo de la función."), \
					LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal(); return; }
	// get the calll
	wxArrayString args;
	wxString func_name = src->GetCurrentCall(args, pos);
	if (func_name=="") GenerateFunction_error;
	// get the scope, to insert the code right before
	int scope_start = -1;
	wxString scope_args, scope = src->FindScope(pos,&scope_args,true,&scope_start);
	if (scope=="") GenerateFunction_error;
	scope_start = src->GetStatementStartPos(scope_start);
	// build function prototype
	wxString func_proto = wxString()<<"void "<<func_name<<"(";
	if (args.GetCount()) {
		for(unsigned int i=0;i<args.GetCount();i++) {
			func_proto<<args[i]<<", ";
		}
		func_proto.RemoveLast();
		func_proto.RemoveLast();
	}
	func_proto<<")";
	// insert code, indent, move cursor there
	src->InsertText(scope_start,func_proto+" {\n\t\n}\n\n");
	int line_start = src->LineFromPosition(scope_start);
//	src->Indent(line_start,line_start+3);
	src->GotoPos(src->GetLineIndentPosition(line_start+1));
}

wxString LocalRefactory::GetLiteralType (wxString literal_expression) {
	wxString &s=literal_expression; int l=s.Len(), i=0; if (!l) return "";
	if (s[0]=='\'') return "char";
	else if (s[0]=='\"') return "const char *";
	else if (s[0]>='0'||s[0]<='1'||s[0]=='.') {
		bool is_real = s[0]=='.';
		while(++i<l) {
			if (s[i]=='.') is_real=true;
			else if (s[i]=='u') {
				if (i+1<l && s[i+1]=='l') return "unsigned long";
				else return "unsigned int";
			}
			else if (s[i]=='f') {
				if (i+1<l && s[i+1]=='l') return "float";
				else return "unsigned int";
			}
			else if (s[i]<'0'&&s[i]>'9'&&s[i]!='\'') break;
		}
		return is_real?"float":"int";
	}
	
}

