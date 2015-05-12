/**
* @file mxMainWindowMenus
* @brief Contiene las implementaciones de los métodos que gestionan la creación y 
*        visualización de los menúes y barras de herramientas de la ventana principal
**/

#include "mxMainWindow.h"
#include "mxUtils.h"
#include "Language.h"
#include "ids.h"
#include "mxMessageDialog.h"
#include "ProjectManager.h"
#include "mxArt.h"
#include "MenusAndToolsConfig.h"
#include "mxPreferenceWindow.h"


void mxMainWindow::UpdateInHistory(wxString filename, bool is_project) {
	wxString ipre=DIR_PLUS_FILE("16","recent");
	wxString *cfglast = is_project?config->Files.last_project:config->Files.last_source;
	wxMenuItem **mnihistory = is_project?menu_data->file_project_history:menu_data->file_source_history;
	wxMenu *mnurecent = menu_data->GetMenu(is_project?mxID_FILE_PROJECT_RECENT:mxID_FILE_SOURCE_RECENT);
	int history_id = is_project?mxID_FILE_PROJECT_HISTORY_0:mxID_FILE_SOURCE_HISTORY_0, i;
	for (i=0;i<CM_HISTORY_MAX_LEN;i++)
		if (cfglast[i][0]==0 || cfglast[i]==filename)
			break;
	if (i==CM_HISTORY_MAX_LEN) 
		i--;
	for (int j=i;j>0;j--)
		cfglast[j]=cfglast[j-1];
	cfglast[0]=filename;
	// actualizar el menu archivo
	for (i=0;i<config->Init.history_len;i++) {
		wxString icon_fname=wxString(ipre)<<i<<".png";
#ifdef __WIN32__
		if (mnihistory[i])
			mnurecent->Remove(mnihistory[i]);
		if (cfglast[i][0])
			mnihistory[i] = mxUT::AddItemToMenu(mnurecent, history_id+i,cfglast[i],"",cfglast[i],ipre+icon_fname,i);
#else
		if (cfglast[i][0])
			if (mnihistory[i]) {
				if (wxFileName::FileExists(icon_fname))
					mnihistory[i]->SetBitmap(wxBitmap(ipre+icon_fname,wxBITMAP_TYPE_PNG));
				mnurecent->SetLabel(mnihistory[i]->GetId(),cfglast[i]);
			} else {
				mnihistory[i] = mxUT::AddItemToMenu(mnurecent, history_id+i,cfglast[i],"",cfglast[i],ipre+icon_fname,i);
			}
			else
				break;
#endif
	}
}


void mxMainWindow::UpdateCustomTools(bool for_project) {
	
	int count=(for_project?MAX_PROJECT_CUSTOM_TOOLS:MAX_CUSTOM_TOOLS);
//	cfgCustomTool *tools=(for_project?project->custom_tools:config->CustomTools);
	
	if (!for_project) {
		wxMenu *menu_custom_tools = menu_data->GetMenu(mxID_TOOLS_CUSTOM_TOOLS);
		// remove previous items
		for (int i=0;i<count;i++) {
			wxMenuItem *wx_mi = menu_custom_tools->FindItem(mxID_CUSTOM_TOOL_0+i);
			if (wx_mi) menu_custom_tools->Delete(wx_mi);
		}
		// update data in menu_data->menues based con data on config
		menu_data->TransferStatesFromConfig();
		// create new items
		wxString ipre=DIR_PLUS_FILE("16","");
		for (int i=0,c=0;i<count;i++) {
			MenusAndToolsConfig::myMenuItem &mitem = *menu_data->GetMyMenuItem(MenusAndToolsConfig::mnTOOLS,mxID_CUSTOM_TOOL_0+i);
			if (!(mitem.properties&MenusAndToolsConfig::maHIDDEN))
				mxUT::AddItemToMenu(menu_custom_tools, mxID_CUSTOM_TOOL_0+i,mitem.label,mitem.shortcut,mitem.description,ipre+mitem.icon,c++);
		}
	}
	
	menu_data->UpdateToolbar(for_project?MenusAndToolsConfig::tbPROJECT:MenusAndToolsConfig::tbTOOLS,true);
	if (main_window) main_window->aui_manager.Update();
}

void mxMainWindow::SortToolbars(bool update_aui) {
	wxAuiManager &a=aui_manager;
	int c[10]={0};
#define _aui_update_toolbar_pos(NAME) { \
	wxAuiPaneInfo &pi=a.GetPane(_get_toolbar(tb##NAME)); \
	MenusAndToolsConfig::toolbarPosition &t=menu_data->GetToolbarPosition(MenusAndToolsConfig::tb##NAME); \
	pi.LeftDockable(t.right||t.left); \
	pi.RightDockable(t.right||t.left); \
	pi.TopDockable(t.top); \
	if (t.left) pi.Left(); else if (t.right) pi.Right(); else if (t.top) pi.Top(); else pi.Float(); \
	pi.Row(t.row).Position(c[t.row]++); \
	if (!gui_debug_mode&&!gui_fullscreen_mode) \
	{ if (t.visible) pi.Show(); else pi.Hide(); } }
	_aui_update_toolbar_pos(FILE);
	_aui_update_toolbar_pos(EDIT);
	_aui_update_toolbar_pos(VIEW);
	_aui_update_toolbar_pos(TOOLS);
	_aui_update_toolbar_pos(RUN);
	_aui_update_toolbar_pos(MISC);
	_aui_update_toolbar_pos(FIND);
	_aui_update_toolbar_pos(DEBUG);
	_aui_update_toolbar_pos(STATUS);
	_aui_update_toolbar_pos(DIFF);
	if (project) _aui_update_toolbar_pos(PROJECT);
	if (update_aui) a.Update();
}

void mxMainWindow::GetToolbarsPositions() {
	if ((gui_debug_mode&&config->Debug.autohide_toolbars) || (gui_fullscreen_mode&&config->Init.autohide_toolbars_fs)) return;
	
	for(int tb_id=0;tb_id<MenusAndToolsConfig::tbCOUNT_FULL;tb_id++) {
		wxAuiPaneInfo &pi=aui_manager.GetPane(menu_data->GetToolbar(tb_id));
		MenusAndToolsConfig::toolbarPosition &position = menu_data->GetToolbarPosition(tb_id);
		if (pi.IsOk() && pi.IsShown()) {
			position.visible=true;
			position.row=pi.dock_row; 
		} else {
			if (project || tb_id!=MenusAndToolsConfig::tbPROJECT) position.visible=false;
		}
	}
}

static bool auxToolbarPopup(int tool_id, wxMenu &menu, int menu_id, int id1, int id2, int id3=wxID_ANY, int id4=wxID_ANY, int id5=wxID_ANY, int id6=wxID_ANY, int id7=wxID_ANY, int id8=wxID_ANY) {
	if (tool_id!=id1 && tool_id!=id2 && tool_id!=id3 && tool_id!=id4 && tool_id!=id5) return false;
	if (id1!=wxID_ANY) mxUT::AddItemToMenu(&menu,menu_data->GetMyMenuItem(menu_id,id1));
	if (id2!=wxID_ANY) mxUT::AddItemToMenu(&menu,menu_data->GetMyMenuItem(menu_id,id2));
	if (id3!=wxID_ANY) mxUT::AddItemToMenu(&menu,menu_data->GetMyMenuItem(menu_id,id3));
	if (id4!=wxID_ANY) mxUT::AddItemToMenu(&menu,menu_data->GetMyMenuItem(menu_id,id4));
	if (id5!=wxID_ANY) mxUT::AddItemToMenu(&menu,menu_data->GetMyMenuItem(menu_id,id5));
	if (id6!=wxID_ANY) mxUT::AddItemToMenu(&menu,menu_data->GetMyMenuItem(menu_id,id6));
	if (id7!=wxID_ANY) mxUT::AddItemToMenu(&menu,menu_data->GetMyMenuItem(menu_id,id7));
	if (id8!=wxID_ANY) mxUT::AddItemToMenu(&menu,menu_data->GetMyMenuItem(menu_id,id8));
	return true;
}

void mxMainWindow::OnToolRightClick(wxCommandEvent &evt) {
	wxMenu menu;
	int id=evt.GetId();
	
	if (project && id==mxID_RUN_CONFIG) {
		for(int i=0;i<project->configurations_count;i++) {
			wxMenuItem *mi=menu.AppendRadioItem(mxID_LAST_ID+i,project->configurations[i]->name); // para mostrar en el dialogo
			if (project->configurations[i]==project->active_configuration) mi->Check(true);
		}
		
	} else if ( id==mxID_TOOLS_CUSTOM_TOOLS_SETTINGS || id==mxID_TOOLS_PROJECT_TOOLS_SETTINGS ||
		(id>=mxID_CUSTOM_TOOL_0 && id<mxID_CUSTOM_TOOL_0+MAX_CUSTOM_TOOLS) || 
		(id>=mxID_CUSTOM_PROJECT_TOOL_0 && id<mxID_CUSTOM_PROJECT_TOOL_0+MAX_PROJECT_CUSTOM_TOOLS) ) 
	{
		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_CUSTOM_TOOLS_SETTINGS));
		if (project) mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_PROJECT_TOOLS_SETTINGS));
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnTOOLS,
		mxID_TOOLS_VALGRIND_RUN,mxID_TOOLS_VALGRIND_DEBUG,mxID_TOOLS_VALGRIND_VIEW,mxID_TOOLS_VALGRIND_HELP
	)) {
		;
		
	} else if ( project && auxToolbarPopup(id,menu,MenusAndToolsConfig::mnTOOLS,
		mxID_TOOLS_DOXY_GENERATE,mxID_TOOLS_DOXY_CONFIG,mxID_TOOLS_DOXY_VIEW,mxID_TOOLS_DOXY_HELP
	)) {
		;
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnTOOLS,
		mxID_TOOLS_GPROF_SHOW,mxID_TOOLS_GPROF_LIST,mxID_TOOLS_GPROF_SET,mxID_TOOLS_GPROF_HELP
	)) {
		;
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnTOOLS,
		mxID_TOOLS_GCOV_SHOW,mxID_TOOLS_GCOV_RESET,mxID_TOOLS_GCOV_SET,mxID_TOOLS_GCOV_SHOW
	)) {
		;
		
	} else if ( project && auxToolbarPopup(id,menu,MenusAndToolsConfig::mnTOOLS,
		mxID_TOOLS_CPPCHECK_RUN,mxID_TOOLS_CPPCHECK_CONFIG,mxID_TOOLS_CPPCHECK_VIEW,mxID_TOOLS_CPPCHECK_HELP
	)) {
		;
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnTOOLS,
		mxID_TOOLS_DIFF_TWO,mxID_TOOLS_DIFF_DISK,mxID_TOOLS_DIFF_HIMSELF,mxID_TOOLS_DIFF_CLEAR,mxID_TOOLS_DIFF_HELP
	)) {
		;
		
	} else if ( project && auxToolbarPopup(id,menu,MenusAndToolsConfig::mnTOOLS,
		mxID_TOOLS_WXFB_CONFIG,mxID_TOOLS_WXFB_REGEN,mxID_TOOLS_WXFB_INHERIT_CLASS,mxID_TOOLS_WXFB_UPDATE_INHERIT,mxID_TOOLS_WXFB_NEW_RES,mxID_TOOLS_WXFB_LOAD_RES,mxID_TOOLS_WXFB_HELP_WX,mxID_TOOLS_WXFB_HELP_WX
	)) {
		;
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnTOOLS,
		mxID_TOOLS_SHARE_SHARE,mxID_TOOLS_SHARE_OPEN,mxID_TOOLS_SHARE_LIST,mxID_TOOLS_SHARE_HELP
	)) {
		;
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnTOOLS,
		mxID_TOOLS_PREPROC_EXPAND_MACROS,mxID_TOOLS_PREPROC_MARK_VALID,mxID_TOOLS_PREPROC_UNMARK_ALL,mxID_TOOLS_PREPROC_HELP
	)) {
		;
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnRUN,
		mxID_RUN_RUN,mxID_RUN_RUN_OLD,mxID_RUN_COMPILE,mxID_RUN_CLEAN,mxID_RUN_CONFIG
	)) {
		;
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnDEBUG,
		mxID_DEBUG_RUN, mxID_DEBUG_ATTACH,mxID_DEBUG_LOAD_CORE_DUMP,mxID_DEBUG_TARGET
	)) {
		;
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnVIEW,
		mxID_VIEW_CODE_COLOURS,mxID_VIEW_CODE_STYLE
	)) {
		;
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnFILE,
		mxID_FILE_NEW,mxID_FILE_PROJECT
	)) {
		;
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnFILE,
		mxID_FILE_SAVE,mxID_FILE_SAVE_AS,mxID_FILE_SAVE_ALL,(project?int(mxID_FILE_SAVE_PROJECT):int(wxID_ANY))
	)) {
		;
		
	} else if ( auxToolbarPopup(id,menu,MenusAndToolsConfig::mnFILE,
		mxID_FILE_OPEN,mxID_FILE_RELOAD,mxID_FILE_SOURCE_RECENT,mxID_FILE_PROJECT_RECENT
	)) {
		;
		
	}
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_VALGRIND_DEBUG));
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_VALGRIND_VIEW));
//	} else if (id==mxID_TOOLS_DOXY_GENERATE) {
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_DOXY_VIEW));
//	} else if (id==mxID_TOOLS_DOXY_VIEW) {
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_DOXY_GENERATE));
//	} else if (id==mxID_TOOLS_CPPCHECK_RUN) {
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_CPPCHECK_CONFIG));
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_CPPCHECK_VIEW));
//	} else if (id==mxID_TOOLS_GPROF_SHOW) {
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_GPROF_SET));
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_GPROF_LIST));
//	} else if (id==mxID_TOOLS_GPROF_LIST) {
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_GPROF_SET));
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_GPROF_SHOW));
//	} else if (id==mxID_TOOLS_GCOV_SHOW) {
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_GCOV_RESET));
//		mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_GCOV_SET));
//	}
	if (menu.GetMenuItemCount()) menu.AppendSeparator();
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnHIDDEN,mxID_TOOLBAR_SETTINGS));
	PopupMenu(&menu);
//	wxString stoolbar;
//	switch (menu_data->ToolbarFromTool(id)) {
//	case MenusAndToolsConfig::tbFILE: stoolbar="file"; break;
//	case MenusAndToolsConfig::tbEDIT: stoolbar="edit"; break;
//	case MenusAndToolsConfig::tbVIEW: stoolbar="view"; break;
//	case MenusAndToolsConfig::tbDEBUG: stoolbar="debug"; break;
//	case MenusAndToolsConfig::tbRUN: stoolbar="run"; break;
//	case MenusAndToolsConfig::tbMISC: stoolbar="misc"; break;
//	case MenusAndToolsConfig::tbTOOLS: stoolbar="tools"; break;
//	}
	
}
