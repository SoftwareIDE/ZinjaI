#ifndef MXREFERENCEWINDOW_H
#define MXREFERENCEWINDOW_H
#include "mxGenericHelpWindow.h"
#include <stack>
#include <map>
#include <vector>
using namespace std;

class mxReferenceWindow:public mxGenericHelpWindow {
	
	vector< pair<wxString,wxString> > search_index;
	void LoadSearchIndex();
	
	wxTreeItemId page_tree_item;
	void PopulateInitialTree();
	
	vector<pair<wxTreeItemId,wxString> > items_general,items_page;
	
	static mxReferenceWindow *instance;
	mxReferenceWindow(wxString file="");
	wxString current_path;
	wxString current_page;
	
	stack<wxString> history_prev;
	stack<wxString> history_next;
	
public:
	static void ShowHelp(wxString page="");
	static wxString ProcessHTML(wxString fname, mxReferenceWindow *w=NULL);
	
	void LoadHelp(wxString file, bool update_history=true);
	
	void OnPrev();
	void OnNext();
	void OnTree(wxTreeItemId item);
	void ShowIndex();
	void OnSearch(wxString value);
	bool OnLink(wxString href);
	
	~mxReferenceWindow();
};

#endif

