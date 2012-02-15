///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __Ventanas__
#define __Ventanas__

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class Frame0
///////////////////////////////////////////////////////////////////////////////
class Frame0 : public wxFrame 
{
	private:
	
	protected:
		wxStaticText* m_staticText1;
		wxButton* m_button1;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnBotonCerrar( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		Frame0( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~Frame0();
	
};

#endif //__Ventanas__
