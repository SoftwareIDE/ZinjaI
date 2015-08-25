#include "LocalRefactory.h"
#include "mxSource.h"
#include <wx/msgdlg.h>
#include "Language.h"
#include "mxMainWindow.h"
#include "mxMessageDialog.h"

#define GenerateFunction_error(retval) { \
	mxMessageDialog(main_window,LANG(CODE_CANNOT_GENERATE_FUNCTION,"No se pudo determinar el prototipo de la función."), \
		LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal(); return retval; }

int LocalRefactory::GetScopeStart(mxSource * src, int pos) {
	// get the scope, to insert the code right before
	int scope_start = -1;
	wxString scope_args, scope = src->FindScope(pos,&scope_args,true,&scope_start,true);
	if (scope=="") GenerateFunction_error(wxSTC_INVALID_POSITION);
	return src->GetStatementStartPos(scope_start);
}

wxString LocalRefactory::GetFunctionProto(mxSource * src, int pos) {

	// get the calll
	wxArrayString args; wxString func_name, func_type;
	if (!src->GetCurrentCall(func_type, func_name, args, pos))
		GenerateFunction_error("");
	// build function prototype
	wxString func_proto = func_type + " " +  func_name + "(";
	if (args.GetCount()) {
		for(unsigned int i=0;i<args.GetCount();i++) {
			func_proto<<args[i]<<", ";
		}
		func_proto.RemoveLast();
		func_proto.RemoveLast();
	}
	func_proto<<")";
	return func_proto;
}

wxString LocalRefactory::GetLiteralType (wxString literal_expression) {
	wxString &s=literal_expression; int l=s.Len(), i=0; if (!l) return "";
	if (s[0]=='\'') return "char";
	else if (s[0]=='\"') return "const char *";
	else if ((s[0]>='0'&&s[0]<='9')||s[0]=='.') {
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
	else return "";
}

void LocalRefactory::GenerateFunctionDef (mxSource * src, int pos) {
	mxSource::UndoActionGuard undo_action(src);
	int scope_start = GetScopeStart(src,pos);
	if (scope_start==wxSTC_INVALID_POSITION) GenerateFunction_error();
	wxString func_proto = GetFunctionProto(src,pos);
	// insert code, indent, move cursor there
	int line_start = src->LineFromPosition(scope_start);
	InsertText(src, line_start, func_proto+"{\n\n}\n");
	src->GotoPos(src->GetLineIndentPosition(line_start+1));
}

void LocalRefactory::GenerateFunctionDec (mxSource * src, int pos) {
	mxSource::UndoActionGuard undo_action(src);
	int scope_start = GetScopeStart(src,pos);
	if (scope_start==wxSTC_INVALID_POSITION) GenerateFunction_error();
	wxString func_proto = GetFunctionProto(src,pos);
	// insert code, indent, move cursor there
	int line_start = src->LineFromPosition(scope_start);
	InsertText(src, line_start, func_proto+";\n");
	src->GotoPos(src->GetLineIndentPosition(line_start));
}

void LocalRefactory::ExtractFunctionPost (mxSource * src, int line_proto, int line_call) {
	int x = src->GetLineIndentPosition(line_call), e = src->GetLineEndPosition(line_call), p = wxSTC_INVALID_POSITION;
	while (x<e && src->GetCharAt(x)!='(') {
		char c = src->GetCharAt(x);
		if (src->IsKeywordChar(c)) p=x;
		x++;
	}
	if (p!=wxSTC_INVALID_POSITION) {
		wxString proto = GetFunctionProto(src,p);
		if (proto!="") {
			src->SetTargetStart( src->GetLineIndentPosition(line_proto) );
			src->SetTargetEnd( src->GetLineEndPosition(line_proto) );
			src->ReplaceTarget(proto+" {");
		}
	}
	src->GotoPos(src->GetLineEndPosition(line_proto)-2);
}

void LocalRefactory::ExtractFunction (mxSource * src, int pos) {
	mxSource::UndoActionGuard undo_action(src);
	int scope_start = GetScopeStart(src,pos);
	if (scope_start==wxSTC_INVALID_POSITION) GenerateFunction_error();
	// get the function body
	int lmin, lmax; src->GetSelectedLinesRange(lmin,lmax);
	int p0 = src->GetLineIndentPosition(lmin), p1 = src->GetLineEndPosition(lmax);
	wxString body = src->GetTextRange(p0,p1);
	// remove these lines and place there the new call
	src->SetTargetStart(p0); src->SetTargetEnd(p1);	src->ReplaceTarget("foo();");
	src->SetSelectionStart(p0); src->SetSelectionEnd(p0+3);
	// cretate the new function definition
	int line_start = src->LineFromPosition(scope_start);
	int delta = InsertText(src, line_start, wxString("void foo() {\n")+body+"\n}\n");
	lmin+=delta; lmax+=delta;
	// start call and prototype simultaneous edition 
	src->multi_sel.Reset();
	src->multi_sel.SetEditRegion(src,lmin,src->GetLineIndentPosition(lmin),src->GetLineEndPosition(lmin));
	src->multi_sel.AddPos(src,line_start,src->GetLineIndentPosition(line_start)+5);
	_CAPTURELIST_3(s_lmbRefUpdateProto, lmb_arg,
				mxSource*,src,
				int,line_start,
				int,lmin);
	_LAMBDA_1(lmbRefUpdateProto, s_lmbRefUpdateProto,lmb_arg, {
		LocalRefactory::ExtractFunctionPost(lmb_arg.src,lmb_arg.line_start,lmb_arg.lmin);
	});
	src->multi_sel.Begin(src,false,true,true,new lmbRefUpdateProto(lmb_arg));
	src->EnsureCaretVisible();
}

void LocalRefactory::SurroundIf (mxSource * src, int pos) {
	int lmin, lmax; src->GetSelectedLinesRange(lmin,lmax);
	Surround(src,lmin,lmax,"if (#here#) {","}");
}

void LocalRefactory::SurroundWhile (mxSource * src, int pos) {
	int lmin, lmax; src->GetSelectedLinesRange(lmin,lmax);
	Surround(src,lmin,lmax,"while (#here#) {","}");
}

void LocalRefactory::SurroundDo (mxSource * src, int pos) {
	int lmin, lmax; src->GetSelectedLinesRange(lmin,lmax);
	Surround(src,lmin,lmax,"do {","} while (#here#);");
}

void LocalRefactory::SurroundFor (mxSource * src, int pos) {
	int lmin, lmax; src->GetSelectedLinesRange(lmin,lmax);
	Surround(src,lmin,lmax,"for (#here#;;) {","}");
}

void LocalRefactory::SurroundIfdef (mxSource * src, int pos) {
	int lmin, lmax; src->GetSelectedLinesRange(lmin,lmax);
	Surround(src,lmin,lmax,"#ifdef #here#","#endif",false);
}

void LocalRefactory::Surround (mxSource *src, int lmin, int lmax, wxString spre, wxString spos, bool fix_indent) {
	mxSource::UndoActionGuard undo_action(src);
	if (fix_indent) {
		for(int i=lmin;i<=lmax;i++)
			src->SetLineIndentation(i,src->GetLineIndentation(i)+src->config_source.tabWidth);
	}
	int pmax = src->GetLineEndPosition(lmax);
	src->InsertText(pmax,wxString("\n")+spos);
	int pmin = src->PositionFromLine(lmin);
	src->InsertText(pmin,spre+"\n");
	lmax+=2; pmax=src->GetLineEndPosition(lmax); 
	if (fix_indent) {
		src->Colourise(pmin,pmax);
		src->Indent(lmin,lmin);
		src->Indent(lmax,lmax);
	}
	if (spre.Contains("#here#")) {
		int p = src->GetLineIndentPosition(lmin) + spre.Index("#here#");
		src->SetTargetStart(p); src->SetTargetEnd(p+6); 
		src->ReplaceTarget(""); src->SetSelection(p,p);
	} else if (spos.Contains("#here#")) {
		int p = src->GetLineIndentPosition(lmax) + spos.Index("#here#");
		src->SetTargetStart(p); src->SetTargetEnd(p+6); 
		src->ReplaceTarget(""); src->SetSelection(p,p);
	}
}

int LocalRefactory::InsertText (mxSource * src, int line, wxString content, bool fix_indent) {
	int p = src->PositionFromLine(line);
	src->InsertText(p,content+"\n");
	int lmax = src->LineFromPosition(p+content.size());
	if (fix_indent) {
		src->Colourise(p,p+content.size());
		src->Indent(line,lmax);
	}
	return lmax-line+1;
}

