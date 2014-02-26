#ifndef MXGENERICHELPWINDOW_H
#define MXGENERICHELPWINDOW_H

#include <wx/frame.h>
#include <wx/sashwin.h>
#include <wx/html/htmlwin.h>
#include "mxUtils.h" // HashStringTreeItem

class wxTextCtrl;
class wxTreeCtrl;
class wxSashLayoutWindow;
class wxBoxSizer;
class wxHtmlEasyPrinting;

class mxHtmlWindow :public wxHtmlWindow {
public:
	mxHtmlWindow(wxWindow *parent, wxWindowID id, wxPoint position, wxSize size) : wxHtmlWindow(parent,id,position,size) {}
	void ScrollToAnchor(const wxString &anchor) { 
		wxHtmlWindow::ScrollToAnchor(anchor); // why was this method private in wxHtmlWidgetCell??
		ScrollLines(1);
	}
};

class mxGenericHelpWindow:public wxFrame {
	DECLARE_EVENT_TABLE();
protected:
	wxBoxSizer *general_sizer;
	wxBoxSizer *bottomSizer;
	mxHtmlWindow *html;
	wxTextCtrl *search_text;
	wxTreeCtrl *tree;
	wxSashLayoutWindow *index_sash;
	wxHtmlEasyPrinting *printer;
	
	void OnTree(wxTreeEvent &event);
	void OnLink (wxHtmlLinkEvent &event);
	void OnHideTree(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void OnHome(wxCommandEvent &event);
	void OnPrev(wxCommandEvent &event);
	void OnNext(wxCommandEvent &event);
	void OnCopy(wxCommandEvent &event);
	void OnPrint(wxCommandEvent &event);
	void OnSearch(wxCommandEvent &event);
	void OnSashDrag (wxSashEvent& event);
	void OnCharHook(wxKeyEvent &evt);
	
public:
	mxGenericHelpWindow(bool use_tree);
	~mxGenericHelpWindow();
	
	virtual void OnPrev() { html->HistoryBack(); }
	virtual void OnNext() { html->HistoryForward(); }
	virtual void ShowIndex()=0;
	virtual void OnSearch(wxString value){};
	virtual bool OnLink(wxString href){return true;};
	virtual void OnTree(wxTreeItemId item){};
	
};

#endif

