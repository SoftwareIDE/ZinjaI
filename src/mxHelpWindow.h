#ifndef MXHELPWINDOW_H
#define MXHELPWINDOW_H
#include "mxGenericHelpWindow.h"

class wxDialog;

class mxHelpWindow:public mxGenericHelpWindow {
	
	static mxHelpWindow *instance;
	mxHelpWindow(wxString file="");
	HashStringTreeItem items;
	
	wxString GetHelpFile(wxString file);
	
public:
	static void ShowHelp(wxString page="", wxDialog *from_modal=NULL);
	
	void LoadHelp(wxString file);
	
	void ShowIndex();
	void OnTree(wxTreeItemId item);
	void OnSearch(wxString value);
	bool OnLink(wxString href);
	
	void OnForum(wxCommandEvent &event);
	
	/// @brief Wrapper for wxHtmlWindow::LoadPage that fix href path (it enters relative to current page, but current page's path is not current working directory)
	void FixLoadPage(const wxString &href); 
	
private:
	bool ignore_tree_event;
public:
	/// @brief Selects an iden in the index tree skipping its selecion event callback
	void SelectTreeItem(const wxString &fname);
	
	
	~mxHelpWindow();
};

#endif
