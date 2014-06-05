#include "mxGenericInspectionCtrl.h"

SingleList<mxGenericInspectionCtrl*> mxGenericInspectionCtrl::list;

mxGenericInspectionCtrl::mxGenericInspectionCtrl() {
	list.Add(this);
}

mxGenericInspectionCtrl::~mxGenericInspectionCtrl() {
	list.FastRemove(list.Find(this));
}

void mxGenericInspectionCtrl::UpdateAll ( ) {
	for(int i=0;i<list.GetSize();i++) { 
		list[i]->OnInspectionsUpdated();
	}
}

