#ifndef MENUSANDTOOLSCONFIG_H
#define MENUSANDTOOLSCONFIG_H
#include <wx/menuitem.h>
#include <vector>
#include "ids.h"
using namespace std;

class wxMenuBar;

class MenusAndToolsConfig {
private:
	
	wxMenuBar *wx_menu_bar; ///< main window's menu bar
	
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
	
	/// @brief for storing menues structure before really creating them, and for simplifing AddItem interface (only one flexible parameter for that method)
	struct myMenuItem {
		int wx_id, properties;
		wxString label, description, icon, shortcut;
		myMenuItem(int _id=0, int _props=0):wx_id(_id),properties(_props){}
		myMenuItem(int _id, const wxString &_label):wx_id(_id),properties(0),label(_label) {}
		myMenuItem &Label(const wxString &_label) { label=_label; return *this; }
		myMenuItem &ShortCut(const wxString &_shortcut) { shortcut=_shortcut; return *this; }
		myMenuItem &Description(const wxString &_description) { description=_description; return *this; }
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
	vector<AutoenabligItem> items_debug, items_project;
	
	/// struct for storing refences to items that will be needed later outside this class
	struct MappedItem { 
		int wx_id; wxMenuItem *wx_item;
		MappedItem() {}
		MappedItem(int _wx_id, wxMenuItem *_wx_item):wx_id(_wx_id),wx_item(_wx_item) {}
	};
	vector<MappedItem> mapped_items;
	
	/// struct for storing refences to items that will be needed later outside this class
	struct MappedMenu { 
		int wx_id; wxMenu *wx_menu;
		MappedMenu() {}
		MappedMenu(int _wx_id, wxMenu *_wx_menu):wx_id(_wx_id),wx_menu(_wx_menu) {}
	};
	vector<MappedMenu> mapped_menues;
	
	struct myMenu {
		wxString label;
		wxMenu *wx_menu;
		vector<myMenuItem> my_items;
//		vector<wxMenuItem*> wx_items;
	};
	
	enum mnID { mnFILE=0, mnEDIT=1, mnVIEW=2, mnRUN=3, mnDEBUG=4, mnTOOLS=5, mnHELP=6, mnHIDDEN=7, mnCOUNT=8 };
	myMenu menues[mnCOUNT];
	
	void AddMenuItem(int menu_id, const myMenuItem &mi) {
		menues[menu_id].my_items.push_back(mi);
	}
	
	void AddSeparator(int menu_id) {
		menues[menu_id].my_items.push_back(myMenuItem(wxID_ANY,maSEPARATOR));
	}
	
	void BeginSubMenu(int menu_id, const wxString &label, const wxString &description="", const wxString &icon="", int wx_id=wxID_ANY, int properties=0) {
		menues[menu_id].my_items.push_back(myMenuItem(wx_id,maBEGIN_SUBMENU|(wx_id==wxID_ANY?0:maMAPPED)).Label(label).Description(description).Icon(icon).AddProps(properties));
	}
	
	void EndSubMenu(int menu_id) {
		menues[menu_id].my_items.push_back(myMenuItem(wxID_ANY,maEND_SUBMENU));
	}
	
	// submenues de tratamiento especial (porque se actualizan dinamicamente durante la ejecucion)
	wxMenuItem **file_source_history;
	wxMenuItem **file_project_history;
	wxMenuItem **tools_custom_item;
	friend class mxOpenRecentDialog;
	friend class mxMainWindow;
	
public:
	MenusAndToolsConfig(wxMenuBar *_menu_bar);
	void LoadMenuData();
	void LoadToolbarData();
	void ParseMenuConfigLine(const wxString &key, const wxString &value);
	void ParseToolbarConfigLine(const wxString &key, const wxString &value);
	void CreateToolbars();
	
	void CreateMenues();
	
	void SetDebugMode(bool mode);
	
	void SetProjectMode(bool mode);
	
	wxMenuItem *GetItem(int wx_id) {
		for(unsigned int i=0;i<mapped_items.size();i++) { 
			if (mapped_items[i].wx_id==wx_id) return mapped_items[i].wx_item;
		}
		return NULL;
	}
	wxMenu *GetMenu(int wx_id) {
		for(unsigned int i=0;i<mapped_menues.size();i++) { 
			if (mapped_menues[i].wx_id==wx_id) return mapped_menues[i].wx_menu;
		}
		return NULL;
	}
};

extern MenusAndToolsConfig *menu_data;

#define _menu_item(id) menu_data->GetItem(id)

#endif

