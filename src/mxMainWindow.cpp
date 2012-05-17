#include "mxMainWindow.h"

#include <wx/imaglist.h>
#include <wx/artprov.h>
#include <wx/txtstrm.h>
#include <wx/grid.h>
#include <wx/dir.h>
#include <iostream>

#include "egdata.h"
#include "mxFindDialog.h"
#include "paf_defs.h"
#include "mxNewWizard.h"
#include "mxDropTarget.h"
#include "ids.h"
#include "Parser.h"
#include "ConfigManager.h"
#include "DebugManager.h"
#include "HelpManager.h"
#include "ShareManager.h"
#include "ProjectManager.h"
#include "mxUtils.h"
#include "mxSource.h"
#include "mxGotoLineWindow.h"
#include "mxAboutWindow.h"
#include "mxCompileConfigWindow.h"
#include "mxProjectConfigWindow.h"
#include "mxPreferenceWindow.h"
#include "mxTipsWindow.h"
#include "mxHelpWindow.h"
#include "mxStatusBar.h"
#include "mxMakefileDialog.h"
#include "mxArt.h"
#include "mxMessageDialog.h"
#include "mxValgrindOuput.h"
#include "mxGotoFunctionDialog.h"
#include "mxGotoFileDialog.h"
#include "mxInspectionGrid.h"
#include "mxThreadGrid.h"
#include "mxBacktraceGrid.h"
#include "CodeExporter.h"
#include "mxExeInfo.h"
#include "mxBreakList.h"
#include "mxUpdatesChecker.h"
#include "mxBreakOptions.h"
#include "mxArgumentsDialog.h"
#include "error_recovery.h"
#include "mxSplashScreen.h"
#include "mxProjectGeneralConfig.h"
#include "mxPrintOut.h"
#include "mxTreeCtrl.h"
#include "mxWelcomePanel.h"
#include "mxSourceProperties.h"
#include "mxOpenRecentDialog.h"
#include "Language.h"
#include "mxOSD.h"
#include "mxApplication.h"
#include "mxBeginnerPanel.h"
#include "mxDiffSideBar.h"
#include "mxOpinionWindow.h"
#include "mxCustomTools.h"
#include "parserData.h"
#include "mxHidenPanel.h"
#include "mxTextDialog.h"
#include "mxMultipleFileChooser.h"
#include "mxCompiler.h"
#include "Autocoder.h"
#include "mxColoursEditor.h"

#define SIN_TITULO (wxString("<")<<LANG(UNTITLED,"sin_titulo_")<<(++untitled_count)<<">")
#define LAST_TITULO (wxString("<")<<LANG(UNTITLED,"sin_titulo_")<<(untitled_count)<<">")

#ifdef __WIN32__
#include "zinjai-w32.xpm"
#else
#include "zinjai-lnx.xpm"
#endif

#ifdef __WIN32__
#define SameFile(f1,f2) (f1==f2)
#else
#include <sys/stat.h>
inline bool SameFile(wxString f1, wxString f2) {
	struct stat df1; struct stat df2;
	lstat (f1.c_str(), &df1);
	lstat (f2.c_str(), &df2);
	return (df1.st_dev==df2.st_dev && df1.st_ino==df2.st_ino);
}
inline bool SameFile(wxString f1, wxFileName f2) {
	struct stat df1; struct stat df2;
	lstat (f1.c_str(), &df1);
	lstat (f2.GetFullPath().c_str(), &df2);
	return (df1.st_dev==df2.st_dev && df1.st_ino==df2.st_ino);
}
inline bool SameFile(wxFileName f1, wxString f2) {
	struct stat df1; struct stat df2;
	lstat (f1.GetFullPath().c_str(), &df1);
	lstat (f2.c_str(), &df2);
	return (df1.st_dev==df2.st_dev && df1.st_ino==df2.st_ino);
}
inline bool SameFile(wxFileName f1, wxFileName f2) {
	struct stat df1; struct stat df2;
	lstat (f1.GetFullPath().c_str(), &df1);
	lstat (f2.GetFullPath().c_str(), &df2);
	return (df1.st_dev==df2.st_dev && df1.st_ino==df2.st_ino);
}
#endif


mxMainWindow *main_window;
mxSource *external_source;

BEGIN_EVENT_TABLE(mxMainWindow, wxFrame)
	
	EVT_SIZE(mxMainWindow::OnResize)
	
	EVT_TOOL_RCLICKED(wxID_ANY,mxMainWindow::OnToolRightClick)
	
	EVT_MENU(mxID_FILE_PRINT, mxMainWindow::OnFilePrint)
	EVT_MENU(wxID_NEW, mxMainWindow::OnFileNew)
	EVT_MENU(mxID_FILE_PROJECT, mxMainWindow::OnFileNewProject)
	EVT_MENU(wxID_OPEN, mxMainWindow::OnFileOpen)
	EVT_MENU(mxID_FILE_OPEN_SELECTED, mxMainWindow::OnFileOpenSelected)
	EVT_MENU(mxID_FILE_OPEN_H, mxMainWindow::OnFileOpenH)
	EVT_MENU(mxID_FILE_RELOAD, mxMainWindow::OnFileReload)
	EVT_MENU(mxID_FILE_EXPORT_HTML, mxMainWindow::OnFileExportHtml)
	EVT_MENU(wxID_CLOSE, mxMainWindow::OnFileClose)
	EVT_MENU(mxID_FILE_CLOSE_ALL, mxMainWindow::OnFileCloseAll)
	EVT_MENU(mxID_FILE_CLOSE_ALL_BUT_ONE, mxMainWindow::OnFileCloseAllButOne)
	EVT_MENU(mxID_FILE_SAVE_PROJECT, mxMainWindow::OnFileSaveProject)
	EVT_MENU(mxID_FILE_CLOSE_PROJECT, mxMainWindow::OnFileCloseProject)
	EVT_MENU(wxID_SAVE, mxMainWindow::OnFileSave)
	EVT_MENU(wxID_SAVEAS, mxMainWindow::OnFileSaveAs)
	EVT_MENU(mxID_FILE_SAVE_ALL, mxMainWindow::OnFileSaveAll)
	EVT_MENU(wxID_EXIT, mxMainWindow::OnExit)
	EVT_MENU(mxID_FILE_EXPLORE_FOLDER, mxMainWindow::OnFileExploreFolder)
	EVT_MENU(mxID_FILE_OPEN_FOLDER, mxMainWindow::OnFileOpenFolder)
	EVT_MENU(mxID_FILE_PROPERTIES, mxMainWindow::OnFileProperties)
	EVT_MENU(mxID_FILE_PREFERENCES, mxMainWindow::OnPreferences)
	EVT_MENU(mxID_FILE_PROJECT_CONFIG, mxMainWindow::OnFileProjectConfig)
	EVT_MENU(mxID_FILE_SOURCE_HISTORY_MORE, mxMainWindow::OnFileSourceHistoryMore)
	EVT_MENU(mxID_FILE_PROJECT_HISTORY_MORE, mxMainWindow::OnFileProjectHistoryMore)
	EVT_MENU_RANGE(mxID_FILE_SOURCE_HISTORY_0, mxID_FILE_SOURCE_HISTORY_30,mxMainWindow::OnFileSourceHistory)
	EVT_MENU_RANGE(mxID_FILE_PROJECT_HISTORY_0, mxID_FILE_PROJECT_HISTORY_30,mxMainWindow::OnFileProjectHistory)
	
	EVT_MENU(wxID_SELECTALL, mxMainWindow::OnEdit)
	EVT_MENU(wxID_UNDO, mxMainWindow::OnEdit)
	EVT_MENU(wxID_REDO, mxMainWindow::OnEdit)
	EVT_MENU(wxID_COPY, mxMainWindow::OnEditNeedFocus)
	EVT_MENU(wxID_CUT, mxMainWindow::OnEditNeedFocus)
	EVT_MENU(wxID_PASTE, mxMainWindow::OnEditNeedFocus)
	EVT_MENU(mxID_EDIT_TOOLBAR_FIND, mxMainWindow::OnToolbarFindEnter)
	EVT_MENU(mxID_EDIT_FIND_FROM_TOOLBAR, mxMainWindow::OnGotoToolbarFind)
	EVT_MENU(mxID_EDIT_FIND, mxMainWindow::OnEditFind)
	EVT_MENU(mxID_EDIT_FIND_NEXT, mxMainWindow::OnEditFindNext)
	EVT_MENU(mxID_EDIT_FIND_PREV, mxMainWindow::OnEditFindPrev)
	EVT_MENU(mxID_EDIT_REPLACE, mxMainWindow::OnEditReplace)
	EVT_MENU(mxID_EDIT_BRACEMATCH, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_INDENT, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_GOTO, mxMainWindow::OnEditGoto)
	EVT_MENU(mxID_EDIT_GOTO_FUNCTION, mxMainWindow::OnEditGotoFunction)
	EVT_MENU(mxID_EDIT_GOTO_FILE, mxMainWindow::OnEditGotoFile)
	EVT_MENU(mxID_EDIT_COMMENT, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_UNCOMMENT, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_TOGGLE_LINES_UP, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_TOGGLE_LINES_DOWN, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_DUPLICATE_LINES, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_DELETE_LINES, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_MARK_LINES, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_LIST_MARKS, mxMainWindow::OnEditListMarks)
	EVT_MENU(mxID_EDIT_GOTO_MARK, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_INSERT_HEADER, mxMainWindow::OnEditInsertInclude)
	EVT_MENU(mxID_EDIT_AUTOCODE_AUTOCOMPLETE, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_FORCE_AUTOCOMPLETE, mxMainWindow::OnEdit)
//	EVT_MENU(mxID_EDIT_FUZZY_AUTOCOMPLETE, mxMainWindow::OnEdit)
	
	EVT_MENU(mxID_RUN_RUN, mxMainWindow::OnRunRun)
	EVT_MENU(mxID_RUN_RUN_OLD, mxMainWindow::OnRunRunOld)
	EVT_MENU(mxID_RUN_STOP, mxMainWindow::OnRunStop)
	EVT_MENU(mxID_RUN_BUILD, mxMainWindow::OnRunBuild)
	EVT_MENU(mxID_RUN_COMPILE, mxMainWindow::OnRunCompile)
	EVT_MENU(mxID_RUN_CLEAN, mxMainWindow::OnRunClean)
	EVT_MENU(mxID_RUN_MAKEFILE, mxMainWindow::OnRunExportMakefile)
	EVT_MENU(mxID_RUN_CONFIG, mxMainWindow::OnRunCompileConfig)
	
	EVT_MENU(mxID_DEBUG_ATTACH, mxMainWindow::OnDebugAttach)
	EVT_MENU(mxID_DEBUG_CORE_DUMP, mxMainWindow::OnDebugCoreDump)
	EVT_MENU(mxID_DEBUG_THREADLIST, mxMainWindow::OnDebugThreadList)
	EVT_MENU(mxID_DEBUG_BACKTRACE, mxMainWindow::OnDebugBacktrace)
	EVT_MENU(mxID_DEBUG_INSPECT, mxMainWindow::OnDebugInspect)
	EVT_MENU(mxID_DEBUG_STOP, mxMainWindow::OnDebugStop)
	EVT_MENU(mxID_DEBUG_PAUSE, mxMainWindow::OnDebugPause)
	EVT_MENU(mxID_DEBUG_RUN, mxMainWindow::OnDebugRun)
	EVT_MENU(mxID_DEBUG_STEP_IN, mxMainWindow::OnDebugStepIn)
	EVT_MENU(mxID_DEBUG_STEP_OUT, mxMainWindow::OnDebugStepOut)
	EVT_MENU(mxID_DEBUG_STEP_OVER, mxMainWindow::OnDebugStepOver)
	EVT_MENU(mxID_DEBUG_FUNCTION_BREAKPOINT, mxMainWindow::OnDebugFunctionBreakpoint)
	EVT_MENU(mxID_DEBUG_BREAKPOINT_OPTIONS, mxMainWindow::OnDebugBreakpointOptions)
	EVT_MENU(mxID_DEBUG_INSERT_WATCHPOINT, mxMainWindow::OnDebugInsertWatchpoint)
	EVT_MENU(mxID_DEBUG_LIST_BREAKPOINTS, mxMainWindow::OnDebugListBreakpoints)
//	EVT_MENU(mxID_DEBUG_LIST_WATCHPOINTS, mxMainWindow::OnDebugListWatchpoints)
	EVT_MENU(mxID_DEBUG_TOGGLE_BREAKPOINT, mxMainWindow::OnDebugToggleBreakpoint)
	EVT_MENU(mxID_DEBUG_RETURN, mxMainWindow::OnDebugReturn)
	EVT_MENU(mxID_DEBUG_JUMP, mxMainWindow::OnDebugJump)
	EVT_MENU(mxID_DEBUG_RUN_UNTIL, mxMainWindow::OnDebugRunUntil)
	EVT_MENU(mxID_DEBUG_DO_THAT, mxMainWindow::OnDebugDoThat)
	EVT_MENU(mxID_DEBUG_ENABLE_INVERSE_EXEC, mxMainWindow::OnDebugEnableInverseExecution)
	EVT_MENU(mxID_DEBUG_INVERSE_EXEC, mxMainWindow::OnDebugInverseExecution)
	EVT_MENU(mxID_DEBUG_LOG_PANEL, mxMainWindow::OnDebugShowLogPanel)

	EVT_MENU(mxID_INTERNAL_INFO, mxMainWindow::OnInternalInfo)
	
	EVT_MENU(mxID_VIEW_HIDE_SOMETHING, mxMainWindow::OnEscapePressed)
	EVT_MENU(mxID_VIEW_DUPLICATE_TAB, mxMainWindow::OnViewDuplicateTab)
	EVT_MENU(mxID_VIEW_BEGINNER_PANEL, mxMainWindow::OnViewBeginnerPanel)
	EVT_MENU(mxID_VIEW_NEXT_ERROR, mxMainWindow::OnViewNextError)
	EVT_MENU(mxID_VIEW_PREV_ERROR, mxMainWindow::OnViewPrevError)
	EVT_MENU(mxID_VIEW_LEFT_PANELS, mxMainWindow::OnViewLeftPanels)
	EVT_MENU(mxID_VIEW_PROJECT_TREE, mxMainWindow::OnViewProjectTree)
	EVT_MENU(mxID_VIEW_COMPILER_TREE, mxMainWindow::OnViewCompilerTree)
	EVT_MENU(mxID_VIEW_EXPLORER_TREE, mxMainWindow::OnViewExplorerTree)
	EVT_MENU(mxID_VIEW_SYMBOLS_TREE, mxMainWindow::OnViewSymbolsTree)
	EVT_MENU(mxID_VIEW_UPDATE_SYMBOLS, mxMainWindow::OnViewUpdateSymbols)
	EVT_MENU(mxID_VIEW_TOOLBAR_MISC, mxMainWindow::OnViewToolbarMisc)
	EVT_MENU(mxID_VIEW_TOOLBAR_TOOLS, mxMainWindow::OnViewToolbarTools)
	EVT_MENU(mxID_VIEW_TOOLBAR_VIEW, mxMainWindow::OnViewToolbarView)
	EVT_MENU(mxID_VIEW_TOOLBAR_FILE, mxMainWindow::OnViewToolbarFile)
	EVT_MENU(mxID_VIEW_TOOLBAR_EDIT, mxMainWindow::OnViewToolbarEdit)
	EVT_MENU(mxID_VIEW_TOOLBAR_DEBUG, mxMainWindow::OnViewToolbarDebug)
	EVT_MENU(mxID_VIEW_TOOLBAR_RUN, mxMainWindow::OnViewToolbarRun)
	EVT_MENU(mxID_VIEW_TOOLBAR_FIND, mxMainWindow::OnViewToolbarFind)
	EVT_MENU(mxID_VIEW_WHITE_SPACE, mxMainWindow::OnViewWhiteSpace)
	EVT_MENU(mxID_VIEW_LINE_WRAP, mxMainWindow::OnViewLineWrap)
	EVT_MENU(mxID_VIEW_CODE_STYLE, mxMainWindow::OnViewCodeStyle)
	EVT_MENU(mxID_VIEW_NOTEBOOK_NEXT, mxMainWindow::OnViewNotebookNext)
	EVT_MENU(mxID_VIEW_NOTEBOOK_PREV, mxMainWindow::OnViewNotebookPrev)
	EVT_MENU(mxID_VIEW_HIDE_BOTTOM, mxMainWindow::OnViewHideBottom)
	EVT_MENU(mxID_VIEW_FULLSCREEN, mxMainWindow::OnViewFullScreen)

	EVT_MENU(mxID_FOLD_SHOW_1, mxMainWindow::OnFoldShow1)
	EVT_MENU(mxID_FOLD_SHOW_2, mxMainWindow::OnFoldShow2)
	EVT_MENU(mxID_FOLD_SHOW_3, mxMainWindow::OnFoldShow3)
	EVT_MENU(mxID_FOLD_SHOW_4, mxMainWindow::OnFoldShow4)
	EVT_MENU(mxID_FOLD_SHOW_5, mxMainWindow::OnFoldShow5)
	EVT_MENU(mxID_FOLD_SHOW_ALL, mxMainWindow::OnFoldShowAll)
	EVT_MENU(mxID_FOLD_HIDE_1, mxMainWindow::OnFoldHide1)
	EVT_MENU(mxID_FOLD_HIDE_2, mxMainWindow::OnFoldHide2)
	EVT_MENU(mxID_FOLD_HIDE_3, mxMainWindow::OnFoldHide3)
	EVT_MENU(mxID_FOLD_HIDE_4, mxMainWindow::OnFoldHide4)
	EVT_MENU(mxID_FOLD_HIDE_5, mxMainWindow::OnFoldHide5)
	EVT_MENU(mxID_FOLD_HIDE_ALL, mxMainWindow::OnFoldHideAll)
	EVT_MENU(mxID_FOLD_FOLD, mxMainWindow::OnFoldFold)
	EVT_MENU(mxID_FOLD_UNFOLD, mxMainWindow::OnFoldUnFold)
	
	EVT_MENU(mxID_TOOLS_PREPROC_UNMARK_ALL, mxMainWindow::OnToolsPreprocUnMarkAll)
	EVT_MENU(mxID_TOOLS_PREPROC_MARK_VALID, mxMainWindow::OnToolsPreprocMarkValid)
	EVT_MENU(mxID_TOOLS_PREPROC_EXPAND_MACROS, mxMainWindow::OnToolsPreprocReplaceMacros)
	EVT_MENU(mxID_TOOLS_PREPROC_HELP, mxMainWindow::OnToolsPreprocHelp)
	EVT_MENU(mxID_TOOLS_CODE_COPY_FROM_H, mxMainWindow::OnToolsCodeCopyFromH)
	EVT_MENU(mxID_TOOLS_CONSOLE, mxMainWindow::OnToolsConsole)
	EVT_MENU(mxID_TOOLS_DRAW_PROJECT, mxMainWindow::OnToolsDrawProject)
	EVT_MENU(mxID_TOOLS_DRAW_CLASSES, mxMainWindow::OnToolsDrawClasses)
	EVT_MENU(mxID_TOOLS_DRAW_FLOW, mxMainWindow::OnToolsDrawFlow)
	EVT_MENU(mxID_TOOLS_PROJECT_STATISTICS, mxMainWindow::OnToolsProjectStatistics)
	EVT_MENU(mxID_TOOLS_EXE_PROPS, mxMainWindow::OnToolsExeProps)
	EVT_MENU(mxID_TOOLS_SHARE_SHARE, mxMainWindow::OnToolsShareShare)
	EVT_MENU(mxID_TOOLS_SHARE_OPEN, mxMainWindow::OnToolsShareOpen)
	EVT_MENU(mxID_TOOLS_SHARE_LIST, mxMainWindow::OnToolsShareList)
	EVT_MENU(mxID_TOOLS_SHARE_HELP, mxMainWindow::OnToolsShareHelp)
	EVT_MENU(mxID_TOOLS_DOXY_HELP, mxMainWindow::OnToolsDoxyHelp)
	EVT_MENU(mxID_TOOLS_DOXY_CONFIG, mxMainWindow::OnToolsDoxyConfig)
	EVT_MENU(mxID_TOOLS_DOXY_GENERATE, mxMainWindow::OnToolsDoxyGenerate)
	EVT_MENU(mxID_TOOLS_DOXY_VIEW, mxMainWindow::OnToolsDoxyView)
	EVT_MENU(mxID_TOOLS_WXFB_UPDATE_INHERIT, mxMainWindow::OnToolsWxfbUpdateInherit)
	EVT_MENU(mxID_TOOLS_WXFB_INHERIT_CLASS, mxMainWindow::OnToolsWxfbInheritClass)
	EVT_MENU(mxID_TOOLS_WXFB_AUTO, mxMainWindow::OnToolsWxfbAuto)
	EVT_MENU(mxID_TOOLS_WXFB_ACTIVATE, mxMainWindow::OnToolsWxfbActivate)
	EVT_MENU(mxID_TOOLS_WXFB_NEW_RES, mxMainWindow::OnToolsWxfbNewRes)
	EVT_MENU(mxID_TOOLS_WXFB_LOAD_RES, mxMainWindow::OnToolsWxfbLoadRes)
	EVT_MENU(mxID_TOOLS_WXFB_REGEN, mxMainWindow::OnToolsWxfbRegen)
	EVT_MENU(mxID_TOOLS_WXFB_HELP, mxMainWindow::OnToolsWxfbHelp)
	EVT_MENU(mxID_TOOLS_WXFB_HELP_WX, mxMainWindow::OnToolsWxfbHelpWx)
	EVT_MENU(mxID_TOOLS_DIFF_NEXT, mxMainWindow::OnToolsDiffNextMark)
	EVT_MENU(mxID_TOOLS_DIFF_PREV, mxMainWindow::OnToolsDiffPrevMark)
	EVT_MENU(mxID_TOOLS_DIFF_TWO, mxMainWindow::OnToolsDiffTwoSources)
	EVT_MENU(mxID_TOOLS_DIFF_DISK, mxMainWindow::OnToolsDiffToDiskFile)
	EVT_MENU(mxID_TOOLS_DIFF_HIMSELF, mxMainWindow::OnToolsDiffToHimself)
	EVT_MENU(mxID_TOOLS_DIFF_CLEAR, mxMainWindow::OnToolsDiffClear)
	EVT_MENU(mxID_TOOLS_DIFF_SHOW, mxMainWindow::OnToolsDiffShow)
	EVT_MENU(mxID_TOOLS_DIFF_APPLY, mxMainWindow::OnToolsDiffApply)
	EVT_MENU(mxID_TOOLS_DIFF_DISCARD, mxMainWindow::OnToolsDiffDiscard)
	EVT_MENU(mxID_TOOLS_DIFF_HELP, mxMainWindow::OnToolsDiffHelp)
	EVT_MENU(mxID_TOOLS_GPROF_SHOW, mxMainWindow::OnToolsGprofShow)
	EVT_MENU(mxID_TOOLS_GPROF_LIST, mxMainWindow::OnToolsGprofList)
	EVT_MENU(mxID_TOOLS_GPROF_SET, mxMainWindow::OnToolsGprofSet)
	EVT_MENU(mxID_TOOLS_GPROF_HELP, mxMainWindow::OnToolsGprofHelp)
	EVT_MENU(mxID_TOOLS_GPROF_FDP, mxMainWindow::OnToolsGprofFdp)
	EVT_MENU(mxID_TOOLS_GPROF_DOT, mxMainWindow::OnToolsGprofDot)
	EVT_MENU(mxID_TOOLS_ALIGN_COMMENTS, mxMainWindow::OnToolsAlignComments)
	EVT_MENU(mxID_TOOLS_REMOVE_COMMENTS, mxMainWindow::OnToolsRemoveComments)
	EVT_MENU(mxID_TOOLS_CPPCHECK_RUN, mxMainWindow::OnToolsCppCheckRun)
	EVT_MENU(mxID_TOOLS_CPPCHECK_CONFIG, mxMainWindow::OnToolsCppCheckConfig)
	EVT_MENU(mxID_TOOLS_CPPCHECK_VIEW, mxMainWindow::OnToolsCppCheckView)
	EVT_MENU(mxID_TOOLS_CPPCHECK_HELP, mxMainWindow::OnToolsCppCheckHelp)
#if !defined(_WIN32) && !defined(__WIN32__)
	EVT_MENU(mxID_TOOLS_VALGRIND_RUN, mxMainWindow::OnToolsValgrindRun)
	EVT_MENU(mxID_TOOLS_VALGRIND_VIEW, mxMainWindow::OnToolsValgrindView)
	EVT_MENU(mxID_TOOLS_VALGRIND_HELP, mxMainWindow::OnToolsValgrindHelp)
#endif
	EVT_MENU_RANGE(mxID_CUSTOM_TOOL_0, mxID_CUSTOM_TOOL_9,mxMainWindow::OnToolsCustomTool)
	EVT_MENU(mxID_TOOLS_CUSTOM_SETTINGS, mxMainWindow::OnToolsCustomSettings)
	EVT_MENU(mxID_TOOLS_CUSTOM_HELP, mxMainWindow::OnToolsCustomHelp)
	EVT_MENU(mxID_TOOLS_INSTALL_COMPLEMENTS, mxMainWindow::OnToolsInstallComplements)
	
	EVT_MENU(mxID_HELP_OPINION, mxMainWindow::OnHelpOpinion)
	EVT_MENU(mxID_HELP_TUTORIAL, mxMainWindow::OnHelpTutorial)
	EVT_MENU(mxID_HELP_ABOUT, mxMainWindow::OnHelpAbout)
	EVT_MENU(mxID_HELP_GUI, mxMainWindow::OnHelpGui)
	EVT_MENU(mxID_HELP_CPP, mxMainWindow::OnHelpCpp)
	EVT_MENU(mxID_HELP_TIP, mxMainWindow::OnHelpTip)
	EVT_MENU(mxID_HELP_UPDATES, mxMainWindow::OnHelpUpdates)
		
	EVT_AUI_PANE_CLOSE(mxMainWindow::OnPaneClose)
	EVT_AUINOTEBOOK_PAGE_CLOSE(mxID_NOTEBOOK_SOURCES, mxMainWindow::OnNotebookPageClose)
	EVT_AUINOTEBOOK_PAGE_CHANGED(mxID_NOTEBOOK_SOURCES, mxMainWindow::OnNotebookPageChanged)
	EVT_AUINOTEBOOK_TAB_RIGHT_DOWN(mxID_NOTEBOOK_SOURCES, mxMainWindow::OnNotebookRightClick)
	EVT_AUINOTEBOOK_PAGE_CHANGED(mxID_LEFT_PANELS, mxMainWindow::OnNotebookPanelsChanged)
	
	EVT_END_PROCESS(wxID_ANY, mxMainWindow::OnProcessTerminate)
	
	EVT_TREE_ITEM_ACTIVATED(wxID_ANY, mxMainWindow::OnSelectTreeItem)
	
	EVT_HTML_LINK_CLICKED(wxID_ANY, mxMainWindow::OnQuickHelpLink)
	
	EVT_CLOSE(mxMainWindow::OnClose)
	
	EVT_TREE_ITEM_RIGHT_CLICK(mxID_TREE_SYMBOLS, mxMainWindow::OnSymbolTreePopup)
	EVT_MENU(mxID_SYMBOL_POPUP_DEC, mxMainWindow::OnSymbolTreeDec)
	EVT_MENU(mxID_SYMBOL_POPUP_DEF, mxMainWindow::OnSymbolTreeDef)
	EVT_MENU(mxID_SYMBOL_POPUP_INCLUDES, mxMainWindow::OnSymbolTreeIncludes)
	EVT_MENU(mxID_SYMBOL_GENERATE_CACHE, mxMainWindow::OnSymbolsGenerateCache)
	
	EVT_TREE_ITEM_RIGHT_CLICK(mxID_TREE_EXPLORER, mxMainWindow::OnExplorerTreePopup)
	EVT_MENU(mxID_EXPLORER_POPUP_UPDATE, mxMainWindow::OnExplorerTreeUpdate)
	EVT_MENU(mxID_EXPLORER_POPUP_CHANGE_PATH, mxMainWindow::OnExplorerTreeChangePath)
	EVT_MENU(mxID_EXPLORER_POPUP_PATH_UP, mxMainWindow::OnExplorerTreePathUp)
	EVT_MENU(mxID_EXPLORER_POPUP_OPEN_ONE, mxMainWindow::OnExplorerTreeOpenOne)
	EVT_MENU(mxID_EXPLORER_POPUP_OPEN_ALL, mxMainWindow::OnExplorerTreeOpenAll)
	EVT_MENU(mxID_EXPLORER_POPUP_OPEN_SOURCES, mxMainWindow::OnExplorerTreeOpenSources)
	EVT_MENU(mxID_EXPLORER_POPUP_SHOW_ONLY_SOURCES, mxMainWindow::OnExplorerTreeShowOnlySources)
	EVT_MENU(mxID_EXPLORER_POPUP_SET_AS_PATH, mxMainWindow::OnExplorerTreeSetAsPath)
	
	EVT_TREE_ITEM_RIGHT_CLICK(mxID_TREE_PROJECT, mxMainWindow::OnProjectTreePopup)
	EVT_TREE_ITEM_RIGHT_CLICK(mxID_TREE_COMPILER, mxMainWindow::OnCompilerTreePopup)
	EVT_MENU(mxID_COMPILER_POPUP_FULL, mxMainWindow::OnCompilerTreeShowFull)
	
	EVT_MENU(mxID_PROJECT_POPUP_OPEN_FOLDER, mxMainWindow::OnProjectTreeOpenFolder)
	EVT_MENU(mxID_PROJECT_POPUP_PROPERTIES, mxMainWindow::OnProjectTreeProperties)
	EVT_MENU(mxID_PROJECT_POPUP_OPEN, mxMainWindow::OnProjectTreeOpen)
	EVT_MENU(mxID_PROJECT_POPUP_OPEN_ALL, mxMainWindow::OnProjectTreeOpenAll)
	EVT_MENU(mxID_PROJECT_POPUP_COMPILE_NOW, mxMainWindow::OnProjectTreeCompileNow)
	EVT_MENU(mxID_PROJECT_POPUP_COMPILE_FIRST, mxMainWindow::OnProjectTreeCompileFirst)
	EVT_MENU(mxID_PROJECT_POPUP_RENAME, mxMainWindow::OnProjectTreeRename)
	EVT_MENU(mxID_PROJECT_POPUP_DELETE, mxMainWindow::OnProjectTreeDelete)
	EVT_MENU(mxID_PROJECT_POPUP_MOVE_TO_SOURCES, mxMainWindow::OnProjectTreeMoveToSources)
	EVT_MENU(mxID_PROJECT_POPUP_MOVE_TO_HEADERS, mxMainWindow::OnProjectTreeMoveToHeaders)
	EVT_MENU(mxID_PROJECT_POPUP_MOVE_TO_OTHERS, mxMainWindow::OnProjectTreeMoveToOthers)
	EVT_MENU(mxID_PROJECT_POPUP_ADD, mxMainWindow::OnProjectTreeAdd)
	EVT_MENU(mxID_PROJECT_POPUP_ADD_MULTI, mxMainWindow::OnProjectTreeAddMultiple)
	EVT_MENU(mxID_PROJECT_POPUP_TOGGLE_FULLPATH, mxMainWindow::OnProjectTreeToggleFullPath)
	EVT_MENU(mxID_PROJECT_POPUP_ADD_SELECTED, mxMainWindow::OnProjectTreeAddSelected)
	
	EVT_SOCKET(wxID_ANY,mxMainWindow::OnSocketEvent)
	
//	EVT_TIMER(mxID_WHERE_TIMER, mxMainWindow::OnWhereTime)
	EVT_MENU(mxID_WHERE_TIMER, mxMainWindow::OnWhereAmI)
	EVT_TIMER(mxID_FOCUS_TIMER, mxMainWindow::OnFocusTime)
	EVT_TIMER(mxID_COMPILER_TIMER, mxMainWindow::OnParseOutputTime)
	EVT_TIMER(mxID_PARSER_TIMER, mxMainWindow::OnParseSourceTime)

	EVT_TEXT(mxID_TOOLBAR_FIND, mxMainWindow::OnToolbarFindChange)
	EVT_TEXT_ENTER(mxID_TOOLBAR_FIND, mxMainWindow::OnToolbarFindEnter)
	
//	EVT_KEY_DOWN(mxMainWindow::OnKey)
//	EVT_CHAR_HOOK(mxMainWindow::OnKey)
	EVT_ACTIVATE (mxMainWindow::OnActivate)
	
END_EVENT_TABLE()



mxMainWindow::mxMainWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style) {
	
	external_source=(mxSource*)this;

	SHOW_MILLIS("Creating main window...");
	
	SetAccelerators();
	
	toolbar_first_time=true;
	untitled_count=0;
	preference_window=NULL;
	valgrind_panel=NULL; 
	beginner_panel=NULL;
	diff_sidebar=NULL;
	for (int i=0;i<ATH_COUNT;i++)
		autohide_handlers[i]=NULL;
	
	SetIcon(wxIcon(zinjai_xpm));
	
 	aui_manager.SetManagedWindow(this);
		
	toolbar_tools=NULL;
	SHOW_MILLIS("Creating menues...");
	CreateMenus();
	SHOW_MILLIS("Creating toolbars...");
	CreateToolbars();
	CreateStatusBar(1,0);
	status_bar->SetStatusText(LANG(MAINW_INITIALIZING,"Inicializando..."));
	SHOW_MILLIS("Setting panes v1...");
	if (config->Init.left_panels && !config->Init.autohiding_panels) {
		aui_manager.AddPane(CreateLeftPanels(), wxAuiPaneInfo().Name("left_panels").Left().CloseButton(true).MaximizeButton(true).Caption(_T("Arboles")).Hide());
		left_panels->AddPage(CreateProjectTree(),"P");
		left_panels->AddPage(CreateSymbolsTree(),"S");
		left_panels->AddPage(CreateExplorerTree(),"E");
		left_panels->SetSelection(1);
		SetExplorerPath(config->Files.last_dir);
	} else {
		left_panels=NULL;
		aui_manager.AddPane(CreateExplorerTree(), wxAuiPaneInfo().Name("explorer_tree").Caption(LANG(CAPTION_EXPLORER_TREE,"Explorador de Archivos")).Left().CloseButton(true).MaximizeButton(true).Hide().Position(0).MaximizeButton(!config->Init.autohiding_panels));
		aui_manager.AddPane(CreateProjectTree(), wxAuiPaneInfo().Name("project_tree").Caption(LANG(CAPTION_PROJECT_TREE,"Arbol de Archivos")).Left().CloseButton(true).MaximizeButton(true).Hide().Position(1).MaximizeButton(!config->Init.autohiding_panels));
		aui_manager.AddPane(CreateSymbolsTree(), wxAuiPaneInfo().Name("symbols_tree").Caption(LANG(CAPTION_SYMBOLS_TREE,"Arbol de Simbolos")).Left().CloseButton(true).MaximizeButton(true).Hide().Position(2).MaximizeButton(!config->Init.autohiding_panels));
		if (config->Init.autohiding_panels) SetExplorerPath(config->Files.last_dir);
	}
	SHOW_MILLIS("Setting panes v2...");
	aui_manager.AddPane(CreateCompilerTree(), wxAuiPaneInfo().Name("compiler_tree").Bottom().Caption(LANG(CAPTION_COMPILER_OUTPUT,"Resultados de la Compilacion")).CloseButton(true).MaximizeButton(true).Hide().MaximizeButton(!config->Init.autohiding_panels));
	aui_manager.AddPane(CreateQuickHelp(), wxAuiPaneInfo().Name("quick_help").Bottom().Caption(LANG(CAPTION_QUIKHELP,"Ayuda Rapida")).CloseButton(true).MaximizeButton(true).Hide().MaximizeButton(!config->Init.autohiding_panels));
	if (config->Debug.inspections_on_right)
		aui_manager.AddPane((wxGrid*)(inspection_ctrl = new mxInspectionGrid(this,wxID_ANY)), wxAuiPaneInfo().Name("inspection").Caption(LANG(CAPTION_INSPECTIONS,"Inspecciones")).Right().CloseButton(true).MaximizeButton(true).Hide().Position(0).MaximizeButton(!config->Init.autohiding_panels));
	else
		aui_manager.AddPane((wxGrid*)(inspection_ctrl = new mxInspectionGrid(this,wxID_ANY)), wxAuiPaneInfo().Name("inspection").Caption(LANG(CAPTION_INSPECTIONS,"Inspecciones")).Bottom().CloseButton(true).MaximizeButton(true).Hide().Position(2).MaximizeButton(!config->Init.autohiding_panels));
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_INSPECTIONS] = new mxHidenPanel(this,inspection_ctrl,config->Debug.inspections_on_right?2:1,LANG(MAINW_AUTOHIDE_INSPECTIONS,"Inspecciones"));
	aui_manager.AddPane((wxGrid*)(backtrace_ctrl = new mxBacktraceGrid(this,wxID_ANY)), wxAuiPaneInfo().Name("backtrace").Caption(LANG(CAPTION_BACKTRACE,"Trazado Inverso")).Bottom().CloseButton(true).MaximizeButton(true).Hide().Position(1).MaximizeButton(!config->Init.autohiding_panels));
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_BACKTRACE] = new mxHidenPanel(this,backtrace_ctrl,1,LANG(MAINW_AUTOHIDE_BACKTRACE,"Trazado Inverso"));
	aui_manager.AddPane((wxGrid*)(threadlist_ctrl = new mxThreadGrid(this,wxID_ANY)), wxAuiPaneInfo().Name("threadlist").Caption(LANG(CAPTION_THREADLIST,"Hilos de Ejecucion")).Bottom().CloseButton(true).MaximizeButton(true).Hide().Position(0).MaximizeButton(!config->Init.autohiding_panels));
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_THREADS] = new mxHidenPanel(this,threadlist_ctrl,1,LANG(MAINW_AUTOHIDE_THREADS,"Hilos"));
	aui_manager.AddPane(CreateNotebookSources(), wxAuiPaneInfo().Name("notebook_sources").CenterPane().PaneBorder(false));
	aui_manager.AddPane(debug_log_panel=new wxListBox(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,NULL,wxLB_HSCROLL),wxAuiPaneInfo().Name("quick_help").Bottom().Caption(LANG(CAPTION_DEBUGGER_LOG,"Mensajes del Depurador")).CloseButton(true).MaximizeButton(true).Hide().MaximizeButton(!config->Init.autohiding_panels));
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_DEBUG_LOG] = new mxHidenPanel(this,debug_log_panel,1,LANG(MAINW_AUTOHIDE_DEBUG_LOG,"Log Depurador"));
	
	if (config->Init.autohiding_panels) {
		aui_manager.AddPane(autohide_handlers[ATH_COMPILER], wxAuiPaneInfo().CaptionVisible(false).Bottom().Position(1).Show());
		aui_manager.AddPane(autohide_handlers[ATH_PROJECT], wxAuiPaneInfo().CaptionVisible(false).Left().Position(1).Show());
		aui_manager.AddPane(autohide_handlers[ATH_SYMBOL], wxAuiPaneInfo().CaptionVisible(false).Left().Position(2).Show());
		aui_manager.AddPane(autohide_handlers[ATH_EXPLORER], wxAuiPaneInfo().CaptionVisible(false).Left().Position(3).Show());
		
		aui_manager.AddPane(autohide_handlers[ATH_QUICKHELP], wxAuiPaneInfo().CaptionVisible(false).Bottom().Position(2).Show());
		if (config->Debug.show_log_panel)
			aui_manager.AddPane(autohide_handlers[ATH_DEBUG_LOG], wxAuiPaneInfo().CaptionVisible(false).Bottom().Position(3).Show());
		else
			aui_manager.AddPane(autohide_handlers[ATH_DEBUG_LOG], wxAuiPaneInfo().CaptionVisible(false).Bottom().Position(3).Hide());
		if (config->Debug.show_thread_panel)
			aui_manager.AddPane(autohide_handlers[ATH_THREADS], wxAuiPaneInfo().CaptionVisible(false).Bottom().Position(4).Show());
		else
			aui_manager.AddPane(autohide_handlers[ATH_THREADS], wxAuiPaneInfo().CaptionVisible(false).Bottom().Position(4).Hide());
		aui_manager.AddPane(autohide_handlers[ATH_BACKTRACE], wxAuiPaneInfo().CaptionVisible(false).Bottom().Position(5).Show());
		if (config->Debug.inspections_on_right)
			aui_manager.AddPane(autohide_handlers[ATH_INSPECTIONS], wxAuiPaneInfo().CaptionVisible(false).Right().Position(1).Show());
		else
			aui_manager.AddPane(autohide_handlers[ATH_INSPECTIONS], wxAuiPaneInfo().CaptionVisible(false).Bottom().Position(6).Show());
		
	}
		
	SHOW_MILLIS("Setting panes v3...");
	if (config->Toolbars.wich_ones.file) 
		aui_manager.AddPane(toolbar_file, wxAuiPaneInfo().Name("toolbar_file").Caption(LANG(CAPTION_TOOLBAR_FILE,"Archivo")).ToolbarPane().Top().Position(0).Row(config->Toolbars.positions.row_file).LeftDockable(false).RightDockable(false));
	else
		aui_manager.AddPane(toolbar_file, wxAuiPaneInfo().Name("toolbar_file").Caption(LANG(CAPTION_TOOLBAR_FILE,"Archivo")).ToolbarPane().Top().Position(0).Row(config->Toolbars.positions.row_file).LeftDockable(false).RightDockable(false).Hide());
	
	if (config->Toolbars.wich_ones.edit) 
		aui_manager.AddPane(toolbar_edit, wxAuiPaneInfo().Name("toolbar_edit").Caption(LANG(CAPTION_TOOLBAR_EDIT,"Edicion")).ToolbarPane().Top().Position(1).Row(config->Toolbars.positions.row_edit).LeftDockable(false).RightDockable(false));
	else
		aui_manager.AddPane(toolbar_edit, wxAuiPaneInfo().Name("toolbar_edit").Caption(LANG(CAPTION_TOOLBAR_EDIT,"Edicion")).ToolbarPane().Top().Position(1).Row(config->Toolbars.positions.row_edit).LeftDockable(false).RightDockable(false).Hide());
	
	if (config->Toolbars.wich_ones.view) 
		aui_manager.AddPane(toolbar_view, wxAuiPaneInfo().Name("toolbar_view").Caption(LANG(CAPTION_TOOLBAR_VIEW,"Ver")).ToolbarPane().Top().Position(2).Row(config->Toolbars.positions.row_view).LeftDockable(false).RightDockable(false));
	else
		aui_manager.AddPane(toolbar_view, wxAuiPaneInfo().Name("toolbar_view").Caption(LANG(CAPTION_TOOLBAR_VIEW,"Ver")).ToolbarPane().Top().Position(2).Row(config->Toolbars.positions.row_view).LeftDockable(false).RightDockable(false).Hide());
	
	if (config->Toolbars.wich_ones.tools) 
		aui_manager.AddPane(toolbar_tools, wxAuiPaneInfo().Name("toolbar_tools").Caption(LANG(CAPTION_TOOLBAR_TOOLS,"Herramientas")).ToolbarPane().Top().Position(3).Row(config->Toolbars.positions.row_tools).LeftDockable(false).RightDockable(false));
	else
		aui_manager.AddPane(toolbar_tools, wxAuiPaneInfo().Name("toolbar_tools").Caption(LANG(CAPTION_TOOLBAR_TOOLS,"Herramientas")).ToolbarPane().Top().Position(3).Row(config->Toolbars.positions.row_tools).LeftDockable(false).RightDockable(false).Hide());

	if (config->Toolbars.wich_ones.run) 
		aui_manager.AddPane(toolbar_run, wxAuiPaneInfo().Name("toolbar_run").Caption(LANG(CAPTION_TOOLBAR_RUN,"Ejecucion")).ToolbarPane().Top().Position(4).Row(config->Toolbars.positions.row_run).LeftDockable(false).RightDockable(false));
	else
		aui_manager.AddPane(toolbar_run, wxAuiPaneInfo().Name("toolbar_run").Caption(LANG(CAPTION_TOOLBAR_RUN,"Ejecucion")).ToolbarPane().Top().Position(4).Row(config->Toolbars.positions.row_run).LeftDockable(false).RightDockable(false).Hide());

	if (config->Toolbars.wich_ones.misc) 
		aui_manager.AddPane(toolbar_misc, wxAuiPaneInfo().Name("toolbar_misc").Caption(LANG(CAPTION_TOOLBAR_MISC,"Miscelanea")).ToolbarPane().Top().Position(5).Row(config->Toolbars.positions.row_misc).LeftDockable(false).RightDockable(false));
	else
		aui_manager.AddPane(toolbar_misc, wxAuiPaneInfo().Name("toolbar_misc").Caption(LANG(CAPTION_TOOLBAR_MISC,"Miscelanea")).ToolbarPane().Top().Position(5).Row(config->Toolbars.positions.row_misc).LeftDockable(false).RightDockable(false).Hide());
	
	if (config->Toolbars.wich_ones.find) 
		aui_manager.AddPane(toolbar_find, wxAuiPaneInfo().Name("toolbar_find").Caption(LANG(CAPTION_TOOLBAR_FIND,"Busqueda")).ToolbarPane().Top().Position(6).Row(config->Toolbars.positions.row_find).LeftDockable(false).RightDockable(false));
	else
		aui_manager.AddPane(toolbar_find, wxAuiPaneInfo().Name("toolbar_find").Caption(LANG(CAPTION_TOOLBAR_FIND,"Busqueda")).ToolbarPane().Top().Position(6).Row(config->Toolbars.positions.row_find).LeftDockable(false).RightDockable(false).Hide());

	if (config->Toolbars.wich_ones.debug) 
		aui_manager.AddPane(toolbar_debug, wxAuiPaneInfo().Name("toolbar_debug").Caption(LANG(CAPTION_TOOLBAR_DEBUG,"Depuracion")).ToolbarPane().Top().Position(0).Row(config->Toolbars.positions.row_debug).LeftDockable(false).RightDockable(false).Row(3));
	else
		aui_manager.AddPane(toolbar_debug, wxAuiPaneInfo().Name("toolbar_debug").Caption(LANG(CAPTION_TOOLBAR_DEBUG,"Depuracion")).ToolbarPane().Top().Position(0).Row(config->Toolbars.positions.row_debug).LeftDockable(false).RightDockable(false).Row(3).Hide());
	aui_manager.AddPane(toolbar_status, wxAuiPaneInfo().Name("toolbar_status").Caption(LANG(CAPTION_TOOLBAR_STATUS,"Status Toolbar")).ToolbarPane().Top().Position(1).Row(config->Toolbars.positions.row_debug).LeftDockable(false).RightDockable(false).Row(3).Hide());
	aui_manager.AddPane(toolbar_diff, wxAuiPaneInfo().Name("toolbar_diff").Caption(LANG(CAPTION_TOOLBAR_DIFF,"Diff")).ToolbarPane().Top().Float().LeftDockable(false).RightDockable(false).Hide());
	if (config->Init.show_welcome) {
		SHOW_MILLIS("Creating welcome panel...");
		welcome_panel=new mxWelcomePanel(this);
		aui_manager.AddPane(welcome_panel, wxAuiPaneInfo().Name("welcome_panel").CenterPane().PaneBorder(false).Hide());
	}

	aui_manager.SetFlags(aui_manager.GetFlags() | wxAUI_MGR_TRANSPARENT_DRAG | wxAUI_MGR_LIVE_RESIZE);
	
	if (config->Init.show_explorer_tree) {
		wxCommandEvent evt;
		OnViewExplorerTree(evt); // aui update
	}
	aui_manager.Update();

	SHOW_MILLIS("Starting parser...");
	parser = new Parser(symbols_tree.treeCtrl);
	code_helper->AppendIndexes(config->Help.autocomp_indexes);
	autocoder = new Autocoder;
	
	compiler = new mxCompiler(compiler_tree.treeCtrl,compiler_tree.state,compiler_tree.errors,compiler_tree.warnings,compiler_tree.all);

	SHOW_MILLIS("Creating wizard and others...");
	wizard = NULL; //new mxNewWizard(this);
	open_shared = NULL; // new mxOpenSharedWindow(this);
	share = NULL; // new ShareManager();

	parser_timer = new wxTimer(GetEventHandler(),mxID_PARSER_TIMER);
//	where_timer = new wxTimer(GetEventHandler(),mxID_WHERE_TIMER);
//	where_timer->Start(1000,false);
	focus_timer = new wxTimer(GetEventHandler(),mxID_FOCUS_TIMER);
	compiler->timer = new wxTimer(GetEventHandler(),mxID_COMPILER_TIMER);
	project=NULL;
	find_replace_dialog = NULL; // new mxFindDialog(this,wxID_ANY);

	SetDropTarget(new mxDropTarget(NULL));
	
	if (config->Init.show_beginner_panel) {
		CreateBeginnersPanel();
		menu.view_beginner_panel->Check(true);
		if (!config->Init.show_welcome)
			ShowBeginnersPanel();
	}
	
	status_bar->SetStatusText(LANG(GENERAL_READY,"Listo"));
	
	SHOW_MILLIS("Main window done...");
	
	Show(true);
	Maximize(config->Init.maximized);
	
}

void mxMainWindow::OnSymbolTreeDec(wxCommandEvent &event) {
	parser->OnGotoDec(notebook_sources);
}

void mxMainWindow::OnSymbolTreeDef(wxCommandEvent &event) {
	parser->OnGotoDef(notebook_sources);
}

void mxMainWindow::OnSymbolTreePopup(wxTreeEvent &event) {
	mxHidenPanel::ignore_autohide=true;
	parser->OnPopupMenu(event,notebook_sources);
	mxHidenPanel::ignore_autohide=false;
}

void mxMainWindow::OnProjectTreePopup(wxTreeEvent &event) {
	
	if (!project) {
		wxMenu menu("");
		menu.Append(mxID_EDIT_GOTO_FILE, LANG(MENUITEM_EDIT_FIND,"&Buscar..."));
		project_tree.treeCtrl->PopupMenu(&menu,event.GetPoint());
		return;
	}
	
	mxHidenPanel::ignore_autohide=true;
	
	// obtener informacion del item seleccionado
	project_tree.selected_item = event.GetItem();
	bool is_file = !(project_tree.selected_item==project_tree.sources || project_tree.selected_item==project_tree.headers || project_tree.selected_item==project_tree.others);
	wxMenu menu("");
	if (!is_file) {
		project_tree.selected_parent=project_tree.selected_item;
		menu.Append(mxID_PROJECT_POPUP_ADD, LANG(MAINW_PROJECT_FILE_POPUP_ADD,"&Agregar Archivo..."));
		menu.Append(mxID_PROJECT_POPUP_ADD_MULTI, LANG(MAINW_PROJECT_FILE_POPUP_ADD_MULTI,"&Agregar Múltiples Archivos..."));
		menu.Append(mxID_PROJECT_POPUP_OPEN_ALL, LANG(MAINW_PROJECT_FILE_POPUP_OPEN_ALL,"Abrir &Todos"));
	} else {
		project_tree.selected_parent = project_tree.treeCtrl->GetItemParent(project_tree.selected_item);
		menu.Append(mxID_PROJECT_POPUP_OPEN, LANG(MAINW_PROJECT_FILE_POPUP_SHOW_FILE,"&Mostrar Archivo"));
		menu.Append(mxID_PROJECT_POPUP_RENAME, LANG(MAINW_PROJECT_FILE_POPUP_RENAME,"&Renombrar Archivo..."));
		menu.Append(mxID_PROJECT_POPUP_DELETE, LANG(MAINW_PROJECT_FILE_POPUP_DETACH,"&Quitar Archivo"));
		if (project_tree.selected_parent==project_tree.sources) {
			menu.AppendSeparator();
			menu.Append(mxID_PROJECT_POPUP_COMPILE_FIRST, LANG(MAINW_PROJECT_FILE_POPUP_COMPILE_FIRST,"Compilar &Primero"))->Enable(!compiler->IsCompiling());
			menu.Append(mxID_PROJECT_POPUP_COMPILE_NOW, LANG(MAINW_PROJECT_FILE_POPUP_RECOMPILE,"Recompilar A&hora"))->Enable(!compiler->IsCompiling());
		}
		menu.AppendSeparator();
		if (project_tree.selected_parent!=project_tree.sources)
			menu.Append(mxID_PROJECT_POPUP_MOVE_TO_SOURCES, LANG(MAINW_PROJECT_FILE_POPUP_MOVE_TO_SOURCES,"Mover a &Fuentes"));
		if (project_tree.selected_parent!=project_tree.headers)
			menu.Append(mxID_PROJECT_POPUP_MOVE_TO_HEADERS, LANG(MAINW_PROJECT_FILE_POPUP_MOVE_TO_HEADERS,"Mover a &Cabeceras"));
		if (project_tree.selected_parent!=project_tree.others)
			menu.Append(mxID_PROJECT_POPUP_MOVE_TO_OTHERS, LANG(MAINW_PROJECT_FILE_POPUP_MOVE_TO_OTHERS,"Mover a &Otros"));
		menu.AppendSeparator();
		menu.Append(mxID_PROJECT_POPUP_OPEN_FOLDER, LANG(MAINW_PROJECT_FILE_POPUP_OPEN_FOLDER,"Abrir carpeta contenedora..."));
		menu.Append(mxID_PROJECT_POPUP_PROPERTIES, LANG(MAINW_PROJECT_FILE_POPUP_PROPERTIES,"Propiedades..."));
		menu.AppendSeparator();
		menu.Append(mxID_PROJECT_POPUP_ADD_MULTI, LANG(MAINW_PROJECT_FILE_POPUP_ADD_MULTI,"&Agregar Múltiples Archivos..."));
	}	
	menu.AppendSeparator();
	if (config->Init.fullpath_on_project_tree)
		menu.Append(mxID_PROJECT_POPUP_TOGGLE_FULLPATH, LANG(MAINW_PROJECT_FILE_POPUP_TOGGLE_FULL_PATH_OFF,"Mostrar solo nombres de archivos"));
	else
		menu.Append(mxID_PROJECT_POPUP_TOGGLE_FULLPATH, LANG(MAINW_PROJECT_FILE_POPUP_TOGGLE_FULL_PATH_ON,"Mostrar rutas relativas completas"));
	menu.Append(mxID_EDIT_GOTO_FILE, LANG(MENUITEM_EDIT_FIND,"&Buscar..."));
	project_tree.treeCtrl->PopupMenu(&menu,event.GetPoint());
	mxHidenPanel::ignore_autohide=false;
}

void mxMainWindow::OnCompilerTreePopup(wxTreeEvent &event) {
	mxHidenPanel::ignore_autohide=true;
	wxMenu menu("");
	menu.Append(mxID_COMPILER_POPUP_FULL, LANG(MAINW_OPEN_LAST_COMPILER_OUTPUT,"Abrir ultima salida"));
	project_tree.treeCtrl->PopupMenu(&menu);
	mxHidenPanel::ignore_autohide=false;
}

void mxMainWindow::OnCompilerTreeShowFull(wxCommandEvent &event) {
	if (config->Init.show_welcome) main_window->ShowWelcome(false);
	wxString name = _T("<ultima_compilacion>");
	mxSource* source = new mxSource(notebook_sources, AvoidDuplicatePageText(name));
	source->SetStyle(false);
	for (unsigned int i=0;i<compiler->full_output.GetCount();i++)
		source->AppendText(compiler->full_output[i]+"\n");
	notebook_sources->AddPage(source, name ,true, *bitmaps->files.other);
	if (!project) source->treeId = AddToProjectTreeSimple(name,'o');
	source->SetModify(false);
	source->SetReadOnly(true);
	source->SetFocus();
}

void mxMainWindow::OnProjectTreeCompileFirst(wxCommandEvent &event) {
	project->MoveFirst(project_tree.selected_item);
}

void mxMainWindow::OnProjectTreeCompileNow(wxCommandEvent &event) {
	project->PrepareForBuilding(project->FindFromItem(project_tree.selected_item));
	status_bar->SetStatusText(LANG(MAINW_COMPILING_DOTS,"Compilando..."));
	compiler->ResetCompileData();
	wxString current;
	compile_and_run_struct_single *compile_and_run=new compile_and_run_struct_single("OnProjectTreeCompileNow");
	compile_and_run->pid = project->CompileNext(compile_and_run, current);
	StartExecutionStuff(true,false,compile_and_run,current);
}

void mxMainWindow::OnProjectTreeOpen(wxCommandEvent &event) {
	for (int i=0,j=notebook_sources->GetPageCount();i<j;i++)
		if (((mxSource*)(notebook_sources->GetPage(i)))->treeId==project_tree.selected_item) {
			notebook_sources->SetSelection(i);
			return;
		}
	if (project)
		if (!OpenFile(project->GetNameFromItem(project_tree.selected_item),false))
			mxMessageDialog(main_window,wxString()<<LANG(MAINW_FILE_NOT_FOUND,"No se encontro el archivo:")<<"\n"<<project->GetNameFromItem(project_tree.selected_item),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
}

void mxMainWindow::OnProjectTreeOpenAll(wxCommandEvent &event) {
	wxTreeItemIdValue cookie;
	wxTreeItemId item = project_tree.treeCtrl->GetFirstChild(project_tree.selected_item,cookie);
	while ( item.IsOk() ) {
		SetStatusText(wxString(LANG(MAINW_OPENING,"Abriendo"))<<" \""<<project_tree.treeCtrl->GetItemText(item)<<_T("\"..."));
		project_tree.selected_item = item;
		OnProjectTreeOpen(event);
		item = project_tree.treeCtrl->GetNextSibling( item );
	}
	SetStatusText(LANG(GENERAL_READY,"Listo"));
}

void mxMainWindow::OnProjectTreeRename(wxCommandEvent &event) {
	wxFileName fn(DIR_PLUS_FILE(project->path,project->GetNameFromItem(project_tree.selected_item,true)));
	wxFileDialog dlg (this, _T("Renombrar"),fn.GetPath(),fn.GetFullName(), _T("Any file (*)|*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	dlg.SetDirectory(fn.GetPath());
	dlg.SetWildcard(_T("Todos los archivos|"WILDCARD_ALL"|Archivos de C/C++|"WILDCARD_CPP"|Fuentes|"WILDCARD_SOURCE"|Cabeceras|"WILDCARD_HEADER));
	if (dlg.ShowModal() == wxID_OK)
		if (!project->RenameFile(project_tree.selected_item,dlg.GetPath()))
			mxMessageDialog(main_window,LANG(MAINW_PROBLEM_RENAMING,"No se pudo renombrar el archivo"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		
//	wxString res = mxGetTextFromUser(_T("Nuevo nombre:"), _T("Renombrar archivo") , project->GetNameFromItem(project_tree.selected_item,true), this);
//	if (res!=_T(""))
//		if (!project->RenameFile(project_tree.selected_item,res))
//			mxMessageDialog(main_window,_T("No se pudo renombrar el archivo"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
}

void mxMainWindow::OnProjectTreeDelete(wxCommandEvent &event) {
	project->DeleteFile(project_tree.selected_item);
}

void mxMainWindow::OnProjectTreeMoveToSources(wxCommandEvent &event) {
	project->MoveFile(project_tree.selected_item,'s');
}

void mxMainWindow::OnProjectTreeMoveToHeaders(wxCommandEvent &event) {
	project->MoveFile(project_tree.selected_item,'h');
}

void mxMainWindow::OnProjectTreeMoveToOthers(wxCommandEvent &event) {
	project->MoveFile(project_tree.selected_item,'o');
}

void mxMainWindow::OnProjectTreeAddMultiple(wxCommandEvent &event) {
	new mxMultipleFileChooser();
}

void mxMainWindow::OnProjectTreeAddSelected(wxCommandEvent &event) {
	mxSource *src=CURRENT_SOURCE;
	if (project) OpenFile(src->source_filename.GetFullPath(),true);
}

void mxMainWindow::OnProjectTreeAdd(wxCommandEvent &event) {
	wxFileDialog dlg (this, _T("Abrir Archivo"), project?project->last_dir:config->Files.last_dir, _T(" "), _T("Any file (*)|*"), wxFD_OPEN | wxFD_MULTIPLE);
	dlg.SetWildcard(_T("Archivos de C/C++|"WILDCARD_CPP"|Fuentes|"WILDCARD_SOURCE"|Cabeceras|"WILDCARD_HEADER"|Todos los archivos|*"));
	if (project_tree.selected_parent==project_tree.sources)
		dlg.SetFilterIndex(1);
	else if (project_tree.selected_parent==project_tree.headers)
		dlg.SetFilterIndex(2);
	else if (project_tree.selected_parent==project_tree.others)
		dlg.SetFilterIndex(3);
	if (dlg.ShowModal() == wxID_OK) {
		if (project)
			project->last_dir=dlg.GetDirectory();
		else
			config->Files.last_dir=dlg.GetDirectory();
		wxArrayString paths;
		dlg.GetPaths(paths);
		for (unsigned int i=0;i<paths.GetCount();i++) {
			if (!wxFileName::FileExists(dlg.GetPath())) {
				if (mxMD_YES==mxMessageDialog(main_window,LANG(MAINW_CREATE_FILE_QUESTION,"El archivo no existe, desea crearlo?"),dlg.GetPath(),mxMD_YES_NO).ShowModal()) {
					wxTextFile fil(dlg.GetPath());
					fil.Create();
					fil.Write();
				} else
					continue;
			}
			OpenFile(paths[i],true);
		}
	}
}

void mxMainWindow::OnClose (wxCloseEvent &event) {
	if (debug->debugging) {
		debug->Stop();
		return;
	}
	if (parser->working) {
		parser->OnEnd(POE_NONE);
		parser->Stop();
	}
	int pres=mxMD_OK;
	if (project/* && project->modified*/) {
		if (config->Init.save_project) {
			project->Save();
		} else {
			pres = mxMessageDialog(main_window,LANG(MAINW_SAVE_PROJECT_BEFORE_CLOSING,"Desea guardar los cambios del proyecto antes de cerrarlo?"),project->GetFileName(),mxMD_YES_NO_CANCEL|mxMD_QUESTION,LANG(MAINW_ALWAYS_SAVE_PROJECT_ON_CLOSE,"Guardar cambios siempre al cerrar un proyecto"),false).ShowModal();
			if (pres&mxMD_CANCEL)
				return;
			if (pres&mxMD_CHECKED)
				config->Init.save_project=true;
		}
	}
	if (IsMaximized()) {
		config->Init.maximized=true;
	} else if (!IsIconized()) { 
		config->Init.pos_x=GetPosition().x;
		config->Init.pos_y=GetPosition().y;
		config->Init.size_x=GetSize().GetWidth();
		config->Init.size_y=GetSize().GetHeight();
		config->Init.maximized=false;
	}
	if (aui_manager.GetPane("toolbar_file").IsOk())
		config->Toolbars.positions.row_file=aui_manager.GetPane("toolbar_file").dock_row;
	IF_THERE_IS_SOURCE {
		for (int i=0,j=notebook_sources->GetPageCount();i<j;i++) {
			mxSource *source = (mxSource*)(notebook_sources->GetPage(i));
			if (source->GetModify()) {
				notebook_sources->SetSelection(i);
				int res=mxMessageDialog(main_window,LANG(MAINW_SAVE_CHANGES_BEFORE_EXIT_QUESTION,"Hay cambios sin guardar. Desea guardarlos antes de salir?"), source->page_text, (mxMD_YES_NO_CANCEL|mxMD_QUESTION)).ShowModal();
				if (res&mxMD_YES) {
					if (!source->sin_titulo)
						source->SaveSource();
					else {
						wxCommandEvent evt;
						OnFileSaveAs(evt);
					}
				}
				if (res&mxMD_CANCEL || (res&mxMD_YES && source->sin_titulo) )
					return;
			}
		}
	}
	if (project && (config->Init.save_project || (/*project->modified && */pres&mxMD_YES)))
		project->Save();
	config->Init.show_beginner_panel=menu.view_beginner_panel->IsChecked();
	config->Save();
	if (share) delete share;
	main_window=NULL;
	er_uninit();
#if defined(__APPLE__)
	aui_manager.GetPane(toolbar_find).Hide();
	aui_manager.Update(); wxYield();
#endif
	wxExit();
}

void mxMainWindow::OnEditFind (wxCommandEvent &event) {
	if (!find_replace_dialog) find_replace_dialog = new mxFindDialog(this,wxID_ANY);
	IF_THERE_IS_SOURCE {
		find_replace_dialog->ShowFind(CURRENT_SOURCE);
	} else
		find_replace_dialog->ShowFind(NULL);
	return;
}

void mxMainWindow::OnEditFindNext (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		CURRENT_SOURCE->CallTipCancel();
		CURRENT_SOURCE->AutoCompCancel();
	}
	if (!find_replace_dialog) find_replace_dialog = new mxFindDialog(this,wxID_ANY);
	if (find_replace_dialog->last_search.Len()) {
		if (!find_replace_dialog->FindNext())
			mxMessageDialog(main_window,wxString(LANG(FIND_NOT_FOUND_PRE,"La cadena \""))<<find_replace_dialog->last_search<<LANG(FIND_NOT_FOUND_POST,"\" no se encontro."), LANG(FIND_CAPTION,"Buscar"), mxMD_OK|mxMD_INFO).ShowModal();
	} else {
		OnEditFind(event);
	}
}

void mxMainWindow::OnEditFindPrev (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		CURRENT_SOURCE->CallTipCancel();
		CURRENT_SOURCE->AutoCompCancel();
	}
	if (!find_replace_dialog) find_replace_dialog = new mxFindDialog(this,wxID_ANY);
	if (find_replace_dialog->last_search.Len()) {
		if (!find_replace_dialog->FindPrev())
			mxMessageDialog(main_window,wxString(LANG(FIND_NOT_FOUND_PRE,"La cadena \""))<<find_replace_dialog->last_search<<LANG(FIND_NOT_FOUND_POST,"\" no se encontro."), LANG(FIND_CAPTION,"Buscar"), mxMD_OK|mxMD_INFO).ShowModal();
	} else {
		OnEditFind(event);
	}
}

void mxMainWindow::OnEditReplace (wxCommandEvent &event) {
	if (!find_replace_dialog) find_replace_dialog = new mxFindDialog(this,wxID_ANY);
	IF_THERE_IS_SOURCE {
		find_replace_dialog->ShowReplace(CURRENT_SOURCE);
	} else
		find_replace_dialog->ShowReplace(NULL);
	return;
}

void mxMainWindow::OnEditGoto (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		new mxGotoLineWindow(CURRENT_SOURCE,this,wxID_ANY);
	}
}

void mxMainWindow::OnEditGotoFunction (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source=CURRENT_SOURCE;
		int pos=source->GetCurrentPos();
		int s=source->WordStartPosition(pos,true);
		int e=source->WordEndPosition(pos,true);
		wxString key = source->GetTextRange(s,e);
		new mxGotoFunctionDialog(key,this);
	} else
		new mxGotoFunctionDialog("",this);
}

void mxMainWindow::OnEditGotoFile (wxCommandEvent &event) {
	new mxGotoFileDialog("",this);
}

void mxMainWindow::OnQuickHelpLink (wxHtmlLinkEvent &event) {
	wxString action(event.GetLinkInfo().GetHref().BeforeFirst(':'));
	if (action=="quickhelp")
//		ShowQuickHelp(event.GetLinkInfo().GetHref().AfterFirst(':'));
		quick_help->SetPage(help->GetQuickHelp( event.GetLinkInfo().GetHref().AfterFirst(':') ));
	else if (action=="quickfile")
		quick_help->LoadPage(DIR_PLUS_FILE(config->Help.quickhelp_dir,event.GetLinkInfo().GetHref().AfterFirst(':')));
	else if (action=="doxygen")
		utils->OpenInBrowser(wxString("file://")<<event.GetLinkInfo().GetHref().AfterFirst(':'));
	else if (action=="example")
		NewFileFromTemplate(DIR_PLUS_FILE(config->Help.quickhelp_dir,event.GetLinkInfo().GetHref().AfterFirst(':')));
	else if (action=="gotoline") {
		wxString the_one=event.GetLinkInfo().GetHref().AfterFirst(':').BeforeLast(':');
		long int line;
		event.GetLinkInfo().GetHref().AfterLast(':').ToLong(&line);
		for (int i=0,j=notebook_sources->GetPageCount();i<j;i++) {
			mxSource *src =((mxSource*)(notebook_sources->GetPage(i)));
			if ((!src->sin_titulo && src->source_filename==the_one) || 
				(src->sin_titulo && src->page_text==the_one) ||
				(src->temp_filename==the_one && src==parser->source)
				) {
				notebook_sources->SetSelection(i);
				CURRENT_SOURCE->MarkError(line-1);
				return;
			}
		}
		// si no esta abierto
		//		if (mxMD_YEW == mxMessageDialog(main_window,wxString(_T("El archivo "))<<the_one.GetFullName()<<_T(" no esta cargado. Desea cargarlo?"), the_one.GetFullPath(), mxMD_YES_NO|mxMD_QUESTION).ShowModal();
		mxSource *src=OpenFile(the_one,!project);
		if (src && src!=external_source) src->MarkError(line-1);
	} else if (action=="gotopos") {
		mxSource *source=NULL;
		wxString the_one=event.GetLinkInfo().GetHref().AfterFirst(':').BeforeLast(':').BeforeLast(':');
		long int p1=0,p2=0;
		event.GetLinkInfo().GetHref().BeforeLast(':').AfterLast(':').ToLong(&p1);
		event.GetLinkInfo().GetHref().AfterLast(':').ToLong(&p2);
		for (int i=0,j=notebook_sources->GetPageCount();i<j;i++) {
			mxSource *src = ((mxSource*)(notebook_sources->GetPage(i)));
			if ((!src->sin_titulo && src->source_filename==the_one) || (src->sin_titulo && src->page_text==the_one)) {
				notebook_sources->SetSelection(i);
				source=CURRENT_SOURCE;
				break;
			}
		}
		// si no esta abierto
		//		if (mxMD_YEW == mxMessageDialog(main_window,wxString(_T("El archivo "))<<the_one.GetFullName()<<_T(" no esta cargado. Desea cargarlo?"), the_one.GetFullPath(), mxMD_YES_NO|mxMD_QUESTION).ShowModal();
		if (!source)
			source = OpenFile(the_one,!project);
		if (source && source!=external_source) {
			int line=source->LineFromPosition(p1);
			source->MarkError(line-1);
			source->SetSelection(p1,p1+p2);
		}
	} else if (action=="gotolinepos") {
		mxSource *source=NULL;
		wxString the_one=event.GetLinkInfo().GetHref().AfterFirst(':').BeforeLast(':').BeforeLast(':').BeforeLast(':');
		long int p1=0,p2=0,line=0;
		event.GetLinkInfo().GetHref().BeforeLast(':').BeforeLast(':').AfterLast(':').ToLong(&line);
		event.GetLinkInfo().GetHref().BeforeLast(':').AfterLast(':').ToLong(&p1);
		event.GetLinkInfo().GetHref().AfterLast(':').ToLong(&p2);
		for (int i=0,j=notebook_sources->GetPageCount();i<j;i++) {
			mxSource *src = ((mxSource*)(notebook_sources->GetPage(i)));
			if ((src->source_filename==the_one && !src->sin_titulo) || (src->sin_titulo && src->page_text==the_one)) {
				notebook_sources->SetSelection(i);
				source=CURRENT_SOURCE;
				break;
			}
		}
		// si no esta abierto
		//		if (mxMD_YEW == mxMessageDialog(main_window,wxString(_T("El archivo "))<<the_one.GetFullName()<<_T(" no esta cargado. Desea cargarlo?"), the_one.GetFullPath(), mxMD_YES_NO|mxMD_QUESTION).ShowModal();
		if (!source)
			source = OpenFile(the_one,!project);
		if (source && source!=external_source) {
			p1+=source->PositionFromLine(line);
			source->MarkError(line-1);
			source->SetSelection(p1,p1+p2);
		}
	} else
		event.Skip();
}

/// @brief evento generico para el doble click en cualquier arbol, desde aqui se llama al que corresponda
void mxMainWindow::OnSelectTreeItem (wxTreeEvent &event){
	wxYield();
	if (event.GetEventObject()==project_tree.treeCtrl)
		OnSelectSource(event);
	else if (event.GetEventObject()==compiler_tree.treeCtrl)
		OnSelectError(event);
	else if (event.GetEventObject()==symbols_tree.treeCtrl) {
		parser->OnSelectSymbol(event,notebook_sources);
#if defined(_WIN32) || defined(__WIN32__)
		focus_timer->Start(333,true);
#endif
	} else if (event.GetEventObject()==explorer_tree.treeCtrl)
		OnSelectExplorerItem(event);
//	evt.Skip();
}

void mxMainWindow::OnSelectSource (wxTreeEvent &event){
	wxTreeItemId item=event.GetItem();
	for (int i=0,j=notebook_sources->GetPageCount();i<j;i++)
		if (((mxSource*)(notebook_sources->GetPage(i)))->treeId==item) {
			notebook_sources->SetSelection(i);
#if defined(_WIN32) || defined(__WIN32__)
			focus_timer->Start(333,true);
#endif
			return;
		}
	if (project) {
		if (item==project_tree.headers||item==project_tree.sources||item==project_tree.others) {
			event.Skip(); return;
		}
		mxSource *source = OpenFile(project->GetNameFromItem(item),false);
		if (!source)
			mxMessageDialog(main_window,wxString()<<LANG(MAINW_FILE_NOT_FOUND,"No se encontro el archivo:")<<"\n"<<project->GetNameFromItem(item),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		else if (source!=external_source) {
			if (source && project_tree.treeCtrl->GetItemParent(item)==project_tree.others)
				//source->SetStyle(false);
				menu.view_code_style->Check(false);
#if defined(_WIN32) || defined(__WIN32__)
				focus_timer->Start(333,true);
#endif
		}
	}
}

void mxMainWindow::OnSelectError (wxTreeEvent &event){
	// ver si es alguno de los mensajes de zinjai
	wxString item_text=(compiler_tree.treeCtrl->GetItemText(event.GetItem()));
	if (item_text==LANG(MAINW_WARNING_NO_EXCUTABLE_PERMISSION,"El binario no tiene permisos de ejecución.")) {
		LoadInQuickHelpPanel(DIR_PLUS_FILE(config->Help.quickhelp_dir,"zerror_noexecperm.html"),false); return;
	} else if (item_text.EndsWith(LANG(PROJMNGR_FUTURE_SOURCE_POST," tenia fecha de modificacion en el futuro. Se reemplazo por la fecha actual."))) {
		LoadInQuickHelpPanel(DIR_PLUS_FILE(config->Help.quickhelp_dir,"zerror_futuretimestamp.html"),false); return;
	} else if (item_text==LANG(PROJMNGR_MANIFEST_NOT_FOUND,"No se ha encontrado el archivo manifest.xml.")) {
		LoadInQuickHelpPanel(DIR_PLUS_FILE(config->Help.quickhelp_dir,"zerror_missingiconmanifest.html"),false); return;
	}
	// ver que dijo el compilador
	wxYield();
	long int line;
	mxCompilerItemData *comp_data = (mxCompilerItemData*)(compiler_tree.treeCtrl->GetItemData(event.GetItem()));
	if (!comp_data) return;
	wxString error=comp_data->file_info;
	if (!error.Len()) error=compiler_tree.treeCtrl->GetItemText(event.GetItem());;
	bool opened=false;
	
	wxString preline=error[1]==':'?error.AfterFirst(':').AfterFirst(':'):error.AfterFirst(':');
	
	if ( preline.BeforeFirst(':').ToLong(&line) ) {
		// ver si esta abierto
		wxString sthe_one(error[1]!=':' ?error.BeforeFirst(':'):(error.Mid(0,2)+error.AfterFirst(':').BeforeFirst(':')));
		wxFileName the_one;
		if (project)
			the_one=sthe_one=DIR_PLUS_FILE(project->path,sthe_one);
		else IF_THERE_IS_SOURCE
			the_one=sthe_one=DIR_PLUS_FILE(CURRENT_SOURCE->source_filename.GetPath(),sthe_one);
		else
			the_one=sthe_one;
		for (int i=0,j=notebook_sources->GetPageCount();i<j;i++) {
			mxSource *src = ((mxSource*)(notebook_sources->GetPage(i)));
			if ((!src->sin_titulo && SameFile(src->source_filename,the_one)) || (src->temp_filename==the_one && src==compiler->last_compiled) ) {
				notebook_sources->SetSelection(i);
				opened=true; break;
			}
		}
		// si no esta abierto
		if (!opened) {
//			if (mxMD_YES == mxMessageDialog(main_window,wxString(_T("El archivo "))<<the_one.GetFullName()<<_T(" no esta cargado. Desea cargarlo?"), the_one.GetFullPath(), mxMD_YES_NO|mxMD_QUESTION).ShowModal() ) {
				mxSource *src=OpenFile(sthe_one,!project);
				if (src && src!=external_source) src->MarkError(line-1);
//			} else
//				return;
		}

		mxSource *source=CURRENT_SOURCE;
		source->MarkError(line-1);
		
		preline=preline.AfterFirst(':').BeforeFirst(':');
		if (preline.Len()) {
			unsigned int i=0, n=0;
			while (i<preline.size() && preline[i]>='0' && preline[i]<='9') 
				{ n=n*10+preline[i++]-'0'; }
			if (i==preline.Len()) {
				n+=source->PositionFromLine(line-1)-1;
				source->SelectError(0,n,n);
#if defined(_WIN32) || defined(__WIN32__)
				focus_timer->Start(333,true);
#endif
				ShowCompilerTreePanel();
				return;
			}
		}
		
		wxString keyword=error;
		
		int a1 = keyword.Find('\'',true), a2 = keyword.Find('`',true);
		int p1=wxNOT_FOUND, p2=wxNOT_FOUND;
		if (a1==wxNOT_FOUND && a2!=wxNOT_FOUND)
			p1=a2;
		else if (a2==wxNOT_FOUND && a1!=wxNOT_FOUND)
			p1=a1;
		else if (a2!=wxNOT_FOUND && a1!=wxNOT_FOUND)
			p1=a1>a2?a1:a2;
		
		if (p1!=wxNOT_FOUND)
			keyword=keyword.Left(p1);
		
		a1 = keyword.Find('\'',true), a2 = keyword.Find('`',true);
		if (a1==wxNOT_FOUND && a2!=wxNOT_FOUND)
			p2=a2;
		else if (a2==wxNOT_FOUND && a1!=wxNOT_FOUND)
			p2=a1;
		else if (a2!=wxNOT_FOUND && a1!=wxNOT_FOUND)
			p2=a1>a2?a1:a2;
		
		if (p2!=wxNOT_FOUND)
			keyword=keyword.Mid(p2+1);

		bool found=false;
		int endpos = source->PositionFromLine(line);
		int startpos = source->PositionFromLine(line-1);
		int pos;
		if (keyword!=_T("") && keyword!=error) {
			pos=source->FindText(startpos,endpos,keyword,wxSTC_FIND_MATCHCASE|wxSTC_FIND_WHOLEWORD);
			if (pos>=0) {
				found=true;
			} else {
				if (!found && keyword[keyword.Len()-1]==')' && keyword.BeforeLast('(')!="") {
					keyword=keyword.BeforeLast('(');
					if (keyword.AfterLast(':')!="")
						keyword=keyword.AfterLast(':');
					pos=source->FindText(startpos,endpos,keyword,wxSTC_FIND_MATCHCASE|wxSTC_FIND_WHOLEWORD);
					if (pos>=0) {
						found=true;
					} else {
						if (!found && keyword.AfterLast(':')!="") {
							keyword=keyword.AfterLast(':');
							int pos=source->FindText(startpos,endpos,keyword,wxSTC_FIND_MATCHCASE|wxSTC_FIND_WHOLEWORD);
							if (pos>=0) {
								found=true;
							}
						}
					}
				}
			}
		}
		if (found) {
			int p=source->FindText(pos+keyword.Len(),endpos,keyword,wxSTC_FIND_MATCHCASE|wxSTC_FIND_WHOLEWORD);
			if (p>=0) {
				source->SelectError(1,pos,pos+keyword.Len());
				source->SelectError(1,p,p+keyword.Len());
				p=p+keyword.Len();
				while ( (p=source->FindText(p,endpos,keyword,wxSTC_FIND_MATCHCASE|wxSTC_FIND_WHOLEWORD))>=0 ) {
					source->SelectError(1,p,p+keyword.Len());
					p=p+keyword.Len();
				}
				source->GotoPos(source->GetLineIndentPosition(line-1));
			} else {
				source->SelectError(0,pos,pos+keyword.Len());
			}
		}
#if defined(_WIN32) || defined(__WIN32__)
		focus_timer->Start(333,true);
#endif
		ShowCompilerTreePanel();
		return;
	}
}

void mxMainWindow::OnFileOpenH(wxCommandEvent &event){
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		if (source->sin_titulo) return;
		wxString the_one(utils->GetComplementaryFile(source->source_filename));
		if (the_one.Len()) OpenFile(the_one,!project);
	}
}

void mxMainWindow::OnFileOpenSelected(wxCommandEvent &event){
	IF_THERE_IS_SOURCE {
		
		mxSource *source=CURRENT_SOURCE;
		int p1=source->GetSelectionStart();
		int p2=source->GetSelectionEnd();
		if (p1==p2) {
			int pos=source->GetCurrentPos();
			p1=source->WordStartPosition(pos,true);
			p2=source->WordEndPosition(pos,true);
			while ((source->GetStyleAt(p1-1)==wxSTC_C_STRING&&source->GetCharAt(p1-1)!='\"') || source->GetCharAt(p1-1)=='.' || source->GetCharAt(p1-1)=='/' || source->GetCharAt(p1-1)=='\\' || source->GetCharAt(p1-1)==':' || source->GetCharAt(p1-1)=='-' || source->GetCharAt(p1-1)=='_')
				p1=source->WordStartPosition(p1-1,true);
			while ((source->GetStyleAt(p2)==wxSTC_C_STRING&&source->GetCharAt(p2)!='\"') ||source->GetCharAt(p2)=='.' || source->GetCharAt(p2)=='/' || source->GetCharAt(p2)=='\\' || source->GetCharAt(p2)==':' || source->GetCharAt(p2)=='-' || source->GetCharAt(p2)=='_')
				p2=source->WordEndPosition(p2+1,true);
		}
		wxString base_path;
		if (source->GetStyleAt((p1+p2)/2)==wxSTC_C_PREPROCESSOR) {
			if (source->sin_titulo)
				base_path=source->temp_filename.GetPath();
			else
				base_path=source->source_filename.GetPath();
		} else {
			if (project)
				base_path=project->path;
			else
				base_path=source->working_folder.GetFullPath();
		}
		wxString fname=source->GetTextRange(p1,p2);
		if (source->GetStyleAt(p1-1)==wxSTC_C_STRING) fname.Replace("\\\\","\\");
		wxFileName the_one (DIR_PLUS_FILE(base_path,fname));
		if (wxFileName::FileExists(the_one.GetFullPath()))
			OpenFile(the_one.GetFullPath(),!project);
		else 
			new mxGotoFileDialog(source->GetTextRange(p1,p2),this);
		
	}
}

void mxMainWindow::OnHelpOpinion (wxCommandEvent &event){
	new mxOpinionWindow;
}

void mxMainWindow::OnHelpTutorial (wxCommandEvent &event){
	SHOW_HELP("tutorials.html");
}

void mxMainWindow::OnHelpAbout (wxCommandEvent &event){
	new mxAboutWindow(this);
}

void mxMainWindow::OnHelpGui (wxCommandEvent &event){
	if (helpw) {
		helpw->ShowIndex();
		if (helpw->IsIconized())
			helpw->Maximize(false);
		else
			helpw->Raise();
	} else
		helpw = new mxHelpWindow();
}

void mxMainWindow::OnHelpTip (wxCommandEvent &event){
	new mxTipsWindow(this, wxID_ANY, wxDefaultPosition, wxSize(500, 300));
}

void mxMainWindow::OnHelpCpp (wxCommandEvent &event){
	bool ask = true; // preguntar si no hay fuente abierto o palabra seleccionada
	wxString key;
	IF_THERE_IS_SOURCE { // si hay fuente abierto
		mxSource *source=CURRENT_SOURCE; 
		int pos=source->GetCurrentPos(); // buscar la palabra sobre el cursor
		int s=source->WordStartPosition(pos,true);
		int e=source->WordEndPosition(pos,true);
		key = source->GetTextRange(s,e);
		if (key.Len()!=0) { // puede ser una directiva de preprocesador
			ask=false;
			if (source->GetCharAt(s-1)=='#')
				key = source->GetTextRange(s-1,e);
		}
	}
	if (ask) // si no hay clave, preguntar
		key = mxGetTextFromUser(LANG(QUICKHELP_WORDS_TO_SEARCH,"Palabra a buscar:"), LANG(CAPTION_QUICKHELP,"Ayuda Rapida") , _(""), this);
	if (key=="Zaskar") {
		new mxSplashScreen(zskr,GetPosition().x+GetSize().x/2-100,GetPosition().y+GetSize().y/2-150);
		wxString s("Hola, este soy yo... Pablo Novara, alias Zaskar... ;).");
		ShowInQuickHelpPanel(s);
	} else if (key.Len())
		ShowQuickHelp(key); // buscar en la ayuda y mostrar
}


void mxMainWindow::OnNotebookPageChanged(wxAuiNotebookEvent& event) {
//	if (page_change_event_on) {
		if (diff_sidebar) diff_sidebar->Refresh();
		mxSource *source = (mxSource*)notebook_sources->GetPage(event.GetOldSelection());
		if (source) {
			source->CallTipCancel();
			source->AutoCompCancel();
		}
		menu.view_white_space->Check(CURRENT_SOURCE->config_source.whiteSpace);
		menu.view_line_wrap->Check(CURRENT_SOURCE->config_source.wrapMode);
		menu.view_code_style->Check(CURRENT_SOURCE->config_source.syntaxEnable);
		if (!project)
			parser_timer->Start(2000,true);
		event.Veto();
//	}
}

void mxMainWindow::OnNotebookRightClick(wxAuiNotebookEvent& event) {
	// obtener informacion del item seleccionado
	mxSource *src=((mxSource*)notebook_sources->GetPage(event.GetSelection()));
	notebook_sources->SetSelection(notebook_sources->GetPageIndex(src));
	wxMenu menu("");
	if (project) {
		if (project->FindFromItem(src->treeId)) {
			menu.Append(mxID_PROJECT_POPUP_RENAME, LANG(MAINW_PROJECT_FILE_POPUP_RENAME,"Re&nombrar archivo..."));
			project_tree.treeCtrl->SelectItem(src->treeId);
			project_tree.selected_item = src->treeId;
			project_tree.selected_parent = project_tree.treeCtrl->GetItemParent(project_tree.selected_item);
			if (project_tree.selected_parent==project_tree.sources) 
				menu.Append(mxID_PROJECT_POPUP_COMPILE_NOW, LANG(MAINW_PROJECT_FILE_POPUP_RECOMPILE,"Reco&mpilar"))->Enable();
		} else {
			menu.Append(mxID_PROJECT_POPUP_ADD_SELECTED, LANG(MAINW_PROJECT_FILE_POPUP_ADD_TO_PROJECT,"Agregar al proyecto"));
		}
	}
	if (!src->sin_titulo && utils->GetComplementaryFile(src->source_filename).Len())
		menu.Append(mxID_FILE_OPEN_H, LANG(MAINW_PROJECT_FILE_POPUP_OPEN_H,"&Abrir complementario...\tF12"));
	menu.Append(mxID_FILE_OPEN_FOLDER, LANG(MAINW_PROJECT_FILE_POPUP_OPEN_FOLDER,"Abrir carpeta contenedora..."));
	menu.Append(mxID_FILE_EXPLORE_FOLDER, LANG(MAINW_PROJECT_FILE_POPUP_EXPLORE_FOLDER,"Explorar carpeta contenedora"));
	menu.Append(mxID_VIEW_DUPLICATE_TAB, LANG(MENUITEM_VIEW_SPLIT_VIEW,"&Duplicar vista"));
	menu.Append(wxID_SAVE, LANG(MAINW_PROJECT_FILE_POPUP_SAVE,"&Guardar\tCtrl+S"));
	if (!project)
		menu.Append(wxID_SAVEAS, LANG(MAINW_PROJECT_FILE_POPUP_SAVE_AS,"G&uardar Como...\tCtrl+Shift+S"));
	menu.Append(mxID_FILE_RELOAD, LANG(MAINW_PROJECT_FILE_POPUP_RELOAD,"&Recargar\tCtrl+Shift+R"))->Enable(!src->sin_titulo);
	menu.Append(wxID_CLOSE, LANG(MAINW_PROJECT_FILE_POPUP_CLOSE,"&Cerrar\tCtrl+W"));
	if (notebook_sources->GetPageCount()>1)
		menu.Append(mxID_FILE_CLOSE_ALL_BUT_ONE, LANG(MAINW_PROJECT_FILE_POPUP_CLOSE_ALL_BUT_ONE,"Cerrar &todas las demas\tCtrl+Alt+W"));
	menu.Append(mxID_FILE_PROPERTIES, LANG(MAINW_PROJECT_FILE_POPUP_PROPERTIES,"&Propiedades..."));
	notebook_sources->PopupMenu(&menu);
}

void mxMainWindow::OnNotebookPageClose(wxAuiNotebookEvent& event) {
	mxSource *source = CURRENT_SOURCE;
	if (source->GetModify() && source->next_source_with_same_file==source) {
		int res=mxMessageDialog(main_window,LANG(MAINW_SAVE_CHANGES_BEFORE_CLOSING_QUESTION,"Hay cambios sin guardar. Desea guardarlos antes de cerrar el archivo?"), source->page_text, mxMD_YES_NO_CANCEL|mxMD_QUESTION).ShowModal();
		if (res==mxMD_CANCEL) {
			event.Veto();
			return;
		}
		if (res==mxMD_YES) {
			if (source->sin_titulo) {
				wxCommandEvent evt;
				OnFileSaveAs(evt);
				if (source->GetModify()) {
					event.Veto();
					return;
				}
			} else
				source->SaveSource();
		}
	}
	if (!project) {
		project_tree.treeCtrl->Delete(source->treeId);
	} else {
		project->GetSourceExtras(source);
	}
	if (share && share->Exists(source))  {
		int ans =mxMessageDialog(main_window,LANG(MAINW_ASK_CLOSE_SHARED,"El archivo esta siendo compartido con modificaciones. Si lo cierra dejara de estar disponible.\n¿Realmente desea cerrar el archivo?"),source->page_text, mxMD_YES_NO,LANG(MAINW_SHARE_AFTER_CLOSE,"Continuar compartiendo (\"sin modificaciones\") despues de cerrarlo."),false).ShowModal();
		if (mxMD_YES&ans) {
			if (mxMD_CHECKED&ans)
				share->Freeze(source);
			else
				share->Delete(source);
		} else {
			event.Veto();
			return;
		}
	}
	if (!project)
		parser->RemoveSource(source);
	else
		parser->ParseIfUpdated(source->source_filename);
	debug->CloseSource(source);
	if (!project && welcome_panel && notebook_sources->GetPageCount()==1)
		ShowWelcome(true);
}


void mxMainWindow::OnPaneClose(wxAuiManagerEvent& event) {
	if (config->Init.autohiding_panels) {
		for (int i=0;i<ATH_COUNT;i++)
			if (autohide_handlers[i] && autohide_handlers[i]->control==event.pane->window)
				autohide_handlers[i]->ProcessClose();
	}
	if (event.pane->name == "compiler_tree") {
		if (!config->Init.autohiding_panels) compiler_tree.menuItem->Check(false);
	} else if (event.pane->name == "diff_sidebar") {
		aui_manager.DetachPane(diff_sidebar); diff_sidebar->Destroy(); diff_sidebar=NULL;
	} else if (event.pane->name == "left_panels")
		menu.view_left_panels->Check(false);
	else if (event.pane->name == "project_tree") {
		if (!config->Init.autohiding_panels) project_tree.menuItem->Check(false);
	} else if (event.pane->name == "explorer_tree") {
		if (!config->Init.autohiding_panels) explorer_tree.menuItem->Check(false);
	} else if (event.pane->name == "symbols_tree") {
		symbols_tree.menuItem->Check(false);
	}
	else if (event.pane->name == "toolbar_misc") { menu.view_toolbar_misc->Check(false); config->Toolbars.wich_ones.misc=false; }
	else if (event.pane->name == "toolbar_find") { menu.view_toolbar_find->Check(false); config->Toolbars.wich_ones.find=false; }
	else if (event.pane->name == "toolbar_view") { menu.view_toolbar_view->Check(false); config->Toolbars.wich_ones.view=false; }
	else if (event.pane->name == "toolbar_tools") { menu.view_toolbar_tools->Check(false); config->Toolbars.wich_ones.tools=false; }
	else if (event.pane->name == "toolbar_file") { menu.view_toolbar_file->Check(false); config->Toolbars.wich_ones.file=false; }
	else if (event.pane->name == "toolbar_edit") { menu.view_toolbar_edit->Check(false); config->Toolbars.wich_ones.edit=false; }
	else if (event.pane->name == "toolbar_run") { menu.view_toolbar_run->Check(false); config->Toolbars.wich_ones.run=false; }
	else if (event.pane->name == "toolbar_debug") { menu.view_toolbar_debug->Check(false); config->Toolbars.wich_ones.debug=false; }
	else if (event.pane->name == "threadlist" && !config->Init.autohide_panels) debug->threadlist_visible=false;
//	else if (event.pane->name == "backtrace") debug->backtrace_visible=false;
	else if (event.pane->name == "beginner_panel") menu.view_beginner_panel->Check(false);
}

void mxMainWindow::CreateMenus() {
	
	menu.menu = new wxMenuBar;
	
	wxString ipre=DIR_PLUS_FILE(_T("16"),_T(""));

	menu.file = new wxMenu;
	utils->AddItemToMenu(menu.file, wxID_NEW, LANG(MENUITEM_FILE_NEW,"&Nuevo..."),_T("Ctrl+N"),_T("Crear un nuevo archivo"),ipre+_T("nuevo.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_PROJECT, LANG(MENUITEM_FILE_NEW_PROJECT,"&Nuevo Proyecto..."),_T("Ctrl+Shift+N"),_T("Crear un nuevo proyecto"),ipre+_T("proyecto.png"));
	utils->AddItemToMenu(menu.file, wxID_OPEN, LANG(MENUITEM_FILE_OPEN,"&Abrir..."),_T("Ctrl+O"),_T("Abrir un archivo o un proyecto existente..."),ipre+_T("abrir.png"));
	utils->AddSubMenuToMenu(menu.file, menu.file_source_recent = new wxMenu,LANG(MENUITEM_FILE_RECENT_SOURCES,"Fuentes Abiertos Recientemente"),_T("Muestra los ultimos archivos abiertos como programas simples"),ipre+_T("recentSimple.png"));
	utils->AddSubMenuToMenu(menu.file, menu.file_project_recent = new wxMenu,LANG(MENUITEM_FILE_RECENT_PROJECTS,"Proyectos Abiertos Recientemente"),_T("Muestra los ultimos proyectos abiertos"),ipre+_T("recentProject.png"));
//	utils->AddItemToMenu(menu.file, mxID_FILE_OPEN_H, LANG(MENUITEM_FILE_OPEN_H,"Abrir &h/cpp complementario"),_T("F12"),_T("Si se esta editando un .h/.cpp, abrir el .cpp/.h homonimo."),ipre+_T("abrirp.png"));
//	utils->AddItemToMenu(menu.file, mxID_FILE_OPEN_SELECTED, LANG(MENUITEM_FILE_OPEN_SELECTED,"Abrir Se&leccionado"),_T("Ctrl+Enter"),_T("Abrir el archivo seleccionado en el codigo"),ipre+_T("abrirh.png"));
	utils->AddItemToMenu(menu.file, wxID_SAVE, LANG(MENUITEM_FILE_SAVE,"&Guardar"),_T("Ctrl+S"),_T("Guardar el archivo actual"),ipre+_T("guardar.png"));
	utils->AddItemToMenu(menu.file, wxID_SAVEAS, LANG(MENUITEM_FILE_SAVE_AS,"G&uardar Como..."),_T("Ctrl+Shift+S"),_T("Guardar el archivo actual con otro nombre..."),ipre+_T("guardarComo.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_SAVE_ALL, LANG(MENUITEM_FILE_SAVE_ALL,"Guardar &Todo..."),_T("Ctrl+Alt+Shift+S"),_T("Guarda todos los archivos abiertos y el proyecto actual..."),ipre+_T("guardarTodo.png"));
	menu.file_save_project = utils->AddItemToMenu(menu.file, mxID_FILE_SAVE_PROJECT, LANG(MENUITEM_FILE_SAVE_PROJECT,"Guar&dar Proyecto"),_T("Ctrl+Alt+S"),_T("Guardar la configuaricion actual del proyecto"),ipre+_T("guardarProyecto.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_EXPORT_HTML, LANG(MENUITEM_FILE_EXPORT_HTML,"Exportar a HTML..."),_T(""),_T("Genera un archiv HTML con el codigo fuente"),ipre+_T("exportHtml.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_PRINT, LANG(MENUITEM_FILE_PRINT,"&Imprimir..."),_T(""),_T("Imprime el codigo fuente actual"),ipre+_T("imprimir.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_RELOAD, LANG(MENUITEM_FILE_RELOAD,"&Recargar"),_T("Ctrl+Shift+R"),_T("Recarga la version en disco del archivo actual."),ipre+_T("recargar.png"));
	utils->AddItemToMenu(menu.file, wxID_CLOSE, LANG(MENUITEM_FILE_CLOSE,"&Cerrar"),_T("Ctrl+W"),_T("Cerrar el archivo actual"),ipre+_T("cerrar.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_CLOSE_ALL, LANG(MENUITEM_FILE_CLOSE_ALL,"Cerrar &Todo"),_T("Ctrl+Alt+Shift+W"),_T("Cierra todos los archivos abiertos"),ipre+_T("cerrarTodo.png"));
	menu.file_close_project = utils->AddItemToMenu(menu.file, mxID_FILE_CLOSE_PROJECT, LANG(MENUITEM_FILE_CLOSE_PROJECT,"Cerrar Pro&yecto"),_T("Ctrl+Shift+W"),_T("Cierra el proyecto actual"),ipre+_T("cerrarProyecto.png"));
	menu.file->AppendSeparator();
	menu.file_project_config = utils->AddItemToMenu(menu.file, mxID_FILE_PROJECT_CONFIG, LANG(MENUITEM_FILE_PROJECT_CONFIG,"&Configuracion del Proyecto..."),_T("Ctrl+Shift+P"),_T("Configurar las propiedades generales de un proyecto..."),ipre+_T("projectConfig.png"));
	utils->AddItemToMenu(menu.file, mxID_FILE_PREFERENCES, LANG(MENUITEM_FILE_PREFERENCES,"&Preferencias..."),_T("Ctrl+P"),_T("Configurar el entorno..."),ipre+_T("preferencias.png"));
	menu.file->AppendSeparator();
	utils->AddItemToMenu(menu.file, wxID_EXIT, LANG(MENUITEM_FILE_EXIT,"&Salir"),_T("Alt+F4"),_T("Salir del programa!"),ipre+_T("salir.png"));
	menu.file_source_history=new wxMenuItem*[CM_HISTORY_MAX_LEN];
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++) {
		if (i<config->Init.history_len && config->Files.last_source[i][0]!=0)
			menu.file_source_history[i] = utils->AddItemToMenu(menu.file_source_recent, mxID_FILE_SOURCE_HISTORY_0+i,config->Files.last_source[i],_T(""),config->Files.last_source[i],wxString(ipre+_T("recent"))<<i<<(_T(".png")));
		else
			menu.file_source_history[i] = NULL;
	}
	menu.file_source_recent->AppendSeparator();
	utils->AddItemToMenu(menu.file_source_recent, mxID_FILE_SOURCE_HISTORY_MORE,LANG(MENU_FILE_RECENT_MORE,"Mas..."),_T(""),_T("Muestra un dialogo con la lista completa de archivos recientes"),ipre+_T("recentMore.png"));
	menu.file_project_history=new wxMenuItem*[CM_HISTORY_MAX_LEN];
	for (int i=0;i<CM_HISTORY_MAX_LEN;i++) {
		if (i<config->Init.history_len && config->Files.last_project[i][0]!=0)
			menu.file_project_history[i] = utils->AddItemToMenu(menu.file_project_recent, mxID_FILE_PROJECT_HISTORY_0+i, config->Files.last_project[i],_T(""),config->Files.last_project[i],wxString(ipre+_T("recent"))<<i<<(_T(".png")));
		else
			menu.file_project_history[i]=NULL;
	}
	menu.file_project_recent->AppendSeparator();
	utils->AddItemToMenu(menu.file_project_recent, mxID_FILE_PROJECT_HISTORY_MORE,LANG(MENU_FILE_RECENT_MORE,"Mas..."),_T(""),_T("Muestra un dialogo con la lista completa de archivos recientes"),ipre+_T("recentMore.png"));
	menu.menu->Append(menu.file, LANG(MENUITEM_FILE,"&Archivo"));
	menu.file_save_project->Enable(false);
	menu.file_close_project->Enable(false);
	menu.file_project_config->Enable(false);

	menu.edit = new wxMenu;
	utils->AddItemToMenu(menu.edit, wxID_UNDO, LANG(MENUITEM_EDIT_UNDO,"&Deshacer"),_T("Ctrl+Z"),_T("Deshacer el ultimo cambio"),ipre+_T("deshacer.png"));
	utils->AddItemToMenu(menu.edit, wxID_REDO, LANG(MENUITEM_EDIT_REDO,"&Rehacer"),_T("Ctrl+Shift+Z"),_T("Rehacer el ultimo cambio desecho"),ipre+_T("rehacer.png"));
	menu.edit->AppendSeparator();
	utils->AddItemToMenu(menu.edit, wxID_CUT, LANG(MENUITEM_EDIT_CUT,"C&ortar"),_T("Ctrl+X"),_T("Cortar la seleccion al portapapeles"),ipre+_T("cortar.png"));
	utils->AddItemToMenu(menu.edit, wxID_COPY, LANG(MENUITEM_EDIT_COPY,"&Copiar"),_T("Ctrl+C"),_T("Copiar la seleccion al portapapeles"),ipre+_T("copiar.png"));
	utils->AddItemToMenu(menu.edit, wxID_PASTE, LANG(MENUITEM_EDIT_PASTE,"&Pegar"),_T("Ctrl+V"),_T("Pegar el contenido del portapapeles"),ipre+_T("pegar.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_TOGGLE_LINES_UP, LANG(MENUITEM_EDIT_LINES_UP,"Mover Hacia Arriba"),_T("Ctrl+T"),_T("Mueve la o las lineas seleccionadas hacia arriba"),ipre+_T("toggleLinesUp.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_TOGGLE_LINES_DOWN, LANG(MENUITEM_EDIT_LINES_DOWN,"Mover Hacia Abajo"),_T("Ctrl+Shift+T"),_T("Mueve la o las lineas seleccionadas hacia abajo"),ipre+_T("toggleLinesDown.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_DUPLICATE_LINES, LANG(MENUITEM_EDIT_DUPLICATE_LINES,"&Duplicar Linea(s)"),_T("Ctrl+L"),_T("Copia la linea actual del cursor, o las lineas seleccionadas, nuevamente a continuacion"),ipre+_T("duplicarLineas.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_DELETE_LINES, LANG(MENUITEM_EDIT_DELETE_LINES,"&Eliminar Linea(s)"),_T("Shift+Ctrl+L"),_T("Elimina la linea actual del cursor nuevamente, o las lineas seleccionadas"),ipre+_T("borrarLineas.png"));
	menu.edit->AppendSeparator();
	utils->AddItemToMenu(menu.edit, mxID_EDIT_GOTO_FUNCTION, LANG(MENUITEM_EDIT_GOTO_FUNCTION,"&Ir a Funcion/Clase/Metodo..."),_T("Ctrl+Shift+G"),_T("Abrir el fuente con la declaracion de una funcion, clase o metodo..."),ipre+_T("irAFuncion.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_GOTO_FILE, LANG(MENUITEM_EDIT_GOTO_FILE,"&Ir a Archivo..."),_T("Ctrl+Shift+F"),_T("Abrir un archivo en particular buscandolo por parte de su nombre..."),ipre+_T("irAArchivo.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_GOTO, LANG(MENUITEM_EDIT_GOTO_LINE,"&Ir a Linea..."),_T("Ctrl+G"),_T("Mover el cursor a una linea determinada en el archivo..."),ipre+_T("irALinea.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_FIND, LANG(MENUITEM_EDIT_FIND,"&Buscar..."),_T("Ctrl+F"),_T("Buscar una cadena en el archivo..."),ipre+_T("buscar.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_FIND_PREV, LANG(MENUITEM_EDIT_FIND_PREV,"Buscar &Anterior"),_T("Shift+F3"),_T("Repetir la ultima busqueda a partir del cursor hacia atras"),ipre+_T("buscarAnterior.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_FIND_NEXT, LANG(MENUITEM_EDIT_FIND_NEXT,"Buscar &Siguiente"),_T("F3"),_T("Repetir la ultima busqueda a partir del cursor"),ipre+_T("buscarSiguiente.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_REPLACE, LANG(MENUITEM_EDIT_REPLACE,"&Reemplazar..."),_T("Ctrl+R"),_T("Reemplazar una cadena con otra en el archivo..."),ipre+_T("reemplazar.png"));
	menu.edit->AppendSeparator();
	utils->AddItemToMenu(menu.edit, mxID_EDIT_INSERT_HEADER, LANG(MENUITEM_EDIT_INSERT_INCLUDE,"Insertar #include Correspondiente"),_T("Ctrl+H"),_T("Si es posible, inserta el #include necesario para utilizar la funcion/clase en la cual se encuentra el cursor."),ipre+_T("insertarInclude.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_COMMENT, LANG(MENUITEM_EDIT_COMMENT,"Comentar"),_T("Ctrl+D"),_T("Convierte el texto seleccionado en comentario anadiendo \"//\" a cada linea"),ipre+_T("comentar.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_UNCOMMENT, LANG(MENUITEM_EDIT_UNCOMMENT,"Descomentar"),_T("Shift+Ctrl+D"),_T("Descomente el texto seleccionado eliminando \"//\" de cada linea"),ipre+_T("descomentar.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_INDENT, LANG(MENUITEM_EDIT_INDENT,"Indentar Blo&que"),_T("Ctrl+I"),_T("Corrige el indentado de un bloque de codigo agregando o quitando tabs segun corresponda"),ipre+_T("indent.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_BRACEMATCH, LANG(MENUITEM_EDIT_BRACEMATCH,"Seleccionar Blo&que"),_T("Ctrl+M"),_T("Seleccionar todo el bloque correspondiente a la llave o parentesis sobre el cursor"),ipre+_T("mostrarLlave.png"));
	utils->AddItemToMenu(menu.edit, wxID_SELECTALL, LANG(MENUITEM_EDIT_SELECT_ALL,"&Seleccionar Todo"),_T("Ctrl+A"),_T("Seleccionar todo el contenido del archivo"),ipre+_T("seleccionarTodo.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_MARK_LINES, LANG(MENUITEM_EDIT_HIGHLIGHT_LINES,"&Resaltar Linea(s)/Quitar Resaltado"),_T("Ctrl+B"),_T("Resalta la linea pintandola de otro color"),ipre+_T("marcar.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_GOTO_MARK, LANG(MENUITEM_EDIT_FIND_HIGHLIGHTS,"Buscar &Resaltado"),_T("Ctrl+Shift+B"),_T("Mueve el cursor a la siguiente linea resaltada"),ipre+_T("irAMarca.png"));
	utils->AddItemToMenu(menu.edit, mxID_EDIT_LIST_MARKS, LANG(MENUITEM_EDIT_LIST_HIGHLIGHTS,"&Listar Lineas Resaltadas"),_T("Ctrl+Alt+B"),_T("Muestra una lista de las lineas marcadas en todos los archivos"),ipre+_T("listarMarcas.png"));
//	utils->AddItemToMenu(menu.edit, mxID_EDIT_FORCE_AUTOCOMPLETE, LANG(MENUITEM_EDIT_FORCE_AUTOCOMPLETE,"Autocompletar"),_T("Ctrl+Space"),_T("Muestra el menu emergente de autocompletado"),ipre+_T("autocompletar.png"));
//#ifdef DEBUG
//	utils->AddItemToMenu(menu.edit, mxID_EDIT_FUZZY_AUTOCOMPLETE, _T("Autocompletar extendido"),_T("Ctrl+alt+Space"),_T("Muestra el menu emergente de autocompletado"),ipre+_T("autocompletar.png"));
//#endif
	menu.menu->Append(menu.edit, LANG(MENUITEM_EDIT,"&Editar"));
	
	menu.view = new wxMenu;
	wxMenu *fold_menu;
	utils->AddItemToMenu(menu.view, mxID_VIEW_DUPLICATE_TAB, LANG(MENUITEM_VIEW_SPLIT_VIEW,"&Duplicar vista"),_T(""),_T(""),ipre+_T("duplicarVista.png"));
	menu.view_line_wrap = utils->AddCheckToMenu(menu.view, mxID_VIEW_LINE_WRAP, LANG(MENUITEM_VIEW_LINE_WRAP,"&Ajuste de linea"),_T("Alt+F11"),_T("Muestra las lineas largas como en varios renglones"), false);	
	menu.view_white_space = utils->AddCheckToMenu(menu.view, mxID_VIEW_WHITE_SPACE, LANG(MENUITEM_VIEW_WHITE_SPACES,"Mostrar espacios y caracteres de &fin de linea"),_T(""),_T("Muestra las lineas largas como en varios renglones"), false);	
	menu.view_code_style = utils->AddCheckToMenu(menu.view, mxID_VIEW_CODE_STYLE, LANG(MENUITEM_VIEW_SYNTAX_HIGHLIGHT,"&Colorear Sintaxis"),_T("Shift+F11"),_T("Resalta el codigo con diferentes colores y formatos de fuente."), false);	
	utils->AddSubMenuToMenu(menu.view, fold_menu = new wxMenu,LANG(MENUITEM_VIEW_FOLDING,"Plegado"),_T("Muestra opciones para plegar y desplegar codigo en distintos niveles"),ipre+_T("folding.png"));
	menu.view->AppendSeparator();
	menu.view_fullscreen = utils->AddCheckToMenu(menu.view, mxID_VIEW_FULLSCREEN, LANG(MENUITEM_VIEW_FULLSCREEN,"Ver a Pantalla &Completa"),_T("F11"),_T("Muestra el editor a pantalla completa, ocultando tambien los demas paneles"), false);
	menu.view_beginner_panel = utils->AddCheckToMenu(menu.view, mxID_VIEW_BEGINNER_PANEL, LANG(MENUITEM_VIEW_BEGINNER_PANEL,"Mostrar Panel de Mini-Plantillas"),_T(""),_T("Muestra un panel con plantillas y estructuras basicas de c++"), false);
	if (config->Init.left_panels)
		menu.view_left_panels = utils->AddCheckToMenu(menu.view, mxID_VIEW_LEFT_PANELS, LANG(MENUITEM_VIEW_LEFT_PANELS,"&Mostrar Panel de Arboles"),_T(""),_T("Muestra el panel con los arboles de proyecto, simbolos y explorador de archivos"), false);
	project_tree.menuItem = utils->AddCheckToMenu(menu.view, mxID_VIEW_PROJECT_TREE, LANG(MENUITEM_VIEW_PROJECT_TREE,"&Mostrar Arbol de &Proyecto"),_T(""),_T("Muestra el panel del arbol de proyecto/archivos abiertos"), false);
	explorer_tree.menuItem = utils->AddCheckToMenu(menu.view, mxID_VIEW_EXPLORER_TREE, LANG(MENUITEM_VIEW_EXPLORER_TREE,"Mostrar &Explorardor de Archivos"),_T("Ctrl+E"),_T("Muestra el panel explorador de archivos"), false);
	symbols_tree.menuItem = utils->AddCheckToMenu(menu.view, mxID_VIEW_SYMBOLS_TREE, LANG(MENUITEM_VIEW_SYMBOLS_TREE,"Mostrar Arbol de &Simbolos"),_T(""),_T("Analiza el codigo fuente y construye un arbol con los simbolos declarados en el mismo."), false);
	compiler_tree.menuItem = utils->AddCheckToMenu(menu.view, mxID_VIEW_COMPILER_TREE, LANG(MENUITEM_VIEW_COMPILER_TREE,"&Mostrar Resultados de la Compilacion"),_T(""),_T("Muestra un panel con la salida del compilador"), false);
	utils->AddItemToMenu(menu.view, mxID_VIEW_UPDATE_SYMBOLS, LANG(MENUITEM_VIEW_UPDATE_SYMBOLS,"&Actualizar Arbol de Simbolos"),_T("F2"),_T("Actualiza el arbol de simbolos."),ipre+_T("simbolos.png"));
//	utils->AddItemToMenu(menu.view, mxID_VIEW_HIDE_BOTTOM, _T("&Ocultar paneles inferiores"),_T("Escape"),_T("Oculta los paneles de informacion de compilacion y ayuda rapida."),ipre+_T("hideBottom.png"));
	menu.view_toolbar_file = utils->AddCheckToMenu(menu.view, mxID_VIEW_TOOLBAR_FILE, LANG(MENUITEM_VIEW_TOOLBAR_FILE,"&Mostrar Barra de Herramientas Archivo"),_T(""),_T("Muestra la barra de herramientas para el manejo de archivos"), config->Toolbars.wich_ones.file);
	menu.view_toolbar_edit = utils->AddCheckToMenu(menu.view, mxID_VIEW_TOOLBAR_EDIT, LANG(MENUITEM_VIEW_TOOLBAR_EDIT,"&Mostrar Barra de Herramientas Edicion"),_T(""),_T("Muestra la barra de herramientas para la edicion del fuente"), config->Toolbars.wich_ones.edit);
	menu.view_toolbar_view = utils->AddCheckToMenu(menu.view, mxID_VIEW_TOOLBAR_VIEW, LANG(MENUITEM_VIEW_TOOLBAR_VIEW,"&Mostrar Barra de Herramientas Ver"),_T(""),_T("Muestra la barra de herramientas para las opciones de visualizacion"), config->Toolbars.wich_ones.view);
	menu.view_toolbar_find = utils->AddCheckToMenu(menu.view, mxID_VIEW_TOOLBAR_FIND, LANG(MENUITEM_VIEW_TOOLBAR_FIND,"&Mostrar Barra de Busqueda Rapida"),_T(""),_T("Muestra un cuadro de texto en la barra de herramientas que permite buscar rapidamente en un fuente"), config->Toolbars.wich_ones.find);
	menu.view_toolbar_run = utils->AddCheckToMenu(menu.view, mxID_VIEW_TOOLBAR_RUN, LANG(MENUITEM_VIEW_TOOLBAR_RUN,"&Mostrar Barra de Herramientas Ejecucion"),_T(""),_T("Muestra la barra de herramientas para la compilacion y ejecucion del programa"), config->Toolbars.wich_ones.run);
	menu.view_toolbar_tools = utils->AddCheckToMenu(menu.view, mxID_VIEW_TOOLBAR_TOOLS, LANG(MENUITEM_VIEW_TOOLBAR_TOOLS,"&Mostrar Barra de Herramientas Herramientas"),_T(""),_T("Muestra la barra de herramientas para las herramientas adicionales"), config->Toolbars.wich_ones.tools);
	menu.view_toolbar_debug = utils->AddCheckToMenu(menu.view, mxID_VIEW_TOOLBAR_DEBUG, LANG(MENUITEM_VIEW_TOOLBAR_DEBUG,"&Mostrar Barra de Herramientas Depuracion"),_T(""),_T("Muestra la barra de herramientas para la depuracion del programa"), config->Toolbars.wich_ones.debug);
	menu.view_toolbar_misc = utils->AddCheckToMenu(menu.view, mxID_VIEW_TOOLBAR_MISC, LANG(MENUITEM_VIEW_TOOLBAR_MISC,"&Mostrar Barra de Herramientas Miscelanea"),_T(""),_T("Muestra la barra de herramientas con commandos miselaneos"), config->Toolbars.wich_ones.misc);
	menu.view->AppendSeparator();
#if !defined(_WIN32) && !defined(__WIN32__)
	utils->AddItemToMenu(menu.view, mxID_VIEW_PREV_ERROR, LANG(MENUITEM_VIEW_PREV_ERROR,"&Ir a error anterior"),_T("Ctrl+>"),_T("Selecciona el error/advertencia anterior de la salida del compilador."),ipre+_T("errorPrev.png"));
#else
	utils->AddItemToMenu(menu.view, mxID_VIEW_PREV_ERROR, LANG(MENUITEM_VIEW_PREV_ERROR,"&Ir a error anterior"),_T("Ctrl+Shift+<"),_T("Selecciona el error/advertencia anterior de la salida del compilador."),ipre+_T("errorPrev.png"));
#endif
	utils->AddItemToMenu(menu.view, mxID_VIEW_NEXT_ERROR, LANG(MENUITEM_VIEW_NEXT_ERROR,"&Ir a siguiente error"),_T("Ctrl+<"),_T("Selecciona el proximo error/advertencia de la salida del compilador."),ipre+_T("errorNext.png"));
//	menu.view->AppendSeparator();
#if !defined(_WIN32) && !defined(__WIN32__)
//	utils->AddItemToMenu(menu.view, mxID_VIEW_NOTEBOOK_PREV, LANG(MENUITEM_VIEW_PREV_SOURCE,"&Fuente anterior"),_T("Ctrl+PageUp"),_T("Selecciona la pestana del fuente anterior al actual en el area de codigo."),ipre+_T("notebookPrev.png"));
//	utils->AddItemToMenu(menu.view, mxID_VIEW_NOTEBOOK_NEXT, LANG(MENUITEM_VIEW_NEXT_SOURCE,"&Fuente siguiente"),_T("Ctrl+PageDown"),_T("Selecciona la pestana del fuente siguiente al actual en el area de codigo."),ipre+_T("notebookNext.png"));
#else
//	utils->AddItemToMenu(menu.view, mxID_VIEW_NOTEBOOK_PREV, LANG(MENUITEM_VIEW_PREV_SOURCE,"&Fuente anterior"),_T("Ctrl+Shift+Tab"),_T("Selecciona la pestana del fuente anterior al actual en el area de codigo."),ipre+_T("notebookPrev.png"));
//	utils->AddItemToMenu(menu.view, mxID_VIEW_NOTEBOOK_NEXT, LANG(MENUITEM_VIEW_NEXT_SOURCE,"&Fuente siguiente"),_T("Ctrl+Tab"),_T("Selecciona la pestana del fuente siguiente al actual en el area de codigo."),ipre+_T("notebookNext.png"));
#endif
#if defined(__APPLE__)
	menu.menu->Append(menu.view, LANG(MENUITEM_VIEW,"Ver"));
#else
	menu.menu->Append(menu.view, LANG(MENUITEM_VIEW,"&Ver"));
#endif

#ifdef __APPLE__
	utils->AddItemToMenu(fold_menu, mxID_FOLD_FOLD,LANG(MENUITEM_FOLD_FOLD_THIS_LINE,"Plegar en esta linea"),_T(""),_T(""),ipre+_T("foldOne.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_UNFOLD,LANG(MENUITEM_FOLD_UNFOLD_THIS_LINE,"Desplegar en esta linea"),_T(""),_T(""),ipre+_T("unfoldOne.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_1,LANG(MENUITEM_FOLD_FOLD_LEVEL_1,"Plegar el primer nivel"),_T(""),_T("Cierra todos los bolques del primer nivel"),ipre+_T("fold1.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_2,LANG(MENUITEM_FOLD_FOLD_LEVEL_2,"Plegar el segundo nivel"),_T(""),_T("Cierra todos los bolques del segundo nivel"),ipre+_T("unfold2.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_3,LANG(MENUITEM_FOLD_FOLD_LEVEL_3,"Plegar el tercer nivel"),_T(""),_T("Cierra todos los bolques del tercer nivel"),ipre+_T("fold3.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_4,LANG(MENUITEM_FOLD_FOLD_LEVEL_4,"Plegar el cuarto nivel"),_T(""),_T("Cierra todos los bolques del cuarto nivel"),ipre+_T("fold4.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_5,LANG(MENUITEM_FOLD_FOLD_LEVEL_5,"Plegar el quinto nivel"),_T(""),_T("Cierra todos los bolques del quinto nivel"),ipre+_T("fold5.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_ALL,LANG(MENUITEM_FOLD_FOLD_ALL_LEVELS,"Plegar todos los niveles"),_T(""),_T("Cierra todos los bolques de todos los niveles"),ipre+_T("foldAll.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_1,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_1,"Desplegar el primer nivel"),_T(""),_T("Abre todos los bolques del primer nivel"),ipre+_T("unfold1.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_2,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_2,"Desplegar el segundo nivel"),_T(""),_T("Abre todos los bolques del segundo nivel"),ipre+_T("unfold2.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_3,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_3,"Desplegar el tercer nivel"),_T(""),_T("Abre todos los bolques del tercer nivel"),ipre+_T("unfold3.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_4,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_4,"Desplegar el cuarto nivel"),_T(""),_T("Abre todos los bolques del cuarto nivel"),ipre+_T("unfold4.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_5,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_5,"Desplegar el quinto nivel"),_T(""),_T("Abre todos los bolques del quinto nivel"),ipre+_T("unfold5.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_ALL,LANG(MENUITEM_FOLD_UNFOLD_ALL_LEVELS,"Desplegar todos los niveles"),_T(""),_T("Abre todos los bolques de todos los niveles"),ipre+_T("unfoldAll.png"));
#else
	utils->AddItemToMenu(fold_menu, mxID_FOLD_FOLD,LANG(MENUITEM_FOLD_FOLD_THIS_LINE,"Plegar en esta linea"),_T("Alt+Left"),_T(""),ipre+_T("foldOne.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_UNFOLD,LANG(MENUITEM_FOLD_UNFOLD_THIS_LINE,"Desplegar en esta linea"),_T("Alt+Right"),_T(""),ipre+_T("unfoldOne.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_1,LANG(MENUITEM_FOLD_FOLD_LEVEL_1,"Plegar el primer nivel"),_T("Ctrl+1"),_T("Cierra todos los bolques del primer nivel"),ipre+_T("fold1.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_2,LANG(MENUITEM_FOLD_FOLD_LEVEL_2,"Plegar el segundo nivel"),_T("Ctrl+2"),_T("Cierra todos los bolques del segundo nivel"),ipre+_T("unfold2.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_3,LANG(MENUITEM_FOLD_FOLD_LEVEL_3,"Plegar el tercer nivel"),_T("Ctrl+3"),_T("Cierra todos los bolques del tercer nivel"),ipre+_T("fold3.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_4,LANG(MENUITEM_FOLD_FOLD_LEVEL_4,"Plegar el cuarto nivel"),_T("Ctrl+4"),_T("Cierra todos los bolques del cuarto nivel"),ipre+_T("fold4.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_5,LANG(MENUITEM_FOLD_FOLD_LEVEL_5,"Plegar el quinto nivel"),_T("Ctrl+5"),_T("Cierra todos los bolques del quinto nivel"),ipre+_T("fold5.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_HIDE_ALL,LANG(MENUITEM_FOLD_FOLD_ALL_LEVELS,"Plegar todos los niveles"),_T("Ctrl+0"),_T("Cierra todos los bolques de todos los niveles"),ipre+_T("foldAll.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_1,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_1,"Desplegar el primer nivel"),_T("Alt+1"),_T("Abre todos los bolques del primer nivel"),ipre+_T("unfold1.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_2,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_2,"Desplegar el segundo nivel"),_T("Alt+2"),_T("Abre todos los bolques del segundo nivel"),ipre+_T("unfold2.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_3,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_3,"Desplegar el tercer nivel"),_T("Alt+3"),_T("Abre todos los bolques del tercer nivel"),ipre+_T("unfold3.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_4,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_4,"Desplegar el cuarto nivel"),_T("Alt+4"),_T("Abre todos los bolques del cuarto nivel"),ipre+_T("unfold4.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_5,LANG(MENUITEM_FOLD_UNFOLD_LEVEL_5,"Desplegar el quinto nivel"),_T("Alt+5"),_T("Abre todos los bolques del quinto nivel"),ipre+_T("unfold5.png"));
	utils->AddItemToMenu(fold_menu, mxID_FOLD_SHOW_ALL,LANG(MENUITEM_FOLD_UNFOLD_ALL_LEVELS,"Desplegar todos los niveles"),_T("Alt+0"),_T("Abre todos los bolques de todos los niveles"),ipre+_T("unfoldAll.png"));
#endif
	
	menu.run = new wxMenu;
	menu.run_run = utils->AddItemToMenu(menu.run, mxID_RUN_RUN, LANG(MENUITEM_RUN_RUN,"&Ejecutar..."),_T("F9"),_T("Guarda y compila si es necesario, luego ejecuta el programa"),ipre+_T("ejecutar.png"));
	utils->AddItemToMenu(menu.run, mxID_RUN_RUN_OLD, LANG(MENUITEM_RUN_OLD,"Ejecutar (sin recompilar)..."),_T("Ctrl+F9"),_T("Ejecuta el binario existente sin recompilar primero"),ipre+_T("ejecutar_old.png"));
	menu.run_compile = utils->AddItemToMenu(menu.run, mxID_RUN_COMPILE, LANG(MENUITEM_RUN_COMPILE,"&Compilar"),_T("Shift+F9"),_T("Guarda y compila el fuente actual"),ipre+_T("compilar.png"));
//	menu.run_build = utils->AddItemToMenu(menu.run, mxID_RUN_BUILD, LANG(MENUITEM_RUN_BUILD,"Co&nstruir"),_T("Ctrl+F9"),_T("Guarda y compila todo el proyecto"),ipre+_T("construir.png")));
	menu.run_clean = utils->AddItemToMenu(menu.run, mxID_RUN_CLEAN, LANG(MENUITEM_RUN_CLEAN,"&Limpiar"),_T("Ctrl+Shift+F9"),_T("Elimina los objetos y ejecutables compilados"),ipre+_T("limpiar.png"));
	menu.run_stop = utils->AddItemToMenu(menu.run, mxID_RUN_STOP, LANG(MENUITEM_RUN_STOP,"&Detener"),_T(""),_T("Detiene la ejecucion del programa"),ipre+_T("detener.png"));
	utils->AddItemToMenu(menu.run, mxID_RUN_CONFIG, LANG(MENUITEM_RUN_OPTIONS,"&Opciones..."),_T("Alt+F9"),_T("Configura la compilacion y ejecucion de los programas"),ipre+_T("opciones.png"));
	menu.menu->Append(menu.run, LANG(MENUITEM_RUN,"E&jecucion"));
//	menu.run_clean->Enable(false);
	menu.run_stop->Enable(false);
//	menu.run_build->Enable(false);

	menu.debug = new wxMenu;
	menu.debug_run = utils->AddItemToMenu(menu.debug, mxID_DEBUG_RUN, LANG(MENUITEM_DEBUG_START,"&Iniciar/Continuar"),_T("F5"),_T(""),ipre+_T("depurar.png"));
#if !defined(_WIN32) && !defined(__WIN32__)
	menu.debug_attach = utils->AddItemToMenu(menu.debug, mxID_DEBUG_ATTACH, LANG(MENUITEM_DEBUG_ATTACH,"Ad&juntar..."),_T(""),_T(""),ipre+_T("debug_attach.png"));
#endif
	menu.debug_pause = utils->AddItemToMenu(menu.debug, mxID_DEBUG_PAUSE, LANG(MENUITEM_DEBUG_PAUSE,"Interrum&pir"),_T(""),_T(""),ipre+_T("pausar.png"));
	menu.debug_pause->Enable(false);
//	menu.debug_continue = utils->AddItemToMenu(menu.debug, mxID_DEBUG_CONTINUE, _T("&Continuar"),_T(""),_T(""),ipre+_T("continuar.png"));
	menu.debug_stop = utils->AddItemToMenu(menu.debug, mxID_DEBUG_STOP, LANG(MENUITEM_DEBUG_STOP,"&Detener"),_T("Shift+F5"),_T(""),ipre+_T("detener.png"));
	menu.debug_stop->Enable(false);
	menu.debug_step_in = utils->AddItemToMenu(menu.debug, mxID_DEBUG_STEP_IN, LANG(MENUITEM_DEBUG_STEP_IN,"Step &In"),_T("F6"),_T(""),ipre+_T("step_in.png"));
	menu.debug_step_in->Enable(false);
	menu.debug_step_over = utils->AddItemToMenu(menu.debug, mxID_DEBUG_STEP_OVER, LANG(MENUITEM_DEBUG_STEP_OVER,"Step &Over"),_T("F7"),_T(""),ipre+_T("step_over.png"));
	menu.debug_step_over->Enable(false);
	menu.debug_step_out = utils->AddItemToMenu(menu.debug, mxID_DEBUG_STEP_OUT, LANG(MENUITEM_DEBUG_STEP_OUT,"Step O&ut"),_T("Shift+F6"),_T(""),ipre+_T("step_out.png"));
	menu.debug_step_out->Enable(false);
	menu.debug_return = utils->AddItemToMenu(menu.debug, mxID_DEBUG_RETURN, LANG(MENUITEM_DEBUG_RETURN,"&Return"),_T("Ctrl+F6"),_T(""),ipre+_T("return.png"));
	menu.debug_return->Enable(false);
	menu.debug_run_until = utils->AddItemToMenu(menu.debug, mxID_DEBUG_RUN_UNTIL, LANG(MENUITEM_DEBUG_RUN_UNTIL,"Ejecutar &Hasta el Cursor"),_T("Shift+F7"),_T(""),ipre+_T("run_until.png"));
	menu.debug_run_until->Enable(false);
	menu.debug_jump = utils->AddItemToMenu(menu.debug, mxID_DEBUG_JUMP, LANG(MENUITEM_DEBUG_JUMP,"Continuar Desde Aqui"),_T("Ctrl+F5"),_T(""),ipre+_T("debug_jump.png"));
	menu.debug_jump->Enable(false);
#if !defined(_WIN32) && !defined(__WIN32__)
	menu.debug_enable_inverse_execution = utils->AddCheckToMenu(menu.debug, mxID_DEBUG_ENABLE_INVERSE_EXEC, LANG(MENUITEM_DEBUG_ENABLE_INVERSE,"Habilitar Ejecucion Hacia Atras"),_T(""),_T(""),false);
	menu.debug_enable_inverse_execution->Enable(false);
	menu.debug_inverse_execution = utils->AddCheckToMenu(menu.debug, mxID_DEBUG_INVERSE_EXEC, LANG(MENUITEM_DEBUG_INVERSE,"Ejecutar Hacia Atras"),_T(""),_T(""),false);
	menu.debug_inverse_execution->Enable(false);
#endif
	menu.debug_jump->Enable(false);
	menu.debug_inspect = utils->AddItemToMenu(menu.debug, mxID_DEBUG_INSPECT, LANG(MENUITEM_DEBUG_INSPECT,"Panel de In&speccion"),_T(""),_T(""),ipre+_T("inspect.png"));
	menu.debug_backtrace = utils->AddItemToMenu(menu.debug, mxID_DEBUG_BACKTRACE, LANG(MENUITEM_DEBUG_BACKTRACE,"&Trazado Inverso"),_T(""),_T(""),ipre+_T("backtrace.png"));
	menu.debug_threadlist = utils->AddItemToMenu(menu.debug, mxID_DEBUG_THREADLIST, LANG(MENUITEM_DEBUG_THREADLIST,"&Hilos de Ejecucion"),_T(""),_T(""),ipre+_T("threadlist.png"));
	menu.debug_list_breakpoints = utils->AddItemToMenu(menu.debug, mxID_DEBUG_LIST_BREAKPOINTS, LANG(MENUITEM_DEBUG_LIST_BREAKPOINTS,"&Listar Watch/Break points..."),_T("Shift+F8"),_T(""),ipre+_T("breakpoint_list.png"));
//	menu.debug_list_breakpoints->Enable(false);
	menu.debug_toggle_breakpoint = utils->AddItemToMenu(menu.debug, mxID_DEBUG_TOGGLE_BREAKPOINT, LANG(MENUITEM_DEBUG_TOGGLE_BREAKPOINT,"&Agregar/quitar Breakpoint"),_T("F8"),_T(""),ipre+_T("breakpoint.png"));
	menu.debug_toggle_breakpoint = utils->AddItemToMenu(menu.debug, mxID_DEBUG_BREAKPOINT_OPTIONS, LANG(MENUITEM_DEBUG_BREAKPOINT_OPTIONS,"&Opciones del Breakpoint..."),_T("Ctrl+F8"),_T(""),ipre+_T("breakpoint_options.png"));
//	menu.debug_insert_watchpoint = utils->AddItemToMenu(menu.debug, mxID_DEBUG_INSERT_WATCHPOINT, LANG(MENUITEM_DEBUG_INSERT_WATCHPOINT,"&Agregar Watchpoint..."),_T(""),ipre+_T("insert_watchpoint.png"));
//	menu.debug_insert_watchpoint->Enable(false);
//	menu.debug_function_breakpoint = utils->AddItemToMenu(menu.debug, mxID_DEBUG_FUNCTION_BREAKPOINT, LANG(MENUITEM_DEBUG_FUNCTION_BREAKPOINT,"&Insertar Breakpoint en Funcion..."),_T("Alt+F8"),_T(""),ipre+_T("function_breakpoint.png"));
//	menu.debug_function_breakpoint->Enable(false);
	utils->AddItemToMenu(menu.debug, mxID_DEBUG_LOG_PANEL, LANG(MENUITEM_DEBUG_SHOW_LOG_PANEL,"&Mostrar mensajes del depurador"),_T(""),_T(""),ipre+_T("debug_log_panel.png"));
#if !defined(_WIN32) && !defined(__WIN32__)
	menu.debug_core_dump = utils->AddItemToMenu(menu.debug, mxID_DEBUG_CORE_DUMP, LANG(MENUITEM_DEBUG_LOAD_CORE_DUMP,"Cargar &Volcado de Memoria..."),_T(""),_T(""),ipre+_T("core_dump.png"));
#endif
	if (config->Debug.show_do_that) menu.debug_function_breakpoint = utils->AddItemToMenu(menu.debug, mxID_DEBUG_DO_THAT, LANG(MENUITEM_DEBUG_DO_THAT,"DO_THAT"),_T(""),_T(""),ipre+_T("do_that.png"));
	menu.menu->Append(menu.debug, LANG(MENUITEM_DEBUG,"&Depuracion"));


	menu.tools = new wxMenu;
	
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_DRAW_FLOW, LANG(MENUITEM_TOOLS_DRAW_FLOWCHART,"Dibujar Diagrama de &Flujo..."),_T(""),_T("Genera un diagrama de flujo a partir del bloque de codigo actual"),ipre+_T("flujo.png"));
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_DRAW_CLASSES, LANG(MENUITEM_TOOLS_DRAW_CLASS_HIERARCHY,"Dibujar &Jerarquia de Clases..."),_T(""),_T(""),ipre+_T("clases.png"));
	
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_CODE_COPY_FROM_H, LANG(MENUITEM_TOOLS_CODE_COPY_FROM_H,"Implementar Metodos/Funciones faltantes..."),_T("Ctrl+Shift+H"),_T(""),ipre+_T("copy_code_from_h.png"));
	
	wxMenu *comments_menu = new wxMenu;
	utils->AddItemToMenu(comments_menu, mxID_TOOLS_ALIGN_COMMENTS, LANG(MENUITEM_TOOLS_COMMENTS_ALIGN_COMMENTS,"&Alinear Comentarios..."),_T(""),_T("Mueve todos los comentarios hacia una determinada columna"),ipre+_T("align_comments.png"));
	utils->AddItemToMenu(comments_menu, mxID_EDIT_COMMENT, LANG(MENUITEM_TOOLS_COMMENTS_COMMENT,"&Comentar"),_T(""),_T("Convierte el texto seleccionado en comentario anadiendo \"//\" a cada linea"),ipre+_T("comentar.png"));
	utils->AddItemToMenu(comments_menu, mxID_EDIT_UNCOMMENT, LANG(MENUITEM_TOOLS_COMMENTS_UNCOMMENT,"&Descomentar"),_T(""),_T("Descomente el texto seleccionado eliminando \"//\" de cada linea"),ipre+_T("descomentar.png"));
	utils->AddItemToMenu(comments_menu, mxID_TOOLS_REMOVE_COMMENTS, LANG(MENUITEM_TOOLS_COMMENTS_DELETE_COMMENTS,"&Eliminar Comentarios"),_T(""),_T("Quita todos los comentarios del codigo fuente o de la seleccion"),ipre+_T("remove_comments.png"));
	utils->AddSubMenuToMenu(menu.tools, comments_menu,LANG(MENUITEM_TOOLS_COMMENTS,"Coment&arios"),_T("Permite alinear o quitar los comentarios del codigo"),ipre+_T("comments.png"));
	
	wxMenu *preproc_menu = new wxMenu;
	utils->AddItemToMenu(preproc_menu , mxID_TOOLS_PREPROC_EXPAND_MACROS, LANG(MENUITEM_TOOLS_PREPROC_EXPAND_MACROS,"Expandir Macros"),_T("Ctrl+Shift+M"),_T(""),ipre+_T("preproc_expand_macros.png"));
	utils->AddItemToMenu(preproc_menu , mxID_TOOLS_PREPROC_MARK_VALID, LANG(MENUITEM_TOOLS_PREPROC_MARK_VALID,"Marcar Lineas No Compiladas"),_T("Ctrl+Alt+M"),_T(""),ipre+_T("preproc_mark_valid.png"));
	utils->AddItemToMenu(preproc_menu , mxID_TOOLS_PREPROC_UNMARK_ALL, LANG(MENUITEM_TOOLS_PREPROC_UNMARK_ALL,"Borrar Marcas"),_T("Ctrl+Alt+Shift+M"),_T(""),ipre+_T("preproc_unmark_all.png"));
	preproc_menu->AppendSeparator();
	utils->AddItemToMenu(preproc_menu , mxID_TOOLS_PREPROC_HELP, LANG(MENUITEM_TOOLS_PREPROC_HELP,"Ayuda..."),_T(""),_T(""),ipre+_T("ayuda.png"));
	utils->AddSubMenuToMenu(menu.tools, preproc_menu,LANG(MENUITEM_TOOLS_PREPROC,"Preprocesador"),_T("Muestra información generada por el preprocesador de C++"),ipre+_T("preproc.png"));
	
	menu.tools_makefile = utils->AddItemToMenu(menu.tools, mxID_RUN_MAKEFILE, LANG(MENUITEM_TOOLS_GENERATE_MAKEFILE,"&Generar Makefile..."),_T(""),_T("Genera el Makefile a partir de los fuentes y la configuracion seleccionada"),ipre+_T("makefile.png"));
	menu.tools_makefile->Enable(false);
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_CONSOLE, LANG(MENUITEM_TOOLS_OPEN_TERMINAL,"Abrir Co&nsola..."),_T(""),_T("Inicia una terminal para interactuar con el interprete de comandos del sistema operativo"),ipre+_T("console.png"));
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_EXE_PROPS, LANG(MENUITEM_TOOLS_EXE_INFO,"&Propiedades del Ejecutable..."),_T(""),_T("Muestra informacion sobre el archivo compilado"),ipre+_T("exeinfo.png"));
	menu.tools_stats = utils->AddItemToMenu(menu.tools, mxID_TOOLS_PROJECT_STATISTICS, LANG(MENUITEM_TOOLS_PROJECT_STATISTICS,"E&stadisticas del Proyecto..."),_T(""),_T("Muestra informacion estadistica sobre los fuentes y demas archivos del proyecto"),ipre+_T("proystats.png"));
	menu.tools_stats->Enable(false);
	menu.tools_proy_graph = utils->AddItemToMenu(menu.tools, mxID_TOOLS_DRAW_PROJECT, LANG(MENUITEM_TOOLS_DRAW_PROJECT,"Grafo del Proyecto..."),_T(""),_T(""),ipre+_T("draw_project.png"));
	menu.tools_proy_graph->Enable(false);

	wxMenu *share_menu = new wxMenu;
	utils->AddItemToMenu(share_menu, mxID_TOOLS_SHARE_OPEN, LANG(MENUITEM_TOOLS_SHARE_OPEN,"&Abrir compartido..."),_T(""),_T("Abre un archivo compartido por otra PC en la red local."),ipre+_T("abrirs.png"));
	utils->AddItemToMenu(share_menu, mxID_TOOLS_SHARE_SHARE, LANG(MENUITEM_TOOLS_SHARE_SHARE,"&Compartir actual..."),_T(""),_T("Comparte el archivo en la red local."),ipre+_T("compartir.png"));
	utils->AddItemToMenu(share_menu, mxID_TOOLS_SHARE_LIST, LANG(MENUITEM_TOOLS_SHARE_LIST,"&Ver lista de compartidos propios..."),_T(""),_T("Comparte el archivo en la red local."),ipre+_T("share_list.png"));
	share_menu->AppendSeparator();
	utils->AddItemToMenu(share_menu, mxID_TOOLS_SHARE_HELP, LANG(MENUITEM_TOOLS_SHARE_HELP,"A&yuda..."),_T(""),_T("Muestra ayuda acerca de la comparticion de archivos en ZinjaI"),ipre+_T("ayuda.png"));
	utils->AddSubMenuToMenu(menu.tools, share_menu,LANG(MENUITEM_TOOLS_SHARE,"Compartir Archivos en la &Red Local"),_T("Permite enviar o recibir codigos fuentes a traves de una red LAN"),ipre+_T("share.png"));
	
	wxMenu *diff_menu = new wxMenu;
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_TWO, LANG(MENUITEM_TOOLS_DIFF_TWO,"&Dos fuentes abiertos..."),_T(""),_T("Compara dos archivos de texto abiertos y los colorea segun sus diferencias"),ipre+_T("diff_sources.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_DISK, LANG(MENUITEM_TOOLS_DIFF_DISK,"&Fuente actual contra archivo en disco..."),_T(""),_T("Compara un archivo abierto contra un archivo en disco y lo colorea segun sus diferencias"),ipre+_T("diff_source_file.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_HIMSELF, LANG(MENUITEM_TOOLS_DIFF_HIMSELF,"&Cambios en fuente actual contra su version en disco..."),_T(""),_T("Compara un archivos abierto y modificado contra su version en disco y lo colorea segun sus diferencias"),ipre+_T("diff_source_himself.png"));
	diff_menu->AppendSeparator();
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_PREV, LANG(MENUITEM_TOOLS_DIFF_PREV,"Ir a Diferencia Anterior"),_T("Shift+Alt+PageUp"),_T(""),ipre+_T("diff_prev.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_NEXT, LANG(MENUITEM_TOOLS_DIFF_NEXT,"Ir a Siguiente Diferencia"),_T("Shift+Alt+PageDown"),_T(""),ipre+_T("diff_next.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_SHOW, LANG(MENUITEM_TOOLS_DIFF_SHOW,"Mostrar Cambio"),_T("Alt+Shift+Ins"),_T("Muestra en un globo emergente el cambio a aplicar"),ipre+_T("diff_show.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_APPLY, LANG(MENUITEM_TOOLS_DIFF_APPLY,"Apl&icar Cambio"),_T("Alt+Ins"),_T("Aplica el cambio marcado en la linea actual"),ipre+_T("diff_apply.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_DISCARD, LANG(MENUITEM_TOOLS_DIFF_DISCARD,"De&scartar Cambio"),_T("Alt+Del"),_T("Descarta el cambio marcado en la linea actual"),ipre+_T("diff_discard.png"));
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_CLEAR, LANG(MENUITEM_TOOLS_DIFF_CLEAR,"&Borrar Marcas"),_T("Alt+Shift+Del"),_T("Quita los colores y marcas que se agregaron en un fuente producto de una comparacion"),ipre+_T("diff_clear.png"));
	diff_menu->AppendSeparator();
	utils->AddItemToMenu(diff_menu,mxID_TOOLS_DIFF_HELP, LANG(MENUITEM_TOOLS_DIFF_HELP,"A&yuda..."),_T(""),_T("Muestra ayuda acerca de la comparacion de fuentes en ZinjaI"),ipre+_T("ayuda.png"));
	utils->AddSubMenuToMenu(menu.tools, diff_menu,LANG(MENUITEM_TOOLS_DIFF,"&Comparar Archivos (diff)"),_T("Muestra opciones para plegar y desplegar codigo en distintos niveles"),ipre+_T("diff.png"));

	wxMenu *doxy_menu = new wxMenu;
	utils->AddItemToMenu(doxy_menu,mxID_TOOLS_DOXY_GENERATE, LANG(MENUITEM_TOOLS_DOXYGEN_GENERATE,"&Generar..."),_T("Ctrl+Shift+F1"),_T("Ejecuta doxygen para generar la documentacion de forma automatica"),ipre+_T("doxy_run.png"));
	utils->AddItemToMenu(doxy_menu,mxID_TOOLS_DOXY_CONFIG, LANG(MENUITEM_TOOLS_DOXYGEN_CONFIGURE,"&Configurar..."),_T(""),_T("Permite establecer opciones para el archivo de configuracion de doxygen"),ipre+_T("doxy_config.png"));
	utils->AddItemToMenu(doxy_menu,mxID_TOOLS_DOXY_VIEW, LANG(MENUITEM_TOOLS_DOXYGEN_VIEW,"&Ver..."),_T("Ctrl+F1"),_T("Abre un explorador y muestra la documentacion generada"),ipre+_T("doxy_view.png"));
	doxy_menu->AppendSeparator();
	utils->AddItemToMenu(doxy_menu,mxID_TOOLS_DOXY_HELP, LANG(MENUITEM_TOOLS_DOXYGE_HELP,"A&yuda..."),_T(""),_T("Muestra una breve ayuda acerca de la integracion de Doxygen en ZinjaI"),ipre+_T("ayuda.png"));
	menu.tools_doxygen = utils->AddSubMenuToMenu(menu.tools, doxy_menu,LANG(MENUITEM_TOOLS_DOXYGEN,"Generar &Documentacion (doxygen)"),_T("Doxygen permite generar automaticamente documentacion a partir del codigo y sus comentarios"),ipre+_T("doxy.png"));
	menu.tools_doxygen->Enable(false);

	wxMenu *wxfb_menu = new wxMenu;
	menu.tools_wxfb_activate = utils->AddCheckToMenu(wxfb_menu,mxID_TOOLS_WXFB_ACTIVATE, LANG(MENUITEM_TOOLS_WXFB_ACTIVATE,"Activar &Integracion wxFormBuilder"),_T(""),_T("Añade los pasos necesarios a la compilacion para utilizar wxFormBuilder"),false);
	menu.tools_wxfb_auto = utils->AddCheckToMenu(wxfb_menu,mxID_TOOLS_WXFB_AUTO, LANG(MENUITEM_TOOLS_WXFB_AUTO,"Regenerar y Actualizar Automaticamente"),_T(""),_T("Regenera los proyectos si es necesario cada vez que ZinjaI recibe el foco"),false);
	utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_NEW_RES, LANG(MENUITEM_TOOLS_WXFB_NEW_RESOURCE,"&Adjuntar un Nuevo Projecto wxFB..."),_T(""),_T("Crea un nuevo proyecto wxFormBuilder y lo agrega al proyecto en ZinjaI"),ipre+_T("wxfb_new_res.png"));
	utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_LOAD_RES, LANG(MENUITEM_TOOLS_WXFB_LOAD_RESOURCE,"&Adjuntar un Proyecto wxFB Existente..."),_T(""),_T("Agrega un proyecto wxFormBuilder ya existente al proyecto en ZinjaI"),ipre+_T("wxfb_load_res.png"));
	menu.tools_wxfb_regen = utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_REGEN, LANG(MENUITEM_TOOLS_WXFB_REGENERATE,"&Regenerar Proyectos wxFB"),_T("Shift+Alt+F9"),_T("Ejecuta wxFormBuilder para regenerar los archivos de recurso o fuentes que correspondan"),ipre+_T("wxfb_regen.png"));
	menu.tools_wxfb_inherit = utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_INHERIT_CLASS, LANG(MENUITEM_TOOLS_WXFB_INHERIT,"&Generar Clase Heredada..."),_T(""),_T("Genera una nueva clase a partir de las definidas por algun proyecto wxfb"),ipre+_T("wxfb_inherit.png"));
	menu.tools_wxfb_update_inherit = utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_UPDATE_INHERIT, LANG(MENUITEM_TOOLS_WXFB_UPDATE_INHERIT,"Act&ualizar Clase Heredada..."),_T(""),_T("Actualiza los metodos de una clase que hereda de las definidas por algun proyecto wxfb"),ipre+_T("wxfb_update_inherit.png"));
	wxfb_menu->AppendSeparator();
	utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_HELP_WX, LANG(MENUITEM_TOOLS_WXFB_REFERENCE,"Referencia &wxWidgets..."),_T(""),_T("Muestra la ayuda de la biblioteca wxWidgets"),ipre+_T("ayuda_wx.png"));
	utils->AddItemToMenu(wxfb_menu,mxID_TOOLS_WXFB_HELP, LANG(MENUITEM_TOOLS_WXFB_HELP,"A&yuda wxFB..."),_T(""),_T("Muestra una breve ayuda acerca de la integracion de wxFormBuilder en ZinjaI"),ipre+_T("ayuda.png"));
	menu.tools_wxfb = utils->AddSubMenuToMenu(menu.tools, wxfb_menu,LANG(MENUITEM_TOOLS_WXFB,"Diseñar &Interfases (wxFormBuilder)"),_T("Diseño visual de interfaces con la biblioteca wxWidgets"),ipre+_T("wxfb.png"));
	menu.tools_wxfb->Enable(false);

	wxMenu *cppcheck_menu = new wxMenu;
	utils->AddItemToMenu(cppcheck_menu, mxID_TOOLS_CPPCHECK_RUN, LANG(MENUITEM_TOOLS_CPPCHECK_RUN,"Iniciar..."),_T(""),_T(""),ipre+_T("cppcheck_run.png"));
	utils->AddItemToMenu(cppcheck_menu, mxID_TOOLS_CPPCHECK_CONFIG, LANG(MENUITEM_TOOLS_CPPCHECK_CONFIG,"Configurar..."),_T(""),_T(""),ipre+_T("cppcheck_config.png"));
	utils->AddItemToMenu(cppcheck_menu, mxID_TOOLS_CPPCHECK_VIEW, LANG(MENUITEM_TOOLS_CPPCHECK_VIEW,"Mostrar Panel de Resultados"),_T(""),_T(""),ipre+_T("cppcheck_view.png"));
	cppcheck_menu->AppendSeparator();
	utils->AddItemToMenu(cppcheck_menu,mxID_TOOLS_CPPCHECK_HELP, LANG(MENUITEM_TOOLS_CPPCHECK_HELP,"A&yuda..."),_T(""),_T(""),ipre+_T("ayuda.png"));
	menu.tools_cppcheck = utils->AddSubMenuToMenu(menu.tools, cppcheck_menu,LANG(MENUITEM_TOOLS_CPPCHECK,"Analisis Estatico (cppcheck)"),_T(""),ipre+_T("cppcheck.png"));
	menu.tools_cppcheck->Enable(false);
	
	wxMenu *gprof_menu = new wxMenu;
	utils->AddItemToMenu(gprof_menu, mxID_TOOLS_GPROF_SET, LANG(MENUITEM_TOOLS_GPROF_ACTIVATE,"Activar"),_T(""),_T("Anade las opciones necesarias a la compila y reconstruye el ejecutable."),ipre+_T("comp_for_prof.png"));
	wxMenu *gprof_gv_menu = new wxMenu;
	menu.tools_gprof_dot = utils->AddCheckToMenu(gprof_gv_menu, mxID_TOOLS_GPROF_DOT, LANG(MENUITEM_TOOLS_GPROF_DOT,"dot"),_T(""),_T("dot"),config->Init.graphviz_dot);
	menu.tools_gprof_fdp = utils->AddCheckToMenu(gprof_gv_menu, mxID_TOOLS_GPROF_FDP, LANG(MENUITEM_TOOLS_GPROF_FDP,"fdp"),_T(""),_T("fdp"),!config->Init.graphviz_dot);
	utils->AddSubMenuToMenu(gprof_menu, gprof_gv_menu,LANG(MENUITEM_TOOLS_GPROF_LAYOUT,"Algoritmo de Dibujo"),_T("Permite seleccionar entre dos algoritmos diferentes para dibujar el grafo"),ipre+_T("dotfdp.png"));
	utils->AddItemToMenu(gprof_menu, mxID_TOOLS_GPROF_SHOW, LANG(MENUITEM_TOOLS_GPROF_SHOW,"Visualizar Resultados (grafo)..."),_T(""),_T("Muestra graficamente la informacion de profiling de la ultima ejecucion."),ipre+_T("showgprof.png"));
	utils->AddItemToMenu(gprof_menu, mxID_TOOLS_GPROF_LIST, LANG(MENUITEM_TOOLS_GPROF_LIST,"Listar Resultados (texto)"),_T(""),_T("Muestra la informacion de profiling de la ultima ejecucion sin procesar."),ipre+_T("listgprof.png"));
	gprof_menu->AppendSeparator();
	utils->AddItemToMenu(gprof_menu,mxID_TOOLS_GPROF_HELP, LANG(MENUITEM_TOOLS_GPROF_HELP,"A&yuda..."),_T(""),_T("Muestra ayuda acerca de como generar e interpretar la informacion de profiling"),ipre+_T("ayuda.png"));
	utils->AddSubMenuToMenu(menu.tools, gprof_menu,LANG(MENUITEM_TOOLS_GPROF,"Perfil de &Ejecucion (gprof)"),_T("Gprof permite analizar las llamadas a funciones y sus tiempos de ejecucion."),ipre+_T("gprof.png"));

#if !defined(_WIN32) && !defined(__WIN32__)
	wxMenu *valgrind_menu = new wxMenu;
	utils->AddItemToMenu(valgrind_menu, mxID_TOOLS_VALGRIND_RUN, LANG(MENUITEM_TOOLS_VALGRIND_RUN,"Ejecutar..."),_T(""),_T(""),ipre+_T("valgrind_run.png"));
	utils->AddItemToMenu(valgrind_menu, mxID_TOOLS_VALGRIND_VIEW, LANG(MENUITEM_TOOLS_VALGRIND_VIEW,"Mostrar Panel de Resultados"),_T(""),_T(""),ipre+_T("valgrind_view.png"));
	valgrind_menu->AppendSeparator();
	utils->AddItemToMenu(valgrind_menu,mxID_TOOLS_VALGRIND_HELP, LANG(MENUITEM_TOOLS_VALGRIND_HELP,"A&yuda..."),_T(""),_T(""),ipre+_T("ayuda.png"));
	utils->AddSubMenuToMenu(menu.tools, valgrind_menu,LANG(MENUITEM_TOOLS_VALGRIND,"Analisis Dinamico (valgrind)"),_T("Valgrind permite analizar el uso de memoria dinamica para detectar perdidas y otros errores"),ipre+_T("valgrind.png"));
#endif

	utils->AddSubMenuToMenu(menu.tools, menu.tools_custom_menu = new wxMenu,LANG(MENUITEM_TOOLS_CUSTOM_TOOLS,"Herramientas Personalizables"),_T(""),ipre+_T("customTools.png"));
	menu.tools_custom_item=new wxMenuItem*[10];
	for (int i=0;i<10;i++) menu.file_source_history[i] = NULL;
	menu.tools_custom_menu->AppendSeparator();
	utils->AddItemToMenu(menu.tools_custom_menu,mxID_TOOLS_CUSTOM_SETTINGS, LANG(MENUITEM_TOOLS_CUSTOM_SETTINGS,"&Configurar..."),_T(""),_T(""),ipre+_T("customToolsSettings.png"));
	utils->AddItemToMenu(menu.tools_custom_menu,mxID_TOOLS_CUSTOM_HELP, LANG(MENUITEM_TOOLS_CUSTOM_HELP,"A&yuda..."),_T(""),_T(""),ipre+_T("ayuda.png"));	
	UpdateCustomTools();
	
	utils->AddItemToMenu(menu.tools, mxID_TOOLS_INSTALL_COMPLEMENTS, LANG(MENUITEM_TOOLS_INSTALL_COMPLEMENTS,"Instalar Complementos..."),_T(""),_T("Permite instalar un complemento ya descargado para ZinjaI"),ipre+_T("updates.png"));
	
	menu.menu->Append(menu.tools, LANG(MENUITEM_TOOLS,"&Herramientas"));

	menu.help = new wxMenu;
	utils->AddItemToMenu(menu.help,mxID_HELP_ABOUT, LANG(MENUITEM_HELP_ABOUT,"Acerca de..."),_T(""),_T("Acerca de..."),ipre+_T("acercaDe.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_TUTORIAL, LANG(MENUITEM_HELP_TUTORIALS,"Tutoriales..."),_T(""),_T("Abre el cuadro de ayuda y muestra el indice de tutoriales disponibles"),ipre+_T("tutoriales.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_GUI, LANG(MENUITEM_HELP_ZINJAI,"Ayuda sobre ZinjaI..."),_T("F1"),_T("Muestra la ayuda sobre el uso y las caracteristicas de este entorno..."),ipre+_T("ayuda.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_CPP, LANG(MENUITEM_HELP_CPP,"Ayuda sobre C++..."),_T("Shift+F1"),_T("Muestra una ayuda rapida sobre la palabra en la que se encuentra el cursor"),ipre+_T("referencia.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_TIP, LANG(MENUITEM_HELP_TIPS,"&Mostrar sugerencia..."),_T(""),_T("Muestra sugerencias sobre el uso del programa..."),ipre+_T("tip.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_OPINION, LANG(MENUITEM_HELP_OPINION,"Opina sobre ZinjaI..."),_T(""),_T("Permite enviar un mensaje directamente al desarrollador de ZinjaI"),ipre+_T("opinion.png"));
	utils->AddItemToMenu(menu.help,mxID_HELP_UPDATES, LANG(MENUITEM_HELP_UPDATES,"&Buscar actualizaciones..."),_T(""),_T("Comprueba a traves de Internet si hay versiones mas recientes de ZinjaI disponibles..."),ipre+_T("updates.png"));
	menu.menu->Append(menu.help, LANG(MENUITEM_HELP,"A&yuda"));

	SetMenuBar(menu.menu);
}


void mxMainWindow::CreateToolbars(wxToolBar *wich_one) {
	
	toolbar_first_time = false;
	wxString ipre=DIR_PLUS_FILE(wxString()<<config->Toolbars.icon_size,_T(""));
	
	if (wich_one) {
		wich_one->ClearTools();
	} else {
		if (!wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,wxString()<<config->Toolbars.icon_size))) {
			wxString icsz = wxString()<<config->Toolbars.icon_size<<_T("x")<<config->Toolbars.icon_size;
			mxMessageDialog(this,
				wxString()<<LANG(MAIN_WINDOW_NO_ICON_SIZE_PRE,"El tema de iconos seleccionado no tiene iconos del tamano elegido (")
				<<icsz<<LANG(MAIN_WINDOW_NO_ICON_SIZE_POST,")\nSe utilizaran los iconos del tamano predeterminado (16x16).\nPara modificarlo utilice el cuadro de Preferencias (menu Archivo)."),LANG(GENERAL_WARNING,"Advertencia"),mxMD_WARNING|mxMD_OK).ShowModal();
			config->Toolbars.icon_size=16;
			ipre=DIR_PLUS_FILE(_T("16"),_T(""));
		}
		
		toolbar_file = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_edit = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_run = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_misc = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_view = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_tools = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_debug = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_file->SetToolBitmapSize(wxSize(config->Toolbars.icon_size,config->Toolbars.icon_size));
		toolbar_edit->SetToolBitmapSize(wxSize(config->Toolbars.icon_size,config->Toolbars.icon_size));
		toolbar_run->SetToolBitmapSize(wxSize(config->Toolbars.icon_size,config->Toolbars.icon_size));
		toolbar_misc->SetToolBitmapSize(wxSize(config->Toolbars.icon_size,config->Toolbars.icon_size));
		toolbar_debug->SetToolBitmapSize(wxSize(config->Toolbars.icon_size,config->Toolbars.icon_size));
		toolbar_tools->SetToolBitmapSize(wxSize(config->Toolbars.icon_size,config->Toolbars.icon_size));
		toolbar_view->SetToolBitmapSize(wxSize(config->Toolbars.icon_size,config->Toolbars.icon_size));
		
		toolbar_diff = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_diff->SetToolBitmapSize(wxSize(config->Toolbars.icon_size,config->Toolbars.icon_size));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_PREV,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_PREV,"Diferencia Anterior"),ipre+_T("diff_prev.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_PREV,"Herramientas -> Comparar Archivos -> Ir a Diferencia Anterior"));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_NEXT,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_NEXT,"Siguiente Diferencia"),ipre+_T("diff_next.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_NEXT,"Herramientas -> Comparar Archivos -> Ir a Siguiente Diferencia"));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_SHOW,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_SHOW,"Mostrar Cambio"),ipre+_T("diff_show.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_SHOW,"Herramientas -> Comparar Archivos -> Mostrar Cambio"));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_APPLY,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_APPLY,"Aplicar Cambio"),ipre+_T("diff_apply.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_APPLY,"Herramientas -> Comparar Archivos -> Aplicar Cambio"));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_DISCARD,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_DISCARD,"Descartar Cambio"),ipre+_T("diff_discard.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_DISCARD,"Herramientas -> Comparar Archivos -> Descartar Cambio"));
		utils->AddTool(toolbar_diff,mxID_TOOLS_DIFF_CLEAR,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_CLEAR,"Borrar Marcas de Comparacion"),ipre+_T("diff_clear.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_CLEAR,"Herramientas -> Comparar Archivos -> Borrar Marcas"));
		toolbar_diff->Realize();
	}
	
	if (!wich_one || wich_one==toolbar_tools) {
		
		if (config->Toolbars.tools.draw_flow) utils->AddTool(toolbar_tools,mxID_TOOLS_DRAW_FLOW,LANG(TOOLBAR_CAPTION_TOOLS_DRAW_FLOWCHART,"Dibujar Diagrama de Flujo..."),ipre+_T("flujo.png"),LANG(TOOLBAR_DESC_TOOLS_DRAW_FLOWCHART,"Herramientas -> Dibujar Diagrama de Flujo..."));
		if (config->Toolbars.tools.draw_classes) utils->AddTool(toolbar_tools,mxID_TOOLS_DRAW_CLASSES,LANG(TOOLBAR_CAPTION_TOOLS_DRAW_CLASS_HIERARCHY,"Dibujar Jerarquia de Clases..."),ipre+_T("clases.png"),LANG(TOOLBAR_DESC_TOOLS_DRAW_CLASS_HIERARCHY,"Herramientas -> Dibujar Jerarquia de Clases..."));
		if (config->Toolbars.tools.copy_code_from_h) utils->AddTool(toolbar_tools,mxID_TOOLS_CODE_COPY_FROM_H,LANG(TOOLBAR_CAPTION_TOOLS_COPY_CODE_FROM_H,"Implementar Metodos/Funciones Faltantes..."),ipre+_T("copy_code_from_h.png"),LANG(TOOLBAR_DESC_TOOLS_COPY_CODE_FROM_H,"Herramientas -> Implementar Metodos/Funciones Faltantes..."));
		if (config->Toolbars.tools.align_comments) utils->AddTool(toolbar_tools,mxID_TOOLS_ALIGN_COMMENTS,LANG(TOOLBAR_CAPTION_TOOLS_COMMENTS_ALIGN_COMMENTS,"Alinear Comentarios..."),ipre+_T("align_comments.png"),LANG(TOOLBAR_TOOLS_DESC_COMMENTS_ALIGN_COMMENTS,"Herramientas -> Comentarios -> Alinear Comentarios..."));
		if (config->Toolbars.tools.remove_comments) utils->AddTool(toolbar_tools,mxID_TOOLS_REMOVE_COMMENTS,LANG(TOOLBAR_CAPTION_TOOLS_COMMENTS_DELETE_COMMENTS,"Eliminar Comentarios"),ipre+_T("remove_comments.png"),LANG(TOOLBAR_DESC_TOOLS_COMMENTS_DELETE_COMMENTS,"Herramientas -> Comtearios -> Eliminar Comentarios"));
		if (config->Toolbars.tools.preproc_mark_valid) utils->AddTool(toolbar_tools,mxID_TOOLS_PREPROC_MARK_VALID,LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_MARK_VALID,"Marcar Lineas No Compiladas"),ipre+_T("preproc_mark_valid.png"),LANG(TOOLBAR_DESC_TOOLS_PREPROC_MARK_VALID,"Herramientas -> Preprocesador -> Marcar Lineas No Compiladas"));
		if (config->Toolbars.tools.preproc_unmark_all) utils->AddTool(toolbar_tools,mxID_TOOLS_PREPROC_UNMARK_ALL,LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_UNMARK_ALL,"Borrar Marcas"),ipre+_T("preproc_unmark_all.png"),LANG(TOOLBAR_DESC_TOOLS_PREPROC_UNMARK_ALL,"Herramientas -> Preprocesador -> Borrar Marcas"));
		if (config->Toolbars.tools.preproc_expand_macros) utils->AddTool(toolbar_tools,mxID_TOOLS_PREPROC_EXPAND_MACROS,LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_EXPAND_MACROS,"Expandir Macros"),ipre+_T("preproc_expand_macros.png"),LANG(TOOLBAR_DESC_TOOLS_PREPROC_EXPAND_MACROS,"Herramientas -> Preprocesador -> Expandir Macros"));
		if (config->Toolbars.tools.generate_makefile) utils->AddTool(toolbar_tools,mxID_RUN_MAKEFILE,LANG(TOOLBAR_CAPTION_TOOLS_GENERATE_MAKEFILE,"Generar Makefile..."),ipre+_T("makefile.png"),LANG(TOOLBAR_DESC_TOOLS_GENERATE_MAKEFILE,"Herramientas -> Generar Makefile..."));
		if (config->Toolbars.tools.open_terminal) utils->AddTool(toolbar_tools,mxID_TOOLS_CONSOLE,LANG(TOOLBAR_CAPTION_TOOLS_OPEN_TERMINAL,"Abrir terminal..."),ipre+_T("console.png"),LANG(TOOLBAR_DESC_OPEN_TERMINAL,"Herramientas -> Abrir consola..."));
		if (config->Toolbars.tools.exe_info) utils->AddTool(toolbar_tools,mxID_TOOLS_EXE_PROPS,LANG(TOOLBAR_CAPTION_TOOLS_EXE_INFO,"Propiedades del Ejecutable..."),ipre+_T("exeinfo.png"),LANG(TOOLBAR_DESC_EXE_INFO,"Herramientas -> Propiedades del Ejecutable..."));
		if (config->Toolbars.tools.proy_stats) utils->AddTool(toolbar_tools,mxID_TOOLS_PROJECT_STATISTICS,LANG(TOOLBAR_CAPTION_TOOLS_PROY_STATS,"Estadisticas del Proyecto..."),ipre+_T("proystats.png"),LANG(TOOLBAR_DESC_PROY_STATS,"Herramientas -> Estadisticas del Proyecto..."));
		if (config->Toolbars.tools.draw_classes) utils->AddTool(toolbar_tools,mxID_TOOLS_DRAW_CLASSES,LANG(TOOLBAR_CAPTION_TOOLS_DRAW_PROJECT,"Grafo del Proyecto..."),ipre+_T("draw_project.png"),LANG(TOOLBAR_DESC_TOOLS_DRAW_PROJECT,"Herramientas -> Grafo del Proyecto..."));
		if (config->Toolbars.tools.open_shared) utils->AddTool(toolbar_tools,mxID_TOOLS_SHARE_OPEN,LANG(TOOLBAR_CAPTION_TOOLS_SHARE_OPEN,"Abrir Archivo Compartido..."),ipre+_T("abrirs.png"),LANG(TOOLBAR_DESC_TOOLS_SHARE_OPEN,"Herramientas -> Compartir Archivos en la Red Local -> Abrir Compartido..."));
		if (config->Toolbars.tools.share_source) utils->AddTool(toolbar_tools,mxID_TOOLS_SHARE_SHARE,LANG(TOOLBAR_CAPTION_TOOLS_SHARE_SHARE,"Compartir Fuente..."),ipre+_T("compartir.png"),LANG(TOOLBAR_DESC_TOOLS_SHARE_SHARE,"Herramientas -> Compartir Archivos en la Red Local -> Compartir Actual..."));
		if (config->Toolbars.tools.share_list) utils->AddTool(toolbar_tools,mxID_TOOLS_SHARE_LIST,LANG(TOOLBAR_CAPTION_TOOLS_SHARE_LIST,"Listar Compartidos Propios..."),ipre+_T("share_list.png"),LANG(TOOLBAR_DESC_TOOLS_SHARE_LIST,"Herramientas -> Compartir Archivos en la Red Local -> Listar Compartidos Propios..."));
		if (config->Toolbars.tools.diff_two_sources) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_TWO,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_TWO,"Compara Dos Fuentes Abiertos..."),ipre+_T("diff_sources.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_TWO,"Herramientas -> Comparar Archivos -> Dos Fuentes Abiertos"));
		if (config->Toolbars.tools.diff_other_file) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_DISK,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_DISK,"Comparar Fuente con Archivo en Disco..."),ipre+_T("diff_source_file.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_DISK,"Herramientas -> Comparar Archivos -> Fuente Actual Contra Archivo en Disco"));
		if (config->Toolbars.tools.diff_himself) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_HIMSELF,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_HIMSELF,"Comparar Cambios en el Fuente con su Version en Disco..."),ipre+_T("diff_source_himself.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_HIMSELF,"Herramientas -> Comparar Archivos -> Fuente Actual Contra Version en Disco"));
		if (config->Toolbars.tools.diff_show) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_SHOW,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_SHOW,"Mostrar Cambio"),ipre+_T("diff_show.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_SHOW,"Herramientas -> Comparar Archivos -> Mostrar Cambio"));
		if (config->Toolbars.tools.diff_apply) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_APPLY,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_APPLY,"Aplicar Cambio"),ipre+_T("diff_apply.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_APPLY,"Herramientas -> Comparar Archivos -> Aplicar Cambio"));
		if (config->Toolbars.tools.diff_discard) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_DISCARD,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_DISCARD,"Descartar Cambio"),ipre+_T("diff_discard.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_DISCARD,"Herramientas -> Comparar Archivos -> Descartar Cambio"));
		if (config->Toolbars.tools.diff_clear) utils->AddTool(toolbar_tools,mxID_TOOLS_DIFF_CLEAR,LANG(TOOLBAR_CAPTION_TOOLS_DIFF_CLEAR,"Borrar Marcas de Comparacion"),ipre+_T("diff_clear.png"),LANG(TOOLBAR_DESC_TOOLS_DIFF_CLEAR,"Herramientas -> Comparar Archivos -> Borrar Marcas"));
		if (config->Toolbars.tools.doxy_generate) utils->AddTool(toolbar_tools,mxID_TOOLS_DOXY_GENERATE,LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_GENERATE,"Generar Documentacion Doxygen..."),ipre+_T("doxy_run.png"),LANG(TOOLBAR_DESC_TOOLS_DOXYGEN_GENERATE,"Herramientas -> Generar Documentacion -> Generar..."));
		if (config->Toolbars.tools.doxy_config) utils->AddTool(toolbar_tools,mxID_TOOLS_DOXY_CONFIG,LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_CONFIGURE,"Configurar Documentacion Doxygen..."),ipre+_T("doxy_config.png"),LANG(TOOLBAR_DESC_TOOLS_DOXYGEN_CONFIGURE,"Herramientas -> Generar Documentacion -> Configurar..."));
		if (config->Toolbars.tools.doxy_view) utils->AddTool(toolbar_tools,mxID_TOOLS_DOXY_VIEW,LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_VIEW,"Ver Documentacion Doxygen..."),ipre+_T("doxy_view.png"),LANG(TOOLBAR_DESC_TOOLS_DOXYGEN_VIEW,"Herramientas -> Generar Documentacion -> Ver..."));
		if (config->Toolbars.tools.wxfb_activate) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_ACTIVATE,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_ACTIVATE,"Activar Integracion wxFormBuilder"),ipre+_T("wxfb_activate.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_ACTIVATE,"Herramientas -> wxFormBuilder -> Activar Integracion wxFormBuilder"));
		if (config->Toolbars.tools.wxfb_new_res) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_NEW_RES,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_NEW_RESOURCE,"Adjuntar un Nuevo Proyecto wxFB"),ipre+_T("wxfb_new_res.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_NEW_RESOURCE,"Herramientas -> wxFormBuilder -> Adjuntar un Nuevo Proyecto wxFB"));
		if (config->Toolbars.tools.wxfb_load_res) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_LOAD_RES,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_LOAD_RESOURCE,"Adjuntar un Proyecto wxFB Existente"),ipre+_T("wxfb_load_res.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_LOAD_RESOURCE,"Herramientas -> wxFormBuilder -> Adjuntar un Proyecto wxFB Existente"));
		if (config->Toolbars.tools.wxfb_regen) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_REGEN,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_REGENERATE,"Regenerar Proyectos wxFB"),ipre+_T("wxfb_regen.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_REGENERATE,"Herramientas -> wxFormBuilder -> Regenerar Proyectos wxFB"));
		if (config->Toolbars.tools.wxfb_inherit) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_INHERIT_CLASS,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_INHERIT,"Generar Clase Heredada..."),ipre+_T("wxfb_inherit.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_INHERIT,"Herramientas -> wxFormBuilder -> Generar Clase Heredada..."));
		if (config->Toolbars.tools.wxfb_update_inherit) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_UPDATE_INHERIT,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_UPDATE_INHERIT,"Actualizar Clase Heredada..."),ipre+_T("wxfb_update_inherit.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_UPDATE_INHERIT,"Herramientas -> wxFormBuilder -> Actualizar Clase Heredada..."));
		if (config->Toolbars.tools.wxfb_help_wx) utils->AddTool(toolbar_tools,mxID_TOOLS_WXFB_HELP_WX,LANG(TOOLBAR_CAPTION_TOOLS_WXFB_REFERENCE,"Referencia wxWidgets..."),ipre+_T("ayuda_wx.png"),LANG(TOOLBAR_DESC_TOOLS_WXFB_REFERENCE,"Herramientas -> wxFormBuilder -> Referencia wxWidgets..."));
		if (config->Toolbars.tools.cppcheck_run) utils->AddTool(toolbar_tools,mxID_TOOLS_CPPCHECK_RUN,LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_RUN,"Comenzar Analisis Estatico..."),ipre+_T("cppcheck_run.png"),LANG(TOOLBAR_DESC_TOOLS_CPPCHECK_RUN,"Herramientas -> Analisis Estatico -> Iniciar..."));
		if (config->Toolbars.tools.cppcheck_config) utils->AddTool(toolbar_tools,mxID_TOOLS_CPPCHECK_CONFIG,LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_CONFIG,"Configurar Analisis Estatico..."),ipre+_T("cppcheck_config.png"),LANG(TOOLBAR_DESC_TOOLS_CPPCHECK_CONFIG,"Herramientas -> Analisis Estatico-> Configurar..."));
		if (config->Toolbars.tools.cppcheck_view) utils->AddTool(toolbar_tools,mxID_TOOLS_CPPCHECK_VIEW,LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_VIEW,"Mostrar Resultados del Analisis Estatico"),ipre+_T("cppcheck_view.png"),LANG(TOOLBAR_DESC_TOOLS_CPPCHECK_VIEW,"Herramientas -> Analisis Estatico -> Mostrar Panel de Resultados"));
		if (config->Toolbars.tools.gprof_activate) utils->AddTool(toolbar_tools,mxID_TOOLS_GPROF_SET,LANG(TOOLBAR_CAPTION_TOOLS_GPROF_ACTIVATE,"Activar Perfilado de Ejecucion"),ipre+_T("comp_for_prog.png"),LANG(TOOLBAR_DESC_TOOLS_GPROF_ACTIVATE,"Herramientas -> Perfil de Ejecucion -> Activar"));
		if (config->Toolbars.tools.gprof_show_graph) utils->AddTool(toolbar_tools,mxID_TOOLS_GPROF_SHOW,LANG(TOOLBAR_CAPTION_TOOLS_GPROF_SHOW,"Graficar Resultados del Perfilado de Ejecucion"),ipre+_T("showgprof.png"),LANG(TOOLBAR_DESC_TOOLS_GPROF_SHOW,"Herramientas -> Perfil de Ejecucion -> Visualizar Resultados (grafo)..."));
		if (config->Toolbars.tools.gprof_list_output) utils->AddTool(toolbar_tools,mxID_TOOLS_GPROF_LIST,LANG(TOOLBAR_CAPTION_TOOLS_GPROF_LIST,"Listar Resultados del Perfilado de Ejecucion"),ipre+_T("listgprof.png"),LANG(TOOLBAR_DESC_TOOLS_GPROF_LIST,"Herramientas -> Perfil de Ejecucion -> Listar Resultados (texto)..."));
#if !defined(__WIN32__)
		if (config->Toolbars.tools.valgrind_run) utils->AddTool(toolbar_tools,mxID_TOOLS_VALGRIND_RUN,LANG(TOOLBAR_CAPTION_TOOLS_VALGRIND_RUN,"Ejecutar Para Analisis Dinamico"),ipre+_T("valgrind_run.png"),LANG(TOOLBAR_DESC_TOOLS_VALGRIND_RUN,"Herramientas -> Analisis Dinámico -> Ejecutar..."));
		if (config->Toolbars.tools.valgrind_view) utils->AddTool(toolbar_tools,mxID_TOOLS_VALGRIND_VIEW,LANG(TOOLBAR_CAPTION_TOOLS_VALGRIND_VIEW,"Mostrar Resultados del Analisis Dinamico"),ipre+_T("valgrind_view.png"),LANG(TOOLBAR_DESC_TOOLS_VALGRIND_VIEW,"Herramientas -> Analisis Dinámico -> Mostrar Panel de Resultados"));
#endif
		for (int i=0;i<10;i++) {
			if (config->Toolbars.tools.custom[i]) {
				if (config->CustomTools.names[i].Len())
					utils->AddTool(toolbar_tools,mxID_CUSTOM_TOOL_0+i,
						wxString(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_TOOL,"Herramienta Personalizada "))<<i<<" ("<<config->CustomTools.names[i]<<")",
						ipre+wxString("customTool")<<i<<".png",
						wxString(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_TOOL,"Herramienta Personalizada "))<<i<<" ("<<config->CustomTools.names[i]<<")");
				else
					utils->AddTool(toolbar_tools,mxID_CUSTOM_TOOL_0+i,
						wxString(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_TOOL,"Herramienta Personalizada "))<<i<<" ("<<LANG(CUSTOM_TOOLS_NOT_CONFIGURED,"no configurada")<<")",
						ipre+wxString("customTool")<<i<<".png",
						wxString(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_TOOL,"Herramienta Personalizada "))<<i<<" ("<<LANG(CUSTOM_TOOLS_NOT_CONFIGURED,"no configurada")<<")");
			}
		}
		if (config->Toolbars.tools.custom_settings) utils->AddTool(toolbar_tools,mxID_TOOLS_CUSTOM_SETTINGS,LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_SETTINGS,"Configurar Herramientas Personalizadas"),ipre+_T("customToolsSettings.png"),LANG(TOOLBAR_DESC_TOOLS_CUSTOM_SETTINGS,"Herramientas -> Herramientas Personalizadas -> Configurar..."));
		toolbar_tools->Realize();
	}
	
	if (!wich_one || wich_one==toolbar_view) {
		
		if (config->Toolbars.view.split_view) utils->AddTool(toolbar_view,mxID_VIEW_DUPLICATE_TAB,LANG(TOOLBAR_CAPTION_VIEW_SPLIT_VIEW,"Duplicar Vista"),ipre+_T("duplicarVista.png"),LANG(TOOLBAR_DESC_VIEW_SPLIT_VIEW,""));
		if (config->Toolbars.view.line_wrap) utils->AddTool(toolbar_view,mxID_VIEW_LINE_WRAP,LANG(TOOLBAR_CAPTION_VIEW_LINE_WRAP,"Ajuste de Linea"),ipre+_T("lineWrap.png"),LANG(TOOLBAR_DESC_VIEW_LINE_WRAP,""));
		if (config->Toolbars.view.white_space) utils->AddTool(toolbar_view,mxID_VIEW_WHITE_SPACE,LANG(TOOLBAR_CAPTION_VIEW_WHITE_SPACES,"Mostrar espacios y caracteres de de fin de linea"),ipre+_T("whiteSpace.png"),LANG(TOOLBAR_DESC_VIEW_WHITE_SPACES,""));
		if (config->Toolbars.view.sintax_colour) utils->AddTool(toolbar_view,mxID_VIEW_CODE_STYLE,LANG(TOOLBAR_CAPTION_VIEW_SYNTAX_HIGHLIGHT,"Colorear Sintaxis"),ipre+_T("syntaxColour.png"),LANG(TOOLBAR_DESC_VIEW_SYNTAX_HIGHLIGHT,""));
		if (config->Toolbars.view.update_symbols) utils->AddTool(toolbar_view,mxID_VIEW_UPDATE_SYMBOLS,LANG(TOOLBAR_CAPTION_VIEW_SYMBOLS_TREE,"Arbol de simbolos"),ipre+_T("symbolsTree.png"),LANG(TOOLBAR_DESC_VIEW_SYMBOLS_TREE,""));
		if (config->Toolbars.view.explorer_tree) utils->AddTool(toolbar_view,mxID_VIEW_EXPLORER_TREE,LANG(TOOLBAR_CAPTION_VIEW_EXPLORER_TREE,"Explorador de Archivos"),ipre+_T("explorerTree.png"),LANG(TOOLBAR_DESC_VIEW_EXPLORER_TREE,""));
		if (config->Toolbars.view.project_tree) utils->AddTool(toolbar_view,mxID_VIEW_PROJECT_TREE,LANG(TOOLBAR_CAPTION_VIEW_PROJECT_TREE,"Arbol de Proyecto"),ipre+_T("projectTree.png"),LANG(TOOLBAR_DESC_VIEW_PROJECT_TREE,""));
		if (config->Toolbars.view.compiler_tree) utils->AddTool(toolbar_view,mxID_VIEW_COMPILER_TREE,LANG(TOOLBAR_CAPTION_VIEW_COMPILER_TREE,"Resultados de La Compilacion"),ipre+_T("compilerTree.png"),LANG(TOOLBAR_DESC_VIEW_COMPILER_TREE,""));
		if (config->Toolbars.view.full_screen) utils->AddTool(toolbar_view,mxID_VIEW_FULLSCREEN,LANG(TOOLBAR_CAPTION_VIEW_FULLSCREEN,"Pantalla Completa"),ipre+_T("fullScreen.png"),LANG(TOOLBAR_DESC_VIEW_FULLSCREEN,""));
		if (config->Toolbars.view.beginner_panel) utils->AddTool(toolbar_view,mxID_VIEW_BEGINNER_PANEL,LANG(TOOLBAR_CAPTION_VIEW_BEGINNER_PANEL,"Panel de Asistencias"),ipre+_T("beginer_panel.png"),LANG(TOOLBAR_DESC_VIEW_BEGINER_PANEL,""));
		if (config->Toolbars.view.prev_error) utils->AddTool(toolbar_view,mxID_VIEW_PREV_ERROR,LANG(TOOLBAR_CAPTION_VIEW_PREV_ERROR,"Ir a Error Anterior"),ipre+_T("errorPrev.png"),LANG(TOOLBAR_DESC_VIEW_PREV_ERROR,""));
		if (config->Toolbars.view.next_error) utils->AddTool(toolbar_view,mxID_VIEW_NEXT_ERROR,LANG(TOOLBAR_CAPTION_VIEW_NEXT_ERROR,"Ir a siguiente error"),ipre+_T("errorNext.png"),LANG(TOOLBAR_DESC_VIEW_NEXT_ERROR,""));
		if (config->Toolbars.view.fold_all) utils->AddTool(toolbar_view,mxID_FOLD_HIDE_ALL,LANG(TOOLBAR_CAPTION_FOLD_ALL_LEVELS,"Plegar Todos los Niveles"),ipre+_T("foldAll.png"),LANG(TOOLBAR_DESC_FOLD_ALL_LEVELS,""));
		if (config->Toolbars.view.fold_1) utils->AddTool(toolbar_view,mxID_FOLD_HIDE_1,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_1,"Plegar el Primer Nivel"),ipre+_T("fold1.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_1,""));
		if (config->Toolbars.view.fold_2) utils->AddTool(toolbar_view,mxID_FOLD_HIDE_2,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_2,"Plegar el Segundo Nivel"),ipre+_T("fold2.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_2,""));
		if (config->Toolbars.view.fold_3) utils->AddTool(toolbar_view,mxID_FOLD_HIDE_3,LANG(TOOLBAR_CAPTION_FOLD_LEVEL_3,"Plegar el Tercer Nivel"),ipre+_T("fold3.png"),LANG(TOOLBAR_DESC_FOLD_LEVEL_3,""));
		if (config->Toolbars.view.unfold_all) utils->AddTool(toolbar_view,mxID_FOLD_SHOW_ALL,LANG(TOOLBAR_CAPTION_UNFOLD_ALL_LEVELS,"Desplegar Todos los Niveles"),ipre+_T("unfoldAll.png"),LANG(TOOLBAR_DESC_UNFOLD_ALL_LEVELS,""));
		if (config->Toolbars.view.unfold_1) utils->AddTool(toolbar_view,mxID_FOLD_SHOW_1,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_1,"Desplegar el Primer Nivel"),ipre+_T("unfold1.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_1,""));
		if (config->Toolbars.view.unfold_2) utils->AddTool(toolbar_view,mxID_FOLD_SHOW_2,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_2,"Desplegar el Segundo Nivel"),ipre+_T("unfold2.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_2,""));
		if (config->Toolbars.view.unfold_3) utils->AddTool(toolbar_view,mxID_FOLD_SHOW_3,LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_3,"Desplegar el Tercer Nivel"),ipre+_T("unfold3.png"),LANG(TOOLBAR_DESC_UNFOLD_LEVEL_3,""));
		toolbar_view->Realize();
	}
	
	if (!wich_one || wich_one==toolbar_file) {
		if (config->Toolbars.file.new_file) utils->AddTool(toolbar_file,wxID_NEW,LANG(TOOLBAR_CAPTION_FILE_NEW,"Nuevo..."),ipre+_T("nuevo.png"),LANG(TOOLBAR_DESC_FILE_NEW,"Archivo -> Nuevo..."));
		if (config->Toolbars.file.new_project) utils->AddTool(toolbar_file,mxID_FILE_PROJECT,LANG(TOOLBAR_CAPTION_FILE_NEW_PROJECT,"Nuevo Proyecto..."),ipre+_T("proyecto.png"),LANG(TOOLBAR_DESC_FILE_NEW_PROJECT,"Archivo -> Nuevo Proyecto..."));
		if (config->Toolbars.file.open) utils->AddTool(toolbar_file,wxID_OPEN,LANG(TOOLBAR_CAPTION_FILE_OPEN,"Abrir..."),ipre+_T("abrir.png"),LANG(TOOLBAR_DESC_FILE_OPEN,"Archivo -> Abrir..."));
		if (config->Toolbars.file.recent_simple) utils->AddTool(toolbar_file,mxID_FILE_SOURCE_HISTORY_MORE,LANG(TOOLBAR_CAPTION_FILE_RECENT_SOURCES,"Fuentes Recientes..."),ipre+_T("recentSimple.png"),LANG(TOOLBAR_DESC_FILE_RECENT_SOURCES,"Archivo -> Fuentes Abiertos Recientemente..."));
		if (config->Toolbars.file.recent_project) utils->AddTool(toolbar_file,mxID_FILE_PROJECT_HISTORY_MORE,LANG(TOOLBAR_CAPTION_FILE_RECENT_PROJECTS,"Proyectos Recientes..."),ipre+_T("recentProject.png"),LANG(TOOLBAR_DESC_FILE_RECENT_PROJECTS,"Archivo -> Proyectos Abiertos Recientemete..."));
		if (config->Toolbars.file.open_header) utils->AddTool(toolbar_file,mxID_FILE_OPEN_H,LANG(TOOLBAR_CAPTION_FILE_OPEN_H,"Abrir h/cpp Complementario"),ipre+_T("abrirp.png"),LANG(TOOLBAR_DESC_FILE_OPEN_H,"Abrir h/cpp Complementario"));
		if (config->Toolbars.file.open_selected) utils->AddTool(toolbar_file,mxID_FILE_OPEN_SELECTED,LANG(TOOLBAR_CAPTION_FILE_OPEN_SELECTED,"Abrir Seleccionado"),ipre+_T("abrirh.png"),LANG(TOOLBAR_DESC_FILE_OPEN_SELECTED,"Archivo -> Abrir Seleccionado"));
		if (config->Toolbars.file.save) utils->AddTool(toolbar_file,wxID_SAVE,LANG(TOOLBAR_CAPTION_FILE_SAVE,"Guardar"),ipre+_T("guardar.png"),LANG(TOOLBAR_DESC_FILE_SABE,"Archivo -> Guardar"));
		if (config->Toolbars.file.save_as) utils->AddTool(toolbar_file,wxID_SAVEAS,LANG(TOOLBAR_CAPTION_FILE_SAVE_AS,"Guardar Como..."),ipre+_T("guardarComo.png"),LANG(TOOLBAR_DESC_FILE_SAVE_AS,"Archivo -> Guardar Como..."));
		if (config->Toolbars.file.save_all) utils->AddTool(toolbar_file,mxID_FILE_SAVE_ALL,LANG(TOOLBAR_CAPTION_FILE_SAVE_ALL,"Guardar Todo"),ipre+_T("guardarTodo.png"),LANG(TOOLBAR_DESC_FILE_SAVE_ALL,"Archivo -> Guardar Todo"));
		if (config->Toolbars.file.save_project) utils->AddTool(toolbar_file,mxID_FILE_SAVE_PROJECT,LANG(TOOLBAR_CAPTION_FILE_SAVE_PROJECT,"Guardar Proyecto"),ipre+_T("guardarProyecto.png"),LANG(TOOLBAR_DESC_FILE_SAVE_PROJECT,"Archivo -> Guardar Proyecto"));
		if (config->Toolbars.file.export_html) utils->AddTool(toolbar_file,mxID_FILE_EXPORT_HTML,LANG(TOOLBAR_CAPTION_FILE_EXPORT_HTML,"Exportar a HTML..."),ipre+_T("exportHtml.png"),LANG(TOOLBAR_DESC_FILE_EXPORT_HTML,"Archivo -> Exportar a HTML..."));
		if (config->Toolbars.file.print) utils->AddTool(toolbar_file,mxID_FILE_PRINT,LANG(TOOLBAR_CAPTION_FILE_PRINT,"Imprimir..."),ipre+_T("imprimir.png"),LANG(TOOLBAR_DESC_FILE_PRINT,"Archivo -> Imprimir..."));
		if (config->Toolbars.file.reload) utils->AddTool(toolbar_file,mxID_FILE_RELOAD,LANG(TOOLBAR_CAPTION_FILE_RELOAD,"Recargar"),ipre+_T("recargar.png"),LANG(TOOLBAR_DESC_FILE_RELOAD,"Archivo -> Recargar"));
		if (config->Toolbars.file.close) utils->AddTool(toolbar_file,wxID_CLOSE,LANG(TOOLBAR_CAPTION_FILE_CLOSE,"Cerrar"),ipre+_T("cerrar.png"),LANG(TOOLBAR_DESC_FILE_CLOSE,"Archivo -> Cerrar"));
		if (config->Toolbars.file.close_all) utils->AddTool(toolbar_file,mxID_FILE_CLOSE_ALL,LANG(TOOLBAR_CAPTION_FILE_CLOSE_ALL,"Cerrar Todo"),ipre+_T("cerrarTodo.png"),LANG(TOOLBAR_DESC_FILE_CLOSE_ALL,"Archivo -> Cerrar Todo"));
		if (config->Toolbars.file.close_project) utils->AddTool(toolbar_file,mxID_FILE_CLOSE_PROJECT,LANG(TOOLBAR_CAPTION_FILE_CLOSE_PROJECT,"Cerrar Proyecto"),ipre+_T("cerrarProyecto.png"),LANG(TOOLBAR_DESC_FILE_CLOSE_PROJECT,"Archivo -> Cerrar Proyecto"));
		if (config->Toolbars.file.project_config) utils->AddTool(toolbar_file,mxID_FILE_PROJECT_CONFIG,LANG(TOOLBAR_CAPTION_FILE_PROJECT_CONFIG,"Configuracion de proyecto..."),ipre+_T("projectConfig.png"),LANG(TOOLBAR_DESC_FILE_PROJECT_CONFIG,"Archivo -> Configuracion del Proyecto..."));
		toolbar_file->Realize();
	}
	
	if (!wich_one || wich_one==toolbar_misc) {
		if (config->Toolbars.misc.preferences) utils->AddTool(toolbar_misc,mxID_FILE_PREFERENCES,LANG(TOOLBAR_CAPTION_FILE_PREFERENCES,"Preferencias..."),ipre+_T("preferencias.png"),LANG(TOOLBAR_DESC_FILE_PREFERENCES,"Archivo -> Preferencias..."));
		if (config->Toolbars.misc.tutorials) utils->AddTool(toolbar_misc,mxID_HELP_TUTORIAL,LANG(TOOLBAR_CAPTION_HELP_TUTORIALS,"Tutoriales..."),ipre+_T("tutoriales.png"),LANG(TOOLBAR_DESC_HELP_TUTORIALS,"Ayuda -> Tutoriales..."));
		if (config->Toolbars.misc.help_ide) utils->AddTool(toolbar_misc,mxID_HELP_GUI,LANG(TOOLBAR_CAPTION_HELP_ZINJAI,"Ayuda Sobre ZinjaI..."),ipre+_T("ayuda.png"),LANG(TOOLBAR_DESC_HELP_ZINJAI,"Ayuda -> Ayuda sobre ZinjaI..."));
		if (config->Toolbars.misc.help_cpp) utils->AddTool(toolbar_misc,mxID_HELP_CPP,LANG(TOOLBAR_CAPTION_HELP_CPP,"Ayuda Sobre C++..."),ipre+_T("referencia.png"),LANG(TOOLBAR_DESC_HELP_CPP,"Ayuda -> Ayuda sobre C++..."));
		if (config->Toolbars.misc.show_tips) utils->AddTool(toolbar_misc,mxID_HELP_TIP,LANG(TOOLBAR_CAPTION_HELP_TIPS,"Sugerencias..."),ipre+_T("tip.png"),LANG(TOOLBAR_DESC_HELP_TIPS,"Ayuda -> Mostrar Sugerencias..."));
		if (config->Toolbars.misc.about) utils->AddTool(toolbar_misc,mxID_HELP_ABOUT,LANG(TOOLBAR_CAPTION_HELP_ABOUT,"Acerca de..."),ipre+_T("acercaDe.png"),LANG(TOOLBAR_DESC_HELP_ABOUT,"Ayuda -> Acerca de..."));
		if (config->Toolbars.misc.opinion) utils->AddTool(toolbar_misc,mxID_HELP_OPINION,LANG(TOOLBAR_CAPTION_HELP_TUTORIALS,"Opina sobre ZinjaI..."),ipre+_T("opinion.png"),LANG(TOOLBAR_DESC_HELP_OPINION,"Ayuda -> Opina sobre ZinjaI..."));
		if (config->Toolbars.misc.find_updates) utils->AddTool(toolbar_misc,mxID_HELP_UPDATES,LANG(TOOLBAR_CAPTION_HELP_UPDATES,"Buscar Actualizaciones..."),ipre+_T("updates.png"),LANG(TOOLBAR_DESC_HELP_UPDATES,"Ayuda -> Buscar Actualizaciones..."));
		if (config->Toolbars.misc.exit) utils->AddTool(toolbar_misc,wxID_EXIT,LANG(TOOLBAR_CAPTION_FILE_EXIT,"Salir"),ipre+_T("salir.png"),LANG(TOOLBAR_DESC_FILE_EXIT,"Archivo -> Salir"));
		toolbar_misc->Realize();
	}
	
	if (!wich_one || wich_one==toolbar_edit) {
		
		if (config->Toolbars.edit.undo) utils->AddTool(toolbar_edit,wxID_UNDO,LANG(TOOLBAR_CAPTION_EDIT_UNDO,"Deshacer"),ipre+_T("deshacer.png"),LANG(TOOLBAR_DESC_EDIT_UNDO,"Editar -> Deshacer"));
		if (config->Toolbars.edit.redo) utils->AddTool(toolbar_edit,wxID_REDO,LANG(TOOLBAR_CAPTION_EDIT_REDO,"Rehacer"),ipre+_T("rehacer.png"),LANG(TOOLBAR_DESC_EDIT_REDO,"Editar -> Rehacer"));
		if (config->Toolbars.edit.copy) utils->AddTool(toolbar_edit,wxID_COPY,LANG(TOOLBAR_CAPTION_EDIT_COPY,"Copiar"),ipre+_T("copiar.png"),LANG(TOOLBAR_DESC_EDIT_COPY,"Editar -> Copiar"));
		if (config->Toolbars.edit.cut) utils->AddTool(toolbar_edit,wxID_CUT,LANG(TOOLBAR_CAPTION_EDIT_CUT,"Cortar"),ipre+_T("cortar.png"),LANG(TOOLBAR_DESC_EDIT_CUT,"Editar -> Cortar"));
		if (config->Toolbars.edit.paste) utils->AddTool(toolbar_edit,wxID_PASTE,LANG(TOOLBAR_CAPTION_EDIT_PASTE,"Pegar"),ipre+_T("pegar.png"),LANG(TOOLBAR_DESC_EDIT_PASTE,"Editar -> Pegar"));
		if (config->Toolbars.edit.move_up) utils->AddTool(toolbar_edit,mxID_EDIT_TOGGLE_LINES_UP,LANG(TOOLBAR_CAPTION_EDIT_LINES_UP,"Mover Hacia Arriba"),ipre+_T("toggleLinesUp.png"),LANG(TOOLBAR_DESC_EDIT_LINES_UP,"Editar -> Mover Hacia Arriba"));
		if (config->Toolbars.edit.move_down) utils->AddTool(toolbar_edit,mxID_EDIT_TOGGLE_LINES_DOWN,LANG(TOOLBAR_CAPTION_EDIT_LINES_DOWN,"Mover Hacia Abajo"),ipre+_T("toggleLinesDown.png"),LANG(TOOLBAR_DESC_EDIT_LINES_DOWN,"Editar -> Mover Hacia Abajo"));
		if (config->Toolbars.edit.duplicate) utils->AddTool(toolbar_edit,mxID_EDIT_DUPLICATE_LINES,LANG(TOOLBAR_CAPTION_EDIT_DUPLICATE_LINES,"Duplicar Linea(s)"),ipre+_T("duplicarLineas.png"),LANG(TOOLBAR_DESC_EDIT_DUPLICATE_LINES,"Editar -> Duplicar Linea(s)"));
		if (config->Toolbars.edit.delete_lines) utils->AddTool(toolbar_edit,mxID_EDIT_DELETE_LINES,LANG(TOOLBAR_CAPTION_EDIT_DELETE_LINES,"Eliminar Linea(s)"),ipre+_T("borrarLineas.png"),LANG(TOOLBAR_DESC_EDIT_DELETE_LINES,"Editar -> Eliminar Linea(s)"));
	//	toolbar_edit->AddSeparator();
		if (config->Toolbars.edit.goto_line) utils->AddTool(toolbar_edit,mxID_EDIT_GOTO,LANG(TOOLBAR_CAPTION_EDIT_GOTO_LINE,"Ir a Linea..."),ipre+_T("irALinea.png"),LANG(TOOLBAR_DESC_EDIT_GOTO_LINE,"Editar -> Ir a linea..."));
		if (config->Toolbars.edit.goto_class) utils->AddTool(toolbar_edit,mxID_EDIT_GOTO_FUNCTION,LANG(TOOLBAR_CAPTION_EDIT_GOTO_FUNCTION,"Ir a Clase/Metodo/Funcion..."),ipre+_T("irAFuncion.png"),LANG(TOOLBAR_DESC_EDIT_GOTO_FUNCTION,"Editar -> Ir a Clase/Metodo/Funcion..."));
		if (config->Toolbars.edit.goto_file) utils->AddTool(toolbar_edit,mxID_EDIT_GOTO_FILE,LANG(TOOLBAR_CAPTION_EDIT_GOTO_FILE,"Ir a Archivo..."),ipre+_T("irAArchivo.png"),LANG(TOOLBAR_DESC_EDIT_GOTO_FILE,"Editar -> Ir a Archivo..."));
		if (config->Toolbars.edit.find) utils->AddTool(toolbar_edit,mxID_EDIT_FIND,LANG(TOOLBAR_CAPTION_EDIT_FIND,"Buscar..."),ipre+_T("buscar.png"),LANG(TOOLBAR_DESC_EDIT_FIND,"Editar -> Buscar..."));
		if (config->Toolbars.edit.find_prev) utils->AddTool(toolbar_edit,mxID_EDIT_FIND_PREV,LANG(TOOLBAR_CAPTION_EDIT_FIND_PREV,"Buscar anterior"),ipre+_T("buscarAnterior.png"),LANG(TOOLBAR_DESC_EDIT_FIND_PREV,"Editar -> Buscar Anterior"));
		if (config->Toolbars.edit.find_next) utils->AddTool(toolbar_edit,mxID_EDIT_FIND_NEXT,LANG(TOOLBAR_CAPTION_EDIT_FIND_NEXT,"Buscar siguiente"),ipre+_T("buscarSiguiente.png"),LANG(TOOLBAR_DESC_EDIT_FIND_NEXT,"Editar -> Buscar Siguiente"));
		if (config->Toolbars.edit.replace) utils->AddTool(toolbar_edit,mxID_EDIT_REPLACE,LANG(TOOLBAR_CAPTION_EDIT_REPLACE,"Reemplazar..."),ipre+_T("reemplazar.png"),LANG(TOOLBAR_DESC_EDIT_REPLACE,"Editar -> Reemplazar..."));
		
		if (config->Toolbars.edit.inser_header) utils->AddTool(toolbar_edit,mxID_EDIT_INSERT_HEADER,LANG(TOOLBAR_CAPTION_EDIT_INSERT_INCLUDE,"Insertar #include Correspondiente"),ipre+_T("insertarInclude.png"),LANG(TOOLBAR_DESC_EDIT_INSERT_INCLUDE,"Editar -> Insertar #include Correspondiente"));
		if (config->Toolbars.edit.comment) utils->AddTool(toolbar_edit,mxID_EDIT_COMMENT,LANG(TOOLBAR_CAPTION_EDIT_COMMENT,"Comentar"),ipre+_T("comentar.png"),LANG(TOOLBAR_DESC_EDIT_COMMNENT,"Editar -> Comentar"));
		if (config->Toolbars.edit.uncomment) utils->AddTool(toolbar_edit,mxID_EDIT_UNCOMMENT,LANG(TOOLBAR_CAPTION_EDIT_UNCOMMENT,"Descomentar"),ipre+_T("descomentar.png"),LANG(TOOLBAR_DESC_EDIT_UNCOMMENT,"Editar -> Descomentar"));
		if (config->Toolbars.edit.indent) utils->AddTool(toolbar_edit,mxID_EDIT_INDENT,LANG(TOOLBAR_CAPTION_EDIT_INDENT,"Indentar"),ipre+_T("indent.png"),LANG(TOOLBAR_DESC_EDIT_INDENT,"Editar -> Indentar"));
		if (config->Toolbars.edit.select_block) utils->AddTool(toolbar_edit,mxID_EDIT_BRACEMATCH,LANG(TOOLBAR_CAPTION_EDIT_BRACEMATCH,"Seleccionar Bloque"),ipre+_T("mostrarLlave.png"),LANG(TOOLBAR_DESC_EDIT_BRACEMATCH,"Editar -> Seleccionar Bloque"));
		if (config->Toolbars.edit.select_all) utils->AddTool(toolbar_edit,wxID_SELECTALL,LANG(TOOLBAR_CAPTION_EDIT_SELECT_ALL,"Seleccionar Todo"),ipre+_T("seleccionarTodo.png"),LANG(TOOLBAR_DESC_EDIT_SELECT_ALL,"Editar -> Seleccionar Todo"));
		if (config->Toolbars.edit.toggle_user_mark) utils->AddTool(toolbar_edit,mxID_EDIT_MARK_LINES,LANG(TOOLBAR_CAPTION_EDIT_HIGHLIGHT_LINES,"Resaltar Linea(s)/Quitar Resaltado"),ipre+_T("marcar.png"),LANG(TOOLBAR_DESC_EDIT_HIGHLIGHT_LINES,"Editar -> Resaltar Linea(s)/Quitar Resaltado"));
		if (config->Toolbars.edit.find_user_mark) utils->AddTool(toolbar_edit,mxID_EDIT_GOTO_MARK,LANG(TOOLBAR_CAPTION_EDIT_FIND_HIGHLIGHTS,"Buscar Resaltado"),ipre+_T("irAMarca.png"),LANG(TOOLBAR_DESC_EDIT_FIND_HIGHLIGHTS,"Editar -> Buscar Resaltado"));
		if (config->Toolbars.edit.list_user_marks) utils->AddTool(toolbar_edit,mxID_EDIT_LIST_MARKS,LANG(TOOLBAR_CAPTION_EDIT_LIST_HIGHLIGHTS,"Listar Lineas Resaltadas"),ipre+_T("listarMarcas.png"),LANG(TOOLBAR_DESC_EDIT_LIST_HIGHLIGHTS,"Editar -> Listar Lineas Resaltadas"));
		if (config->Toolbars.edit.autocomplete) utils->AddTool(toolbar_edit,mxID_EDIT_FORCE_AUTOCOMPLETE,LANG(TOOLBAR_CAPTION_EDIT_FORCE_AUTOCOMPLETE,"Autocompletar"),ipre+_T("autocompletar.png"),LANG(TOOLBAR_DESC_EDIT_FORCE_AUTOCOMPLETE,"Editar -> Autocompletar"));
		
		toolbar_edit->Realize();
	}
	
	if (!wich_one || wich_one==toolbar_debug) {
		if (config->Toolbars.debug.start) utils->AddTool(toolbar_debug,mxID_DEBUG_RUN,LANG(TOOLBAR_CAPTION_DEBUG_START,"Comenzar/Continuar Depuracion"),ipre+_T("ejecutar.png"),LANG(TOOLBAR_DESC_DEBUG_START,"Depurar -> Comenzar/Continuar"));
		if (config->Toolbars.debug.pause) utils->AddTool(toolbar_debug,mxID_DEBUG_PAUSE,LANG(TOOLBAR_CAPTION_DEBUG_PAUSE,"Interrumpir Ejecucion en Depuracion"),ipre+_T("pausar.png"),LANG(TOOLBAR_DESC_DEBUG_PAUSE,"Depurar -> Interrumpir"));
		if (config->Toolbars.debug.stop) utils->AddTool(toolbar_debug,mxID_DEBUG_STOP,LANG(TOOLBAR_CAPTION_DEBUG_STOP,"Detener Depuracion"),ipre+_T("detener.png"),LANG(TOOLBAR_DESC_DEBUG_STOP,"Depurar -> Detener"));
		if (config->Toolbars.debug.step_in) utils->AddTool(toolbar_debug,mxID_DEBUG_STEP_IN,LANG(TOOLBAR_CAPTION_DEBUG_STEP_IP,"Step In"),ipre+_T("step_in.png"),LANG(TOOLBAR_DESC_DEBUG_STEP_IN,"Depurar -> Step In"));
		if (config->Toolbars.debug.step_over) utils->AddTool(toolbar_debug,mxID_DEBUG_STEP_OVER,LANG(TOOLBAR_CAPTION_DEBUG_STEP_OVER,"Step Over"),ipre+_T("step_over.png"),LANG(TOOLBAR_DESC_DEBUG_STEP_OVER,"Depurar -> Step Over"));
		if (config->Toolbars.debug.step_out) utils->AddTool(toolbar_debug,mxID_DEBUG_STEP_OUT,LANG(TOOLBAR_CAPTION_DEBUG_STEP_OUT,"Step Out"),ipre+_T("step_out.png"),LANG(TOOLBAR_DESC_DEBUG_STEP_OUT,"Depurar -> Step Out"));
		if (config->Toolbars.debug.run_until) utils->AddTool(toolbar_debug,mxID_DEBUG_RUN_UNTIL,LANG(TOOLBAR_CAPTION_DEBUG_RUN_UNTIL,"Ejecutar Hasta El Cursor"),ipre+_T("run_until.png"),LANG(TOOLBAR_DESC_DEBUG_RUN_UNTIL,"Depurar -> Ejecutar Hasta el Cursos"));
		if (config->Toolbars.debug.function_return) utils->AddTool(toolbar_debug,mxID_DEBUG_RETURN,LANG(TOOLBAR_CAPTION_DEBUG_RETURN,"Return"),ipre+_T("return.png"),LANG(TOOLBAR_DESC_DEBUG_RETURN,"Depurar -> Return"));
		if (config->Toolbars.debug.jump) utils->AddTool(toolbar_debug,mxID_DEBUG_JUMP,LANG(TOOLBAR_CAPTION_DEBUG_JUMP,"Continuar desde aqui"),ipre+_T("debug_jump.png"),LANG(TOOLBAR_DESC_DEBUG_JUMP,"Depurar -> Continuar Desde Aqui"));
#if !defined(_WIN32) && !defined(__WIN32__)
		if (config->Toolbars.debug.enable_inverse_exec)  utils->AddTool(toolbar_debug,mxID_DEBUG_ENABLE_INVERSE_EXEC,LANG(TOOLBAR_CAPTION_DEBUG_ENABLE_INVERSE,"Habilitar Ejecucion Hacia Atras"),ipre+_T("reverse_enable.png"),LANG(TOOLBAR_DESC_DEBUG_ENABLE_INVERSE,"Depurar -> Habilitar Ejecucion Hacia Atras"),wxITEM_CHECK);
		if (config->Toolbars.debug.inverse_exec)  utils->AddTool(toolbar_debug,mxID_DEBUG_INVERSE_EXEC,LANG(TOOLBAR_CAPTION_DEBUG_INVERSE,"Ejecutar Hacia Atras"),ipre+_T("reverse_toggle.png"),LANG(TOOLBAR_DESC_DEBUG_INVERSE,"Depurar -> Ejecutar Hacia Atras..."),wxITEM_CHECK);
#endif
		if (config->Toolbars.debug.inspections) utils->AddTool(toolbar_debug,mxID_DEBUG_INSPECT,LANG(TOOLBAR_CAPTION_INSPECT,"Panel de Inspecciones"),ipre+_T("inspect.png"),LANG(TOOLBAR_DESC_INSPECT,"Depurar -> Panel de Inspecciones"));
		if (config->Toolbars.debug.backtrace) utils->AddTool(toolbar_debug,mxID_DEBUG_BACKTRACE,LANG(TOOLBAR_CAPTION_BACKTRACE,"Trazado Inverso"),ipre+_T("backtrace.png"),LANG(TOOLBAR_DESC_BACKTRACE,"Depurar -> Trazado Inverso"));
		if (config->Toolbars.debug.threadlist) utils->AddTool(toolbar_debug,mxID_DEBUG_THREADLIST,LANG(TOOLBAR_CAPTION_THREADLIST,"Hilos de Ejecucion"),ipre+_T("threadlist.png"),LANG(TOOLBAR_DESC_THREADLIST,"Depurar -> Hilos de Ejecucion"));
		
		if (config->Toolbars.debug.break_toggle) utils->AddTool(toolbar_debug,mxID_DEBUG_BREAKPOINT,LANG(TOOLBAR_CAPTION_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpointo"),ipre+_T("breakpoint.png"),LANG(TOOLBAR_DESC_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpoint"));
		if (config->Toolbars.debug.break_options) utils->AddTool(toolbar_debug,mxID_DEBUG_BREAKPOINT_OPTIONS,LANG(TOOLBAR_CAPTION_DEBUG_BREAKPOINT_OPTIONS,"Opciones del Breakpoint..."),ipre+_T("breakpoint_options.png"),LANG(TOOLBAR_DESC_DEBUG_BREAKPOINT_OPTIONS,"Depurar -> Opciones del Breakpoint..."));
		if (config->Toolbars.debug.break_list) utils->AddTool(toolbar_debug,mxID_DEBUG_LIST_BREAKPOINTS,LANG(TOOLBAR_CAPTION_DEBUG_LIST_BREAKPOINTS,"Listar Watch/Break points..."),ipre+_T("breakpoint_list.png"),LANG(TOOLBAR_DESC_DEBUG_LIST_BREAKPOINTS,"Depurar -> Listar Watch/Break points..."));
		if (config->Toolbars.debug.log_panel)  utils->AddTool(toolbar_debug,mxID_DEBUG_LOG_PANEL,LANG(TOOLBAR_CAPTION_DEBUG_SHOW_LOG_PANEL,"Mostrar Mensajes del Depurador"),ipre+_T("debug_log_panel.png"),LANG(TOOLBAR_DESC_DEBUG_SHOW_LOG_PANEL,"Depurar -> Mostrar Mensajes del Depurador..."));
		
		toolbar_debug->Realize();
	}
	
	if (!wich_one || wich_one==toolbar_run) {
		if (config->Toolbars.run.compile) utils->AddTool(toolbar_run,mxID_RUN_COMPILE,LANG(TOOLBAR_CAPTION_RUN_COMPILE,"Compilar"),ipre+_T("compilar.png"),LANG(TOOLBAR_DESC_RUN_COMPILE,"Ejecucion -> Compilar"));
		if (config->Toolbars.run.run) utils->AddTool(toolbar_run,mxID_RUN_RUN,LANG(TOOLBAR_CAPTION_RUN_RUN,"Guardar, Compilar y Ejecutar..."),ipre+_T("ejecutar.png"),LANG(TOOLBAR_DESC_RUN_RUN,"Ejecucion -> Guardar, Compilar y Ejecutar"));
		if (config->Toolbars.run.run_old) utils->AddTool(toolbar_run,mxID_RUN_RUN_OLD,LANG(TOOLBAR_CAPTION_RUN_OLD,"Ejecutar Sin Recompilar..."),ipre+_T("ejecutar_old.png"),LANG(TOOLBAR_DESC_RUN_OLD,"Ejecucion -> Ejecutar Sin Recompilar"));
		if (config->Toolbars.run.stop) utils->AddTool(toolbar_run,mxID_RUN_STOP,LANG(TOOLBAR_CAPTION_RUN_STOP,"Detener"),ipre+_T("detener.png"),LANG(TOOLBAR_DESC_RUN_STOP,"Ejecucion -> Detener"));
		if (config->Toolbars.run.clean) utils->AddTool(toolbar_run,mxID_RUN_CLEAN,LANG(TOOLBAR_CAPTION_RUN_CLEAN,"Limpiar"),ipre+_T("limpiar.png"),LANG(TOOLBAR_DESC_RUN_CLEAN,"Ejecucion -> Limpiar"));
		if (config->Toolbars.run.options) utils->AddTool(toolbar_run,mxID_RUN_CONFIG,LANG(TOOLBAR_CAPTION_RUN_OPTIONS,"Opciones de compilacion y ejecucion..."),ipre+_T("opciones.png"),LANG(TOOLBAR_DESC_RUN_OPTIONS,"Ejecucion -> Opciones..."));
		if (config->Toolbars.run.debug) utils->AddTool(toolbar_run,mxID_DEBUG_RUN,LANG(TOOLBAR_CAPTION_DEBUG_RUN,"Depurar"),ipre+_T("depurar.png"),LANG(TOOLBAR_DESC_DEBUG_RUN,"Depurar -> Iniciar"));
#if !defined(__WIN32__)		
		if (config->Toolbars.run.debug_attach) utils->AddTool(toolbar_run,mxID_DEBUG_ATTACH,LANG(TOOLBAR_CAPTION_DEBUG_ATTACH,"Attachear Depurador"),ipre+_T("debug_attach.png"),LANG(TOOLBAR_DESC_DEBUG_ATTACH,"Depurar -> Adjuntar"));
		if (config->Toolbars.run.load_core_dump) utils->AddTool(toolbar_run,mxID_DEBUG_CORE_DUMP,LANG(TOOLBAR_CAPTION_DEBUG_CORE_DUMP,"Cargar Volcado de Memoria"),ipre+_T("core_dump.png"),LANG(TOOLBAR_DESC_DEBUG_CORE_DUMP,"Depurar -> Cargar Volcado de Memoria"));
#endif
		if (config->Toolbars.run.break_toggle) utils->AddTool(toolbar_run,mxID_DEBUG_BREAKPOINT,LANG(TOOLBAR_CAPTION_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpointo"),ipre+_T("breakpoint.png"),LANG(TOOLBAR_DESC_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpoint"));
		if (config->Toolbars.run.break_options) utils->AddTool(toolbar_run,mxID_DEBUG_BREAKPOINT_OPTIONS,LANG(TOOLBAR_CAPTION_DEBUG_BREAKPOINT_OPTIONS,"Opciones del Breakpoint..."),ipre+_T("breakpoint_options.png"),LANG(TOOLBAR_DESC_DEBUG_BREAKPOINT_OPTIONS,"Depurar -> Opciones del Breakpoint..."));
		if (config->Toolbars.run.break_list) utils->AddTool(toolbar_run,mxID_DEBUG_LIST_BREAKPOINTS,LANG(TOOLBAR_CAPTION_DEBUG_LIST_BREAKPOINTS,"Listar Watch/Break points..."),ipre+_T("breakpoint_list.png"),LANG(TOOLBAR_DESC_DEBUG_LIST_BREAKPOINTS,"Depurar -> Listar Watch/Break points..."));
		if (config->Toolbars.run.inspections) utils->AddTool(toolbar_run,mxID_DEBUG_INSPECT,LANG(TOOLBAR_CAPTION_INSPECT,"Panel de Inspecciones"),ipre+_T("inspect.png"),LANG(TOOLBAR_DESC_INSPECT,"Depurar -> Panel de Inspecciones"));
		toolbar_run->Realize();
	}
	
	if (wich_one) {
		aui_manager.GetPane(wich_one).BestSize(wich_one->GetBestSize());
		aui_manager.Update();
	} else {
		toolbar_find = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_find->SetToolBitmapSize(wxSize(ICON_SIZE,ICON_SIZE));
		toolbar_find_text = new wxTextCtrl(toolbar_find,mxID_TOOLBAR_FIND,wxEmptyString,wxDefaultPosition,wxSize(100,20),wxTE_PROCESS_ENTER);
		toolbar_find->AddControl( toolbar_find_text );
		toolbar_find_text->SetToolTip(LANG(TOOLBAR_FIND_TEXT,"Texto a Buscar"));
		utils->AddTool(toolbar_find,mxID_EDIT_TOOLBAR_FIND,LANG(TOOLBAR_FIND_CAPTION,"Busqueda Rapida"),ipre+_T("buscar.png"),LANG(TOOLBAR_FIND_BUTTON,"Buscar siguiente"));
		toolbar_find->Realize();

		toolbar_status = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER);
		toolbar_status->SetToolBitmapSize(wxSize(ICON_SIZE,ICON_SIZE));
		toolbar_status->AddControl( toolbar_status_text = new wxStaticText(toolbar_status,wxID_ANY,_T(""),wxDefaultPosition,wxSize(2500,20)) );
		toolbar_status_text->SetForegroundColour(wxColour(_T("Z DARK BLUE")));
		toolbar_status->Realize();
	}
	
}

void mxMainWindow::OnExit(wxCommandEvent &event) {
    Close(true);
}


mxMainWindow::~mxMainWindow() {
	main_window=NULL;
    aui_manager.UnInit();
}


void mxMainWindow::OnEdit (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		CURRENT_SOURCE->ProcessEvent(event);
		CURRENT_SOURCE->SetFocus();
	}
}

void mxMainWindow::OnEditNeedFocus (wxCommandEvent &event) {
	wxWindow *focus = main_window->FindFocus();
	if (focus && focus->IsKindOf(toolbar_find_text->GetClassInfo()))
		focus->ProcessEvent(event);
	else IF_THERE_IS_SOURCE 
		CURRENT_SOURCE->ProcessEvent(event);
}


wxHtmlWindow* mxMainWindow::CreateQuickHelp(wxWindow* parent) {
    quick_help = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(400,300));
    quick_help->SetPage(LANG(MAINW_QUICKHELP_INIT,"Coloca el cursor de texto sobre una palabra y presona Shift+F1 para ver la ayuda en este cuadro."));
	
	if (config->Init.autohiding_panels) {
		autohide_handlers[ATH_QUICKHELP] = new mxHidenPanel(this,quick_help,1,LANG(MAINW_AUTOHIDE_QUICKHELP,"Ayuda/Busqueda"));
	}
	
    return quick_help;
}

void mxMainWindow::OnNotebookPanelsChanged(wxAuiNotebookEvent& event) {
	wxWindow *c = left_panels->GetPage(left_panels->GetSelection());
	if (!c) return; 
	else if (c==explorer_tree.treeCtrl) aui_manager.GetPane(left_panels).Caption(LANG(CAPTION_EXPLORER_TREE,"Arbol de Archivos"));
	else if (c==symbols_tree.treeCtrl) aui_manager.GetPane(left_panels).Caption(LANG(CAPTION_PROJECT_SYMBOLS_TREE,"Arbol de Simbolos"));
	else if (c==project_tree.treeCtrl) aui_manager.GetPane(left_panels).Caption(LANG(CAPTION_PROJECT_PROJECT_TREE,"Arbol de Proyecto"));
	aui_manager.Update();
}

wxAuiNotebook *mxMainWindow::CreateLeftPanels() {
	explorer_tree.treeCtrl=symbols_tree.treeCtrl=project_tree.treeCtrl=NULL;
//	wxSize client_size = GetClientSize();
	left_panels = new wxAuiNotebook(this, mxID_LEFT_PANELS, wxDefaultPosition, wxSize(200,400), wxAUI_NB_BOTTOM | wxNO_BORDER);
	return left_panels;
}

wxAuiNotebook *mxMainWindow::CreateNotebookSources() {
//	wxSize client_size = GetClientSize();
	notebook_sources = new wxAuiNotebook(this, mxID_NOTEBOOK_SOURCES, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER | wxAUI_NB_WINDOWLIST_BUTTON);
//	wxBitmap page_bmp = wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16));
	return notebook_sources;
}

wxTreeCtrl* mxMainWindow::CreateExplorerTree() {

	explorer_tree.treeCtrl = new mxTreeCtrl(this, mxID_TREE_EXPLORER, wxDefaultPosition, wxSize(160,100), wxTR_DEFAULT_STYLE | wxNO_BORDER /*| wxTR_HIDE_ROOT*/);
					
	wxImageList* imglist = new wxImageList(16, 16,true,5);
	
	imglist->Add(wxBitmap(SKIN_FILE(_T("ap_folder.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(*(bitmaps->files.source));
	imglist->Add(*(bitmaps->files.header));
	imglist->Add(*(bitmaps->files.other));
	imglist->Add(*(bitmaps->files.blank));
	imglist->Add(wxBitmap(SKIN_FILE(_T("ap_zpr.png")),wxBITMAP_TYPE_PNG));
	explorer_tree.treeCtrl->AssignImageList(imglist);
	
	explorer_tree.show_only_sources = false;
	
	if (config->Init.autohiding_panels) {
		autohide_handlers[ATH_EXPLORER] = new mxHidenPanel(this,explorer_tree.treeCtrl,0,LANG(MAINW_AUTOHIDE_EXPLORER,"Explorador"));
	}
	
	return explorer_tree.treeCtrl;
}

wxTreeCtrl* mxMainWindow::CreateProjectTree() {

	project_tree.treeCtrl = new mxTreeCtrl(this, mxID_TREE_PROJECT, wxPoint(0,0), wxSize(160,100), wxTR_DEFAULT_STYLE | wxNO_BORDER | wxTR_HIDE_ROOT);
					
	wxImageList* imglist = new wxImageList(16, 16,true,5);
	
	imglist->Add(wxBitmap(SKIN_FILE(_T("ap_folder.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(*(bitmaps->files.source));
	imglist->Add(*(bitmaps->files.header));
	imglist->Add(*(bitmaps->files.other));
	imglist->Add(*(bitmaps->files.blank));
	imglist->Add(wxBitmap(SKIN_FILE(_T("ap_wxfb.png")),wxBITMAP_TYPE_PNG));
	project_tree.treeCtrl->AssignImageList(imglist);
	
	project_tree.root = project_tree.treeCtrl->AddRoot(_T("Archivos Abiertos"), 0);
	wxArrayTreeItemIds items;
	project_tree.sources = project_tree.treeCtrl->AppendItem(project_tree.root, LANG(MAINW_PT_SOURCES,"Fuentes"), 0);
	project_tree.headers = project_tree.treeCtrl->AppendItem(project_tree.root, LANG(MAINW_PT_HEADERS,"Cabeceras"), 0);
	project_tree.others = project_tree.treeCtrl->AppendItem(project_tree.root, LANG(MAINW_PT_OTHERS,"Otros"), 0);
	items.Add(project_tree.sources);
	items.Add(project_tree.headers);
	items.Add(project_tree.others);

	project_tree.treeCtrl->ExpandAll();
	
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_PROJECT] = new mxHidenPanel(this,project_tree.treeCtrl,0,LANG(MAINW_AUTOHIDE_PROJECT,"Proyecto"));
	
	return project_tree.treeCtrl;
}

wxTreeCtrl* mxMainWindow::CreateSymbolsTree() {
	symbols_tree.treeCtrl = new wxTreeCtrl(this, mxID_TREE_SYMBOLS, wxPoint(0,0), wxSize(160,100), wxTR_DEFAULT_STYLE | wxNO_BORDER | wxTR_HIDE_ROOT);
	wxImageList* imglist = new wxImageList(16, 16, true, 15);
	imglist->Add(wxBitmap(SKIN_FILE(_T("as_folder.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16)));
	
	imglist->Add(*(bitmaps->parser.icon2));
	imglist->Add(*(bitmaps->parser.icon3));
	imglist->Add(*(bitmaps->parser.icon4));
	imglist->Add(*(bitmaps->parser.icon5));
	imglist->Add(*(bitmaps->parser.icon6));
	imglist->Add(*(bitmaps->parser.icon7));
	imglist->Add(*(bitmaps->parser.icon8));
	imglist->Add(*(bitmaps->parser.icon9));
	imglist->Add(*(bitmaps->parser.icon10));
	imglist->Add(*(bitmaps->parser.icon11));
	imglist->Add(*(bitmaps->parser.icon12));
	imglist->Add(*(bitmaps->parser.icon13));
	imglist->Add(*(bitmaps->parser.icon14));
	imglist->Add(*(bitmaps->parser.icon18));
	
	symbols_tree.treeCtrl->AssignImageList(imglist);
//	symbols_tree.treeCtrl->AddRoot(_T("Simbolos encontrados"), 0);
	
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_SYMBOL] = new mxHidenPanel(this,symbols_tree.treeCtrl,0,LANG(MAINW_AUTOHIDE_SYMBOLS,"Simbolos"));
	
	return symbols_tree.treeCtrl;
}


wxTreeCtrl* mxMainWindow::CreateCompilerTree() {
	compiler_tree.treeCtrl = new wxTreeCtrl(this, mxID_TREE_COMPILER, wxPoint(0,0), wxSize(160,250), wxTR_DEFAULT_STYLE | wxNO_BORDER | wxTR_HIDE_ROOT);
// 	wxFont tree_font=compiler_tree.treeCtrl->GetFont();
// 	tree_font.SetFaceName("courier");
// 	compiler_tree.treeCtrl->SetFont(tree_font);
	
	wxImageList* imglist = new wxImageList(16, 16, true, 2);
	imglist->Add(wxBitmap(SKIN_FILE(_T("co_folder.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16)));
	imglist->Add(wxBitmap(SKIN_FILE(_T("co_info.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("co_warning.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("co_error.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("co_err_info.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("co_out.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("co_project_warning.png")),wxBITMAP_TYPE_PNG));
	compiler_tree.treeCtrl->AssignImageList(imglist);
	
	compiler_tree.root = compiler_tree.treeCtrl->AddRoot(_T("Resultados de la Compilacion:"), 0);
	wxArrayTreeItemIds items;
	compiler_tree.state = compiler_tree.treeCtrl->AppendItem(compiler_tree.root, _T("ZinjaI"),2);
	compiler_tree.errors = compiler_tree.treeCtrl->AppendItem(compiler_tree.root, LANG(MAINW_CT_ERRORS,"Errores"), 0);
	compiler_tree.warnings = compiler_tree.treeCtrl->AppendItem(compiler_tree.root, LANG(MAINW_CT_WARNINGS,"Advertencias"), 0);
	compiler_tree.all = compiler_tree.treeCtrl->AppendItem(compiler_tree.root, LANG(MAINW_CT_ALL,"Toda la salida"), 0);
	items.Add(compiler_tree.state);
	items.Add(compiler_tree.errors);
	items.Add(compiler_tree.warnings);
	items.Add(compiler_tree.all);
	
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_COMPILER] = new mxHidenPanel(this,compiler_tree.treeCtrl,1,LANG(MAINW_AUTOHIDE_COMPILER,"Compilador"));

	return compiler_tree.treeCtrl;
}

void mxMainWindow::OnProcessTerminate (wxProcessEvent& event) {
	if (event.GetPid()==debug->pid) {
		debug->ProcessKilled();
		return;
#if !defined(_WIN32) && !defined(__WIN32__)
	} else if (event.GetPid()==debug->tty_pid) {
		debug->TtyProcessKilled();
		return;
#endif
	}
	compiler->timer->Stop();
	menu.run_stop->Enable(false);
	menu.run_run->Enable(true);
	menu.run_compile->Enable(true);
	if (project) {
		menu.run_clean->Enable(true);
		menu.tools_makefile->Enable(true);
	}
	
	// ver si es uno de los procesos que se estan esperando
	compile_and_run_struct_single *compile_and_run=compiler->compile_and_run_single;
	while (compile_and_run && compile_and_run->pid!=event.GetPid()) 
		compile_and_run=compile_and_run->next;
	// si es uno interrumpido adrede, liberar memoria y no hacer nada mas
	if (compile_and_run && compile_and_run->killed) { 
		delete compile_and_run; return;
	}
	if (!compile_and_run) { // esto no deberia ocurrir
#ifdef DEBUG
		wxMessageBox(wxString()<<event.GetPid());
#endif
		cerr<<"Warning: Unknown process id: "<<event.GetPid()<<endl;
		return;
	}
	// actualizar el compiler_tree
	if (compile_and_run->compiling) { // si termino la compilacion
		compiler->ParseCompilerOutput(compile_and_run,event.GetExitCode()==0);
	} else { // si termino la ejecucion
		status_bar->SetStatusText(wxString(LANG(GENERAL_READY,"Listo")));
		compiler_tree.treeCtrl->SetItemText(compiler_tree.state,LANG(MAINW_STATUS_RUN_FINISHED,"Ejecucion Finalizada"));
		if (compile_and_run->valgrind_cmd.Len()) ShowValgrindPanel('v',DIR_PLUS_FILE(config->temp_dir,_T("valgrind.out")));
		delete compile_and_run->process;
		delete compile_and_run;
	}
}

void mxMainWindow::OnRunExportMakefile (wxCommandEvent &event) {
	new mxMakefileDialog(this);
}

void mxMainWindow::OnRunClean (wxCommandEvent &event) {
	ABORT_IF_PARSING;
	if (project) project->Clean();
	else IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		if (src->sin_titulo) return;
		wxRemoveFile(src->binary_filename.GetFullPath());
	}
}


void mxMainWindow::OnRunBuild (wxCommandEvent &event) {
	compiler->BuildOrRunProject(false,false,false);
}


void mxMainWindow::StartExecutionStuff (bool compile, bool run, compile_and_run_struct_single *compile_and_run, wxString msg) {
	// setear banderas
	compile_and_run->compiling=compile;
	compile_and_run->run_after_compile=(run&&compile);
//	compile_and_run.linking=(what==mES_LINK || what==mES_LINK_AND_RUN);
	// ver si comenzo correctamente
	if (compile_and_run->pid==0) {
		wxBell();
		if (compile_and_run->compiling)
			compiler_tree.treeCtrl->SetItemText(compiler_tree.state,LANG(MAINW_COULDNOT_LAUNCH_PROCESS,"No se pudo lanzar el proceso"));
		else
			compiler_tree.treeCtrl->SetItemText(compiler_tree.state,LANG(MAINW_COULDNOT_RUN,"No se pudo lanzar la ejecucion!"));
		delete compile_and_run;
		return;
	}
	// habilitar y deshabilitar lo que corresponda en menues
	menu.run_stop->Enable(true);
	menu.run_clean->Enable(false);
	menu.run_compile->Enable(false);
	if (compile_and_run->compiling) {
		menu.run_run->Enable(false);
		menu.tools_makefile->Enable(false);
		// mostrar el arbol de compilacion
		if (!config->Init.autohiding_panels) {
			if (!aui_manager.GetPane(compiler_tree.treeCtrl).IsShown()) {
				aui_manager.GetPane(quick_help).Hide();
				aui_manager.GetPane(compiler_tree.treeCtrl).Show();
				compiler_tree.menuItem->Check(true);
				aui_manager.Update();
			}
		}
		
		compiler->timer->Start(500);
	}
	// informar al usuario
	compiler_tree.treeCtrl->SetItemText(compiler_tree.state,msg);
	if (!project) status_bar->SetStatusText(msg);
}


void mxMainWindow::OnRunRun (wxCommandEvent &event) {
	if (!compiler->valgrind_cmd.Len() && config->Debug.always_debug) { // si siempre hay que ejecutar en el depurador
		OnDebugRun(event); // patearle la bocha
		return;
	}
	
	if (project) { // si hay que ejecutar un proyecto
		IF_THERE_IS_SOURCE {
			CURRENT_SOURCE->AutoCompCancel();
			CURRENT_SOURCE->CallTipCancel();
		}
		compiler->BuildOrRunProject(true,false,false);
		
	} else IF_THERE_IS_SOURCE { // si hay que ejecutar un ejercicio
		mxSource *source=CURRENT_SOURCE;
		source->AutoCompCancel();
		source->CallTipCancel();
		if (source->sin_titulo) { // si no esta guardado, siempre compilar
			if (source->GetLine(0).StartsWith(_T("make me a sandwich"))) { wxMessageBox("No way!"); return; }
			else if (source->GetLine(0).StartsWith(_T("sudo make me a sandwich"))) source->SetText(wxString(_T("/** Ok, you win! **/"))+wxString(250,' ')+_T("#include <iostream>\n")+wxString(250,' ')+_T("int main(int argc, char *argv[]) {std::cout<<\"Here you are:\\n\\n   /-----------\\\\\\n  ~~~~~~~~~~~~~~~\\n   \\\\-----------/\\n\";return 0;}\n\n"));
			source->SaveTemp();
			compiler->CompileSource(source,true,false);
		} else { // si estaba guardado ver si cambio
			// si es un .h, preguntar si no es mejor ejecutar un cpp
			wxString ext=source->sin_titulo||compiler->last_runned==source?wxString(_T("")):source->source_filename.GetExt().MakeUpper();
			if (ext==_T("H") || ext==_T("HPP") || ext==_T("HXX") || ext==_T("H++")) {
				if (source->GetModify())
					source->SaveSource();
				if (compiler->last_runned) {
					if (config->Running.dont_run_headers)
						source=compiler->last_runned;
					else {
						int ans = mxMessageDialog(this,wxString(LANG(MAINW_RUN_SOURCE_INSTEAD_OF_HEADER_QUESTION_PRE,"Esta intentando ejecutar un archivo de cabecera.\n"
							"Desea ejecutar en su lugar "))<<compiler->last_caption<<LANG(MAINW_RUN_SOURCE_INSTEAD_OF_HEADER_QUESTION_POST,"?"),
							LANG(GENERAL_WARNING,"Aviso"),mxMD_YES|mxMD_NO|mxMD_CANCEL,_T("Siempre ejecutar el ultimo cpp.")).ShowModal();
						if (ans&mxMD_CANCEL) return;
						if (ans&mxMD_CHECKED) config->Running.dont_run_headers = true;
						if (ans&mxMD_YES) source = compiler->last_runned;
					}
				} else {
					int ans = mxMessageDialog(this,LANG(MAINW_RUN_HEADER_QUESTION,"Esta intentando ejecutar un archivo de cabecera. Desea continuar?"),LANG(GENERAL_WARNING,"Aviso"),mxMD_YES|mxMD_CANCEL).ShowModal();
					if (ans&mxMD_CANCEL) return;
				}
			}
			
			if (source->GetModify() || !wxFileName::FileExists(source->binary_filename.GetFullPath()) || source->binary_filename.GetModificationTime()<source->source_filename.GetModificationTime()) {
				source->SaveSource();
				compiler->CompileSource(source,true,false);
			} else { // si no cambio nada, ver si cambiaron sus includes
				if (source->GetModify())
					source->SaveSource();
				if (config->Running.check_includes && utils->AreIncludesUpdated(source->binary_filename.GetModificationTime(),CURRENT_SOURCE->source_filename)) {
					compiler->CompileSource(source,true,false);
				} else { // si no cambio nada, correr el ya compilado
					RunSource(source);
				}
			}
		}
	}
}

void mxMainWindow::OnRunRunOld (wxCommandEvent &event) {
	if (project) { // si hay que ejecutar un proyecto
		IF_THERE_IS_SOURCE {
			CURRENT_SOURCE->AutoCompCancel();
			CURRENT_SOURCE->CallTipCancel();
		}
		compile_and_run_struct_single *compile_and_run=new compile_and_run_struct_single("OnRunRunOld");
		project->Run(compile_and_run);
		StartExecutionStuff(false,true,compile_and_run,LANG(GENERAL_RUNNING_DOTS,"Ejecutando..."));
	} else IF_THERE_IS_SOURCE { // si hay que ejecutar un ejercicio
		mxSource *source=CURRENT_SOURCE;
		source->AutoCompCancel();
		source->CallTipCancel();
		RunSource(source);
	}
}

void mxMainWindow::OnRunCompileConfig (wxCommandEvent &event) {
	if (compiler->IsCompiling()) {
		compile_and_run_struct_single *compile_and_run=compiler->compile_and_run_single;
		while (compile_and_run) {
			if (compile_and_run->process->Exists(compile_and_run->pid))
				wxProcess::Kill(compile_and_run->pid,wxSIGKILL,wxKILL_CHILDREN);
			compile_and_run=compile_and_run->next;
		}
	}
	if (project)
		new mxProjectConfigWindow(this);
	else IF_THERE_IS_SOURCE	
		new mxCompileConfigWindow(CURRENT_SOURCE,this);
}

void mxMainWindow::OnPreferences (wxCommandEvent &event) {
	if (preference_window) {
		preference_window->ShowUp();
	} else {
		preference_window = new mxPreferenceWindow(this);
	}
}

void mxMainWindow::OnRunCompile (wxCommandEvent &event) {
	if (project) {
		compiler->BuildOrRunProject(false,false,false);
	} else {
		IF_THERE_IS_SOURCE {
			mxSource *source=CURRENT_SOURCE;
			// save the current source
			if (source->sin_titulo)
				source->SaveTemp();
			else
				source->SaveSource();
			compiler->CompileSource(source,false,false);
		}
	}
}


void mxMainWindow::RunSource (mxSource *source) {
	
	// armar la linea de comando para ejecutar
	compiler->last_caption = source->page_text;
	compiler->last_runned = source;
	
	// agregar el prefijo para valgrind
	wxString exe_pref;
#if !defined(_WIN32) && !defined(__WIN32__)
	if (compiler->valgrind_cmd.Len())
		exe_pref = compiler->valgrind_cmd+_T(" ");
#endif
	
	wxString command(config->Files.terminal_command);
	command.Replace("${TITLE}",LANG(GENERA_CONSOLE_CAPTION,"ZinjaI - Consola de Ejecucion"));
	if (command.Len()!=0) {
		if (command==_T(" ")) 
			command=_T("");
		else if (command[command.Len()-1]!=' ') 
			command<<_T(" ");
	}
	command<<_T("\"")<<config->Files.runner_command<<_T("\" ");
	command<<_T("-lang \"")<<config->Init.language_file<<_T("\" ");
	if (source->config_running.wait_for_key) command<<_T("-waitkey ");
	command<<_T("\"")<<source->working_folder.GetFullPath()<<(source->working_folder.GetFullPath().Last()=='\\'?_T("\\\" "):_T("\" "));

	compiler->CheckForExecutablePermision(source->binary_filename.GetFullPath());
	
	command<<exe_pref<<_T("\"")<<source->binary_filename.GetFullPath()<<_T("\"");
//	utils->ParameterReplace(command,_T("${ZINJAI_DIR}"),wxGetCwd());
	// agregar los argumentos de ejecucion
	if (source->config_running.always_ask_args) {
		int res = (new mxArgumentsDialog(this,source->exec_args))->ShowModal();
		if (res&AD_CANCEL) return;
		if (res&AD_ARGS) {
			source->exec_args = mxArgumentsDialog::last_arguments;;
			command<<' '<<source->exec_args;
		}
		if (res&AD_REMEMBER) {
			source->config_running.always_ask_args=false;
			if (res&AD_EMPTY) source->exec_args=_T("");
		}
	} else if (source->exec_args.Len())
		command<<' '<<source->exec_args;	
	
	// lanzar la ejecucion
	compile_and_run_struct_single *compile_and_run=new compile_and_run_struct_single("OnRunSource");
	compile_and_run->process=new wxProcess(this->GetEventHandler(),mxPROCESS_COMPILE);
	compile_and_run->pid=wxExecute(command, wxEXEC_NOHIDE|wxEXEC_MAKE_GROUP_LEADER,compile_and_run->process);
	if (exe_pref.Len()) compile_and_run->valgrind_cmd=exe_pref;
	StartExecutionStuff (false,true, compile_and_run, LANG(GENERAL_RUNNING_DOTS,"Ejecutando...") );
	
}



void mxMainWindow::OnRunStop (wxCommandEvent &event) {
	if (compiler->IsCompiling()) {
		compile_and_run_struct_single *compile_and_run=compiler->compile_and_run_single, *next;
		while (compile_and_run) {
			next=compile_and_run->next;
			if (compile_and_run->pid!=0 && (compile_and_run->compiling || compile_and_run->linking) && compile_and_run->process->Exists(compile_and_run->pid)) {
				compile_and_run->killed=true;
				wxProcess::Kill(compile_and_run->pid,wxSIGKILL,wxKILL_CHILDREN);
			}
			compile_and_run=next;
		}
		compiler_tree.treeCtrl->SetItemText(compiler_tree.state,_T("Detenido!"));
	} else if (compiler->compile_and_run_single) {
		compile_and_run_struct_single *compile_and_run=compiler->compile_and_run_single;;
		compile_and_run->killed=true;
		wxProcess::Kill(compile_and_run->pid,wxSIGKILL,wxKILL_CHILDREN);
	}
	menu.run_stop->Enable(false);
	menu.run_compile->Enable(true);
	menu.run_run->Enable(true);
	if (project) {
		menu.run_clean->Enable(true);
		menu.tools_makefile->Enable(true);
	}
	status_bar->SetProgress(0);
	status_bar->SetStatusText(wxString(LANG(GENERAL_READY,"Listo")));
	
}

void mxMainWindow::OnFileSaveAll (wxCommandEvent &event) {
	if (project)
		project->Save();
	IF_THERE_IS_SOURCE {
		for (int i=0,j=notebook_sources->GetPageCount();i<j;i++) {
			mxSource *source = (mxSource*)(notebook_sources->GetPage(i));
			if (source->GetModify()) {
				notebook_sources->SetSelection(i);
				if (!source->sin_titulo) {
					source->SaveSource();
					parser->ParseSource(source,true);
				} else
					OnFileSaveAs(event);
			}
		}
	}
}

/**
* @brief Cierra una pestaña de codigo, pidiendo confirmacion al usuario si hay cambios
* @param Indice de la pestaña a cerrar, o -1 para cerrar la actual
**/
bool mxMainWindow::CloseFromGui (int i) {
	if (i<0) i=notebook_sources->GetSelection();
	if (i<0||i>=int(notebook_sources->GetPageCount())) return false;
	mxSource *source=(mxSource*)notebook_sources->GetPage(i);
	if (source->GetModify() && source->next_source_with_same_file==source) {
		notebook_sources->SetSelection(i);
		int res=mxMessageDialog(main_window,LANG(MAINW_SAVE_CHANGES_BEFORE_CLOSING_QUESTION,"Hay cambios sin guardar. Desea guardarlos antes de cerrar el archivo?"), source->page_text, mxMD_YES_NO_CANCEL|mxMD_QUESTION).ShowModal();
		if (res==mxMD_CANCEL)
			return false;
		else if (res==mxMD_YES) {
			if (source->sin_titulo) {
				wxCommandEvent evt;
				OnFileSaveAs(evt);
				if (source->GetModify())
					return false;
			} else
				source->SaveSource();
		}
	}
	CloseSource(i);
	return true;
}

void mxMainWindow::OnFileClose (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE CloseFromGui();
}

void mxMainWindow::OnFileCloseAll (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		for (int i=notebook_sources->GetPageCount()-1;i>=0;i--)
			CloseFromGui(i);
	}
}

void mxMainWindow::OnFileCloseAllButOne (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		int sel=notebook_sources->GetSelection();
		cerr<<notebook_sources->GetPageText(sel)<<endl;
		for (int i=notebook_sources->GetPageCount()-1;i>=0;i--)
			if (i!=sel) CloseFromGui(i);
	}
}

void mxMainWindow::OnFileCloseProject (wxCommandEvent &event) {
	if (debug->debugging) debug->Stop();
//	if (!project || parser->working) { 
//		parser->Stop(); 
//		event.Skip();
//		while (parser->working) wxYield(); 
//	}
//	if (project->modified) {
		if (config->Init.save_project)
			project->Save();
		else {
			int ret = mxMessageDialog(main_window,LANG(MAINW_SAVE_PROJECT_BEFORE_CLOSING_QUESTION,"Desea guardar los cambios del proyecto anterior antes de cerrarlo?"),project->GetFileName(),mxMD_YES_NO_CANCEL|mxMD_QUESTION,LANG(MAINW_ALWAYS_SAVE_PROJECT_ON_CLOSE,"Guardar cambios siempre al cerrar un proyecto"),false).ShowModal();
			if (mxMD_CANCEL&ret)
				return;
			else if (mxMD_YES&ret)
				project->Save();
			if (ret&mxMD_CHECKED)
				config->Init.save_project=true;
		}
//	}
	
	// cerrar si habia un proyecto anterior
	bool cerrar=true;
	for (int i=notebook_sources->GetPageCount()-1;i>=0;i--)
		if (((mxSource*)(notebook_sources->GetPage(i)))->GetModify()) {
			cerrar=false;
			break;
		}
	if (notebook_sources->GetPageCount()!=0) {
		if (cerrar || mxMD_YES==mxMessageDialog(main_window,LANG(MAINW_CHANGE_CLOSE_ALL_QUESTION,"Hay cambios sin guardar. Se cerraran todos los archivos. Desea Continuar?"),LANG(GENERAL_WARNING,"Aviso"),mxMD_YES_NO|mxMD_QUESTION).ShowModal()) {
			for (int i=notebook_sources->GetPageCount()-1;i>=0;i--) {
				project->GetSourceExtras((mxSource*)(notebook_sources->GetPage(i)));
				notebook_sources->DeletePage(i);;
			}
		} else
			return;
	}
	main_window->project_tree.treeCtrl->DeleteChildren(main_window->project_tree.sources);
	main_window->project_tree.treeCtrl->DeleteChildren(main_window->project_tree.headers);
	main_window->project_tree.treeCtrl->DeleteChildren(main_window->project_tree.others);
	delete project;
	project=NULL;
	symbols_tree.menuItem->Check(false);
	aui_manager.GetPane(symbols_tree.treeCtrl).Hide();
	compiler_tree.menuItem->Check(false);
	aui_manager.GetPane(compiler_tree.treeCtrl).Hide();
	project_tree.menuItem->Check(false);
	if (valgrind_panel) aui_manager.GetPane(valgrind_panel).Hide();
	if (config->Init.show_explorer_tree) {
		explorer_tree.menuItem->Check(true);
		aui_manager.GetPane(explorer_tree.treeCtrl).Show();
	} else {
		explorer_tree.menuItem->Check(false);
		aui_manager.GetPane(explorer_tree.treeCtrl).Hide();
	}
	aui_manager.GetPane(project_tree.treeCtrl).Hide();
	if (welcome_panel) 
		ShowWelcome(true);
	else {
		NewFileFromTemplate(config->Files.default_template);
		aui_manager.Update();
	}
}

void mxMainWindow::OnFileExportHtml (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		wxFileDialog dlg (this, LANG(GENERAL_SAVE,"Guardar"),source->sin_titulo?wxString(wxFileName::GetHomeDir()):wxFileName(source->source_filename).GetPath(),source->sin_titulo?wxString(wxEmptyString):wxFileName(source->source_filename).GetName()+_T(".html"), _T("Documento HTML | *.html"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		dlg.SetDirectory(source->sin_titulo?wxString(project?project->last_dir:config->Files.last_dir):wxFileName(source->source_filename).GetPath());
		if (dlg.ShowModal() == wxID_OK) {
			CodeExporter ce;
			mxSource *source = CURRENT_SOURCE;
			wxString title = notebook_sources->GetPageText(notebook_sources->GetSelection());
			if (title.Last()=='*') 
				title.RemoveLast();
			if (!ce.ExportHtml(source,title,dlg.GetPath()))
				mxMessageDialog(this,LANG(MAINW_COULD_NOT_EXPORT_HTML,"No se pudo guardar el archivo"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		}
	}
}

void mxMainWindow::OnFileReload (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		if (!source->sin_titulo) {
			if (source->GetModify() && mxMD_NO==mxMessageDialog(this,LANG(MAINW_CHANGES_CONFIRM_RELOAD,"Hay Cambios sin guardar, desea recargar igualmente la version del disco?"),source->source_filename.GetFullPath(),mxMD_YES_NO|mxMD_QUESTION).ShowModal())
				return;
			source->Reload();
			parser->ParseFile(source->source_filename.GetFullPath());
		}
	}
}

bool mxMainWindow::CloseSource (int i) {
	mxSource *source=(mxSource*)notebook_sources->GetPage(i);
	if (share && share->Exists(source))  {
		int ans =mxMessageDialog(main_window,_T("El archivo esta siendo compartido con modificaciones. Si lo cierra dejara de estar disponible.\nRealmente desea cerrar el archivo?"),source->page_text, mxMD_YES_NO,_T("Continuar compartiendo (\"sin modificaciones\") despues de cerrarlo."),false).ShowModal();
		if (mxMD_YES&ans) {
			if (mxMD_CHECKED&ans)
				share->Freeze(source);
			else
				share->Delete(source);
		} else {
			return false;
		}
	}
	if (!project) {
		if (source->next_source_with_same_file==source) {
			project_tree.treeCtrl->Delete(source->treeId);
			parser->RemoveSource(source);
		}
	} else {
		parser->ParseIfUpdated(source->source_filename);
		project->GetSourceExtras(source);
	}
	debug->CloseSource(source);
	notebook_sources->DeletePage(i);
	if (!project && welcome_panel && notebook_sources->GetPageCount()==0) ShowWelcome(true);
	return true;
}

void mxMainWindow::SetAccelerators() {
	wxAcceleratorEntry entries[14];
	entries[0].Set(wxACCEL_CTRL, WXK_SPACE, mxID_EDIT_FORCE_AUTOCOMPLETE);
	entries[1].Set(wxACCEL_CTRL, WXK_RETURN, mxID_FILE_OPEN_SELECTED);
	entries[2].Set(wxACCEL_CTRL|wxACCEL_SHIFT, WXK_TAB, mxID_VIEW_NOTEBOOK_PREV);
	entries[3].Set(wxACCEL_CTRL, WXK_TAB, mxID_VIEW_NOTEBOOK_NEXT);
	entries[4].Set(wxACCEL_CTRL, WXK_PAGEUP, mxID_VIEW_NOTEBOOK_PREV);
	entries[5].Set(wxACCEL_CTRL, WXK_PAGEDOWN, mxID_VIEW_NOTEBOOK_NEXT);
	entries[6].Set(wxACCEL_CTRL|wxACCEL_SHIFT, WXK_F5, mxID_DEBUG_DO_THAT);
	entries[7].Set(0, WXK_F12, mxID_FILE_OPEN_H);
	entries[8].Set(wxACCEL_ALT|wxACCEL_CTRL, 'p', mxID_RUN_CONFIG);
	entries[9].Set(wxACCEL_SHIFT|wxACCEL_CTRL, WXK_F6, mxID_INTERNAL_INFO);
	entries[10].Set(wxACCEL_ALT|wxACCEL_CTRL, WXK_SPACE, mxID_WHERE_TIMER);
	entries[11].Set(wxACCEL_CTRL|wxACCEL_SHIFT, WXK_SPACE, mxID_EDIT_AUTOCODE_AUTOCOMPLETE);
	entries[12].Set(wxACCEL_CTRL|wxACCEL_ALT, 'w', mxID_FILE_CLOSE_ALL_BUT_ONE);
	entries[13].Set(wxACCEL_CTRL|wxACCEL_ALT, 'f', mxID_EDIT_FIND_FROM_TOOLBAR);
	wxAcceleratorTable accel(14,entries);
	SetAcceleratorTable(accel);
}

void mxMainWindow::OnViewFullScreen(wxCommandEvent &event) {
	if (IsFullScreen()) {
		menu.view_fullscreen->Check(false);
		if (config->Init.autohide_toolbars_fs) { // reacomodar las barras de herramientas
			if (menu.view_toolbar_debug->IsChecked()!=fullscreen_toolbars_status[0]) {
				if (fullscreen_toolbars_status[0]) {
					menu.view_toolbar_debug->Check(true);
					aui_manager.GetPane(toolbar_debug).Show();
				} else {
					menu.view_toolbar_debug->Check(false);
					aui_manager.GetPane(toolbar_debug).Hide();
				}
			}
			if (menu.view_toolbar_file->IsChecked()!=fullscreen_toolbars_status[1]) {
				if (fullscreen_toolbars_status[1]) {
					menu.view_toolbar_file->Check(true);
					aui_manager.GetPane(toolbar_file).Show();
				} else {
					menu.view_toolbar_file->Check(false);
					aui_manager.GetPane(toolbar_file).Hide();
				}
			}
			if (menu.view_toolbar_run->IsChecked()!=fullscreen_toolbars_status[2]) {
				if (fullscreen_toolbars_status[2]) {
					menu.view_toolbar_run->Check(true);
					aui_manager.GetPane(toolbar_run).Show();
				} else {
					menu.view_toolbar_run->Check(false);
					aui_manager.GetPane(toolbar_run).Hide();
				}
			}
			if (menu.view_toolbar_edit->IsChecked()!=fullscreen_toolbars_status[3]) {
				if (fullscreen_toolbars_status[3]) {
					menu.view_toolbar_edit->Check(true);
					aui_manager.GetPane(toolbar_edit).Show();
				} else {
					menu.view_toolbar_edit->Check(false);
					aui_manager.GetPane(toolbar_edit).Hide();
				}
			}
			if (menu.view_toolbar_misc->IsChecked()!=fullscreen_toolbars_status[4]) {
				if (fullscreen_toolbars_status[4]) {
					menu.view_toolbar_misc->Check(true);
					aui_manager.GetPane(toolbar_misc).Show();
				} else {
					menu.view_toolbar_misc->Check(false);
					aui_manager.GetPane(toolbar_misc).Hide();
				}
			}
			if (menu.view_toolbar_find->IsChecked()!=fullscreen_toolbars_status[5]) {
				if (fullscreen_toolbars_status[5]) {
					menu.view_toolbar_find->Check(true);
					aui_manager.GetPane(toolbar_find).Show();
				} else {
					menu.view_toolbar_find->Check(false);
					aui_manager.GetPane(toolbar_find).Hide();
				}
			}
			if (menu.view_toolbar_debug->IsChecked()!=fullscreen_toolbars_status[6]) {
				if (fullscreen_toolbars_status[6]) {
					menu.view_toolbar_debug->Check(true);
					aui_manager.GetPane(toolbar_debug).Show();
				} else {
					menu.view_toolbar_debug->Check(false);
					aui_manager.GetPane(toolbar_debug).Hide();
				}
			}
			if (menu.view_toolbar_tools->IsChecked()!=fullscreen_toolbars_status[7]) {
				if (fullscreen_toolbars_status[7]) {
					menu.view_toolbar_tools->Check(true);
					aui_manager.GetPane(toolbar_tools).Show();
				} else {
					menu.view_toolbar_tools->Check(false);
					aui_manager.GetPane(toolbar_tools).Hide();
				}
			}
			if (menu.view_toolbar_view->IsChecked()!=fullscreen_toolbars_status[8]) {
				if (fullscreen_toolbars_status[8]) {
					menu.view_toolbar_view->Check(true);
					aui_manager.GetPane(toolbar_view).Show();
				} else {
					menu.view_toolbar_view->Check(false);
					aui_manager.GetPane(toolbar_view).Hide();
				}
			}
		}
		
		if (config->Init.autohide_panels_fs && !config->Init.autohiding_panels) { // reacomodar los paneles
			if ( fullscreen_panels_status[0]!=aui_manager.GetPane(compiler_tree.treeCtrl).IsShown() ) {
				if (fullscreen_panels_status[0]) {
					aui_manager.GetPane(compiler_tree.treeCtrl).Show();
					compiler_tree.menuItem->Check(true);
				} else {
					aui_manager.GetPane(compiler_tree.treeCtrl).Hide();
					compiler_tree.menuItem->Check(false);
				}
			}
			if ( fullscreen_panels_status[1]!=aui_manager.GetPane(quick_help).IsShown() ) {
				if (fullscreen_panels_status[1])
					aui_manager.GetPane(quick_help).Show();
				else
					aui_manager.GetPane(quick_help).Hide();
			}
			
			if ( !fullscreen_panels_status[2] )
				aui_manager.GetPane(backtrace_ctrl).Hide();
			if ( !fullscreen_panels_status[3] )
				aui_manager.GetPane((wxGrid*)inspection_ctrl).Hide();

			if (left_panels) {
				if ( fullscreen_panels_status[4]!=aui_manager.GetPane(left_panels).IsShown() ) {
					if (fullscreen_panels_status[4]) {
						aui_manager.GetPane(left_panels).Show();
						menu.view_left_panels->Check(true);
					} else {
						aui_manager.GetPane(left_panels).Hide();
						menu.view_left_panels->Check(false);
					}
				}
			} else {
				if ( fullscreen_panels_status[4]!=aui_manager.GetPane(symbols_tree.treeCtrl).IsShown() ) {
					if (fullscreen_panels_status[4]) {
						aui_manager.GetPane(symbols_tree.treeCtrl).Show();
						symbols_tree.menuItem->Check(true);
					} else {
						aui_manager.GetPane(symbols_tree.treeCtrl).Hide();
						symbols_tree.menuItem->Check(false);
					}
				}
				if ( fullscreen_panels_status[5]!=aui_manager.GetPane(project_tree.treeCtrl).IsShown() ) {
					if (fullscreen_panels_status[5]) {
						aui_manager.GetPane(project_tree.treeCtrl).Show();
						project_tree.menuItem->Check(true);
					} else {
						aui_manager.GetPane(project_tree.treeCtrl).Hide();
						project_tree.menuItem->Check(false);
					}
				}
				if ( fullscreen_panels_status[6]!=aui_manager.GetPane(explorer_tree.treeCtrl).IsShown() ) {
					if (fullscreen_panels_status[6]) {
						aui_manager.GetPane(explorer_tree.treeCtrl).Show();
						explorer_tree.menuItem->Check(true);
					} else {
						aui_manager.GetPane(explorer_tree.treeCtrl).Hide();
						explorer_tree.menuItem->Check(false);
					}
				}
			}
			if ( beginner_panel && fullscreen_panels_status[7]!=aui_manager.GetPane(beginner_panel).IsShown() ) {
				if (fullscreen_panels_status[7]) {
					aui_manager.GetPane(beginner_panel).Show();
					menu.view_beginner_panel->Check(true);
				} else {
					aui_manager.GetPane(beginner_panel).Hide();
					menu.view_beginner_panel->Check(false);
				}
			}
			if ( fullscreen_panels_status[8]!=aui_manager.GetPane(threadlist_ctrl).IsShown() ) {
				if (fullscreen_panels_status[8]) {
					debug->threadlist_visible=true;
					aui_manager.GetPane(threadlist_ctrl).Show();
				} else {
					debug->threadlist_visible=false;
					aui_manager.GetPane(threadlist_ctrl).Hide();
				}
			}
		}		
		
		ShowFullScreen(false);
		
	} else {
		for (int i=0;i<10;i++) debug_toolbars_status[i]=false;
		menu.view_fullscreen->Check(true);
		if (valgrind_panel) aui_manager.GetPane(valgrind_panel).Hide();
		if (config->Init.autohide_toolbars_fs) { // reacomodar las barras de herramientas
			if ( (fullscreen_toolbars_status[1]=menu.view_toolbar_file->IsChecked()) ) {
				menu.view_toolbar_file->Check(false);
				aui_manager.GetPane(toolbar_file).Hide();
			}
			if ( (fullscreen_toolbars_status[2]=menu.view_toolbar_run->IsChecked()) ) {
				menu.view_toolbar_run->Check(false);
				aui_manager.GetPane(toolbar_run).Hide();
			}
			if ( (fullscreen_toolbars_status[3]=menu.view_toolbar_edit->IsChecked()) ) {
				menu.view_toolbar_edit->Check(false);
				aui_manager.GetPane(toolbar_edit).Hide();
			}
			if ( (fullscreen_toolbars_status[4]=menu.view_toolbar_misc->IsChecked()) ) {
				menu.view_toolbar_misc->Check(false);
				aui_manager.GetPane(toolbar_misc).Hide();
			}
			if ( (fullscreen_toolbars_status[5]=menu.view_toolbar_find->IsChecked()) ) {
				menu.view_toolbar_find->Check(false);
				aui_manager.GetPane(toolbar_find).Hide();
			}
			if ( (fullscreen_toolbars_status[6]=menu.view_toolbar_debug->IsChecked()) ) {
				menu.view_toolbar_debug->Check(false);
				aui_manager.GetPane(toolbar_debug).Hide();
			}
			if ( (fullscreen_toolbars_status[7]=menu.view_toolbar_tools->IsChecked()) ) {
				menu.view_toolbar_tools->Check(false);
				aui_manager.GetPane(toolbar_tools).Hide();
			}
			if ( (fullscreen_toolbars_status[8]=menu.view_toolbar_view->IsChecked()) ) {
				menu.view_toolbar_view->Check(false);
				aui_manager.GetPane(toolbar_view).Hide();
			}
		}
		if (config->Init.autohide_panels_fs && !config->Init.autohiding_panels) { // reacomodar los paneles
			if ( (fullscreen_panels_status[0]=aui_manager.GetPane(compiler_tree.treeCtrl).IsShown()) ) {
				aui_manager.GetPane(compiler_tree.treeCtrl).Hide();
				compiler_tree.menuItem->Check(false);
			}
			if ( (fullscreen_panels_status[1]=aui_manager.GetPane(quick_help).IsShown()) ) {
				aui_manager.GetPane(quick_help).Hide();
			}
			if ( (fullscreen_panels_status[2]=aui_manager.GetPane(backtrace_ctrl).IsShown()) )
				aui_manager.GetPane(backtrace_ctrl).Hide();
			if ( (fullscreen_panels_status[3]=aui_manager.GetPane((wxGrid*)inspection_ctrl).IsShown()) )
				aui_manager.GetPane((wxGrid*)inspection_ctrl).Hide();
			if (left_panels) {
				if ( (fullscreen_panels_status[4]=aui_manager.GetPane(left_panels).IsShown()) ) {
					aui_manager.GetPane(left_panels).Hide();
					menu.view_left_panels->Check(false);
				}
			} else {
				if ( (fullscreen_panels_status[4]=aui_manager.GetPane(symbols_tree.treeCtrl).IsShown()) ) {
					aui_manager.GetPane(symbols_tree.treeCtrl).Hide();
					symbols_tree.menuItem->Check(false);
				}
				if ( (fullscreen_panels_status[5]=aui_manager.GetPane(project_tree.treeCtrl).IsShown()) ) {
					aui_manager.GetPane(project_tree.treeCtrl).Hide();
					project_tree.menuItem->Check(false);
				}
				if ( (fullscreen_panels_status[6]=aui_manager.GetPane(explorer_tree.treeCtrl).IsShown()) ) {
					aui_manager.GetPane(explorer_tree.treeCtrl).Hide();
					explorer_tree.menuItem->Check(false);
				}
			}
			if ( beginner_panel && (fullscreen_panels_status[7]=aui_manager.GetPane(beginner_panel).IsShown()) ) {
				aui_manager.GetPane(beginner_panel).Hide();
				menu.view_beginner_panel->Check(false);
			}
			if ( (fullscreen_panels_status[8]=aui_manager.GetPane(threadlist_ctrl).IsShown()) ) {
				debug->threadlist_visible=false;
				aui_manager.GetPane(threadlist_ctrl).Hide();
			}
		}
		
		ShowFullScreen(true,(config->Init.autohide_menus_fs?wxFULLSCREEN_NOMENUBAR:0)|wxFULLSCREEN_NOTOOLBAR|wxFULLSCREEN_NOSTATUSBAR|wxFULLSCREEN_NOBORDER|wxFULLSCREEN_NOCAPTION );
		new mxOSD(this,LANG(MAINW_FULLSCREEN_OUT_TIP,"Presione F11 para salir del modo pantalla completa"),3000,true);
		Raise();
		wxYield();
//		IF_THERE_IS_SOURCE CURRENT_SOURCE->SetFocus();
	}
	
	aui_manager.Update();
	
	SetAccelerators();
	
}

void mxMainWindow::OnViewHideBottom (wxCommandEvent &event) {
	if (aui_manager.GetPane(compiler_tree.treeCtrl).IsShown()) {
		compiler_tree.menuItem->Check(false);
		aui_manager.GetPane(compiler_tree.treeCtrl).Hide();
	}
	if (aui_manager.GetPane(quick_help).IsShown()) {
		aui_manager.GetPane(quick_help).Hide();
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewCodeStyle (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		menu.view_code_style->Check(!CURRENT_SOURCE->config_source.syntaxEnable);
		CURRENT_SOURCE->SetStyle(menu.view_code_style->IsChecked());
		CURRENT_SOURCE->SetColours(false); // por alguna razon el SetStyle de arriba cambia el fondo de los nros de linea
	}
}

void mxMainWindow::OnViewLineWrap (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source=CURRENT_SOURCE;
		int pos=source->GetCurrentPos();
		source->config_source.wrapMode=!source->config_source.wrapMode;
		menu.view_line_wrap->Check(source->config_source.wrapMode);
		source->SetWrapMode (source->config_source.wrapMode?wxSTC_WRAP_WORD: wxSTC_WRAP_NONE);
		source->GotoPos(pos);
		source->SetFocus();
	}
}

void mxMainWindow::OnViewWhiteSpace (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source=CURRENT_SOURCE;
		source->config_source.whiteSpace = !(source->config_source.whiteSpace);
		menu.view_white_space->Check(source->config_source.whiteSpace);
		source->SetViewWhiteSpace(source->config_source.whiteSpace?wxSTC_WS_VISIBLEALWAYS:wxSTC_WS_INVISIBLE);
		source->SetViewEOL(source->config_source.whiteSpace);
		source->SetFocus();
		wxYield();
	}
}

void mxMainWindow::OnViewLeftPanels (wxCommandEvent &event) {
	if (!menu.view_left_panels->IsChecked()) {
		menu.view_left_panels->Check(false);
		aui_manager.GetPane(left_panels).Hide();
	} else {
		menu.view_left_panels->Check(true);
		aui_manager.GetPane(left_panels).Show();
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewProjectTree (wxCommandEvent &event) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(project_tree.treeCtrl).IsShown())
			autohide_handlers[ATH_PROJECT]->ForceShow();
	} else {	
		if(left_panels) {
			if (!menu.view_left_panels->IsChecked()) {
				menu.view_left_panels->Check(true);
				aui_manager.GetPane(left_panels).Show();
				aui_manager.Update();
			}
			left_panels->SetSelection(0);
			project_tree.menuItem->Check(false);
		} else {
			if (!project_tree.menuItem->IsChecked()) {
				project_tree.menuItem->Check(false);
				aui_manager.GetPane(project_tree.treeCtrl).Hide();
			} else {
				project_tree.menuItem->Check(true);
				aui_manager.GetPane(project_tree.treeCtrl).Show();
				project_tree.treeCtrl->ExpandAll();
			}
			aui_manager.Update();
		}
	}
}

void mxMainWindow::OnViewSymbolsTree (wxCommandEvent &event) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(symbols_tree.treeCtrl).IsShown())
			autohide_handlers[ATH_SYMBOL]->ForceShow();
	} else {	
		if(left_panels) {
			if (!menu.view_left_panels->IsChecked()) {
				menu.view_left_panels->Check(true);
				aui_manager.GetPane(left_panels).Show();
				aui_manager.Update();
			}
			left_panels->SetSelection(1);
			symbols_tree.menuItem->Check(false);
		} else {
			if (!symbols_tree.menuItem->IsChecked()) {
				symbols_tree.menuItem->Check(false);
				aui_manager.GetPane(symbols_tree.treeCtrl).Hide();
			} else {
				symbols_tree.menuItem->Check(true);
				aui_manager.GetPane(symbols_tree.treeCtrl).Show();
			}
			aui_manager.Update();
		}
	}
}

void mxMainWindow::OnViewUpdateSymbols (wxCommandEvent &event) {
	wxWindow *focus = main_window->FindFocus();
	if (focus) focus = focus->GetParent();
	if (focus==inspection_ctrl) {
		int r =inspection_ctrl->GetGridCursorRow();
		inspection_ctrl->SetGridCursor(r,IG_COL_EXPR);
		inspection_ctrl->EnableCellEditControl(true);
		return;
	}
	if (config->Init.autohiding_panels) {
//		if (!aui_manager.GetPane(symbols_tree.treeCtrl).IsShown())
//			autohide_handlers[ATH_SYMBOL]->ForceShow();
	} else {	
		if(left_panels) {
			if (!menu.view_left_panels->IsChecked()) {
				menu.view_left_panels->Check(true);
				aui_manager.GetPane(left_panels).Show();
				aui_manager.Update();
			}
			left_panels->SetSelection(1);
		} else {
			if (!symbols_tree.menuItem->IsChecked()) {
				symbols_tree.menuItem->Check(true);
				aui_manager.GetPane(symbols_tree.treeCtrl).Show();
				aui_manager.Update();
			}
		}
	}
	UpdateSymbols();
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFocus();
}

void mxMainWindow::OnSymbolTreeIncludes (wxCommandEvent &event) {
	parser->follow_includes=event.IsChecked();
	UpdateSymbols();
}

void mxMainWindow::ShowQuickHelp (wxString keyword, bool hide_compiler_tree) {
	// load help text
	IF_THERE_IS_SOURCE {
		CURRENT_SOURCE->AutoCompCancel();
		CURRENT_SOURCE->CallTipCancel();
	}
	
	quick_help->SetPage(help->GetQuickHelp(keyword));
	ShowQuickHelpPanel(hide_compiler_tree);
}

void mxMainWindow::OnViewToolbarView (wxCommandEvent &event) {
	if (!menu.view_toolbar_view->IsChecked()) {
		menu.view_toolbar_view->Check(false);
		aui_manager.GetPane(toolbar_view).Hide();
		config->Toolbars.wich_ones.view=false;
	} else {
		menu.view_toolbar_view->Check(true);
		aui_manager.GetPane(toolbar_view).Show();
		config->Toolbars.wich_ones.view=true;
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewToolbarTools (wxCommandEvent &event) {
	if (!menu.view_toolbar_tools->IsChecked()) {
		menu.view_toolbar_tools->Check(false);
		aui_manager.GetPane(toolbar_tools).Hide();
		config->Toolbars.wich_ones.tools=false;
	} else {
		menu.view_toolbar_tools->Check(true);
		aui_manager.GetPane(toolbar_tools).Show();
		config->Toolbars.wich_ones.tools=true;
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewToolbarFile (wxCommandEvent &event) {
	if (!menu.view_toolbar_file->IsChecked()) {
		menu.view_toolbar_file->Check(false);
		aui_manager.GetPane(toolbar_file).Hide();
		config->Toolbars.wich_ones.file=false;
	} else {
		menu.view_toolbar_file->Check(true);
		aui_manager.GetPane(toolbar_file).Show();
		config->Toolbars.wich_ones.file=true;
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewToolbarFind (wxCommandEvent &event) {
	if (!menu.view_toolbar_find->IsChecked()) {
		menu.view_toolbar_find->Check(false);
		aui_manager.GetPane(toolbar_find).Hide();
		config->Toolbars.wich_ones.find=false;
	} else {
		menu.view_toolbar_find->Check(true);
		aui_manager.GetPane(toolbar_find).Show();
		config->Toolbars.wich_ones.find=true;
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewToolbarDebug (wxCommandEvent &event) {
	if (!menu.view_toolbar_debug->IsChecked()) {
		menu.view_toolbar_debug->Check(false);
		aui_manager.GetPane(toolbar_debug).Hide();
		config->Toolbars.wich_ones.debug=false;
	} else {
		menu.view_toolbar_debug->Check(true);
		aui_manager.GetPane(toolbar_debug).Show();
		config->Toolbars.wich_ones.debug=true;
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewToolbarMisc (wxCommandEvent &event) {
	if (!menu.view_toolbar_misc->IsChecked()) {
		menu.view_toolbar_misc->Check(false);
		aui_manager.GetPane(toolbar_misc).Hide();
		config->Toolbars.wich_ones.misc=false;
	} else {
		menu.view_toolbar_misc->Check(true);
		aui_manager.GetPane(toolbar_misc).Show();
		config->Toolbars.wich_ones.misc=true;
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewToolbarEdit (wxCommandEvent &event) {
	if (!menu.view_toolbar_edit->IsChecked()) {
		menu.view_toolbar_edit->Check(false);
		aui_manager.GetPane(toolbar_edit).Hide();
		config->Toolbars.wich_ones.edit=false;
	} else {
		menu.view_toolbar_edit->Check(true);
		aui_manager.GetPane(toolbar_edit).Show();
		config->Toolbars.wich_ones.edit=true;
	}
	aui_manager.Update();
}
void mxMainWindow::OnViewToolbarRun (wxCommandEvent &event) {
	if (!menu.view_toolbar_run->IsChecked()) {
		menu.view_toolbar_run->Check(false);
		aui_manager.GetPane(toolbar_run).Hide();
		config->Toolbars.wich_ones.run=false;
	} else {
		menu.view_toolbar_run->Check(true);
		aui_manager.GetPane(toolbar_run).Show();
		config->Toolbars.wich_ones.run=true;
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewCompilerTree (wxCommandEvent &event) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(compiler_tree.treeCtrl).IsShown())
			autohide_handlers[ATH_COMPILER]->ForceShow();
	} else {	
		if (!compiler_tree.menuItem->IsChecked()) {
			compiler_tree.menuItem->Check(false);
			aui_manager.GetPane(compiler_tree.treeCtrl).Hide();
		} else {
			compiler_tree.menuItem->Check(true);
			aui_manager.GetPane(compiler_tree.treeCtrl).Show();
		}
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewExplorerTree (wxCommandEvent &event) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(explorer_tree.treeCtrl).IsShown()) {
			autohide_handlers[ATH_EXPLORER]->ForceShow();
			if (!project) SetExplorerPath(config->Files.last_dir);
		}
	} else {	
		if(left_panels) {
			if (!menu.view_left_panels->IsChecked()) {
				menu.view_left_panels->Check(true);
				aui_manager.GetPane(left_panels).Show();
				aui_manager.Update();
			}
			explorer_tree.menuItem->Check(false);
			left_panels->SetSelection(2);
			explorer_tree.treeCtrl->SetFocus();
		} else {
			if (!explorer_tree.menuItem->IsChecked()) {
				explorer_tree.menuItem->Check(false);
				aui_manager.GetPane(explorer_tree.treeCtrl).Hide();
			} else {
				explorer_tree.menuItem->Check(true);
				if (!project) SetExplorerPath(config->Files.last_dir);
				aui_manager.GetPane(explorer_tree.treeCtrl).Show();
			}
			aui_manager.Update();
		}
	}
}


mxSource *mxMainWindow::IsOpen (wxFileName filename) {
	for (int i=0,j=notebook_sources->GetPageCount();i<j;i++)
		if ( !((mxSource*)(notebook_sources->GetPage(i)))->sin_titulo && 
			SameFile(((mxSource*)(notebook_sources->GetPage(i)))->source_filename,filename))
				return (mxSource*)(notebook_sources->GetPage(i));
	return NULL;
}

mxSource *mxMainWindow::IsOpen (wxTreeItemId tree_item) {
	for (int i=0,j=notebook_sources->GetPageCount();i<j;i++)
		if ( ((mxSource*)(notebook_sources->GetPage(i)))->treeId == tree_item )
			return (mxSource*)(notebook_sources->GetPage(i));
	return NULL;
}

// esta funcion solo se llama cuando no es proyecto
wxTreeItemId mxMainWindow::AddToProjectTreeProject(wxString name, char where, bool sort) {
	wxString iname=config->Init.fullpath_on_project_tree?name:wxFileName(name).GetFullName();
	wxTreeItemId item;
	switch (where) {
	case 's':
		item = project_tree.treeCtrl->AppendItem(project_tree.sources, iname, 1);
		if (sort) project_tree.treeCtrl->SortChildren(main_window->project_tree.sources);
		return item;
	case 'h':
		item = project_tree.treeCtrl->AppendItem(project_tree.headers, iname, 2);
		if (sort) project_tree.treeCtrl->SortChildren(main_window->project_tree.headers);
		break;
	default:
		item = project_tree.treeCtrl->AppendItem(project_tree.others, iname, wxFileName(name).GetExt().MakeUpper()==_T("FBP")?5:3);
		if (sort) project_tree.treeCtrl->SortChildren(main_window->project_tree.others);
		break;
	};
	return item;
}

wxTreeItemId mxMainWindow::AddToProjectTreeSimple(wxFileName filename, char where) {
	if (where=='u') where=utils->GetFileType(filename.GetFullName(),false);
	switch (where) {
		case 's':
			return project_tree.treeCtrl->AppendItem(project_tree.sources, filename.GetFullName(), 1);
			break;
		case 'h':
			return project_tree.treeCtrl->AppendItem(project_tree.headers, filename.GetFullName(), 2);
			break;
		default:
			if (filename.GetExt().MakeUpper()==_T("FBP"))
				return project_tree.treeCtrl->AppendItem(project_tree.others, filename.GetFullName(), 5);
			else
				return project_tree.treeCtrl->AppendItem(project_tree.others, filename.GetFullName(), 3);
			break;
	};
}


mxSource *mxMainWindow::FindSource(wxFileName filename, int *pos) {
	for (int i=0,j=notebook_sources->GetPageCount();i<j;i++) {
		if ( !((mxSource*)(notebook_sources->GetPage(i)))->sin_titulo && SameFile(((mxSource*)(notebook_sources->GetPage(i)))->source_filename,filename) ) {
			if (pos) *pos=i;
			return (mxSource*)(notebook_sources->GetPage(i));
		}
	}
	return NULL;
}

/**
* @return NULL si no encuentra el archivo, puntero al mxSource si lo abre en Zinjai, 
*         o puntero a main_window si se abre con un programa externo (como wxFormBuilder)
**/
mxSource *mxMainWindow::OpenFile (wxString filename, bool add_to_project) {
	if (welcome_panel && notebook_sources->GetPageCount()==0) ShowWelcome(false);
	if (filename==_T("") || !wxFileName::FileExists(filename))
		return NULL;
	
	if (project && project->use_wxfb && filename.Len()>4 && filename.Mid(filename.Len()-4).CmpNoCase(_T(".fbp"))==0) {
		if (add_to_project) {
			project->AddFile('o',filename);
		} else {
			mxOSD osd(this,LANG(WXFB_OPENING,"Abriendo wxFormBuilder..."));
			wxExecute(wxString(_T("\""))+config->Files.wxfb_command+_T("\" \"")+filename+_T("\""));
			wxYield(); wxMilliSleep(1000);
		}
		return external_source;
	}
	
	int i;
	mxSource *source = FindSource(filename,&i);
	bool not_opened=true;
	if (source) {
		notebook_sources->SetSelection(i);
		source->SetFocus();
		not_opened=false;
	} else {
		source = new mxSource(notebook_sources, AvoidDuplicatePageText(wxFileName(filename).GetFullName()));
		source->sin_titulo=false;
		source->LoadFile(filename);
		if (project) project->SetSourceExtras(source);
	}
	wxString ext=wxFileName(filename).GetExt().MakeUpper();
	if (ext==_T("CPP") || ext==_T("CXX") || ext==_T("C") || ext==_T("C++")) {
		if (not_opened) notebook_sources->AddPage(source, source->page_text, true, *bitmaps->files.source);
		if (add_to_project) {
			if (project) {
				 if (!project->HasFile(filename)) {
					 file_item *fi=project->AddFile('s',filename);
					 source->treeId=fi->item;
				 }
			} else {
				source->treeId = AddToProjectTreeSimple(filename,'s');
			}
			source->never_parsed=false;
			parser->ParseFile(filename);
		}
	} else if (ext==_T("H") || ext==_T("HXX") || ext==_T("HPP")) {
		if (not_opened) notebook_sources->AddPage(source, source->page_text, true, *bitmaps->files.header);
		if (add_to_project) {
			if (project) {
				if (!project->HasFile(filename)) {
					file_item *fi=project->AddFile('h',filename);
					source->treeId = fi->item;
				}
			} else {
				source->treeId = AddToProjectTreeSimple(filename,'h');
			}
			source->never_parsed=false;
			parser->ParseFile(filename);
		}
	} else {
		if (not_opened) notebook_sources->AddPage(source, source->page_text, true, *bitmaps->files.other);
		if (add_to_project) {
			if (project) {
				if (!project->HasFile(filename)) {
					file_item *fi=project->AddFile('o',filename);
					source->treeId=fi->item;
				}
			} else {
				source->treeId = AddToProjectTreeSimple(filename,'o');
			}
		}
	}
	return source;
}

void mxMainWindow::OpenFileFromGui (wxFileName filename, int *multiple) {
	if (!filename.FileExists()) {
		if (wxFileName::DirExists(DIR_PLUS_FILE(filename.GetFullPath(),"."))) {
			SetExplorerPath(filename.GetFullPath());
			ShowExplorerTreePanel();
		} else 
			mxMessageDialog(main_window,LANG(MAINW_FILE_NOT_EXISTS,"El archivo no existe."),filename.GetFullPath(),mxMD_OK|mxMD_INFO).ShowModal();
		return;
	}
	status_bar->SetStatusText(wxString(_T("Abriendo "))<<filename.GetFullPath());
	if (!project) config->Files.last_dir=filename.GetPath();
	if (filename.GetExt().CmpNoCase(_T(PROJECT_EXT))==0) { // si es un proyecto
		// cerrar si habia un proyecto anterior
		if (project) {
			int ret=0;
			if (config->Init.save_project || (/*project->modified && */mxMD_YES&(ret=mxMessageDialog(main_window,LANG(MAINW_ASK_SAVE_PREVIOUS_PROJECT,"Desea guardar los cambios del proyecto anterior antes de cerrarlo?"),project->GetFileName(),mxMD_YES_NO|mxMD_QUESTION,LANG(MAINW_ALWAYS_SAVE_PROJECT_ON_CLOSE,"Guardar cambios siempre al cerrar un proyecto"),false).ShowModal()))) {
				if (!config->Init.save_project && ret&mxMD_CHECKED)
					config->Init.save_project=true;
				project->Save();
			}
		}
		// cerrar todos los archivos que no pertenezcan al proyecto
		if (notebook_sources->GetPageCount()!=0 && (!multiple || (*multiple)!=-5)) {
			bool do_close = config->Init.close_files_for_project;
			if (!do_close) {
				int ans = mxMessageDialog(main_window,LANG(MAINW_CLOSE_ALL_BEFORE_PROJECT_QUESTION,"Desea cerrar todos los archivos que no pertenecen al proyecto?"),LANG(GENERAL_WARNING,"Aviso"),mxMD_YES_NO|mxMD_QUESTION,LANG(MAINW_CLOSE_ALL_BEFORE_PROJECT_CHECK,"Siempre cerrar los demas archivos al abrir un proyecto"),false).ShowModal();
					if (ans&mxMD_YES)
						do_close=true;
					if (ans&mxMD_CHECKED)
						config->Init.close_files_for_project = true;
			}
			if (do_close) {
				mxSource *source;
				for (int i=notebook_sources->GetPageCount()-1;i>=0;i--) {
					source = ((mxSource*)(notebook_sources->GetPage(i)));
					if (source ->GetModify()) {
						notebook_sources->SetSelection(i);
						int res=mxMessageDialog(main_window,LANG(MAINW_SAVE_CHANGES_QUESTION,"Hay cambios sin guardar. Desea guardarlos?"), source->page_text, mxMD_QUESTION|mxMD_YES_NO_CANCEL).ShowModal();
						if (mxMD_CANCEL==res) {
							status_bar->SetStatusText(LANG(GENERAL_READY,"Listo"));
							return;
						} else if (mxMD_YES==res)
							source->SaveSource();
						CloseSource(i);//notebook_sources->DeletePage(i);;
					} else
						CloseSource(i);//notebook_sources->DeletePage(i);
				}
			}
		}
		if (project) { // eliminar el proyecto viejo de la memoria
			delete project;
			project=NULL;
		}
		if (welcome_panel && notebook_sources->GetPageCount()==0) ShowWelcome(false);
		// abrir el proyecto
		project = new ProjectManager(filename);
		// mostrar el arbol de proyecto
		if (!config->Init.autohiding_panels) {
			if (left_panels) {
				menu.view_left_panels->Check(true);
				aui_manager.GetPane(left_panels).Show();
				left_panels->SetSelection(config->Init.prefer_explorer_tree?2:0);
			} else {
				if (config->Init.prefer_explorer_tree) {
					explorer_tree.menuItem->Check(true);
					aui_manager.GetPane(explorer_tree.treeCtrl).Show();
					project_tree.menuItem->Check(false);
					aui_manager.GetPane(project_tree.treeCtrl).Hide();
				} else {
					explorer_tree.menuItem->Check(false);
					aui_manager.GetPane(explorer_tree.treeCtrl).Hide();
					project_tree.menuItem->Check(true);
					aui_manager.GetPane(project_tree.treeCtrl).Show();
					project_tree.treeCtrl->ExpandAll();
				}
			}
		}
		IF_THERE_IS_SOURCE CURRENT_SOURCE->SetFocus();
		aui_manager.Update();
		// mostrar arbol de simbolos
//		if (!left_panels && project) {
//			symbols_tree.menuItem->Check(true);
//			aui_manager.GetPane(symbols_tree.treeCtrl).Show();
//			aui_manager.Update();
//		}
		
		parser->ParseProject(true);
		
	} else { // si era otro archivo
		// abrir el archivo
		int ans = 0;
		if (project && !project->HasFile(filename) &&  ( (multiple && (*multiple)==1) || ((!multiple || (*multiple)==0) &&
			mxMD_YES&(ans=mxMessageDialog(main_window,LANG(MAINW_ADD_TO_PROJECT_QUESTION,"Desea agregar el archivo al proyecto?"), filename.GetFullPath(), mxMD_QUESTION|mxMD_YES_NO,multiple?LANG(MAINW_ADD_TO_PROJECT_CHECK,"Hacer lo mismo para todos"):_T(""),false).ShowModal()) 
			) )) {
			OpenFile(filename.GetFullPath(),true);
		} else {
			OpenFile(filename.GetFullPath(),!project);
		}
		if (ans&mxMD_CHECKED) {
			(*multiple)=ans&mxMD_YES?1:2; // todo
		}
	}
	// actualizar el historial de archivos abiertos recientemente
	if (!project || filename.GetExt().CmpNoCase(_T(PROJECT_EXT))==0)
		UpdateInHistory(filename.GetFullPath());
	status_bar->SetStatusText(LANG(GENERAL_READY,"Listo"));
}
	
void mxMainWindow::UpdateInHistory(wxString filename) {
	wxString ipre=DIR_PLUS_FILE(_T("16"),_T("recent"));
	bool is_project = wxFileName(filename).GetExt().CmpNoCase(_T(PROJECT_EXT))==0;
	int i;
	wxString *cfglast = is_project?config->Files.last_project:config->Files.last_source;
	wxMenuItem **mnihistory = is_project?menu.file_project_history:menu.file_source_history;
	wxMenu *mnurecent = is_project?menu.file_project_recent:menu.file_source_recent;
	int history_id = is_project?mxID_FILE_PROJECT_HISTORY_0:mxID_FILE_SOURCE_HISTORY_0;
	
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
#if defined(_WIN32) || defined(__WIN32__)
		if (mnihistory[i])
			mnurecent->Remove(mnihistory[i]);
		if (cfglast[i][0])
			mnihistory[i] = utils->AddItemToMenu(mnurecent, history_id+i,cfglast[i],_T(""),cfglast[i],wxString(_T("recent"))<<i<<_T(".png"),i);
#else
		if (cfglast[i][0])
			if (mnihistory[i]) {
				if (wxFileName::FileExists(SKIN_FILE(wxString(ipre)<<i<<_T(".png"))))
					mnihistory[i]->SetBitmap(wxBitmap(SKIN_FILE(wxString(ipre)<<i<<_T(".png")),wxBITMAP_TYPE_PNG));
				mnurecent->SetLabel(mnihistory[i]->GetId(),cfglast[i]);
			} else {
				mnihistory[i] = utils->AddItemToMenu(mnurecent, history_id+i,cfglast[i],_T(""),cfglast[i],wxString(_T("recent"))<<i<<_T(".png"),i);
			}
		else
			break;
#endif
	}
}

void mxMainWindow::OnFileOpen (wxCommandEvent &event) {
	wxFileDialog dlg (this, _T("Abrir Archivo"), project?project->last_dir:config->Files.last_dir, _T(" "), _T("Any file (*)|*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	dlg.SetWildcard(_T("Archivos de C/C++ y Proyectos|"WILDCARD_CPP_EXT"|Fuentes|"WILDCARD_SOURCE"|Cabeceras|"WILDCARD_HEADER"|Proyectos|"WILDCARD_PROJECT"|Todos los archivos|*"));
	if (dlg.ShowModal() == wxID_OK) {
		if (project)
			project->last_dir=dlg.GetDirectory();
		else
			config->Files.last_dir=dlg.GetDirectory();
		wxArrayString paths;
		dlg.GetPaths(paths);
		int ans=0;
		if (paths.GetCount()==1)
			OpenFileFromGui(paths[0]);
		else
			for (unsigned int i=0;i<paths.GetCount();i++)
				OpenFileFromGui(paths[i],&ans);
	}
}

void mxMainWindow::OnFileSourceHistoryMore (wxCommandEvent &event) {
	new mxOpenRecentDialog(this,false);
}

void mxMainWindow::OnFileProjectHistoryMore (wxCommandEvent &event) {
	new mxOpenRecentDialog(this,true);
}

void mxMainWindow::OnFileSourceHistory (wxCommandEvent &event) {
	OpenFileFromGui(wxString(config->Files.last_source[event.GetId()-mxID_FILE_SOURCE_HISTORY_0]));
}

void mxMainWindow::OnFileProjectHistory (wxCommandEvent &event) {
	OpenFileFromGui(wxString(config->Files.last_project[event.GetId()-mxID_FILE_PROJECT_HISTORY_0]));
}

void mxMainWindow::OnFilePrint (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		if (!printDialogData) printDialogData=new wxPrintDialogData;
		mxSource *src=CURRENT_SOURCE;
		wxPrinter printer(printDialogData);
		mxPrintOut printout(src,src->page_text);
		src->SetPrintMagnification(config->Styles.print_size-config->Styles.font_size);
		src->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_NONE);
		if (!printer.Print(this, &printout, true)) {
			if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
				mxMessageDialog(this,LANG(MAINW_ERROR_PRITING,"Ha ocurrido un error al intentar imprimir"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		}
		src->SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_START|wxSTC_WRAPVISUALFLAG_END);
//		(*pageSetupData) = * printData;
//		wxPageSetupDialog pageSetupDialog(NULL, pageSetupData);
//		if (wxID_OK!=pageSetupDialog.ShowModal()) return;
//		*printData = pageSetupDialog.GetPageSetupData().GetPrintData();
//		*pageSetupData = pageSetupDialog.GetPageSetupData();	
		
//		wxPrintDialogData printDialogData(*printData);
//		wxPrinter printer(&printDialogData);
//		if (!printer.Print(this, &printout, true)) {
//			if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
//				mxMessageDialog(this,_T("Ha ocurrido un error al intentar imprimir"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();;
//		} else
//			(*printData) = printer.GetPrintDialogData().GetPrintData();
	}
}

void mxMainWindow::OnFileNewProject (wxCommandEvent &event) {
	if (!wizard) wizard = new mxNewWizard(this);
	wizard->RunWizard(_T("new_project"));
}

void mxMainWindow::OnFileNew (wxCommandEvent &event) {
	if (project) {
		if (!wizard) wizard = new mxNewWizard(this);
		wizard->RunWizard(_T("on_project"));
	} else 
		switch (config->Init.new_file){
			case 0:
				NewFileFromText(_T(""));
				break;
			case 1:
				main_window->NewFileFromTemplate(config->Files.default_template);
				break;
			default: {
				if (!wizard) wizard = new mxNewWizard(this);
				wizard->RunWizard();
				break;
			}
		}	
}

mxSource *mxMainWindow::NewFileFromText (wxString text, wxString name, int pos) {
	if (welcome_panel && notebook_sources->GetPageCount()==0) ShowWelcome(false);
	mxSource* source = new mxSource(notebook_sources, AvoidDuplicatePageText(name));
	source->AppendText(text);
	source->MoveCursorTo(pos);
	source->SetLineNumbers();
	notebook_sources->AddPage(source, name ,true, *bitmaps->files.blank);
	if (!project) source->treeId = AddToProjectTreeSimple(name,'s');
	source->SetModify(false);
	source->SetFocus();
	return source;
}

mxSource *mxMainWindow::NewFileFromText (wxString text, int pos) {
	return NewFileFromText(text,SIN_TITULO,pos);
}

mxSource *mxMainWindow::NewFileFromTemplate (wxString filename) {
	if (project) {
		mxMessageDialog(this,LANG(MAINW_CANT_OPEN_TEMPLATE_WHILE_PROJECT,"No puede abrir un ejemplo mientras trabaja en un proyecto.\nCierre el proyecto e intente nuevamente."),LANG(GENERAL_WARNING,"Advertencia"),mxMD_OK|mxMD_WARNING).ShowModal();;
		return NULL;
	}
	if (welcome_panel && notebook_sources->GetPageCount()==0) ShowWelcome(false);
	mxSource* source = new mxSource(notebook_sources, SIN_TITULO);
	wxTextFile file(filename);
	file.Open();
	if (file.IsOpened()) {
		long pos=0;
		wxString line = file.GetFirstLine();
		while (line.Left(7)==_T("// !Z! ")) {
			if (line.Left(13)==_T("// !Z! Caret:")) {
				line.Mid(13).Trim(false).Trim(true).ToLong(&pos);
			} else if (line.Left(15)==_T("// !Z! Options:")) {
				source->config_running.compiler_options=line.Mid(15).Trim(false).Trim(true);
			}
			line = file.GetNextLine();
		}
		if (line!=_T(""))
			source->AppendText(line+_T("\n"));
		while (!file.Eof()) 
			source->AppendText(file.GetNextLine()+_T("\n"));
		source->MoveCursorTo(pos);
		file.Close();
	}
	source->SetLineNumbers();
	notebook_sources->AddPage(source, LAST_TITULO ,true, *bitmaps->files.blank);
	if (!project) source->treeId = AddToProjectTreeSimple(LAST_TITULO,'s');
	source->SetModify(false);
	source->SetFocus();
	return source;
}

void mxMainWindow::OnFileSaveProject (wxCommandEvent &event) {
	project->Save();
}

void mxMainWindow::OnFileSave (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src = CURRENT_SOURCE;
		if (src->sin_titulo)
			OnFileSaveAs(event);
		else {
			src->SaveSource();
			char where;
			if (!project || (project->FindFromName(src->source_filename.GetFullPath(),&where) && where!='o'))
				parser->ParseSource(CURRENT_SOURCE,true);
		} 
	}
}

void mxMainWindow::OnFileSaveAs (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE  {
		mxSource *source=CURRENT_SOURCE;
		wxFileDialog dlg (this, LANG(GENERAL_SAVE,"Guardar"),source->sin_titulo?wxString(wxFileName::GetHomeDir()):wxFileName(source->source_filename).GetPath(),source->sin_titulo?wxString(wxEmptyString):wxFileName(source->source_filename).GetFullName(), _T("Any file (*)|*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		dlg.SetDirectory(source->sin_titulo?wxString(project?project->last_dir:config->Files.last_dir):wxFileName(source->source_filename).GetPath());
		dlg.SetWildcard(_T("Todos los archivos|*|Archivos de C/C++|"WILDCARD_CPP"|Fuentes|"WILDCARD_SOURCE"|Cabeceras|"WILDCARD_HEADER));
//		dlg.SetFilterIndex(1);
		if (dlg.ShowModal() == wxID_OK) {
			wxFileName file = dlg.GetPath();
			if (!project) {
				if (file.GetExt().Len()==0) {
					bool add=config->Init.always_add_extension;
					if (!add) {
						int res = mxMessageDialog(this,LANG(MAINW_NO_EXTENSION_ADD_CPP_QUESTION,"No ha definido una extension en el nombre de archivo indicado.\nSi es un codigo fuente se recomienda utilizar la extension cpp\npara que el compilador pueda identificar el lenguaje.\nDesea agregar la extension cpp al nombre?"),LANG(GENERAL_WARNING,"Advertencia"),mxMD_YES_NO,LANG(MAINW_ALWAYS_APPEND_EXTENSION,"Siempre agregar la extension sin preguntar."),false).ShowModal();;
						if (res&mxMD_CHECKED)
							config->Init.always_add_extension=true;
						add=res&mxMD_YES;
					}
					if (add)
						file.SetExt(_T("cpp"));
				}
			}
			if (source->SaveSource(file)) {
				parser->RenameSource(source,file);
				wxString filename = file.GetFullName();
				char ftype=utils->GetFileType(filename);
				source->SetPageText(filename);
				if (project)
					project->last_dir=dlg.GetDirectory();
				else
					config->Files.last_dir=dlg.GetDirectory();
				if (!project) {
					if (ftype=='s') {
						notebook_sources->SetPageBitmap(notebook_sources->GetSelection(),*bitmaps->files.source);
						source->SetStyle(wxSTC_LEX_CPP);
						if (project_tree.treeCtrl->GetItemParent(source->treeId)!=project_tree.sources) {
							project_tree.treeCtrl->Delete(source->treeId);
							source->treeId = project_tree.treeCtrl->AppendItem(project_tree.sources, filename, 1);
						} else
							project_tree.treeCtrl->SetItemText(source->treeId,filename);
						project_tree.treeCtrl->Expand(project_tree.sources);
					} else if (ftype=='h') {
						notebook_sources->SetPageBitmap(notebook_sources->GetSelection(),*bitmaps->files.header);
						source->SetStyle(wxSTC_LEX_CPP);
						if (project_tree.treeCtrl->GetItemParent(source->treeId)!=project_tree.headers) {
							project_tree.treeCtrl->Delete(source->treeId);
							source->treeId = project_tree.treeCtrl->AppendItem(project_tree.headers, filename, 2);
						} else
							project_tree.treeCtrl->SetItemText(source->treeId,filename);
						project_tree.treeCtrl->Expand(project_tree.headers);
					} else {
						wxString ext=file.GetExt();
						notebook_sources->SetPageBitmap(notebook_sources->GetSelection(),*bitmaps->files.other);
						if (ext==_T("HTM") || ext==_T("HTML"))
							source->SetStyle(wxSTC_LEX_HTML);
						else if (ext==_T("XML"))
							source->SetStyle(wxSTC_LEX_XML);
						else if (ext==_T("SH"))
							source->SetStyle(wxSTC_LEX_BASH);
						else if (file.GetName().MakeUpper()==_T("MAKEFILE"))
							source->SetStyle(wxSTC_LEX_MAKEFILE);
						if (project_tree.treeCtrl->GetItemParent(source->treeId)!=project_tree.others) {
							project_tree.treeCtrl->Delete(source->treeId);
							source->treeId = project_tree.treeCtrl->AppendItem(project_tree.others, filename, 3);
							project_tree.treeCtrl->Expand(project_tree.others);
						} else { 
							project_tree.treeCtrl->SetItemText(source->treeId,filename);
						}
					}
				}
				if (!project) {
					UpdateInHistory(file.GetFullPath());
					parser->ParseSource(source,true);
				}
	/*			if (symbols_tree.menuItem->IsChecked())
					parser->ParseSource(source);*/
			}
			if (wxFileName(file.GetPath())==explorer_tree.path) 
				SetExplorerPath(explorer_tree.path);
		}
	}
}

void mxMainWindow::UpdateSymbols () {
	if (project) {
		project->UpdateSymbols();
//		project->SaveAll(false);
	} else
		IF_THERE_IS_SOURCE
			parser->ParseSource(CURRENT_SOURCE);
//			parser->ParseFile(CURRENT_SOURCE->source_filename);
}

void mxMainWindow::OnSocketEvent(wxSocketEvent &event){
	if (share) share->OnSocketEvent(&event);
}

void mxMainWindow::OnParseSourceTime(wxTimerEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		if (!project && (source->never_parsed || source->GetModify()))
			parser->ParseSource(source);
	}
}

void mxMainWindow::OnParseOutputTime(wxTimerEvent &event) {
	compile_and_run_struct_single *compile_and_run=compiler->compile_and_run_single;
	while (compile_and_run) {
		if (compile_and_run->process)
			compiler->ParseSomeErrors(compile_and_run);
		compile_and_run=compile_and_run->next;
	}
}





void mxMainWindow::OnFoldFold(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		int line = source->GetCurrentLine();
		if (source->GetFoldExpanded(line)) {
			source->ToggleFold(line);
		}
	}
}
void mxMainWindow::OnFoldUnFold(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		int line = source->GetCurrentLine();
		if (!source->GetFoldExpanded(line)) {
			source->ToggleFold(line);
		}
	}
}

void mxMainWindow::OnFoldShow1(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(1,false);
}
void mxMainWindow::OnFoldShow2(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(2,false);
}
void mxMainWindow::OnFoldShow3(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(3,false);
}
void mxMainWindow::OnFoldShow4(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(4,false);
}
void mxMainWindow::OnFoldShow5(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(5,false);
}
void mxMainWindow::OnFoldShowAll(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(0,false);
}

void mxMainWindow::OnFoldHide1(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(1,true);
}
void mxMainWindow::OnFoldHide2(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(2,true);
}
void mxMainWindow::OnFoldHide3(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(3,true);
}
void mxMainWindow::OnFoldHide4(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(4,true);
}
void mxMainWindow::OnFoldHide5(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(5,true);
}
void mxMainWindow::OnFoldHideAll(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->SetFolded(0,true);
}

/// @brief inserta el include correspondiente a la palabra sobre el cursor si lo conoce y no estaba
void mxMainWindow::OnEditInsertInclude(wxCommandEvent &event) {
	
	IF_THERE_IS_SOURCE {
		mxSource *source=CURRENT_SOURCE;
		// separar la palabra
		int pos=source->GetCurrentPos();
		if (source->GetCharAt(pos)==')') pos=source->BraceMatch(pos)-1; // si esta en un parentesis que cierrar, puede ser donde termina una funcion, buscar el nombre
		if (pos<0) { // si no hay palabra quejarse
			mxMessageDialog(main_window,LANG(MAINW_INSERT_HEADIR_NO_WORD,"Debe colocar el cursor de texto sobre el nombre de la clase que desee incluir."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_INFO).ShowModal();
			return;
		}
		int s=source->WordStartPosition(pos,true);
		int e=source->WordEndPosition(pos,true);
		wxString key = source->GetTextRange(s,e);
		
		if (key.Len()==0) { // si no hay palabra quejarse
			mxMessageDialog(main_window,LANG(MAINW_INSERT_HEADIR_NO_WORD,"Debe colocar el cursor de texto sobre el nombre de la clase que desee incluir."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_INFO).ShowModal();
			return;
		} else { // conseguir el h y darselo al source para que haga lo que corresponda
			wxString header = code_helper->GetInclude(source->sin_titulo?wxString(_T("")):source->source_filename.GetPathWithSep(),key);
			if (!header.Len()) {
				wxString bkey=key, type = source->FindTypeOf(e-1,s);
				if ( s!=SRC_PARSING_ERROR && type.Len() ) {
					header = code_helper->GetInclude(source->sin_titulo?wxString(_T("")):source->source_filename.GetPathWithSep(),type);
				} else { // buscar el scope y averiguar si es algo de la clase
					type = source->FindScope(s);
					if (type.Len()) {
						type = code_helper->GetAttribType(type,bkey,s);
						if (!type.Len())
							type=code_helper->GetGlobalType(bkey,s);
					} else {
						type=code_helper->GetGlobalType(bkey,s);
					}
					if (type.Len()) {
						header=code_helper->GetInclude(source->sin_titulo?wxString(_T("")):source->source_filename.GetPathWithSep(),type);
					}
				}
			}
			if (header.Len()) {
				if (utils->GetFileType(header)=='s')
					mxMessageDialog(main_window,key+LANG(MAINW_INSERT_HEADIR_CPP," esta declarada en un archivo fuente. Solo deben realizarse #includes para archivos de cabecera."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_INFO).ShowModal();
				else
					source->AddInclude(header);
			} else if (key==_T("Clippo"))
				new mxSplashScreen(clpeg,GetPosition().x+GetSize().x-215,GetPosition().y+GetSize().y-230);
			else
				mxMessageDialog(main_window,wxString(LANG(MAINW_NO_HEADER_FOR_PRE,"No se encontro cabecera correspondiente a \""))+key+LANG(MAINW_NO_HEADER_FOR_POST,"\""),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
		}
	}
}

void mxMainWindow::OnViewNotebookNext(wxCommandEvent &evt){
	unsigned int i=notebook_sources->GetSelection();
	i++; 
	if (i>=notebook_sources->GetPageCount())
		i=0;
	notebook_sources->SetSelection(i);
}

void mxMainWindow::OnViewNotebookPrev(wxCommandEvent &evt){
	int i=notebook_sources->GetSelection();
	i--; 
	if (i<0)
		i=notebook_sources->GetPageCount()-1;
	notebook_sources->SetSelection(i);
}

wxStatusBar* mxMainWindow::OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name) {
	status_bar = new mxStatusBar(this, id, style, name);
	SetStatusBarFields();
	return status_bar;
}


void mxMainWindow::OnDebugAttach ( wxCommandEvent &event ) {
	ABORT_IF_PARSING;
	long dpid=0;
	wxArrayString options;
	wxString otro=_T("<<<Otro>>>"),cual;
	if (!dpid) utils->GetRunningChilds(options);
	options.Add(cual=otro);
	if (options.GetCount()>1) {
		cual=wxGetSingleChoice(_T("Proceso:"),_T("Adjuntar Depurador"),options,this,-1,-1,true);
		if (!cual.Len()) return;
	}
	if (cual==otro) 
		mxGetTextFromUser(_T("PID:"),_T("Adjuntar Depurador"),_T(""),this).ToLong(&dpid);
	else
		cual.BeforeFirst(' ').ToLong(&dpid);
	if (!dpid) return;
	if (project) 
		debug->Attach(dpid,NULL);
	else IF_THERE_IS_SOURCE
		debug->Attach(dpid,CURRENT_SOURCE);
}

/** 
* inicia la depuracion ejecutando el programa
* o reanuda la depuracion si ya estaba en proceso pero interrumpida
**/
void mxMainWindow::OnDebugRun ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE {
		CURRENT_SOURCE->AutoCompCancel();
		CURRENT_SOURCE->CallTipCancel();
	}
	if (debug->debugging) {
		if (!debug->running && !debug->waiting)
			debug->Continue();
	} else {
		compiler_tree.treeCtrl->SetItemText(compiler_tree.state,_T("Preparando depuracion..."));
		status_bar->SetStatusText(_T("Preparando depuracion..."));
		wxYield();
		if (project) {
			debug->Start(config->Debug.compile_again);
		} else IF_THERE_IS_SOURCE {
			mxSource *source=CURRENT_SOURCE;
			// si es un .h, preguntar si no es mejor ejecutar un cpp
			wxString ext=source->sin_titulo||compiler->last_runned==source?wxString(_T("")):source->source_filename.GetExt().MakeUpper();
			if (ext==_T("H") || ext==_T("HPP") || ext==_T("HXX") || ext==_T("H++")) {
				if (source->GetModify())
					source->SaveSource();
				if (compiler->last_runned) {
					if (config->Running.dont_run_headers)
						source=compiler->last_runned;
					else {
						int ans = mxMessageDialog(this,wxString(_T("Esta intentando ejecutar un archivo de cabecera.\n"
							"Desea ejecutar en su lugar "))<<compiler->last_caption<<_T("?"),
							LANG(GENERAL_WARNING,"Aviso"),mxMD_YES|mxMD_NO|mxMD_CANCEL,_T("Siempre ejecutar el ultimo cpp.")).ShowModal();
						if (ans&mxMD_CANCEL) return;
						if (ans&mxMD_CHECKED) config->Running.dont_run_headers = true;
						if (ans&mxMD_YES) source = compiler->last_runned;
					}
				} else {
					int ans = mxMessageDialog(this,_T("Esta intentando ejecutar un archivo de cabecera. Desea continuar?"),LANG(GENERAL_WARNING,"Aviso"),mxMD_YES|mxMD_CANCEL).ShowModal();
					if (ans&mxMD_CANCEL) return;
				}
			}				
			debug->Start(config->Debug.compile_again,source);
		}
	}
//	st->Destroy();
}

void mxMainWindow::OnDebugPause ( wxCommandEvent &event ) {
	debug->Pause();
}

void mxMainWindow::OnDebugContinue ( wxCommandEvent &event ) {
	debug->Continue();
}


void mxMainWindow::OnDebugStop ( wxCommandEvent &event ) {
	if (!debug->debugging)
		OnRunStop(event);
	else
		debug->Stop();
}

void mxMainWindow::OnDebugInspect ( wxCommandEvent &event ) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(inspection_ctrl).IsShown())
			autohide_handlers[ATH_INSPECTIONS]->ForceShow();
	} else {	
		if ( !aui_manager.GetPane((wxGrid*)inspection_ctrl).IsShown() )
			aui_manager.GetPane((wxGrid*)inspection_ctrl).Show();
		aui_manager.Update();
	}
	inspection_ctrl->SetFocus();
	wxKeyEvent evt;
	evt.m_keyCode=WXK_INSERT;
	inspection_ctrl->OnKey(evt);
}

void mxMainWindow::OnDebugBacktrace ( wxCommandEvent &event ) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(backtrace_ctrl).IsShown())
			autohide_handlers[ATH_BACKTRACE]->ForceShow();
	} else {	
		aui_manager.GetPane(backtrace_ctrl).Show();
		aui_manager.Update();
	}
//	debug->backtrace_visible=true;
	debug->Backtrace();
	backtrace_ctrl->SetFocus();
//	backtrace_ctrl->SelectRow(0);
}

void mxMainWindow::OnDebugThreadList ( wxCommandEvent &event ) {
	mxMessageDialog(main_window,LANG(DEBUG_THREAD_SYSTEM_INCOMPLETE,"El manejo de hilos en la depuracion aun esta en construccion.\n"
									"Puede que los niveles en las inspecciones no se correspondan con\n"
									"el trazado inverso al cambiar de hilo."),LANG(GENERAL_WARNING,"Advertencia"),mxMD_OK|mxMD_WARNING).ShowModal();
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(autohide_handlers[ATH_THREADS]).IsShown()) {
			aui_manager.GetPane(autohide_handlers[ATH_THREADS]).Show();
			aui_manager.Update();
		}
		if (!aui_manager.GetPane(threadlist_ctrl).IsShown())
			autohide_handlers[ATH_THREADS]->ForceShow();
		debug->threadlist_visible=true;
		debug->ListThreads();
	} else {
		aui_manager.GetPane(threadlist_ctrl).Show();
		aui_manager.Update();
		debug->threadlist_visible=true;
		debug->ListThreads();
		threadlist_ctrl->SetFocus();
	}
}

void mxMainWindow::OnDebugStepIn ( wxCommandEvent &event ) {
	debug->StepIn();
}

void mxMainWindow::OnDebugStepOver ( wxCommandEvent &event ) {
	debug->StepOver();
}

void mxMainWindow::OnDebugReturn ( wxCommandEvent &event ) {
	if (debug->debugging && !debug->waiting) {
		wxString res;
		if (mxGetTextFromUser(res,LANG(DEBUG_RETURN_VALUE,"Valor de retorno:"), LANG(DEBUG_RETURN_FROM_FUNCTION,"Salir de la funcion") , _T(""), this))
			debug->Return(res);
	}
}

void mxMainWindow::OnDebugJump ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		debug->Jump((source->sin_titulo?source->temp_filename:source->source_filename).GetFullPath(),source->GetCurrentLine()+1);
	}
}

void mxMainWindow::OnDebugRunUntil ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		debug->RunUntil(source->source_filename.GetFullPath(),source->GetCurrentLine()+1);
	}
}

void mxMainWindow::OnDebugListWatchpoints ( wxCommandEvent &event ) {
	
}

void mxMainWindow::OnDebugListBreakpoints ( wxCommandEvent &event ) {
	new mxBreakList();
}

void mxMainWindow::OnDebugInsertWatchpoint ( wxCommandEvent &event ) {
	
}

void mxMainWindow::OnDebugToggleBreakpoint ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		wxStyledTextEvent evt;
		evt.SetMargin(1);
		evt.SetPosition(source->GetCurrentPos());
		source->OnMarginClick(evt);
	}
	
}

void mxMainWindow::OnDebugBreakpointOptions ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE {
		mxSource *source=CURRENT_SOURCE;
		int l = source->LineFromPosition (source->GetCurrentPos());
		if (!debug->debugging || !debug->waiting)
			new mxBreakOptions(source->sin_titulo?source->temp_filename.GetFullPath():source->source_filename.GetFullPath(),l,source);
	}
}

void mxMainWindow::OnDebugFunctionBreakpoint ( wxCommandEvent &event ) {
	
}

void mxMainWindow::OnDebugStepOut ( wxCommandEvent &event ) {
	debug->StepOut();
}


void mxMainWindow::OnDebugDoThat ( wxCommandEvent &event ) {
	static wxString what;
	wxString res = mxGetTextFromUser(_T("Comando:"), _T("Comandos internos") , what, this);
	if (res=="kboom") {
		int *p=NULL;
		// cppcheck-suppress nullPointer
		cout<<*p;
	}
	if (debug->debugging && res.Len()) debug->DoThat(what=res);
}

void mxMainWindow::ShowInQuickHelpPanel(wxString &res, bool hide_compiler_tree) {
	quick_help->SetPage(res);
	ShowQuickHelpPanel(hide_compiler_tree);
}

void mxMainWindow::LoadInQuickHelpPanel(wxString file, bool hide_compiler_tree) {
	quick_help->LoadPage(file);
	ShowQuickHelpPanel(hide_compiler_tree);
}


void mxMainWindow::PrepareGuiForDebugging(bool debug_mode) {
	static bool backtrace_visible=true;
	static bool inspections_visible=true;
	static bool threads_visible=true;
	static bool log_visible=true;
	
	wxCommandEvent evt;
	if (debug_mode) { // si comienza la depuracion
		// habilitar y deshabilitar cosas en los menues
		menu.debug_stop->Enable(true);
		menu.debug_return->Enable(true);
		menu.debug_step_out->Enable(true);
		menu.debug_step_over->Enable(true);
		menu.debug_step_in->Enable(true);
		menu.debug_run_until->Enable(true);
		menu.debug_jump->Enable(true);
//		menu.debug_list_breakpoints->Enable(true);
		menu.debug_pause->Enable(true);
#if !defined(_WIN32) && !defined(__WIN32__)
		menu.debug_attach->Enable(false);
		menu.debug->SetLabel(mxID_DEBUG_CORE_DUMP,LANG(MENUITEM_SAVE_CORE_DUMP,"Guardar &Volcado de Memoria..."));
		menu.debug_inverse_execution->Enable(false);
		menu.debug_inverse_execution->Check(false);
		menu.debug_enable_inverse_execution->Enable(true);
		menu.debug_enable_inverse_execution->Check(false);
		main_window->toolbar_debug->EnableTool(mxID_DEBUG_INVERSE_EXEC,false);
		main_window->toolbar_debug->ToggleTool(mxID_DEBUG_INVERSE_EXEC,false);
		main_window->toolbar_debug->ToggleTool(mxID_DEBUG_ENABLE_INVERSE_EXEC,false);
#endif
		
		if (!config->Debug.allow_edition) { // no permitir editar los fuentes durante la depuracion
			for (unsigned int i=0;i<notebook_sources->GetPageCount();i++) 
				((mxSource*)(notebook_sources->GetPage(i)))->SetDebugTime(true);
		}

		if (config->Debug.autohide_toolbars) { // reacomodar las barras de herramientas
			if ( !(debug_toolbars_status[0]=menu.view_toolbar_debug->IsChecked()) ) {
				menu.view_toolbar_debug->Check(true);
				aui_manager.GetPane(toolbar_debug).Show();
			}
			aui_manager.GetPane(toolbar_status).Show();
			if ( (debug_toolbars_status[1]=menu.view_toolbar_file->IsChecked()) ) {
				menu.view_toolbar_file->Check(false);
				aui_manager.GetPane(toolbar_file).Hide();
			}
			if ( (debug_toolbars_status[2]=menu.view_toolbar_run->IsChecked()) ) {
				menu.view_toolbar_run->Check(false);
				aui_manager.GetPane(toolbar_run).Hide();
			}
			if ( (debug_toolbars_status[3]=menu.view_toolbar_edit->IsChecked()) ) {
				menu.view_toolbar_edit->Check(false);
				aui_manager.GetPane(toolbar_edit).Hide();
			}
			if ( (debug_toolbars_status[4]=menu.view_toolbar_misc->IsChecked()) ) {
				menu.view_toolbar_misc->Check(false);
				aui_manager.GetPane(toolbar_misc).Hide();
			}
			if ( (debug_toolbars_status[5]=menu.view_toolbar_find->IsChecked()) ) {
				menu.view_toolbar_find->Check(false);
				aui_manager.GetPane(toolbar_find).Hide();
			}
			if ( (debug_toolbars_status[6]=menu.view_toolbar_tools->IsChecked()) ) {
				menu.view_toolbar_tools->Check(false);
				aui_manager.GetPane(toolbar_tools).Hide();
			}
			if ( (debug_toolbars_status[7]=menu.view_toolbar_view->IsChecked()) ) {
				menu.view_toolbar_view->Check(false);
				aui_manager.GetPane(toolbar_view).Hide();
			}
		}
		
		if (config->Debug.autohide_panels) { // reacomodar los paneles
			if (config->Init.autohiding_panels) {
				autohide_handlers[ATH_QUICKHELP]->Hide();
				autohide_handlers[ATH_COMPILER]->Hide();
				if (backtrace_visible) autohide_handlers[ATH_BACKTRACE]->Select();
				if (inspections_visible) autohide_handlers[ATH_INSPECTIONS]->Select();
				if (threads_visible && aui_manager.GetPane(autohide_handlers[ATH_THREADS]).IsShown())
					autohide_handlers[ATH_THREADS]->Select();
				if (log_visible && aui_manager.GetPane(autohide_handlers[ATH_DEBUG_LOG]).IsShown())
					autohide_handlers[ATH_DEBUG_LOG]->Select();
			} else {
				if ( (debug_panels_status[0]=aui_manager.GetPane(compiler_tree.treeCtrl).IsShown()) ) {
					aui_manager.GetPane(compiler_tree.treeCtrl).Hide();
					compiler_tree.menuItem->Check(false);
				}
				if ( (debug_panels_status[1]=aui_manager.GetPane(quick_help).IsShown()) ) {
					aui_manager.GetPane(quick_help).Hide();
				}
				if ( backtrace_visible && !(debug_panels_status[2]=aui_manager.GetPane(backtrace_ctrl).IsShown()) )
					aui_manager.GetPane(backtrace_ctrl).Show();
				if ( inspections_visible && !(debug_panels_status[3]=aui_manager.GetPane((wxGrid*)inspection_ctrl).IsShown()) )
					aui_manager.GetPane((wxGrid*)inspection_ctrl).Show();
				if ( threads_visible && config->Debug.show_thread_panel &&  !(debug_panels_status[4]=aui_manager.GetPane((wxGrid*)threadlist_ctrl).IsShown()) ) {
					debug->threadlist_visible=true;
					aui_manager.GetPane((wxGrid*)threadlist_ctrl).Show();
				}
//			debug->backtrace_visible=true;
				debug->BacktraceClean();
				
				if (log_visible && config->Debug.show_log_panel) aui_manager.GetPane(debug_log_panel).Show();
				
			}
		}
		
	} else { // si finaliza la depuracion
		
		// habilitar y deshabilitar cosas en los menues
		menu.debug_stop->Enable(false);
		menu.debug_return->Enable(false);
		menu.debug_step_out->Enable(false);
		menu.debug_step_in->Enable(false);
		menu.debug_step_over->Enable(false);
		menu.debug_run_until->Enable(false);
		menu.debug_jump->Enable(false);
//		menu.debug_list_breakpoints->Enable(false);
		menu.debug_pause->Enable(false);
#if !defined(_WIN32) && !defined(__WIN32__)
		menu.debug_attach->Enable(true);
		menu.debug->SetLabel(mxID_DEBUG_CORE_DUMP,LANG(MENUITEM_LOAD_CORE_DUMP,"Cargar &Volcado de Memoria..."));
		menu.debug_inverse_execution->Enable(false);
		menu.debug_enable_inverse_execution->Enable(false);
#endif
		// reestablecer la edicion de fuentes
		for (unsigned int i=0;i<notebook_sources->GetPageCount();i++)
			((mxSource*)(notebook_sources->GetPage(i)))->SetDebugTime(false);
		
		if (config->Debug.autohide_toolbars) { // reacomodar las barras de herramientas
			if (menu.view_toolbar_debug->IsChecked()!=debug_toolbars_status[0]) {
				if (debug_toolbars_status[0]) {
					menu.view_toolbar_debug->Check(true);
					aui_manager.GetPane(toolbar_debug).Show();
				} else {
					menu.view_toolbar_debug->Check(false);
					aui_manager.GetPane(toolbar_debug).Hide();
				}
			}
			aui_manager.GetPane(toolbar_status).Hide();
			if (menu.view_toolbar_file->IsChecked()!=debug_toolbars_status[1]) {
				if (debug_toolbars_status[1]) {
					menu.view_toolbar_file->Check(true);
					aui_manager.GetPane(toolbar_file).Show();
				} else {
					menu.view_toolbar_file->Check(false);
					aui_manager.GetPane(toolbar_file).Hide();
				}
			}
			if (menu.view_toolbar_run->IsChecked()!=debug_toolbars_status[2]) {
				if (debug_toolbars_status[2]) {
					menu.view_toolbar_run->Check(true);
					aui_manager.GetPane(toolbar_run).Show();
				} else {
					menu.view_toolbar_run->Check(false);
					aui_manager.GetPane(toolbar_run).Hide();
				}
			}
			if (menu.view_toolbar_edit->IsChecked()!=debug_toolbars_status[3]) {
				if (debug_toolbars_status[3]) {
					menu.view_toolbar_edit->Check(true);
					aui_manager.GetPane(toolbar_edit).Show();
				} else {
					menu.view_toolbar_edit->Check(false);
					aui_manager.GetPane(toolbar_edit).Hide();
				}
			}
			if (menu.view_toolbar_misc->IsChecked()!=debug_toolbars_status[4]) {
				if (debug_toolbars_status[4]) {
					menu.view_toolbar_misc->Check(true);
					aui_manager.GetPane(toolbar_misc).Show();
				} else {
					menu.view_toolbar_misc->Check(false);
					aui_manager.GetPane(toolbar_misc).Hide();
				}
			}
			if (menu.view_toolbar_find->IsChecked()!=debug_toolbars_status[5]) {
				if (debug_toolbars_status[5]) {
					menu.view_toolbar_find->Check(true);
					aui_manager.GetPane(toolbar_find).Show();
				} else {
					menu.view_toolbar_find->Check(false);
					aui_manager.GetPane(toolbar_find).Hide();
				}
			}
			if (menu.view_toolbar_tools->IsChecked()!=debug_toolbars_status[6]) {
				if (debug_toolbars_status[6]) {
					menu.view_toolbar_tools->Check(true);
					aui_manager.GetPane(toolbar_tools).Show();
				} else {
					menu.view_toolbar_tools->Check(false);
					aui_manager.GetPane(toolbar_tools).Hide();
				}
			}
			if (menu.view_toolbar_view->IsChecked()!=debug_toolbars_status[7]) {
				if (debug_toolbars_status[7]) {
					menu.view_toolbar_view->Check(true);
					aui_manager.GetPane(toolbar_view).Show();
				} else {
					menu.view_toolbar_view->Check(false);
					aui_manager.GetPane(toolbar_view).Hide();
				}
			}
		}
		
		if (config->Debug.autohide_panels) { // reacomodar los paneles
			if (config->Init.autohiding_panels) {
				inspections_visible=autohide_handlers[ATH_INSPECTIONS]->IsDocked();
				threads_visible=autohide_handlers[ATH_THREADS]->IsDocked();
				log_visible=autohide_handlers[ATH_DEBUG_LOG]->IsDocked();
				backtrace_visible=autohide_handlers[ATH_BACKTRACE]->IsDocked();
				autohide_handlers[ATH_BACKTRACE]->Hide();
				autohide_handlers[ATH_INSPECTIONS]->Hide();
				if (aui_manager.GetPane(autohide_handlers[ATH_THREADS]).IsShown())
					autohide_handlers[ATH_THREADS]->Hide();
				if (aui_manager.GetPane(autohide_handlers[ATH_DEBUG_LOG]).IsShown())
					autohide_handlers[ATH_DEBUG_LOG]->Hide();
			} else {
				
				if ( debug_panels_status[0]!=aui_manager.GetPane(compiler_tree.treeCtrl).IsShown() ) {
					if (debug_panels_status[0]) {
						aui_manager.GetPane(compiler_tree.treeCtrl).Show();
						compiler_tree.menuItem->Check(true);
					} else {
						aui_manager.GetPane(compiler_tree.treeCtrl).Hide();
						compiler_tree.menuItem->Check(false);
					}
				}
				if ( debug_panels_status[1]!=aui_manager.GetPane(quick_help).IsShown() ) {
					if (debug_panels_status[1])
						aui_manager.GetPane(quick_help).Show();
					else
						aui_manager.GetPane(quick_help).Hide();
				}
				
				threads_visible=aui_manager.GetPane((wxGrid*)threadlist_ctrl).IsShown();
				log_visible=aui_manager.GetPane(debug_log_panel).IsShown();
				backtrace_visible=aui_manager.GetPane(backtrace_ctrl).IsShown();
				inspections_visible=aui_manager.GetPane((wxGrid*)inspection_ctrl).IsShown();
				
				if ( !debug_panels_status[2] )
					aui_manager.GetPane(backtrace_ctrl).Hide();
				if ( !debug_panels_status[3] )
					aui_manager.GetPane((wxGrid*)inspection_ctrl).Hide();
				if ( !debug_panels_status[4] ) {
					debug->threadlist_visible=false;
					aui_manager.GetPane((wxGrid*)threadlist_ctrl).Hide();
				}
				
				aui_manager.GetPane(debug_log_panel).Hide();
				
			}
		}

	}
	
	if (config->Debug.autohide_toolbars) {
		fullscreen_toolbars_status[1]=!debug_mode&&config->Toolbars.wich_ones.file;
		fullscreen_toolbars_status[2]=!debug_mode&&config->Toolbars.wich_ones.run;
		fullscreen_toolbars_status[3]=!debug_mode&&config->Toolbars.wich_ones.edit;
		fullscreen_toolbars_status[4]=!debug_mode&&config->Toolbars.wich_ones.misc;
		fullscreen_toolbars_status[5]=!debug_mode&&config->Toolbars.wich_ones.find;
		fullscreen_toolbars_status[6]=debug_mode;
		fullscreen_toolbars_status[7]=!debug_mode&&config->Toolbars.wich_ones.tools;
		fullscreen_toolbars_status[8]=!debug_mode&&config->Toolbars.wich_ones.view;
	}
	
	aui_manager.Update(); // mostrar cambios en la interfaz
}

/// @brief muestra en que funcion/clase estamos (Ctrl+Shift+Space)
void mxMainWindow::OnWhereAmI(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		if (src->config_source.syntaxEnable) {
			wxString where;
			if (!src->sin_titulo) {
				wxFileName fname=src->source_filename;
				fname.Normalize();
				where<<fname.GetFullPath()<<" - "<<LANG(FIND_LINE,"linea")<<" "<<src->GetCurrentLine()<<" col "<<
					src->GetCurrentPos()-src->PositionFromLine(src->GetCurrentLine())<<"\n";
			}
			where<<src->WhereAmI();
			src->ShowBaloon(where);
		}
	}
}

/**
* esta funcion responde al timer mxID_FOCUS_TIMER y es un parche para 
* windows, que le da el foco al fuente medio segundo despues de que
* se hizo doble click en el error en el panel compiler_output, ya que
* utilizar el metodo SetFocus en el evento no funciona en Windows
**/
void mxMainWindow::OnFocusTime(wxTimerEvent &event) {
	main_window->Raise();
	IF_THERE_IS_SOURCE 
		CURRENT_SOURCE->SetFocus();
}

void mxMainWindow::OnGotoToolbarFind (wxCommandEvent &evt) {
	if (FindFocus()==toolbar_find_text)
		FocusToSource();
	else
		toolbar_find_text->SetFocus();
}

void mxMainWindow::OnToolbarFindEnter (wxCommandEvent &evt) {
	wxString stext = toolbar_find_text->GetValue();
	if (stext.Len()==0)
		return;
	if (!project && stext==_T("Cuack Attack!")) { // are you looking for a duck?
		toolbar_find_text->SetValue(_T(""));
		NewFileFromText(_T(
"#include <iostream>\n#include <cstdlib>\n#include <ctime>\nusing namespace std;\n"
"char *cuack() {\nstatic char cuack[] = \"cuack\";\nfor (int i=0;i<4;i++)\nif (rand()%2)\ncuack[i]=cuack[i]|32;\nelse\ncuack[i]=cuack[i]&(~32);\nreturn cuack;\n}\n"
"int main(int argc, char *argv[]) {\nsrand(time(0));\nwhile (true) {\nclock_t t1=clock();\nwhile (clock()==t1);\ncout<<string(rand()%50,' ')<<cuack()<<\"!\"<<endl;\n}\nreturn 0;\n}"
			),_T("Cuack Attack!"));
		wxYield();
		CURRENT_SOURCE->OnEditSelectAll(evt);
		CURRENT_SOURCE->OnIndentSelection(evt);
		OnRunRun(evt);
		return;
	}
	if (!project && stext==_T("moonwalk")) { // are you looking for a duck?
		NewFileFromText(_T("\n\n\n\n\n\n\n"));
		parser_timer->Stop();
		wxYield();
		mxSource *src=CURRENT_SOURCE;
		src->SetModify(false);
		int delay=200;
		for (int i=0;i<20;i++) {
			wxString s(_T("\n")), d(i,' ');
			s<<d<<_T(" _A_\n")<<d<<_T("  O \n")<<d<<_T("  |\\\n")<<d<<_T("  |/\n");
			if (i%4==0)	s<<d<<_T(" /|\n")<<d<<_T(" \\|\n");
			if (i%4==1) s<<d<<_T(" /|\n")<<d<<_T(" ||\n");
			if (i%4==2) s<<d<<_T(" /|\n")<<d<<_T("/ |\n");
			if (i%4==3) s<<d<<_T(" /|\n")<<d<<_T(" |\\\n");
			src->SetText(s); wxYield();	wxMilliSleep(delay);
		}
		wxString d(23,' ');
		src->SetText(wxString()<<_T("\n")<<d<<_T(" _A_\n")<<d<<_T("  O |\n")<<d<<_T(" /|/\n")<<d<<_T(" || \n")<<d<<_T(" / \\\n")<<d<<_T(" | |\n"));
		wxYield(); wxMilliSleep(delay*2);
		src->SetText(wxString()<<_T("\n")<<d<<_T("\\\n")<<d<<_T(" \\O \n")<<d<<_T("  |\\\n")<<d<<_T("  |_A_\n")<<d<<_T(" / \\\n")<<d<<_T(" | |\n"));
		wxYield(); wxMilliSleep(delay*3);
		src->SetModify(false);
		OnFileClose(evt);
		return;
	}
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		int pos = source->FindText(source->GetSelectionEnd(),source->GetLength(),toolbar_find_text->GetValue(),0);
		if (pos==wxSTC_INVALID_POSITION) 
			pos = source->FindText(0,source->GetSelectionEnd()+stext.Len(),toolbar_find_text->GetValue(),0);
		if (pos!=wxSTC_INVALID_POSITION) {
			source->EnsureVisibleEnforcePolicy(source->LineFromPosition(pos));
			source->SetSelection(pos,pos+stext.Len());
		}
	}
}

void mxMainWindow::OnToolbarFindChange (wxCommandEvent &evt) {
	wxString stext = toolbar_find_text->GetValue();
	if (stext.Len()==0)
		return;
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		int pos = source->FindText(source->GetSelectionStart(),source->GetLength(),toolbar_find_text->GetValue(),0);
		if (pos==wxSTC_INVALID_POSITION) 
			pos = source->FindText(0,source->GetSelectionStart()+stext.Len(),toolbar_find_text->GetValue(),0);
		if (pos!=wxSTC_INVALID_POSITION) {
			source->EnsureVisibleEnforcePolicy(source->LineFromPosition(pos));
			source->SetSelection(pos,pos+stext.Len());
		}
	}
}

void mxMainWindow::OnHelpUpdates(wxCommandEvent &evt) {
	new mxUpdatesChecker();
}

void mxMainWindow::SetExplorerPath(wxString path) {
	if (!project) config->Files.last_dir=path;
	explorer_tree.treeCtrl->Freeze();
//	explorer_tree.treeCtrl->DeleteChildren(explorer_tree.root);
	explorer_tree.treeCtrl->DeleteAllItems();
	explorer_tree.root = explorer_tree.treeCtrl->AddRoot(_T("Archivos Abiertos"), 0);
	if (!wxFileName(path).DirExists()) { // corregir si no existe
		wxFileName fn(path);
		while (fn.GetDirCount()>0 && !fn.DirExists()) fn.RemoveLastDir();
		if (fn.GetDirCount()==0) fn = (project?project->path:wxFileName::GetHomeDir());
		path=fn.GetFullPath();
	}
	explorer_tree.treeCtrl->SetItemText(explorer_tree.root,path);
	explorer_tree.path = path;
	wxDir dir(path);
	if ( dir.IsOpened() ) {
		wxString filename;
		wxString spec;
		bool cont = dir.GetFirst(&filename, spec , wxDIR_DIRS);
		wxArrayString as;
		while ( cont ) {
			as.Add(filename);
			cont = dir.GetNext(&filename);
		}	
		as.Sort();
		for (unsigned int i=0;i<as.GetCount();i++)
			explorer_tree.treeCtrl->AppendItem(explorer_tree.root,as[i],0);
		as.Clear();
		cont = dir.GetFirst(&filename, spec , wxDIR_FILES);
		while ( cont ) {
			as.Add(filename);
			cont = dir.GetNext(&filename);
		}
		as.Sort();
		for (unsigned int i=0;i<as.GetCount();i++) {
			char ctype=utils->GetFileType(as[i]);
			int t=4;
			if (ctype=='s')	t=1;
			else if (ctype=='h') t=2;
			else if (ctype=='z') t=5;
			if (!explorer_tree.show_only_sources || t<3)
				explorer_tree.treeCtrl->AppendItem(explorer_tree.root,as[i],t);
		}	
	}
//	explorer_tree.treeCtrl->SortChildren(explorer_tree.root);
	explorer_tree.treeCtrl->SelectItem(explorer_tree.root);
	explorer_tree.treeCtrl->SetFocus();
	explorer_tree.treeCtrl->Expand(explorer_tree.root);
	explorer_tree.treeCtrl->Thaw();
}

void mxMainWindow::OnSelectExplorerItem (wxTreeEvent &event) {
	
	explorer_tree.selected_item = event.GetItem();
	
	if (explorer_tree.selected_item==explorer_tree.root) {
		
		wxDirDialog *dlg=new wxDirDialog(this,_T("Seleccione la ubicacion:"),explorer_tree.path);
		if (wxID_OK==dlg->ShowModal()) {
			SetExplorerPath(dlg->GetPath());
			config->Files.last_dir = dlg->GetPath();
		}
		
	} else {
		
		wxCommandEvent evt;
		OnExplorerTreeOpenOne(evt);
		
	}
	
}

void mxMainWindow::OnExplorerTreePopup(wxTreeEvent &event) {
	
	mxHidenPanel::ignore_autohide=true;
	
	explorer_tree.selected_item = event.GetItem();
	
	wxMenu menu(_T(""));
	if (explorer_tree.selected_item==explorer_tree.root) {
		menu.Append(mxID_EXPLORER_POPUP_CHANGE_PATH, LANG(MAINW_EXPLORER_POPUP_CHANGE_PATH,"&Cambiar Ubicacion..."));
		menu.Append(mxID_EXPLORER_POPUP_PATH_UP, LANG(MAINW_EXPLORER_POPUP_LEVEL_UP,"&Subir un Nivel"));
		if (notebook_sources->GetPageCount())
			menu.Append(mxID_FILE_EXPLORE_FOLDER, LANG(MAINW_EXPLORER_POPUP_TAKE_FROM_SOURCE,"Tomar Ubicacion del Archivo Abierto"));
		if (!explorer_tree.show_only_sources)
			menu.Append(mxID_EXPLORER_POPUP_OPEN_ALL, LANG(MAINW_EXPLORER_POPUP_OPEN_ALL,"&Abrir Todos los Archivos"));
		menu.Append(mxID_EXPLORER_POPUP_OPEN_SOURCES, LANG(MAINW_EXPLORER_POPUP_OPEN_SOURCES,"Abrir &Todos los Fuentes"));
		menu.Append(mxID_EXPLORER_POPUP_UPDATE, LANG(MAINW_EXPLORER_POPUP_UPDATE,"Ac&tualizar"));
	} else {
		if (explorer_tree.treeCtrl->GetItemImage(explorer_tree.selected_item)) {
			menu.Append(mxID_EXPLORER_POPUP_OPEN_ONE, LANG(MAINW_EXPLORER_POPUP_OPEN_FILE,"&Abrir archivo"));
		} else {
			menu.Append(mxID_EXPLORER_POPUP_OPEN_ONE, LANG(MAINW_EXPLORER_POPUP_EXPAND,"&Expandir"));
			menu.Append(mxID_EXPLORER_POPUP_UPDATE, LANG(MAINW_EXPLORER_POPUP_UPDATE,"Ac&tualizar"));
			if (!explorer_tree.show_only_sources)
				menu.Append(mxID_EXPLORER_POPUP_OPEN_ALL, LANG(MAINW_EXPLORER_POPUP_OPEN_ALL,"&Abrir Todos los Archivos"));
			menu.Append(mxID_EXPLORER_POPUP_OPEN_SOURCES, LANG(MAINW_EXPLORER_POPUP_OPEN_SOURCES,"Abrir &Todos los Fuentes"));
			menu.Append(mxID_EXPLORER_POPUP_SET_AS_PATH, LANG(MAINW_EXPLORER_POPUP_SET_AS_ROOT,"Utilizar Como &Raiz"));
		}
	}
	menu.AppendSeparator();
	if (explorer_tree.show_only_sources)
		menu.Append(mxID_EXPLORER_POPUP_SHOW_ONLY_SOURCES, LANG(MAINW_EXPLORER_POPUP_SHOW_ALL,"&Mostrar Todos los Archivos"));
	else
		menu.Append(mxID_EXPLORER_POPUP_SHOW_ONLY_SOURCES, LANG(MAINW_EXPLORER_POPUP_SHOW_SOURCES,"&Mostrar Solo los Fuentes"));
	
	explorer_tree.treeCtrl->PopupMenu(&menu,event.GetPoint());	
	
	mxHidenPanel::ignore_autohide=false;
}

void mxMainWindow::OnExplorerTreeUpdate(wxCommandEvent &evt) {
	if (explorer_tree.selected_item==explorer_tree.root)
		SetExplorerPath(explorer_tree.path);
	else {
		wxCommandEvent evt;
		OnExplorerTreeOpenOne(evt);
	}
}

void mxMainWindow::OnExplorerTreeChangePath(wxCommandEvent &evy) {
	wxDirDialog *dlg=new wxDirDialog(this,LANG(MAINW_EXPLORER_SELECT_PATH,"Seleccione la ubicacion:"),explorer_tree.path);
	if (wxID_OK==dlg->ShowModal()) {
		SetExplorerPath(dlg->GetPath());
		config->Files.last_dir = dlg->GetPath();
	}
}

void mxMainWindow::OnExplorerTreePathUp(wxCommandEvent &evy) {
	for (int i=explorer_tree.path.Len()-2;i>=0;i--) {
		if ((explorer_tree.path[i]=='\\' || explorer_tree.path[i]=='/') && (i==0 || explorer_tree.path[i]!=':')) {
			explorer_tree.path=explorer_tree.path.Mid(0,i==0?1:i);
			break;
		}
	}
	SetExplorerPath(explorer_tree.path);
}

void mxMainWindow::OnExplorerTreeOpenOne(wxCommandEvent &evt) {
	
	
	wxString path = GetExplorerItemPath(explorer_tree.selected_item);
	if (explorer_tree.treeCtrl->GetItemImage(explorer_tree.selected_item)) {
		OpenFileFromGui(path);
#if defined(_WIN32) || defined(__WIN32__)
		focus_timer->Start(333,true);
#endif
	} else {
		
		explorer_tree.treeCtrl->Freeze();
		explorer_tree.treeCtrl->DeleteChildren(explorer_tree.selected_item);
		wxDir dir(path);
		if ( dir.IsOpened() ) {
			wxString filename;
			wxString spec;
			wxArrayString as;
			bool cont = dir.GetFirst(&filename, spec , wxDIR_DIRS);
			while ( cont ) {
				as.Add(filename);
				cont = dir.GetNext(&filename);
			}	
			as.Sort();
			for (unsigned int i=0;i<as.GetCount();i++)
				explorer_tree.treeCtrl->AppendItem(explorer_tree.selected_item,as[i],0);
			as.Clear();
			cont = dir.GetFirst(&filename, spec , wxDIR_FILES);
			while ( cont ) {
				as.Add(filename);
				cont = dir.GetNext(&filename);
			}
			as.Sort();
			for (unsigned int i=0;i<as.GetCount();i++) {
				char ctype=utils->GetFileType(as[i]);
				int t=4;
				if (ctype=='s') t=1;
				else if (ctype=='h') t=2;
				else if (ctype=='z') t=5;
				if (!explorer_tree.show_only_sources || (t==1||t==2))
					explorer_tree.treeCtrl->AppendItem(explorer_tree.selected_item,as[i],t);
			}	
		}
		//			explorer_tree.treeCtrl->SortChildren(explorer_tree.selected_item);
		explorer_tree.treeCtrl->Thaw();	
		explorer_tree.treeCtrl->Expand(explorer_tree.selected_item);
		wxYield();
	}
	
}

void mxMainWindow::OnExplorerTreeSetAsPath(wxCommandEvent &evt) {
	SetExplorerPath(GetExplorerItemPath(explorer_tree.selected_item));
}

void mxMainWindow::OnExplorerTreeOpenAll(wxCommandEvent &evt) {
	if (!explorer_tree.treeCtrl->GetChildrenCount(explorer_tree.selected_item)) {
		wxCommandEvent evt;
		OnExplorerTreeOpenOne(evt);
	}
	wxString path = GetExplorerItemPath(explorer_tree.selected_item);
	wxTreeItemIdValue cookie;
	wxTreeItemId item = explorer_tree.treeCtrl->GetFirstChild(explorer_tree.selected_item,cookie);
	while (item.IsOk()) {
		if (explorer_tree.treeCtrl->GetItemImage(item)!=0)
			OpenFileFromGui(DIR_PLUS_FILE(path,explorer_tree.treeCtrl->GetItemText(item)));
		item = explorer_tree.treeCtrl->GetNextChild(explorer_tree.selected_item,cookie);
	}
}

void mxMainWindow::OnExplorerTreeOpenSources(wxCommandEvent &evt) {
	wxString path = GetExplorerItemPath(explorer_tree.selected_item);
	wxTreeItemIdValue cookie;
	wxTreeItemId item = explorer_tree.treeCtrl->GetFirstChild(explorer_tree.selected_item,cookie);
	while (item.IsOk()) {
		if (explorer_tree.treeCtrl->GetItemImage(item)!=0 && explorer_tree.treeCtrl->GetItemImage(item)<3)
			OpenFileFromGui(DIR_PLUS_FILE(path,explorer_tree.treeCtrl->GetItemText(item)));
		item = explorer_tree.treeCtrl->GetNextChild(explorer_tree.selected_item,cookie);
	}
}

void mxMainWindow::OnExplorerTreeShowOnlySources(wxCommandEvent &evt) {
	explorer_tree.show_only_sources = ! explorer_tree.show_only_sources;
	SetExplorerPath(explorer_tree.path);
}

wxString mxMainWindow::GetExplorerItemPath(wxTreeItemId item) {
	if (explorer_tree.root==item)
		return explorer_tree.path;
	wxTreeItemId parent = explorer_tree.treeCtrl->GetItemParent(item);
	wxString path = explorer_tree.treeCtrl->GetItemText(item);
	while (parent!=explorer_tree.root) {
		path = DIR_PLUS_FILE(explorer_tree.treeCtrl->GetItemText(parent),path);
		parent = explorer_tree.treeCtrl->GetItemParent(parent);
	}
	path = DIR_PLUS_FILE(explorer_tree.path,path);	
	return path;
}

void mxMainWindow::OnSymbolsGenerateCache(wxCommandEvent &evt) {
	wxFileDialog dlg1(this,_T("Archivo indice:"),"","","*.*",wxFD_SAVE);
	if (wxID_OK!=dlg1.ShowModal()) return;
	wxString fname = dlg1.GetPath();
	wxDirDialog dlg2(this,_T("Directorio base:"),_T(""));
	if (wxID_OK!=dlg2.ShowModal()) return;
	if (code_helper->GenerateCacheFile(dlg2.GetPath(),fname))
		mxMessageDialog(main_window,_T("Indice generado"),fname,mxMD_OK|mxMD_INFO).ShowModal();
	else
		mxMessageDialog(main_window,_T("No se pudo generar el archivo"),fname,mxMD_OK|mxMD_ERROR).ShowModal();
}

void mxMainWindow::OnToolRightClick(wxCommandEvent &evt) {
	int id=evt.GetId();
	if (id>=mxID_CUSTOM_TOOL_0 && id<mxID_CUSTOM_TOOL_0+10) {
		new mxCustomTools(id-mxID_CUSTOM_TOOL_0);
		return;
	}
	if (preference_window) {
		preference_window->ShowUp();
	} else {
		preference_window = new mxPreferenceWindow(this);
	}
	preference_window->SetToolbarPage();
	if (toolbar_file->FindById(id)) preference_window->OnToolbarsFile(evt);
	else if (toolbar_edit->FindById(id)) preference_window->OnToolbarsEdit(evt);
	else if (toolbar_view->FindById(id)) preference_window->OnToolbarsView(evt);
	else if (toolbar_debug->FindById(id)) preference_window->OnToolbarsDebug(evt);
	else if (toolbar_run->FindById(id)) preference_window->OnToolbarsRun(evt);
	else if (toolbar_misc->FindById(id)) preference_window->OnToolbarsMisc(evt);
	else if (toolbar_tools->FindById(id)) preference_window->OnToolbarsTools(evt);
}

void mxMainWindow::OnEditListMarks (wxCommandEvent &event) {
	if (project) {
		
		for (int i=notebook_sources->GetPageCount()-1;i>=0;i--)
			project->GetSourceExtras((mxSource*)(notebook_sources->GetPage(i)));
		
		wxString res(_T("<HTML><HEAD><TITLE>Lineas Resaltadas</TITLE></HEAD><BODY><B>Lineas Resaltadas:</B><BR><UL>"));
		wxString restmp;
		
		file_item *fi = project->first_source;
		while (fi) {
			marked_line_item *mi = fi->markers;
			restmp=_T("");
			while (mi) {
				restmp=wxString(_T("<LI><A href=\"gotoline:"))<<DIR_PLUS_FILE(project->path,fi->name)<<_T(":")<<mi->line+1<<_T("\">")<<fi->name<<_T(": linea ")<<mi->line+1<<_T("</A></LI>")<<restmp;
				ML_NEXT(mi);
			}
			res<<restmp;
			ML_NEXT(fi);
		}
		fi = project->first_header;
		while (fi) {
			marked_line_item *mi = fi->markers;
			restmp=_T("");
			while (mi) {
				restmp=wxString(_T("<LI><A href=\"gotoline:"))<<DIR_PLUS_FILE(project->path,fi->name)<<_T(":")<<mi->line+1<<_T("\">")<<fi->name<<_T(": linea ")<<mi->line+1<<_T("</A></LI>")<<restmp;
				ML_NEXT(mi);
			}
			res<<restmp;
			ML_NEXT(fi);
		}
		fi = project->first_other;
		while (fi) {
			marked_line_item *mi = fi->markers;
			restmp=_T("");
			while (mi) {
				restmp=wxString(_T("<LI><A href=\"gotoline:"))<<DIR_PLUS_FILE(project->path,fi->name)<<_T(":")<<mi->line+1<<_T("\">")<<fi->name<<_T(": linea ")<<mi->line+1<<_T("</A></LI>")<<restmp;
				ML_NEXT(mi);
			}
			res<<restmp;
			ML_NEXT(fi);
		}
		
		res<<_T("</UL><BR><BR></BODY></HTML>");
		main_window->ShowInQuickHelpPanel(res);	
	} else {
		wxString res(_T("<HTML><HEAD><TITLE>Lineas Resaltadas</TITLE></HEAD><BODY><B>Lineas Resaltadas:</B><BR><UL>"));
		for (int i=0,j=notebook_sources->GetPageCount();i<j;i++) {
			mxSource *src= (mxSource*)(notebook_sources->GetPage(i));
			for (int k=0;k<src->GetLineCount();k++) {
				wxString file_name = src->sin_titulo?src->page_text:src->source_filename.GetFullPath();
				wxString page_text = utils->ToHtml(src->page_text);
				if (src->MarkerGet(k)&1<<mxSTC_MARK_USER)
					res<<_T("<LI><A href=\"gotoline:")<<file_name<<_T(":")<<k+1<<_T("\">")<<page_text<<_T(": linea ")<<k+1<<_T("</A></LI>");
			}
		}
		res<<_T("</UL><BR><BR></BODY></HTML>");
		main_window->ShowInQuickHelpPanel(res);	
	}
}

void mxMainWindow::OnFileProjectConfig (wxCommandEvent &event) {
	if (project) new mxProjectGeneralConfig;
}

wxString mxMainWindow::AvoidDuplicatePageText(wxString ptext) {
	wxString text=ptext;
	int n=1,i=0,np=notebook_sources->GetPageCount();
	while (true) {
		while (i<np && ((mxSource*)(notebook_sources->GetPage(i)))->page_text!=text) i++;
		if (i<np) {
			n++;
			text=ptext;
			text<<_T("(")<<n<<_T(")");
			i=0;
		} else
			return text;
	}
	return text;
}

void mxMainWindow::OnDebugCoreDump (wxCommandEvent &event) {
	if (notebook_sources->GetPageCount()>0||project) {
		if (!debug->debugging && (project || notebook_sources->GetPageCount())) {
			wxString dir = project?DIR_PLUS_FILE(project->path,project->active_configuration->working_folder):CURRENT_SOURCE->working_folder.GetFullPath();
			wxFileDialog dlg (this, _T("Abrir Archivo"), dir, _T(" "), _T("Volcados de memoria|core*|Todos los Archivos|*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (dlg.ShowModal() == wxID_OK)
				debug->LoadCoreDump(dlg.GetPath(),project?NULL:CURRENT_SOURCE);
		} else if (debug->debugging && !debug->waiting) {
			wxString sPath = project?project->path:(CURRENT_SOURCE->sin_titulo?wxString(wxFileName::GetHomeDir()):wxFileName(CURRENT_SOURCE->source_filename).GetPath());
			wxFileDialog dlg (this, _T("Guardar Volcade de Memoria"),sPath,_T("core"), _T("Any file (*)|*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			dlg.SetDirectory(sPath);
			dlg.SetWildcard(_T("Volcados de memoria|core*|Todos los Archivos|*"));
			if (dlg.ShowModal() == wxID_OK)
				debug->SaveCoreDump(dlg.GetPath());
		}
	}
}

void mxMainWindow::SetOpenedFileName(wxString name) {
	SetTitle(wxString(_T("ZinjaI - "))+name);
}

void mxMainWindow::OnKeyEvent(wxWindow *who, wxKeyEvent &evt) {
	if (who==project_tree.treeCtrl && project) {
		project_tree.selected_item = project_tree.treeCtrl->GetSelection();
		if (config->Init.autohiding_panels && evt.GetKeyCode()==WXK_ESCAPE && !autohide_handlers[ATH_PROJECT]->IsDocked() ) {
			autohide_handlers[ATH_PROJECT]->Hide();
		} else if (evt.GetKeyCode()==WXK_MENU) {
			wxTreeEvent te;
			te.SetItem(project_tree.selected_item);
			wxRect r;
			project_tree.treeCtrl->GetBoundingRect(project_tree.selected_item,r,true);
			wxPoint p(r.GetX()+r.GetHeight(),r.GetY()+r.GetHeight());
			te.SetPoint(p);
			OnProjectTreePopup(te);
		} else if (project_tree.selected_item==project_tree.sources || project_tree.selected_item==project_tree.headers || project_tree.selected_item==project_tree.others) {
			wxCommandEvent cmd;
			project_tree.selected_parent = project_tree.selected_item;
			if (evt.GetKeyCode()==WXK_INSERT) OnProjectTreeAdd(cmd);
			else evt.Skip();
		} else {
			project_tree.selected_parent=project_tree.selected_item;
			wxCommandEvent cmd;
			if (evt.GetKeyCode()==WXK_DELETE) OnProjectTreeDelete(cmd);
			else evt.Skip();
		}
	} else if (who==explorer_tree.treeCtrl) {
		explorer_tree.selected_item=explorer_tree.treeCtrl->GetSelection();
		if (config->Init.autohiding_panels && evt.GetKeyCode()==WXK_ESCAPE && !autohide_handlers[ATH_EXPLORER]->IsDocked() ) {
			autohide_handlers[ATH_EXPLORER]->Hide();
		} else if (evt.GetKeyCode()==WXK_MENU) {
			wxTreeEvent te;
			te.SetItem(explorer_tree.selected_item);
			wxRect r;
			explorer_tree.treeCtrl->GetBoundingRect(explorer_tree.selected_item,r,true);
			wxPoint p(r.GetX()+r.GetHeight(),r.GetY()+r.GetHeight());
			te.SetPoint(p);
			OnExplorerTreePopup(te);
		} else if (evt.GetKeyCode()==WXK_RETURN && !explorer_tree.treeCtrl->GetItemImage(explorer_tree.selected_item)) {
			wxCommandEvent evt;
			OnExplorerTreeSetAsPath(evt);
		} else if (evt.GetKeyCode()==WXK_LEFT) {
			explorer_tree.treeCtrl->Collapse(explorer_tree.selected_item);
		} else if (evt.GetKeyCode()==WXK_RIGHT) {
			if (explorer_tree.selected_item==explorer_tree.root) {
				explorer_tree.treeCtrl->Expand(explorer_tree.root);
			} else {
				wxTreeEvent te;
				te.SetItem(explorer_tree.selected_item);
				OnSelectExplorerItem(te);
				explorer_tree.treeCtrl->Expand(explorer_tree.selected_item);
			}
		} else if (evt.GetKeyCode()==WXK_BACK) {
			if (explorer_tree.selected_item==explorer_tree.root) {
				wxCommandEvent evt;
				OnExplorerTreePathUp(evt);
			} else {
				explorer_tree.treeCtrl->SelectItem(explorer_tree.treeCtrl->GetItemParent(explorer_tree.selected_item));
			}
		} else
			evt.Skip();
	} else evt.Skip();
}

void mxMainWindow::ShowWelcome(bool show) {
	wxAuiPaneInfo &pns = aui_manager.GetPane(notebook_sources);
	wxAuiPaneInfo &pwp = aui_manager.GetPane(welcome_panel);
	if (show) {
		aui_manager.GetPane(compiler_tree.treeCtrl).Hide();
		pns.Hide();
		pwp.Show();
		welcome_panel->Reload();
	} else {
		if (config->Init.show_beginner_panel && !config->Init.autohide_panels)
			ShowBeginnersPanel();
		pwp.Hide();
		pns.Show();
	}
	aui_manager.Update();
	if ((welcome_panel->is_visible=show)) welcome_panel->SetFocus();
}

void mxMainWindow::OnActivate (wxActivateEvent &event) {
	if (project && project->use_wxfb && project->auto_wxfb) {
		project->WxfbAutoCheck();
	}
	event.Skip();
}

void mxMainWindow::OnKey(wxKeyEvent &evt) {
	evt.Skip();
}

void mxMainWindow::OnProjectTreeProperties (wxCommandEvent &event) {
	(new mxSourceProperties(project->GetNameFromItem(project_tree.selected_item)))->Show();
}

void mxMainWindow::OnProjectTreeOpenFolder (wxCommandEvent &event) {
	utils->OpenFolder(wxFileName(project->GetNameFromItem(project_tree.selected_item)).GetPath());
}

void mxMainWindow::OnFileOpenFolder(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		utils->OpenFolder(src->sin_titulo?src->temp_filename.GetPath():src->source_filename.GetPath());
	}
}

void mxMainWindow::OnFileExploreFolder(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		SetExplorerPath(src->sin_titulo?src->temp_filename.GetPath():src->source_filename.GetPath());
		ShowExplorerTreePanel();
	}
}

void mxMainWindow::OnFileProperties (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		(new mxSourceProperties(src->sin_titulo?src->temp_filename.GetFullPath():src->source_filename.GetFullPath()),src)->Show();
	}
}

void mxMainWindow::OnViewNextError (wxCommandEvent &event) {
	wxTreeCtrl *t = compiler_tree.treeCtrl;
	wxTreeItemId ip,it = t->GetSelection();
	if (!it.IsOk() || it==compiler_tree.root || it==compiler_tree.state) it=ip=compiler_tree.root; else {
		ip=t->GetItemParent(it);
		while (ip!=compiler_tree.root && ip!=compiler_tree.errors && ip!=compiler_tree.all && ip!=compiler_tree.warnings) {
			it=ip;
			ip=t->GetItemParent(it);
		}
	}
	wxTreeItemIdValue c;
	if (it==compiler_tree.root || it==compiler_tree.errors || it==compiler_tree.all || ip==compiler_tree.all) {
		if (t->GetChildrenCount(compiler_tree.errors)) it=t->GetFirstChild(compiler_tree.errors,c);
		else if (t->GetChildrenCount(compiler_tree.warnings)) it=t->GetFirstChild(compiler_tree.warnings,c);
		else it=compiler_tree.root;
	} else if (it==compiler_tree.warnings) {
		if (t->GetChildrenCount(compiler_tree.warnings)) it=t->GetFirstChild(compiler_tree.warnings,c);
		else if (t->GetChildrenCount(compiler_tree.errors)) it=t->GetFirstChild(compiler_tree.errors,c);
		else it=compiler_tree.root;
	} else if (ip==compiler_tree.errors) {
		it = t->GetNextSibling(it);
		if (!it.IsOk()) {
			if (t->GetChildrenCount(compiler_tree.warnings)) it=t->GetFirstChild(compiler_tree.warnings,c);
			else if (t->GetChildrenCount(compiler_tree.errors)) it=t->GetFirstChild(compiler_tree.errors,c);
			else it=compiler_tree.root;
		}
	} else if (ip==compiler_tree.warnings) {
		it = t->GetNextSibling(it);
		if (!it.IsOk()) {
			if (t->GetChildrenCount(compiler_tree.errors)) it=t->GetFirstChild(compiler_tree.errors,c);
			else if (t->GetChildrenCount(compiler_tree.warnings)) it=t->GetFirstChild(compiler_tree.warnings,c);
			else it=compiler_tree.root;
		}
	}
	if (it!=compiler_tree.root) {
		t->Expand(t->GetItemParent(it));
		t->Expand(it);
		t->SelectItem(it);
		wxTreeEvent evt;
		evt.SetItem(it);
		OnSelectError(evt);
	}
}

void mxMainWindow::OnViewPrevError (wxCommandEvent &event) {
	wxTreeCtrl *t = compiler_tree.treeCtrl;
	wxTreeItemId ip,it = t->GetSelection();
	if (!it.IsOk() || it==compiler_tree.root || it==compiler_tree.state) it=ip=compiler_tree.root; else {
		ip=t->GetItemParent(it);
		while (ip!=compiler_tree.root && ip!=compiler_tree.errors && ip!=compiler_tree.all && ip!=compiler_tree.warnings) {
			it=ip;
			ip=t->GetItemParent(it);
		}
	}
	if (it==compiler_tree.root || it==compiler_tree.errors || it==compiler_tree.all || ip==compiler_tree.all) {
		if (t->GetChildrenCount(compiler_tree.warnings)) it=t->GetLastChild(compiler_tree.warnings);
		else if (t->GetChildrenCount(compiler_tree.errors)) it=t->GetLastChild(compiler_tree.errors);
		else it=compiler_tree.root;
	} else if (it==compiler_tree.warnings) {
		if (t->GetChildrenCount(compiler_tree.errors)) it=t->GetLastChild(compiler_tree.errors);
		else if (t->GetChildrenCount(compiler_tree.warnings)) it=t->GetLastChild(compiler_tree.warnings);
		else it=compiler_tree.root;
	} else if (ip==compiler_tree.errors) {
		it = t->GetPrevSibling(it);
		if (!it.IsOk()) {
			if (t->GetChildrenCount(compiler_tree.warnings)) it=t->GetLastChild(compiler_tree.warnings);
			else if (t->GetChildrenCount(compiler_tree.errors)) it=t->GetLastChild(compiler_tree.errors);
			else it=compiler_tree.root;
		}
	} else if (ip==compiler_tree.warnings) {
		it = t->GetPrevSibling(it);
		if (!it.IsOk()) {
			if (t->GetChildrenCount(compiler_tree.errors)) it=t->GetLastChild(compiler_tree.errors);
			else if (t->GetChildrenCount(compiler_tree.warnings)) it=t->GetLastChild(compiler_tree.warnings);
			else it=compiler_tree.root;
		}
	}
	if (it!=compiler_tree.root) {
		t->Expand(t->GetItemParent(it));
		t->Expand(it);
		t->SelectItem(it);
		wxTreeEvent evt;
		evt.SetItem(it);
		OnSelectError(evt);
	}
}

void mxMainWindow::OnDebugShowLogPanel (wxCommandEvent &event) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(autohide_handlers[ATH_DEBUG_LOG]).IsShown()) {
			aui_manager.GetPane(autohide_handlers[ATH_DEBUG_LOG]).Show();
			aui_manager.Update();
		}
		if (!aui_manager.GetPane(debug_log_panel).IsShown())
			autohide_handlers[ATH_DEBUG_LOG]->ForceShow();
	} else {
		if (!aui_manager.GetPane(debug_log_panel).IsShown()) {
			aui_manager.GetPane(debug_log_panel).Show();
			aui_manager.Update();
		}	
	}
}

void mxMainWindow::OnDebugEnableInverseExecution (wxCommandEvent &event) {
	bool inv = debug->EnableInverseExec();
	main_window->menu.debug_inverse_execution->Check(false);
	main_window->menu.debug_inverse_execution->Enable(inv);
	main_window->menu.debug_enable_inverse_execution->Check(inv);
	main_window->toolbar_debug->ToggleTool(mxID_DEBUG_ENABLE_INVERSE_EXEC,inv);
	main_window->toolbar_debug->ToggleTool(mxID_DEBUG_INVERSE_EXEC,false);
	main_window->toolbar_debug->EnableTool(mxID_DEBUG_INVERSE_EXEC,inv);
}

void mxMainWindow::OnDebugInverseExecution (wxCommandEvent &event) {
	bool inv=debug->ToggleInverseExec();
	menu.debug_inverse_execution->Check(inv);
	toolbar_debug->ToggleTool(mxID_DEBUG_INVERSE_EXEC,inv);
}

void mxMainWindow::AddToDebugLog(wxString str) {
	debug_log_panel->Append(str);
//	debug_log_panel->SetSelection(debug_log_panel->GetCount());
	debug_log_panel->ScrollLines(1);
}

void mxMainWindow::ClearDebugLog() {
	debug_log_panel->Clear();
}

/**
* @brief Muestra el panel de valgrind y carga algun resultado
* @param what   tipo de resultado, v=valgrind, c=cppcheck
* @param file   archivo de donde leer los resultados
**/
void mxMainWindow::ShowValgrindPanel(char what, wxString file) {
	if (valgrind_panel) {
		aui_manager.GetPane(valgrind_panel).Show();
		valgrind_panel->SetMode(what,file);
	} else {
		aui_manager.AddPane(
			valgrind_panel = new mxValgrindOuput(this,what,file)
			, wxAuiPaneInfo().Name(_T("valgrind_output")).Bottom().Caption(LANG(CAPTION_TOOLS_RESULTS_PANEL,"Panel de resultados")).CloseButton(true).MaximizeButton(true).Row(8));
	}
	aui_manager.Update();
	valgrind_panel->LoadOutput();
}

void mxMainWindow::OnViewBeginnerPanel (wxCommandEvent &event) {
	if (!beginner_panel) CreateBeginnersPanel();
	if (config->Init.autohide_panels) {
		if (!menu.view_beginner_panel->IsChecked()) {
			autohide_handlers[ATH_BEGINNERS]->Hide();
			aui_manager.GetPane(autohide_handlers[ATH_BEGINNERS]).Hide();
			menu.view_beginner_panel->Check(false);
		} else {
			aui_manager.GetPane(autohide_handlers[ATH_BEGINNERS]).Show();
			autohide_handlers[ATH_BEGINNERS]->Show();
			menu.view_beginner_panel->Check(true);
		}
	} else {
		if (!menu.view_beginner_panel->IsChecked()) {
			menu.view_beginner_panel->Check(false);
			aui_manager.GetPane(beginner_panel).Hide();
		} else {
			menu.view_beginner_panel->Check(true);
			aui_manager.GetPane(beginner_panel).Show();
		}
	}
	aui_manager.Update();
}

void mxMainWindow::ShowDiffSideBar(bool bar, bool map) {
	if (map) {
		if (!diff_sidebar) {
			diff_sidebar=new mxDiffSideBar;
			aui_manager.AddPane(diff_sidebar, wxAuiPaneInfo().Name(_T("diff_sidebar")).Caption(_T("diff")).Right().Row(2).Show().MaxSize(20,-1));
		}
	}
	if (bar)
		aui_manager.GetPane(toolbar_diff).Show();
	if (bar||map) aui_manager.Update();
}

mxSource *mxMainWindow::GetCurrentSource() {
	IF_THERE_IS_SOURCE
		return CURRENT_SOURCE;
	else 
		return NULL;
}

void mxMainWindow::OnEscapePressed(wxCommandEvent &event) {
	bool do_update=false;
	if (aui_manager.GetPane(compiler_tree.treeCtrl).IsShown()) {
		if (config->Init.autohide_panels)
			autohide_handlers[ATH_COMPILER]->Hide();
		else {
			compiler_tree.menuItem->Check(false);
			aui_manager.GetPane(compiler_tree.treeCtrl).Hide();
		}
		do_update=true;
	}
	if (aui_manager.GetPane(quick_help).IsShown()) {
		if (config->Init.autohide_panels)
			autohide_handlers[ATH_QUICKHELP]->Hide();
		else {
			compiler_tree.menuItem->Check(false);
			aui_manager.GetPane(quick_help).Hide();
		}
		do_update=true;
	}
//#ifndef __WIN32__
	if (aui_manager.GetPane(valgrind_panel).IsShown()) {
		compiler_tree.menuItem->Check(false);
		aui_manager.GetPane(valgrind_panel).Hide();
		do_update=true;
	}
//#endif
	if (do_update) aui_manager.Update();	
}

void mxMainWindow::ShowQuickHelpPanel(bool hide_compiler_tree) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(quick_help).IsShown()) {
			autohide_handlers[ATH_QUICKHELP]->ForceShow();
		}
	} else {	
		if (!aui_manager.GetPane(quick_help).IsShown()) {
			// hide compiler results pane
			if (hide_compiler_tree) {
				compiler_tree.menuItem->Check(false);
				aui_manager.GetPane(compiler_tree.treeCtrl).Hide();
			}
			// show quick help pane
			aui_manager.GetPane(quick_help).Show();
			aui_manager.Update();
		}
	}
}

void mxMainWindow::ShowCompilerTreePanel() {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(compiler_tree.treeCtrl).IsShown()) {
			autohide_handlers[ATH_COMPILER]->ForceShow();
			focus_timer->Start(333,true);
		}
	} else {	
		if (!aui_manager.GetPane(compiler_tree.treeCtrl).IsShown()) {
			aui_manager.GetPane(quick_help).Hide();
			aui_manager.GetPane(compiler_tree.treeCtrl).Show();
			compiler_tree.menuItem->Check(true);
			aui_manager.Update();
		}
	}
}

void mxMainWindow::ShowExplorerTreePanel() {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(explorer_tree.treeCtrl).IsShown()) {
			autohide_handlers[ATH_EXPLORER]->ForceShow();
			focus_timer->Start(333,true);
		}
	} else {	
		if(left_panels) {
			if (!menu.view_left_panels->IsChecked()) {
				menu.view_left_panels->Check(true);
				aui_manager.GetPane(left_panels).Show();
				aui_manager.Update();
			}
			explorer_tree.menuItem->Check(false);
			left_panels->SetSelection(2);
			explorer_tree.treeCtrl->SetFocus();
		} else if (!aui_manager.GetPane(explorer_tree.treeCtrl).IsShown()) {
			aui_manager.GetPane(explorer_tree.treeCtrl).Show();
			explorer_tree.menuItem->Check(true);
			aui_manager.Update();
		}
	}
}

void mxMainWindow::CreateBeginnersPanel() {
	beginner_panel = new mxBeginnerPanel(this);
	if (config->Init.autohiding_panels) {
		autohide_handlers[ATH_BEGINNERS] = new mxHidenPanel(this,beginner_panel,2,LANG(MAINW_BEGGINERS_PANEL,"Asistencias"));
		aui_manager.AddPane(autohide_handlers[ATH_BEGINNERS], wxAuiPaneInfo().CaptionVisible(false).Right().Position(0).Show());
	}
	aui_manager.AddPane(beginner_panel, wxAuiPaneInfo().Name(_T("beginner_panel")).Caption(LANG(MAINW_BEGGINERS_PANEL,"Panel de Asistencias")).Right().Hide());
	aui_manager.Update();
}

void mxMainWindow::ShowBeginnersPanel() {
	if (!beginner_panel) CreateBeginnersPanel();
	if (!aui_manager.GetPane(beginner_panel).IsShown()) {
		if (config->Init.autohiding_panels) {
			autohide_handlers[ATH_BEGINNERS]->ForceShow();
		} else {
			aui_manager.GetPane(beginner_panel).Show();
			menu.view_beginner_panel->Check(true);
			aui_manager.Update();
		}
	}
}

void mxMainWindow::OnResize(wxSizeEvent &evt) {
	if (config->Init.autohide_panels) {
		for (int i=0;i<ATH_COUNT;i++)
			if (autohide_handlers[i])
				autohide_handlers[i]->ProcessParentResize();		
	}
}

void mxMainWindow::SetStatusBarFields() {
	if (config->Source.lineNumber) {
		status_bar->SetFieldsCount(1); 
	} else {
		int sz[2]={-5,-1}; 
		status_bar->SetFieldsCount(2,sz);
	}
}

void mxMainWindow::OnViewDuplicateTab(wxCommandEvent &evt) {
	IF_THERE_IS_SOURCE {
		mxSource *orig = CURRENT_SOURCE;
		if (orig->sin_titulo) {
			mxMessageDialog(main_window,LANG(MAINW_CANNOT_SPLIT_VIEW,"No se puede duplicar archivos sin nombre"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
			return;
		}
		int opage=notebook_sources->GetSelection();
		mxSource *source = new mxSource(notebook_sources, orig->source_filename.GetFullName());
		source->SplitFrom(orig);
		notebook_sources->AddPage(source, source->page_text, true, *bitmaps->files.source);
		
		main_window->notebook_sources->Split(notebook_sources->GetPageCount()-1,wxBOTTOM);
		main_window->notebook_sources->SetSelection(opage);
		main_window->notebook_sources->SetSelection(notebook_sources->GetPageCount()-1);
	}
}

void mxMainWindow::OnProjectTreeToggleFullPath(wxCommandEvent &event) {
	bool full = config->Init.fullpath_on_project_tree = !config->Init.fullpath_on_project_tree;
	file_item *it;
	it=project->first_header;
	ML_ITERATE(it) {
		project_tree.treeCtrl->SetItemText(it->item,full?it->name:wxFileName(it->name).GetFullName());
	}
	it=project->first_source;
	ML_ITERATE(it) {
		project_tree.treeCtrl->SetItemText(it->item,full?it->name:wxFileName(it->name).GetFullName());
	}
	it=project->first_other;
	ML_ITERATE(it) {
		project_tree.treeCtrl->SetItemText(it->item,full?it->name:wxFileName(it->name).GetFullName());
	}
	project_tree.treeCtrl->SortChildren(project_tree.sources);
	project_tree.treeCtrl->SortChildren(project_tree.others);
	project_tree.treeCtrl->SortChildren(project_tree.headers);
}

void mxMainWindow::OnInternalInfo ( wxCommandEvent &event ) {
	wxString info;
	compile_and_run_struct_single *item=compiler->compile_and_run_single;
	while (item) {
		info<<item->GetInfo()<<"\n";
		item=item->next;
	}
	wxMessageBox(info);
}

void mxMainWindow::SetStatusText(wxString text) {
	status_bar->SetStatusText(text);
}

/// @brief sets status bar progress (0-100 to show, -1 to hide)
void mxMainWindow::SetStatusProgress(int prog) {
	status_bar->SetProgress(prog);
}


void mxMainWindow::FocusToSource() {
	IF_THERE_IS_SOURCE CURRENT_SOURCE->SetFocus();
}
