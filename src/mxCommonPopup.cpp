#include "mxCommonPopup.h"
#include "mxUtils.h"
#include "mxLongTextEditor.h"
#include "mxEnumerationEditor.h"
#include "mxThreeDotsUtils.h"

void mxCommonPopup::ProcessCommandEvent(wxCommandEvent &evt) {
	if (!m_text_ctrl) return;
	switch (evt.GetId()) {
	case mxID_POPUPS_EDIT_AS_TEXT:
		mxLongTextEditor(m_parent,m_caption,m_text_ctrl);
		return;
	case mxID_POPUPS_EDIT_AS_LIST:
		mxEnumerationEditor(m_parent,m_caption,m_text_ctrl,m_comma_split);
		return;
	case mxID_POPUPS_INSERT_FILE:
		mxThreeDotsUtils::ReplaceSelectionWithFile(m_parent,m_text_ctrl,m_base_path); 
		return;
	case mxID_POPUPS_INSERT_DIR:
		mxThreeDotsUtils::ReplaceSelectionWithDirectory(m_parent,m_text_ctrl,m_base_path); 
		return;
	case mxID_POPUPS_INSERT_MINGW_DIR:
		ReplaceSelectionWith("${MINGW_DIR}");
		return;
	case mxID_POPUPS_INSERT_TEMP_DIR:
		ReplaceSelectionWith("${TEMP_DIR}");
		return;
	case mxID_POPUPS_INSERT_BROWSER:
		ReplaceSelectionWith("${BROWSER}");
		return;
	case mxID_POPUPS_INSERT_SHELL_EXECUTE:
		ReplaceSelectionWith("${OPEN}");
		return;
	case mxID_POPUPS_INSERT_PROJECT_PATH:
		ReplaceSelectionWith("${PROJECT_PATH}");
		return;
	case mxID_POPUPS_INSERT_WORKDIR:
		ReplaceSelectionWith("${BIN_WORKDIR}");
		return;
	case mxID_POPUPS_INSERT_PROJECT_BIN:
		ReplaceSelectionWith("${PROJECT_BIN}");
		return;
	case mxID_POPUPS_INSERT_ARGS:
		ReplaceSelectionWith("${ARGS}");
		return;
	case mxID_POPUPS_INSERT_ZINJAI_DIR:
		ReplaceSelectionWith("${ZINJAI_DIR}");
		return;
	case mxID_POPUPS_INSERT_CURRENT_FILE:
		ReplaceSelectionWith("${CURRENT_SOURCE}");
		return;
	case mxID_POPUPS_INSERT_CURRENT_DIR:
		ReplaceSelectionWith("${CURRENT_DIR}");
		return;
	case mxID_POPUPS_INSERT_OUTPUT:
		ReplaceSelectionWith("${OUTPUT}");
		return;
	case mxID_POPUPS_INSERT_DEPS:
		ReplaceSelectionWith("${DEPS}");
		return;
	}
}

