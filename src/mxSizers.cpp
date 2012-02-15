#include "mxSizers.h"

mxSizers *sizers;

mxSizers::mxSizers() {
	
	Exp1.Proportion(1).Expand();
	Exp0.Proportion(0).Expand();
	Exp0_Right.Right().Proportion(0).Expand();
	BA10.Border(wxALL,10);
	BR10.Border(wxRIGHT,10);
	BA5.Border(wxALL,5);
	BA5_Center.Border(wxALL,5).Center();
	BA5_Right.Border(wxALL,5).Right();
	BA5_Left.Border(wxALL,5).Left();
	BA10_Right.Border(wxALL,10).Right();
	BA5_Exp0.Proportion(0).Expand().Border(wxALL,5);
	BA5_Exp1.Proportion(1).Expand().Border(wxALL,5);
	BLRT5_Exp0.Border(wxLEFT|wxRIGHT|wxTOP, 5).Proportion(0).Expand();
	BLRB5_Exp0.Border(wxLEFT|wxRIGHT|wxBOTTOM, 5).Proportion(0).Expand();
	BLRB5_Exp1.Border(wxLEFT|wxRIGHT|wxBOTTOM, 5).Proportion(1).Expand();
	BA2.Border(wxALL,2);
	BA10_Exp0.Proportion(0).Expand().Border(wxALL,10);
	BA10_Exp1.Proportion(1).Expand().Border(wxALL,10);
	Right.Right();
	Center.Center();
	
	
	BLR10.Border(wxLEFT|wxRIGHT,10);
	
	BT10.Border(wxTOP,10);
	BL10_Exp0.Proportion(0).Expand().Border(wxLEFT,10);
	BL10_Exp1.Proportion(1).Expand().Border(wxLEFT,10);
	BT10_Exp0.Proportion(0).Expand().Border(wxTOP,10);
	BT5.Border(wxTOP,5);
	BB5.Border(wxBOTTOM,5);
	
	BLT5_Exp0.Proportion(0).Expand().Border(wxLEFT|wxTOP,5);
	BLRT5.Border(wxLEFT|wxRIGHT|wxTOP,5);
	BL5_Exp1.Proportion(1).Expand().Border(wxLEFT,5);
	BB5_Exp0.Proportion(0).Expand().Border(wxBOTTOM,5);
		
	BA5_DL_Exp0.Proportion(0).Expand().Border(wxALL,5).DoubleBorder(wxLEFT);
	BA5_DL_Exp1.Proportion(1).Expand().Border(wxALL,5).DoubleBorder(wxLEFT);
	BA10_Exp0_Right.Proportion(0).Expand().Right();
	BTR10_Right.Right().Border(wxTOP|wxRIGHT,10);
	BT5_Exp0.Proportion(0).Expand().Border(wxTOP,5);
	BA2_Exp1.Border(wxALL,2).Proportion(1).Expand();
	BLB5_Exp0.Proportion(0).Expand().Border(wxLEFT|wxBOTTOM,5);
	
}
