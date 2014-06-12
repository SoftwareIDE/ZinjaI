#ifndef MENUSANDTOOLSCONFIG_H
#define MENUSANDTOOLSCONFIG_H
#include <vector>
#include <wx/menuitem.h>
#include <wx/textfile.h>
#include "ids.h"
#ifdef _ZINJAI_DEBUG
#include<iostream>
#endif
using namespace std;

class wxMenuBar;
class wxStaticText;
class mxMainWindow;


class MenusAndToolsConfig {
	friend class mxPreferenceWindow;
private:
	
	wxMenuBar *wx_menu_bar; ///< main window's menu bar
	
	///< flags for myMenuItem's properties
	enum mnAttribs { 
		maCHECKEABLE=1, ///< check item, default unchecked
		maCHECKED=2, ///< check item, default checked
		maPROJECT=4, ///< only enabled for project mode
		maNOPROJECT=8, ///< only enabled for simple program mode
		maDEBUG=16, ///< only enabled while debugging
		maNODEBUG=32, ///< only enabled while no debugging
		maMAPPED=64, ///< store a reference to this item, to be used outside this class
		maBEGIN_SUBMENU=128, ///< indica el comienzo de un submenu
		maEND_SUBMENU=256, ///< indica el final de un submenu
		maSEPARATOR=512 ///< representa un separatod
	};
	
	/// @brief for storing menues structure before really creating them, and for simplifing AddMenuItem interface (only one flexible parameter for that method)
	struct myMenuItem {
		int wx_id, properties;
		wxString label, description, icon, shortcut;
		myMenuItem(int _id=0, int _props=0):wx_id(_id),properties(_props){}
		myMenuItem(int _id, const wxString &_label):wx_id(_id),properties(0),label(_label) {}
		myMenuItem &Label(const wxString &_label) { label=_label; return *this; }
		myMenuItem &ShortCut(const wxString &_shortcut) { shortcut=_shortcut; return *this; }
		myMenuItem &Description(const wxString &_description) { description=_description; return *this; }
		/*myMenuItem &Key(const wxString &_key) { key=_key; return *this; }*/
		myMenuItem &Icon(const wxString &_icon) { icon=_icon; return *this; }
		myMenuItem &Checkeable(bool checked) { properties|=(checked?maCHECKED:maCHECKEABLE)|maMAPPED; return *this; }
		myMenuItem &Debug(bool enabled_on_debug_mode) { properties|=(enabled_on_debug_mode?maDEBUG:maNODEBUG); return *this; }
		myMenuItem &Project(bool enabled_on_project_mode) { properties|=(enabled_on_project_mode?maPROJECT:maNOPROJECT); return *this; }
		myMenuItem &Map() { properties|=maMAPPED; return *this; }
		myMenuItem &AddProps(int props) { properties|=props; return *this; }
	};
	
	/// struct for storing wich items should be enabled/disabled when changing project mode or debug mode
	struct AutoenabligItem { 
		wxMenuItem *item; bool value; 
		AutoenabligItem() {}
//		AutoenabligItem(wxMenu *_wx_menu, bool _value):item(_wx_menu),is_item(false),value(_value) {}
		AutoenabligItem(wxMenuItem *_wx_item, bool _value):item(_wx_item),value(_value) {}
		void Enable(bool _value) {
			item->Enable(value==_value);
		}
	};
	/// list with items that should be enabled/disabled when openning/closing a project
	vector<AutoenabligItem> items_project;
	/// list with items that should be starting/ending a debug session
	vector<AutoenabligItem> items_debug;
	
	/// struct for storing refences to items that will be needed later outside this class
	template<class T>
	struct MappedSomething { 
		int wx_id; T something;
		MappedSomething() {}
		MappedSomething(int _wx_id, T _something):wx_id(_wx_id),something(_something) {}
	};
	/// items that can be queried with GetItem
	vector<MappedSomething<wxMenuItem*> > mapped_items;
	/// items that can be queried with GetMenu
	vector<MappedSomething<wxMenu*> > mapped_menues;
	
	/// struct for storing information for a single menu and all its items
	struct myMenu {
		wxString label;
		wxMenu *wx_menu;
		vector<myMenuItem> items;
	};

public:
	/// enums with menus names to be used as indexes
	enum mnID { mnFILE, mnEDIT, mnVIEW, mnRUN, mnDEBUG, mnTOOLS, mnHELP, mnCOUNT };
private:
	/// array containing the information for each menu
	myMenu menues[mnCOUNT];
	
	/// adds a new item to an existing menu/submenu
	void AddMenuItem(int menu_id, const myMenuItem &mi) {
		menues[menu_id].items.push_back(mi);
	}
	
	/// adds a separator line to an existing menu/submenu
	void AddSeparator(int menu_id) {
		menues[menu_id].items.push_back(myMenuItem(wxID_ANY,maSEPARATOR));
	}
	
	/// adds a new submenu to an existing menu/submenu... following calls to AddMenuItem will place items in this submenu until EndSubMenu is called
	void BeginSubMenu(int menu_id, const wxString &label, const wxString &description="", const wxString &icon="", int wx_id=wxID_ANY, int properties=0) {
		menues[menu_id].items.push_back(myMenuItem(wx_id,maBEGIN_SUBMENU|(wx_id==wxID_ANY?0:maMAPPED)).Label(label).Description(description).Icon(icon).AddProps(properties));
	}
	
	/// see BeginSubMenu
	void EndSubMenu(int menu_id) {
		menues[menu_id].items.push_back(myMenuItem(wxID_ANY,maEND_SUBMENU));
	}
	
	// submenues de tratamiento especial (porque se actualizan dinamicamente durante la ejecucion)
	wxMenuItem **file_source_history;
	wxMenuItem **file_project_history;
	wxMenuItem **tools_custom_item;
	friend class mxOpenRecentDialog;
	friend class mxMainWindow;
	
	
	
	/// @brief for storing toolbar structure before really creating them, and for simplifing AddToolbarItem interface (only one flexible parameter for that method)
	struct myToolbarItem {
		int wx_id; bool visible, checkeable;
		wxString key, label, description, icon;
		myToolbarItem(){}
		myToolbarItem(const wxString &_key, int _id, const wxString &_icon, const wxString &_label):wx_id(_id),visible(false),key(_key),label(_label),icon(_icon) {}
		myToolbarItem(const wxString &_key, myMenu &menu, int _wx_id) : wx_id(_wx_id), visible(false), key(_key) {
			wxString menulabel=menu.label; int submenu_deep=0;
			for(unsigned int i=0,l=menu.items.size();i<l;i++) { 
				myMenuItem &mi = menu.items[i];
				if (mi.properties&maBEGIN_SUBMENU) {
					submenu_deep++; if (submenu_deep==1) menulabel = mi.label;
				} else if (mi.properties&maEND_SUBMENU) { 
					submenu_deep--; if (submenu_deep==0) menulabel = menu.label;
				}
				if (mi.wx_id==_wx_id) {
					icon = mi.icon;
					label = menulabel + " -> " + mi.label;
					description = mi.description;
					break;
				}
			}
#ifdef _ZINJAI_DEBUG
			if (!label.Len()) { cerr<<"EMPTY TOOLBAR ITEM!!"<<endl; }
#endif
			label.Replace("&","",true);
			description.Replace("&","",true);
		}
		myToolbarItem &Label(const wxString &_label) { label=_label; return *this; }
		myToolbarItem &Visible() { visible=true; return *this; }
		myToolbarItem &Checkeable() { checkeable=true; return *this; }
		myToolbarItem &Description(const wxString &_description) { description=_description; return *this; }
	};
	
	
	int icon_size;
	
	//! Información sobre la posición y visibilidad de una barra de herramientas
	struct toolbarPosition {
		bool top,left,right,visible;
		long row/*,pos*/;
		toolbarPosition():top(true),left(false),right(false),visible(false),row(0)/*,pos(0)*/ {}
		void operator=(wxString s) { 
			if (s.Len()==0) return;
			else if (s[0]=='T') { top=true; left=right=false; visible=true; }
			else if (s[0]=='R') { right=true; left=top=false; visible=true; }
			else if (s[0]=='L') { left=true; top=right=false; visible=true; }
			else if (s[0]=='t') { top=true; left=right=false; visible=false; }
			else if (s[0]=='r') { right=true; left=top=false; visible=false; }
			else if (s[0]=='l') { left=true; top=right=false; visible=false; }
			s=s.Mid(1); if (s.Len()==0) return; else s.ToLong(&row);
		}
		operator wxString() {
			wxString s;
			if (left) s<<"L"<<row; 
			else if (right) s<<"R"<<row; 
			else s<<"T"<<row;
			if (!visible) s[0]+=32;
			return s;
		}
	};
	
	/// struct for storing information for a single toolbar and all its items
	struct myToolbar {
		wxString label, key;
		wxToolBar *wx_toolbar;
		vector<myToolbarItem> items;
		toolbarPosition position;
		void Init(const wxString &_key, const wxString &_label, const wxString &_position) {
			key=_key; label=_label; position=_position;
		}
	};
	
public:
	/// enums with toolbars names to be used as indexes
	enum tbID { tbFILE, tbEDIT, tbVIEW, tbRUN, tbDEBUG, tbTOOLS, tbMISC, tbPROJECT, tbSTATUS, tbFIND, tbDIFF, tbCOUNT_FULL };
private:
	/// array containing the information for each menu
	myToolbar toolbars[tbCOUNT_FULL];
	
	void AddToolbarItem(int toolbar_id, const myToolbarItem &ti) {
		toolbars[toolbar_id].items.push_back(ti);
	}
	
	// elementos de las barras de tratamiento especial
	wxStaticText *toolbar_status_text;
	wxTextCtrl* toolbar_find_text;
	
public:
	
	MenusAndToolsConfig();
	
	void LoadMenuData();
//	bool ParseMenuConfigLine(const wxString &key, const wxString &value);
private:
	void CreateMenues();
	void PopulateMenu(int menu_id);
public:
	wxMenuItem *GetItem(int wx_id) {
		for(unsigned int i=0;i<mapped_items.size();i++) { 
			if (mapped_items[i].wx_id==wx_id) return mapped_items[i].something;
		}
		return NULL;
	}
	wxMenu *GetMenu(int wx_id) {
		for(unsigned int i=0;i<mapped_menues.size();i++) { 
			if (mapped_menues[i].wx_id==wx_id) return mapped_menues[i].something;
		}
		return NULL;
	}
//	void SaveMenuConfig(wxTextFile &file);
	
	
	void LoadToolbarsData();
	bool ParseToolbarConfigLine(const wxString &key, const wxString &value);
private:
	void CreateWxToolbar(int tb_id);
	void PopulateToolbar(int tb_id);
	void AdjustToolbarSize(int tb_id);
	void CreateToolbars();
public:
	/// update a wxToolBar to match new settings in its myToolbar (use only_items=false when orientation or icon size also changes)
	void UpdateToolbar(int tb_id, bool only_items);
	wxToolBar *GetToolbar(int toolbar_id) { return toolbars[toolbar_id].wx_toolbar; }
	toolbarPosition &GetToolbarPosition(int toolbar_id) { return toolbars[toolbar_id].position; }
	/// finds wich toolbar has an item with the given id
	int ToolbarFromTool(int tool_id);
	int GetToolbarIconSize() { return icon_size; }
	
	void SaveToolbarConfig(wxTextFile &file);
	
	void SetDebugMode(bool mode);
	void SetProjectMode(bool mode);
	
	void CreateMenuesAndToolbars(mxMainWindow *_main_window);
	
};

extern MenusAndToolsConfig *menu_data;

#define _menu_item(id) menu_data->GetItem(id)
#define _get_toolbar(id) menu_data->GetToolbar(MenusAndToolsConfig::id)
#define _toolbar_visible(id) menu_data->GetToolbarPosition(MenusAndToolsConfig::id).visible
#endif

