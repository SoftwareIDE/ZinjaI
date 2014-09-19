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
#if defined(_WIN32) || defined(__WIN32__)
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
	if (t.visible) pi.Show(); else pi.Hide(); }
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
			if (!project || tb_id!=MenusAndToolsConfig::tbPROJECT) position.visible=false;
		}
	}
}

void mxMainWindow::OnToolRightClick(wxCommandEvent &evt) {
	int id=evt.GetId();
	if (project && id==mxID_RUN_CONFIG) {
		wxMenu menu;
		for(int i=0;i<project->configurations_count;i++) {
			wxMenuItem *mi=menu.AppendRadioItem(mxID_LAST_ID+i,project->configurations[i]->name); // para mostrar en el dialogo
			if (project->configurations[i]==project->active_configuration) mi->Check(true);
		}
		PopupMenu(&menu);
		return;
	}
	if (id>=mxID_CUSTOM_TOOL_0 && id<mxID_CUSTOM_TOOL_0+MAX_CUSTOM_TOOLS) {
		new mxCustomTools(false,id-mxID_CUSTOM_TOOL_0);
		return;
	}
	if (id>=mxID_CUSTOM_PROJECT_TOOL_0 && id<mxID_CUSTOM_PROJECT_TOOL_0+MAX_PROJECT_CUSTOM_TOOLS) {
		new mxCustomTools(true,id-mxID_CUSTOM_PROJECT_TOOL_0);
		return;
	}
	wxString stoolbar;
	switch (menu_data->ToolbarFromTool(id)) {
	case MenusAndToolsConfig::tbFILE: stoolbar="file"; break;
	case MenusAndToolsConfig::tbEDIT: stoolbar="edit"; break;
	case MenusAndToolsConfig::tbVIEW: stoolbar="view"; break;
	case MenusAndToolsConfig::tbDEBUG: stoolbar="debug"; break;
	case MenusAndToolsConfig::tbRUN: stoolbar="run"; break;
	case MenusAndToolsConfig::tbMISC: stoolbar="misc"; break;
	case MenusAndToolsConfig::tbTOOLS: stoolbar="tools"; break;
	}
	mxPreferenceWindow::ShowUp()->SetToolbarPage(stoolbar);
}
