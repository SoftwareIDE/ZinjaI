#ifndef VentanaPrincipal_H
#define VentanaPrincipal_H
#include <wx/frame.h>
#include <wx/image.h>

#if wxABI_VERSION<30000
#define wxImageResizeQuality int
#endif

enum State { stNULL, stMOVING };

class VentanaPrincipal:public wxFrame {
	wxImage img;
	int x0, y0; // win coord donde comienza el dibujo de la imagen
	float zoom; // factor de escala para dibujar la imagen (>1: agrandar, <1:achicar)
	State state;
	int mx,my;
	wxImageResizeQuality quality;
public:
	VentanaPrincipal(const wxString &fname);
	void OnPaint(wxPaintEvent &event);
	void OnWheel(wxMouseEvent &event);
	void OnMotion(wxMouseEvent &event);
	void OnLeftDown(wxMouseEvent &event);
	void OnLeftUp(wxMouseEvent &event);
	void OnRightUp(wxMouseEvent &event);
	void OnSaveAs(wxCommandEvent &event);
	void OnZoomOneOnOne(wxCommandEvent &event);
	void OnZoomExtend(wxCommandEvent &event);
	void OnChangeQuality(wxCommandEvent &event);
	
	void ImgCoordToWinCoord(int ix, int iy, int wx, int wy);
	void ZoomExtend();
	void ZoomOneOnOne();
	int WinCoortToImgCoordX(int wx);
	int WinCoortToImgCoordY(int wx);
	DECLARE_EVENT_TABLE();
};

#endif
