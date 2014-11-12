#include <wx/aui/aui.h> // si no hago este include antes del de imaglist no enlaza en windows por "undefined reference to wxGenericImageList::..." (wft?)
#include <wx/imaglist.h>
#include <wx/artprov.h>
#include <wx/txtstrm.h>
#include <wx/grid.h>
#include <wx/dir.h>
#include "mxMainWindow.h"
#include "egdata.h"
#include "mxFindDialog.h"
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
#include "mxArt.h"
#include "mxMessageDialog.h"
#include "mxValgrindOuput.h"
#include "mxGotoFunctionDialog.h"
#include "mxGotoFileDialog.h"
#include "mxInspectionsPanel.h"
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
#include "mxCustomTools.h"
#include "parserData.h"
#include "mxHidenPanel.h"
#include "mxTextDialog.h"
#include "mxMultipleFileChooser.h"
#include "mxCompiler.h"
#include "Autocoder.h"
#include "mxColoursEditor.h"
#include "Toolchain.h"
#include "CodeHelper.h"
#include <iostream>
#include "mxGCovSideBar.h"
#include "mxReferenceWindow.h"
#include "DebugPatcher.h"
#include "mxGdbCommandsPanel.h"
#include "mxSignalsSettings.h"
#include "MenusAndToolsConfig.h"
#include "mxShortcutsDialog.h"
#include "mxExternCompilerOutput.h"
using namespace std;

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


#define _record_this_action_in_macro(_id) \
	class MacroMasker {  \
		bool is_masked; \
	public: \
		MacroMasker(int id) { \
			is_masked = main_window->m_macro && (*main_window->m_macro)[0].msg==1; \
			if (!is_masked) return; (*main_window->m_macro)[0].msg=0; \
			main_window->m_macro->Add(mxSource::MacroAction(id)); \
		} \
		~MacroMasker() { if (is_masked) (*main_window->m_macro)[0].msg=1; } \
	} macro_masker(_id);

mxMainWindow *main_window;
mxSource *EXTERNAL_SOURCE; // will be main_window address, an impossible address for a real mxSource, so OpenFile can use to say "was opened, but not by me, amy be wxfb or someone else"

BEGIN_EVENT_TABLE(mxMainWindow, wxFrame)
	
	EVT_SIZE(mxMainWindow::OnResize)
//	EVT_MENU_OPEN(mxMainWindow::OnMenuOpen)
	
	EVT_TOOL_RCLICKED(wxID_ANY,mxMainWindow::OnToolRightClick)
	
	EVT_MENU(mxID_FILE_PRINT, mxMainWindow::OnFilePrint)
	EVT_MENU(mxID_FILE_NEW, mxMainWindow::OnFileNew)
	EVT_MENU(mxID_FILE_PROJECT, mxMainWindow::OnFileNewProject)
	EVT_MENU(mxID_FILE_OPEN, mxMainWindow::OnFileOpen)
	EVT_MENU(mxID_FILE_OPEN_SELECTED, mxMainWindow::OnFileOpenSelected)
	EVT_MENU(mxID_FILE_OPEN_H, mxMainWindow::OnFileOpenH)
	EVT_MENU(mxID_FILE_RELOAD, mxMainWindow::OnFileReload)
	EVT_MENU(mxID_FILE_EXPORT_HTML, mxMainWindow::OnFileExportHtml)
	EVT_MENU(mxID_FILE_CLOSE, mxMainWindow::OnFileClose)
	EVT_MENU(mxID_FILE_CLOSE_ALL, mxMainWindow::OnFileCloseAll)
	EVT_MENU(mxID_FILE_CLOSE_ALL_BUT_ONE, mxMainWindow::OnFileCloseAllButOne)
	EVT_MENU(mxID_FILE_SAVE_PROJECT, mxMainWindow::OnFileSaveProject)
	EVT_MENU(mxID_FILE_CLOSE_PROJECT, mxMainWindow::OnFileCloseProject)
	EVT_MENU(mxID_FILE_SAVE, mxMainWindow::OnFileSave)
	EVT_MENU(mxID_FILE_SAVE_AS, mxMainWindow::OnFileSaveAs)
	EVT_MENU(mxID_FILE_SAVE_ALL, mxMainWindow::OnFileSaveAll)
	EVT_MENU(mxID_FILE_EXIT, mxMainWindow::OnExit)
	EVT_MENU(mxID_FILE_EXPLORE_FOLDER, mxMainWindow::OnFileExploreFolder)
	EVT_MENU(mxID_FILE_OPEN_FOLDER, mxMainWindow::OnFileOpenFolder)
	EVT_MENU(mxID_FILE_PROPERTIES, mxMainWindow::OnFileProperties)
	EVT_MENU(mxID_FILE_PREFERENCES, mxMainWindow::OnPreferences)
	EVT_MENU(mxID_FILE_PROJECT_CONFIG, mxMainWindow::OnFileProjectConfig)
	EVT_MENU(mxID_FILE_SOURCE_HISTORY_MORE, mxMainWindow::OnFileSourceHistoryMore)
	EVT_MENU(mxID_FILE_PROJECT_HISTORY_MORE, mxMainWindow::OnFileProjectHistoryMore)
	EVT_MENU_RANGE(mxID_FILE_SOURCE_HISTORY_0, mxID_FILE_SOURCE_HISTORY_30,mxMainWindow::OnFileSourceHistory)
	EVT_MENU_RANGE(mxID_FILE_PROJECT_HISTORY_0, mxID_FILE_PROJECT_HISTORY_30,mxMainWindow::OnFileProjectHistory)
	EVT_MENU(mxID_FILE_SET_AS_MASTER, mxMainWindow::OnFileSetAsMaster)
	
	EVT_MENU(mxID_EDIT_SELECT_ALL, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_UNDO, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_REDO, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_MAKE_LOWERCASE, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_MAKE_UPPERCASE, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_COPY, mxMainWindow::OnEditNeedFocus)
	EVT_MENU(mxID_EDIT_CUT, mxMainWindow::OnEditNeedFocus)
	EVT_MENU(mxID_EDIT_PASTE, mxMainWindow::OnEditNeedFocus)
	EVT_MENU(mxID_EDIT_TOOLBAR_FIND, mxMainWindow::OnToolbarFindEnter)
	EVT_MENU(mxID_EDIT_FIND_FROM_TOOLBAR, mxMainWindow::OnGotoToolbarFind)
	EVT_MENU(mxID_EDIT_FIND, mxMainWindow::OnEditFind)
	EVT_MENU(mxID_EDIT_FIND_NEXT, mxMainWindow::OnEditFindNext)
	EVT_MENU(mxID_EDIT_FIND_PREV, mxMainWindow::OnEditFindPrev)
	EVT_MENU(mxID_EDIT_REPLACE, mxMainWindow::OnEditReplace)
	EVT_MENU(mxID_EDIT_BRACEMATCH, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_INDENT, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_GOTO, mxMainWindow::OnEditGoto)
	EVT_MENU(mxID_SOURCE_GOTO_DEFINITION, mxMainWindow::OnSourceGotoDefinition)
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
	EVT_MENU(mxID_EDIT_HIGHLIGHT_WORD, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_INSERT_HEADER, mxMainWindow::OnEditInsertInclude)
	EVT_MENU(mxID_EDIT_AUTOCODE_AUTOCOMPLETE, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_FORCE_AUTOCOMPLETE, mxMainWindow::OnEdit)
	EVT_MENU(mxID_EDIT_RECTANGULAR_EDITION, mxMainWindow::OnEdit)
//	EVT_MENU(mxID_EDIT_FUZZY_AUTOCOMPLETE, mxMainWindow::OnEdit)
	EVT_MENU(mxID_NAVIGATION_HISTORY_PREV, mxMainWindow::OnNavigationHistoryPrev)
	EVT_MENU(mxID_NAVIGATION_HISTORY_NEXT, mxMainWindow::OnNavigationHistoryNext)
	
	EVT_MENU(mxID_CHANGE_SHORTCUTS, mxMainWindow::OnChangeShortcuts)
	
	EVT_MENU(mxID_MACRO_RECORD, mxMainWindow::OnMacroRecord)
	EVT_MENU(mxID_MACRO_REPLAY, mxMainWindow::OnMacroReplay)
	
	EVT_MENU(mxID_RUN_RUN, mxMainWindow::OnRunRun)
	EVT_MENU(mxID_RUN_RUN_OLD, mxMainWindow::OnRunRunOld)
	EVT_MENU(mxID_RUN_STOP, mxMainWindow::OnRunStop)
	EVT_MENU(mxID_RUN_BUILD, mxMainWindow::OnRunBuild)
	EVT_MENU(mxID_RUN_COMPILE, mxMainWindow::OnRunCompile)
	EVT_MENU(mxID_RUN_CLEAN, mxMainWindow::OnRunClean)
	EVT_MENU(mxID_RUN_CONFIG, mxMainWindow::OnRunCompileConfig)
	
	EVT_MENU(mxID_DEBUG_ATTACH, mxMainWindow::OnDebugAttach)
	EVT_MENU(mxID_DEBUG_TARGET, mxMainWindow::OnDebugTarget)
	EVT_MENU(mxID_DEBUG_PATCH, mxMainWindow::OnDebugPatch)
	EVT_MENU(mxID_DEBUG_SAVE_CORE_DUMP, mxMainWindow::OnDebugCoreDump)
	EVT_MENU(mxID_DEBUG_LOAD_CORE_DUMP, mxMainWindow::OnDebugCoreDump)
	EVT_MENU(mxID_DEBUG_SEND_SIGNAL, mxMainWindow::OnDebugSendSignal)
	EVT_MENU(mxID_DEBUG_SET_SIGNALS, mxMainWindow::OnDebugSetSignals)
	EVT_MENU(mxID_DEBUG_GDB_COMMAND, mxMainWindow::OnDebugGdbCommand)
	EVT_MENU(mxID_DEBUG_THREADLIST, mxMainWindow::OnDebugThreadList)
	EVT_MENU(mxID_DEBUG_BACKTRACE, mxMainWindow::OnDebugBacktrace)
//	EVT_MENU(mxID_DEBUG_UPDATE_INSPECTIONS, mxMainWindow::OnDebugUpdateInspections)
	EVT_MENU(mxID_DEBUG_INSPECT, mxMainWindow::OnDebugInspect)
	EVT_MENU(mxID_DEBUG_STOP, mxMainWindow::OnDebugStop)
	EVT_MENU(mxID_DEBUG_PAUSE, mxMainWindow::OnDebugPause)
	EVT_MENU(mxID_DEBUG_RUN, mxMainWindow::OnDebugRun)
	EVT_MENU(mxID_DEBUG_STEP_IN, mxMainWindow::OnDebugStepIn)
	EVT_MENU(mxID_DEBUG_STEP_OUT, mxMainWindow::OnDebugStepOut)
	EVT_MENU(mxID_DEBUG_STEP_OVER, mxMainWindow::OnDebugStepOver)
	EVT_MENU(mxID_DEBUG_BREAKPOINT_OPTIONS, mxMainWindow::OnDebugBreakpointOptions)
	EVT_MENU(mxID_DEBUG_INSERT_WATCHPOINT, mxMainWindow::OnDebugInsertWatchpoint)
	EVT_MENU(mxID_DEBUG_LIST_BREAKPOINTS, mxMainWindow::OnDebugListBreakpoints)
	EVT_MENU(mxID_DEBUG_TOGGLE_BREAKPOINT, mxMainWindow::OnDebugToggleBreakpoint)
	EVT_MENU(mxID_DEBUG_ENABLE_DISABLE_BREAKPOINT, mxMainWindow::OnDebugEnableDisableBreakpoint)
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
	EVT_MENU(mxID_VIEW_TOOLBAR_PROJECT, mxMainWindow::OnViewToolbarProject)
	EVT_MENU(mxID_VIEW_TOOLBAR_MISC, mxMainWindow::OnViewToolbarMisc)
	EVT_MENU(mxID_VIEW_TOOLBAR_TOOLS, mxMainWindow::OnViewToolbarTools)
	EVT_MENU(mxID_VIEW_TOOLBAR_VIEW, mxMainWindow::OnViewToolbarView)
	EVT_MENU(mxID_VIEW_TOOLBAR_FILE, mxMainWindow::OnViewToolbarFile)
	EVT_MENU(mxID_VIEW_TOOLBAR_EDIT, mxMainWindow::OnViewToolbarEdit)
	EVT_MENU(mxID_VIEW_TOOLBAR_DEBUG, mxMainWindow::OnViewToolbarDebug)
	EVT_MENU(mxID_VIEW_TOOLBAR_RUN, mxMainWindow::OnViewToolbarRun)
	EVT_MENU(mxID_VIEW_TOOLBAR_FIND, mxMainWindow::OnViewToolbarFind)
	EVT_MENU(mxID_VIEW_TOOLBARS_CONFIG, mxMainWindow::OnViewToolbarsConfig)
	EVT_MENU(mxID_VIEW_WHITE_SPACE, mxMainWindow::OnViewWhiteSpace)
	EVT_MENU(mxID_VIEW_LINE_WRAP, mxMainWindow::OnViewLineWrap)
	EVT_MENU(mxID_VIEW_CODE_STYLE, mxMainWindow::OnViewCodeStyle)
	EVT_MENU(mxID_VIEW_CODE_COLOURS, mxMainWindow::OnViewCodeColours)
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
	
	EVT_MENU(mxID_TOOLS_MAKEFILE, mxMainWindow::OnToolsExportMakefile)
	EVT_MENU(mxID_TOOLS_CREATE_TEMPLATE, mxMainWindow::OnToolsCreateTemplate)
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
	EVT_MENU(mxID_TOOLS_WXFB_CONFIG, mxMainWindow::OnToolsWxfbConfig)
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
	EVT_MENU(mxID_TOOLS_GCOV_SET, mxMainWindow::OnToolsGcovSet)
	EVT_MENU(mxID_TOOLS_GCOV_SHOW, mxMainWindow::OnToolsGcovShow)
	EVT_MENU(mxID_TOOLS_GCOV_RESET, mxMainWindow::OnToolsGcovReset)
	EVT_MENU(mxID_TOOLS_GCOV_HELP, mxMainWindow::OnToolsGcovHelp)
	EVT_MENU(mxID_TOOLS_ALIGN_COMMENTS, mxMainWindow::OnToolsAlignComments)
	EVT_MENU(mxID_TOOLS_REMOVE_COMMENTS, mxMainWindow::OnToolsRemoveComments)
	EVT_MENU(mxID_TOOLS_CPPCHECK_RUN, mxMainWindow::OnToolsCppCheckRun)
	EVT_MENU(mxID_TOOLS_CPPCHECK_CONFIG, mxMainWindow::OnToolsCppCheckConfig)
	EVT_MENU(mxID_TOOLS_CPPCHECK_VIEW, mxMainWindow::OnToolsCppCheckView)
	EVT_MENU(mxID_TOOLS_CPPCHECK_HELP, mxMainWindow::OnToolsCppCheckHelp)
#if !defined(_WIN32) && !defined(__WIN32__)
	EVT_MENU(mxID_TOOLS_VALGRIND_RUN, mxMainWindow::OnToolsValgrindRun)
	EVT_MENU(mxID_TOOLS_VALGRIND_DEBUG, mxMainWindow::OnToolsValgrindDebug)
	EVT_MENU(mxID_TOOLS_VALGRIND_VIEW, mxMainWindow::OnToolsValgrindView)
	EVT_MENU(mxID_TOOLS_VALGRIND_HELP, mxMainWindow::OnToolsValgrindHelp)
#endif
	EVT_MENU_RANGE(mxID_CUSTOM_TOOL_0, mxID_CUSTOM_TOOL_0+MAX_CUSTOM_TOOLS,mxMainWindow::OnToolsCustomTool)
	EVT_MENU_RANGE(mxID_CUSTOM_PROJECT_TOOL_0, mxID_CUSTOM_PROJECT_TOOL_0+MAX_PROJECT_CUSTOM_TOOLS,mxMainWindow::OnToolsCustomProjectTool)
	EVT_MENU(mxID_TOOLS_CUSTOM_TOOLS_SETTINGS, mxMainWindow::OnToolsCustomToolsSettings)
	EVT_MENU(mxID_TOOLS_PROJECT_TOOLS_SETTINGS, mxMainWindow::OnToolsProjectToolsSettings)
	EVT_MENU(mxID_TOOLS_CUSTOM_HELP, mxMainWindow::OnToolsCustomHelp)
	EVT_MENU(mxID_TOOLS_INSTALL_COMPLEMENTS, mxMainWindow::OnToolsInstallComplements)
	
	EVT_MENU(mxID_HELP_SHORTCUTS, mxMainWindow::OnHelpShortcuts)
	EVT_MENU(mxID_HELP_OPINION, mxMainWindow::OnHelpOpinion)
	EVT_MENU(mxID_HELP_TUTORIAL, mxMainWindow::OnHelpTutorial)
	EVT_MENU(mxID_HELP_ABOUT, mxMainWindow::OnHelpAbout)
	EVT_MENU(mxID_HELP_GUI, mxMainWindow::OnHelpGui)
	EVT_MENU(mxID_HELP_CPP, mxMainWindow::OnHelpCpp)
	EVT_MENU(mxID_HELP_CODE, mxMainWindow::OnHelpCode)
	EVT_MENU(mxID_HELP_TIP, mxMainWindow::OnHelpTip)
	EVT_MENU(mxID_HELP_UPDATES, mxMainWindow::OnHelpUpdates)
	
	EVT_MENU_RANGE(mxID_LAST_ID, mxID_LAST_ID+50,mxMainWindow::OnToolbarMenu)
		
	EVT_AUI_PANE_CLOSE(mxMainWindow::OnPaneClose)
	EVT_AUINOTEBOOK_PAGE_CLOSE(mxID_NOTEBOOK_SOURCES, mxMainWindow::OnNotebookPageClose)
	EVT_AUINOTEBOOK_TAB_RIGHT_DOWN(mxID_NOTEBOOK_SOURCES, mxMainWindow::OnNotebookRightClick)
	EVT_AUINOTEBOOK_PAGE_CHANGED(mxID_NOTEBOOK_SOURCES, mxMainWindow::OnNotebookPageChanged)
	EVT_AUINOTEBOOK_PAGE_CHANGED(mxID_LEFT_PANELS, mxMainWindow::OnNotebookPanelsChanged)
	
	EVT_END_PROCESS(wxID_ANY, mxMainWindow::OnProcessTerminate)
	
	EVT_TREE_ITEM_ACTIVATED(wxID_ANY, mxMainWindow::OnSelectTreeItem)
	
	EVT_HTML_LINK_CLICKED(wxID_ANY, mxMainWindow::OnQuickHelpLink)
	
	EVT_CLOSE(mxMainWindow::OnClose)
	
	EVT_TREE_ITEM_RIGHT_CLICK(mxID_TREE_SYMBOLS, mxMainWindow::OnSymbolTreePopup)
	EVT_MENU(mxID_SYMBOL_POPUP_DEC, mxMainWindow::OnSymbolTreeDec)
	EVT_MENU(mxID_SYMBOL_POPUP_DEF, mxMainWindow::OnSymbolTreeDef)
	EVT_MENU(mxID_SYMBOL_POPUP_INCLUDES, mxMainWindow::OnSymbolTreeIncludes)
	EVT_MENU(mxID_SYMBOL_GENERATE_CACHE, mxMainWindow::OnSymbolsGenerateAutocompletionIndex)
	
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
	EVT_MENU(mxID_PROJECT_POPUP_READONLY, mxMainWindow::OnProjectTreeToggleReadOnly)
	EVT_MENU(mxID_PROJECT_POPUP_HIDE_SYMBOLS, mxMainWindow::OnProjectTreeToggleHideSymbols)
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
	
	EVT_TIMER(mxID_TIMER_AFTER_EVENTS, mxMainWindow::OnAfterEventsTimer)
	EVT_MENU(mxID_WHERE_AM_I, mxMainWindow::OnWhereAmI)
	EVT_TIMER(mxID_COMPILER_TIMER, mxMainWindow::OnParseOutputTime)
	EVT_TIMER(mxID_PARSER_TIMER, mxMainWindow::OnParseSourceTime)
	EVT_TIMER(mxID_PARSER_PROCESS_TIMER, mxMainWindow::OnParserContinueProcess)

	EVT_TEXT(mxID_TOOLBAR_FIND, mxMainWindow::OnToolbarFindChange)
	EVT_TEXT_ENTER(mxID_TOOLBAR_FIND, mxMainWindow::OnToolbarFindEnter)
	
//	EVT_KEY_DOWN(mxMainWindow::OnKey)
//	EVT_CHAR_HOOK(mxMainWindow::OnKey)
	EVT_ACTIVATE (mxMainWindow::OnActivate)
	
END_EVENT_TABLE()


/**
* Hay eventos que llaman al AnalizeConfig del proyecto, que puede requerir expandir subcomandos 
* y dejar que en el yield del execute se vuelva a invocar otro de estos eventos provocando las 
* "colisiones" en los executes (yields en yields no tienen efecto). Esta clase maneja un flag
* para evitar procesar uno de esos eventos.
**/
class PreventExecuteYieldExecuteProblem {
	static bool flag; 
	bool owns_flag;
public:
	PreventExecuteYieldExecuteProblem() {
		owns_flag=!flag;
		flag=true;
	}
	~PreventExecuteYieldExecuteProblem() {
		if (owns_flag) flag=false;
	}
	bool IsOk() { return owns_flag; }
};

bool PreventExecuteYieldExecuteProblem::flag=false;

#define _prevent_execute_yield_execute_problem \
	PreventExecuteYieldExecuteProblem prevent_execute_yield_execute_problem; \
	if (!prevent_execute_yield_execute_problem.IsOk()) return
	

mxMainWindow::mxMainWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style) {
	
	
	EXTERNAL_SOURCE=(mxSource*)this;
	focus_source=NULL;
	m_macro=NULL;
	master_source=NULL;
	
	gui_fullscreen_mode=gui_debug_mode=gui_project_mode=false;
	untitled_count=0;
	valgrind_panel=NULL; 
	beginner_panel=NULL;
	gcov_sidebar=NULL;
	diff_sidebar=NULL;
	for (int i=0;i<ATH_COUNT;i++)
		autohide_handlers[i]=NULL;

#ifndef __APPLE__
	// esto genera el problema de "image file is not of type 9"?
	SetIcon(wxIcon(zinjai_xpm));
#endif
	
 	aui_manager.SetManagedWindow(this);
	menu_data->CreateMenuesAndToolbars(this); 
	CreateStatusBar(1,0);
	status_bar->SetStatusText(LANG(MAINW_INITIALIZING,"Inicializando..."));	

	if (config->Init.left_panels && !config->Init.autohiding_panels) {
		aui_manager.AddPane(CreateLeftPanels(), wxAuiPaneInfo().Name("left_panels").Left().CloseButton(true).MaximizeButton(true).Caption("Arboles").Hide());
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
	aui_manager.AddPane(CreateCompilerTree(), wxAuiPaneInfo().Name("compiler_tree").Bottom().Caption(LANG(CAPTION_COMPILER_OUTPUT,"Resultados de la Compilacion")).CloseButton(true).MaximizeButton(true).Hide().MaximizeButton(!config->Init.autohiding_panels));
	aui_manager.AddPane(CreateQuickHelp(), wxAuiPaneInfo().Name("quick_help").Bottom().Caption(LANG(CAPTION_QUIKHELP,"Ayuda Rapida")).CloseButton(true).MaximizeButton(true).Hide().MaximizeButton(!config->Init.autohiding_panels));
	if (config->Debug.inspections_on_right)
		aui_manager.AddPane((wxGrid*)(inspection_ctrl = new mxInspectionsPanel()), wxAuiPaneInfo().Name("inspection").Caption(LANG(CAPTION_INSPECTIONS,"Inspecciones")).Right().CloseButton(true).MaximizeButton(true).Hide().Position(0).MaximizeButton(!config->Init.autohiding_panels));
	else
		aui_manager.AddPane((wxGrid*)(inspection_ctrl = new mxInspectionsPanel()), wxAuiPaneInfo().Name("inspection").Caption(LANG(CAPTION_INSPECTIONS,"Inspecciones")).Bottom().CloseButton(true).MaximizeButton(true).Hide().Position(2).MaximizeButton(!config->Init.autohiding_panels));
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_INSPECTIONS] = new mxHidenPanel(this,inspection_ctrl,config->Debug.inspections_on_right?HP_RIGHT:HP_BOTTOM,LANG(MAINW_AUTOHIDE_INSPECTIONS,"Inspecciones"));
	aui_manager.AddPane((wxGrid*)(backtrace_ctrl = new mxBacktraceGrid(this)), wxAuiPaneInfo().Name("backtrace").Caption(LANG(CAPTION_BACKTRACE,"Trazado Inverso")).Bottom().CloseButton(true).MaximizeButton(true).Hide().Position(1).MaximizeButton(!config->Init.autohiding_panels));
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_BACKTRACE] = new mxHidenPanel(this,backtrace_ctrl,HP_BOTTOM,LANG(MAINW_AUTOHIDE_BACKTRACE,"Trazado Inverso"));
	aui_manager.AddPane((wxGrid*)(threadlist_ctrl = new mxThreadGrid(this)), wxAuiPaneInfo().Name("threadlist").Caption(LANG(CAPTION_THREADLIST,"Hilos de Ejecucion")).Bottom().CloseButton(true).MaximizeButton(true).Hide().Position(0).MaximizeButton(!config->Init.autohiding_panels));
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_THREADS] = new mxHidenPanel(this,threadlist_ctrl,HP_BOTTOM,LANG(MAINW_AUTOHIDE_THREADS,"Hilos"));
	aui_manager.AddPane(CreateNotebookSources(), wxAuiPaneInfo().Name("notebook_sources").CenterPane().PaneBorder(false));
	aui_manager.AddPane(debug_log_panel=new wxListBox(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,NULL,wxLB_HSCROLL),wxAuiPaneInfo().Name("debug_messages").Bottom().Caption(LANG(CAPTION_DEBUGGER_LOG,"Mensajes del Depurador")).CloseButton(true).MaximizeButton(true).Hide().MaximizeButton(!config->Init.autohiding_panels));
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_DEBUG_LOG] = new mxHidenPanel(this,debug_log_panel,HP_BOTTOM,LANG(MAINW_AUTOHIDE_DEBUG_LOG,"Log Depurador"));
	
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
		
	if (config->Init.show_welcome) {
		welcome_panel=new mxWelcomePanel(this);
		aui_manager.AddPane(welcome_panel, wxAuiPaneInfo().Name("welcome_panel").CenterPane().PaneBorder(false).Hide());
	}

	aui_manager.SetFlags(aui_manager.GetFlags() | wxAUI_MGR_TRANSPARENT_DRAG | wxAUI_MGR_LIVE_RESIZE);
	
	if (config->Init.show_explorer_tree) {
		wxCommandEvent evt;
		OnViewExplorerTree(evt); // aui update
	}
	aui_manager.Update();

	parser = new Parser(this);
	code_helper->AppendIndexes(config->Help.autocomp_indexes);
	autocoder = new Autocoder;
	
	compiler = new mxCompiler(compiler_tree.treeCtrl,compiler_tree.state,compiler_tree.errors,compiler_tree.warnings,compiler_tree.all);

	wizard = NULL; //new mxNewWizard(this);
	share = NULL; // new ShareManager();

	parser_timer = new wxTimer(GetEventHandler(),mxID_PARSER_TIMER);
	compiler->timer = new wxTimer(GetEventHandler(),mxID_COMPILER_TIMER);
	find_replace_dialog = NULL; // new mxFindDialog(this,wxID_ANY);
	
	current_after_events_action = call_after_events = NULL;
	after_events_timer = new wxTimer(GetEventHandler(),mxID_TIMER_AFTER_EVENTS);
	
	SetDropTarget(new mxDropTarget(NULL));
	
	if (config->Init.show_beginner_panel) {
		CreateBeginnersPanel();
		_menu_item(mxID_VIEW_BEGINNER_PANEL)->Check(true);
		if (!config->Init.show_welcome)
			ShowBeginnersPanel();
	}
	
	status_bar->SetStatusText(LANG(GENERAL_READY,"Listo"));
	
	Show(true); Maximize(config->Init.maximized);
	
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

void mxMainWindow::PopulateProjectFilePopupMenu(wxMenu &menu, project_file_item *fi, bool for_tab) {
	if (!fi) menu.Append(mxID_PROJECT_POPUP_ADD_SELECTED, LANG(MAINW_PROJECT_FILE_POPUP_ADD_TO_PROJECT,"Agregar al proyecto"));
	if (fi) project_tree.selected_parent = project_tree.treeCtrl->GetItemParent(project_tree.selected_item);
	if (!for_tab) menu.Append(mxID_PROJECT_POPUP_OPEN, LANG(MAINW_PROJECT_FILE_POPUP_SHOW_FILE,"&Mostrar Archivo"));
	if (fi) menu.Append(mxID_PROJECT_POPUP_RENAME, LANG(MAINW_PROJECT_FILE_POPUP_RENAME,"&Renombrar Archivo..."));
	if (fi) menu.Append(mxID_PROJECT_POPUP_DELETE, LANG(MAINW_PROJECT_FILE_POPUP_DETACH,"&Quitar Archivo"));
	if (fi) {
		menu.AppendSeparator();
		if (project_tree.selected_parent==project_tree.sources) {
			wxMenuItem *item=menu.AppendCheckItem(mxID_PROJECT_POPUP_COMPILE_FIRST, LANG(MAINW_PROJECT_FILE_POPUP_COMPILE_FIRST,"Compilar &Primero"));
			item->Enable(fi!=project->files_sources[0] && !compiler->IsCompiling()); item->Check(fi==project->files_sources[0]);
			menu.Append(mxID_PROJECT_POPUP_COMPILE_NOW, LANG(MAINW_PROJECT_FILE_POPUP_RECOMPILE,"Recompilar A&hora"))->Enable(!compiler->IsCompiling());
		}
		menu.AppendCheckItem(mxID_PROJECT_POPUP_READONLY, LANG(MAINW_PROJECT_FILE_POPUP_READONLY,"Solo lectura"))->Check(fi->read_only);
		if (fi->where!=FT_OTHER) menu.AppendCheckItem(mxID_PROJECT_POPUP_HIDE_SYMBOLS, LANG(MAINW_PROJECT_FILE_POPUP_HIDE_SYMBOLS,"Ignorar símbolos en búsquedas"))->Check(fi->hide_symbols);
	}
	menu.AppendSeparator();
	if (!for_tab && fi) {
		if (project_tree.selected_parent!=project_tree.sources)
			menu.Append(mxID_PROJECT_POPUP_MOVE_TO_SOURCES, LANG(MAINW_PROJECT_FILE_POPUP_MOVE_TO_SOURCES,"Mover a &Fuentes"));
		if (project_tree.selected_parent!=project_tree.headers)
			menu.Append(mxID_PROJECT_POPUP_MOVE_TO_HEADERS, LANG(MAINW_PROJECT_FILE_POPUP_MOVE_TO_HEADERS,"Mover a &Cabeceras"));
		if (project_tree.selected_parent!=project_tree.others)
			menu.Append(mxID_PROJECT_POPUP_MOVE_TO_OTHERS, LANG(MAINW_PROJECT_FILE_POPUP_MOVE_TO_OTHERS,"Mover a &Otros"));
		menu.AppendSeparator();
	}
	menu.Append(mxID_PROJECT_POPUP_OPEN_FOLDER, LANG(MAINW_PROJECT_FILE_POPUP_OPEN_FOLDER,"Abrir carpeta contenedora..."));
	menu.Append(mxID_FILE_EXPLORE_FOLDER, LANG(MAINW_PROJECT_FILE_POPUP_EXPLORE_FOLDER,"Explorar carpeta contenedora"));
	menu.Append(mxID_PROJECT_POPUP_PROPERTIES, LANG(MAINW_PROJECT_FILE_POPUP_PROPERTIES,"Propiedades..."));
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
		menu.Append(mxID_PROJECT_POPUP_OPEN_ALL, LANG(MAINW_PROJECT_FILE_POPUP_OPEN_ALL,"Abrir &Todos"));
		menu.Append(mxID_PROJECT_POPUP_ADD, LANG(MAINW_PROJECT_FILE_POPUP_ADD,"&Agregar Archivo..."));
		menu.Append(mxID_PROJECT_POPUP_ADD_MULTI, LANG(MAINW_PROJECT_FILE_POPUP_ADD_MULTI,"&Agregar Múltiples Archivos..."));
	} else {
		PopulateProjectFilePopupMenu(menu,project->FindFromItem(project_tree.selected_item),false); // el if debería ser innecesario en este punto
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
	ShowSpecilaUnnamedSource("<ultima_compilacion>",compiler->full_output);
}

void mxMainWindow::ShowSpecilaUnnamedSource(const wxString &tab_name, const wxArrayString &lines) {
	if (config->Init.show_welcome) main_window->ShowWelcome(false);
	mxSource* source = new mxSource(notebook_sources, AvoidDuplicatePageText(tab_name));
	source->SetStyle(false);
	for (unsigned int i=0;i<lines.GetCount();i++) source->AppendText(lines[i]+"\n");
	notebook_sources->AddPage(source, tab_name ,true, *bitmaps->files.other);
	if (!project) source->treeId = AddToProjectTreeSimple(tab_name,FT_OTHER);
	source->SetModify(false);
	source->SetReadOnlyMode(ROM_SPECIAL);
	source->SetFocus();
}

void mxMainWindow::OnProjectTreeCompileFirst(wxCommandEvent &event) {
	project->MoveFirst(project_tree.selected_item);
}

void mxMainWindow::OnProjectTreeToggleReadOnly(wxCommandEvent &event) {
	project_file_item *item=project->FindFromItem(project_tree.selected_item);
	if (item) project->SetFileReadOnly(item,!item->read_only);
}

void mxMainWindow::OnProjectTreeToggleHideSymbols(wxCommandEvent &event) {
	project_file_item *item=project->FindFromItem(project_tree.selected_item);
	if (item) project->SetFileHideSymbols(item,!item->hide_symbols);
}

void mxMainWindow::OnProjectTreeCompileNow(wxCommandEvent &event) {
	project_file_item *item = project->FindFromItem(project_tree.selected_item);
	if (item) AuxCompileOne(item);
}

void mxMainWindow::AuxCompileOne(project_file_item *item) {
	project->PrepareForBuilding(item);
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
		SetStatusText(wxString(LANG(MAINW_OPENING,"Abriendo"))<<" \""<<project_tree.treeCtrl->GetItemText(item)<<"\"...");
		project_tree.selected_item = item;
		OnProjectTreeOpen(event);
		item = project_tree.treeCtrl->GetNextSibling( item );
	}
	SetStatusText(LANG(GENERAL_READY,"Listo"));
}

void mxMainWindow::OnProjectTreeRename(wxCommandEvent &event) {
	wxFileName fn(DIR_PLUS_FILE(project->path,project->GetNameFromItem(project_tree.selected_item,true)));
	wxFileDialog dlg (this, "Renombrar",fn.GetPath(),fn.GetFullName(), "Any file (*)|*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	dlg.SetDirectory(fn.GetPath());
	dlg.SetWildcard("Todos los archivos|"WILDCARD_ALL"|Archivos de C/C++|"WILDCARD_CPP"|Fuentes|"WILDCARD_SOURCE"|Cabeceras|"WILDCARD_HEADER);
	if (dlg.ShowModal() == wxID_OK)
		if (!project->RenameFile(project_tree.selected_item,dlg.GetPath()))
			mxMessageDialog(main_window,LANG(MAINW_PROBLEM_RENAMING,"No se pudo renombrar el archivo"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		
//	wxString res = mxGetTextFromUser("Nuevo nombre:", "Renombrar archivo" , project->GetNameFromItem(project_tree.selected_item,true), this);
//	if (res!="")
//		if (!project->RenameFile(project_tree.selected_item,res))
//			mxMessageDialog(main_window,"No se pudo renombrar el archivo",LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
}

void mxMainWindow::OnProjectTreeDelete(wxCommandEvent &event) {
	project->DeleteFile(project_tree.selected_item);
}

void mxMainWindow::OnProjectTreeMoveToSources(wxCommandEvent &event) {
	project->MoveFile(project_tree.selected_item,FT_SOURCE);
}

void mxMainWindow::OnProjectTreeMoveToHeaders(wxCommandEvent &event) {
	project->MoveFile(project_tree.selected_item,FT_HEADER);
}

void mxMainWindow::OnProjectTreeMoveToOthers(wxCommandEvent &event) {
	project->MoveFile(project_tree.selected_item,FT_OTHER);
}

void mxMainWindow::OnProjectTreeAddMultiple(wxCommandEvent &event) {
	new mxMultipleFileChooser();
}

void mxMainWindow::OnProjectTreeAddSelected(wxCommandEvent &event) {
	mxSource *src=CURRENT_SOURCE;
	if (project) OpenFile(src->source_filename.GetFullPath(),true);
}

void mxMainWindow::OnProjectTreeAdd(wxCommandEvent &event) {
	wxFileDialog dlg (this, "Abrir Archivo", project?project->last_dir:config->Files.last_dir, " ", "Any file (*)|*", wxFD_OPEN | wxFD_MULTIPLE);
	dlg.SetWildcard("Archivos de C/C++|"WILDCARD_CPP"|Fuentes|"WILDCARD_SOURCE"|Cabeceras|"WILDCARD_HEADER"|Todos los archivos|*");
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
		int multiple=1; // 1 en OpenFileFromGui significa always_attach
		for (unsigned int i=0;i<paths.GetCount();i++) {
			if (!wxFileName::FileExists(dlg.GetPath())) {
				if (mxMD_YES==mxMessageDialog(main_window,LANG(MAINW_CREATE_FILE_QUESTION,"El archivo no existe, desea crearlo?"),dlg.GetPath(),mxMD_YES_NO).ShowModal()) {
					wxTextFile fil(dlg.GetPath());
					fil.Create();
					fil.Write();
				} else
					continue;
			}
			OpenFileFromGui(paths[i],&multiple);
		}
	}
}

void mxMainWindow::OnClose (wxCloseEvent &event) {
	if (debug->IsDebugging()) {
		debug->Stop();
		return;
	}
	if (parser->working) {
		parser->Stop(true);
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
	GetToolbarsPositions();
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
	config->Init.show_beginner_panel=_menu_item(mxID_VIEW_BEGINNER_PANEL)->IsChecked();
	config->Save();
	while (notebook_sources->GetPageCount()) notebook_sources->DeletePage(0); // close sources to avoid paint events and other calls that could use some just deleted objects
	if (share) delete share;
	main_window=NULL;
	er_uninit();
#if defined(__APPLE__)
	aui_manager.GetPane(_get_toolbar(tbFIND)).Hide();
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
	_record_this_action_in_macro(event.GetId());
	IF_THERE_IS_SOURCE CURRENT_SOURCE->HideCalltip();
	if (!find_replace_dialog) find_replace_dialog = new mxFindDialog(this,wxID_ANY);
	if (find_replace_dialog->last_search.Len()) {
		if (!find_replace_dialog->FindNext())
			mxMessageDialog(main_window,LANG1(FIND_NOT_FOUND,"La cadena \"<{1}>\" no se encontro.",find_replace_dialog->last_search), LANG(FIND_CAPTION,"Buscar"), mxMD_OK|mxMD_INFO).ShowModal();
	} else {
		OnEditFind(event);
	}
}

void mxMainWindow::OnEditFindPrev (wxCommandEvent &event) {
	_record_this_action_in_macro(event.GetId());
	IF_THERE_IS_SOURCE CURRENT_SOURCE->HideCalltip();
	if (!find_replace_dialog) find_replace_dialog = new mxFindDialog(this,wxID_ANY);
	if (find_replace_dialog->last_search.Len()) {
		if (!find_replace_dialog->FindPrev())
			mxMessageDialog(main_window,LANG1(FIND_NOT_FOUND,"La cadena \"<{1}>\" no se encontro.",find_replace_dialog->last_search), LANG(FIND_CAPTION,"Buscar"), mxMD_OK|mxMD_INFO).ShowModal();
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
	wxString action(event.GetLinkInfo().GetHref().BeforeFirst(':')), post=event.GetLinkInfo().GetHref().AfterFirst(':');
	if (action=="quickhelp")
		quick_help->SetPage(help->GetQuickHelp( post ));
//	else if (action=="quickfile")
//		quick_help->LoadPage(DIR_PLUS_FILE(config->Help.quickhelp_dir,post));
	else if (action=="doxygen")
		mxUT::OpenInBrowser(wxString("file://")<<post);
//	else if (action=="example")
//		NewFileFromTemplate(DIR_PLUS_FILE(config->Help.quickhelp_dir,post));
	else if (action=="cppreference")
		mxReferenceWindow::ShowAndSearch( post );
	else if (action=="gotoline") {
		wxString the_one=post.BeforeLast(':');
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
		//		if (mxMD_YEW == mxMessageDialog(main_window,wxString("El archivo ")<<the_one.GetFullName()<<" no esta cargado. Desea cargarlo?", the_one.GetFullPath(), mxMD_YES_NO|mxMD_QUESTION).ShowModal();
		mxSource *src=OpenFile(the_one);
		if (src && src!=EXTERNAL_SOURCE) src->MarkError(line-1);
	} else if (action=="gotopos") { // not used anymore?
		mxSource *source=NULL;
		wxString the_one=post.BeforeLast(':').BeforeLast(':');
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
		//		if (mxMD_YEW == mxMessageDialog(main_window,wxString("El archivo ")<<the_one.GetFullName()<<" no esta cargado. Desea cargarlo?", the_one.GetFullPath(), mxMD_YES_NO|mxMD_QUESTION).ShowModal();
		if (!source) source = OpenFile(the_one);
		if (source && source!=EXTERNAL_SOURCE) {
			int line=source->LineFromPosition(p1);
			source->MarkError(line-1);
			source->SetSelection(p1,p1+p2);
		}
	} else if (action=="gotolinepos") {
		mxSource *source=NULL;
		wxString the_one=post.BeforeLast(':').BeforeLast(':').BeforeLast(':');
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
		//		if (mxMD_YEW == mxMessageDialog(main_window,wxString("El archivo ")<<the_one.GetFullName()<<" no esta cargado. Desea cargarlo?", the_one.GetFullPath(), mxMD_YES_NO|mxMD_QUESTION).ShowModal();
		if (!source) source = OpenFile(the_one);
		if (source && source!=EXTERNAL_SOURCE) {
			p1+=source->PositionFromLine(line);
			source->MarkError(line-1);
			source->SetSelection(p1,p1+p2);
		}
	} else
		event.Skip();
}

/// @brief evento generico para el doble click en cualquier arbol, desde aqui se llama al que corresponda
void mxMainWindow::OnSelectTreeItem (wxTreeEvent &event){
//DEBUG_INFO("wxYield:in  mxMainWindow::OnSelectTreeItem");
//	wxYield(); /// para que estaba este yield??
//DEBUG_INFO("wxYield:out mxMainWindow::OnSelectTreeItem");
	if (event.GetEventObject()==project_tree.treeCtrl)
		OnSelectSource(event);
	else if (event.GetEventObject()==compiler_tree.treeCtrl)
		OnSelectError(event);
	else if (event.GetEventObject()==symbols_tree.treeCtrl) {
		parser->OnSelectSymbol(event,notebook_sources);
#if defined(_WIN32) || defined(__WIN32__)
		SetFocusToSourceAfterEvents();
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
			SetFocusToSourceAfterEvents();
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
		else if (source!=EXTERNAL_SOURCE) {
			// el if de abajo se comento porque el "source->SetStyle(false)" ya estaba comentado, y lo del menu esta en OnNotebookPageChanged
//			if (source && project_tree.treeCtrl->GetItemParent(item)==project_tree.others)
				//source->SetStyle(false);
//				_menu_item(mxID_VIEW_CODE_STYLE)->Check(false);
#if defined(_WIN32) || defined(__WIN32__)
			SetFocusToSourceAfterEvents();
#endif
		}
	}
}

#define EN_COMPOUT_FILE_NOT_RECOGNIZED ".o: file not recognized"

void mxMainWindow::OnSelectError (wxTreeEvent &event) {
	// ver si es alguno de los mensajes de zinjai
	wxString item_text=(compiler_tree.treeCtrl->GetItemText(event.GetItem()));
	if (item_text==LANG(MAINW_WARNING_NO_EXCUTABLE_PERMISSION,"El binario no tiene permisos de ejecución.")) {
		LoadInQuickHelpPanel(DIR_PLUS_FILE(config->Help.guihelp_dir,"zerror_noexecperm.html"),false); return;
	} else if (item_text.EndsWith(LANG(PROJMNGR_FUTURE_SOURCE_POST," tenia fecha de modificacion en el futuro. Se reemplazo por la fecha actual."))) {
		LoadInQuickHelpPanel(DIR_PLUS_FILE(config->Help.guihelp_dir,"zerror_futuretimestamp.html"),false); return;
	} else if (item_text==LANG(PROJMNGR_MANIFEST_NOT_FOUND,"No se ha encontrado el archivo manifest.xml.")) {
		LoadInQuickHelpPanel(DIR_PLUS_FILE(config->Help.guihelp_dir,"zerror_missingiconmanifest.html"),false); return;
	} else if (item_text.Contains(EN_COMPOUT_FILE_NOT_RECOGNIZED)) {
		wxString obj_name = item_text.Mid(0,item_text.Find(EN_COMPOUT_FILE_NOT_RECOGNIZED));
		LocalListIterator<project_file_item*> fi(&project->files_sources);
		while(fi.IsValid()) {
			project_file_item *p = *fi;
			if (obj_name == wxFileName(p->name).GetName()) {
				int res=mxMessageDialog(main_window,LANG1(MAINW_SAVE_LINK_ERROR_TRUNCATED_FILE,"Este error puede deberse a compilaciones interrumpidas, o a la presencia\n"
																								"de objetos compilados en otros sistemas. Si este fuera el caso, podría\n"
																								" solucionarse simplemente recompilando el fuente asociado. ¿Desea recompilar\n"
																								"\"<{1}>\" ahora?",p->name), p->name, mxMD_YES_NO|mxMD_QUESTION).ShowModal();
				if (res==mxMD_YES) { AuxCompileOne(p); return; }
				break;
			}
			fi.Next();
		}
	}
	// ver que dijo el compilador
DEBUG_INFO("wxYield:in  mxMainWindow::OnSelectError");
	wxYield();
DEBUG_INFO("wxYield:out mxMainWindow::OnSelectError");
	mxCompilerItemData *comp_data = (mxCompilerItemData*)(compiler_tree.treeCtrl->GetItemData(event.GetItem()));
	wxString error = comp_data ? comp_data->file_info : compiler_tree.treeCtrl->GetItemText(event.GetItem());
	if (!error.Len()) error=compiler_tree.treeCtrl->GetItemText(event.GetItem());;
	if (error.Len()) OnSelectErrorCommon(error);
}

void mxMainWindow::OnFileOpenH(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		if (source->sin_titulo) return;
		wxString the_one(mxUT::GetComplementaryFile(source->source_filename));
		if (the_one.Len()) OpenFile(the_one);
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
			base_path=source->GetPath();
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
			OpenFile(the_one.GetFullPath());
		else 
			new mxGotoFileDialog(source->GetTextRange(p1,p2),this);
		
	}
}

void mxMainWindow::OnHelpOpinion (wxCommandEvent &event){
	mxUT::OpenZinjaiSite("contacto.php");
//	new mxOpinionWindow(this);
}

void mxMainWindow::OnHelpTutorial (wxCommandEvent &event){
	mxHelpWindow::ShowHelp("tutorials.html");
}

void mxMainWindow::OnHelpAbout (wxCommandEvent &event){
	new mxAboutWindow(this);
}

void mxMainWindow::OnHelpGui (wxCommandEvent &event){
	mxHelpWindow::ShowHelp();
}

void mxMainWindow::OnHelpTip (wxCommandEvent &event){
	new mxTipsWindow(this, wxID_ANY, wxDefaultPosition, wxSize(500, 300));
}

void mxMainWindow::OnHelpCpp (wxCommandEvent &event) {
	mxReferenceWindow::ShowPage();
}

void mxMainWindow::OnHelpCode (wxCommandEvent &event) {
	wxString key;
	IF_THERE_IS_SOURCE { // si hay fuente abierto
		mxSource *source=CURRENT_SOURCE;
		int pos=source->GetCurrentPos(); // buscar la palabra sobre el cursor
		key = source->GetCurrentKeyword(pos);
		if (key.StartsWith("#")) {
			mxReferenceWindow::ShowAndSearch("Preprocessor");
			return;
		}
		int s=source->GetStyleAt(pos);
		if (s==wxSTC_C_WORD||s==wxSTC_C_WORD2) {
			mxReferenceWindow::ShowAndSearch(key);
			return;
		}
	}
	if (!key.Len()) // si no hay clave, preguntar
		key = mxGetTextFromUser(LANG(QUICKHELP_WORDS_TO_SEARCH,"Palabra a buscar:"), LANG(CAPTION_QUICKHELP,"Ayuda Rapida") , "", this);
	if (key=="Zaskar") {
		new mxSplashScreen(zskr,GetPosition().x+GetSize().x/2-100,GetPosition().y+GetSize().y/2-150);
		wxString s("Hola, este soy yo... Pablo Novara, alias Zaskar... ;).");
		ShowInQuickHelpPanel(s);
	} else if (key.Len())
		ShowQuickHelp(key); // buscar en la ayuda y mostrar
}


void mxMainWindow::OnNotebookPageChanged(wxAuiNotebookEvent& event) {
	static wxMenuItem *menu_view_white_space=_menu_item(mxID_VIEW_WHITE_SPACE);
	static wxMenuItem *menu_view_line_wrap=_menu_item(mxID_VIEW_LINE_WRAP);
	static wxMenuItem *menu_view_code_style=_menu_item(mxID_VIEW_CODE_STYLE);
//	if (page_change_event_on) {
		if (diff_sidebar) diff_sidebar->Refresh();
		int old_sel = event.GetOldSelection();
		if (old_sel!=-1) {
			mxSource *old_source = (mxSource*)notebook_sources->GetPage(old_sel);
			if (old_source) old_source->HideCalltip();
		}
		menu_view_white_space->Check(CURRENT_SOURCE->config_source.whiteSpace);
		menu_view_line_wrap->Check(CURRENT_SOURCE->config_source.wrapMode);
		menu_view_code_style->Check(CURRENT_SOURCE->config_source.syntaxEnable);
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
		project_file_item *fi=project->FindFromItem(src->treeId);
		if (fi) {
			// seleccionarlo en el arbol de proyecto
			project_tree.treeCtrl->SelectItem(src->treeId);
			project_tree.selected_item = src->treeId;
			project_tree.selected_parent = project_tree.treeCtrl->GetItemParent(project_tree.selected_item);
			// colocar las opciones comunes al popup del arbol de proyecto
			PopulateProjectFilePopupMenu(menu,fi,true);
		} else {
			PopulateProjectFilePopupMenu(menu,NULL,true);
		}
	}
	menu.AppendSeparator();
	if (!src->sin_titulo) { 
		wxString comp = mxUT::GetComplementaryFile(src->source_filename); 
		if (comp.Len()) menu.Append(mxID_FILE_OPEN_H, LANG1(MAINW_OPEN_FILENAME,"Abrir \"<{1}>\"",comp));
	}
	
	/*wxMenuItem *shared = */mxUT::AddItemToMenu(&menu,_menu_item_2(mnTOOLS,mxID_TOOLS_SHARE_SHARE));
//	shared->Check(share && share->Exists(src));
		
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnVIEW,mxID_VIEW_DUPLICATE_TAB));
	if (!project) menu.AppendCheckItem(mxID_FILE_SET_AS_MASTER, LANG(MENUITEM_FILE_SET_AS_MASTER,"Ejecutar siempre este fuente"))->Check(src==master_source);
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnFILE,mxID_FILE_SAVE));
	if (!project) mxUT::AddItemToMenu(&menu,_menu_item_2(mnFILE,mxID_FILE_SAVE_AS));
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnFILE,mxID_FILE_RELOAD));
	mxUT::AddItemToMenu(&menu,_menu_item_2(mnFILE,mxID_FILE_CLOSE));
	if (notebook_sources->GetPageCount()>1)
		mxUT::AddItemToMenu(&menu,_menu_item_2(mnHIDDEN,mxID_FILE_CLOSE_ALL_BUT_ONE));
	notebook_sources->PopupMenu(&menu);
}

void mxMainWindow::OnNotebookPageClose(wxAuiNotebookEvent& event) {
	mxSource *source = (mxSource*)notebook_sources->GetPage(event.GetSelection());
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
		if (source->next_source_with_same_file==source) project_tree.treeCtrl->Delete(source->treeId);
	} else {
		source->UpdateExtras();
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
		parser->RemoveFile(source->GetFullPath());
	else
		parser->ParseIfUpdated(source->source_filename);
//	debug->OnSourceClosed(source); // supuestamente lo hace el destructor de mxSource llamando a debug->UnregisterSource
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
		if (!config->Init.autohiding_panels) _menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
	} else if (event.pane->name == "diff_sidebar") {
		aui_manager.DetachPane(diff_sidebar); diff_sidebar->Destroy(); diff_sidebar=NULL;
	} else if (event.pane->name == "gcov_sidebar") {
		aui_manager.DetachPane(gcov_sidebar); gcov_sidebar->Destroy(); gcov_sidebar=NULL;
	} else if (event.pane->name == "left_panels")
		_menu_item(mxID_VIEW_LEFT_PANELS)->Check(false);
	else if (event.pane->name == "project_tree") {
		if (!config->Init.autohiding_panels) _menu_item(mxID_VIEW_PROJECT_TREE)->Check(false);
	} else if (event.pane->name == "explorer_tree") {
		if (!config->Init.autohiding_panels) _menu_item(mxID_VIEW_EXPLORER_TREE)->Check(false);
	} else if (event.pane->name == "symbols_tree") {
		_menu_item(mxID_VIEW_SYMBOLS_TREE)->Check(false);
	}
	else if (event.pane->name == "toolbar_misc") { _menu_item(mxID_VIEW_TOOLBAR_MISC)->Check(false); if (!gui_debug_mode && !gui_fullscreen_mode) _toolbar_visible(tbMISC)=false; }
	else if (event.pane->name == "toolbar_find") { _menu_item(mxID_VIEW_TOOLBAR_FIND)->Check(false); if (!gui_debug_mode && !gui_fullscreen_mode) _toolbar_visible(tbFIND)=false; }
	else if (event.pane->name == "toolbar_view") { _menu_item(mxID_VIEW_TOOLBAR_VIEW)->Check(false); if (!gui_debug_mode && !gui_fullscreen_mode) _toolbar_visible(tbVIEW)=false; }
	else if (event.pane->name == "toolbar_project") { _menu_item(mxID_VIEW_TOOLBAR_PROJECT)->Check(false); if (!gui_debug_mode && !gui_fullscreen_mode) _toolbar_visible(tbPROJECT)=false; }
	else if (event.pane->name == "toolbar_tools") { _menu_item(mxID_VIEW_TOOLBAR_TOOLS)->Check(false); if (!gui_debug_mode && !gui_fullscreen_mode) _toolbar_visible(tbTOOLS)=false; }
	else if (event.pane->name == "toolbar_file") { _menu_item(mxID_VIEW_TOOLBAR_FILE)->Check(false); if (!gui_debug_mode && !gui_fullscreen_mode) _toolbar_visible(tbFILE)=false; }
	else if (event.pane->name == "toolbar_edit") { _menu_item(mxID_VIEW_TOOLBAR_EDIT)->Check(false); if (!gui_debug_mode && !gui_fullscreen_mode) _toolbar_visible(tbEDIT)=false; }
	else if (event.pane->name == "toolbar_run") { _menu_item(mxID_VIEW_TOOLBAR_RUN)->Check(false); if (!gui_debug_mode && !gui_fullscreen_mode) _toolbar_visible(tbRUN)=false; }
	else if (event.pane->name == "toolbar_debug") { _menu_item(mxID_VIEW_TOOLBAR_DEBUG)->Check(false); if (!gui_debug_mode && !gui_fullscreen_mode) _toolbar_visible(tbDEBUG)=false; }
	else if (event.pane->name == "threadlist" && !config->Init.autohide_panels) debug->threadlist_visible=false;
//	else if (event.pane->name == "backtrace") debug->backtrace_visible=false;
	else if (event.pane->name == "beginner_panel") _menu_item(mxID_VIEW_BEGINNER_PANEL)->Check(false);
}


void mxMainWindow::OnExit(wxCommandEvent &event) {
    Close(true);
}


mxMainWindow::~mxMainWindow() {
	main_window=NULL;
    aui_manager.UnInit();
}


void mxMainWindow::OnEdit (wxCommandEvent &event) {
	_record_this_action_in_macro(event.GetId());
	IF_THERE_IS_SOURCE {
		CURRENT_SOURCE->ProcessEvent(event);
		CURRENT_SOURCE->SetFocus();
	}
}

void mxMainWindow::OnEditNeedFocus (wxCommandEvent &event) {
#warning usar esto para las demas acciones de edicion que tambien tengan atajos compartidos
	_record_this_action_in_macro(event.GetId());
	wxWindow *focus = main_window->FindFocus();
	if (focus && focus->IsKindOf(menu_data->toolbar_find_text->GetClassInfo())) {
		focus->ProcessEvent(event);
	} else if (focus && (focus==inspection_ctrl || focus->GetParent()==inspection_ctrl)) {
//		inspection_ctrl->OnRedirectedEditEvent(event);
	} else IF_THERE_IS_SOURCE {
		CURRENT_SOURCE->ProcessEvent(event);
	}
}


wxHtmlWindow* mxMainWindow::CreateQuickHelp(wxWindow* parent) {
    quick_help = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(400,300));
    quick_help->SetPage(LANG(MAINW_QUICKHELP_INIT,"Coloca el cursor de texto sobre una palabra y presiona Shift+F1 para ver la ayuda en este cuadro."));
	
	if (config->Init.autohiding_panels) {
		autohide_handlers[ATH_QUICKHELP] = new mxHidenPanel(this,quick_help,HP_BOTTOM,LANG(MAINW_AUTOHIDE_QUICKHELP,"Ayuda/Busqueda"));
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
	
	imglist->Add(wxBitmap(SKIN_FILE("ap_folder.png"),wxBITMAP_TYPE_PNG));
	imglist->Add(*(bitmaps->files.source));
	imglist->Add(*(bitmaps->files.header));
	imglist->Add(*(bitmaps->files.other));
	imglist->Add(*(bitmaps->files.blank));
	imglist->Add(wxBitmap(SKIN_FILE("ap_zpr.png"),wxBITMAP_TYPE_PNG));
	explorer_tree.treeCtrl->AssignImageList(imglist);
	
	explorer_tree.show_only_sources = false;
	
	if (config->Init.autohiding_panels) {
		autohide_handlers[ATH_EXPLORER] = new mxHidenPanel(this,explorer_tree.treeCtrl,HP_LEFT,LANG(MAINW_AUTOHIDE_EXPLORER,"Explorador"));
	}
	
	return explorer_tree.treeCtrl;
}

wxTreeCtrl* mxMainWindow::CreateProjectTree() {

	project_tree.treeCtrl = new mxTreeCtrl(this, mxID_TREE_PROJECT, wxPoint(0,0), wxSize(160,100), wxTR_DEFAULT_STYLE | wxNO_BORDER | wxTR_HIDE_ROOT);
					
	wxImageList* imglist = new wxImageList(16, 16,true,5);
	
	imglist->Add(wxBitmap(SKIN_FILE("ap_folder.png"),wxBITMAP_TYPE_PNG));
	imglist->Add(*(bitmaps->files.source));
	imglist->Add(*(bitmaps->files.header));
	imglist->Add(*(bitmaps->files.other));
	imglist->Add(*(bitmaps->files.blank));
	imglist->Add(wxBitmap(SKIN_FILE("ap_wxfb.png"),wxBITMAP_TYPE_PNG));
	project_tree.treeCtrl->AssignImageList(imglist);
	
	project_tree.root = project_tree.treeCtrl->AddRoot("Archivos Abiertos", 0);
	wxArrayTreeItemIds items;
	project_tree.sources = project_tree.treeCtrl->AppendItem(project_tree.root, LANG(MAINW_PT_SOURCES,"Fuentes"), 0);
	project_tree.headers = project_tree.treeCtrl->AppendItem(project_tree.root, LANG(MAINW_PT_HEADERS,"Cabeceras"), 0);
	project_tree.others = project_tree.treeCtrl->AppendItem(project_tree.root, LANG(MAINW_PT_OTHERS,"Otros"), 0);
	items.Add(project_tree.sources);
	items.Add(project_tree.headers);
	items.Add(project_tree.others);

	project_tree.treeCtrl->ExpandAll();
	
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_PROJECT] = new mxHidenPanel(this,project_tree.treeCtrl,HP_LEFT,LANG(MAINW_AUTOHIDE_PROJECT,"Proyecto"));
	
	return project_tree.treeCtrl;
}

wxTreeCtrl* mxMainWindow::CreateSymbolsTree() {
	symbols_tree.treeCtrl = new wxTreeCtrl(this, mxID_TREE_SYMBOLS, wxPoint(0,0), wxSize(160,100), wxTR_DEFAULT_STYLE | wxNO_BORDER | wxTR_HIDE_ROOT);
	wxImageList* imglist = new wxImageList(16, 16, true, 15);
	imglist->Add(wxBitmap(SKIN_FILE("as_folder.png"),wxBITMAP_TYPE_PNG));
	imglist->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16)));
	
	imglist->Add(*(bitmaps->parser.icon02_define));
	imglist->Add(*(bitmaps->parser.icon03_func));
	imglist->Add(*(bitmaps->parser.icon04_class));
	imglist->Add(*(bitmaps->parser.icon05_att_unk));
	imglist->Add(*(bitmaps->parser.icon06_att_pri));
	imglist->Add(*(bitmaps->parser.icon07_att_pro));
	imglist->Add(*(bitmaps->parser.icon08_att_pub));
	imglist->Add(*(bitmaps->parser.icon09_mem_unk));
	imglist->Add(*(bitmaps->parser.icon10_mem_pri));
	imglist->Add(*(bitmaps->parser.icon11_mem_pro));
	imglist->Add(*(bitmaps->parser.icon12_mem_pub));
	imglist->Add(*(bitmaps->parser.icon13_none));
	imglist->Add(*(bitmaps->parser.icon14_global_var));
	imglist->Add(*(bitmaps->parser.icon18_typedef));
	imglist->Add(*(bitmaps->parser.icon19_enum_const));
	
	symbols_tree.treeCtrl->AssignImageList(imglist);
//	symbols_tree.treeCtrl->AddRoot("Simbolos encontrados", 0);
	
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_SYMBOL] = new mxHidenPanel(this,symbols_tree.treeCtrl,HP_LEFT,LANG(MAINW_AUTOHIDE_SYMBOLS,"Simbolos"));
	
	return symbols_tree.treeCtrl;
}


wxPanel* mxMainWindow::CreateCompilerTree() {
	
	compiler_panel=new wxPanel(this,wxID_ANY,wxDefaultPosition,wxSize(160,250));
	
	compiler_tree.treeCtrl = new wxTreeCtrl(compiler_panel, mxID_TREE_COMPILER, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxNO_BORDER | wxTR_HIDE_ROOT);
// 	wxFont tree_font=compiler_tree.treeCtrl->GetFont();
// 	tree_font.SetFaceName("courier");
// 	compiler_tree.treeCtrl->SetFont(tree_font);
	
	wxImageList* imglist = new wxImageList(16, 16, true, 2);
	imglist->Add(wxBitmap(SKIN_FILE("co_folder.png"),wxBITMAP_TYPE_PNG));
	imglist->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16)));
	imglist->Add(wxBitmap(SKIN_FILE("co_info.png"),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE("co_warning.png"),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE("co_error.png"),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE("co_err_info.png"),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE("co_out.png"),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE("co_project_warning.png"),wxBITMAP_TYPE_PNG));
	compiler_tree.treeCtrl->AssignImageList(imglist);
	
	compiler_tree.root = compiler_tree.treeCtrl->AddRoot("Resultados de la Compilacion:", 0);
	wxArrayTreeItemIds items;
	compiler_tree.state = compiler_tree.treeCtrl->AppendItem(compiler_tree.root, "ZinjaI",2);
	compiler_tree.errors = compiler_tree.treeCtrl->AppendItem(compiler_tree.root, LANG(MAINW_CT_ERRORS,"Errores"), 0);
	compiler_tree.warnings = compiler_tree.treeCtrl->AppendItem(compiler_tree.root, LANG(MAINW_CT_WARNINGS,"Advertencias"), 0);
	compiler_tree.all = compiler_tree.treeCtrl->AppendItem(compiler_tree.root, LANG(MAINW_CT_ALL,"Toda la salida"), 0);
	items.Add(compiler_tree.state);
	items.Add(compiler_tree.errors);
	items.Add(compiler_tree.warnings);
	items.Add(compiler_tree.all);
	
	// added for enabling extern toolchains, output will go to a textbox instead of a tree
	wxBoxSizer *compiler_sizer = new wxBoxSizer(wxVERTICAL);
	extern_compiler_output = new mxExternCompilerOutput(compiler_panel);
	wxSizerFlags sf; sf.Expand().Proportion(1).Border(0,0);
	compiler_sizer->Add(compiler_tree.treeCtrl,sf);
	compiler_sizer->Add(extern_compiler_output,sf);
	extern_compiler_output->Hide();
	compiler_panel->SetSizer(compiler_sizer);
	
	if (config->Init.autohiding_panels)
		autohide_handlers[ATH_COMPILER] = new mxHidenPanel(this,compiler_panel,HP_BOTTOM,LANG(MAINW_AUTOHIDE_COMPILER,"Compilador"));
	
	return compiler_panel;
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
	bool there_are_other_compiling_now=false;
	// ver si es uno de los procesos que se estan esperando, y si era el ultimo del compilador para que se detenga el timer que analiza su salida
	compile_and_run_struct_single *compile_and_run=compiler->compile_and_run_single;
	while (compile_and_run && compile_and_run->pid!=event.GetPid()) {
		if (compile_and_run->process && compile_and_run->compiling) there_are_other_compiling_now=true;
		compile_and_run=compile_and_run->next;
	}
	if (!compile_and_run) { // esto no deberia ocurrir?
		DEBUG_INFO("Warning: Unknown process id: "<<event.GetPid());
		cerr<<"Warning: Unknown process id: "<<event.GetPid()<<endl;
		return;
	}
	// ver si hay otro proceso de compilacion en curso en paralelo
	compile_and_run_struct_single *aux_compile_and_run=compile_and_run->next;
	while (!there_are_other_compiling_now && aux_compile_and_run) {
		if (aux_compile_and_run->process && aux_compile_and_run->compiling) there_are_other_compiling_now=true;
		aux_compile_and_run=aux_compile_and_run->next;
	}
	if (!there_are_other_compiling_now) {
		compiler->timer->Stop();
		_menu_item(mxID_RUN_STOP)->Enable(false);
		_menu_item(mxID_RUN_RUN)->Enable(true);
		_menu_item(mxID_RUN_COMPILE)->Enable(true);
		_menu_item(mxID_RUN_CLEAN)->Enable(true);
//		menu.tools_makefile->Enable(true);
	}
	
	// si es uno interrumpido adrede, liberar memoria y no hacer nada mas
	if (compile_and_run->killed) { 
		SetCompilingStatus(LANG(MAINW_STATUS_RUN_FINISHED,"Ejecucion Finalizada"));
		delete compile_and_run; return;
	}

	// actualizar el compiler_tree
	if (compile_and_run->compiling) { // si termino la compilacion
		compiler->ParseCompilerOutput(compile_and_run,event.GetExitCode()==0);
	} else { // si termino la ejecucion
		SetCompilingStatus(LANG(MAINW_STATUS_RUN_FINISHED,"Ejecucion Finalizada"));
		if (compile_and_run->valgrind_cmd.Len()) ShowValgrindPanel(mxVO_VALGRIND,DIR_PLUS_FILE(config->temp_dir,"valgrind.out"));
		delete compile_and_run->process;
		delete compile_and_run;
		if (gcov_sidebar) gcov_sidebar->LoadData();
	}
}

void mxMainWindow::OnRunClean (wxCommandEvent &event) {
	_prevent_execute_yield_execute_problem;
	if (project) project->Clean();
	else IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		if (src->sin_titulo) return;
		wxRemoveFile(src->GetBinaryFileName().GetFullPath());
	}
}


void mxMainWindow::OnRunBuild (wxCommandEvent &event) {
	_prevent_execute_yield_execute_problem;
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
			SetCompilingStatus(LANG(MAINW_COULDNOT_LAUNCH_PROCESS,"No se pudo lanzar el proceso"));
		else
			SetCompilingStatus(LANG(MAINW_COULDNOT_RUN,"No se pudo lanzar la ejecucion!"));
		delete compile_and_run;
		return;
	}
	// habilitar y deshabilitar lo que corresponda en menues
	_menu_item(mxID_RUN_STOP)->Enable(true);
	_menu_item(mxID_RUN_CLEAN)->Enable(false);
	_menu_item(mxID_RUN_COMPILE)->Enable(false);
	if (compile_and_run->compiling) {
		_menu_item(mxID_RUN_RUN)->Enable(false);
//		menu.tools_makefile->Enable(false);
		// mostrar el arbol de compilacion
		if (!config->Init.autohiding_panels) {
			if (!aui_manager.GetPane(compiler_panel).IsShown()) {
				aui_manager.GetPane(quick_help).Hide();
				aui_manager.GetPane(compiler_panel).Show();
				_menu_item(mxID_VIEW_COMPILER_TREE)->Check(true);
				aui_manager.Update();
			}
		}
		
		compiler->timer->Start(500);
	}
	// informar al usuario
	if (msg.Len()) SetCompilingStatus(msg,!project);
}


void mxMainWindow::OnRunRun (wxCommandEvent &event) {
	_prevent_execute_yield_execute_problem;
	if (!compiler->valgrind_cmd.Len() && config->Debug.always_debug) { // si siempre hay que ejecutar en el depurador
		OnDebugRun(event); // patearle la bocha
		return;
	}
	IF_THERE_IS_SOURCE CURRENT_SOURCE->HideCalltip();
	if (project) { // si hay que ejecutar un proyecto
		compiler->BuildOrRunProject(true,false,false);
		
	} else IF_THERE_IS_SOURCE { // si hay que ejecutar un ejercicio
		mxSource *source=CURRENT_SOURCE;
		if (source->sin_titulo) { // si no esta guardado, siempre compilar
			if (source->GetLine(0).StartsWith("make me a sandwich")) { wxMessageBox("No way!"); return; }
			else if (source->GetLine(0).StartsWith("sudo make me a sandwich")) source->SetText(wxString("/** Ok, you win! **/")+wxString(250,' ')+"#include <iostream>\n"+wxString(250,' ')+"int main(int argc, char *argv[]) {std::cout<<\"Here you are:\\n\\n   /-----------\\\\\\n  ~~~~~~~~~~~~~~~\\n   \\\\-----------/\\n\";return 0;}\n\n");
		}
		CompileOrRunSource(true,true,false);
	}
}

void mxMainWindow::OnRunRunOld (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE CURRENT_SOURCE->HideCalltip();
	if (project) { // si hay que ejecutar un proyecto
		compile_and_run_struct_single *compile_and_run=new compile_and_run_struct_single("OnRunRunOld");
		project->Run(compile_and_run);
		StartExecutionStuff(false,true,compile_and_run,LANG(GENERAL_RUNNING_DOTS,"Ejecutando..."));
	} else IF_THERE_IS_SOURCE { // si hay que ejecutar un ejercicio
		CompileOrRunSource(false,true,false);
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
	mxPreferenceWindow::ShowUp();
}

void mxMainWindow::OnRunCompile (wxCommandEvent &event) {
	_prevent_execute_yield_execute_problem;
	if (project) {
		compiler->BuildOrRunProject(false,false,false);
	} else IF_THERE_IS_SOURCE {
		CompileOrRunSource(true,false,false);
	}
}


void mxMainWindow::RunSource (mxSource *source) {
	
	if (source->config_running.always_ask_args) {
		int res = mxArgumentsDialog(this,source->exec_args,source->working_folder.GetFullPath()).ShowModal();
		if (res&AD_CANCEL) return;
		source->working_folder = mxArgumentsDialog::last_workdir;
		source->exec_args = (res&AD_EMPTY) ? "" : mxArgumentsDialog::last_arguments;
		if (res&AD_REMEMBER) source->config_running.always_ask_args=false;
	}
	
	// armar la linea de comando para ejecutar
	compiler->last_caption = source->page_text;
	compiler->last_runned = source;
	
	// agregar el prefijo para valgrind
	wxString exe_pref;
#if !defined(_WIN32) && !defined(__WIN32__)
	if (compiler->valgrind_cmd.Len())
		exe_pref = compiler->valgrind_cmd+" ";
#endif
	
	wxString command(config->Files.terminal_command);
	command.Replace("${TITLE}",LANG(GENERA_CONSOLE_CAPTION,"ZinjaI - Consola de Ejecucion"));
	if (command.Len()!=0) {
		if (command==" ") 
			command="";
		else if (command[command.Len()-1]!=' ') 
			command<<" ";
	}
	command<<"\""<<config->Files.runner_command<<"\" ";
	command<<"-lang \""<<config->Init.language_file<<"\" ";
	if (source->config_running.wait_for_key) command<<"-waitkey ";
	command<<"\""<<source->working_folder.GetFullPath()<<(source->working_folder.GetFullPath().Last()=='\\'?"\\\" ":"\" ");

	compiler->CheckForExecutablePermision(source->GetBinaryFileName().GetFullPath());
	
	command<<exe_pref<<"\""<<source->GetBinaryFileName().GetFullPath()<<"\"";
//	mxUT::ParameterReplace(command,"${ZINJAI_DIR}",wxGetCwd());
	// agregar los argumentos de ejecucion
	if (source->exec_args.Len()) command<<' '<<source->exec_args;	
	
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
		SetCompilingStatus("Detenido!");
	} else if (compiler->compile_and_run_single) {
		compile_and_run_struct_single *compile_and_run=compiler->compile_and_run_single;;
		compile_and_run->killed=true;
		wxProcess::Kill(compile_and_run->pid,wxSIGKILL,wxKILL_CHILDREN);
	}
	_menu_item(mxID_RUN_STOP)->Enable(false);
	_menu_item(mxID_RUN_COMPILE)->Enable(true);
	_menu_item(mxID_RUN_RUN)->Enable(true);
	_menu_item(mxID_RUN_CLEAN)->Enable(true);
//		menu.tools_makefile->Enable(true);
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
* @param i Indice de la pestaña a cerrar, o -1 para cerrar la actual
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
		for (int i=notebook_sources->GetPageCount()-1;i>=0;i--)
			if (i!=sel) CloseFromGui(i);
	}
}

void mxMainWindow::OnFileCloseProject (wxCommandEvent &event) {
	if (debug->IsDebugging()) debug->Stop();
//	if (project->modified) {
		if (config->Init.save_project) {
			project->Save();
		} else {
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
				((mxSource*)(notebook_sources->GetPage(i)))->UpdateExtras();
				notebook_sources->DeletePage(i);;
			}
		} else
			return;
	}
	main_window->project_tree.treeCtrl->DeleteChildren(main_window->project_tree.sources);
	main_window->project_tree.treeCtrl->DeleteChildren(main_window->project_tree.headers);
	main_window->project_tree.treeCtrl->DeleteChildren(main_window->project_tree.others);
	delete project;
	if (config->Init.autohiding_panels) {
		autohide_handlers[ATH_EXPLORER]->Hide();
		autohide_handlers[ATH_PROJECT]->Hide();
		autohide_handlers[ATH_SYMBOL]->Hide();
		autohide_handlers[ATH_COMPILER]->Hide();
	} else if (left_panels) {
		aui_manager.GetPane(left_panels).Hide();
		_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
		aui_manager.GetPane(compiler_panel).Hide();
	} else {
		_menu_item(mxID_VIEW_SYMBOLS_TREE)->Check(false);
		aui_manager.GetPane(symbols_tree.treeCtrl).Hide();
		_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
		aui_manager.GetPane(compiler_panel).Hide();
		_menu_item(mxID_VIEW_PROJECT_TREE)->Check(false);
		if (config->Init.show_explorer_tree) {
			_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(true);
			aui_manager.GetPane(explorer_tree.treeCtrl).Show();
		} else {
			_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(false);
			aui_manager.GetPane(explorer_tree.treeCtrl).Hide();
		}
		aui_manager.GetPane(project_tree.treeCtrl).Hide();
	}
	if (valgrind_panel) aui_manager.GetPane(valgrind_panel).Hide();
	if (welcome_panel) 
		ShowWelcome(true);
	else {
		NewFileFromTemplate(mxUT::WichOne(config->Files.default_template,"templates",true));
		aui_manager.Update();
	}
}

void mxMainWindow::OnFileExportHtml (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		wxFileDialog dlg (this, LANG(GENERAL_SAVE,"Guardar"),source->GetPath(true),source->GetFileName()+".html", "Documento HTML | *.html", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		dlg.SetDirectory(wxString(project?project->last_dir:config->Files.last_dir));
		if (dlg.ShowModal() == wxID_OK) {
			project?project->last_dir:config->Files.last_dir=dlg.GetPath();
			CodeExporter ce;
			wxString title = notebook_sources->GetPageText(notebook_sources->GetSelection());
			if (title.Last()=='*') 
				title.RemoveLast();
			if (!ce.ExportHtml(source,title,dlg.GetPath()))
				mxMessageDialog(this,LANG(MAINW_COULD_NOT_EXPORT_HTML,"No se pudo guardar el archivo"),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		}
	}
}

void mxMainWindow::OnFileReload (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE CURRENT_SOURCE->UserReload();
}

bool mxMainWindow::CloseSource (mxSource *src) {
	for(unsigned int i=0;i<notebook_sources->GetPageCount();i++) { 
		if (notebook_sources->GetPage(i)==src) return CloseSource(i);
	}
	return false;
}

bool mxMainWindow::CloseSource (int i) {
	mxSource *source=(mxSource*)notebook_sources->GetPage(i);
	if (share && share->Exists(source))  {
		int ans =mxMessageDialog(main_window,"El archivo esta siendo compartido con modificaciones. Si lo cierra dejara de estar disponible.\nRealmente desea cerrar el archivo?",source->page_text, mxMD_YES_NO,"Continuar compartiendo (\"sin modificaciones\") despues de cerrarlo.",false).ShowModal();
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
			parser->RemoveFile(source->GetFullPath());
		}
	} else {
		parser->ParseIfUpdated(source->source_filename);
		source->UpdateExtras();
	}
//	debug->OnSourceClosed(source); // supuestamente lo hace el destructor de mxSource llamando a debug->UnregisterSource
	notebook_sources->DeletePage(i);
	if (!project && welcome_panel && notebook_sources->GetPageCount()==0) ShowWelcome(true);
	return true;
}

void mxMainWindow::OnViewFullScreen(wxCommandEvent &event) {
	gui_fullscreen_mode=!gui_fullscreen_mode;
	if (!gui_fullscreen_mode) { // sale de la pantalla completa y vuelve a ser ventana
		_menu_item(mxID_VIEW_FULLSCREEN)->Check(false);
		if (config->Init.autohide_toolbars_fs && (!debug->debugging || !config->Debug.autohide_toolbars)) { // reacomodar las barras de herramientas (si no esta depurando, por que si esta depurando las reacomoda el depurador cuando termina)
#define _aux_fstb_1(NAME) \
			if (_toolbar_visible(tb##NAME)) { _menu_item(mxID_VIEW_TOOLBAR_##NAME)->Check(true); aui_manager.GetPane(_get_toolbar(tb##NAME)).Show(); } \
			else { _menu_item(mxID_VIEW_TOOLBAR_##NAME)->Check(false); aui_manager.GetPane(_get_toolbar(tb##NAME)).Hide(); }
			_aux_fstb_1(FILE);
			_aux_fstb_1(EDIT);
			_aux_fstb_1(VIEW);
			_aux_fstb_1(RUN);
			_aux_fstb_1(TOOLS);
			_aux_fstb_1(MISC);
			_aux_fstb_1(FIND);
			if (project) { _aux_fstb_1(PROJECT); }
			_aux_fstb_1(DEBUG);
		}
		
		if (config->Init.autohide_panels_fs && !config->Init.autohiding_panels) { // reacomodar los paneles
			if ( fullscreen_panels_status[0]!=aui_manager.GetPane(compiler_panel).IsShown() ) {
				if (fullscreen_panels_status[0]) {
					aui_manager.GetPane(compiler_panel).Show();
					_menu_item(mxID_VIEW_COMPILER_TREE)->Check(true);
				} else {
					aui_manager.GetPane(compiler_panel).Hide();
					_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
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
						_menu_item(mxID_VIEW_LEFT_PANELS)->Check(true);
					} else {
						aui_manager.GetPane(left_panels).Hide();
						_menu_item(mxID_VIEW_LEFT_PANELS)->Check(false);
					}
				}
			} else {
				if ( fullscreen_panels_status[4]!=aui_manager.GetPane(symbols_tree.treeCtrl).IsShown() ) {
					if (fullscreen_panels_status[4]) {
						aui_manager.GetPane(symbols_tree.treeCtrl).Show();
						_menu_item(mxID_VIEW_SYMBOLS_TREE)->Check(true);
					} else {
						aui_manager.GetPane(symbols_tree.treeCtrl).Hide();
						_menu_item(mxID_VIEW_SYMBOLS_TREE)->Check(false);
					}
				}
				if ( fullscreen_panels_status[5]!=aui_manager.GetPane(project_tree.treeCtrl).IsShown() ) {
					if (fullscreen_panels_status[5]) {
						aui_manager.GetPane(project_tree.treeCtrl).Show();
						_menu_item(mxID_VIEW_PROJECT_TREE)->Check(true);
					} else {
						aui_manager.GetPane(project_tree.treeCtrl).Hide();
						_menu_item(mxID_VIEW_PROJECT_TREE)->Check(false);
					}
				}
				if ( fullscreen_panels_status[6]!=aui_manager.GetPane(explorer_tree.treeCtrl).IsShown() ) {
					if (fullscreen_panels_status[6]) {
						aui_manager.GetPane(explorer_tree.treeCtrl).Show();
						_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(true);
					} else {
						aui_manager.GetPane(explorer_tree.treeCtrl).Hide();
						_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(false);
					}
				}
			}
			if ( beginner_panel && fullscreen_panels_status[7]!=aui_manager.GetPane(beginner_panel).IsShown() ) {
				if (fullscreen_panels_status[7]) {
					aui_manager.GetPane(beginner_panel).Show();
					_menu_item(mxID_VIEW_BEGINNER_PANEL)->Check(true);
				} else {
					aui_manager.GetPane(beginner_panel).Hide();
					_menu_item(mxID_VIEW_BEGINNER_PANEL)->Check(false);
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
		
	} else { // entra al modo pantalla completa
		_menu_item(mxID_VIEW_FULLSCREEN)->Check(true);
		if (valgrind_panel) aui_manager.GetPane(valgrind_panel).Hide();
		if (config->Init.autohide_toolbars_fs) { // reacomodar las barras de herramientas
			if (!debug->debugging || !config->Debug.autohide_toolbars) { // si esta depurando, las oculta el depurador cuando termina, sino...
#define _on_view_fullscreen_aux_1(ID) { \
	wxMenuItem *menu_item = _menu_item(mxID_VIEW_TOOLBAR_##ID); \
	if (menu_item->IsChecked()) { menu_item->Check(false); aui_manager.GetPane(_get_toolbar(tb##ID)).Hide(); } \
}
				_on_view_fullscreen_aux_1(FILE);
				_on_view_fullscreen_aux_1(VIEW);
				_on_view_fullscreen_aux_1(EDIT);
				_on_view_fullscreen_aux_1(RUN);
				_on_view_fullscreen_aux_1(DEBUG);
				_on_view_fullscreen_aux_1(TOOLS);
				_on_view_fullscreen_aux_1(MISC);
				_on_view_fullscreen_aux_1(FIND);
				if (project) _on_view_fullscreen_aux_1(PROJECT);
			}
		}
		if (config->Init.autohide_panels_fs && !config->Init.autohiding_panels) { // reacomodar los paneles
			if ( (fullscreen_panels_status[0]=aui_manager.GetPane(compiler_panel).IsShown()) ) {
				aui_manager.GetPane(compiler_panel).Hide();
				_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
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
					_menu_item(mxID_VIEW_LEFT_PANELS)->Check(false);
				}
			} else {
				if ( (fullscreen_panels_status[4]=aui_manager.GetPane(symbols_tree.treeCtrl).IsShown()) ) {
					aui_manager.GetPane(symbols_tree.treeCtrl).Hide();
					_menu_item(mxID_VIEW_SYMBOLS_TREE)->Check(false);
				}
				if ( (fullscreen_panels_status[5]=aui_manager.GetPane(project_tree.treeCtrl).IsShown()) ) {
					aui_manager.GetPane(project_tree.treeCtrl).Hide();
					_menu_item(mxID_VIEW_PROJECT_TREE)->Check(false);
				}
				if ( (fullscreen_panels_status[6]=aui_manager.GetPane(explorer_tree.treeCtrl).IsShown()) ) {
					aui_manager.GetPane(explorer_tree.treeCtrl).Hide();
					_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(false);
				}
			}
			if ( beginner_panel && (fullscreen_panels_status[7]=aui_manager.GetPane(beginner_panel).IsShown()) ) {
				aui_manager.GetPane(beginner_panel).Hide();
				_menu_item(mxID_VIEW_BEGINNER_PANEL)->Check(false);
			}
			if ( (fullscreen_panels_status[8]=aui_manager.GetPane(threadlist_ctrl).IsShown()) ) {
				debug->threadlist_visible=false;
				aui_manager.GetPane(threadlist_ctrl).Hide();
			}
		}
		
		ShowFullScreen(true,(config->Init.autohide_menus_fs?wxFULLSCREEN_NOMENUBAR:0)|wxFULLSCREEN_NOTOOLBAR|wxFULLSCREEN_NOSTATUSBAR|wxFULLSCREEN_NOBORDER|wxFULLSCREEN_NOCAPTION );
		new mxOSD(this,LANG(MAINW_FULLSCREEN_OUT_TIP,"Presione F11 para salir del modo pantalla completa"),3000,true);
		Raise();
DEBUG_INFO("wxYield:in  mxMainWindow::OnViewFullScreen");
		wxYield();
DEBUG_INFO("wxYield:out mxMainWindow::OnViewFullScreen");
//		IF_THERE_IS_SOURCE CURRENT_SOURCE->SetFocus();
	}
	
	aui_manager.Update();
	
	menu_data->SetAccelerators(); // por alguna razon, pasar a pantalla completa hace que se pierdan los accelerators
	
}

void mxMainWindow::OnViewHideBottom (wxCommandEvent &event) {
	if (aui_manager.GetPane(compiler_panel).IsShown()) {
		_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
		aui_manager.GetPane(compiler_panel).Hide();
	}
	if (aui_manager.GetPane(quick_help).IsShown()) {
		aui_manager.GetPane(quick_help).Hide();
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewCodeStyle (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src = CURRENT_SOURCE;
		_menu_item(mxID_VIEW_CODE_STYLE)->Check(!src->config_source.syntaxEnable);
		src->SetStyle(!src->config_source.syntaxEnable);
		src->SetColours(false); // por alguna razon el SetStyle de arriba cambia el fondo de los nros de linea
	}
}

void mxMainWindow::OnViewCodeColours (wxCommandEvent &event) {
	new mxColoursEditor(this);
}

void mxMainWindow::OnViewLineWrap (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source=CURRENT_SOURCE;
		int pos=source->GetCurrentPos();
		source->config_source.wrapMode=!source->config_source.wrapMode;
		_menu_item(mxID_VIEW_LINE_WRAP)->Check(source->config_source.wrapMode);
		source->SetWrapMode (source->config_source.wrapMode?wxSTC_WRAP_WORD: wxSTC_WRAP_NONE);
		source->GotoPos(pos);
		source->SetFocus();
	}
}

void mxMainWindow::OnViewWhiteSpace (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source=CURRENT_SOURCE;
		source->config_source.whiteSpace = !(source->config_source.whiteSpace);
		_menu_item(mxID_VIEW_WHITE_SPACE)->Check(source->config_source.whiteSpace);
		source->SetViewWhiteSpace(source->config_source.whiteSpace?wxSTC_WS_VISIBLEALWAYS:wxSTC_WS_INVISIBLE);
		source->SetViewEOL(source->config_source.whiteSpace);
		source->SetFocus();
//		wxYield();
	}
}

void mxMainWindow::OnViewLeftPanels (wxCommandEvent &event) {
	wxMenuItem *mi_view_left_panels = _menu_item(mxID_VIEW_LEFT_PANELS);
	if (!mi_view_left_panels->IsChecked()) {
		mi_view_left_panels->Check(false);
		aui_manager.GetPane(left_panels).Hide();
	} else {
		mi_view_left_panels->Check(true);
		aui_manager.GetPane(left_panels).Show();
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewProjectTree (wxCommandEvent &event) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(project_tree.treeCtrl).IsShown())
			autohide_handlers[ATH_PROJECT]->ForceShow(false);
	} else {	
		wxMenuItem *mi_view_project_tree = _menu_item(mxID_VIEW_PROJECT_TREE);
		if(left_panels) {
			wxMenuItem *mi_view_left_panels = _menu_item(mxID_VIEW_LEFT_PANELS);
			if (!mi_view_left_panels->IsChecked()) {
				mi_view_left_panels->Check(true);
				aui_manager.GetPane(left_panels).Show();
				aui_manager.Update();
			}
			left_panels->SetSelection(0);
			mi_view_project_tree->Check(false);
		} else {
			if (!mi_view_project_tree->IsChecked()) {
				mi_view_project_tree->Check(false);
				aui_manager.GetPane(project_tree.treeCtrl).Hide();
			} else {
				mi_view_project_tree->Check(true);
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
			autohide_handlers[ATH_SYMBOL]->ForceShow(true);
	} else {	
		wxMenuItem *mi_view_symbols_tree = _menu_item(mxID_VIEW_SYMBOLS_TREE);
		if(left_panels) {
			wxMenuItem *mi_view_left_panels = _menu_item(mxID_VIEW_LEFT_PANELS);
			if (!mi_view_left_panels->IsChecked()) {
				mi_view_left_panels->Check(true);
				aui_manager.GetPane(left_panels).Show();
				aui_manager.Update();
			}
			left_panels->SetSelection(1);
			mi_view_symbols_tree->Check(false);
		} else {
			if (!mi_view_symbols_tree->IsChecked()) {
				mi_view_symbols_tree->Check(false);
				aui_manager.GetPane(symbols_tree.treeCtrl).Hide();
			} else {
				mi_view_symbols_tree->Check(true);
				aui_manager.GetPane(symbols_tree.treeCtrl).Show();
			}
			aui_manager.Update();
		}
	}
}

void mxMainWindow::OnViewUpdateSymbols (wxCommandEvent &event) {
	wxWindow *focus = main_window->FindFocus();
	if (focus) focus = focus->GetParent();
	if (config->Init.autohiding_panels) {
//		if (!aui_manager.GetPane(symbols_tree.treeCtrl).IsShown())
//			autohide_handlers[ATH_SYMBOL]->ForceShow();
	} else {	
		if(left_panels) {
			wxMenuItem *mi_view_left_panels = _menu_item(mxID_VIEW_LEFT_PANELS);
			if (!mi_view_left_panels ->IsChecked()) {
				mi_view_left_panels ->Check(true);
				aui_manager.GetPane(left_panels).Show();
				aui_manager.Update();
			}
			left_panels->SetSelection(1);
		} else {
			wxMenuItem *mi_view_symbols_tree = _menu_item(mxID_VIEW_SYMBOLS_TREE);
			if (!mi_view_symbols_tree->IsChecked()) {
				mi_view_symbols_tree->Check(true);
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
	IF_THERE_IS_SOURCE CURRENT_SOURCE->HideCalltip();
	quick_help->SetPage(help->GetQuickHelp(keyword));
	ShowQuickHelpPanel(hide_compiler_tree);
}

void mxMainWindow::OnViewToolbarsConfig (wxCommandEvent &event) {
	mxPreferenceWindow::ShowUp()->SetToolbarPage();
}

void mxMainWindow::OnToggleToolbar (int menu_item_id, int toolbar_id, bool update_aui) {
	wxMenuItem *menu_item = _menu_item(menu_item_id);
	wxToolBar *toolbar = menu_data->GetToolbar(toolbar_id);
	bool &config_entry = menu_data->GetToolbarPosition(toolbar_id).visible;
	if (config_entry) {
		menu_item->Check(false);
		aui_manager.GetPane(toolbar).Hide();
		if (!(gui_debug_mode&&config->Debug.autohide_toolbars) && !(gui_fullscreen_mode&&config->Init.autohide_toolbars_fs)) config_entry=false;
	} else {
		menu_item->Check(true);
		aui_manager.GetPane(toolbar).Show();
		if (!(gui_debug_mode&&config->Debug.autohide_toolbars) && !(gui_fullscreen_mode&&config->Init.autohide_toolbars_fs)) config_entry=true;
	}
	SortToolbars(update_aui);
}

void mxMainWindow::OnViewToolbarView (wxCommandEvent &event) {
	OnToggleToolbar(mxID_VIEW_TOOLBAR_VIEW,MenusAndToolsConfig::tbVIEW);
}

void mxMainWindow::OnViewToolbarTools (wxCommandEvent &event) {
	OnToggleToolbar(mxID_VIEW_TOOLBAR_TOOLS,MenusAndToolsConfig::tbTOOLS);
}

void mxMainWindow::OnViewToolbarProject (wxCommandEvent &event) {
	OnToggleToolbar(mxID_VIEW_TOOLBAR_PROJECT,MenusAndToolsConfig::tbPROJECT);
}

void mxMainWindow::OnViewToolbarFile (wxCommandEvent &event) {
	OnToggleToolbar(mxID_VIEW_TOOLBAR_FILE,MenusAndToolsConfig::tbFILE);
}

void mxMainWindow::OnViewToolbarFind (wxCommandEvent &event) {
	OnToggleToolbar(mxID_VIEW_TOOLBAR_FIND,MenusAndToolsConfig::tbFIND);
	aui_manager.Update();
}

void mxMainWindow::OnViewToolbarDebug (wxCommandEvent &event) {
	OnToggleToolbar(mxID_VIEW_TOOLBAR_DEBUG,MenusAndToolsConfig::tbDEBUG);
}

void mxMainWindow::OnViewToolbarMisc (wxCommandEvent &event) {
	OnToggleToolbar(mxID_VIEW_TOOLBAR_MISC,MenusAndToolsConfig::tbMISC);
}

void mxMainWindow::OnViewToolbarEdit (wxCommandEvent &event) {
	OnToggleToolbar(mxID_VIEW_TOOLBAR_EDIT,MenusAndToolsConfig::tbEDIT);
}
void mxMainWindow::OnViewToolbarRun (wxCommandEvent &event) {
	OnToggleToolbar(mxID_VIEW_TOOLBAR_RUN,MenusAndToolsConfig::tbRUN);
}

void mxMainWindow::OnViewCompilerTree (wxCommandEvent &event) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(compiler_panel).IsShown())
			autohide_handlers[ATH_COMPILER]->ForceShow(false);
	} else {	
		if (!_menu_item(mxID_VIEW_COMPILER_TREE)->IsChecked()) {
			_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
			aui_manager.GetPane(compiler_panel).Hide();
		} else {
			_menu_item(mxID_VIEW_COMPILER_TREE)->Check(true);
			aui_manager.GetPane(compiler_panel).Show();
		}
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewExplorerTree (wxCommandEvent &event) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(explorer_tree.treeCtrl).IsShown()) {
			if (!project) SetExplorerPath(config->Files.last_dir);
		}
		autohide_handlers[ATH_EXPLORER]->ForceShow(true); // afuera del if para que reciba el foco siempre
		explorer_tree.treeCtrl->SetFocus(); // para que tenga el foco el control dentro del panel
	} else {	
		if(left_panels) {
			wxMenuItem *mi_view_left_panels = _menu_item(mxID_VIEW_LEFT_PANELS);
			if (!mi_view_left_panels->IsChecked()) {
				mi_view_left_panels->Check(true);
				aui_manager.GetPane(left_panels).Show();
				aui_manager.Update();
			}
			_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(false);
			left_panels->SetSelection(2);
			explorer_tree.treeCtrl->SetFocus();
		} else {
			if (!_menu_item(mxID_VIEW_EXPLORER_TREE)->IsChecked()) {
				_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(false);
				aui_manager.GetPane(explorer_tree.treeCtrl).Hide();
			} else {
				_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(true);
				if (!project) SetExplorerPath(config->Files.last_dir);
				aui_manager.GetPane(explorer_tree.treeCtrl).Show();
				explorer_tree.treeCtrl->SetFocus();
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
wxTreeItemId mxMainWindow::AddToProjectTreeProject(wxString name, eFileType where, bool sort) {
	wxString iname=config->Init.fullpath_on_project_tree?name:wxFileName(name).GetFullName();
	wxTreeItemId item;
	switch (where) {
	case FT_SOURCE:
		item = project_tree.treeCtrl->AppendItem(project_tree.sources, iname, 1);
		if (sort) project_tree.treeCtrl->SortChildren(main_window->project_tree.sources);
		return item;
	case FT_HEADER:
		item = project_tree.treeCtrl->AppendItem(project_tree.headers, iname, 2);
		if (sort) project_tree.treeCtrl->SortChildren(main_window->project_tree.headers);
		break;
	default:
		item = project_tree.treeCtrl->AppendItem(project_tree.others, iname, wxFileName(name).GetExt().MakeUpper()=="FBP"?5:3);
		if (sort) project_tree.treeCtrl->SortChildren(main_window->project_tree.others);
		break;
	};
	return item;
}

wxTreeItemId mxMainWindow::AddToProjectTreeSimple(wxFileName filename, eFileType where) {
	if (where==FT_NULL) where=mxUT::GetFileType(filename.GetFullName(),false);
	switch (where) {
		case FT_SOURCE:
			return project_tree.treeCtrl->AppendItem(project_tree.sources, filename.GetFullName(), 1);
			break;
		case FT_HEADER:
			return project_tree.treeCtrl->AppendItem(project_tree.headers, filename.GetFullName(), 2);
			break;
		default:
			if (filename.GetExt().MakeUpper()=="FBP")
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
*
* @param filename 			path completo del archivo a abrir
* @param add_to_project		indica si hay que agregar el archivo al arbol de archivos (deberia 
*							ser siempre asi cuando no hay proyecto) y al proyecto (en caso
*							de haber uno) 
**/
mxSource *mxMainWindow::OpenFile (const wxString &filename, bool add_to_project) {
	if (welcome_panel && notebook_sources->GetPageCount()==0) ShowWelcome(false);
	if (filename=="" || !wxFileName::FileExists(filename))
		return NULL;
	
	if (project && project->GetWxfbActivated() && filename.Len()>4 && filename.Mid(filename.Len()-4).CmpNoCase(".fbp")==0) {
		if (add_to_project) {
			project->AddFile(FT_OTHER,filename);
		} else {
			mxOSD osd(this,LANG(WXFB_OPENING,"Abriendo wxFormBuilder..."));
			wxExecute(wxString("\"")+config->Files.wxfb_command+"\" \""+filename+"\"");
DEBUG_INFO("wxYield:in  mxMainWindow::OpenFile");
			wxYield(); 
DEBUG_INFO("wxYield:out mxMainWindow::OpenFile");
			wxMilliSleep(1000);
		}
		return EXTERNAL_SOURCE;
	}
	
	int i;
	mxSource *source = FindSource(filename,&i);
	bool not_opened=true;
	if (source) {
		notebook_sources->SetSelection(i);
		source->SetFocus();
		not_opened=false;
	} else {
		project_file_item *fitem=project?project->FindFromName(filename):NULL;
		source = new mxSource(notebook_sources, AvoidDuplicatePageText(wxFileName(filename).GetFullName()),fitem);
		source->sin_titulo=false;
		source->LoadFile(filename);
		if (project) source->m_extras->ToSource(source);
	}
	wxString ext=wxFileName(filename).GetExt().MakeLower();
	if (mxUT::ExtensionIsCpp(ext)) {
		if (not_opened) notebook_sources->AddPage(source, source->page_text, true, *bitmaps->files.source);
		if (add_to_project) {
			if (project) {
				 if (!project->HasFile(filename)) {
					project_file_item *fi=project->AddFile(FT_SOURCE,filename);
					source->treeId=fi->item;
				 }
			} else {
				source->treeId = AddToProjectTreeSimple(filename,FT_SOURCE);
			}
			source->never_parsed=false;
			parser->ParseFile(filename);
		}
	} else if (ext=="" || mxUT::ExtensionIsH(ext)) {
		if (not_opened) notebook_sources->AddPage(source, source->page_text, true, *bitmaps->files.header);
		if (add_to_project) {
			if (project) {
				if (!project->HasFile(filename)) {
					project_file_item *fi=project->AddFile(FT_HEADER,filename);
					source->treeId = fi->item;
				}
			} else {
				source->treeId = AddToProjectTreeSimple(filename,FT_HEADER);
			}
			source->never_parsed=false;
			parser->ParseFile(filename);
		}
	} else {
		if (not_opened) notebook_sources->AddPage(source, source->page_text, true, *bitmaps->files.other);
		if (add_to_project) {
			if (project) {
				if (!project->HasFile(filename)) {
					project_file_item *fi=project->AddFile(FT_OTHER,filename);
					source->treeId=fi->item;
				}
			} else {
				source->treeId = AddToProjectTreeSimple(filename,FT_OTHER);
			}
		}
	}
	return source;
}

mxSource *mxMainWindow::OpenFile (const wxString &filename) {
	return OpenFile(filename,!project);
}

/**
* - Al abrir un proyecto multiple indica si cerrar o no los archivos que tengamos abiertos de antes (1024 es no, otra cosa es si)
* - Al abrir archivos sueltos puede contener respuestas a las preguntas de si agregar al proyecto, mover a la carpeta o esas cosas (para
*   cuando elegimo hacer lo mismo para todos en la primer pregunta), como flags por bits
**/
void mxMainWindow::OpenFileFromGui (wxFileName filename, int *multiple) {
//	cerr<<"*"<<filename.GetFullPath()<<"*"<<endl;
	if (!filename.FileExists()) {
		if (wxFileName::DirExists(DIR_PLUS_FILE(filename.GetFullPath(),"."))) {
			SetExplorerPath(filename.GetFullPath());
			ShowExplorerTreePanel();
		} else 
			mxMessageDialog(main_window,LANG(MAINW_FILE_NOT_EXISTS,"El archivo no existe."),filename.GetFullPath(),mxMD_OK|mxMD_INFO).ShowModal();
		return;
	}
	status_bar->SetStatusText(wxString("Abriendo ")<<filename.GetFullPath());
	if (!project) config->Files.last_dir=filename.GetPath();
	if (filename.GetExt().CmpNoCase(_T(PROJECT_EXT))==0) { // si es un proyecto
		// cerrar si habia un proyecto anterior
		if (project && filename!=DIR_PLUS_FILE(project->path,project->filename)) { // la segunda condicion es porque puedo estar creando uno nuevo encima del abierto, en ese caso, si guardo el abiero pierdo el que creo el asistente
			int ret=0;
			if (config->Init.save_project || (/*project->modified && */mxMD_YES&(ret=mxMessageDialog(main_window,LANG(MAINW_ASK_SAVE_PREVIOUS_PROJECT,"Desea guardar los cambios del proyecto anterior antes de cerrarlo?"),project->GetFileName(),mxMD_YES_NO|mxMD_QUESTION,LANG(MAINW_ALWAYS_SAVE_PROJECT_ON_CLOSE,"Guardar cambios siempre al cerrar un proyecto"),false).ShowModal()))) {
				if (!config->Init.save_project && ret&mxMD_CHECKED)
					config->Init.save_project=true;
				project->Save();
			}
		}
		// cerrar todos los archivos que no pertenezcan al proyecto
		if ((!multiple || (*multiple)!=1024)) {
				for (int i=notebook_sources->GetPageCount()-1;i>=0;i--) {
					mxSource *source = ((mxSource*)(notebook_sources->GetPage(i)));
					if (source ->GetModify()) {
						notebook_sources->SetSelection(i);
						int res=mxMessageDialog(main_window,LANG(MAINW_SAVE_CHANGES_QUESTION,"Hay cambios sin guardar. Desea guardarlos?"), source->page_text, mxMD_QUESTION|mxMD_YES_NO_CANCEL).ShowModal();
						if (mxMD_CANCEL==res) {
							status_bar->SetStatusText(LANG(GENERAL_READY,"Listo"));
							return;
						} else if (mxMD_YES==res) {
							source->SaveSource();
						}
					} 
					CloseSource(i);//notebook_sources->DeletePage(i);
				}
		}
		if (project) { // eliminar el proyecto viejo de la memoria
			delete project;
		}
		if (welcome_panel && notebook_sources->GetPageCount()==0) ShowWelcome(false);
		// abrir el proyecto
		project = new ProjectManager(filename);
		// mostrar el arbol de proyecto
		if (!config->Init.autohiding_panels) {
			if (left_panels) {
				wxMenuItem *mi_view_left_panels = _menu_item(mxID_VIEW_LEFT_PANELS);
				mi_view_left_panels->Check(true);
				aui_manager.GetPane(left_panels).Show();
				left_panels->SetSelection(config->Init.prefer_explorer_tree?2:0);
			} else {
				if (config->Init.prefer_explorer_tree) {
					_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(true);
					aui_manager.GetPane(explorer_tree.treeCtrl).Show();
					_menu_item(mxID_VIEW_PROJECT_TREE)->Check(false);
					aui_manager.GetPane(project_tree.treeCtrl).Hide();
				} else {
					_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(false);
					aui_manager.GetPane(explorer_tree.treeCtrl).Hide();
					_menu_item(mxID_VIEW_PROJECT_TREE)->Check(true);
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
		if (project && !project->HasFile(filename)) {
			// constantes para setear bits en *multiple
			const int always_attach=1; // ojo, esto se usa en OnProjectTreeAdd, asi que si se cambia el valor/significado, hay que revisar tambien ahi
			const int never_attach=2;
			const int always_move=4;
			const int never_move=8;
			const int always_replace=16;
			const int never_replace=32;
			// ver si hay que adjuntarlo al proyecto además de abrirlo
			bool attach=true;
			if (multiple && (*multiple)&(always_attach|never_attach)) {
				attach=(*multiple)&always_attach;
			} else {
				int ans1=mxMessageDialog(main_window,LANG(MAINW_ADD_TO_PROJECT_QUESTION,"¿Desea agregar el archivo al proyecto?"), filename.GetFullPath(), mxMD_QUESTION|mxMD_YES_NO,multiple?LANG(MAINW_ADD_TO_PROJECT_CHECK,"Hacer lo mismo para todos"):"",false).ShowModal();
				attach=ans1&mxMD_YES;
				if (multiple && ans1&mxMD_CHECKED) (*multiple)|=(attach?always_attach:never_attach);
			}
			if (attach) {
				// si no esta en la carpeta del proyecto, preguntar si hay que copiarlo ahí
				wxString aux_project_path=project->path; aux_project_path.Replace("\\","/",true); if (aux_project_path.EndsWith("/")) aux_project_path.RemoveLast();
				wxString aux_file_path=filename.GetFullPath(); aux_file_path.Replace("\\","/",true); if (aux_file_path.EndsWith("/")) aux_file_path.RemoveLast();
#ifdef __WIN32__
				aux_file_path.MakeLower();
				aux_project_path.MakeLower();
#endif
				if (!aux_file_path.StartsWith(aux_project_path)) {
					wxString dest_filename=DIR_PLUS_FILE(project->path,filename.GetFullName());
					bool move=false;
					if (multiple && (*multiple)&(always_move|never_move)) {
						move=(*multiple)&always_move;
					} else {
						int ans2=mxMessageDialog(main_window,LANG(MAINW_MOVE_TO_PROJECT_PATH_QUESTION,"El archivo que intenta agregar no se encuentra en el directorio del proyecto.\n¿Desea copiar el archivo al directorio del proyecto?"), filename.GetFullPath(), mxMD_QUESTION|mxMD_YES_NO,multiple?LANG(MAINW_ADD_TO_PROJECT_CHECK,"Hacer lo mismo para todos"):"",false).ShowModal();
						move=ans2&mxMD_YES;
						if (multiple && ans2&mxMD_CHECKED) (*multiple)|=(move?always_move:never_move);
					}
					if (move && wxFileExists(dest_filename)) {
						bool replace=false;
						if (multiple && (*multiple)|(always_replace|never_replace)) {
							replace=(*multiple)|always_replace;
						} else {
							int ans3=mxMessageDialog(main_window,LANG(MAINW_OVERWRITE_ON_PROJECT_PATH_QUESTION,"El archivo ya existe en el directorio de proyecto.\n¿Desea reemplazarlo?"), filename.GetFullPath(), mxMD_QUESTION|mxMD_YES_NO,multiple?LANG(MAINW_ADD_TO_PROJECT_CHECK,"Hacer lo mismo para todos"):"",true).ShowModal();
							replace=(!(ans3&mxMD_YES));
							if (multiple && ans3&mxMD_CHECKED) (*multiple)|=(replace?always_replace:never_replace);
						}
						if (!replace) move=false;
					}
					if (move) {
						wxCopyFile(filename.GetFullPath(),DIR_PLUS_FILE(project->path,filename.GetFullName()));
						filename=dest_filename;
					}
				}
			}
			OpenFile(filename.GetFullPath(),attach);
		} else {
			OpenFile(filename.GetFullPath());
		}
	}
	// actualizar el historial de archivos abiertos recientemente
	if (!project || filename.GetExt().CmpNoCase(PROJECT_EXT)==0)
		UpdateInHistory(filename.GetFullPath(),filename.GetExt().CmpNoCase(PROJECT_EXT)==0);
	status_bar->SetStatusText(LANG(GENERAL_READY,"Listo"));
}
	

void mxMainWindow::OnFileOpen (wxCommandEvent &event) {
	wxFileDialog dlg (this, "Abrir Archivo", project?project->last_dir:config->Files.last_dir, " ", "Any file (*)|*", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	dlg.SetWildcard("Archivos de C/C++ y Proyectos|"WILDCARD_CPP_EXT"|Fuentes|"WILDCARD_SOURCE"|Cabeceras|"WILDCARD_HEADER"|Proyectos|"WILDCARD_PROJECT"|Todos los archivos|*");
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
//				mxMessageDialog(this,"Ha ocurrido un error al intentar imprimir",LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();;
//		} else
//			(*printData) = printer.GetPrintDialogData().GetPrintData();
	}
}

void mxMainWindow::OnFileNewProject (wxCommandEvent &event) {
	if (!wizard) wizard = new mxNewWizard(this);
	wizard->RunWizard("new_project");
}

void mxMainWindow::OnFileNew (wxCommandEvent &event) {
	if (project) {
		if (!wizard) wizard = new mxNewWizard(this);
		wizard->RunWizard("on_project");
	} else 
		switch (config->Init.new_file){
			case 0:
				NewFileFromText("");
				break;
			case 1:
				main_window->NewFileFromTemplate(mxUT::WichOne(config->Files.default_template,"templates",true));
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
	if (!project) source->treeId = AddToProjectTreeSimple(name,FT_SOURCE);
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
		while (line.Left(7)=="// !Z! ") {
			if (line=="// !Z! Type: C") {
				source->cpp_or_just_c=false;
				source->temp_filename.SetExt("c");
			} else if (line.Left(13)=="// !Z! Caret:") {
				line.Mid(13).Trim(false).Trim(true).ToLong(&pos);
			} else if (line.Left(15)=="// !Z! Options:") {
				wxString comp_opts=line.Mid(15).Trim(false).Trim(true);
				// here we assume Type is before Options
				comp_opts.Replace("${DEFAULT}",config->GetDefaultCompilerOptions(source->IsCppOrJustC()),true);
				source->SetCompilerOptions(comp_opts); 
			}
			line = file.GetNextLine();
		}
		if (line!="")
			source->AppendText(line+"\n");
		while (!file.Eof()) 
			source->AppendText(file.GetNextLine()+"\n");
		source->MoveCursorTo(pos);
		file.Close();
	}
	source->SetLineNumbers();
	notebook_sources->AddPage(source, LAST_TITULO ,true, *bitmaps->files.blank);
	if (!project) source->treeId = AddToProjectTreeSimple(LAST_TITULO,FT_SOURCE);
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
			project_file_item *fi;
			if (!project || ((fi=project->FindFromName(src->source_filename.GetFullPath())) && fi->where!=FT_OTHER))
				parser->ParseSource(CURRENT_SOURCE,true);
		} 
	}
}

void mxMainWindow::OnFileSaveAs (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE  {
		mxSource *source=CURRENT_SOURCE;
		wxFileDialog dlg (this, LANG(GENERAL_SAVE,"Guardar"),source->sin_titulo?config->Files.last_dir:source->GetPath(true),source->GetFileName(), "Any file (*)|*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
//		dlg.SetDirectory(source->GetPath(true));
		dlg.SetWildcard("Todos los archivos|*|Archivos de C/C++|"WILDCARD_CPP"|Fuentes|"WILDCARD_SOURCE"|Cabeceras|"WILDCARD_HEADER);
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
						file.SetExt("cpp");
				}
			}
			if (source->SaveSource(file)) {
				parser->RenameFile(source->GetFullPath(),file.GetFullPath());
				wxString filename = file.GetFullName();
				eFileType ftype=mxUT::GetFileType(filename);
				source->SetPageText(filename);
				if (project)
					project->last_dir=dlg.GetDirectory();
				else
					config->Files.last_dir=dlg.GetDirectory();
				if (!project) {
					if (ftype==FT_SOURCE) {
						notebook_sources->SetPageBitmap(notebook_sources->GetSelection(),*bitmaps->files.source);
						source->SetStyle(wxSTC_LEX_CPP);
						if (project_tree.treeCtrl->GetItemParent(source->treeId)!=project_tree.sources) {
							project_tree.treeCtrl->Delete(source->treeId);
							source->treeId = project_tree.treeCtrl->AppendItem(project_tree.sources, filename, 1);
						} else
							project_tree.treeCtrl->SetItemText(source->treeId,filename);
						project_tree.treeCtrl->Expand(project_tree.sources);
					} else if (ftype==FT_HEADER) {
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
						if (ext=="HTM" || ext=="HTML")
							source->SetStyle(wxSTC_LEX_HTML);
						else if (ext=="XML")
							source->SetStyle(wxSTC_LEX_XML);
						else if (ext=="SH")
							source->SetStyle(wxSTC_LEX_BASH);
						else if (file.GetName().MakeUpper()=="MAKEFILE")
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
					UpdateInHistory(file.GetFullPath(),false);
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
	_record_this_action_in_macro(event.GetId());
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
			wxString header = code_helper->GetInclude(source->sin_titulo?wxString(""):source->source_filename.GetPathWithSep(),key);
			if (!header.Len()) {
				wxString bkey=key, type = source->FindTypeOf(e-1,s);
				if ( s!=SRC_PARSING_ERROR && type.Len() ) {
					header = code_helper->GetInclude(source->sin_titulo?wxString(""):source->source_filename.GetPathWithSep(),type);
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
						header=code_helper->GetInclude(source->sin_titulo?wxString(""):source->source_filename.GetPathWithSep(),type);
					}
				}
			}
			if (header.Len()) {
				if (mxUT::GetFileType(header)==FT_SOURCE)
					mxMessageDialog(main_window,key+LANG(MAINW_INSERT_HEADIR_CPP," esta declarada en un archivo fuente. Solo deben realizarse #includes para archivos de cabecera."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_INFO).ShowModal();
				else {
					header.Replace("\\","/");
					source->AddInclude(header);
				}
			} else if (key=="Clippo")
				new mxSplashScreen(clpeg,GetPosition().x+GetSize().x-215,GetPosition().y+GetSize().y-230);
			else
				mxMessageDialog(main_window,LANG1(MAINW_NO_HEADER_FOR,"No se encontro cabecera correspondiente a \"<{1}>\".",key),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
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
	long dpid=0;
	wxArrayString options;
	wxString otro="<<<Otro>>>",cual;
	if (!dpid) mxUT::GetRunningChilds(options);
	options.Add(cual=otro);
	if (options.GetCount()>1) {
		cual=wxGetSingleChoice("Process:",_menu_item_2(mnDEBUG,mxID_DEBUG_ATTACH)->GetPlainLabel(),options,this,-1,-1,true);
		if (!cual.Len()) return;
	}
	if (cual==otro) 
		mxGetTextFromUser("PID:",_menu_item_2(mnDEBUG,mxID_DEBUG_ATTACH)->GetPlainLabel(),"",this).ToLong(&dpid);
	else
		cual.BeforeFirst(' ').ToLong(&dpid);
	if (!dpid) return;
	wxString command = wxString("attach ")<<dpid;
	wxString message = wxString(LANG(DEBUG_STATUS_ATTACHING_TO,"Depurador adjuntado al proceso "))<<dpid;
	if (project) 
		debug->SpecialStart(NULL,command,message,false);
	else IF_THERE_IS_SOURCE
		debug->SpecialStart(CURRENT_SOURCE,command,message,false);
}

void mxMainWindow::OnDebugTarget ( wxCommandEvent &event ) {
	static wxString target;
	wxString new_target = DebugTargetCommon(target);
	if (new_target.Len()) target=new_target;
}

wxString mxMainWindow::DebugTargetCommon (wxString target) {
	target = mxGetTextFromUser("Target (arguments for gdb's target command):",_menu_item_2(mnDEBUG,mxID_DEBUG_ATTACH)->GetPlainLabel(),target,this);
	if (target.Len()) {
		wxString command = wxString("target ")<<target;
		wxString message = LANG(DEBUG_STATUS_TARGET_DONE,"Depuración iniciado correctamente.");
		if (project) 
			debug->SpecialStart(NULL,command,message,true);
		else IF_THERE_IS_SOURCE
			debug->SpecialStart(CURRENT_SOURCE,command,message,true);
	}
	return target;
}


/** 
* inicia la depuracion ejecutando el programa
* o reanuda la depuracion si ya estaba en proceso pero interrumpida
**/
void mxMainWindow::OnDebugRun ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE CURRENT_SOURCE->HideCalltip();
	if (debug->IsDebugging()) {
		if (debug->IsPaused())
			debug->Continue();
	} else {
		SetCompilingStatus("Preparando depuracion...");
DEBUG_INFO("wxYield:in mxMainWindow::OnDebugRun");
		wxYield();
DEBUG_INFO("wxYield:out mxMainWindow::OnDebugRun");
		if (project) {
			if (project->active_configuration->exec_method==EMETHOD_SCRIPT) { // if the script launches the executable, we can only attach the debugger to it
				OnDebugAttach(event);
				return;
			}
			debug->Start(config->Debug.compile_again);
		} else IF_THERE_IS_SOURCE {
			CompileOrRunSource(config->Debug.compile_again,true,true);
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
	if (!debug->IsDebugging())
		OnRunStop(event);
	else
		debug->Stop();
}

//void mxMainWindow::OnDebugUpdateInspections ( wxCommandEvent &event ) {
//	if (!debug->IsDebugging()) return;
//	if (debug->IsPaused()) debug->UpdateInspections();
//	class OnPauseUpdateInspections : public DebugManager::OnPauseAction {
//	public:
//		void Do() { debug->UpdateInspections(); }
//	};
//	debug->PauseFor(new OnPauseUpdateInspections());
//}

void mxMainWindow::OnDebugInspect ( wxCommandEvent &event ) {
	if (config->Init.autohiding_panels) {
//		if (!aui_manager.GetPane(inspection_ctrl).IsShown())
			autohide_handlers[ATH_INSPECTIONS]->ForceShow(true);
	} else {	
		if ( !aui_manager.GetPane((wxGrid*)inspection_ctrl).IsShown() )
			aui_manager.GetPane((wxGrid*)inspection_ctrl).Show();
		aui_manager.Update();
	}
	inspection_ctrl->SetFocus();
}

void mxMainWindow::OnDebugBacktrace ( wxCommandEvent &event ) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(backtrace_ctrl).IsShown())
			autohide_handlers[ATH_BACKTRACE]->ForceShow(false);
	} else {	
		aui_manager.GetPane(backtrace_ctrl).Show();
		aui_manager.Update();
	}
//	debug->backtrace_visible=true;
	debug->UpdateBacktrace(false);
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
			autohide_handlers[ATH_THREADS]->ForceShow(false);
		debug->threadlist_visible=true;
		debug->UpdateThreads();
	} else {
		aui_manager.GetPane(threadlist_ctrl).Show();
		aui_manager.Update();
		debug->threadlist_visible=true;
		debug->UpdateThreads();
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
	if (debug->CanTalkToGDB()) {
		wxString res;
		if (mxGetTextFromUser(res,LANG(DEBUG_RETURN_VALUE,"Valor de retorno:"), LANG(DEBUG_RETURN_FROM_FUNCTION,"Salir de la funcion") , "", this))
			debug->Return(res);
	}
}

void mxMainWindow::OnDebugJump ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		debug->Jump(source->GetFullPath(),source->GetCurrentLine());
	}
}

void mxMainWindow::OnDebugRunUntil ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		if (!debug->RunUntil(source->GetFullPath(),source->GetCurrentLine()))
			mxMessageDialog(main_window,LANG(DEBUG_RUN_UNTIL_ERROR,"La dirección actual no es válida."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
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

void mxMainWindow::OnDebugEnableDisableBreakpoint ( wxCommandEvent &event ) {
	if (!debug->CanTalkToGDB()) return;
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		BreakPointInfo *bpi = source->m_extras->FindBreakpointFromLine(source,source->GetCurrentLine());
		if (!bpi) return;
		if (debug->debugging && bpi->IsInGDB()) debug->SetBreakPointEnable(bpi->gdb_id,!bpi->enabled,bpi->action==BPA_STOP_ONCE); // debugger state
		bpi->SetEnabled(!bpi->enabled); // gui and bpi state
	}
}

void mxMainWindow::OnDebugBreakpointOptions ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE {
		mxSource *source=CURRENT_SOURCE;
		if (!debug->IsDebugging() || debug->CanTalkToGDB()) {
			
			// buscar si habia un breakpoint en esa linea
			int l = source->LineFromPosition (source->GetCurrentPos());
			BreakPointInfo *bpi = source->m_extras->FindBreakpointFromLine(source,l);
			
			if (!bpi) { // si no habia, lo crea
				bpi=new BreakPointInfo(source,l);
				if (debug->IsDebugging()) debug->SetBreakPoint(bpi);
			}
			new mxBreakOptions(bpi); // muestra el dialogo de opciones del bp
		}
	}
}

void mxMainWindow::OnDebugStepOut ( wxCommandEvent &event ) {
	debug->StepOut();
}


void mxMainWindow::OnDebugDoThat ( wxCommandEvent &event ) {
	static wxString what;
	wxString res = mxGetTextFromUser("Comando:", "Comandos internos" , what, this);
	if (res=="help") {
		wxMessageBox ("errorsave, kboom, debug on, debug off, wxlog on, wxlog off, gdb cmd, gdb ans");
	} else if (res=="debug on") {
		zinjai_debug_mode=true;
		SetStatusText("DoThat: Modo debug activado");
	} else if (res=="debug off") {
		zinjai_debug_mode=false;
		SetStatusText("DoThat: Modo debug desactivado");
	} else if (res=="gdb cmd") {
		wxMessageBox (debug->last_command);
	} else if (res=="gdb ans") {
		wxMessageBox (debug->last_answer);
	} else if (res=="errorsave") {
		er_sigsev(11);
	} else if (res=="wxlog on") {
		wxLog::SetActiveTarget(new wxLogGui());
		SetStatusText("DoThat: usando wxLogGui");
	} else if (res=="wxlog off") {
		wxLog::SetActiveTarget(new wxLogStderr());
		SetStatusText("DoThat: usando wxLogStrerr");
	} else if (res=="kboom") {
		int *p=NULL;
		// cppcheck-suppress nullPointer
		cout<<*p;
	} else if (res.Len() && debug->debugging) {
		SetStatusText(wxString("DoThat: comando para gdb: ")<<res);
		debug->DoThat(what=res);
	} else if (!debug->IsDebugging()) {
		SetStatusText(wxString("DoThat: gdb is not running")<<res);
	}
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
	
	gui_debug_mode=debug_mode;
	
	static bool backtrace_visible=true;
	static bool inspections_visible=true;
	static bool threads_visible=true;
	static bool log_visible=true;
	
	// habilitar y deshabilitar cosas en los menues
	menu_data->SetDebugMode(debug_mode); 
	
	wxCommandEvent evt;
	if (debug_mode) { // si comienza la depuracion...
#ifndef __WIN32__
		_menu_item(mxID_DEBUG_INVERSE_EXEC)->Check(false);
		_menu_item(mxID_DEBUG_ENABLE_INVERSE_EXEC)->Check(false);
		_get_toolbar(tbDEBUG)->EnableTool(mxID_DEBUG_INVERSE_EXEC,false);
		_get_toolbar(tbDEBUG)->ToggleTool(mxID_DEBUG_INVERSE_EXEC,false);
		_get_toolbar(tbDEBUG)->ToggleTool(mxID_DEBUG_ENABLE_INVERSE_EXEC,false);
#endif
		
		if (!config->Debug.allow_edition) { // no permitir editar los fuentes durante la depuracion
			for (unsigned int i=0;i<notebook_sources->GetPageCount();i++) 
				((mxSource*)(notebook_sources->GetPage(i)))->SetReadOnlyMode(ROM_ADD_DEBUG);
		}

		if (config->Debug.autohide_toolbars) { // reacomodar las barras de herramientas
			aui_manager.GetPane(_get_toolbar(tbSTATUS)).Show();
			
#define _aux_pfd_2(NAME) \
			{ wxMenuItem *mitem = _menu_item(mxID_VIEW_TOOLBAR_##NAME); \
			if (mitem->IsChecked()) { mitem->Check(false); aui_manager.GetPane(_get_toolbar(tb##NAME)).Hide(); } }
#define _aux_pfd_2_not(NAME) \
			{ wxMenuItem *mitem = _menu_item(mxID_VIEW_TOOLBAR_##NAME); \
			if (!mitem->IsChecked()) { mitem->Check(true); aui_manager.GetPane(_get_toolbar(tb##NAME)).Show(); } }
			
			_aux_pfd_2(FILE);
			_aux_pfd_2(EDIT);
			_aux_pfd_2(VIEW);
			_aux_pfd_2(RUN);
			_aux_pfd_2_not(DEBUG);
			_aux_pfd_2(TOOLS);
			_aux_pfd_2(MISC);
			_aux_pfd_2(FIND);
			if (project) _aux_pfd_2(PROJECT);
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
				if ( (debug_panels_status[0]=aui_manager.GetPane(compiler_panel).IsShown()) ) {
					aui_manager.GetPane(compiler_panel).Hide();
					_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
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
		
		// reestablecer la edicion de fuentes
		for (unsigned int i=0;i<notebook_sources->GetPageCount();i++)
			((mxSource*)(notebook_sources->GetPage(i)))->SetReadOnlyMode(ROM_DEL_DEBUG);
		
		if (config->Debug.autohide_toolbars) { // reacomodar las barras de herramientas
			aui_manager.GetPane(_get_toolbar(tbSTATUS)).Hide();
			if (gui_fullscreen_mode) {
				_menu_item(mxID_VIEW_TOOLBAR_DEBUG)->Check(false); aui_manager.GetPane(_get_toolbar(tbDEBUG)).Hide();
			} else {
			#define _aux_pfd_1(NAME) \
				if (_toolbar_visible(tb##NAME)) { _menu_item(mxID_VIEW_TOOLBAR_##NAME)->Check(true); aui_manager.GetPane(_get_toolbar(tb##NAME)).Show(); } \
				else { _menu_item(mxID_VIEW_TOOLBAR_##NAME)->Check(false); aui_manager.GetPane(_get_toolbar(tb##NAME)).Hide(); }
				_aux_pfd_1(DEBUG);
				_aux_pfd_1(FILE);
				_aux_pfd_1(EDIT);
				_aux_pfd_1(VIEW);
				_aux_pfd_1(RUN);
				_aux_pfd_1(TOOLS);
				_aux_pfd_1(MISC);
				_aux_pfd_1(FIND);
				if (project) { _aux_pfd_1(PROJECT); }
				_aux_pfd_1(DEBUG);
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
				
				if ( debug_panels_status[0]!=aui_manager.GetPane(compiler_panel).IsShown() ) {
					if (debug_panels_status[0]) {
						aui_manager.GetPane(compiler_panel).Show();
						_menu_item(mxID_VIEW_COMPILER_TREE)->Check(true);
					} else {
						aui_manager.GetPane(compiler_panel).Hide();
						_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
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
				where<<fname.GetFullPath()<<" - "<<LANG(FIND_LINE,"linea")<<" "<<src->GetCurrentLine()+1<<" col "<<
					src->GetCurrentPos()-src->PositionFromLine(src->GetCurrentLine())+1<<"\n";
			}
			where<<src->WhereAmI();
			src->ShowBaloon(where);
		}
	}
}

/**
* esta funcion es un parche para windows, que le da el foco al fuente 
* despues de que se hizo doble click en el error en el panel compiler_output,
* ya que utilizar el metodo SetFocus en el evento no funciona en Windows
* porque los eventos de click y foco sse procesan en orden invertido
* respecto a como lo hacen en linux
**/
void mxMainWindow::SetFocusToSource() {
//	main_window->Raise();
	IF_THERE_IS_SOURCE 
		CURRENT_SOURCE->SetFocus();
}

void mxMainWindow::OnGotoToolbarFind (wxCommandEvent &evt) {
	if (FindFocus()==menu_data->toolbar_find_text)
		FocusToSource();
	else
		menu_data->toolbar_find_text->SetFocus();
}

void mxMainWindow::OnToolbarFindEnter (wxCommandEvent &evt) {
	wxString stext = menu_data->toolbar_find_text->GetValue();
	if (stext.Len()==0)
		return;
	if (!project && stext=="Cuack Attack!") { // are you looking for a duck?
		menu_data->toolbar_find_text->SetValue("");
		NewFileFromText(_T(
"#include <iostream>\n#include <cstdlib>\n#include <ctime>\nusing namespace std;\n"
"char *cuack() {\nstatic char cuack[] = \"cuack\";\nfor (int i=0;i<4;i++)\nif (rand()%2)\ncuack[i]=cuack[i]|32;\nelse\ncuack[i]=cuack[i]&(~32);\nreturn cuack;\n}\n"
"int main(int argc, char *argv[]) {\nsrand(time(0));\nwhile (true) {\nclock_t t1=clock();\nwhile (clock()==t1);\ncout<<string(rand()%50,' ')<<cuack()<<\"!\"<<endl;\n}\nreturn 0;\n}"
			),"Cuack Attack!");
		wxYield();
		CURRENT_SOURCE->OnEditSelectAll(evt);
		CURRENT_SOURCE->OnIndentSelection(evt);
		OnRunRun(evt);
		return;
	}
	if (!project && stext=="moonwalk") { // are you looking for a duck?
		NewFileFromText("\n\n\n\n\n\n\n");
		parser_timer->Stop();
		wxYield();
		mxSource *src=CURRENT_SOURCE;
		src->SetModify(false);
		int delay=200;
		for (int i=0;i<20;i++) {
			wxString s("\n"), d(i,' ');
			s<<d<<" _A_\n"<<d<<"  O \n"<<d<<"  |\\\n"<<d<<"  |/\n";
			if (i%4==0)	s<<d<<" /|\n"<<d<<" \\|\n";
			if (i%4==1) s<<d<<" /|\n"<<d<<" ||\n";
			if (i%4==2) s<<d<<" /|\n"<<d<<"/ |\n";
			if (i%4==3) s<<d<<" /|\n"<<d<<" |\\\n";
			src->SetText(s); wxYield();	wxMilliSleep(delay);
		}
		wxString d(23,' ');
		src->SetText(wxString()<<"\n"<<d<<" _A_\n"<<d<<"  O |\n"<<d<<" /|/\n"<<d<<" || \n"<<d<<" / \\\n"<<d<<" | |\n");
		wxYield(); wxMilliSleep(delay*2);
		src->SetText(wxString()<<"\n"<<d<<"\\\n"<<d<<" \\O \n"<<d<<"  |\\\n"<<d<<"  |_A_\n"<<d<<" / \\\n"<<d<<" | |\n");
		wxYield(); wxMilliSleep(delay*3);
		src->SetModify(false);
		OnFileClose(evt);
		return;
	}
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		int pos = source->FindText(source->GetSelectionEnd(),source->GetLength(),menu_data->toolbar_find_text->GetValue(),0);
		if (pos==wxSTC_INVALID_POSITION) 
			pos = source->FindText(0,source->GetSelectionEnd()+stext.Len(),menu_data->toolbar_find_text->GetValue(),0);
		if (pos!=wxSTC_INVALID_POSITION) {
			source->EnsureVisibleEnforcePolicy(source->LineFromPosition(pos));
			source->SetSelection(pos,pos+stext.Len());
		}
	}
}

void mxMainWindow::OnToolbarFindChange (wxCommandEvent &evt) {
	wxString stext = menu_data->toolbar_find_text->GetValue();
	if (stext.Len()==0)
		return;
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		int pos = source->FindText(source->GetSelectionStart(),source->GetLength(),menu_data->toolbar_find_text->GetValue(),0);
		if (pos==wxSTC_INVALID_POSITION) 
			pos = source->FindText(0,source->GetSelectionStart()+stext.Len(),menu_data->toolbar_find_text->GetValue(),0);
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
	explorer_tree.root = explorer_tree.treeCtrl->AddRoot("Archivos Abiertos", 0);
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
			eFileType ctype=mxUT::GetFileType(as[i]);
			int t=4;
			if (ctype==FT_SOURCE)	t=1;
			else if (ctype==FT_HEADER) t=2;
			else if (ctype==FT_PROJECT) t=5;
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
		
		wxDirDialog dlg(this,"Seleccione la ubicacion:",explorer_tree.path);
		if (wxID_OK==dlg.ShowModal()) {
			SetExplorerPath(dlg.GetPath());
			config->Files.last_dir = dlg.GetPath();
		}
		
	} else {
		
		wxCommandEvent evt;
		OnExplorerTreeOpenOne(evt);
		
	}
	
}

void mxMainWindow::OnExplorerTreePopup(wxTreeEvent &event) {
	
	mxHidenPanel::ignore_autohide=true;
	
	explorer_tree.selected_item = event.GetItem();
	
	wxMenu menu("");
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
	wxDirDialog dlg(this,LANG(MAINW_EXPLORER_SELECT_PATH,"Seleccione la ubicacion:"),explorer_tree.path);
	if (wxID_OK==dlg.ShowModal()) {
		SetExplorerPath(dlg.GetPath());
		config->Files.last_dir = dlg.GetPath();
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
		SetFocusToSourceAfterEvents();
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
				eFileType ctype=mxUT::GetFileType(as[i]);
				int t=4;
				if (ctype==FT_SOURCE) t=1;
				else if (ctype==FT_HEADER) t=2;
				else if (ctype==FT_PROJECT) t=5;
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
		if (explorer_tree.treeCtrl->GetItemImage(item)!=0 && explorer_tree.treeCtrl->GetItemImage(item)!=5) // 0=folder, 5=zpr
			OpenFileFromGui(DIR_PLUS_FILE(path,explorer_tree.treeCtrl->GetItemText(item)));
		item = explorer_tree.treeCtrl->GetNextChild(explorer_tree.selected_item,cookie);
	}
}

void mxMainWindow::OnExplorerTreeOpenSources(wxCommandEvent &evt) {
	if (!explorer_tree.treeCtrl->GetChildrenCount(explorer_tree.selected_item)) {
		wxCommandEvent evt;
		OnExplorerTreeOpenOne(evt);
	}
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

void mxMainWindow::OnSymbolsGenerateAutocompletionIndex(wxCommandEvent &evt) {
	
	if (!parser->last_file->next) {
		mxMessageDialog(main_window,LANG(MAINW_GENERATE_AUTOCOMP_INDEX_EMPTY,"No hay fuentes para generar el índice. Abra uno \n"
																			 "o más archivos para que ZinjaI analice y extraiga \n"
																			 "los símbolos que conformarán el nuevo índice"),LANG(MAINW_GENERATE_AUTOCOMP_INDEX_CAPTION,"Generación de índice de autocompletado"),mxMD_OK|mxMD_ERROR).ShowModal();
	}
	
	wxString fname = wxGetTextFromUser(
		LANG(MAINW_GENERATE_AUTOCOMP_INDEX_NAME,"Nombre del nuevo índice"),
		LANG(MAINW_GENERATE_AUTOCOMP_INDEX_CAPTION,"Generación de índice de autocompletado"),
		"",this);
	if (!fname.Len()) return;
	fname=DIR_PLUS_FILE(DIR_PLUS_FILE(config->home_dir,"autocomp"),fname);
	if (wxFileName::FileExists(fname)) {
		if (mxMD_NO==mxMessageDialog(main_window,LANG(MAINW_GENERATE_AUTOCOMP_INDEX_OVERWRITE,"El indice ya existe, ¿desea reemplazarlo?"),LANG(MAINW_GENERATE_AUTOCOMP_INDEX_CAPTION,"Generación de índice de autocompletado"),mxMD_YES_NO).ShowModal()) 
			return;
	}
	
	// buscar un buen valor por defecto para "diretorio base", buscando la parte
	// inicial comun al path de todos los archivos que van a parar al indice
	wxString def_dir="<NULL>";
	pd_file *fil = parser->last_file->next;
	while (fil) {
		wxFileName fn(fil->name); fn.Normalize();
		wxString new_path = fn.GetPath();
		if (def_dir=="<NULL>") def_dir=new_path;
		else {
			unsigned int i=0;
			while (i<def_dir.Len() && i<new_path.Len() && def_dir[i]==new_path[i]) 
				i++; 
			def_dir=def_dir.Mid(0,i);
		}
		fil = fil->next;
	}
	
	wxDirDialog dlg2(this,LANG(MAINW_GENERATE_AUTOCOMP_INDEX_BASEDIR,"Directorio base (para formar las rutas relativas para los #includes):"),def_dir);
	if (wxID_OK!=dlg2.ShowModal()) return;
	if (code_helper->GenerateAutocompletionIndex(dlg2.GetPath(),fname)) {
		mxMessageDialog(main_window,LANG(MAINW_GENERATE_AUTOCOMP_INDEX_GENERATED,"Indice generado correctamente."),LANG(MAINW_GENERATE_AUTOCOMP_INDEX_CAPTION,"Generación de índice de autocompletado"),mxMD_OK|mxMD_INFO).ShowModal();
		mxPreferenceWindow::Delete();
	} else
		mxMessageDialog(main_window,LANG(MAINW_GENERATE_AUTOCOMP_INDEX_ERROR,"Ha ocurrido un error al intentar generar el archivo."),LANG(MAINW_GENERATE_AUTOCOMP_INDEX_CAPTION,"Generación de índice de autocompletado"),mxMD_OK|mxMD_ERROR).ShowModal();
}


void mxMainWindow::OnEditListMarks (wxCommandEvent &event) {
	if (project) {
		
		for (int i=notebook_sources->GetPageCount()-1;i>=0;i--)
			((mxSource*)(notebook_sources->GetPage(i)))->UpdateExtras();
		
		wxString res("<HTML><HEAD><TITLE>Lineas Resaltadas</TITLE></HEAD><BODY><B>Lineas Resaltadas:</B><BR><UL>");
		wxString restmp;
		
		GlobalListIterator<project_file_item*> fi(&project->files_all);
		while (fi.IsValid()) {
			const SingleList<int> &markers_list=fi->extras.GetHighlightedLines();
			restmp="";
			for(int i=0;i<markers_list.GetSize();i++)
				restmp=wxString("<LI><A href=\"gotoline:")<<DIR_PLUS_FILE(project->path,fi->name)<<":"<<markers_list[i]+1<<"\">"<<fi->name<<": linea "<<markers_list[i]+1<<"</A></LI>"<<restmp;
			res<<restmp;	
			fi.Next();
		}
		
		res<<"</UL><BR><BR></BODY></HTML>";
		main_window->ShowInQuickHelpPanel(res);	
	} else {
		wxString res("<HTML><HEAD><TITLE>Lineas Resaltadas</TITLE></HEAD><BODY><B>Lineas Resaltadas:</B><BR><UL>");
		for (int i=0,j=notebook_sources->GetPageCount();i<j;i++) {
			mxSource *src= (mxSource*)(notebook_sources->GetPage(i));
			for (int k=0;k<src->GetLineCount();k++) {
				wxString file_name = src->sin_titulo?src->page_text:src->source_filename.GetFullPath();
				wxString page_text = mxUT::ToHtml(src->page_text);
				if (src->MarkerGet(k)&1<<mxSTC_MARK_USER)
					res<<"<LI><A href=\"gotoline:"<<file_name<<":"<<k+1<<"\">"<<page_text<<": linea "<<k+1<<"</A></LI>";
			}
		}
		res<<"</UL><BR><BR></BODY></HTML>";
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
			text<<"("<<n<<")";
			i=0;
		} else
			return text;
	}
	return text;
}

void mxMainWindow::OnDebugPatch (wxCommandEvent &event) {
	if (debug->CanTalkToGDB()) debug->GetPatcher()->Patch();
}

void mxMainWindow::OnDebugCoreDump (wxCommandEvent &event) {
	if (notebook_sources->GetPageCount()>0||project) {
		if (!debug->IsDebugging() && (project || notebook_sources->GetPageCount())) {
			wxString dir = project?DIR_PLUS_FILE(project->path,project->active_configuration->working_folder):CURRENT_SOURCE->working_folder.GetFullPath();
			wxFileDialog dlg (this, _menu_item_2(mnDEBUG,mxID_DEBUG_LOAD_CORE_DUMP)->GetPlainLabel(), dir, " ", "Core dumps|core*|Todos los Archivos|*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (dlg.ShowModal() == wxID_OK)
				debug->LoadCoreDump(dlg.GetPath(),project?NULL:CURRENT_SOURCE);
		} else if (debug->CanTalkToGDB()) {
			wxString sPath = project?project->path:(CURRENT_SOURCE->GetPath(true));
			wxFileDialog dlg (this, _menu_item_2(mnDEBUG,mxID_DEBUG_SAVE_CORE_DUMP)->GetPlainLabel(),sPath,"core", "Any file (*)|*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			dlg.SetDirectory(sPath);
			dlg.SetWildcard("Core dumps|core*|Todos los Archivos|*");
			if (dlg.ShowModal() == wxID_OK)
				debug->SaveCoreDump(dlg.GetPath());
		}
	}
}

void mxMainWindow::SetOpenedFileName(wxString name) {
	SetTitle(wxString("ZinjaI - ")+name);
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
		aui_manager.GetPane(compiler_panel).Hide();
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
	if (project) project->WxfbAutoCheckStep1();
	event.Skip();
}

void mxMainWindow::OnKey(wxKeyEvent &evt) {
	evt.Skip();
}

void mxMainWindow::OnProjectTreeProperties (wxCommandEvent &event) {
	(new mxSourceProperties(project->GetNameFromItem(project_tree.selected_item)))->Show();
}

void mxMainWindow::OnProjectTreeOpenFolder (wxCommandEvent &event) {
	mxUT::OpenFolder(wxFileName(project->GetNameFromItem(project_tree.selected_item)).GetPath());
}

void mxMainWindow::OnFileOpenFolder(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		mxUT::OpenFolder(src->GetPath());
	}
}

void mxMainWindow::OnFileExploreFolder(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		SetExplorerPath(src->GetPath());
		ShowExplorerTreePanel();
	}
}

void mxMainWindow::OnFileProperties (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		(new mxSourceProperties(src->GetFullPath(),src))->Show();
	}
}

void mxMainWindow::OnViewNextError (wxCommandEvent &event) {
	if (current_toolchain.IsExtern()) { extern_compiler_output->OnErrorNext(); return; }
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
	if (current_toolchain.IsExtern()) { extern_compiler_output->OnErrorPrev(); return; }
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
			autohide_handlers[ATH_DEBUG_LOG]->ForceShow(false);
	} else {
		if (!aui_manager.GetPane(debug_log_panel).IsShown()) {
			aui_manager.GetPane(debug_log_panel).Show();
			aui_manager.Update();
		}	
	}
}

void mxMainWindow::OnDebugEnableInverseExecution (wxCommandEvent &event) {
	bool inv = debug->EnableInverseExec();
	_menu_item(mxID_DEBUG_INVERSE_EXEC)->Check(false);
	_menu_item(mxID_DEBUG_INVERSE_EXEC)->Enable(inv);
	_menu_item(mxID_DEBUG_ENABLE_INVERSE_EXEC)->Check(inv);
	wxToolBar *toolbar_debug = _get_toolbar(tbDEBUG);
	toolbar_debug->ToggleTool(mxID_DEBUG_ENABLE_INVERSE_EXEC,inv);
	toolbar_debug->ToggleTool(mxID_DEBUG_INVERSE_EXEC,false);
	toolbar_debug->EnableTool(mxID_DEBUG_INVERSE_EXEC,inv);
}

void mxMainWindow::OnDebugInverseExecution (wxCommandEvent &event) {
	bool inv=debug->ToggleInverseExec();
	_menu_item(mxID_DEBUG_INVERSE_EXEC)->Check(inv);
	_get_toolbar(tbDEBUG)->ToggleTool(mxID_DEBUG_INVERSE_EXEC,inv);
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
void mxMainWindow::ShowValgrindPanel(int what, wxString file, bool force) {
	if (valgrind_panel) {
		aui_manager.GetPane(valgrind_panel).Show();
		valgrind_panel->SetMode((mxVOmode)what,file);
	} else {
		aui_manager.AddPane(
			valgrind_panel = new mxValgrindOuput(this,(mxVOmode)what,file)
			, wxAuiPaneInfo().Name("valgrind_output").Bottom().Caption(LANG(CAPTION_TOOLS_RESULTS_PANEL,"Panel de resultados")).CloseButton(true).MaximizeButton(true).Row(8));
	}
	if (valgrind_panel->LoadOutput() || force) {
		aui_manager.GetPane(valgrind_panel).Show();
	} else {
		aui_manager.GetPane(valgrind_panel).Hide();
	}
	aui_manager.Update();
}

void mxMainWindow::OnViewBeginnerPanel (wxCommandEvent &event) {
	if (!beginner_panel) CreateBeginnersPanel();
	wxMenuItem *mitem = _menu_item(mxID_VIEW_BEGINNER_PANEL);
	if (config->Init.autohide_panels) {
		if (!mitem->IsChecked()) {
			autohide_handlers[ATH_BEGINNERS]->Hide();
			aui_manager.GetPane(autohide_handlers[ATH_BEGINNERS]).Hide();
			mitem->Check(false);
		} else {
			aui_manager.GetPane(autohide_handlers[ATH_BEGINNERS]).Show();
			autohide_handlers[ATH_BEGINNERS]->Show();
			mitem->Check(true);
		}
	} else {
		if (!mitem->IsChecked()) {
			mitem->Check(false);
			aui_manager.GetPane(beginner_panel).Hide();
		} else {
			mitem->Check(true);
			aui_manager.GetPane(beginner_panel).Show();
		}
	}
	aui_manager.Update();
}

void mxMainWindow::ShowDiffSideBar(bool bar, bool map) {
	if (map) {
		if (!diff_sidebar) {
			diff_sidebar=new mxDiffSideBar;
			aui_manager.AddPane(diff_sidebar, wxAuiPaneInfo().Name("diff_sidebar").Caption("diff").Right().Row(2).Show().MaxSize(20,-1));
		}
	}
	if (bar)
		aui_manager.GetPane(_get_toolbar(tbDIFF)).Show();
	if (bar||map) aui_manager.Update();
}

void mxMainWindow::ShowGCovSideBar() {
	if (gcov_sidebar) return;
	gcov_sidebar=new mxGCovSideBar(this);
	aui_manager.AddPane(gcov_sidebar, wxAuiPaneInfo().Name("gcov_sidebar").Caption("gcov").Left().Row(3).Show());
	aui_manager.Update();
}

mxSource *mxMainWindow::GetCurrentSource() {
	IF_THERE_IS_SOURCE
		return CURRENT_SOURCE;
	else 
		return NULL;
}

void mxMainWindow::OnEscapePressed(wxCommandEvent &event) {
	bool do_update=false;
	if (aui_manager.GetPane(compiler_panel).IsShown()) {
		if (config->Init.autohide_panels)
			autohide_handlers[ATH_COMPILER]->Hide();
		else {
			_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
			aui_manager.GetPane(compiler_panel).Hide();
		}
		do_update=true;
	}
	if (aui_manager.GetPane(quick_help).IsShown()) {
		if (config->Init.autohiding_panels)
			autohide_handlers[ATH_QUICKHELP]->Hide();
		else {
			_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
			aui_manager.GetPane(quick_help).Hide();
		}
		do_update=true;
	}
//#ifndef __WIN32__
	if (aui_manager.GetPane(valgrind_panel).IsShown()) {
		_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
		aui_manager.GetPane(valgrind_panel).Hide();
		do_update=true;
	}
//#endif
	if (do_update) aui_manager.Update();	
}

void mxMainWindow::ShowQuickHelpPanel(bool hide_compiler_tree) {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(quick_help).IsShown()) {
			autohide_handlers[ATH_QUICKHELP]->ForceShow(false);
		}
	} else {	
		if (!aui_manager.GetPane(quick_help).IsShown()) {
			// hide compiler results pane
			if (hide_compiler_tree) {
				_menu_item(mxID_VIEW_COMPILER_TREE)->Check(false);
				aui_manager.GetPane(compiler_panel).Hide();
			}
			// show quick help pane
			aui_manager.GetPane(quick_help).Show();
			aui_manager.Update();
		}
	}
}

void mxMainWindow::ShowCompilerTreePanel() {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(compiler_panel).IsShown()) {
			autohide_handlers[ATH_COMPILER]->ForceShow(false);
			SetFocusToSourceAfterEvents();
		}
	} else {	
		if (!aui_manager.GetPane(compiler_panel).IsShown()) {
			aui_manager.GetPane(quick_help).Hide();
			aui_manager.GetPane(compiler_panel).Show();
			_menu_item(mxID_VIEW_COMPILER_TREE)->Check(true);
			aui_manager.Update();
		}
	}
}

void mxMainWindow::ShowExplorerTreePanel() {
	if (config->Init.autohiding_panels) {
		if (!aui_manager.GetPane(explorer_tree.treeCtrl).IsShown()) {
			autohide_handlers[ATH_EXPLORER]->ForceShow(true);
			SetFocusToSourceAfterEvents();
		}
	} else {	
		if(left_panels) {
			wxMenuItem *mi_view_left_panels = _menu_item(mxID_VIEW_LEFT_PANELS);
			if (!mi_view_left_panels->IsChecked()) {
				mi_view_left_panels->Check(true);
				aui_manager.GetPane(left_panels).Show();
				aui_manager.Update();
			}
			_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(false);
			left_panels->SetSelection(2);
			explorer_tree.treeCtrl->SetFocus();
		} else if (!aui_manager.GetPane(explorer_tree.treeCtrl).IsShown()) {
			aui_manager.GetPane(explorer_tree.treeCtrl).Show();
			_menu_item(mxID_VIEW_EXPLORER_TREE)->Check(true);
			aui_manager.Update();
		}
	}
}

void mxMainWindow::CreateBeginnersPanel() {
	beginner_panel = new mxBeginnerPanel(this);
	if (config->Init.autohiding_panels) {
		autohide_handlers[ATH_BEGINNERS] = new mxHidenPanel(this,beginner_panel,HP_RIGHT,LANG(MAINW_BEGGINERS_PANEL,"Asistencias"));
		aui_manager.AddPane(autohide_handlers[ATH_BEGINNERS], wxAuiPaneInfo().CaptionVisible(false).Right().Position(0).Show());
	}
	aui_manager.AddPane(beginner_panel, wxAuiPaneInfo().Name("beginner_panel").Caption(LANG(MAINW_BEGGINERS_PANEL,"Panel de Asistencias")).Right().Hide());
	aui_manager.Update();
}

void mxMainWindow::ShowBeginnersPanel() {
	if (!beginner_panel) CreateBeginnersPanel();
	if (!aui_manager.GetPane(beginner_panel).IsShown()) {
		if (config->Init.autohiding_panels) {
			autohide_handlers[ATH_BEGINNERS]->ForceShow(false);
		} else {
			aui_manager.GetPane(beginner_panel).Show();
			_menu_item(mxID_VIEW_BEGINNER_PANEL)->Check(true);
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
		notebook_sources->AddPage(source, source->page_text, true, notebook_sources->GetPageBitmap(opage));
		
		main_window->notebook_sources->Split(notebook_sources->GetPageCount()-1,wxBOTTOM);
		main_window->notebook_sources->SetSelection(opage);
		main_window->notebook_sources->SetSelection(notebook_sources->GetPageCount()-1);
	}
}

void mxMainWindow::OnProjectTreeToggleFullPath(wxCommandEvent &event) {
	bool full = config->Init.fullpath_on_project_tree = !config->Init.fullpath_on_project_tree;
	
	GlobalListIterator<project_file_item*> it(&project->files_all);
	while(it.IsValid()) {
		project_tree.treeCtrl->SetItemText(it->item,full?it->name:wxFileName(it->name).GetFullName());
		it.Next();
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
	if (prog>100) prog=100;
	status_bar->SetProgress(prog);
}


void mxMainWindow::FocusToSource() {
	IF_THERE_IS_SOURCE CURRENT_SOURCE->SetFocus();
}

void mxMainWindow::PrepareGuiForProject (bool project_mode) {
	
	menu_data->UpdateToolbar(MenusAndToolsConfig::tbPROJECT,true);
	if (_toolbar_visible(tbPROJECT)) {
		if (project_mode) {
			aui_manager.GetPane(_get_toolbar(tbPROJECT)).Show();
			SortToolbars(false);
		} else
			aui_manager.GetPane(_get_toolbar(tbPROJECT)).Hide();
		aui_manager.Update();
	}
	
	// acomodar los menues 
	menu_data->SetProjectMode(project_mode); // habilitar/deshabilitar items exclusivos de proyecto
	// cambiar el nombre del Archivo->Abrir
	wxMenuItem *fo_item= _menu_item(mxID_FILE_OPEN); wxString fo_shortcut = fo_item->GetItemLabel(); 
	if (fo_shortcut.Contains("\t")) fo_shortcut=wxString("\t")+fo_shortcut.AfterLast('\t'); else fo_shortcut="";
	fo_item->SetItemLabel(wxString(project_mode?LANG(MENUITEM_FILE_OPEN_ON_PROJECT,"&Abrir/Agregar al proyecto..."):LANG(MENUITEM_FILE_OPEN,"&Abrir..."))+fo_shortcut);
	// resetear opciones de wxfb
	_menu_item(mxID_TOOLS_WXFB_REGEN)->Enable(false);
	_menu_item(mxID_TOOLS_WXFB_INHERIT_CLASS)->Enable(false);
	_menu_item(mxID_TOOLS_WXFB_UPDATE_INHERIT)->Enable(false);
	
	
	if (project_mode)
		SetTitle(wxString("ZinjaI - ")+project->project_name);
	else {
		SetTitle("ZinjaI");
		SetToolchainMode(false);
	}
	gui_project_mode=project_mode;
}

void mxMainWindow::SetToolchainMode (bool is_extern) {
	if (is_extern) {
		compiler_tree.treeCtrl->Hide();
		extern_compiler_output->Show();
	} else {
		compiler_tree.treeCtrl->Show();
		extern_compiler_output->Hide();
	}
	compiler_panel->GetSizer()->Layout();
}

void mxMainWindow::SetCompilingStatus (const wxString &message, bool also_statusbar) {
	if (current_toolchain.IsExtern()) extern_compiler_output->AddLine("= ",message);
	else {
		compiler_tree.treeCtrl->SetItemText(compiler_tree.state,message);
		if (also_statusbar) main_window->compiler_tree.treeCtrl->SelectItem(main_window->compiler_tree.state);
	}
	if (also_statusbar) main_window->SetStatusText(message);
}

void mxMainWindow::OnSelectErrorCommon (const wxString & error, bool set_focus_timer) {
	long line;
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
		bool opened=false;
		for (int i=0,j=notebook_sources->GetPageCount();i<j;i++) {
			mxSource *src = ((mxSource*)(notebook_sources->GetPage(i)));
			if ((!src->sin_titulo && SameFile(src->source_filename,the_one)) || (src->temp_filename==the_one && src==compiler->last_compiled) ) {
				notebook_sources->SetSelection(i);
				opened=true; break;
			}
		}
		// si no esta abierto
		if (!opened) {
			mxSource *src=OpenFile(sthe_one);
			if (src==EXTERNAL_SOURCE) return; // si era un proyecto wxfb o algo asi que se abre afuera de zinjai
			if (!src) { new mxGotoFileDialog(the_one.GetFullName(),this,line-1); return; }
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
#ifndef __WIN32__
				if (set_focus_timer)
#endif
					SetFocusToSourceAfterEvents();
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
		if (keyword!="" && keyword!=error) {
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
		SetFocusToSourceAfterEvents();
#endif
		ShowCompilerTreePanel();
		return;
	}
}

void mxMainWindow::OnToolbarMenu (wxCommandEvent & evt) {
	project->SetActiveConfiguration(project->configurations[evt.GetId()-mxID_LAST_ID]);
}

void mxMainWindow::CallAfterEvents (AfterEventsAction * action) {
	if (!after_events_timer) return; // main_window not initialized yet
	action->next=call_after_events;
	call_after_events=action;
	if (!current_after_events_action) 
		after_events_timer->Start(50,true);
}

void mxMainWindow::OnAfterEventsTimer (wxTimerEvent & event) {
	wxMouseState ms=wxGetMouseState();
	if (ms.LeftDown()||ms.MiddleDown()||ms.RightDown()) {
		after_events_timer->Start(50,true); return;
	}
	AfterEventsAction * &current = current_after_events_action;
	current = call_after_events; 
	call_after_events = NULL;
	while (current) {
		AfterEventsAction *next = current->next;
		if (current->do_do) current->Do(); delete current;
		current = next;
	}
	if (call_after_events) after_events_timer->Start(50,true);
}

void mxMainWindow::SetFocusToSourceAfterEvents () {
	class SetFocusToSourceAfterEventsAction : public mxMainWindow::AfterEventsAction {
		public: void Do() { main_window->SetFocusToSource(); }
	};
	CallAfterEvents(new SetFocusToSourceAfterEventsAction());
}

void mxMainWindow::OnParserContinueProcess(wxTimerEvent &event) {
	parser->OnParserProcessTimer();
}

void mxMainWindow::OnMacroRecord (wxCommandEvent & evt) {
	if (!m_macro||(*m_macro)[0].msg==0) {
		SetStatusText(LANG(MAINW_MACRO_START,"Generando macro, presione Ctrl+Shift+Q para finalizar."));
		if (!m_macro) m_macro=new SingleList<mxSource::MacroAction>();
		else m_macro->Clear();
		m_macro->Add(mxSource::MacroAction(1));
		IF_THERE_IS_SOURCE CURRENT_SOURCE->StartRecord();
	} else {
		SetStatusText(LANG(MAINW_MACRO_STOP,"Macro guardada, presione Ctrl+Q para reproducirla."));
		(*m_macro)[0].msg=0;
		IF_THERE_IS_SOURCE CURRENT_SOURCE->StopRecord();
	}
}

void mxMainWindow::OnMacroReplay (wxCommandEvent & evt) {
	if (!m_macro) {
		SetStatusText(LANG(MAINW_MACRO_STOP,"No hay macro definida, presione Ctrl+Shift+Q para generarla."));
		return;
	}
	if ((*m_macro)[0].msg==1) OnMacroRecord(evt);
	IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		src->BeginUndoAction();
		for(int i=1;i<m_macro->GetSize();i++) {
			mxSource::MacroAction &m=(*m_macro)[i].Get();
			if (m.for_sci) src->SendMsg(m.msg,m.wp,m.lp);
			else { evt.SetId(m.msg); ProcessEvent(evt); }
		}
		evt.SetId(mxID_MACRO_REPLAY); // just to be sure
		src->EndUndoAction();
	}
}

void mxMainWindow::OnNavigationHistoryNext (wxCommandEvent &evt) {
	navigation_history.Next();
}

void mxMainWindow::OnNavigationHistoryPrev (wxCommandEvent &evt) {
	navigation_history.Prev();
}

void mxMainWindow::OnSourceGotoDefinition (wxCommandEvent & event) {
	IF_THERE_IS_SOURCE CURRENT_SOURCE->JumpToCurrentSymbolDefinition();
}

void mxMainWindow::UpdateStylesInSources ( ) {
	for (unsigned int i=0;i<notebook_sources->GetPageCount();i++)
		((mxSource*)(notebook_sources->GetPage(i)))->SetColours();
}

void mxMainWindow::OnDebugSendSignal (wxCommandEvent & event) {
	static wxString prev;
	wxArrayString signames; signames.Add("<none>: continuar sin enviar ninguna señal)");
	vector<SignalHandlingInfo> vsig;
	debug->GetSignals(vsig);
	for(unsigned int i=0;i<vsig.size();i++) signames.Add(vsig[i].name+": "+vsig[i].description);
	wxSingleChoiceDialog dlg(this,"Signal:","Send signal to running process",signames);
	int pprev=signames.Index(prev);
	if (pprev!=wxNOT_FOUND) dlg.SetSelection(pprev);
	dlg.ShowModal();
	wxString ans=dlg.GetStringSelection();
	if (ans.Len()) {
		prev=ans;
		ans=ans.BeforeFirst(':');
		if (ans=="<none>") ans="0";
		debug->SendSignal(ans);
	}
}

void mxMainWindow::OnDebugSetSignals (wxCommandEvent & event) {
	mxSignalsSettings();
}

void mxMainWindow::OnDebugGdbCommand (wxCommandEvent & event) {
	mxGdbCommandsPanel *gdb_cmd = new mxGdbCommandsPanel();
	aui_manager.AddPane(gdb_cmd, wxAuiPaneInfo().Name("gdb_command").Float().CloseButton(true).MaximizeButton(true).Resizable(true).Caption("gdb").Show());
	aui_manager.Update();
	gdb_cmd->SetFocus();
}

/**
* @brief Enables/disables menu items according to current state (project,debug,current_source,etc)
*
* This is not the best idea... this will disable some items when a menu is shown, and if the state 
* changes but the user don't open the menu again, thoose items will stay disabled, and then their
* respective shortcuts won't work, but they should do so.
**/
void mxMainWindow::OnMenuOpen(wxMenuEvent & evt) {
//	menu_data->SetMenuItemsStates(evt.GetMenu());
}

void mxMainWindow::OnHelpShortcuts (wxCommandEvent & event) {
	mxHelpWindow::ShowHelp("atajos.html");
}

void mxMainWindow::OnChangeShortcuts (wxCommandEvent & event) {
	mxShortcutsDialog(this);
}

void mxMainWindow::OnHighlightKeyword (wxCommandEvent & event) {
	IF_THERE_IS_SOURCE CURRENT_SOURCE->OnHighLightWord(event);
}

void mxMainWindow::UnregisterSource (mxSource * src) {
	if (src==master_source) master_source=NULL;
	AfterEventsAction *current = call_after_events;
	for(int i=0;i<2;i++) {
		while (current) {
			if (current->source==src) current->do_do=false;
			current = current->next;
		}
		if (current_after_events_action) 
			current = current_after_events_action->next;
		else break;
	}
}

void mxMainWindow::OnFileSetAsMaster (wxCommandEvent & event) {
	IF_THERE_IS_SOURCE master_source=CURRENT_SOURCE;
}

void mxMainWindow::CompileOrRunSource (bool compile_if_needed, bool run, bool for_debug) {
	mxSource *source = CURRENT_SOURCE;
	if (master_source) {
		if (!source->sin_titulo && source->GetModify()) source->SaveSource(); // guardar el actual
		source = master_source;
	}
	if (source->sin_titulo) { // si no esta guardado, siempre compilar
		if (source->GetLine(0).StartsWith("make me a sandwich")) { wxMessageBox("No way!"); return; }
		else if (source->GetLine(0).StartsWith("sudo make me a sandwich")) source->SetText(wxString("/** Ok, you win! **/")+wxString(250,' ')+"#include <iostream>\n"+wxString(250,' ')+"int main(int argc, char *argv[]) {std::cout<<\"Here you are:\\n\\n   /-----------\\\\\\n  ~~~~~~~~~~~~~~~\\n   \\\\-----------/\\n\";return 0;}\n\n");
		source->SaveTemp();
		compiler->CompileSource(source,run,for_debug);
	} else { // si estaba guardado ver si cambio
		// si es un .h, avisar que probablemente sea un error intentar ejecutarlo
		wxString ext=source->sin_titulo?wxString(""):source->source_filename.GetExt().MakeLower();
		static bool ask=true;
		if (ask && !master_source && mxUT::ExtensionIsH(ext)) {
			int ans = mxMessageDialog(this,LANG(MAINW_RUN_HEADER_WARNING,""
				"Esta intentando compilar/ejecutar un archivo de cabecera.\n"
				"Probablemente deba intentar ejecutar un archivo fuente que\n"
				"incluya esta cabecera. ¿Desea continuar?\n\n"
				"Nota: puede configurar un fuente para que se ejecute siempre\n"
				"dicho fuente sin importar cual otro tenga el foco con click\n"
				"derecho sobre la pesataña del mismo."),
				LANG(GENERAL_WARNING,"Aviso"),mxMD_YES_NO|mxMD_WARNING,
				LANG(MAINW_RUN_HEADER_CHECK,"No volver a mostrar este mensaje"),false).ShowModal();
				if (ans&mxMD_NO) return;
			if (ans&mxMD_CHECKED) ask=false;
		}
		// si cambio el fuente, guardarlo 
		bool modified = source->GetModify();
		if (modified) source->SaveSource();
		// ver si hay que recompilar
		bool should_compile 
			= !compile_if_needed // si no habia que recompilar se marca igual como que si solo para evitar los tests que siguen
			|| !run // si no hay que ejecutar, es porque se invoco exclusivamente para compilar
			|| !source->GetBinaryFileName().FileExists() // si no hay binario, hay que recompilar
			|| source->GetBinaryFileName().GetModificationTime()<source->source_filename.GetModificationTime() // si el binario es mas viejo que fuente
			|| (config->Running.check_includes && mxUT::AreIncludesUpdated(source->GetBinaryFileName().GetModificationTime(),CURRENT_SOURCE->source_filename)); // si el binario es mas viejo que algun include
		// compilar, o depurar, o ejecutar, segun corresponda
		if (should_compile && compile_if_needed) compiler->CompileSource(source,run,for_debug);
		else if (for_debug) debug->Start(should_compile,source);
		else RunSource(source);
	}
}

