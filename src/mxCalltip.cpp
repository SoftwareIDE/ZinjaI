#include <wx/frame.h>
#include <wx/dc.h>
#include "mxCalltip.h"
#include "mxSource.h"
#include "mxColoursEditor.h"
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxCalltip,wxFrame)
	EVT_PAINT(mxCalltip::OnPaint)
//	EVT_SET_FOCUS(mxCalltip::OnFocus)
//	EVT_ACTIVATE(mxCalltip::OnFocus)
//	EVT_LEFT_DOWN(wxSTCCallTip::OnLeftDown)
END_EVENT_TABLE()

mxCalltip::mxCalltip (mxSource * src) : 
	wxFrame(src, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFRAME_NO_TASKBAR|wxFRAME_FLOAT_ON_PARENT|wxBORDER_NONE), 
	parent(src), current_arg(-1)
{
}

void mxCalltip::OnPaint (wxPaintEvent & event) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.SetBackground(ctheme->CALLTIP_BACK);
	wxFont f1(config->Styles.font_size, wxMODERN, wxNORMAL, wxNORMAL,false,config->Styles.font_name);
	dc.Clear(); dc.SetFont(f1);
//	dc.SetTextBackground(*back);
	dc.SetTextForeground(ctheme->CALLTIP_FORE);
	int w,h; GetClientSize(&w,&h);
	
	int cur_y=0;
	for(int i=0;i<entries.GetSize();i++) {
		if (!entries[i].ShouldDraw(current_arg)) continue;
		dc.DrawText(entries[i].line,0,cur_y); cur_y+=char_h;
	}
	f1.SetWeight(wxFONTWEIGHT_BOLD); dc.SetFont(f1);
	cur_y=0;
	for(int i=0;i<entries.GetSize();i++) {
		if (!entries[i].ShouldDraw(current_arg)) continue;
		dc.DrawText(entries[i].bold_line,0,cur_y); cur_y+=char_h;
	}
}

void mxCalltip::ShowCommon (wxString text) {
	entries.Clear();
	int p = text.Index('\n');
	while (p!=wxNOT_FOUND) {
		entries.Add(text.Mid(0,p));
		text=text.Mid(p+1);
		p = text.Index('\n');
	}
	entries.Add(text);
	SetArg(-2);
	wxFrame::Show();
	main_window->Raise();
}

void mxCalltip::Show (int sp, const wxString &text) {
	wxPoint pt = parent->PointFromPosition(sp);
	pos_x=pt.x; pos_y=pt.y;
	ShowCommon(text);
}

void mxCalltip::Show (int x, int y, const wxString &text) {
	pos_x = x; pos_y = y;
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
	// obtener tamaño de letrasa para calcular el tamaño de la ventana y la longitud de las lineas
	wxMemoryDC dc; wxFont f1(config->Styles.font_size, wxMODERN, wxNORMAL, wxNORMAL,false,config->Styles.font_name); dc.SetFont(f1);
	wxSize sz = dc.GetTextExtent(entries[max_i].line);
	char_h = sz.GetHeight(); 
	int char_w=sz.GetWidth()/max_len;
	wxRect mwr = main_window->GetScreenRect(); wxPoint sp=parent->GetScreenPosition();
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
	// redimensionar acorde a lo que se conto, y pintar
	int my_w=max_len*char_w, my_min_x=sp.x+pos_x, win_min_x=mwr.GetLeft();
	int my_max_x=my_min_x+my_w, win_max_x=win_min_x+mwr.GetWidth();
	if (my_max_x>win_max_x) { my_min_x=win_max_x-my_w; if (my_min_x<0) my_min_x=0; }
	SetSize(my_w,cant_lines*char_h);
	Move(my_min_x,sp.y+pos_y+char_h+5);
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


