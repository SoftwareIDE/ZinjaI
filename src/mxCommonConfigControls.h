#ifndef MXCOMMONCONFIGCONTROLS_H
#define MXCOMMONCONFIGCONTROLS_H
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#ifdef __WIN32__
#	include <wx/settings.h>
#endif
#include "Cpp11.h"
#include "ids.h"

class wxCheckBox;
class wxSizer;
class wxTextCtrl;
class wxBoxSizer;
class wxWindow;
class wxComboBox;
class wxStaticText;
class wxButton;
class wxNotebook;
class widgetDisabler;
class widgetBinder;

/**
* @brief Funciones de utilería para colocar controles en los cuadros de configuracion
**/
class mxDialog : public wxDialog {
protected:
	void OnCloseDestroy(wxCloseEvent &evt) { Destroy(); }
	void OnCloseHide(wxCloseEvent &evt) { EndModal(0); Hide(); }
	wxString GetCaption() { return GetTitle(); }
public:
	static wxCheckBox *AddCheckBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool value=false, wxWindowID id = wxID_ANY,bool margin=false);
	static wxTextCtrl *AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value="", int id=wxID_ANY);
	static wxTextCtrl *AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value="", bool margin=false, wxWindowID id=wxID_ANY);
	static wxTextCtrl *AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int n, wxString tail, bool margin=false);
	static wxTextCtrl *AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int value=0,bool margin=false, int id=wxID_ANY);
	static wxTextCtrl *AddDirCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, wxWindowID id, wxString button_text = "...",bool margin=false);
	static wxComboBox *AddComboBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxArrayString &values, int def, wxWindowID = wxID_ANY,bool margin=false, bool editable=false);
	static wxStaticText* AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool center=true);
	static wxStaticText* AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, bool margin=false);
	static wxStaticText* AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, const char *value, bool margin=false)
		{ return AddStaticText(sizer,panel,text,wxString(value),margin); } // to avoid choosing the first overload instead of the second when using a literal
		
	mxDialog(wxWindow *parent, wxString caption, bool destroy_on_close = true) 
		: wxDialog(parent,wxID_ANY,caption,wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
	{
#ifdef __WIN32__
		SetBackgroundColour(wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ));
#endif
		if (destroy_on_close) this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( mxDialog::OnCloseDestroy ) );
		else                  this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( mxDialog::OnCloseHide ) );
	}
	

private:
	class BaseNotebookAux {
	protected:
		wxNotebook *m_notebook;
		void AddPageHelper(wxString name, wxWindow *page);
		int m_selection, m_id;
	public:
		BaseNotebookAux(wxNotebook *notebook) 
			: m_notebook(notebook), m_selection(0), m_id(wxID_ANY) {}
		wxNotebook *EndNotebook();
	};
	
	template<class TNotebook>
	class BaseNotebook : public BaseNotebookAux {
	protected:
	public:
		BaseNotebook(wxNotebook *notebook) : BaseNotebookAux(notebook) {}
			template<typename class_t,typename method_t>
			TNotebook &AddPageIf(bool condition, class_t _this_, method_t _ref_class__method_, wxString page_label) {
				if (condition) AddPageHelper(page_label, (_this_->*_ref_class__method_)(m_notebook)); 
				return *(static_cast<TNotebook*>(this));
			}
			template<typename class_t,typename method_t>
			TNotebook &AddPage(class_t _this_, method_t _ref_class__method_, wxString page_label) {
				AddPageHelper(page_label, (_this_->*_ref_class__method_)(m_notebook)); 
				return *(static_cast<TNotebook*>(this));
			}
			TNotebook &Id(int id) { m_id= id; return *(static_cast<TNotebook*>(this)); }
			TNotebook &Select(int page_id) { m_selection = page_id; return *(static_cast<TNotebook*>(this)); }
	};
	
private:
	
	class BaseSizer {
	protected:
		wxWindow *m_parent;
		wxBoxSizer *m_sizer;
	public:
		BaseSizer(wxWindow *parent, wxBoxSizer *sizer) : m_parent(parent), m_sizer(sizer) {}
		wxBoxSizer *GetSizer() { return m_sizer; }
		void SetAndFit();
		void Set();
	};
	
	class MainSizer : public BaseSizer {
	
	protected:	
		wxBoxSizer *m_real_sizer;
		
		template<typename TSizer, typename TControl>
		class BaseControl {
			widgetDisabler *m_disabler_1, *m_disabler_2;
		protected:
			wxString m_label;
			TSizer *m_sizer;
			int m_id;
			widgetBinder *m_binder;
			void *m_bind_value;
			void RegisterInDisablers(wxControl *ctrl1, wxControl *ctrl2=nullptr, wxControl *ctrl3=nullptr);
			template<typename T>
			void SetBind(widgetBinder &binder, T &value) { m_binder = &binder; m_bind_value = &value; }
			template<typename wxCtrl_t, typename value_t> void DoBind(wxCtrl_t *ctrl);
		public:
			BaseControl(TSizer *sizer, wxString label) : m_disabler_1(nullptr), m_disabler_2(nullptr),
				  m_label(label), m_sizer(sizer), m_id(wxID_ANY), m_binder(nullptr),m_bind_value(nullptr) {}
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
			wxString m_value;
			int m_selection; ///< -1 default, -2 use value (may not be in the items' list)
			wxArrayString m_items;
			bool m_editable, m_bind_by_pos;
			void FixValueAndSelection();
		public:
			BaseCombo(TSizer *sizer, wxString label) 
				: BaseControl<TSizer,BaseCombo<TSizer> >(sizer,label), m_selection(-1), m_editable(false), m_bind_by_pos(true) {}
			BaseCombo &Add(wxArrayString &items) { for(unsigned int i=0;i<items.GetCount();++i) m_items.Add(items[i]); return *this; }
			BaseCombo &Add(wxString item) { m_items.Add(item); return *this; }
			BaseCombo &AddIf(bool condition, wxString item) { if (condition) m_items.Add(item); return *this; }
			BaseCombo &Select(int index) { m_selection = index; return *this; }
			BaseCombo &Select(wxString item, int default_idx=-1) { m_selection = m_items.Index(item); if (m_selection==wxNOT_FOUND) m_selection=default_idx; return *this; }
			BaseCombo &Value(wxString value) { m_selection = -2; m_value = value; return *this; }
			BaseCombo &Bind(widgetBinder &binder, int &index) { m_bind_by_pos=true; BaseControl<TSizer,BaseCombo<TSizer> >::SetBind(binder,index); return Select(index); }
			BaseCombo &Bind(widgetBinder &binder, wxString &item, int default_idx=-1) { m_bind_by_pos=false; BaseControl<TSizer,BaseCombo<TSizer> >::SetBind(binder,item); return Select(item,default_idx); }
			BaseCombo &Editable(bool editable=true) { m_editable = editable; return *this; }
			virtual TSizer &EndCombo(wxComboBox *&combo_box) = 0;
			TSizer &EndCombo() { wxComboBox *dummy; return EndCombo(dummy); }
		};
		
		template<class TSizer>
		class BaseText : public BaseControl<TSizer,BaseText<TSizer> > {
		protected:
			wxString m_value, m_button_text;
			int m_button_id;
			bool m_is_numeric, m_one_line, m_multiline, m_readonly;
		public:
			BaseText(TSizer *sizer, wxString label) 
				: BaseControl<TSizer,BaseText<TSizer> >(sizer,label), 
				  m_button_id(mxID_NULL), m_is_numeric(false), m_one_line(false), m_multiline(false), m_readonly(false) {}
			BaseText &Button(int id, wxString label="...") { m_button_id = id; m_button_text = label; return *this; }
			BaseText &Value(wxString value) { m_value = value; return *this; }
			BaseText &Value(int value) { m_is_numeric = m_one_line = true; m_value = wxString()<<value; return *this; }
			BaseText &Bind(widgetBinder &binder, wxString &value) { BaseControl<TSizer,BaseText<TSizer> >::SetBind(binder,value); return Value(value); }
			BaseText &Bind(widgetBinder &binder, int &value) { BaseControl<TSizer,BaseText<TSizer> >::SetBind(binder,value); return Value(value); }
			BaseText &Short(bool value_is_short=true) { m_one_line = value_is_short; return *this; }
			BaseText &ReadOnly(bool readonly=true) { m_readonly = readonly; return *this; }
			BaseText &MultiLine(bool multiline=true) { m_multiline = multiline; return *this; }
			virtual TSizer &EndText(wxTextCtrl *&text_ctrl) = 0;
			TSizer &EndText() { wxTextCtrl *dummy; return EndText(dummy); }
		};
		
		template<class TSizer>
		class BaseCheck : public BaseControl<TSizer,BaseCheck<TSizer> > {
		protected:
			bool m_value;
		public:
			BaseCheck(TSizer *sizer, wxString label) 
				: BaseControl<TSizer,BaseCheck<TSizer> >(sizer,label), m_value(false) {}
			BaseCheck &Value(bool value) { m_value = value; return *this; }
			BaseCheck &Bind(widgetBinder &binder, bool &value) { BaseControl<TSizer,BaseCheck<TSizer> >::SetBind(binder,value); return Value(value); }
			virtual TSizer &EndCheck(wxCheckBox *&check_box) = 0;
			TSizer &EndCheck() { wxCheckBox *dummy; return EndCheck(dummy); }
		};
		
		template<class TSizer>
		class BaseLabel : public BaseControl<TSizer,BaseLabel<TSizer> > {
		protected:
			bool m_center;
		public:
			BaseLabel(TSizer *sizer, wxString label) : BaseControl<TSizer,BaseLabel<TSizer> >(sizer,label), m_center(false) {}
			BaseLabel &Center(bool center=true) { m_center = center; return *this; }
			virtual TSizer &EndLabel() = 0;
		};
		
		template<class TSizer>
		class BaseButton : public BaseControl<TSizer,BaseButton<TSizer> > {
		protected:
			bool m_expand;
		public:
			BaseButton(TSizer *sizer, wxString label) 
				: BaseControl<TSizer,BaseButton<TSizer> >(sizer,label), m_expand(false) {}
			BaseButton &Expand(bool expand=true) { m_expand = expand; return *this; }
			virtual TSizer &EndButton() = 0;
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
		
		class MainCheck: public BaseCheck<MainSizer> {
		public:
			MainCheck(MainSizer *sizer, wxString label) : BaseCheck<MainSizer>(sizer,label) {}
			MainSizer &EndCheck(wxCheckBox *&check_box) override;
		};
		
		class MainLabel: public BaseLabel<MainSizer> {
		public:
			MainLabel(MainSizer *sizer, wxString label) : BaseLabel<MainSizer>(sizer,label) {}
			MainSizer &EndLabel() override;
		};
		
		class MainButton: public BaseButton<MainSizer> {
		public:
			MainButton(MainSizer *sizer, wxString label) : BaseButton<MainSizer>(sizer,label) {}
			MainSizer &EndButton() override;
		};
		
		class MainNotebook: public BaseNotebook<MainNotebook> {
			MainSizer *m_sizer;	
		public:
			MainNotebook(MainSizer *sizer, wxNotebook *notebook) : BaseNotebook<MainNotebook>(notebook), m_sizer(sizer) {}
			MainSizer &EndNotebook(wxNotebook *&notebook) { notebook = m_notebook; return EndNotebook(); }
			MainSizer &EndNotebook();
		};
		
		class InnerSizer {
			MainSizer *m_outher_sizer;
			wxBoxSizer *m_inner_sizer;
			
			class InnerLabel : public BaseControl<InnerSizer,InnerLabel> {
			public:
				InnerLabel(InnerSizer *sizer, wxString label) : BaseControl<InnerSizer,InnerLabel>(sizer,label) {}
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
			
			class InnerButton: public BaseButton<InnerSizer> {
			public:
				InnerButton(InnerSizer *sizer, wxString label) : BaseButton<InnerSizer>(sizer,label) {}
				InnerSizer &EndButton() override;
			};
			
			class InnerText: public BaseText<InnerSizer> {
			public:
				InnerText(InnerSizer *sizer, wxString label) : BaseText<InnerSizer>(sizer,label) {}
				InnerSizer &EndText(wxTextCtrl *&text_ctrl) override;
			};
			
		public:
			InnerSizer(MainSizer *sizer) : m_outher_sizer(sizer),m_inner_sizer(new wxBoxSizer(wxHORIZONTAL)) {}
			InnerLabel BeginLabel(wxString label) { return InnerLabel(this,label); }
			InnerCombo BeginCombo(wxString label="") { return InnerCombo(this,label); }
			InnerCheck BeginCheck(wxString label) { return InnerCheck(this,label); }
			InnerButton BeginButton(wxString label) { return InnerButton(this,label); }
			InnerText BeginText(wxString label) { return InnerText(this,label); }
			InnerSizer &Space(int width) { m_inner_sizer->AddSpacer(width); return *this; }
			MainSizer &EndLine();
		};
		
		class BottomSizer {
			MainSizer *m_sizer;
			int m_ok_id, m_cancel_id, m_help_id, m_close_id;
			int m_extra_id; wxString m_extra_label; 
			const wxBitmap *m_extra_icon;
		public:
			BottomSizer(MainSizer *sizer) 
				: m_sizer(sizer), m_ok_id(mxID_NULL), m_cancel_id(mxID_NULL), m_help_id(mxID_NULL),
				  m_close_id(mxID_NULL), m_extra_id(mxID_NULL), m_extra_icon(nullptr) {}
			BottomSizer &Close(int id = wxID_CANCEL) { m_close_id = id; return *this; }
			BottomSizer &Ok(int id = wxID_OK) { m_ok_id = id; return *this; }
			BottomSizer &Cancel(int id = wxID_CANCEL) { m_cancel_id = id; return *this; }
			BottomSizer &Help(int id = mxID_HELP_BUTTON) { m_help_id = id; return *this; }
			BottomSizer &Extra(int id, wxString label, const wxBitmap *icon) { 
				m_extra_id = id; m_extra_label = label; m_extra_icon = icon; return *this; 
			}
			MainSizer &EndBottom(wxDialog *dialog);
		};
		
	public:
		
		MainSizer(wxWindow *parent) : BaseSizer(parent,new wxBoxSizer(wxVERTICAL)) {}
		MainSizer(wxWindow *parent, wxBoxSizer *sizer) : BaseSizer(parent,sizer) {}
		MainText BeginText(wxString label) { return MainText(this,label); }
		MainCombo BeginCombo(wxString label) { return MainCombo(this,label); }
		MainCheck BeginCheck(wxString label) { return MainCheck(this,label); }
		MainButton BeginButton(wxString label) { return MainButton(this,label); }
		MainLabel BeginLabel(wxString label) { return MainLabel(this,label); }
		MainNotebook BeginNotebook();
		InnerSizer BeginLine() { return InnerSizer(this); }
		MainSizer &Spacer() { m_sizer->AddStretchSpacer(); return *this; }
		MainSizer &Add(BaseSizer &main_sizer, const wxSizerFlags &flags) { m_sizer->Add(main_sizer.GetSizer(),flags); return *this; }
		MainSizer &Add(wxWindow *control, const wxSizerFlags &flags) { m_sizer->Add(control,flags); return *this; }
		MainSizer &BeginSection(wxString label);
		MainSizer &BeginSection(int margin=15);
		MainSizer &EndSection() { m_sizer = m_real_sizer; m_real_sizer = nullptr; return *this; }
		BottomSizer BeginBottom() { return BottomSizer(this); }
	};
	
	class HorizontalSizer : public BaseSizer {
	public:
		HorizontalSizer(wxWindow *parent, wxBoxSizer *sizer) : BaseSizer(parent,sizer) {}
		HorizontalSizer &Add(MainSizer &sizer, int proportion=1);
	};
	
public:
	class CreateHorizontalSizer : public HorizontalSizer { 	
		public: CreateHorizontalSizer (wxWindow *parent); 
	};
	
	class CreateSizer : public MainSizer { 	
		public: CreateSizer (wxWindow *parent); 
	};
	
	class CreatePanelAndSizer : public MainSizer { 	
		public: CreatePanelAndSizer (wxWindow *parent);
		public: wxPanel *GetPanel() { return static_cast<wxPanel*>(m_parent); }
	};
	
	class ReuseSizer : public MainSizer {
		public: ReuseSizer (wxWindow *parent, wxBoxSizer *sizer); 
	};

private:
	class FreeNotebook: public BaseNotebook<FreeNotebook> {
	public:
		FreeNotebook(wxNotebook *notebook) : BaseNotebook<FreeNotebook>(notebook) {}
	};
public:
	FreeNotebook CreateNotebook(wxWindow *parent);
};

#endif

