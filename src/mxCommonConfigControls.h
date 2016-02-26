#ifndef MXCOMMONCONFIGCONTROLS_H
#define MXCOMMONCONFIGCONTROLS_H
#include <wx/string.h>
#include <wx/arrstr.h>
#include "ids.h"
#include "Cpp11.h"
#include <wx/sizer.h>

class wxCheckBox;
class wxSizer;
class wxTextCtrl;
class wxBoxSizer;
class wxWindow;
class wxComboBox;
class wxStaticText;
class wxButton;
class widgetDisabler;

/**
* @brief Funciones de utilería para colocar controles en los cuadros de configuracion
**/
class mxCCC {
public:
	static wxStaticText *GetLastLabel(); ///< ultima etiqueta que se uso en alguno de los AddAlgo
	static wxButton *GetLastButton(); ///< ultimo boton colocado por AddDirCtrl
	static wxSizer *GetLastSizer(); ///< ultimo boton colocado por Add{algo}
	static wxCheckBox *AddCheckBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool value=false, wxWindowID id = wxID_ANY,bool margin=false);
	static wxTextCtrl *AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value="", int id=wxID_ANY);
	static wxTextCtrl *AddLongTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value="");
	static wxTextCtrl *AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value="", bool margin=false, wxWindowID id=wxID_ANY);
	static wxTextCtrl *AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int n, wxString tail, bool margin=false);
	static wxTextCtrl *AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int value=0,bool margin=false, int id=wxID_ANY);
	static wxTextCtrl *AddDirCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, wxWindowID id, wxString button_text = "...",bool margin=false);
	static wxComboBox *AddComboBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxArrayString &values, int def, wxWindowID = wxID_ANY,bool margin=false, bool editable=false);
	static wxStaticText* AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool center=true);
	static wxStaticText* AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, bool margin=false);
	static wxStaticText* AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, const char *value, bool margin=false)
		{ return AddStaticText(sizer,panel,text,wxString(value),margin); } // to avoid choosing the first overload instead of the second when using a literal
		
		
		
	class MainSizer {
		
		wxWindow *m_parent;
		wxBoxSizer *m_sizer;
		bool m_set_and_fit;
		
		template<typename TSizer, typename TControl>
		class BaseControl {
			widgetDisabler *m_disabler_1, *m_disabler_2;
		protected:
			TSizer *m_sizer;
			int m_id;
			void RegisterInDisablers(wxControl *ctrl1, wxControl *ctrl2=nullptr, wxControl *ctrl3=nullptr);
		public:
			BaseControl(TSizer *sizer) 
				: m_disabler_1(nullptr), m_disabler_2(nullptr), m_sizer(sizer), m_id(wxID_ANY) {}
			TControl &RegisterIn(widgetDisabler &disabler) { 
				if (!m_disabler_1) m_disabler_1=&disabler; 
				else if (!m_disabler_2) m_disabler_2=&disabler; 
				return *(static_cast<TControl*>(this)); 
			}
			TControl &Id(int id) { m_id = id; return *(static_cast<TControl*>(this));  }
		};
		
		template<class TSizer>
		class BaseCombo: public BaseControl<TSizer,BaseCombo<TSizer> > {
		protected:
			wxString m_label;
			int m_selection;
			wxArrayString m_items;
			bool m_editable;
		public:
			BaseCombo(TSizer *sizer, wxString label) 
				: BaseControl<TSizer,BaseCombo<TSizer> >(sizer), m_label(label), m_selection(-1), m_editable(false) {}
			BaseCombo &Add(wxString item) { m_items.Add(item); return *this; }
			BaseCombo &Select(int index) { m_selection = index; return *this; }
			BaseCombo &Editable(bool editable=true) { m_editable = editable; return *this; }
			BaseCombo &Select(wxString item) { m_selection = m_items.Index(item); if (m_selection==wxNOT_FOUND) m_selection=-1; return *this; }
			virtual TSizer &EndCombo(wxComboBox *&combo_box) = 0;
		};
		
		template<class TSizer>
		class BaseText : public BaseControl<TSizer,BaseText<TSizer> > {
		protected:
			wxString m_label, m_value, m_button_text;
			int m_button_id;
		public:
			BaseText(MainSizer *sizer, wxString label) 
				: BaseControl<TSizer,BaseText<TSizer> >(sizer), m_label(label), m_button_id(mxID_NULL) {}
			BaseText &Button(int id, wxString label="...") { m_button_id = id; m_button_text = label; return *this; }
			BaseText &Value(wxString value) { m_value = value; return *this; }
			virtual TSizer &EndText(wxTextCtrl *&text_ctrl) = 0;
		};
		
		template<class TSizer>
		class BaseCheck : public BaseControl<TSizer,BaseCheck<TSizer> > {
		protected:
			wxString m_label; 
			bool m_value;
		public:
			BaseCheck(TSizer *sizer, wxString label) 
				: BaseControl<TSizer,BaseCheck<TSizer> >(sizer), m_label(label), m_value(false) {}
			BaseCheck &Value(bool value) { m_value = value; return *this; }
			virtual TSizer &EndCheck(wxCheckBox *&check_box) = 0;
		};
		
		class MainText : public BaseText<MainSizer> {
		public:
			MainText(MainSizer *sizer, wxString label) : BaseText<MainSizer>(sizer,label) {}
			MainSizer &EndText(wxTextCtrl *&text_ctrl) override;
		};
		
		class MainCombo: public BaseCombo<MainSizer> {
		public:
			MainCombo(MainSizer *sizer, wxString label) : BaseCombo<MainSizer>(sizer,label) {}
			MainSizer &EndCombo(wxComboBox *&combo_box) override;
		};
		
		class InnerSizer {
			MainSizer *m_outher_sizer;
			wxBoxSizer *m_inner_sizer;
			
			class InnerLabel : public BaseControl<InnerSizer,InnerLabel> {
				wxString m_label;
			public:
				InnerLabel(InnerSizer *sizer, wxString label) : BaseControl<InnerSizer,InnerLabel>(sizer), m_label(label) {}
				InnerSizer &EndLabel();
			};
				
			class InnerCombo: public BaseCombo<InnerSizer> {
			public:
				InnerCombo(InnerSizer *sizer, wxString label) : BaseCombo<InnerSizer>(sizer,label) {}
				InnerSizer &EndCombo(wxComboBox *&combo_box);
			};
			
			class InnerCheck: public BaseCheck<InnerSizer> {
			public:
				InnerCheck(InnerSizer *sizer, wxString label) : BaseCheck<InnerSizer>(sizer,label) {}
				InnerSizer &EndCheck(wxCheckBox *&check_box) override;
			};
			
		public:
			InnerSizer(MainSizer *sizer) : m_outher_sizer(sizer),m_inner_sizer(new wxBoxSizer(wxHORIZONTAL)) {}
			InnerLabel BeginLabel(wxString label) { return InnerLabel(this,label); }
			InnerCombo BeginCombo(wxString label) { return InnerCombo(this,label); }
			InnerCheck BeginCheck(wxString label) { return InnerCheck(this,label); }
			InnerSizer &Space(int width) { m_inner_sizer->AddSpacer(width); return *this; }
			MainSizer &EndInnerSizer();
		};
		
	public:
		
		MainSizer(wxWindow *parent) : m_parent(parent), m_sizer(new wxBoxSizer(wxVERTICAL)), m_set_and_fit(true) {}
		MainSizer(wxWindow *parent, wxBoxSizer *sizer) : m_parent(parent), m_sizer(sizer), m_set_and_fit(false) {}
		MainText BeginText(wxString label) { return MainText(this,label); }
		MainCombo BeginCombo(wxString label) { return MainCombo(this,label); }
		InnerSizer BeginInnerSizer() { return InnerSizer(this); }
		void Done();
		
	};
		
	static MainSizer CreateMainSizer(wxWindow *parent) { return MainSizer(parent); }
};

#endif

