#include <wx/frame.h>
#include <wx/dc.h>
#include "mxCalltip.h"
#include "mxSource.h"
#include "mxColoursEditor.h"
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxCalltip,wxFrame)
	EVT_PAINT(mxCalltip::OnPaint)
	EVT_SET_FOCUS(mxCalltip::OnFocus)
//	EVT_ACTIVATE(mxCalltip::OnFocus)
//	EVT_LEFT_DOWN(wxSTCCallTip::OnLeftDown)
END_EVENT_TABLE()

mxCalltip::mxCalltip (mxSource * src) : 
	wxFrame(src, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFRAME_NO_TASKBAR|wxFRAME_FLOAT_ON_PARENT|wxBORDER_NONE), 
	parent(src), current_arg(-1)
{
	Disable(); // workaround para el no-efecto de AcceptsFocus, ver OnFocus
}

void mxCalltip::OnPaint (wxPaintEvent & event) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.SetBackground(ctheme->CALLTIP_BACK);
	my_font.SetWeight(wxFONTWEIGHT_NORMAL); 
	dc.Clear(); dc.SetFont(my_font);
//	dc.SetTextBackground(*back);
	dc.SetTextForeground(ctheme->CALLTIP_FORE);
	int w,h; GetClientSize(&w,&h);
	
	int cur_y=1;
	for(int i=0;i<entries.GetSize();i++) {
		if (!entries[i].ShouldDraw(current_arg)) continue;
		dc.DrawText(entries[i].line,2,cur_y); cur_y+=char_h;
	}
	my_font.SetWeight(wxFONTWEIGHT_BOLD); 
	dc.SetFont(my_font);
	cur_y=1;
	for(int i=0;i<entries.GetSize();i++) {
		if (!entries[i].ShouldDraw(current_arg)) continue;
		dc.DrawText(entries[i].bold_line,2,cur_y); cur_y+=char_h;
	}
	
	
	wxSize sz = dc.GetSize();
	dc.SetPen(wxPen(ctheme->CALLTIP_FORE));
	dc.DrawLine(0				,0					,sz.GetWidth()-1	,0);
	dc.DrawLine(0				,sz.GetHeight()-1	,sz.GetWidth()-1	,sz.GetHeight()-1);
	dc.DrawLine(0				,0					,0					,sz.GetHeight()-1);
	dc.DrawLine(sz.GetWidth()-1	,0					,sz.GetWidth()-1	,sz.GetHeight()-1);
}

void mxCalltip::ShowCommon (wxString text) {
	my_font = wxFont(config->Styles.font_size+parent->GetZoom(), wxMODERN, wxNORMAL, wxNORMAL,false,config->Styles.font_name);
	entries.Clear();
	int p = text.Index('\n');
	while (p!=wxNOT_FOUND) {
		if (0!=p) entries.Add(text.Mid(0,p));
		text=text.Mid(p+1);
		p = text.Index('\n');
	}
	if (!text.IsEmpty()) entries.Add(text);
	if (entries.GetSize()==0) return;
	SetArg(-2);
	wxFrame::Show();
	main_window->Raise();
}

void mxCalltip::Show (int source_pos, const wxString &text) {
	wxPoint pt = parent->PointFromPosition(source_pos);
	wxPoint sp=parent->GetScreenPosition();
	pos_x=sp.x+pt.x; pos_y=sp.y+pt.y;
	delta_chars=0;
	ShowCommon(text);
}

void mxCalltip::Show (int x, int y, int delta_chars, const wxString &text) {
	pos_x = x; pos_y = y;
	this->delta_chars=delta_chars;
	ShowCommon(text);
}

void mxCalltip::SetArg (int cur_arg) {
	if (current_arg==cur_arg) return;
	if (cur_arg==-2) cur_arg=-1;
	current_arg=cur_arg;
	if (entries.GetSize()==0) return;
	// determinar cual es la linea mas larga para ver si entra en la ventana
	int max_len=entries[0].len, max_i=0;
	for(int i=1;i<entries.GetSize();i++)
		if (entries[i].len>max_len) { 
			max_len=entries[i].len; max_i=i;
		}
	// obtener tama�o de letrasa para calcular el tama�o de la ventana y la longitud de las lineas
	wxMemoryDC dc; dc.SetFont(my_font);
	wxSize sz = dc.GetTextExtent(entries[max_i].line);
	char_h = sz.GetHeight(); 
	int char_w=sz.GetWidth()/max_len;
	wxRect mwr = main_window->GetScreenRect();
//	max_line = (mwr.GetLeft()+mwr.GetWidth()-pos_x-sp.x)/char_w-1; if (max_line<5) max_line=5;
	// ver cuales lineas se van a mostrar y cuan largas son
	int cant_lines=0; max_len=0;
	for(int i=0;i<entries.GetSize();i++) {
		if (!entries[i].ShouldDraw(cur_arg)) continue;
		cant_lines++;
//		if (entries[i].len>max_line) {
//			cant_lines += (entries[i].len/max_line);
//			max_len=max_line;
//		}
//		else 
			if (entries[i].len>max_len) max_len=entries[i].len;
		if (cur_arg==-1||entries[i].argc==-1) { entries[i].bold_line.Clear(); continue; }
		entries[i].bold_line=entries[i].line;
		for (int j=0;j<entries[i].argp[cur_arg]+1;j++) entries[i].bold_line[j]=' ';
		for (int j=entries[i].argp[cur_arg+1];j<entries[i].len;j++) entries[i].bold_line[j]=' ';
	}
	if (cant_lines==0) cant_lines=max_len=1;
	// redimensionar acorde a lo que se conto, y controlar que no se salga de la pantalla 
	// (delta_char!=0 indica que es para el autocompletado)
	// (como no se sabe el tama�o de la pantalla, se asume de 0,0 a la esquina inferior derecha de main_window)
	
	int delta_w=0;
	if (delta_chars) {
		int vscrool_x = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
		delta_w=delta_chars*char_w+2*wxSystemSettings::GetMetric(wxSYS_VSCROLL_X)+10;
		if (delta_w<100-30+vscrool_x) delta_w=100-30+vscrool_x;
		if (delta_w>350-30+vscrool_x) delta_w=350-30+vscrool_x;; // 100  y 250 salen de los fuentes del stc
	}
	int my_w=max_len*char_w, my_min_x=pos_x+delta_w, win_min_x=mwr.GetLeft(), win_h=mwr.GetHeight();
	int my_h=cant_lines*char_h, my_min_y=pos_y+char_h+2;
	int my_max_x=my_min_x+my_w, win_max_x=win_min_x+mwr.GetWidth(), win_max_y=mwr.GetTop()+win_h;
	if (my_max_x>win_max_x) { 
		if (delta_chars) {
			int new_min_x = my_min_x = pos_x-my_w-30;
			if (new_min_x>=0) my_min_x = new_min_x;
		}
		else {
			my_min_x=win_max_x-my_w; 
			if (my_min_x<0) my_min_x=0;
		}
	}
	if (my_min_y+my_h>win_max_y) {
		if (delta_chars) {
			my_min_y = win_max_y-my_h;
			if (my_min_y<0) my_min_y=0;
		} else {
			int new_min_y = pos_y-2-my_h;
			if (new_min_y>0) my_min_y=new_min_y;
		} 
	}
	SetSize(my_w+4,my_h+4);
	Move(my_min_x,my_min_y);
	Refresh();
}

mxCalltip::entry::entry (const wxString & s) : line (s) {
	int i=0,l=len=s.Len(), par=0; argc=-1; bool comillas=false; 
	int last_par_open=-1;
	while(i<l) {
		if (s[i]=='\''||s[i]=='\"') {
			comillas=!comillas;
		} else if (comillas) {
			if (s[i]=='\\') i++;
		} else {
			if (s[i]=='(') {
				if (par==0) last_par_open=i;
				par++;
			} else if (s[i]==')') {
				par--;
			}
		}
		i++;
	}
	if (last_par_open==-1) return;
	par=0; comillas=false; i=last_par_open+1; argc=0;
	argp[0]=last_par_open;
	while(i<l) {
		if (s[i]=='\''||s[i]=='\"') {
			comillas=!comillas;
		} else if (comillas) {
			if (s[i]=='\\') i++;
		} else {
			if (s[i]=='(') {
				par++;
			} else if (par==0 && (s[i]==','||s[i]==')')) {
				argp[++argc]=i;
				if (argc==49) { argc=-1; return; }
			} else if (s[i]==')') {
				par--;
				if (par<0) { argp[++argc]=i; break; }
			}
		}
		i++;
	}
}

void mxCalltip::Hide() {
	wxFrame::Hide();
}


void mxCalltip::OnFocus (wxFocusEvent & event) {
	// esto evita un "flickering" en la barra de titulo de la main_window cuando pierde su foco
	// solo funciona (aparentemente) en gnu/linux... 
	// la solucion "oficial" de sobreescribir el metodo AcceptsFocus no funciona en ningun caso, pero
	// la implemntacion original dice "return IsShown()&&IsEnabled()", por eso el "Disable()" del constructor
	parent->SetFocus();
	event.Skip();
}

