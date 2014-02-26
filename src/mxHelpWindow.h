#ifndef MXHELPWINDOW_H
#define MXHELPWINDOW_H
#include "mxGenericHelpWindow.h"

class mxHelpWindow:public mxGenericHelpWindow {
	static mxHelpWindow *instance;
	mxHelpWindow(wxString file="");
	HashStringTreeItem items;
	
	wxString GetHelpFile(wxString file);
	
public:
	static void ShowHelp(wxString page="");
	
	void LoadHelp(wxString file);
	
	void ShowIndex();
	void OnTree(wxTreeItemId item);
	void OnSearch(wxString value);
	bool OnLink(wxString href);
	
	~mxHelpWindow();
};

#define SHOW_HELP(page) mxHelpWindow::ShowHelp(page)

#endif

