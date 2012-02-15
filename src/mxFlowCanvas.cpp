#include <stack>
	
#include "mxFlowCanvas.h"

#include "mxMainWindow.h"
#include "mxSource.h"
#include "mxUtils.h"
#include "mxMessageDialog.h"

//#define FC_DRAW_LINE(x1,y1,x2,y2,pen) draw.lines.insert(draw.lines.begin(),fc_line(x1,y1,(x2)==(x1)?(x1):(x2)>(x1)?(x2)+1:(x2)-1,(y2)==(y1)?(y1):(y2)>(y1)?(y2)+1:(y2)-1,pen))
#define FC_DRAW_LINE(x1,y1,x2,y2,pen) draw.lines.insert(draw.lines.begin(),fc_line(x1,y1,x2,y2,pen))
#define FC_DRAW_TEXT(x,y,text,col) draw.texts.insert(draw.texts.begin(),fc_text(x,y,text,col))
#define FC_DRAW_CIRCLE(x,y,w,h,col) draw.arcs.insert(draw.arcs.begin(),fc_arc(x,y,w,h,col))

#define FC_COLOUR_FLECHAS pen_red
#define FC_COLOUR_BOOLEANS colour_red
#define FC_COLOUR_MARCOS pen_blue
#define FC_COLOUR_INSTRUCTION colour_black
#define FC_COLOUR_CONDITION colour_black
#define FC_COLOUR_RETURN colour_black
#define FC_COLOUR_JUMP colour_red
#define FC_MARGIN_IN 4
#define FC_MARGIN_BETWEEN 12
#define FC_MARGIN_ARROW 5

BEGIN_EVENT_TABLE(mxFlowCanvas, wxScrolledWindow)
    EVT_PAINT  (mxFlowCanvas::OnPaint)
	EVT_MOUSEWHEEL(mxFlowCanvas::OnMouseWheel)
	EVT_LEFT_DOWN(mxFlowCanvas::OnMouseDown)
	EVT_LEAVE_WINDOW(mxFlowCanvas::OnMouseUp) 
	EVT_LEFT_UP(mxFlowCanvas::OnMouseUp) 
	EVT_MOTION(mxFlowCanvas::OnMouseMotion)
END_EVENT_TABLE()

mxFlowCanvas::mxFlowCanvas(wxWindow *parent, mxSource *src) : wxScrolledWindow (parent,wxID_ANY,wxDefaultPosition,wxSize(200,200)) {
	scale=1;
	reference_index=0;
	source=src;
	colour_black = new wxColour(0,0,0);
	colour_blue = new wxColour(0,0,255);
	colour_red = new wxColour(255,0,0);
	colour_green = new wxColour(0,255,0);
	pen_black = new wxPen(*colour_black);
	pen_blue = new wxPen(*colour_blue);
	pen_red = new wxPen(*colour_red);
	pen_green = new wxPen(*colour_green);
	wxClientDC ddc(this);
	PrepareDC(ddc);
	wxDC *dc = &ddc;
	font = new wxFont(10,wxFONTFAMILY_TELETYPE,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false);
	SetBackgroundColour(*wxWHITE);
	dc->SetFont(*font);
	dc->SetBackgroundMode(wxTRANSPARENT);
    dc->SetTextBackground(*wxWHITE);
    dc->SetTextForeground(*wxBLACK);
	int r1=0,r2=0;
	if (!Analize(source->GetSelectionStart(), source->GetSelectionEnd(),draw,dc,r1,r2))
		mxMessageDialog(main_window,_T("El bloque de codigo no es correcto"),_T("Error"),mxMD_ERROR|mxMD_OK).ShowModal();
	// dibujar inicio y fin
	wxString ini,fin;
	ini<<_T("Lin ")<<source->LineFromPosition(source->GetSelectionStart())+1<<_T(" - Col ")<<source->GetSelectionStart()-source->PositionFromLine(source->LineFromPosition(source->GetSelectionStart()));
	fin<<_T("Lin ")<<source->LineFromPosition(source->GetSelectionEnd())+1<<_T(" - Col ")<<source->GetSelectionEnd()-source->PositionFromLine(source->LineFromPosition(source->GetSelectionEnd()));
	int ih,iw,fh,fw;
	GetTextSize(ini,dc,iw,ih);
	GetTextSize(fin,dc,fw,fh);
	draw.y=ih+FC_MARGIN_IN*2+1;
	// incicio
	FC_DRAW_LINE(-iw,0,iw,0,FC_COLOUR_FLECHAS);
	FC_DRAW_LINE(-iw,-ih-FC_MARGIN_IN*2,iw,-ih-FC_MARGIN_IN*2,FC_COLOUR_FLECHAS);
	FC_DRAW_TEXT(-iw,-ih-FC_MARGIN_IN,ini,FC_COLOUR_CONDITION);
	FC_DRAW_LINE(-iw,0,-iw-FC_MARGIN_BETWEEN,-FC_MARGIN_IN-ih/2,FC_COLOUR_FLECHAS);
	FC_DRAW_LINE(+iw,0,+iw+FC_MARGIN_BETWEEN,-FC_MARGIN_IN-ih/2,FC_COLOUR_FLECHAS);
	FC_DRAW_LINE(+iw,-FC_MARGIN_IN*2-ih,+iw+FC_MARGIN_BETWEEN,-FC_MARGIN_IN-ih/2,FC_COLOUR_FLECHAS);
	FC_DRAW_LINE(-iw,-FC_MARGIN_IN*2-ih,-iw-FC_MARGIN_BETWEEN,-FC_MARGIN_IN-ih/2,FC_COLOUR_FLECHAS);
	// flecha
	FC_DRAW_LINE(0,draw.alto,0,draw.alto+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
	FC_DRAW_LINE(FC_MARGIN_ARROW,draw.alto+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,0,draw.alto+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
	FC_DRAW_LINE(-FC_MARGIN_ARROW,draw.alto+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,0,draw.alto+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
	// fin
	FC_DRAW_LINE(-fw,draw.alto+FC_MARGIN_BETWEEN,fw,draw.alto+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
	FC_DRAW_LINE(-fw,fh+FC_MARGIN_IN*2+draw.alto+FC_MARGIN_BETWEEN,+fw,fh+FC_MARGIN_IN*2+draw.alto+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
	FC_DRAW_TEXT(-fw,draw.alto+FC_MARGIN_BETWEEN+FC_MARGIN_IN,fin,FC_COLOUR_CONDITION);
	FC_DRAW_LINE(-fw,draw.alto+FC_MARGIN_BETWEEN,-fw-FC_MARGIN_BETWEEN,draw.alto+FC_MARGIN_BETWEEN+FC_MARGIN_IN+fh/2,FC_COLOUR_FLECHAS);
	FC_DRAW_LINE(+fw,draw.alto+FC_MARGIN_BETWEEN,+fw+FC_MARGIN_BETWEEN,draw.alto+FC_MARGIN_BETWEEN+FC_MARGIN_IN+fh/2,FC_COLOUR_FLECHAS);
	FC_DRAW_LINE(+fw,draw.alto+FC_MARGIN_BETWEEN+FC_MARGIN_IN*2+fh,+fw+FC_MARGIN_BETWEEN,draw.alto+FC_MARGIN_BETWEEN+FC_MARGIN_IN+fh/2,FC_COLOUR_FLECHAS);
	FC_DRAW_LINE(-fw,draw.alto+FC_MARGIN_BETWEEN+FC_MARGIN_IN*2+fh,-fw-FC_MARGIN_BETWEEN,draw.alto+FC_MARGIN_BETWEEN+FC_MARGIN_IN+fh/2,FC_COLOUR_FLECHAS);
	
	draw.alto+=2*FC_MARGIN_BETWEEN+FC_MARGIN_IN*4+ih+fh;
	
	if (draw.izquierda<(iw>fw?iw:fw)+2*FC_MARGIN_BETWEEN)
		draw.izquierda=iw>fw?iw:fw+2*FC_MARGIN_BETWEEN;
	if (draw.derecha<(iw>fw?iw:fw)+2*FC_MARGIN_BETWEEN)
		draw.derecha=iw>fw?iw:fw+2*FC_MARGIN_BETWEEN;

	SetScrollbars(1,1,draw.izquierda+draw.derecha+2*FC_MARGIN_BETWEEN,draw.alto+FC_MARGIN_BETWEEN*2,draw.izquierda+FC_MARGIN_BETWEEN,FC_MARGIN_BETWEEN,true);
	SetCursor(wxCursor(wxCURSOR_HAND));
}

int mxFlowCanvas::FindBlockEnd(int p, int pe) {
	int s;
	char c;
	FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
	if (source->GetCharAt(p)=='{') {
		return source->BraceMatch(p);
	}
	if (source->GetStyleAt(p)==wxSTC_C_WORD && source->GetCharAt(p)=='i' && source->GetCharAt(p+1)=='f' && source->GetStyleAt(p+2)!=wxSTC_C_WORD) {
		p+=2;
		FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
		if (c!='(' || (p=source->BraceMatch(p))==wxSTC_INVALID_POSITION)
			return wxSTC_INVALID_POSITION;
		p++;	
		if ( (p=FindBlockEnd(p,pe))==wxSTC_INVALID_POSITION)
			return wxSTC_INVALID_POSITION;
		p++;
		FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
		if (source->GetStyleAt(p)==wxSTC_C_WORD && source->GetCharAt(p)=='e' && source->GetCharAt(p+1)=='l' && source->GetCharAt(p+2)=='s' && source->GetCharAt(p+3)=='e' && source->GetStyleAt(p+4)!=wxSTC_C_WORD)
			return FindBlockEnd(p+4,pe);
		else
			return p-1;
	} else {
		while (!FC_IS(';') || FC_SHOULD_IGNORE) {
			if ( (c=='{' || c=='(') && source->GetStyleAt(p)==wxSTC_C_OPERATOR) {
				p=source->BraceMatch(p);
				if (p==wxSTC_INVALID_POSITION)
					return wxSTC_INVALID_POSITION;
			}
			p++;
		}
	}
	return p;
}

bool mxFlowCanvas::Analize(int ps, int pe, draw_data &draw, wxDC *dc, int &break_index, int &continue_index) {
	int p=ps, s;
	char c;
	int p1,p2, ax,ay, tw,th;
	wxString text;
	int x=0, y=0;
	int mb_index, mc_index;
	stack<wxChar> pila;
	while (true) {
		FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
		while (c=='{' || c=='}') {
			p++;
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			if (p>pe) {
				draw.alto = y;
				return true;
			}
		}
		if (p>pe) {
			draw.alto = y;
			return true;
		}
		if (source->GetStyleAt(p)==wxSTC_C_WORD && source->GetCharAt(p)=='i' && source->GetCharAt(p+1)=='f' && source->GetStyleAt(p+2)!=wxSTC_C_WORD) {
			// dibujar flecha
			FC_DRAW_LINE(x,y,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x+FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			y+=FC_MARGIN_BETWEEN;
			
			// extraer condicion
			p+=2;
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			if (c!='(' || (p2=source->BraceMatch(p))==wxSTC_INVALID_POSITION) 
				return false;
			p1=p+1;
			GetTextSize ( text = GetText(p1,p2), dc, tw, th );
			p=p2+1;
			
			// dibujar condicion
			int cw = tw+FC_MARGIN_BETWEEN, ch=th+2*FC_MARGIN_BETWEEN;
			FC_DRAW_LINE(x,y,x-cw,y+ch/2,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x,y,x+cw,y+ch/2,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x,y+ch,x-cw,y+ch/2,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x,y+ch,x+cw,y+ch/2,FC_COLOUR_MARCOS);
			FC_DRAW_TEXT(x-tw,y+FC_MARGIN_BETWEEN,text,FC_COLOUR_CONDITION);
			
			FC_DRAW_TEXT(x-tw-FC_MARGIN_BETWEEN*2,y+ch/2-FC_MARGIN_BETWEEN-FC_MARGIN_IN,_T("f"),FC_COLOUR_BOOLEANS);
			FC_DRAW_TEXT(x+tw+FC_MARGIN_BETWEEN+FC_MARGIN_IN,y+ch/2-FC_MARGIN_BETWEEN-FC_MARGIN_IN,_T("v"),FC_COLOUR_BOOLEANS);
			
			// bloque verdadero
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			p2 = FindBlockEnd(p1=p,pe);
			if (p2==wxSTC_INVALID_POSITION)
				return false;
			draw_data v_draw(x,y+ch);
			if (!Analize(p1,p2,v_draw,dc,break_index,continue_index))
				return false;
			v_draw.x = v_draw.izquierda>cw ? x+FC_MARGIN_BETWEEN+v_draw.izquierda : x+FC_MARGIN_BETWEEN+cw;
			draw.draws.insert(draw.draws.begin(),v_draw);
			p=p2+1;
			if (v_draw.x-x+v_draw.derecha>draw.derecha)
				draw.derecha = v_draw.x-x+v_draw.derecha;
			
			// bloque false
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			if (source->GetStyleAt(p)==wxSTC_C_WORD && source->GetCharAt(p)=='e' && source->GetCharAt(p+1)=='l' && source->GetCharAt(p+2)=='s' && source->GetCharAt(p+3)=='e' && source->GetStyleAt(p+4)!=wxSTC_C_WORD) {			
				p+=4;
				// buscar que sigue
				FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
				p2 = FindBlockEnd(p1=p,pe);
				if (p2==wxSTC_INVALID_POSITION)
					return false;
				draw_data f_draw(x,y+ch);
				if (!Analize(p1,p2,f_draw,dc,break_index,continue_index))
					return false;
				f_draw.x = f_draw.derecha>cw ? x-FC_MARGIN_BETWEEN-f_draw.derecha : x-FC_MARGIN_BETWEEN-cw;
				draw.draws.insert(draw.draws.begin(),f_draw);
				p=p2+1;
				if (x-f_draw.x+f_draw.izquierda>draw.izquierda)
					draw.izquierda = x-f_draw.x+f_draw.izquierda;
				int hh = v_draw.alto>f_draw.alto?v_draw.alto:f_draw.alto;
				
				// flechas verdadero
				FC_DRAW_LINE(x+cw,y+ch/2,v_draw.x,y+ch/2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(v_draw.x,v_draw.y,v_draw.x,y+ch/2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(v_draw.x,v_draw.y+v_draw.alto,v_draw.x,y+ch+hh+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);

				// flechas falso
				FC_DRAW_LINE(x-cw,y+ch/2,f_draw.x,y+ch/2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(f_draw.x,f_draw.y,f_draw.x,y+ch/2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(f_draw.x,f_draw.y+f_draw.alto,f_draw.x,y+ch+hh+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
				
				y+=hh+ch+FC_MARGIN_BETWEEN;
				FC_DRAW_LINE(v_draw.x,y,f_draw.x,y,FC_COLOUR_FLECHAS);
			} else {
				// lineas true
				FC_DRAW_LINE(x+cw,y+ch/2,v_draw.x,y+ch/2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(v_draw.x,v_draw.y,v_draw.x,y+ch/2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(v_draw.x,v_draw.y+v_draw.alto,v_draw.x,v_draw.y+v_draw.alto+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
				// lineas false
				FC_DRAW_LINE(x-cw,y+ch/2,x-cw-FC_MARGIN_BETWEEN,y+ch/2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x-cw-FC_MARGIN_BETWEEN,y+ch+v_draw.alto+FC_MARGIN_BETWEEN,x-cw-FC_MARGIN_BETWEEN,y+ch/2,FC_COLOUR_FLECHAS);
				// linea que cierra abajo
				y+=v_draw.alto+ch+FC_MARGIN_BETWEEN;
				FC_DRAW_LINE(x-cw-FC_MARGIN_BETWEEN,y,x+v_draw.x,y,FC_COLOUR_FLECHAS);
				if (draw.izquierda<cw+FC_MARGIN_BETWEEN)
					draw.izquierda=cw+FC_MARGIN_BETWEEN;
			}
			
		} else if (source->GetStyleAt(p)==wxSTC_C_WORD && source->GetCharAt(p)=='f' && source->GetCharAt(p+1)=='o' && source->GetCharAt(p+2)=='r' && source->GetStyleAt(p+3)!=wxSTC_C_WORD) {
			p+=3;
			FC_DRAW_LINE(x,y,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			y+=FC_MARGIN_BETWEEN;
			FC_DRAW_LINE(x,y,x-FC_MARGIN_ARROW,y-FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x,y,x-FC_MARGIN_ARROW,y+FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);

			// buscar las condiciones
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			if (c!='(' || (p2=source->BraceMatch(p))==wxSTC_INVALID_POSITION) 
				return false;
			int p11,p12,p21,p22,p31,p32;
			// condicion inicial
			p++;
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			p11=p;
			FC_FRONT(!FC_IS(';') || FC_SHOULD_IGNORE);
			p12=p;
			if (p12>=p2) 
				return false;
			//condicion de continuidad
			p++;
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			p21=p;
			FC_FRONT(!FC_IS(';') || FC_SHOULD_IGNORE);
			p22=p;
			if (p22>=p2) 
				return false;
			// incremento
			p++;
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			p31=p;
			p32=p2;
			//cortar los textos seleccionados
			wxString t1,t2,t3;
			int th1,th2,th3,tw1,tw2,tw3;
			GetTextSize ( t1 = GetText(p11,p12), dc, tw1, th1 );
			GetTextSize ( t2 = GetText(p21,p22), dc, tw2, th2 );
			GetTextSize ( t3 = GetText(p31,p32), dc, tw3, th3 );
			p=p2+1;
			
			// buscar que sigue
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			p2 = FindBlockEnd(p1=p,pe);
			if (p2==wxSTC_INVALID_POSITION)
				return false;
			draw_data i_draw(x,y);
			mb_index=0; mc_index=0;
			if (!Analize(p1,p2,i_draw,dc,mb_index,mc_index))
				return false;

			draw.draws.insert(draw.draws.begin(),i_draw);
			p=p2+1;
			
			if (mc_index) { // dibujar referencia del break si es necesario
				y+=i_draw.alto;
				FC_DRAW_LINE(x,y,x,y+FC_MARGIN_IN,FC_COLOUR_FLECHAS);
				y+=FC_MARGIN_IN;
				wxString text;
				text<<mc_index;
				GetTextSize(text,dc,tw,th);
				ax=tw+FC_MARGIN_IN;
				FC_DRAW_LINE(x,y,x,y+ax*2,FC_COLOUR_FLECHAS);;
				FC_DRAW_TEXT(x-FC_MARGIN_BETWEEN-ax-tw,y+ax-th/2,text,FC_COLOUR_JUMP);
				FC_DRAW_CIRCLE(x-FC_MARGIN_BETWEEN-ax*2,y,ax*2,ax*2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_ARROW,y+ax-FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_ARROW,y+ax+FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_BETWEEN,y+ax,FC_COLOUR_FLECHAS);
				y-=FC_MARGIN_IN+i_draw.alto;
				i_draw.alto+=ax*2+FC_MARGIN_IN;
			}
			
			// dibujar flechas y condiciones
			int gw,gh;
			gh = FC_MARGIN_BETWEEN*2+FC_MARGIN_IN*4+th1+th2+th3;
			gw = FC_MARGIN_BETWEEN*2+2*(tw2>tw1*4/3&&tw2>tw3*4/2?tw2:(tw1>tw3?tw1*4/3:tw3*4/3));
			int hh = gh+FC_MARGIN_BETWEEN*2>i_draw.alto?gh+FC_MARGIN_BETWEEN*2:i_draw.alto;
			FC_DRAW_LINE(x,y+i_draw.alto,x,y+hh+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			y+=hh+FC_MARGIN_BETWEEN;
			
			FC_DRAW_CIRCLE(x-i_draw.izquierda-FC_MARGIN_BETWEEN-gw,y-hh/2-gh/2,gw,gh,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x, y,                      x-i_draw.izquierda-gw/2-FC_MARGIN_BETWEEN, y,                      FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x, y-hh-FC_MARGIN_BETWEEN, x-i_draw.izquierda-gw/2-FC_MARGIN_BETWEEN, y-hh-FC_MARGIN_BETWEEN, FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-i_draw.izquierda-gw/2-FC_MARGIN_BETWEEN, y                      ,x-i_draw.izquierda-gw/2-FC_MARGIN_BETWEEN, y-hh/2+gh/2, FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-i_draw.izquierda-gw/2-FC_MARGIN_BETWEEN, y-hh-FC_MARGIN_BETWEEN ,x-i_draw.izquierda-gw/2-FC_MARGIN_BETWEEN, y-hh/2-gh/2, FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-i_draw.izquierda-gw/2-FC_MARGIN_BETWEEN-FC_MARGIN_ARROW, y-hh/2+gh/2+FC_MARGIN_ARROW, x-i_draw.izquierda-gw/2-FC_MARGIN_BETWEEN, y-hh/2+gh/2, FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-i_draw.izquierda-gw/2-FC_MARGIN_BETWEEN+FC_MARGIN_ARROW, y-hh/2+gh/2+FC_MARGIN_ARROW, x-i_draw.izquierda-gw/2-FC_MARGIN_BETWEEN, y-hh/2+gh/2, FC_COLOUR_FLECHAS);
			
			FC_DRAW_TEXT(x-FC_MARGIN_BETWEEN-i_draw.izquierda-gw/2-tw1,y-hh/2-gh/2+FC_MARGIN_BETWEEN,t1,FC_COLOUR_CONDITION);

			/* TODO: mejorar estas dos lineas */
			FC_DRAW_LINE(
				(int)(x-FC_MARGIN_BETWEEN-i_draw.izquierda-gw/2+gw/2*sin((gh-th1-FC_MARGIN_BETWEEN-FC_MARGIN_IN)*M_PI/gh)),
				y-hh/2-gh/2+FC_MARGIN_BETWEEN+th1+FC_MARGIN_IN,
				(int)(x-FC_MARGIN_BETWEEN-i_draw.izquierda-gw/2-gw/2*sin((gh-th1-FC_MARGIN_BETWEEN-FC_MARGIN_IN)*M_PI/gh)) -2,
				y-hh/2-gh/2+FC_MARGIN_BETWEEN+th1+FC_MARGIN_IN,
				FC_COLOUR_MARCOS);
			FC_DRAW_LINE(
				(int)(x-FC_MARGIN_BETWEEN-i_draw.izquierda-gw/2+gw/2*sin((gh-th3-FC_MARGIN_BETWEEN-FC_MARGIN_IN)*M_PI/gh)),
				y-hh/2-gh/2+FC_MARGIN_BETWEEN+th1+th2+3*FC_MARGIN_IN,
				(int)(x-FC_MARGIN_BETWEEN-i_draw.izquierda-gw/2-gw/2*sin((gh-th3-FC_MARGIN_BETWEEN-FC_MARGIN_IN)*M_PI/gh)) -2,
				y-hh/2-gh/2+FC_MARGIN_BETWEEN+th1+th2+3*FC_MARGIN_IN,
				FC_COLOUR_MARCOS);
				
			FC_DRAW_TEXT(x-FC_MARGIN_BETWEEN-i_draw.izquierda-gw/2-tw2,y-hh/2-gh/2+FC_MARGIN_BETWEEN+th1+FC_MARGIN_IN*2,t2,FC_COLOUR_CONDITION);
			FC_DRAW_TEXT(x-FC_MARGIN_BETWEEN-i_draw.izquierda-gw/2-tw3,y-hh/2-gh/2+FC_MARGIN_BETWEEN+th1+th2+FC_MARGIN_IN*4,t3,FC_COLOUR_CONDITION);
			
			if (draw.izquierda<i_draw.izquierda+gw+FC_MARGIN_BETWEEN)
				draw.izquierda=i_draw.izquierda+gw+FC_MARGIN_BETWEEN;
			if (draw.derecha<i_draw.derecha)
				draw.derecha=i_draw.derecha;
			
			if (mb_index) { // dibujar referencia del break si es necesario
				FC_DRAW_LINE(x,y,x,y+FC_MARGIN_IN,FC_COLOUR_FLECHAS);
				y+=FC_MARGIN_IN;
				wxString text;
				text<<mb_index;
				GetTextSize(text,dc,tw,th);
				ax=tw+FC_MARGIN_IN;
				FC_DRAW_LINE(x,y,x,y+ax*2,FC_COLOUR_FLECHAS);;
				FC_DRAW_TEXT(x-FC_MARGIN_BETWEEN-ax-tw,y+ax-th/2,text,FC_COLOUR_JUMP);
				FC_DRAW_CIRCLE(x-FC_MARGIN_BETWEEN-ax*2,y,ax*2,ax*2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_ARROW,y+ax-FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_ARROW,y+ax+FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_BETWEEN,y+ax,FC_COLOUR_FLECHAS);
				y+=ax*2;
			}
			
		} else if (source->GetStyleAt(p)==wxSTC_C_WORD && source->GetCharAt(p)=='b' && source->GetCharAt(p+1)=='r' && source->GetCharAt(p+2)=='e' && source->GetCharAt(p+3)=='a' && source->GetCharAt(p+4)=='k' && source->GetStyleAt(p+5)!=wxSTC_C_WORD) {
			p+=5;
			FC_FRONT(!FC_IS(';') || FC_SHOULD_IGNORE);
			p++;
			FC_DRAW_LINE(x,y,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x+FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			y+=FC_MARGIN_BETWEEN;
			if (break_index==0)
				break_index=++reference_index;
			wxString text;
			text<<reference_index;
			GetTextSize(text,dc,tw,th);
			ax=tw+FC_MARGIN_IN;
			FC_DRAW_TEXT(x-tw,y+ax-th/2,text,FC_COLOUR_JUMP);
			FC_DRAW_CIRCLE(x-ax,y,ax*2,ax*2,FC_COLOUR_FLECHAS);
			y+=ax*2+FC_MARGIN_BETWEEN;
		} else if (source->GetStyleAt(p)==wxSTC_C_WORD && source->GetCharAt(p)=='c' && source->GetCharAt(p+1)=='o' && source->GetCharAt(p+2)=='n' && source->GetCharAt(p+3)=='t' && source->GetCharAt(p+4)=='i' && source->GetCharAt(p+5)=='n' && source->GetCharAt(p+6)=='u' && source->GetCharAt(p+7)=='e' && source->GetStyleAt(p+8)!=wxSTC_C_WORD) {
			p+=8;
			FC_FRONT(!FC_IS(';') || FC_SHOULD_IGNORE);
			p++;
			FC_DRAW_LINE(x,y,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x+FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			y+=FC_MARGIN_BETWEEN;
			if (continue_index==0)
				continue_index=++reference_index;
			wxString text;
			text<<reference_index;
			GetTextSize(text,dc,tw,th);
			ax=tw+FC_MARGIN_IN;
			FC_DRAW_TEXT(x-tw,y+ax-th/2,text,FC_COLOUR_JUMP);
			FC_DRAW_CIRCLE(x-ax,y,ax*2,ax*2,FC_COLOUR_FLECHAS);
			y+=ax*2+FC_MARGIN_BETWEEN;
		} else if (source->GetStyleAt(p)==wxSTC_C_WORD && source->GetCharAt(p)=='r' && source->GetCharAt(p+1)=='e' && source->GetCharAt(p+2)=='t' && source->GetCharAt(p+3)=='u' && source->GetCharAt(p+4)=='r' && source->GetCharAt(p+5)=='n' && source->GetStyleAt(p+6)!=wxSTC_C_WORD) {
			p1 = p;
			FC_FRONT(!FC_IS(';') || FC_SHOULD_IGNORE);
			p2 = p;
			FC_BACK(p1,FC_IS_EMPTY || FC_SHOULD_IGNORE);
			text = GetText(p1,p);
			p=p2;
			if (text.Len()==0) {
				draw.alto = y;
				return true;
			}
			
			// dibujar una instruccion comun
			FC_DRAW_LINE(x,y,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x+FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			y+=FC_MARGIN_BETWEEN;
			
			GetTextSize(text,dc,tw,th);
			FC_DRAW_TEXT(x-tw,y+FC_MARGIN_IN,text,FC_COLOUR_RETURN);
			
			// dibujar flecha de salida
			FC_DRAW_LINE(x+tw-FC_MARGIN_ARROW-FC_MARGIN_IN,-1+y+FC_MARGIN_ARROW,x+tw+FC_MARGIN_ARROW-FC_MARGIN_IN,-1+y-FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x+tw+FC_MARGIN_ARROW-FC_MARGIN_IN,-1+y-FC_MARGIN_ARROW,x+tw-FC_MARGIN_IN,-1+y-FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x+tw+FC_MARGIN_ARROW-FC_MARGIN_IN,-1+y-FC_MARGIN_ARROW,x+tw+FC_MARGIN_ARROW-FC_MARGIN_IN,-1+y,FC_COLOUR_FLECHAS);
			
			ax=tw+FC_MARGIN_IN;
			ay=th+FC_MARGIN_IN+FC_MARGIN_IN;
			FC_DRAW_LINE(x-ax,y,x+ax,y,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x-ax,y+ay,x+ax,y+ay,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x-ax,y,x-ax,y+ay,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x+ax,y,x+ax,y+ay,FC_COLOUR_MARCOS);
			
			y+=ay+FC_MARGIN_BETWEEN;
			if (ax>draw.izquierda) 
				draw.izquierda=ax;
			if (ax>draw.derecha) 
				draw.derecha=ax;
			// ---
			p++;
		
		} else if (source->GetStyleAt(p)==wxSTC_C_WORD && source->GetCharAt(p)=='d' && source->GetCharAt(p+1)=='o' && source->GetStyleAt(p+2)!=wxSTC_C_WORD) {
			p+=2;
			FC_DRAW_LINE(x,y,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			y+=FC_MARGIN_BETWEEN;
			
			// buscar que sigue
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			p2 = FindBlockEnd(p1=p,pe);
			if (p2==wxSTC_INVALID_POSITION)
				return false;
			draw_data i_draw(x,y);
			mb_index=0; mc_index=0;
			if (!Analize(p1,p2,i_draw,dc,mb_index,mc_index))
				return false;
			y+=i_draw.alto;
			draw.draws.insert(draw.draws.begin(),i_draw);
			p=p2+1;
			
			// buscar la condicion
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			if (source->GetCharAt(p)!='w' || source->GetCharAt(p+1)!='h' || source->GetCharAt(p+2)!='i' || source->GetCharAt(p+3)!='l' || source->GetCharAt(p+4)!='e' )
				return false;
			p+=5;
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			p1=p+1;
			if (source->GetCharAt(p)!='(' || (p=source->BraceMatch(p))==wxSTC_INVALID_POSITION)
				return false;
			p2=p;
			p++;
			
			int cont_space;
			if (mc_index) { // dibujar referencia del continue si es necesario
				FC_DRAW_LINE(x,y,x,y+FC_MARGIN_IN,FC_COLOUR_FLECHAS);
				y+=FC_MARGIN_IN;
				wxString text;
				text<<mc_index;
				GetTextSize(text,dc,tw,th);
				ax=tw+FC_MARGIN_IN;
				FC_DRAW_LINE(x,y,x,y+ax*2,FC_COLOUR_FLECHAS);;
				FC_DRAW_TEXT(x+FC_MARGIN_BETWEEN+ax-tw,y+ax-th/2,text,FC_COLOUR_JUMP);
				FC_DRAW_CIRCLE(x+FC_MARGIN_BETWEEN,y,ax*2,ax*2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x+FC_MARGIN_ARROW,y+ax-FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x+FC_MARGIN_ARROW,y+ax+FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x+FC_MARGIN_BETWEEN,y+ax,FC_COLOUR_FLECHAS);
				y+=ax*2;
				FC_DRAW_LINE(x,y,x,y+FC_MARGIN_IN,FC_COLOUR_FLECHAS);
				y+=FC_MARGIN_IN;
				cont_space=(FC_MARGIN_IN+ax)*2;
			} else {
				FC_DRAW_LINE(x,y,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
				y+=FC_MARGIN_BETWEEN;
				cont_space=FC_MARGIN_BETWEEN;
			}

			FC_DRAW_LINE(x-FC_MARGIN_ARROW,y-FC_MARGIN_ARROW,x,y,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x+FC_MARGIN_ARROW,y-FC_MARGIN_ARROW,x,y,FC_COLOUR_FLECHAS);

			// dibujar la condicion
			
			GetTextSize(text=GetText(p1,p2),dc,tw,th);
			int ch = th/2+FC_MARGIN_BETWEEN, cw = tw+FC_MARGIN_BETWEEN;
			FC_DRAW_LINE(x,y,x+cw,y+ch,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x,y,x-cw,y+ch,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x,y+ch*2,x+cw,y+ch,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x,y+ch*2,x-cw,y+ch,FC_COLOUR_MARCOS);
			FC_DRAW_TEXT(x-tw,y+ch-th/2,text,FC_COLOUR_CONDITION);
			y+=ch*2;
			
			// dibujar flechas que faltan
			int fw=(cw>i_draw.izquierda?cw:i_draw.izquierda)+FC_MARGIN_BETWEEN;
			// falso
			FC_DRAW_LINE(x,y,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_TEXT(x+FC_MARGIN_ARROW,y,_T("f"),FC_COLOUR_BOOLEANS);
			// verdadero
			FC_DRAW_LINE(x-cw,y-ch,x-fw,y-ch,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-fw,y-ch,x-fw,y-ch*2-cont_space-i_draw.alto,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x,y-ch*2-cont_space-i_draw.alto,x-fw,y-ch*2-cont_space-i_draw.alto,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-FC_MARGIN_ARROW,y-FC_MARGIN_ARROW-ch*2-cont_space-i_draw.alto,x,y-ch*2-cont_space-i_draw.alto,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-FC_MARGIN_ARROW,y+FC_MARGIN_ARROW-ch*2-cont_space-i_draw.alto,x,y-ch*2-cont_space-i_draw.alto,FC_COLOUR_FLECHAS);
			FC_DRAW_TEXT(x-cw-FC_MARGIN_ARROW*2,y-ch,_T("v"),FC_COLOUR_BOOLEANS);
			y+=FC_MARGIN_BETWEEN;

			if (mb_index) { // dibujar referencia del break si es necesario
				FC_DRAW_LINE(x,y,x,y+FC_MARGIN_IN,FC_COLOUR_FLECHAS);
				y+=FC_MARGIN_IN;
				wxString text;
				text<<mb_index;
				GetTextSize(text,dc,tw,th);
				ax=tw+FC_MARGIN_IN;
				FC_DRAW_LINE(x,y,x,y+ax*2,FC_COLOUR_FLECHAS);;
				FC_DRAW_TEXT(x-FC_MARGIN_BETWEEN-ax-tw,y+ax-th/2,text,FC_COLOUR_JUMP);
				FC_DRAW_CIRCLE(x-FC_MARGIN_BETWEEN-ax*2,y,ax*2,ax*2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_ARROW,y+ax-FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_ARROW,y+ax+FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_BETWEEN,y+ax,FC_COLOUR_FLECHAS);
				y+=ax*2;
			}
			
			if (fw>draw.izquierda) 
				draw.izquierda=fw;
			if (draw.derecha<i_draw.derecha)
				draw.derecha=i_draw.derecha;
			p++;
		} else if (source->GetStyleAt(p)==wxSTC_C_WORD && source->GetCharAt(p)=='w' && source->GetCharAt(p+1)=='h' && source->GetCharAt(p+2)=='i' && source->GetCharAt(p+3)=='l' && source->GetCharAt(p+4)=='e' && source->GetStyleAt(p+5)!=wxSTC_C_WORD) {

			p+=5;
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			p1=p+1;
			if (source->GetCharAt(p)!='(' || (p=source->BraceMatch(p))==wxSTC_INVALID_POSITION)
				return false;
			p2=p;
			p++;
			
			// extraer condicion
			wxString condicion=GetText(p1,p2);

			// buscar que sigue
			int cont_space;
			FC_FRONT(FC_IS_EMPTY || FC_SHOULD_IGNORE);
			p2 = FindBlockEnd(p1=p,pe);
			if (p2==wxSTC_INVALID_POSITION)
				return false;
			draw_data i_draw(x,y);
			mb_index=0; mc_index=0;
			if (!Analize(p1,p2,i_draw,dc,mb_index,mc_index))
				return false;
			if (mc_index) {
				// dibujar flecha
				FC_DRAW_LINE(x,y,x,y+FC_MARGIN_IN,FC_COLOUR_FLECHAS);
				y+=FC_MARGIN_IN;
				wxString text;
				text<<mc_index;
				GetTextSize(text,dc,tw,th);
				ax=tw+FC_MARGIN_IN;
				cont_space=(ax+FC_MARGIN_IN)*2;
				i_draw.y+=cont_space;
				FC_DRAW_LINE(x,y,x,y+cont_space,FC_COLOUR_FLECHAS);
				y+=ax+FC_MARGIN_BETWEEN;
				FC_DRAW_LINE(x,y,x+FC_MARGIN_BETWEEN,y,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y,x+FC_MARGIN_ARROW,y-FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y,x+FC_MARGIN_ARROW,y+FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_TEXT(x+FC_MARGIN_BETWEEN+ax-tw,y-th/2,text,FC_COLOUR_JUMP);
				FC_DRAW_CIRCLE(x+FC_MARGIN_BETWEEN,y-ax,ax*2,ax*2,FC_COLOUR_FLECHAS);
				cont_space+=FC_MARGIN_BETWEEN;
			} else {
				FC_DRAW_LINE(x,y,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
				y+=FC_MARGIN_BETWEEN;
				cont_space=FC_MARGIN_BETWEEN*2;
			}
			p=p2+1;			
			
			// dibujar flecha
			FC_DRAW_LINE(x,y,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x+FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			y+=FC_MARGIN_BETWEEN;
			
			// dibujar condicion
			GetTextSize(condicion,dc,tw,th);
			int ch = th/2+FC_MARGIN_BETWEEN, cw = tw+FC_MARGIN_BETWEEN;
			FC_DRAW_LINE(x,y,x+cw,y+ch,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x,y,x-cw,y+ch,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x,y+ch*2,x+cw,y+ch,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x,y+ch*2,x-cw,y+ch,FC_COLOUR_MARCOS);
			FC_DRAW_TEXT(x-tw,y+ch-th/2,condicion,FC_COLOUR_CONDITION);
			FC_DRAW_LINE(x,y+ch*2,x,y+ch*2+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			// v de verdadero
			y+=ch*2+FC_MARGIN_BETWEEN;
			FC_DRAW_TEXT(x+FC_MARGIN_ARROW,y-FC_MARGIN_BETWEEN,_T("v"),FC_COLOUR_BOOLEANS);
			
			i_draw.y+=ch*2+FC_MARGIN_BETWEEN*2;
			draw.draws.insert(draw.draws.begin(),i_draw);

			
			// dibujar flechas que faltan
			int fwi=(cw>i_draw.izquierda?cw:i_draw.izquierda)+FC_MARGIN_BETWEEN;
			int fwd=(cw>i_draw.derecha?cw:i_draw.derecha)+FC_MARGIN_BETWEEN;
			
			FC_DRAW_LINE(x,y+i_draw.alto,x,y+i_draw.alto+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x,y+i_draw.alto+FC_MARGIN_BETWEEN,x-fwi,y+i_draw.alto+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-fwi,y-ch*2-cont_space,x-fwi,y+i_draw.alto+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			
			FC_DRAW_LINE(x-fwi,y-ch*2-cont_space,x,y-ch*2-cont_space,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-FC_MARGIN_ARROW,y-ch*2-cont_space+FC_MARGIN_ARROW,x,y-ch*2-cont_space,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-FC_MARGIN_ARROW,y-ch*2-cont_space-FC_MARGIN_ARROW,x,y-ch*2-cont_space,FC_COLOUR_FLECHAS);
			
			FC_DRAW_LINE(x+cw,y-ch-FC_MARGIN_BETWEEN,x+fwd,y-ch-FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x+fwd,y-ch-FC_MARGIN_BETWEEN,x+fwd,y+i_draw.alto+FC_MARGIN_BETWEEN*2,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x,y+i_draw.alto+FC_MARGIN_BETWEEN*2,x+fwd,y+i_draw.alto+FC_MARGIN_BETWEEN*2,FC_COLOUR_FLECHAS);
			
			// f de falso
			FC_DRAW_TEXT(x+cw,y-ch-FC_MARGIN_BETWEEN,_T("f"),FC_COLOUR_BOOLEANS);
			
			if (fwi>draw.izquierda) 
				draw.izquierda=fwi;
			if (fwd>draw.derecha)
				draw.derecha=fwd;
			
			y+=i_draw.alto+FC_MARGIN_BETWEEN*2;
			
			if (mb_index) { // dibujar referencia del break si es necesario
				FC_DRAW_LINE(x,y,x,y+FC_MARGIN_IN,FC_COLOUR_FLECHAS);
				y+=FC_MARGIN_IN;
				wxString text;
				text<<mb_index;
				GetTextSize(text,dc,tw,th);
				ax=tw+FC_MARGIN_IN;
				FC_DRAW_LINE(x,y,x,y+ax*2,FC_COLOUR_FLECHAS);;
				FC_DRAW_TEXT(x-FC_MARGIN_BETWEEN-ax-tw,y+ax-th/2,text,FC_COLOUR_JUMP);
				FC_DRAW_CIRCLE(x-FC_MARGIN_BETWEEN-ax*2,y,ax*2,ax*2,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_ARROW,y+ax-FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_ARROW,y+ax+FC_MARGIN_ARROW,FC_COLOUR_FLECHAS);
				FC_DRAW_LINE(x,y+ax,x-FC_MARGIN_BETWEEN,y+ax,FC_COLOUR_FLECHAS);
				y+=ax*2;
			}
			
		} else {
			p1 = p;
			FC_FRONT(!FC_IS(';') || FC_SHOULD_IGNORE);
			p2 = p;
			FC_BACK(p1,FC_IS_EMPTY || FC_SHOULD_IGNORE);
			text = GetText(p1,p);
			p=p2;
			if (text.Len()==0) {
				draw.alto = y;
				return true;
			}
		
			// dibujar una instruccion comun
			FC_DRAW_LINE(x,y,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x+FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			FC_DRAW_LINE(x-FC_MARGIN_ARROW,y+FC_MARGIN_BETWEEN-FC_MARGIN_ARROW,x,y+FC_MARGIN_BETWEEN,FC_COLOUR_FLECHAS);
			y+=FC_MARGIN_BETWEEN;
			
			GetTextSize(text,dc,tw,th);
			FC_DRAW_TEXT(x-tw,y+FC_MARGIN_IN,text,FC_COLOUR_INSTRUCTION);
			
			ax=tw+FC_MARGIN_IN;
			ay=th+FC_MARGIN_IN+FC_MARGIN_IN;
			FC_DRAW_LINE(x-ax,y,x+ax,y,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x-ax,y+ay,x+ax,y+ay,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x-ax,y,x-ax,y+ay,FC_COLOUR_MARCOS);
			FC_DRAW_LINE(x+ax,y,x+ax,y+ay,FC_COLOUR_MARCOS);
			
			y+=ay;
			if (ax>draw.derecha) 
				draw.derecha=ax;
			if (ax>draw.izquierda) 
				draw.izquierda=ax;
			// ---
			p++;
		}
	}
	return true;
}

mxFlowCanvas::~mxFlowCanvas() {
	delete colour_black;
	delete colour_red;
	delete colour_green;
	delete colour_blue;
	delete pen_black;
	delete pen_red;
	delete pen_green;
	delete pen_blue;
	delete font;
}

void mxFlowCanvas::GetTextSize(wxString text, wxDC *dc, int &m_ancho, int &alto) {
	wxSize s = dc->GetTextExtent(text);
	m_ancho = s.GetWidth()/2;
	alto = s.GetHeight();
}

void mxFlowCanvas::OnPaint (wxPaintEvent &event) {
	int w=GetRect().GetWidth();
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.SetUserScale(scale,scale);
	dc.Clear();
	dc.SetFont(*font);
	dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetTextBackground(*wxWHITE);
	if (scale*(draw.izquierda+draw.derecha+2*FC_MARGIN_BETWEEN)<w)
		Draw(dc,draw,int((w/2+(draw.izquierda-(draw.izquierda+draw.derecha)/2))/scale),FC_MARGIN_BETWEEN  /*draw.alto+FC_MARGIN_BETWEEN*/ );
	else
		Draw(dc,draw,draw.izquierda+FC_MARGIN_BETWEEN,FC_MARGIN_BETWEEN /*draw.alto+FC_MARGIN_BETWEEN*/ );
}

void mxFlowCanvas::Draw(wxPaintDC &dc, draw_data &draw, int x, int y) {
	x+=draw.x;
	y+=draw.y;
	list<fc_arc>::iterator it_a=draw.arcs.begin(), ed_a=draw.arcs.end();
	while (it_a!=ed_a) {
		dc.SetPen(*(it_a->pen));
		dc.DrawEllipse(x+it_a->x,y+it_a->y,it_a->w,it_a->h);
		it_a++;
	}
	list<fc_line>::iterator it_l=draw.lines.begin(), ed_l=draw.lines.end();
	while (it_l!=ed_l) {
		dc.SetPen(*(it_l->pen));
		dc.DrawLine(x+it_l->x1,y+it_l->y1,x+it_l->x2,y+it_l->y2);
		it_l++;
	}
	list<fc_text>::iterator it_t=draw.texts.begin(), ed_t=draw.texts.end();
	while (it_t!=ed_t) {
		dc.SetTextForeground(*(it_t->colour));
		dc.DrawText(it_t->text,x+it_t->x,y+it_t->y);
		it_t++;
	}
	list<draw_data>::iterator it_d=draw.draws.begin(), ed_d=draw.draws.end();
	while (it_d!=ed_d) {
		Draw(dc,*it_d, x, y);
		it_d++;
	}
}

wxString mxFlowCanvas::GetText(int p1, int p2) {
	wxString ret;
	int p=p1, s;
	char c;
	while (p<p2) {
		while (p<p2 && !FC_IS_EMPTY && !FC_IS_COMMENT) {
			ret+=c; p++;
		}
		ret+=_T(" ");
		while (p<p2 && (FC_IS_EMPTY || FC_IS_COMMENT) )
			p++;
	}
	ret.RemoveLast();
	return ret;
}

bool mxFlowCanvas::IsWord(wxString &text, wxString word) {
	if (text.Len()<=word.Len()) 
		return false;
	unsigned int i;
	for (i=0; i<word.Len();i++) {
		if (text[i]!=word[i])
			return false;
	}
	char c=text[i];
	return !((c>='a' && c<='z') || (c>='A' && c<='z') || (c>='0' && c<='9') || c=='.' || c=='_');
}

void mxFlowCanvas::ChangeScale(double rel_size, int mx, int my) {
	scale*=rel_size;
	int x,y,w,h;
	GetViewStart(&x,&y);
	GetClientSize(&w,&h);
	SetScrollbars(1,1,int((draw.izquierda+draw.derecha+FC_MARGIN_BETWEEN*2)*scale),int((draw.alto+FC_MARGIN_BETWEEN*2)*scale),int(x*rel_size),int(y*rel_size),false);
//	SetVirtualSize(scale*(draw.izquierda+draw.derecha+2*FC_MARGIN_BETWEEN),scale*(draw.alto+FC_MARGIN_BETWEEN*2));
//	SetScrollRate(1,1);
//	Scroll(x+w/2-w/rel_size,y+h/2-y/rel_size);
	Refresh();
}

void mxFlowCanvas::OnMouseUp(wxMouseEvent &evt) {
	m_motion=false;
}

void mxFlowCanvas::OnMouseDown(wxMouseEvent &evt) {
	m_motion=true;
	m_y=evt.m_y;
	m_x=evt.m_x;
	GetViewStart(&m_sx,&m_sy);
}

void mxFlowCanvas::OnMouseMotion(wxMouseEvent &evt) {
	if (m_motion)
		Scroll(m_sx+m_x-evt.m_x,m_sy+m_y-evt.m_y);
}

void mxFlowCanvas::OnMouseWheel(wxMouseEvent &evt) {
	if (evt.m_wheelRotation>0)
		ChangeScale(5.0/4,evt.m_x,evt.m_y);
	else
		ChangeScale(4.0/5,evt.m_x,evt.m_y);
}
