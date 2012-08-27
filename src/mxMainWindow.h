#ifndef MX_MAIN_WINDOW_H
#define MX_MAIN_WINDOW_H

#include <wx/wxprec.h>

#include <wx/frame.h>
#include <wx/process.h>
#include <wx/treebase.h>
#include <wx/aui/aui.h>
#include <wx/socket.h>
#include <wx/html/htmlwin.h>

#define IF_THERE_IS_SOURCE if (notebook_sources->GetPageCount()>0)
#define CURRENT_SOURCE ((mxSource*)notebook_sources->GetPage(notebook_sources->GetSelection()))

class mxStatusBar;
class mxHidenPanel;
class mxDiffSideBar;
class mxValgrindOuput;
class mxSource;
class mxOpenSharedWindow;
class ProjectManager;
class Parser;
class mxNewWizard;
class mxFindDialog;
class mxThreadGrid;
class mxBacktraceGrid;
class mxInspectionGrid;
class mxPreferenceWindow;
class mxSplashScreen;
class mxCompilerItemData;
class wxListBox;
class wxStaticText;
class wxTreeCtrl;
class wxTextCtrl;
class compile_and_run_struct_single;

extern mxSplashScreen *splash;

enum autohide_ref {ATH_COMPILER=0,ATH_SYMBOL=1,ATH_PROJECT=2,ATH_EXPLORER=3,ATH_DEBUG_LOG=4,ATH_QUICKHELP=5,ATH_THREADS=6,ATH_INSPECTIONS=7,ATH_BACKTRACE=8,ATH_BEGINNERS=9,ATH_COUNT=10};

;

/**
* @brief Ventana principal de la aplicación
* 
* Representa la ventana principal de la aplicación. Hay sólo una instancia de esta
* clase, que la crea mxApplication, y el puntero es main_window (variable global).
**/
class mxMainWindow : public wxFrame {
public:
	
	bool fullscreen_toolbars_status[10];
	bool debug_toolbars_status[10];
	bool fullscreen_panels_status[10];
	bool debug_panels_status[10];
	mxSource *focus_source;
	bool toolbar_first_time;

	mxValgrindOuput *valgrind_panel;
	void ShowValgrindPanel(char what, wxString file);
	
	wxListBox *debug_log_panel;
	void AddToDebugLog(wxString str);
	void ClearDebugLog();
	
	mxMainWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	
	~mxMainWindow();
	
	void CreateMenus();
	void CreateToolbars(wxToolBar *wich_one=NULL);
	void OnExit(wxCommandEvent& event);
	void OnEditNeedFocus (wxCommandEvent &event);
	void OnEdit (wxCommandEvent &event);
	void OnPreferences (wxCommandEvent &event);
	
	void OnFileOpenFolder (wxCommandEvent &event);
	void OnFileExploreFolder (wxCommandEvent &event);
	void OnFileProperties (wxCommandEvent &event);
	void OnFileNew (wxCommandEvent &event);
	void OnFilePrint (wxCommandEvent &event); ///< Callback de menu: Archivo->Imprimir
	void OnFileNewProject (wxCommandEvent &event);
	void OnFileReload (wxCommandEvent &event);
	void OnFileExportHtml (wxCommandEvent &event);
	void OnFileOpen (wxCommandEvent &event);
	void OnFileSourceHistory(wxCommandEvent &event);
	void OnFileSourceHistoryMore(wxCommandEvent &event);
	void OnFileProjectHistory(wxCommandEvent &event);
	void OnFileProjectHistoryMore(wxCommandEvent &event);
	void OnFileOpenH (wxCommandEvent &event);
	void OnFileOpenSelected (wxCommandEvent &event);
	bool CloseFromGui(int i=-1);
	void OnFileClose (wxCommandEvent &event);
	void OnFileCloseAll (wxCommandEvent &event);
	void OnFileCloseAllButOne (wxCommandEvent &event);
	void OnFileCloseProject (wxCommandEvent &event);
	void OnFileSaveProject (wxCommandEvent &event);
	void OnFileSave (wxCommandEvent &event);
	void OnFileSaveAs (wxCommandEvent &event);
	void OnFileSaveAll (wxCommandEvent &event);
	void OnFileProjectConfig (wxCommandEvent &event);
	
	void OnGotoToolbarFind (wxCommandEvent &event);
	void OnToolbarFindEnter (wxCommandEvent &event);
	void OnToolbarFindChange (wxCommandEvent &event);
	
	void OnEditFind (wxCommandEvent &event);
	void OnEditFindNext (wxCommandEvent &event);
	void OnEditFindPrev (wxCommandEvent &event);
	void OnEditReplace (wxCommandEvent &event);
	void OnEditGoto (wxCommandEvent &event);
	void OnEditGotoFunction (wxCommandEvent &event);
	void OnEditGotoFile (wxCommandEvent &event);
	void OnEditInsertInclude (wxCommandEvent &event);
	void OnEditListMarks (wxCommandEvent &event);
	
	void OnViewDuplicateTab (wxCommandEvent &event);
	void OnViewBeginnerPanel (wxCommandEvent &event);
	void OnViewNextError (wxCommandEvent &event);
	void OnViewPrevError (wxCommandEvent &event);
	void OnViewWhiteSpace (wxCommandEvent &event);
	void OnViewLineWrap (wxCommandEvent &event);
	void OnViewCodeStyle (wxCommandEvent &event);
	void OnViewToolbarTools (wxCommandEvent &event);
	void OnViewToolbarView (wxCommandEvent &event);
	void OnViewToolbarFile (wxCommandEvent &event);
	void OnViewToolbarFind (wxCommandEvent &event);
	void OnViewToolbarDebug (wxCommandEvent &event);
	void OnViewToolbarEdit (wxCommandEvent &event);
	void OnViewToolbarRun (wxCommandEvent &event);
	void OnViewToolbarMisc (wxCommandEvent &event);
	void OnViewExplorerTree (wxCommandEvent &event);
	void OnViewCompilerTree (wxCommandEvent &event);
	void OnViewLeftPanels (wxCommandEvent &event);
	void OnViewProjectTree (wxCommandEvent &event);
	void OnViewSymbolsTree (wxCommandEvent &event);
	void OnViewUpdateSymbols (wxCommandEvent &event);
	void OnViewNotebookNext (wxCommandEvent &event);
	void OnViewNotebookPrev (wxCommandEvent &event);
	void OnViewHideBottom (wxCommandEvent &event);
	void OnViewFullScreen (wxCommandEvent &event);

	void OnFoldShow1 (wxCommandEvent &event);
	void OnFoldShow2 (wxCommandEvent &event);
	void OnFoldShow3 (wxCommandEvent &event);
	void OnFoldShow4 (wxCommandEvent &event);
	void OnFoldShow5 (wxCommandEvent &event);
	void OnFoldShowAll (wxCommandEvent &event);
	void OnFoldHide1 (wxCommandEvent &event);
	void OnFoldHide2 (wxCommandEvent &event);
	void OnFoldHide3 (wxCommandEvent &event);
	void OnFoldHide4 (wxCommandEvent &event);
	void OnFoldHide5 (wxCommandEvent &event);
	void OnFoldHideAll (wxCommandEvent &event);
	void OnFoldFold (wxCommandEvent &event);
	void OnFoldUnFold (wxCommandEvent &event);

	void OnRunRun (wxCommandEvent &event);
	void OnRunRunOld (wxCommandEvent &event);
	void OnRunCompile (wxCommandEvent &event);
	void OnRunBuild (wxCommandEvent &event);
	void OnRunCompileConfig (wxCommandEvent &event);
	void OnRunExportMakefile (wxCommandEvent &event);
	void OnRunClean (wxCommandEvent &event);
	void OnRunStop (wxCommandEvent &event);
	
	void OnDebugAttach (wxCommandEvent &event);
	void OnDebugCoreDump (wxCommandEvent &event);
	void OnDebugDoThat (wxCommandEvent &event);
	void OnDebugRun (wxCommandEvent &event);
	void OnDebugPause (wxCommandEvent &event);
	void OnDebugContinue (wxCommandEvent &event);
	void OnDebugJump (wxCommandEvent &event);
	void OnDebugStop (wxCommandEvent &event);
	void OnDebugReturn (wxCommandEvent &event);
	void OnDebugRunUntil (wxCommandEvent &event);
	void OnDebugStepIn (wxCommandEvent &event);
	void OnDebugStepOut (wxCommandEvent &event);
	void OnDebugStepOver (wxCommandEvent &event);
	void OnDebugInspect (wxCommandEvent &event);
	void OnDebugThreadList (wxCommandEvent &event);
	void OnDebugBacktrace (wxCommandEvent &event);
	void OnDebugToggleBreakpoint (wxCommandEvent &event);
	void OnDebugListBreakpoints (wxCommandEvent &event);
	void OnDebugListWatchpoints (wxCommandEvent &event);
	void OnDebugInsertWatchpoint (wxCommandEvent &event);
	void OnDebugFunctionBreakpoint (wxCommandEvent &event);
	void OnDebugBreakpointOptions (wxCommandEvent &event);
	void OnDebugShowLogPanel (wxCommandEvent &event);
	void OnDebugEnableInverseExecution (wxCommandEvent &event);
	void OnDebugInverseExecution (wxCommandEvent &event);
	
	void OnInternalInfo(wxCommandEvent &event);
	
	void OnToolsCodeCopyFromH (wxCommandEvent &event);
	void OnToolsConsole (wxCommandEvent &event);
	void OnToolsExeProps (wxCommandEvent &event);
	void OnToolsProjectStatistics (wxCommandEvent &event);
	
	void OnToolsAlignComments (wxCommandEvent &event);
	void OnToolsRemoveComments (wxCommandEvent &event);
	void OnToolsShareShare (wxCommandEvent &event);
	void OnToolsShareList (wxCommandEvent &event);
	void OnToolsShareOpen (wxCommandEvent &event);
	void OnToolsShareHelp (wxCommandEvent &event);
	void OnToolsDrawFlow (wxCommandEvent &event);
	void OnToolsDrawClasses (wxCommandEvent &event);
	void OnToolsDrawProject (wxCommandEvent &event);
	void OnToolsDiffPrevMark(wxCommandEvent &event);
	void OnToolsDiffNextMark(wxCommandEvent &event);
	void OnToolsDiffTwoSources(wxCommandEvent &event);
	void OnToolsDiffToDiskFile(wxCommandEvent &event);
	void OnToolsDiffToHimself(wxCommandEvent &event);
	void OnToolsDiffClear(wxCommandEvent &event);
	void OnToolsDiffShow(wxCommandEvent &event);
	void OnToolsDiffApply(wxCommandEvent &event);
	void OnToolsDiffDiscard(wxCommandEvent &event);
	void OnToolsDiffHelp(wxCommandEvent &event);
	void OnToolsDoxyConfig(wxCommandEvent &event);
	void OnToolsDoxyGenerate(wxCommandEvent &event);
	void OnToolsDoxyView(wxCommandEvent &event);
	void OnToolsDoxyHelp(wxCommandEvent &event);
	void OnToolsWxfbNewRes(wxCommandEvent &event);
	void OnToolsWxfbLoadRes(wxCommandEvent &event);
	void OnToolsWxfbRegen(wxCommandEvent &event);
	void OnToolsWxfbActivate(wxCommandEvent &event);
	void OnToolsWxfbAuto(wxCommandEvent &event);
	void OnToolsWxfbUpdateInherit(wxCommandEvent &event);
	void OnToolsWxfbInheritClass(wxCommandEvent &event);
	void OnToolsWxfbHelp(wxCommandEvent &event);
	void OnToolsWxfbHelpWx(wxCommandEvent &event);
	void OnToolsGprofSet(wxCommandEvent &event);
	void OnToolsGprofShow(wxCommandEvent &event);
	void OnToolsGprofList(wxCommandEvent &event);
	void OnToolsGprofHelp(wxCommandEvent &event);
	void OnToolsGprofDot(wxCommandEvent &event);
	void OnToolsGprofFdp(wxCommandEvent &event);
	void OnToolsCppCheckRun(wxCommandEvent &event);
	void OnToolsCppCheckConfig(wxCommandEvent &event);
	void OnToolsCppCheckView(wxCommandEvent &event);
	void OnToolsCppCheckHelp(wxCommandEvent &event);
#if !defined(_WIN32) && !defined(__WIN32__)
	void OnToolsValgrindRun(wxCommandEvent &event);
	void OnToolsValgrindView(wxCommandEvent &event);
	void OnToolsValgrindHelp(wxCommandEvent &event);
#endif
	void OnToolsCustomTool(wxCommandEvent &event);
	void RunCustomTool(wxString name, wxString workdir, wxString cmd, bool console);
	void OnToolsCustomSettings(wxCommandEvent &event);
	void OnToolsCustomHelp(wxCommandEvent &event);
	
	void OnToolsPreprocMarkValid ( wxCommandEvent &event );
	void OnToolsPreprocUnMarkAll ( wxCommandEvent &event );
	void OnToolsPreprocReplaceMacros ( wxCommandEvent &event );
	void OnToolsPreprocHelp ( wxCommandEvent &event );
	void ToolsPreproc( int id_command );
	
	void OnToolsInstallComplements( wxCommandEvent &event );
	
	void OnHelpOpinion (wxCommandEvent &event);
	void OnHelpTutorial (wxCommandEvent &event);
	void OnHelpAbout (wxCommandEvent &event);
	void OnHelpGui (wxCommandEvent &event);
	void OnHelpCpp (wxCommandEvent &event);
	void OnHelpTip (wxCommandEvent &event);
	void OnHelpUpdates (wxCommandEvent &event);

	void OnClose (wxCloseEvent &event);
	void OnPaneClose (wxAuiManagerEvent &event);
	void OnNotebookRightClick(wxAuiNotebookEvent& event);
	void OnNotebookPageClose(wxAuiNotebookEvent& event);
	void OnNotebookPageChanged(wxAuiNotebookEvent& event);
	void OnProcessTerminate (wxProcessEvent &event);
	void OnQuickHelpLink (wxHtmlLinkEvent &event);

	void OnSelectSource (wxTreeEvent &event);
	void OnSelectError (wxTreeEvent &event);
	void OnSelectTreeItem (wxTreeEvent &event);
	void OnSelectExplorerItem (wxTreeEvent &event);
	void OnSymbolsGenerateCache(wxCommandEvent &evt);
	
	void OnExplorerTreePopup(wxTreeEvent &event);
	void OnExplorerTreeUpdate(wxCommandEvent &evt);
	void OnExplorerTreeChangePath(wxCommandEvent &evy);
	void OnExplorerTreePathUp(wxCommandEvent &evy);
	void OnExplorerTreeOpenOne(wxCommandEvent &evt);
	void OnExplorerTreeOpenAll(wxCommandEvent &evt);
	void OnExplorerTreeOpenSources(wxCommandEvent &evt);
	void OnExplorerTreeShowOnlySources(wxCommandEvent &evt);
	void OnExplorerTreeSetAsPath(wxCommandEvent &evt);
	wxString GetExplorerItemPath(wxTreeItemId item);
	
	void OnSymbolTreePopup(wxTreeEvent &event);
	void OnSymbolTreeDec(wxCommandEvent &event);
	void OnSymbolTreeDef(wxCommandEvent &event);
	void OnSymbolTreeIncludes(wxCommandEvent &event);

	void OnCompilerTreeShowFull(wxCommandEvent &event);
	void OnCompilerTreePopup(wxTreeEvent &event);
	
	void OnProjectTreePopup(wxTreeEvent &event);
	void OnProjectTreeToggleFullPath(wxCommandEvent &event);
	void OnProjectTreeOpenFolder(wxCommandEvent &event);
	void OnProjectTreeProperties(wxCommandEvent &event);
	void OnProjectTreeOpen(wxCommandEvent &event);
	void OnProjectTreeOpenAll(wxCommandEvent &event);
	void OnProjectTreeCompileNow(wxCommandEvent &event);
	void OnProjectTreeCompileFirst(wxCommandEvent &event);
	void OnProjectTreeRename(wxCommandEvent &event);
	void OnProjectTreeDelete(wxCommandEvent &event);
	void OnProjectTreeMoveToSources(wxCommandEvent &event);
	void OnProjectTreeMoveToHeaders(wxCommandEvent &event);
	void OnProjectTreeMoveToOthers(wxCommandEvent &event);
	void OnProjectTreeAdd(wxCommandEvent &event);
	void OnProjectTreeAddMultiple(wxCommandEvent &event);
	void OnProjectTreeAddSelected(wxCommandEvent &event);

	void OnWhereAmI(wxCommandEvent &event);
//	void OnWhereTime(wxTimerEvent &event);
	void OnFocusTime(wxTimerEvent &event);
	void OnParseSourceTime(wxTimerEvent &event);
	void OnParseOutputTime(wxTimerEvent &event);
	
	void OnActivate (wxActivateEvent &event);
	
	void OnSocketEvent(wxSocketEvent &event);
	
	mxStatusBar *status_bar;
	wxStatusBar *OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name);
	void SetStatusBarFields();
	void SetStatusText(wxString text);
	void SetStatusProgress(int prog);

	mxSource *NewFileFromTemplate (wxString filename);
	mxSource *NewFileFromText (wxString text, int pos=0);
	mxSource *NewFileFromText (wxString text, wxString name, int pos=0);
	mxSource *OpenFile (wxString filename, bool add_to_project=true);
	mxSource *FindSource(wxFileName filename, int *pos=NULL);
	void OpenFileFromGui (wxFileName filename, int *multiple=NULL);
	wxTreeItemId AddToProjectTreeSimple(wxFileName filename, char where='u');
	wxTreeItemId AddToProjectTreeProject(wxString filename, char where, bool sort=true);
	mxSource *IsOpen (wxFileName filename);
	mxSource *IsOpen (wxTreeItemId tree_item);
	bool CloseSource(int i);
	void UpdateInHistory(wxString filename);
	
	void UpdateCustomTools();

	void ShowQuickHelp (wxString keyword, bool hide_compiler_tree=true);
	void RunSource(mxSource *source);
	void UpdateSymbols();
	
	void ShowInQuickHelpPanel(wxString &res, bool hide_compiler_tree=true); ///< carga el fuente de una pagina desde la cadena res en el panel de ayuda rápida y la muestra
	void LoadInQuickHelpPanel(wxString file, bool hide_compiler_tree=true); ///< carga una pagina desde un archivo en el panel de ayuda rápida y lo muestra
	
	void PrepareGuiForDebugging(bool debug_mode);
	void PrepareGuiForProject(bool project_mode);
	
	void OnToolRightClick(wxCommandEvent &evt);
	
	void OnKey(wxKeyEvent &evt);
	void OnKeyEvent(wxWindow *who, wxKeyEvent &evt);
	
	wxAuiNotebook *notebook_sources;
	wxAuiNotebook *left_panels;
	
	wxString AvoidDuplicatePageText(wxString ptext);
	
	void SetExplorerPath(wxString path);
	

	int untitled_count;

	mxFindDialog *find_replace_dialog;

	mxOpenSharedWindow *open_shared;
	
	//! cambia el titulo de la ventana
	void SetOpenedFileName(wxString name);
	
	void StartExecutionStuff (bool compile, bool run, compile_and_run_struct_single *compile_and_run, wxString msg);
	
	//! Componentes del árbol del explorador de archivos
	struct explorer_tree_struct {
		bool show_only_sources;
		wxString path;
		wxTreeCtrl *treeCtrl;
		wxMenuItem *menuItem;
		wxTreeItemId selected_item, root;
	} explorer_tree;
	
	//! Componentes del árbol de proyecto
	struct project_tree_struct {
		wxTreeCtrl *treeCtrl;
		wxMenuItem *menuItem;
		wxTreeItemId selected_item, selected_parent;
		wxTreeItemId root ,sources, headers, others;
	} project_tree;
	
	//! Componentes del árbol de resultados de la compilación
	struct compiler_tree_struct {
		wxTreeCtrl *treeCtrl;
		wxMenuItem *menuItem;
		wxTreeItemId root, state, errors, warnings, all;
	} compiler_tree;

	//! Componentes del árbol de simbolos
	struct symbols_tree_struct {
		wxTreeCtrl *treeCtrl;
		wxMenuItem *menuItem;
	} symbols_tree; 

	wxAuiManager aui_manager;

	//! Componentes de la barra de menúes de la ventana principal
	struct menu_struct {
		wxMenuBar *menu;
		wxMenu *file;
		wxMenu *edit;
		wxMenu *run;
		wxMenu *help;
		wxMenu *tools;
		wxMenu *view;
		wxMenuItem *file_save_project;
		wxMenuItem *file_close_project;
		wxMenuItem *file_project_config;
		wxMenu *file_source_recent;
		wxMenu *file_project_recent;
		wxMenuItem **file_source_history;
		wxMenuItem **file_project_history;
		wxMenuItem *run_stop;
		wxMenuItem *run_build;
		wxMenuItem *run_run;
		wxMenuItem *run_compile;
		wxMenuItem *run_clean;
		wxMenuItem *tools_makefile;
		wxMenuItem *tools_proy_graph;
		wxMenuItem *tools_stats;
		wxMenuItem *tools_doxygen;
		wxMenuItem *tools_wxfb;
		wxMenuItem *tools_cppcheck;
		wxMenuItem *tools_wxfb_activate;
		wxMenuItem *tools_wxfb_auto;
		wxMenuItem *tools_wxfb_regen;
		wxMenuItem *tools_wxfb_inherit;
		wxMenuItem *tools_wxfb_update_inherit;
		wxMenu *tools_custom_menu;
		wxMenuItem **tools_custom_item;
		wxMenu *debug;
		wxMenuItem *debug_run;
		wxMenuItem *debug_attach;
		wxMenuItem *debug_pause;
#if !defined(_WIN32) && !defined(__WIN32__)
		wxMenuItem *debug_core_dump;
#endif
		wxMenuItem *debug_continue;
		wxMenuItem *debug_stop;
		wxMenuItem *debug_threadlist;
		wxMenuItem *debug_backtrace;
		wxMenuItem *debug_inspect;
		wxMenuItem *debug_step_in;
		wxMenuItem *debug_step_out;
		wxMenuItem *debug_step_over;
		wxMenuItem *debug_return;
		wxMenuItem *debug_jump;
		wxMenuItem *debug_run_until;
		wxMenuItem *debug_function_breakpoint;
		wxMenuItem *debug_list_breakpoints;
		wxMenuItem *debug_toggle_breakpoint;
		wxMenuItem *debug_list_watchpoints;
		wxMenuItem *debug_inverse_execution;
		wxMenuItem *debug_enable_inverse_execution;
		wxMenuItem *debug_insert_watchpoint;
		wxMenuItem *view_left_panels;
		wxMenuItem *view_fullscreen;
		wxMenuItem *view_white_space;
		wxMenuItem *view_line_wrap;
		wxMenuItem *view_code_style;
		wxMenuItem *view_toolbar_view;
		wxMenuItem *view_toolbar_debug;
		wxMenuItem *view_toolbar_tools;
		wxMenuItem *view_toolbar_file;
		wxMenuItem *view_toolbar_edit;
		wxMenuItem *view_toolbar_run;
		wxMenuItem *view_toolbar_misc;
		wxMenuItem *view_toolbar_find;
		wxMenuItem *view_beginner_panel;
		wxMenuItem *tools_gprof_dot;
		wxMenuItem *tools_gprof_fdp;
		wxMenuItem *file_open;
	} menu;

	wxTimer *parser_timer, *focus_timer/*, *where_timer*/;

	wxToolBar *toolbar_file, *toolbar_edit, *toolbar_run, *toolbar_misc, *toolbar_debug, *toolbar_find, *toolbar_status, *toolbar_tools, *toolbar_view, *toolbar_diff;
	wxStaticText *toolbar_status_text;

	wxAuiNotebook *CreateNotebookSources();
	
	wxAuiNotebook *CreateLeftPanels();
	void OnNotebookPanelsChanged(wxAuiNotebookEvent& event);
	
	wxMenuItem *view_quick_help;

	wxHtmlWindow* quick_help;
	wxHtmlWindow* CreateQuickHelp(wxWindow* parent = NULL);
	
	void ShowWelcome(bool show=true); ///< alterna entre mostrar el notebook de fuentes y el panel de bienvenida

	wxTextCtrl* toolbar_find_text;
	
	wxTreeCtrl* CreateSymbolsTree();
	wxTreeCtrl* CreateExplorerTree();
	wxTreeCtrl* CreateProjectTree();
	wxTreeCtrl* CreateCompilerTree();
	
	void CreateBeginnersPanel();

	mxThreadGrid *threadlist_ctrl;
	mxBacktraceGrid *backtrace_ctrl;
	mxInspectionGrid *inspection_ctrl;
	
	mxDiffSideBar *diff_sidebar;
	void ShowDiffSideBar(bool bar, bool map);
	
	mxSource *GetCurrentSource();
	
	void SetAccelerators();
	void OnEscapePressed(wxCommandEvent &event);
	

	mxHidenPanel *autohide_handlers[ATH_COUNT];
	void OnResize(wxSizeEvent &evt);
	void ShowQuickHelpPanel(bool hide_compiler_tree=true);
	void ShowCompilerTreePanel();
	void ShowExplorerTreePanel();
	void ShowBeginnersPanel();
	
	void FocusToSource();
	
	DECLARE_EVENT_TABLE();
};

extern mxSource *external_source; // para que devuelva OpenFile cuando al archivo lo abre otro y no ZinjaI

extern mxMainWindow *main_window;

#endif

