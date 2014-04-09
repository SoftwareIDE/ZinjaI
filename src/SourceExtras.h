#ifndef SOURCEEXTRAS_H
#define SOURCEEXTRAS_H
#include "SingleList.h"
#include <wx/string.h>


class mxSource;
class BreakPointInfo;

class SourceExtras {
private:
	long m_current_col; ///< current col for text cursor (if current_line!=-1, wich is the normal case, absolute position othrewise)
	long m_current_line; ///< current line for text cursor (if m_current_line==-1, then m_current_col holds the absolute position, only posible when reading and old project config file)
	LocalList<BreakPointInfo*> m_breakpoints;
	SingleList<int> m_highlighted_lines;
public:
	SourceExtras();
	SourceExtras(mxSource *src); ///< used to transfer data from a project_file_item's SourceExtras to a mxSource's one (when dettaching an opened file from the project)
	SourceExtras(mxSource *src, bool use_a_new_empty_extra_and_reset_source_markers); ///< used to create a new SourceExtras for a file that remains open after closing a project (so the file will loose the project owned data, and now needs to own a new one)
	~SourceExtras();
	void AddHighlightedLine(int l);
	void AddBreakpoint(BreakPointInfo *bpi);
	void FromSource(mxSource *src);
	void ToSource(mxSource *src, bool set_ownership=false);
	void ChangeSource(mxSource *src); ///< when a source that has breakpoints attached is closed, breakpoints's pointers to the source must be updated to NULL or other source (if there was more than one view)
	void SetCurrentPos(wxString pos);
	wxString GetCurrentPos();
	const LocalList<BreakPointInfo*> &GetBreakpoints();
	const SingleList<int> &GetHighlightedLines();
	BreakPointInfo *FindBreakpointFromLine(mxSource *src, int l);
};

#endif

