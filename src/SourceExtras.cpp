#include "SourceExtras.h"
#include "BreakPointInfo.h"
#include "mxSource.h"

void SourceExtras::FromSource (mxSource * src) {
	// breakpoints
	for(int i=0;i<m_breakpoints.GetSize();i++) 
		m_breakpoints[i]->UpdateLineNumber();
	int l = src->GetLineCount();
	// highlighted lines
	m_highlighted_lines.Clear();
	for (int i=0;i<l;i++) {
		int m = src->MarkerGet(i);
		if (m&1<<mxSTC_MARK_USER)
			m_highlighted_lines.Add(i);
	}
	// misc
	m_current_col=src->GetCurrentPos();
	m_current_line=src->LineFromPosition(m_current_col);
	m_current_col-=src->PositionFromLine(m_current_line);
}

void SourceExtras::ToSource (mxSource * src, bool set_ownership) {
	// breakpoints
	for(int i=0;i<m_breakpoints.GetSize();i++) 
		m_breakpoints[i]->SetSource(src);
	// highlighted lines
	for(int i=0;i<m_highlighted_lines.GetSize();i++) 
		src->MarkerAdd(m_highlighted_lines[i], mxSTC_MARK_USER);
	// misc
	if (m_current_line==-1) src->GotoPos(m_current_col);
	else src->GotoPos(src->PositionFromLine(m_current_line)+m_current_col);
	if (set_ownership) {
		src->m_extras=this;
		src->m_owns_extras=false;
	}
}

void SourceExtras::AddHighlightedLine (int l) {
	m_highlighted_lines.Add(l);
}

const LocalList<BreakPointInfo*> & SourceExtras::GetBreakpoints ( ) {
	return m_breakpoints;
}

const SingleList<int> & SourceExtras::GetHighlightedLines ( ) {
	return m_highlighted_lines;
}

SourceExtras::SourceExtras ():m_breakpoints(&BreakPointInfo::global_list) {
	m_current_line=m_current_col=0;
}

SourceExtras::SourceExtras (mxSource *src, bool use_a_new_empty_extra_and_reset_source_markers):m_breakpoints(&BreakPointInfo::global_list) {
	m_current_line=m_current_col=0;
}

SourceExtras::SourceExtras (mxSource *src):m_breakpoints(&BreakPointInfo::global_list) {
	m_breakpoints.StealFrom(&(src->m_extras->m_breakpoints));
	m_highlighted_lines.StealFrom(&(src->m_extras->m_highlighted_lines));
	m_current_line=src->m_extras->m_current_line;
	m_current_col=src->m_extras->m_current_col;
	src->m_extras=this;
	src->m_owns_extras=true;
}

void SourceExtras::AddBreakpoint (BreakPointInfo * bpi) {
	m_breakpoints.Add(bpi);
}

SourceExtras::~SourceExtras ( ) {
	for(int i=0;i<m_breakpoints.GetSize();i++)
		delete m_breakpoints[i];
}

BreakPointInfo * SourceExtras::FindBreakpointFromLine (mxSource *src, int l) {
	for(int i=0;i<m_breakpoints.GetSize();i++)
		if (src->MarkerLineFromHandle(m_breakpoints[i]->marker_handle)==l) return m_breakpoints[i];
	return NULL;
}

void SourceExtras::ChangeSource (mxSource * src) {
	for(int i=0;i<m_breakpoints.GetSize();i++) 
		m_breakpoints[i]->SetSource(src);
}

void SourceExtras::SetCurrentPos (wxString pos) {
	if (pos.Contains(":")) {
		if (pos.BeforeFirst(':').ToLong(&m_current_line)) {
			if (!pos.AfterLast(':').ToLong(&m_current_col))
				m_current_col=0;
		} else {
			m_current_line=0;
		}
	} else {
		m_current_line=-1;
		if (!pos.ToLong(&m_current_col)) m_current_col=0;
	}
	
}

wxString SourceExtras::GetCurrentPos ( ) {
	return wxString()<<m_current_line<<":"<<m_current_col;
}

