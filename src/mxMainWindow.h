#ifndef MX_MAIN_WINDOW_H
#define MX_MAIN_WINDOW_H

#include <wx/frame.h>
#include <wx/process.h>
#include <wx/treebase.h>
#include <wx/aui/aui.h>
#include <wx/socket.h>
#include <wx/html/htmlwin.h>

#define IF_THERE_ISNT_SOURCE if (notebook_sources->GetPageCount()==0)
#define IF_THERE_IS_SOURCE if (notebook_sources->GetPageCount()>0)
#define CURRENT_SOURCE ((mxSource*)notebook_sources->GetPage(notebook_sources->GetSelection()))
#include "mxCustomTools.h"
#include "enums.h"
#include "mxSource.h"

class mxStatusBar;
class mxHidenPanel;
class mxGCovSideBar;
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
class mxInspectionsPanel;
class mxPreferenceWindow;
class mxSplashScreen;
class mxCompilerItemData;
class wxListBox;
class wxStaticText;
class wxTreeCtrl;
class wxTextCtrl;
class compile_and_run_struct_single;
class project_file_item;
class mxExternCompilerOutput;
class mxRegistersGrid;
class mxGdbAsmPanel;

template<class T> class SingleList;

extern mxSplashScreen *g_splash;

enum autohide_ref {ATH_COMPILER=0,ATH_SYMBOL=1,ATH_PROJECT=2,ATH_EXPLORER=3,ATH_DEBUG_LOG=4,ATH_QUICKHELP=5,ATH_THREADS=6,ATH_INSPECTIONS=7,ATH_BACKTRACE=8,ATH_BEGINNERS=9,ATH_COUNT=10};

/**
* @brief Ventana principal de la aplicación
* 
* Representa la ventana principal de la aplicación. Hay sólo una instancia de esta
* clase, que la crea mxApplication, y el puntero es main_window (variable global).
**/
class mxMainWindow : public wxFrame {
public:
	
	bool gui_fullscreen_mode, gui_debug_mode, gui_project_mode; 
	
	enum {fspsCOMPILER,fspsHELP,fspsBACKTRACE,fspsINSPECTIONS,fspsLEFT,fspsPROJECT,fspsEXPLORER,fspsBEGINNER,fspsTHREADS,fspsCOUNT};
	bool fullscreen_panels_status[fspsCOUNT];
	
	enum {dbpsCOMPILER,dbpsHELP,dbpsBACKTRACE,dbpsINSPECTIONS,dbpsTHREADS,dbpsCOUNT};
	bool debug_panels_status[dbpsCOUNT];
	
	mxSource *focus_source; // auxiliar para el NavigationHistory
	
	mxSource *master_source; /// if there's no project, defining a master source force zinjai to always compila and run that one, no the current one

	mxValgrindOuput *valgrind_panel;
	void ShowValgrindPanel(int what, wxString file, bool force=true);
	
	mxRegistersGrid *registers_panel;
	mxGdbAsmPanel *asm_panel;
	wxListBox *debug_log_panel;
	void AddToDebugLog(wxString str);
	void ClearDebugLog();
	
	mxMainWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	
	~mxMainWindow();
//private:
//	void PopulateMenuFile(const wxString &ipre);
//	void PopulateMenuEdit(const wxString &ipre);
//	void PopulateMenuView(const wxString &ipre);
//	void PopulateMenuRun(const wxString &ipre);
//	void PopulateMenuDebug(const wxString &ipre);
//	void PopulateMenuTools(const wxString &ipre);
//	void PopulateMenuHelp(const wxString &ipre);
public:
	void GetToolbarsPositions();
	void SortToolbars(bool update_aui=true);
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
	void OnFileSetAsMaster(wxCommandEvent &event);
	void OnFileProjectConfig (wxCommandEvent &event);
	void OnHighlightKeyword(wxCommandEvent &event);
	void OnChangeShortcuts(wxCommandEvent &event);
	
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
	void OnSourceGotoDefinition (wxCommandEvent &event);
	
	void OnViewDuplicateTab (wxCommandEvent &event);
	void OnViewBeginnerPanel (wxCommandEvent &event);
	void OnViewNextError (wxCommandEvent &event);
	void OnViewPrevError (wxCommandEvent &event);
	void OnViewWhiteSpace (wxCommandEvent &event);
	void OnViewLineWrap (wxCommandEvent &event);
	void OnViewCodeColours (wxCommandEvent &event);
	void OnViewCodeStyle (wxCommandEvent &event);
	void OnToggleToolbar(int menu_item_id, int toolbar_id, bool update_aui=true);
	void OnViewToolbarProject (wxCommandEvent &event);
	void OnViewToolbarTools (wxCommandEvent &event);
	void OnViewToolbarView (wxCommandEvent &event);
	void OnViewToolbarFile (wxCommandEvent &event);
	void OnViewToolbarFind (wxCommandEvent &event);
	void OnViewToolbarDebug (wxCommandEvent &event);
	void OnViewToolbarEdit (wxCommandEvent &event);
	void OnViewToolbarRun (wxCommandEvent &event);
	void OnViewToolbarMisc (wxCommandEvent &event);
	void OnViewToolbarsConfig (wxCommandEvent &event);
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
//	void OnRunBuild (wxCommandEvent &event);
	void OnRunCompileConfig (wxCommandEvent &event);
	void OnToolsExportMakefile (wxCommandEvent &event);
	void OnRunClean (wxCommandEvent &event);
	void OnRunStop (wxCommandEvent &event);
	
	void OnDebugShowRegisters(wxCommandEvent &event);
	void OnDebugShowAsm(wxCommandEvent &event);
	void OnDebugSendSignal(wxCommandEvent &event);
	void OnDebugSetSignals(wxCommandEvent &event);
	void OnDebugGdbCommand(wxCommandEvent &event);
	void OnDebugAttach (wxCommandEvent &event);
	void OnDebugTarget (wxCommandEvent &event);
	wxString DebugTargetCommon (wxString target, bool should_continue);
	void OnDebugPatch (wxCommandEvent &event);
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
	void OnDebugAutoStep (wxCommandEvent &event);
	void OnDebugUpdateInspections (wxCommandEvent &event);
	void OnDebugInspect (wxCommandEvent &event);
	void OnDebugThreadList (wxCommandEvent &event);
	void OnDebugBacktrace (wxCommandEvent &event);
	void OnDebugEnableDisableBreakpoint (wxCommandEvent &event);
	void OnDebugToggleBreakpoint (wxCommandEvent &event);
	void OnDebugListBreakpoints (wxCommandEvent &event);
	void OnDebugListWatchpoints (wxCommandEvent &event);
	void OnDebugInsertWatchpoint (wxCommandEvent &event);
	void OnDebugBreakpointOptions (wxCommandEvent &event);
	void OnDebugShowLogPanel (wxCommandEvent &event);
	void OnDebugEnableInverseExecution (wxCommandEvent &event);
	void OnDebugInverseExecution (wxCommandEvent &event);
	
	void OnInternalInfo(wxCommandEvent &event);
	
	void OnToolsCreateTemplate (wxCommandEvent &event);
	
	void OnToolsCodePoupup(wxCommandEvent &event);
	void OnToolsCodeCopyFromH (wxCommandEvent &event);
	void OnToolsCodeGenerateFunctionDef (wxCommandEvent &event);
	void OnToolsCodeGenerateFunctionDec (wxCommandEvent &event);
	void OnToolsCodeSurroundIf (wxCommandEvent &event);
	void OnToolsCodeSurroundWhile (wxCommandEvent &event);
	void OnToolsCodeSurroundDo (wxCommandEvent &event);
	void OnToolsCodeSurroundFor (wxCommandEvent &event);
	void OnToolsCodeSurroundIfdef(wxCommandEvent &event);
	void OnToolsCodeExtractFunction(wxCommandEvent &event);
	void OnToolsCodeHelp(wxCommandEvent &event);
	void ToolsCodeCopyFromH (mxSource *source, wxString the_one);
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
	void OnToolsWxfbConfig(wxCommandEvent &event);
	void OnToolsWxfbUpdateInherit(wxCommandEvent &event);
	void OnToolsWxfbInheritClass(wxCommandEvent &event);
	void OnToolsWxfbHelp(wxCommandEvent &event);
	void OnToolsWxfbHelpWx(wxCommandEvent &event);
	void OnToolsGprofSet(wxCommandEvent &event);
	wxString OnToolsGprofShowListAux(bool include_command=false); ///< OnToolsGprofShow and OnToolsGprofList have same common initial code here
	void OnToolsGprofShow(wxCommandEvent &event);
	void OnToolsGprofList(wxCommandEvent &event);
	void OnToolsGprofHelp(wxCommandEvent &event);
	void OnToolsGprofDot(wxCommandEvent &event);
	void OnToolsGprofFdp(wxCommandEvent &event);
	void OnToolsLizardRun(wxCommandEvent &event);
	void OnToolsLizardHelp(wxCommandEvent &event);
	void OnToolsGcovSet(wxCommandEvent &event);
	void OnToolsGcovShow(wxCommandEvent &event);
	void OnToolsGcovReset(wxCommandEvent &event);
	void OnToolsGcovHelp(wxCommandEvent &event);
	void OnToolsCppCheckRun(wxCommandEvent &event);
	void OnToolsCppCheckConfig(wxCommandEvent &event);
	void OnToolsCppCheckView(wxCommandEvent &event);
	void OnToolsCppCheckHelp(wxCommandEvent &event);
#ifndef __WIN32__
	void OnToolsValgrindRun(wxCommandEvent &event);
	void OnToolsValgrindDebug(wxCommandEvent &event);
	void OnToolsValgrindCommon(bool debug);
	void OnToolsValgrindView(wxCommandEvent &event);
	void OnToolsValgrindHelp(wxCommandEvent &event);
#endif
	void AuxToolsDisassemble1(GenericActionEx<wxString> *on_end); ///< ejecuta objdump y retorna el path del archivo de salida
	void AuxToolsDisassemble2(wxString out_fname, bool full_scope); ///< filtra la salida y muestra el panel con los resultados
	void OnToolsDisassembleOfflineSel(wxCommandEvent &event);
	void OnToolsDisassembleOfflineFunc(wxCommandEvent &event);
	void OnToolsDisassembleHelp(wxCommandEvent &event);
	void OnToolsCustomTool(wxCommandEvent &event);
	void OnToolsCustomProjectTool(wxCommandEvent &event);
	void OnToolsCustomToolsSettings(wxCommandEvent &event);
	void OnToolsProjectToolsSettings(wxCommandEvent &event);
	void OnToolsCustomHelp(wxCommandEvent &event);
	
	void OnToolsPreprocMarkValid ( wxCommandEvent &event );
	void OnToolsPreprocUnMarkAll ( wxCommandEvent &event );
	void OnToolsPreprocReplaceMacros ( wxCommandEvent &event );
	void OnToolsPreprocHelp ( wxCommandEvent &event );
	void ToolsPreproc( int id_command );
	
	void OnToolsInstallComplements( wxCommandEvent &event );
	
	void OnHelpFindCommand (wxCommandEvent &event);
	void OnHelpShortcuts (wxCommandEvent &event);
	void OnHelpOpinion (wxCommandEvent &event);
	void OnHelpTutorial (wxCommandEvent &event);
	void OnHelpAbout (wxCommandEvent &event);
	void OnHelpGui (wxCommandEvent &event);
	void OnHelpCpp (wxCommandEvent &event);
	void OnHelpCode (wxCommandEvent &event);
	void OnHelpTip (wxCommandEvent &event);
	void OnHelpUpdates (wxCommandEvent &event);


private:
	SingleList<wxWindow*> m_panes_to_destroy_on_close; // a wxaui pane content is not destroyed when the pane is closed... this will do it
public:
	void AttachPane(wxWindow *ctrl, wxString title, wxPoint position, wxSize size, bool handle_deletion=true);
	void OnPaneClose (wxAuiManagerEvent &event);
	
	void OnClose (wxCloseEvent &event);
	void OnNotebookRightClick(wxAuiNotebookEvent& event);
	void OnNotebookPageClose(wxAuiNotebookEvent& event);
	void OnNotebookPageChanged(wxAuiNotebookEvent& event);
	void OnProcessTerminate (wxProcessEvent &event);
	void OnQuickHelpLink (wxHtmlLinkEvent &event);

	void OnSelectSource (wxTreeEvent &event);
	void OnSelectError (wxTreeEvent &event);
	void OnSelectTreeItem (wxTreeEvent &event);
	void OnSelectExplorerItem (wxTreeEvent &event);
	void OnSymbolsGenerateAutocompletionIndex(wxCommandEvent &evt);
	
	void OnExplorerTreePopup(wxTreeEvent &event);
	void OnExplorerTreeUpdate(wxCommandEvent &evt);
	void OnExplorerTreeChangePath(wxCommandEvent &evy);
	void OnExplorerTreePathUp(wxCommandEvent &evy);
	void OnExplorerTreeOpenOneZinjaI(wxCommandEvent &evt);
	void OnExplorerTreeOpenOneExtern(wxCommandEvent &evt);
	void OnExplorerTreeOpenAll(wxCommandEvent &evt);
	void OnExplorerTreeOpenSources(wxCommandEvent &evt);
	void OnExplorerTreeShowOnlySources(wxCommandEvent &evt);
	void OnExplorerTreeSetAsPath(wxCommandEvent &evt);
	wxString GetExplorerItemPath(wxTreeItemId item);
	
	void OnSymbolTreePopup(wxTreeEvent &event);
	void OnSymbolTreeDec(wxCommandEvent &event);
	void OnSymbolTreeDef(wxCommandEvent &event);
	void OnSymbolTreeIncludes(wxCommandEvent &event);

	void ShowSpecilaUnnamedSource(const wxString &tab_name, const wxArrayString &lines);
	void OnCompilerTreeShowFull(wxCommandEvent &event);
	void OnCompilerTreePopup(wxTreeEvent &event);

	void PopulateProjectFilePopupMenu(wxMenu &menu, project_file_item *fi, bool for_tab);
	void OnProjectTreePopup(wxTreeEvent &event);
	void OnProjectTreeToggleFullPath(wxCommandEvent &event);
	void OnProjectTreeOpenFolder(wxCommandEvent &event);
	void OnProjectTreeProperties(wxCommandEvent &event);
	void OnProjectTreeOpen(wxCommandEvent &event);
	void OnProjectTreeOpenAll(wxCommandEvent &event);
	void AuxCompileOne(project_file_item *item);
	void OnProjectTreeCompilingOpts(wxCommandEvent &event);
	void OnProjectTreeCompileNow(wxCommandEvent &event);
	void OnProjectTreeCompileFirst(wxCommandEvent &event);
	void OnProjectTreeToggleReadOnly(wxCommandEvent &event);
	void OnProjectTreeToggleHideSymbols(wxCommandEvent &event);
	void OnProjectTreeRename(wxCommandEvent &event);
	void OnProjectTreeDelete(wxCommandEvent &event);
	void OnProjectTreeMoveToSources(wxCommandEvent &event);
	void OnProjectTreeMoveToHeaders(wxCommandEvent &event);
	void OnProjectTreeMoveToOthers(wxCommandEvent &event);
	void OnProjectTreeAdd(wxCommandEvent &event);
	void OnProjectTreeAddMultiple(wxCommandEvent &event);
	void OnProjectTreeAddSelected(wxCommandEvent &event);
	
	void OnToolbarSettings(wxCommandEvent &evt); // click on an item from a popup menu launched from some toolbar buttons (currently only compiling options in project mode to select a profile)
	void OnToolbarMenu(wxCommandEvent &evt); // click on an item from a popup menu launched from some toolbar buttons (currently only compiling options in project mode to select a profile)

	void OnWhereAmI(wxCommandEvent &event);
	void SetFocusToSource();
	void OnParseSourceTime(wxTimerEvent &event);
	void OnParseOutputTime(wxTimerEvent &event);
	void OnParserContinueProcess(wxTimerEvent &event);
	
public:
	/**
	* @brief lambda-like action to be run after all events are processed, 
	*        to program delayed actions from within an event
	*
	* This is a mechanism for setting something to run after a current event loop
	* example: if you call something that execute a subprocess or display a new dialog inside a 
	* paint/update event, you may have problems with focus after that (easily seen in ubuntu).
	* So, you must register the action with a derived object from this class and RunAfterEvents 
	* function, and the timer will be used to launch a main window event later (outside 
	* the event that registered the action), that will call all pending actions and will delete them
	**/
	class AfterEventsAction {
		AfterEventsAction *m_next; ///< to make a linked list of actions
		bool m_do_run; ///< should be run? (or was cancelled)
		friend class mxMainWindow;
	protected:
		mxSource *m_source; ///< fuente del cual depende, para no ejecutarla si se cierra ese fuente (do_do en false)
	public:
		virtual void Run()=0;
		AfterEventsAction(mxSource *src=nullptr):m_next(nullptr),m_do_run(true),m_source(src){};
		virtual ~AfterEventsAction(){};
	};
private:
	wxTimer *after_events_timer;
	AfterEventsAction *call_after_events, *current_after_events_action;
public:
	void CallAfterEvents(AfterEventsAction *action);
	void OnAfterEventsTimer(wxTimerEvent &event);
	void SetFocusToSourceAfterEvents();
	
	void OnActivate (wxActivateEvent &event);
	
	void OnSocketEvent(wxSocketEvent &event);
	
	mxStatusBar *status_bar;
	wxStatusBar *OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name);
	void SetStatusBarFields();
	void SetStatusText(wxString text);
	void SetStatusProgress(int prog);

	mxSource *NewFileFromTemplate (wxString filename, bool is_full_path = false);
	mxSource *NewFileFromText (wxString text, int pos=0);
	mxSource *NewFileFromText (wxString text, wxString name, int pos=0);
	mxSource *OpenFile (const wxString &filename);
	mxSource *OpenFile (const wxString &filename, bool add_to_project);
	mxSource *FindSource(wxFileName filename, int *pos=nullptr);
	void OpenFileFromGui (wxFileName filename, int *multiple=nullptr);
	wxTreeItemId AddToProjectTreeSimple(wxFileName filename, eFileType where=FT_NULL);
	wxTreeItemId AddToProjectTreeProject(wxString filename, eFileType where, bool sort=true);
	mxSource *IsOpen (wxFileName filename);
	mxSource *IsOpen (wxTreeItemId tree_item);
	bool CloseSource(int i);
	bool CloseSource(mxSource *src);
	void UpdateInHistory(wxString filename, bool is_project);
	
	void UpdateCustomTools(bool for_project);

	void ShowQuickHelp (wxString keyword, bool hide_compiler_tree=true);
	
	void CompileSource(bool force_compile, GenericAction *action=nullptr);
	void RunSource(mxSource *source);
	void UpdateSymbols();
	
	void ShowInQuickHelpPanel(wxString &res, bool hide_compiler_tree=true); ///< carga el fuente de una pagina desde la cadena res en el panel de ayuda rápida y la muestra
	void LoadInQuickHelpPanel(wxString file, bool hide_compiler_tree=true); ///< carga una pagina desde un archivo en el panel de ayuda rápida y lo muestra
	
private:
//	void SetMenusForDebugging(bool debug_mode);
//	void SetMenusForProject(bool project_mode);
public:
	void OnMenuOpen(wxMenuEvent &evt);
	
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
	void FindAll(const wxString &what);

	//! cambia el titulo de la ventana
	void SetOpenedFileName(wxString name);
	
	void StartExecutionStuff (compile_and_run_struct_single *compile_and_run, wxString msg); 
	
	//! Componentes del árbol del explorador de archivos
	struct explorer_tree_struct {
		bool show_only_sources;
		wxString path;
		wxTreeCtrl *treeCtrl;
//		wxMenuItem *menuItem;
		wxTreeItemId selected_item, root;
	} explorer_tree;
	
	//! Componentes del árbol de proyecto
	struct project_tree_struct {
		wxTreeCtrl *treeCtrl;
//		wxMenuItem *menuItem;
		wxTreeItemId selected_item, selected_parent;
		wxTreeItemId root ,sources, headers, others;
	} project_tree;
	
	//! Componentes del árbol de resultados de la compilación
	struct compiler_tree_struct {
		wxTreeCtrl *treeCtrl;
//		wxMenuItem *menuItem;
		wxTreeItemId root, state, errors, warnings, all;
	} compiler_tree;
	//! Componentes del árbol de resultados de la compilación
	mxExternCompilerOutput *extern_compiler_output;
	//! this sizer will hold both compiler_tree.treeCtrl and extern_compiler_output, but only one could be visible at a given time
	wxPanel *compiler_panel;

	//! Componentes del árbol de simbolos
	struct symbols_tree_struct {
		wxTreeCtrl *treeCtrl;
//		wxMenuItem *menuItem;
	} symbols_tree; 

	wxAuiManager aui_manager;

	wxTimer *parser_timer;

//	wxToolBar *toolbar_file, *toolbar_edit, *toolbar_run, *toolbar_misc, *toolbar_debug, *toolbar_find, *toolbar_status, *toolbar_tools, *toolbar_view, *toolbar_diff, *toolbar_project;
//	wxStaticText *toolbar_status_text;

	wxAuiNotebook *CreateNotebookSources();
	
	wxAuiNotebook *CreateLeftPanels();
	void OnNotebookPanelsChanged(wxAuiNotebookEvent& event);
	
//	wxMenuItem *view_quick_help;

	wxHtmlWindow* quick_help;
	wxHtmlWindow* CreateQuickHelp(wxWindow* parent = nullptr);
	
	void ShowWelcome(bool show=true); ///< alterna entre mostrar el notebook de fuentes y el panel de bienvenida

//	wxTextCtrl* toolbar_find_text;
	
	wxTreeCtrl* CreateSymbolsTree();
	wxTreeCtrl* CreateExplorerTree();
	wxTreeCtrl* CreateProjectTree();
	wxPanel* CreateCompilerTree();
	
	void CreateBeginnersPanel();

	mxThreadGrid *threadlist_ctrl;
	mxBacktraceGrid *backtrace_ctrl;
	mxInspectionsPanel *inspection_ctrl;
	
	mxDiffSideBar *diff_sidebar;
	void ShowDiffSideBar(bool bar, bool map);
	
	mxGCovSideBar *gcov_sidebar;
	void ShowGCovSideBar();
	
	mxSource *GetCurrentSource();
	
	void OnEscapePressed(wxCommandEvent &event);
	

	mxHidenPanel *autohide_handlers[ATH_COUNT];
	void OnResize(wxSizeEvent &evt);
	void ShowQuickHelpPanel(bool hide_compiler_tree=true);
	void ShowCompilerTreePanel();
	void HideCompilerTreePanel();
	void ShowExplorerTreePanel();
	void ShowBeginnersPanel();
	
	void FocusToSource();
	
	///@brief adapt gui elements for the current toolchain (is_extern means showing result textbox instead of compiler tree)
	void SetToolchainMode(bool is_extern);
	
	///@brief set the message in compiler panel and status bar
	void SetCompilingStatus(const wxString &message, bool also_statusbar=true); 
	
	void OnSelectErrorCommon(const wxString &error, bool set_focus_timer=false); ///< to be called from OnExternCompilerOutput y OnSelectError
	
	
	SingleList<mxSource::MacroAction> *m_macro;
	void OnMacroRecord(wxCommandEvent &evt);
	void OnMacroReplay(wxCommandEvent &evt);
	
	void OnNavigationHistoryNext(wxCommandEvent &evt);
	void OnNavigationHistoryPrev(wxCommandEvent &evt);
	
	void UpdateStylesInSources();
	
	void UnregisterSource(mxSource *src);
	
	DECLARE_EVENT_TABLE();
};

extern mxSource *EXTERNAL_SOURCE; // para que devuelva OpenFile cuando al archivo lo abre otro y no ZinjaI

extern mxMainWindow *main_window;

#endif
