#ifndef MXHELPWINDOW_H
#define MXHELPWINDOW_H

#include <wx/frame.h>
#include <wx/sashwin.h>
#include <wx/html/htmlwin.h>
#include "mxUtils.h" // HashStringTreeItem

class wxHtmlWindow;
class wxTextCtrl;
class wxTreeCtrl;
class wxSashLayoutWindow;
class wxBoxSizer;
class wxHtmlEasyPrinting;

class mxHelpWindow:public wxFrame {
private:
	DECLARE_EVENT_TABLE();
	wxBoxSizer *bottomSizer;
	wxHtmlWindow *html;
	wxTextCtrl *search_text;
	wxTreeCtrl *tree;
	wxSashLayoutWindow *index_sash;
	HashStringTreeItem items;
	void OnTree(wxTreeEvent &event);
	void OnLink (wxHtmlLinkEvent &event);
	wxHtmlEasyPrinting *printer;
public:
	void ShowIndex();
	void ShowHelp(wxString file);
	void OnHideTree(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void OnHome(wxCommandEvent &event);
	void OnPrev(wxCommandEvent &event);
	void OnNext(wxCommandEvent &event);
	void OnCopy(wxCommandEvent &event);
	void OnPrint(wxCommandEvent &event);
	void OnForum(wxCommandEvent &event);
	void OnSearch(wxCommandEvent &event);
	void OnSearchAll(wxCommandEvent &event);
	void OnSashDrag (wxSashEvent& event);
	void OnCharHook(wxKeyEvent &evt);
	wxString GetHelpFile(wxString file);
	mxHelpWindow(wxString file="");
	~mxHelpWindow();
};

extern mxHelpWindow *helpw;
#define SHOW_HELP(page) if (helpw) helpw->ShowHelp(page); else helpw = new mxHelpWindow(page);

#endif

