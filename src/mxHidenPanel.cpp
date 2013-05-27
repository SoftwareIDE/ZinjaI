#include <wx/dcclient.h>
#include <wx/settings.h>
#include <wx/aui/aui.h>
#include "mxHidenPanel.h"
#include "mxMainWindow.h"
#include "mxWelcomePanel.h"

BEGIN_EVENT_TABLE(mxHidenPanel,wxPanel)
	EVT_PAINT(mxHidenPanel::OnPaint)
	EVT_ENTER_WINDOW(mxHidenPanel::OnMotion)
	EVT_LEAVE_WINDOW(mxHidenPanel::OnMotion)
	EVT_RIGHT_DOWN(mxHidenPanel::OnClick)
	EVT_LEFT_DOWN(mxHidenPanel::OnClick)
	EVT_TIMER(wxID_ANY,mxHidenPanel::OnTimer)
END_EVENT_TABLE()

//mxHidenPanel *mxHidenPanel::lockeds[3]={NULL,NULL,NULL};

int mxHidenPanel::used_bottom=0;
int mxHidenPanel::used_bottom_left=0;
int mxHidenPanel::used_bottom_right=0;
int mxHidenPanel::used_right=0;
int mxHidenPanel::used_right_bottom=0;
int mxHidenPanel::used_left=0;
int mxHidenPanel::used_left_bottom=0;
	
bool mxHidenPanel::ignore_autohide=false;	
	
mxHidenPanel::mxHidenPanel(wxWindow *parent, wxWindow *acontrol, hp_pos apos, wxString alabel):wxPanel(parent,wxID_ANY,wxDefaultPosition,wxSize(15,15)) {
	label=alabel; selected=false; control=acontrol; pos=apos; forced_show=mouse_in=false; showing=false;
	if (pos!=HP_BOTTOM) { 
		unsigned int i=1; 
		while (i<label.Len()) { 
			label=label.Mid(0,i)+"\n"+label.Mid(i); 
			i+=2;
		}
	}
	timer = new wxTimer(GetEventHandler(),wxID_ANY);
}

void mxHidenPanel::OnPaint(wxPaintEvent &evt) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.SetBackground(wxSystemSettings::GetColour(selected?wxSYS_COLOUR_3DSHADOW:(mouse_in||showing?wxSYS_COLOUR_3DHIGHLIGHT:wxSYS_COLOUR_3DFACE)));
	dc.Clear();
	dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	int w,h; GetClientSize(&w,&h);
	dc.DrawLabel(label,wxRect(0,0,w,h),wxALIGN_CENTER);
}

void mxHidenPanel::ToggleDock() {
	if (selected) 
		ShowFloat(true);
	else 
		ShowDock();
}

void mxHidenPanel::ShowFloat(bool set_focus) {
	wxAuiPaneInfo &pane = main_window->aui_manager.GetPane(control);
	int px,py,pw,ph,ax,ay,aw,ah;
	if (welcome_panel && welcome_panel->IsVisible()) {
		welcome_panel->GetScreenPosition(&px,&py);
		welcome_panel->GetSize(&pw,&ph);
	} else {
		main_window->notebook_sources->GetScreenPosition(&px,&py);
		main_window->notebook_sources->GetSize(&pw,&ph);
	}
	GetScreenPosition(&ax,&ay);
	GetSize(&aw,&ah);
	if (pos==HP_LEFT) {
		px=ax+aw;
		pw/=4;
		if (used_bottom && used_bottom_left<px+pw) ph-=used_bottom;
		used_left=pw;
		used_left_bottom=px+ph;
	} else if (pos==HP_BOTTOM) {
		ph/=3;
		py=ay-ph/*-aui_manager.GetArtProvider()->GetMetric(wxAUI_DOCKART_CAPTION_SIZE)*/;
		if (used_left && used_left_bottom>px) { pw-=used_left; px+=used_left; }
		if (used_right && used_right_bottom<px+pw) pw-=used_right;
		used_bottom=ph;
		used_bottom_left=px;
		used_bottom_right=px+pw;
	} else if (pos==HP_RIGHT) {
		pw/=control==(wxWindow*)main_window->inspection_ctrl?3:4;
		px=ax-pw;
		if (used_bottom && used_bottom_right>px) ph-=used_bottom;
		used_right=pw;
		used_right_bottom=px;
	}
	pane.Show();
	pane.Float();
	pane.MinSize(wxSize(10,10));
	pane.FloatingSize(pw,ph);
	pane.FloatingPosition(px,py);
	timer->Start(200,false);
	selected=false; showing=true; Refresh();
	main_window->aui_manager.Update();
	if (!set_focus) main_window->Raise(); // los paneles flotando le sacan el foco a la main window y no se soluciona con setfocus
}

void mxHidenPanel::ShowDock() {
	timer->Stop();
	wxAuiPaneInfo &pane = main_window->aui_manager.GetPane(control);
	pane.Show();
	pane.Row(5);
	pane.Position(5);
	pane.Dock();
	if (pos==HP_LEFT) pane.Left();
	else if (pos==HP_BOTTOM) pane.Bottom();
	else if (pos==HP_RIGHT) pane.Right();
	showing=true;
	main_window->aui_manager.Update();
	selected=true; Refresh();
}

void mxHidenPanel::Hide() {
	if (pos==HP_LEFT) used_left=0;
	if (pos==HP_BOTTOM) used_bottom=0;
	if (pos==HP_RIGHT) used_right=0;
	timer->Stop();
	selected=false; mouse_in=false; forced_show=false;
	main_window->aui_manager.GetPane(control).Hide();
	main_window->aui_manager.Update();
	showing=false; selected=false; Refresh();
}

void mxHidenPanel::ToggleFull() {
	if (selected) return;
	wxAuiPaneInfo &pane = main_window->aui_manager.GetPane(control);
	int px,py,pw,ph;
	main_window->notebook_sources->GetScreenPosition(&px,&py);
	main_window->notebook_sources->GetSize(&pw,&ph);
	int ow,oh; control->GetSize(&ow,&oh);
	if (showing&&ow>2*pw/3&&oh>2*ph/3) {
		Hide();
		return;
	}
	pane.Show();
	pane.Float();
	pane.FloatingSize(pw,ph);
	pane.FloatingPosition(px,py);
	showing=true;
	main_window->aui_manager.Update();
}

void mxHidenPanel::ProcessClose() {
	showing=false; 
	if (selected) { selected=false; Refresh(); }
}

void mxHidenPanel::ForceShow(bool set_focus) {
	forced_show=true;
	if (selected) {
		if (set_focus) Raise();
		if (set_focus) SetFocus();
		if (set_focus) control->Raise();
		if (set_focus) control->SetFocus();
		return;
	}
	if  (!main_window->aui_manager.GetPane(control).IsShown()) 
		ShowFloat(set_focus);
}

void mxHidenPanel::OnClick(wxMouseEvent &evt) {
	if (evt.GetButton()==wxMOUSE_BTN_LEFT) {
		ToggleDock();
	} else {
		if (selected) return;
		ToggleFull();
	}
}

void mxHidenPanel::OnMotion(wxMouseEvent &evt) {
	if (selected) return;
	mouse_in=evt.GetEventType()==wxEVT_ENTER_WINDOW;
	timer->Start(200,false); Refresh();
}

void mxHidenPanel::OnTimer(wxTimerEvent &evt) {
	if (selected || ignore_autohide) return;
	if (showing) {
		if (forced_show) 
			forced_show=!control->GetScreenRect().Contains(wxGetMousePosition());
		else if (!mouse_in && !control->GetScreenRect().Contains(wxGetMousePosition())) {
			Hide();
		}
	} else {
		if (mouse_in && main_window->IsActive()) ShowFloat(false);
	}
}

void mxHidenPanel::ProcessParentResize() {
	if (!selected && showing) Hide();
}

bool mxHidenPanel::IsDocked() {
	return selected;
}
void mxHidenPanel::Select() {
	ShowDock();
	// ver como hacer para que si estaba flotando aparezca otra vez en el mismo lugar
//	wxAuiPaneInfo &pane = main_window->aui_manager.GetPane(control);
//	if (pane.IsDocked()) ShowDock();
//	else { selected=true; pane.Show(); main_window->aui_manager.Update(); }
}
