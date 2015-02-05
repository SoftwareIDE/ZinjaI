#ifndef MXINSPECTIONGRIDCELLEDITOR_H
#define MXINSPECTIONGRIDCELLEDITOR_H
#include <wx/textctrl.h>
#include <wx/menu.h>

/**
* @brief This wxTextCtrl derived class will substitute the one used in wxGridCellTextEditor to add an autocompletion feature
**/
class mxInspectionGridCellEditorControl:public wxTextCtrl {
	wxArrayString comp_options;
public:
	mxInspectionGridCellEditorControl(wxWindow* parent, wxWindowID id):wxTextCtrl(parent,id,"lala",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB|wxTE_AUTO_SCROLL|wxNO_BORDER) {}
	void Autocomplete();
	void OnMenu(wxCommandEvent &evt) {
		SetText(comp_options[evt.GetId()-mxID_LAST_ID-2000]);
	}
	void SetText(const wxString &text) {
		SetValue(text); 
		SetSelection(text.Len(),text.Len());
	}
	void OnChar(wxKeyEvent &evt) {
		if (evt.GetKeyCode()==WXK_TAB) Autocomplete(); 
		else evt.Skip();
	}
	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(mxInspectionGridCellEditorControl,wxTextCtrl)
	EVT_CHAR(mxInspectionGridCellEditorControl::OnChar)
	EVT_MENU_RANGE(mxID_LAST_ID+2000,mxID_LAST_ID+5000,mxInspectionGridCellEditorControl::OnMenu)
END_EVENT_TABLE()
	
/**
* @brief This class will replaces the event handler use by a grid cell editor, to intercept some events
*
* A cell editor use an event special handler that is passed by the grid (and whose implementation is hidden in the grid code).
* That event handler control things such as destroying the editor when the editor's control looses focus.
* My class will encapsulate that editor, and normally forward all the useful events. But, when the focus is lost
* due to the creation of the autocompletion menu, the generated kill focus event will be filtered out.
* mxInspectionGridCellEditorControl will enable/disable this filtering. The three events overriden by
* this class are the three that overrides the handler used regularly by the grid (wxGridCellEditorEvtHandler, 
* seen in wx/src/generic/grid.cpp, might change in the future and then be erroneous).
* Some less intrusive methods did not worked (such as chaining the event handlers with SetNextHandler).
**/
class myCellEditorEventHandler : public wxEvtHandler {
	static bool ignore_focus_kill_event;
	wxEvtHandler *m_evt;
	mxInspectionGridCellEditorControl *m_text;
public:
	myCellEditorEventHandler(wxEvtHandler *evt, mxInspectionGridCellEditorControl*text):m_evt(evt),m_text(text) { /*SetNextHandler(evt); */}
	void OnKeyDown( wxKeyEvent& event ) {
		if (event.GetKeyCode()==WXK_TAB)
			m_text->Autocomplete();
		else
			m_evt->ProcessEvent(event);
	}
	void OnChar( wxKeyEvent& event ) { 
		m_evt->ProcessEvent(event);
	}
	void OnKillFocus(wxFocusEvent& event) {
		if (!ignore_focus_kill_event) m_evt->ProcessEvent(event);
	}
	static void IgnoreFocusKill(bool ignore) { ignore_focus_kill_event=ignore; }
	~myCellEditorEventHandler() { delete m_evt; }
	DECLARE_EVENT_TABLE();
};

bool myCellEditorEventHandler::ignore_focus_kill_event = false;

BEGIN_EVENT_TABLE(myCellEditorEventHandler,wxEvtHandler)
	EVT_KILL_FOCUS(myCellEditorEventHandler::OnKillFocus)
	EVT_KEY_DOWN(myCellEditorEventHandler::OnKeyDown )
	EVT_CHAR(myCellEditorEventHandler::OnChar)
END_EVENT_TABLE()
	
	
void mxInspectionGridCellEditorControl::Autocomplete() {
	if (GetValue().IsEmpty()) return;
	if (!debug->CanTalkToGDB()) return;
	comp_options.Clear(); 
	wxString ans = debug->SendCommand("complete p ",GetValue());
	while (ans.Contains("\n")) {
		wxString line=ans.BeforeFirst('\n');
		ans=ans.AfterFirst('\n');
		if (line.StartsWith("~\"")) {
			wxString ue=mxUT::UnEscapeString(line.Mid(1));
			while (ue.Len() && (ue.Last()=='\n'||ue.Last()=='\r')) ue.RemoveLast();
			comp_options.Add(ue.Mid(2));
		}
	}
	if (!comp_options.GetCount()) return;
	if (comp_options.GetCount()==1) { SetText(comp_options[0]); return; }
	else {
		wxString common_part=comp_options[0];
		for(unsigned int i=1;i<comp_options.GetCount();i++) { 
			int j=0, l1=comp_options[i].Len(), l2=common_part.Len();
			while (j<l1 && j<l2 && comp_options[i][j]==common_part[j]) j++;
			if (j<l2) common_part=common_part.Mid(0,j);
		}
		if (common_part!=GetValue()) SetValue(common_part);
	}
	wxMenu menu("");
	for(unsigned int i=0;i<comp_options.GetCount();i++)
		menu.Append(mxID_LAST_ID+2000+i, comp_options[i]);
	myCellEditorEventHandler::IgnoreFocusKill(true);
	wxRect r = GetScreenRect();
	PopupMenu(&menu,0,r.height);
	myCellEditorEventHandler::IgnoreFocusKill(false);
}

/**
* @brief The actual cell editor used by Expression column. 
*
* This one replaces the text control used in wxGridCellTextEditor by a
* different one that adds an autocompletion feature. That feature will
* be triggered by TAB key, so this class also changes the event handler
* provided by the grid by a custom one that will filter some events in
* order to launch the autocompletion menu, and to avoid that the text's
* lost of focus produced by the menu kill finishes the edition.
**/
class gdbInspCtrl:public wxGridCellTextEditor {
public:
	void Create(wxWindow* parent, wxWindowID id, wxEvtHandler* evtHandler) {
		m_control = new mxInspectionGridCellEditorControl(parent, id);
		wxGridCellEditor::Create(parent, id, new myCellEditorEventHandler(evtHandler,(mxInspectionGridCellEditorControl*)m_control));
	}
	// I don't know if this method is required, but all the editors provided by wx have it
	virtual wxGridCellEditor *Clone() const	{ return new gdbInspCtrl(); }
};

#endif
