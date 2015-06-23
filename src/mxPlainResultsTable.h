#ifndef MXPLAINRESULTSTABLE_H
#define MXPLAINRESULTSTABLE_H
#include <wx/panel.h>
#include <wx/grid.h>
#include <vector>
#include <algorithm>
using namespace std;

class wxTextCtrl;
class mxPlainResultsTable;

class mxPRTData {
	mxPlainResultsTable *parent;
	friend class mxPlainResultsTable;
protected:
	wxString GetCellValue(int row, int col);
public:
	virtual int GetRowsCount() = 0;
	virtual int GetColumnsCount() = 0;
	virtual bool EnableFilter() { return false; }
	virtual bool RowMatches(int row, const wxString &filter) { return true; }
	virtual wxString GetColumnName(int column) = 0;
	virtual wxString GetData(int row, int column) = 0;
	virtual bool Sort(int column, bool ascending) { return false; }
	virtual void OnDClick(int row, int column) {}
	virtual ~mxPRTData() {}
};

template<typename Row>
class mxPRTGenericTable : public mxPRTData {
	wxArrayString titles;
	vector<Row> rows;
public:
	mxPRTGenericTable(wxArrayString &ttitles) :titles(ttitles) {}
	void AddRow(const Row &r) { rows.push_back(r); }
	virtual int GetRowsCount() { return rows.size(); }
	virtual int GetColumnsCount() { return titles.GetCount(); }
	virtual bool EnableFilter() { return true; }
	virtual bool RowMatches(int row, const wxString &filter) { 
		return rows[row].Filter(filter);
	}
	virtual wxString GetColumnName(int column) {
		return titles[column];
	}
	virtual wxString GetData(int row, int column) {
		return rows[row][column];
	}
	virtual bool Sort(int column, bool ascending) { 
		Row::Sort(rows,column,ascending);
		return true;
	}
	virtual void OnDClick(int row, int column) {}
	virtual ~mxPRTGenericTable() {}
};

template<int FilterRow>
struct mxPRTGenericRow {
	vector<wxString> vals_str;
	vector<int> vals_int;
	struct Aux{bool es_str; int pos;};
	vector<Aux> map;
	wxString operator[](int i) {
		if (map[i].es_str) return vals_str[map[i].pos];
		else return wxString()<<vals_int[map[i].pos];
	}
	void Add(int val) { Aux a; a.pos=vals_int.size(); a.es_str=false; map.push_back(a); vals_int.push_back(val); }
	void Add(const wxString &val) { Aux a; a.pos=vals_str.size(); a.es_str=true; map.push_back(a); vals_str.push_back(val); }
	static int sort_aux;
	static bool sort_by_str(const mxPRTGenericRow &r1, const mxPRTGenericRow&r2) 
		{ return r1.vals_str[sort_aux]<r2.vals_str[sort_aux]; }
	static bool sort_by_int(const mxPRTGenericRow &r1, const mxPRTGenericRow &r2) 
		{ return r1.vals_int[sort_aux]<r2.vals_int[sort_aux]; }
	static void Sort(vector<mxPRTGenericRow> &v, int col, bool asc) {
		if (v.empty()) return;
		sort_aux = v[0].map[col].pos;
		sort(v.begin(),v.end(), v[0].map[col].es_str?sort_by_str:sort_by_int);
		if (!asc) reverse(v.begin(),v.end());
	}
	bool Filter(const wxString &str) { return (*this)[FilterRow].Upper().Contains(str.Upper()); }
};
template<int FilterRow> 
int mxPRTGenericRow<FilterRow>::sort_aux;

class mxPlainResultsTable: public wxPanel {
	friend class mxPRTData;
	wxGrid *grid;
	mxPRTData *data;
	wxTextCtrl *search;
	int sort_status;
public:
	void UpdateTable();
	mxPlainResultsTable(wxWindow *parent, mxPRTData *pdata);
	~mxPlainResultsTable();
	void SetData(mxPRTData *pdata);
protected:
	void OnClickTableCell(wxGridEvent &evt);
	void OnDClickTableCell(wxGridEvent &evt);
	void OnClickTableLabel(wxGridEvent &evt);
	void OnSearchTextChange(wxCommandEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

