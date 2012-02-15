#ifndef MXSIZERS_H
#define MXSIZERS_H
#include <wx/sizer.h>

struct mxSizers {
	wxSizerFlags 
		Right, Center,
		Exp1, Exp0, Exp0_Right,
		BR10, BA10, BA5, BA2, BT5, BB5,
		BA5_Center, BA5_Right, BA5_Left,
		BA10_Right, BLR10,
		BL10_Exp0, BL10_Exp1, BT10, BT10_Exp0,
		BA5_Exp1, BA5_Exp0, BA10_Exp0, BA10_Exp1,
		BLRT5_Exp0,	BLRB5_Exp0,	BLRB5_Exp1,
		BLT5_Exp0, BL5_Exp1, BB5_Exp0,
		BLRT5, BA5_DL_Exp0, BA5_DL_Exp1, BT5_Exp0,
		BA10_Exp0_Right, BTR10_Right,
		BA2_Exp1,BLB5_Exp0
		;
	mxSizers();
};

extern mxSizers *sizers;

#endif

