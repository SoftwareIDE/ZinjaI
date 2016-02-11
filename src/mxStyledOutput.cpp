#include "mxStyledOutput.h"
#include "ConfigManager.h"
#include "mxColoursEditor.h"

BEGIN_EVENT_TABLE(mxStyledOutput,wxStyledTextCtrl)
	EVT_STC_MARGINCLICK(wxID_ANY,mxStyledOutput::OnMarginClick)
	EVT_MOUSEWHEEL(mxStyledOutput::OnMouseWheel)
END_EVENT_TABLE()
	
void mxStyledOutput::AppendText (const wxString & str) {
	int n0 = GetLineCount()-1;
	if (is_read_only) SetReadOnly(false);
	wxStyledTextCtrl::AppendText(str);
	if (is_read_only) SetReadOnly(true);
	int n1 = GetLineCount()-1;
	for(int i=n0;i<n1;i++) { 
		if (StartCollapsibleSection(GetLine(i))) {
			MarkerAdd(i,0);
			MarkerAdd(i,2);
		}
	}
	GotoLine(n1-1);
}

void mxStyledOutput::OnMarginClick (wxStyledTextEvent & e) {
	int l = LineFromPosition (e.GetPosition());
	int m = MarkerGet(l);
	if (m) {
		int n=GetLineCount();
		int l2=l+1; while (l2<n && !MarkerGet(l2)) l2++; l2--;
		if (m&1) {
			MarkerDelete(l,0);
			MarkerAdd(l,1);
			HideLines(l+1,l2);
		} else {
			MarkerDelete(l,1);
			MarkerAdd(l,0);
			ShowLines(l+1,l2);
		}
	}
	AfterMarginClick();
}

mxStyledOutput::mxStyledOutput (wxWindow * parent, bool read_only, bool wrap_lines) 
	: wxStyledTextCtrl(parent,wxID_ANY), is_read_only(read_only)
{
//	wxFont font(config->Styles.font_size, wxMODERN, wxNORMAL, wxNORMAL);
//	StyleSetFont (wxSTC_STYLE_DEFAULT, font);
	
	StyleSetFontAttr(wxSTC_STYLE_DEFAULT,config->Styles.font_size,config->Styles.font_name,false,false,false);
	
	MarkerDefine(0,wxSTC_MARK_MINUS, wxColour(255,255,255), wxColour(0,0,0));
	MarkerDefine(1,wxSTC_MARK_PLUS, wxColour(255,255,255), wxColour(0,0,0));
	wxColour mcolor = g_ctheme->CURRENT_LINE;
	float alpha=35.f/255.f; int white=255*(1-alpha);
	mcolor.Set(white+mcolor.Red()*alpha,white+mcolor.Green()*alpha,white+mcolor.Blue()*alpha,255);
	MarkerDefine(2,wxSTC_MARK_BACKGROUND,mcolor,mcolor);
	SetMarginWidth (0, 0);
	SetMarginWidth (1, 16);
	SetMarginSensitive (1, true);
	SetReadOnly(read_only);
	SetWrapMode(wrap_lines?wxSTC_WRAP_WORD:wxSTC_WRAP_NONE);
}

void mxStyledOutput::AppendLine (const wxString & str) {
	mxStyledOutput::AppendLine(str,StartCollapsibleSection(str));
}

void mxStyledOutput::AppendLine (const wxString & str, bool start_collapsible_section) {
	int line_num = GetLineCount()-1;
	if (is_read_only) SetReadOnly(false);
	wxStyledTextCtrl::AppendText(str+"\n");
	if (is_read_only) SetReadOnly(true);
	if (start_collapsible_section) { MarkerAdd(line_num,0); MarkerAdd(line_num,2); }
}

void mxStyledOutput::OnMouseWheel (wxMouseEvent & event) {
	if (event.ControlDown()) {
		if (event.m_wheelRotation>0) {
			ZoomIn();
		} else {
			ZoomOut();
		}
	} else
		event.Skip();
}
