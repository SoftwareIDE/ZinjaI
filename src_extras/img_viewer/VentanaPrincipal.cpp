#include "VentanaPrincipal.h"
#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <iostream>
#include <algorithm>
#include <wx/menu.h>
#include <wx/filedlg.h>
using namespace std;

BEGIN_EVENT_TABLE(VentanaPrincipal,wxFrame)
	EVT_PAINT(VentanaPrincipal::OnPaint)
	EVT_MOUSEWHEEL(VentanaPrincipal::OnWheel)
	EVT_LEFT_DOWN(VentanaPrincipal::OnLeftDown)
	EVT_LEFT_UP(VentanaPrincipal::OnLeftUp)
	EVT_RIGHT_UP(VentanaPrincipal::OnRightUp)
	EVT_MOTION(VentanaPrincipal::OnMotion)
	EVT_MENU(wxID_SAVEAS,VentanaPrincipal::OnSaveAs)
	EVT_MENU(wxID_ZOOM_FIT,VentanaPrincipal::OnZoomExtend)
	EVT_MENU(wxID_ZOOM_100,VentanaPrincipal::OnZoomOneOnOne)
	EVT_MENU(wxID_PREFERENCES,VentanaPrincipal::OnChangeQuality)
END_EVENT_TABLE()

VentanaPrincipal::VentanaPrincipal(const wxString &fname):wxFrame(NULL,wxID_ANY,fname,wxDefaultPosition,wxSize(800,600),wxDEFAULT_FRAME_STYLE) {
	quality = wxIMAGE_QUALITY_NORMAL;
	img.LoadFile(fname);
	wxSize sz = wxSize(img.GetWidth(),img.GetHeight());
	wxSize wz = WindowToClientSize(wxSize(800,600));
	if (sz.GetWidth()>wz.GetWidth()||sz.GetHeight()>wz.GetHeight()) {
		ZoomExtend();
	} else {
		SetSize(ClientToWindowSize(sz));
		ZoomOneOnOne();
	}
	state = stNULL;
	Show();
}

void VentanaPrincipal::OnPaint (wxPaintEvent & event) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	wxSize dc_sz = dc.GetSize();
	int w = dc_sz.GetWidth(), h = dc_sz.GetHeight();
	dc.SetBackground(wxBrush(wxColour(255,255,255)));
	dc.Clear();
	int ix0 = WinCoortToImgCoordX(0);
	int iy0 = WinCoortToImgCoordY(0);
	if (ix0>=img.GetWidth() || iy0>=img.GetHeight()) return;
	int ix1 = WinCoortToImgCoordX(w);
	int iy1 = WinCoortToImgCoordY(h);
	if (ix1<0||iy1<0) return;
	int dx=0, dy=0;
	if (ix0<0) { dx=x0; ix0=0; }
	if (iy0<0) { dy=y0; iy0=0; }
	if (ix1>img.GetWidth()) ix1=img.GetWidth();
	if (iy1>img.GetHeight()) iy1=img.GetHeight();
	wxRect rect(ix0,iy0,ix1-ix0,iy1-iy0);
	dc.DrawBitmap(wxBitmap(img.GetSubImage(rect).Rescale(rect.GetWidth()*zoom,rect.GetHeight()*zoom,quality)),dx,dy,false);
}

void VentanaPrincipal::OnWheel (wxMouseEvent & event) {
	mx = WinCoortToImgCoordX(event.GetX());
	my = WinCoortToImgCoordY(event.GetY());
	if (event.m_wheelRotation<0) zoom=zoom*1.1;
	else zoom=zoom/1.1;
	ImgCoordToWinCoord(mx,my,event.GetX(),event.GetY());
	Refresh();
}

void VentanaPrincipal::OnMotion (wxMouseEvent & event) {
	if (state==stMOVING) {
		ImgCoordToWinCoord(mx,my,event.GetX(),event.GetY());
		Refresh();
	}
}

void VentanaPrincipal::OnLeftDown (wxMouseEvent & event) {
	state=stMOVING;
	mx = WinCoortToImgCoordX(event.GetX());
	my = WinCoortToImgCoordY(event.GetY());
}

void VentanaPrincipal::OnLeftUp (wxMouseEvent & event) {
	state=stNULL;
}

void VentanaPrincipal::ImgCoordToWinCoord (int ix, int iy, int wx, int wy) {
	x0 = wx-ix*zoom;
	y0 = wy-iy*zoom;
}

int VentanaPrincipal::WinCoortToImgCoordX (int wx) {
	return (wx-x0)/zoom;
}

int VentanaPrincipal::WinCoortToImgCoordY (int wy) {
	return (wy-y0)/zoom;
}

void VentanaPrincipal::ZoomExtend ( ) {
	wxSize sz = wxSize(img.GetWidth(),img.GetHeight());
	wxSize wz = WindowToClientSize(GetSize());
	float zoom_x = float(wz.GetWidth())/sz.GetWidth();
	float zoom_y = float(wz.GetHeight())/sz.GetHeight();
	zoom = min(zoom_x,zoom_y);
	ImgCoordToWinCoord(sz.GetWidth()/2,sz.GetHeight()/2,wz.GetWidth()/2,wz.GetHeight()/2);
}

void VentanaPrincipal::ZoomOneOnOne () {
	zoom=1;
	wxSize sz = wxSize(img.GetWidth(),img.GetHeight());
	wxSize wz = WindowToClientSize(GetSize());
	ImgCoordToWinCoord(sz.GetWidth()/2,sz.GetHeight()/2,wz.GetWidth()/2,wz.GetHeight()/2);
}

void VentanaPrincipal::OnRightUp (wxMouseEvent & event) {
	wxMenu menu(_T(""));
	menu.Append(wxID_ZOOM_FIT,_T("Zoom Fit"));
	menu.Append(wxID_ZOOM_100,_T("Zoom 1:1"));
	menu.AppendCheckItem(wxID_PREFERENCES,_T("High quality scaling"))->Check(quality==wxIMAGE_QUALITY_HIGH);
	menu.AppendSeparator();
	menu.Append(wxID_SAVEAS,_T("Save as..."));
	PopupMenu(&menu);
}

void VentanaPrincipal::OnSaveAs (wxCommandEvent & event) {
	wxFileDialog dlg(this,_T("Save as"),_T(""),_T(""),
		_T("Image PNG|*.png;*.PNG|Image JPEG|*.jpg;*.jpeg;*.JPG;*.JPEG|Image BMP|*.bmp;*.BMP"),
		wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_OK) img.SaveFile(dlg.GetPath());
}

void VentanaPrincipal::OnZoomOneOnOne (wxCommandEvent & event) {
	ZoomOneOnOne(); Refresh();
}

void VentanaPrincipal::OnZoomExtend (wxCommandEvent & event) {
	ZoomExtend(); Refresh();
}

void VentanaPrincipal::OnChangeQuality (wxCommandEvent & event) {
	if (quality==wxIMAGE_QUALITY_NORMAL) quality=wxIMAGE_QUALITY_HIGH;
	else quality=wxIMAGE_QUALITY_NORMAL;
	Refresh();
}

