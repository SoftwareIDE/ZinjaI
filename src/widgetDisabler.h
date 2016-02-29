#ifndef WIDGETDISABLER_H
#define WIDGETDISABLER_H
class wxControl;

/**
* This class stores a list of control that can be enabled/disabled
* easily all at once, for using in configuration dialogs when lot
* of items depends on some other item.
*
* used in mxProjectConfigWindow
**/
class widgetDisabler {
private:
	struct node {
		wxControl *control;
		node *next;
	} *m_first;
public:
	widgetDisabler();
	void Add(wxControl *_control);
	void operator+=(wxControl *_control);
	void EnableAll(bool _enable=true);
	void DisableAll();
	~widgetDisabler();
};

#endif

