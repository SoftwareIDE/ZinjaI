// -O2 -s -lpthread -lX11
// /root/.zinjai/graph.png Hola
#define t_color unsigned char
#include<iostream>
#if defined(_WIN32) || defined(__WIN32__)
#define cimg_use_png
#endif
#include "CImg.h"

using namespace std;
using namespace cimg_library;

#define min2(a,b) ((a)<(b)?(a):(b))
#define max2(a,b) ((a)>(b)?(a):(b))

int main(int argc, char *argv[]){
	
	if (argc<2) {
		cerr<<"Use: viewer <imagen> [<titulo>]";
		return 1;
	}
	
	CImg<unsigned char> img(argv[1]);	
	CImg<unsigned char> white(min2(img.width(),CImgDisplay::screen_width()),min2(img.height(),CImgDisplay::screen_height()),1,3,255);
	CImg<unsigned char> buffer(white);
	CImgDisplay ventana(white,argc>2?argv[2]:"Visor de graficos",0); 
	
	int win_w=white.width(), win_h=white.height(), owin_h,owin_w;
	int img_w=img.width(), img_h=img.height();
	
	
	bool redraw=true, moving=false;
	int x=-img_w/2+win_w/2,y=-img_h/2+win_h/2,lmx,lmy,lws=ventana.wheel(), olx,oly;
	double scale=1;
	
	if (argc>2 && string(argv[2])=="-f") {
		ventana.toggle_fullscreen();
		owin_h=win_h; owin_w=win_w;
		win_w=ventana.screen_width();
		win_h=ventana.screen_height();
		ventana.resize(win_w,win_h,false);
		white.resize(win_w,win_h);				
		buffer.resize(win_w,win_h);
		x=-img_w/2+win_w/2;
		y=-img_h/2+win_h/2;
	}
	
	while (!ventana.is_closed()){
		
		int key = ventana.key();
		int ws = ventana.wheel();
		int button = ventana.button();
		int mx = ventana.mouse_x();
		int my = ventana.mouse_y();
		
		if (key==cimg::keyARROWRIGHT) { x-=int(10*scale); redraw=true; }
		if (key==cimg::keyARROWLEFT) { x+=int(10*scale); redraw=true; }
		if (key==cimg::keyARROWDOWN){ y-=int(10*scale); redraw=true; }
		if (key==cimg::keyARROWUP){ y+=int(10*scale); redraw=true; }
		if (key==cimg::keyZ || key==cimg::keyW){ scale = min2(win_w*1.0/img_w,win_h*1.0/img_h); redraw=true; }
		if (key==cimg::keyX || key==cimg::keyO || key==cimg::key1) { scale = 1; redraw=true; }
		if (key==cimg::key1) { scale = 1; redraw=true; }
		if (key==cimg::key2 || key==cimg::keyD) { scale = 2; redraw=true; }
		if (key==cimg::key3) { scale = 3; redraw=true; }
		if (key==cimg::key4) { scale = 4; redraw=true; }
		if (key==cimg::key5) { scale = 5; redraw=true; }
		if (key==cimg::key0) { scale = 1; redraw=true; }
		if (key==cimg::key9 || key==cimg::keyH) { scale = .5; redraw=true; }
		if (key==cimg::key8) { scale = .25; redraw=true; }
		if (key==cimg::key7) { scale = .125; redraw=true; }
		if (key==cimg::key6) { scale = .0625; redraw=true; }
		if (key=='q' || key=='Q' || key==cimg::keyESC){ exit(0); }
		
		//		if (key==cimg::keyQ || key==cimg::keyESC) { return 0; }
		//		if (key==cimg::keyPADADD) { ws++; key=cimg::keyO; }
		//		if (key==cimg::keyPADSUB) { ws--; key=cimg::keyO; }
		//		if (button==2 && lb==0) { ws++; button=0; zoomOnCursor=true; }
		if (button==0) { 
			moving=false; 
		}
		
		if (ws!=lws || key==cimg::keyPADADD || key==cimg::keyPADSUB || key=='+' || key=='-') {
			double oscale=scale;
			if (lws<ws || key==cimg::keyPADADD || key=='+') {
				scale*=1.25;
			} else {
				scale*=0.8;
			}
			redraw=true;
			lws=ws;
			
			x = int(-((-x+mx)*scale/oscale-mx));
			y = int(-((-y+my)*scale/oscale-my));
			
		}

		
		if (mx>=0 && my>=0 && moving) {
			x=olx-(lmx-mx);
			y=oly-(lmy-my);
			redraw=true;
		}
		
		if (button==1 && !moving) { 
			moving=true; 
			oly=y; olx=x;
			lmx=mx;
			lmy=my; 
		}
		
		if (key==cimg::keyF) {
			key=0;
			ventana.toggle_fullscreen();
			if (ventana.is_fullscreen()) {
				owin_h=win_h; owin_w=win_w;
				win_w=ventana.screen_width();
				win_h=ventana.screen_height();
			} else {
				win_w=owin_w;
				win_h=owin_h;
			}
			ventana.resize(win_w,win_h,false);
			white.resize(win_w,win_h);				
			buffer.resize(win_w,win_h);
			redraw=true;
		} else
		
		if (ventana.is_resized()) { 
			ventana.resize(false);
			win_w=ventana.width();
			win_h=ventana.height();
			white.resize(win_w,win_h);
			buffer.resize(win_w,win_h);
			redraw=true;
		}
		key=0;
		
		if (redraw) {
//			scale=4;
			if (img_w*scale<win_w) x=int((win_w-img_w*scale)/2);
			else if (x>0) x=0;
			else if (x+img_w*scale<win_w) x=int(win_w-img_w*scale);
			if (img_h*scale<win_h) y=int((win_h-img_h*scale)/2);
			else if (y>0) y=0;
			else if (y+img_h*scale<win_h) y=int(win_h-img_h*scale);
			int cx,dx,cy,dy;
			double cw,ch;
			if (x<0) { cx=int(-x/scale); dx=0; } else { cx=0; dx=x; }
			if (y<0) { cy=int(-y/scale); dy=0; } else { cy=0; dy=y; }
			cw=win_w/scale; ch=win_h/scale;
			if (cx+cw>img_w) cw=img_w-cx;
			if (cy+ch>img_h) ch=img_h-cy;
			buffer.draw_image(0,0,0,0,white);
			double dws = cw*scale, dhs = ch*scale;
			int iws = int(dws), ihs = int(dhs);
			if (cw>0 && ch>0 && cx+cw<=img_w && cy+ch<=img_h)
				buffer.draw_image(dx,dy,0,0,img.get_crop(cx,cy,int(cx+cw)-1,int(cy+ch)-1).resize(dws==iws?iws:iws+1,dhs==ihs?ihs:ihs+1));
			ventana.render(buffer).paint();
			redraw=false;
		}
	
		ventana.wait();
		
	}
	
	return 0;
}


