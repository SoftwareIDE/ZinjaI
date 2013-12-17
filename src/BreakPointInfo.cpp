#include "BreakPointInfo.h"
#include "ProjectManager.h"
#include "mxSource.h"

int BreakPointInfo::last_zinjai_id=0;
GlobalList<BreakPointInfo*> BreakPointInfo::global_list;

BreakPointInfo::BreakPointInfo(project_file_item *_fitem, int _line_number) {
	fname=DIR_PLUS_FILE(project->path,_fitem->name);
#if defined(_WIN32) || defined(__WIN32__)
	for (unsigned int i=0;i<fname.Len();i++) // corregir las barras en windows para que no sean caracter de escape
		if (fname[i]=='\\') fname[i]='/';
#endif
	line_number=_line_number;
	source=NULL;
	marker_handle=-1;
	enabled=true;
//	only_once=false;
	action=BPA_STOP_ALWAYS;
	ignore_count=0;
	gdb_status=BPS_UNKNOWN;
	gdb_id=-1;
	zinjai_id=++last_zinjai_id;
	_fitem->extras.AddBreakpoint(this);
}

BreakPointInfo::BreakPointInfo(mxSource *_source, int _line_number) {
	// para la lista global de breakpoints, siempre se inserta al principio
	fname=_source->GetFullPath();
#if defined(_WIN32) || defined(__WIN32__)
	for (unsigned int i=0;i<fname.Len();i++) // corregir las barras en windows para que no sean caracter de escape
		if (fname[i]=='\\') fname[i]='/';
#endif
	line_number=_line_number;
	gdb_id=-1;
	source=_source;
	enabled=true;
//	only_once=false;
	action=BPA_STOP_ALWAYS;
	ignore_count=0;
	gdb_status=BPS_UNKNOWN;
	marker_handle=-1; SetMarker();
	zinjai_id=++last_zinjai_id;
	_source->m_extras->AddBreakpoint(this);
}

BreakPointInfo::~BreakPointInfo() {
	if (source && marker_handle!=-1) source->MarkerDeleteHandle(marker_handle);
	global_list.FindAndRemove(this);
}

/// @brief Change its gdb_status, set its gdb_id, and update the source's marker if needed
void BreakPointInfo::SetStatus(BREAK_POINT_STATUS _gdb_status, int _gdb_id) {
	gdb_id=_gdb_id;
	gdb_status=_gdb_status;
	SetMarker();
}

/// @brief Updates line_number from the source's marker (it changes when user edits the source code)
void BreakPointInfo::UpdateLineNumber() {
	if (!source) return;
	fname=source->GetFullPath();
#if defined(_WIN32) || defined(__WIN32__)
	for (unsigned int i=0;i<fname.Len();i++) // corregir las barras en windows para que no sean caracter de escape
		if (fname[i]=='\\') fname[i]='/';
#endif
	line_number = source->MarkerLineFromHandle(marker_handle);
}

/// @brief Updates its source_pointer, and ads the marker if its previous one was NULL
void BreakPointInfo::SetSource(mxSource *_source) {
	source=_source; marker_handle=-1; 
	if (_source) SetMarker();
}

/// @brief returs true if this breakpoint has a valid gdb_id, inspecting its gdb_status
bool BreakPointInfo::IsInGDB() {
	return gdb_status>BPS_GDB_KNOWS_IT;
}

/// @brief return RedStatus
void BreakPointInfo::SetMarker() {
	if (!source) return;
	int new_type=((gdb_status==BPS_UNKNOWN&&enabled)||gdb_status==BPS_SETTED||gdb_status==BPS_ERROR_CONDITION) ? mxSTC_MARK_BREAKPOINT : mxSTC_MARK_BAD_BREAKPOINT;
	if (marker_handle!=-1) {
		if (marker_type==new_type) return;
		else source->MarkerDeleteHandle(marker_handle);
	}
	marker_handle=source->MarkerAdd(line_number, marker_type=new_type);
}

/**
* @brief find a BreakPointInfo that matches a gdb_id or zinjai_id
*
* If use_gdb finds a breakpoint that matches gdb_id, else 
* finds a breakpoint that matches zinjai_id. If there's no match
* returns NULL.
*
* This method searchs first in opened files (main_window->notebook_sources)
* starting by the current source, then searchs in project's files if there
* is a project.
**/
BreakPointInfo *BreakPointInfo::FindFromNumber(int _id, bool use_gdb_id) {
	GlobalListIterator<BreakPointInfo*> bpi=GetGlobalIterator();
	while (bpi.IsValid()) {
		if ( (use_gdb_id&&bpi->gdb_id==_id) || (!use_gdb_id&&bpi->zinjai_id==_id) )
			return bpi.GetData();
		bpi.Next();
	}
	return NULL;
}

GlobalListIterator<BreakPointInfo*> BreakPointInfo::GetGlobalIterator ( ) {
	return GlobalListIterator<BreakPointInfo*>(&global_list);
}

