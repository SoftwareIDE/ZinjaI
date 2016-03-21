#ifndef CUSTOMTOOLS_H
#define CUSTOMTOOLS_H

#define MAX_PROJECT_CUSTOM_TOOLS 5
#define MAX_CUSTOM_TOOLS 10
#include <wx/textfile.h>
#include <wx/process.h>

enum { CT_PRE_NONE=0, CT_PRE_SAVE_ONE, CT_PRE_SAVE_PROJECT, CT_PRE_SAVE_ALL, CT_PRE_COMPILE };
enum { CT_POST_NONE=0, CT_POST_RELOAD_ONE, CT_POST_RELOAD_ALL, CT_POST_RUN, CT_POST_DEBUG };
enum { CT_OUTPUT_HIDDEN=0, CT_OUTPUT_TERMINAL, CT_OUTPUT_TERMINAL_WAIT, CT_OUTPUT_DIALOG };

struct OneCustomTool {
	wxString name;
	wxString command;
	wxString workdir;
	int pre_action;
	int post_action;
	int output_mode;
	bool async_exec;
	bool on_toolbar;
	OneCustomTool():pre_action(CT_PRE_NONE),post_action(CT_POST_NONE),output_mode(CT_OUTPUT_TERMINAL),async_exec(true),on_toolbar(false){}
};

class mxOutputView;

class mxCustomToolProcess:public wxProcess {
	OneCustomTool tool;
	mxOutputView *output_view;
public:
	mxCustomToolProcess(const OneCustomTool &_tool);
	~mxCustomToolProcess();
	void OnTerminate(int pid, int status);
	void SetOutputView(mxOutputView *_output_view);
};

class CustomToolsPack {
private:
	OneCustomTool *tools;
	int cant;
public:
	CustomToolsPack(int _cant);
	CustomToolsPack(const CustomToolsPack &other);
	void ParseConfigLine(const wxString &key, const wxString &value);
	void WriteConfig(wxTextFile &file);
	OneCustomTool &operator[](int i) { return tools[i]; }
	OneCustomTool &GetTool(int i) { return tools[i]; }
	int GetCount() { return cant; }
	int GetFreeSpot(); ///< find an unused custom tool (to add one from complements installation)
	void Run(int i); ///< execute tool's pre action and launchs tool's execution
	~CustomToolsPack();
};

#endif

