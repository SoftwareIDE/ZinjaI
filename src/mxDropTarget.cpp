#include "mxDropTarget.h"

#include "mxUtils.h"
#include "mxMainWindow.h"
#include "mxSource.h"
#include <wx/msgdlg.h>

mxSource *mxDropTarget::current_drag_source=NULL;
bool mxDropTarget::last_drag_cancel=false;

mxDropTarget::mxDropTarget(mxSource *s) {
	src=s;
	data = new wxDataObjectComposite();
	data->Add(file_data=new wxFileDataObject());
	if (src) data->Add(text_data=new wxTextDataObject());
	if (!src) SetDataObject(file_data);
	else SetDataObject (data);
}

mxDropTarget::~mxDropTarget() {
	
}

bool mxDropTarget::OnDrop(wxCoord x, wxCoord y) {
	return true;
}

wxDragResult mxDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def) {
	if (!src) return def;
	return src->DoDragOver(x,y,def);
}

wxDragResult mxDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def) {
	GetData();
	if (!src || data->GetReceivedFormat()==wxDF_FILENAME) {
		wxArrayString files=file_data->GetFilenames(); 
		int ans=0;
		if (files.GetCount()==1)
			main_window->OpenFileFromGui(files[0]);
		else
			for (unsigned int i=0;i<files.GetCount();i++)
				main_window->OpenFileFromGui(files[i],&ans);
	} else if (data->GetReceivedFormat()==wxDF_TEXT && src) {
		wxPoint wxcrd(x,y);
		int p=src->PositionFromPoint(wxcrd);
		int ss=src->GetSelectionStart();
		int se=src->GetSelectionEnd();
		if (p==wxSTC_INVALID_POSITION || (p>=ss && p<=se) || (p>=se && p<=ss)) {
			current_drag_source=NULL;
			last_drag_cancel=true;
			return wxDragCancel;
		}
		wxString str=text_data->GetText();
		if (src==current_drag_source && def==wxDragMove) {
			src->BeginUndoAction();
			def=wxDragCopy;
			if (ss>p||se>p) {
				src->SetTargetStart(ss); src->SetTargetEnd(se);
				src->ReplaceTarget("");
				src->DoDropText(x,y,str);
//				src->SetTargetStart(p); src->SetTargetEnd(p);
//				src->ReplaceTarget(str);
//				src->SetSelection(p,p+str.Len());
			} else {
				src->DoDropText(x,y,str);
//				src->SetTargetStart(p); src->SetTargetEnd(p);
//				src->ReplaceTarget(str);
//				src->SetSelection(p,p+str.Len());
				src->SetTargetStart(ss); src->SetTargetEnd(se);
				src->ReplaceTarget("");
			}
			src->EndUndoAction();
			current_drag_source=NULL;
		} else {
			src->SetTargetStart(p); src->SetTargetEnd(p);
			src->ReplaceTarget(str);
			src->SetSelection(p,p+str.Len());
		}
	} else {
		if (src) src->DoDragOver(x,y,wxDragCancel);
		return wxDragCancel;
	}
	return def;
}

void mxDropTarget::OnLeave() {
}

wxDragResult mxDropTarget::OnEnter(wxCoord x, wxCoord y, wxDragResult def) {
	return OnDragOver(x,y,def);
}
