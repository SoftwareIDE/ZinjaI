#ifdef __linux__

#ifndef LNXSTUFF_H
#define LNXSTUFF_H


// Attempt to identify a window by name or attribute.
// by Adam Pierce <adam@doctort.org>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <iostream>
#include <list>
#include <cstdlib>

using namespace std;

/**
* @brief Attempt to identify a window by its pid
*
* Class by Adam Pierce <adam@doctort.org>, seen on a stackoverflow answer.
**/
class WindowsMatchingPid
	{
	public:
		WindowsMatchingPid(Display *display, Window wRoot, unsigned long pid)
		: _display(display)
		, _pid(pid)
		{
			// Get the PID property atom.
			_atomPID = XInternAtom(display, "_NET_WM_PID", True);
			if(_atomPID == None)
			{
				cout << "No such atom" << endl;
				return;
			}
			
			search(wRoot);
		}
		
		const list<Window> &result() const { return _result; }
		
	private:
		Display       *_display;
		unsigned long  _pid;
		Atom           _atomPID;
		list<Window>   _result;
		
		void search(Window w)
		{
			// Get the PID for the current Window.
			Atom           type;
			int            format;
			unsigned long  nItems;
			unsigned long  bytesAfter;
			unsigned char *propPID = 0;
			if(Success == XGetWindowProperty(_display, w, _atomPID, 0, 1, False, XA_CARDINAL,
				&type, &format, &nItems, &bytesAfter, &propPID))
			{
				if(propPID != 0)
				{
					// If the PID matches, add this window to the result set.
					if(_pid == *((unsigned long *)propPID))
						_result.push_back(w);
					
					XFree(propPID);
				}
			}
			
			// Recurse into child windows.
			Window    wRoot;
			Window    wParent;
			Window   *wChild;
			unsigned  nChildren;
			if(0 != XQueryTree(_display, w, &wRoot, &wParent, &wChild, &nChildren))
			{
				for(unsigned i = 0; i < nChildren; i++)
					search(wChild[i]);
			}
		}
	};

inline bool setFocus(int pid) {
	
	// Start with the root window.
	Display *display = XOpenDisplay(0);
	
	if (!display) return false; // it seems to fail sometimes with no apparent reason
	
	WindowsMatchingPid match(display, XDefaultRootWindow(display), pid);
	
	// Print the result.
	const list<Window> &result = match.result();
	for(list<Window>::const_iterator it = result.begin(); it != result.end(); it++) {
		
		int w=*it;
		
		Atom atom = XInternAtom (display, "_NET_ACTIVE_WINDOW", False);
		XEvent xev;
		xev.xclient.type = ClientMessage;
		xev.xclient.serial = 0;
		xev.xclient.send_event = True;
		xev.xclient.display = display;
		xev.xclient.window = w;
		xev.xclient.message_type = atom;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 2;
		xev.xclient.data.l[1] = 0;
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 0;
		xev.xclient.data.l[4] = 0;
		
		XWindowAttributes attr;
		XGetWindowAttributes(display, w, &attr);
		//attr.override_redirect = (gboolean)1;
		//XChangeWindowAttributes(d, w, 0, attr);
		
		XSendEvent(display,
			attr.root, False,
			SubstructureRedirectMask | SubstructureNotifyMask,
			&xev);
		
	}
	XFlush(display); 
	XSync(display, False);
	return !match.result().empty();
}



#endif

#endif
