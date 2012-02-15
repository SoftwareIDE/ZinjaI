#include <wx/hashmap.h>
#include <wx/string.h>

class mxSource;
class wxSocketServer;
class wxSocketClient;
class wxSocketBase;
class wxSocketEvent;
class wxControlWithItems;

//! for receiving sources
struct share_conection_c {
	wxChar *source; ///< the source itself as plain text
	wxString name; ///< nombre para mostrar en la pestaña del notebook
	long int len; ///< how long is the source text (-1 means len is still unknown)
	long int count; ///< how much has we already receive 
};

//! for receiving sources
struct source_share {
	void *source; ///< the char* if freezed, or the mxSource* otherwise 
	long int len; ///< -1 means not freezed, otherwise means how long is the text
};

WX_DECLARE_HASH_MAP( wxString, source_share, wxStringHash, wxStringEqual, HashStringSource );
WX_DECLARE_VOIDPTR_HASH_MAP( share_conection_c, HashPtrClient );

/**
* @brief Administra la compartición de fuentes a travéz de una LAN
**/
class ShareManager {
private:
	HashStringSource data; ///< sources we share
	HashPtrClient clients; ///< sources we are downloading
	wxSocketServer *index_server; ///< for sending list to others
	wxSocketClient *index_client; ///< for getting other's lists
	bool index_is_waiting; ///< to know if there's an index_client waiting
	wxControlWithItems *my_list; ///< source's list in mxOpenSharedWindow
	wxChar *list_buf; ///< aux for getting other's lists (for socket->Read())
	int list_count;///< aux for getting other's lists (how many bytes have we already received in list_buf)
	int list_last; ///< aux for getting other's lists (where ends the last received source name in list_buf)
	long int list_tot; ///< aux for getting other's lists (how many source's names should we receive)  
	long int list_rec; ///< aux for getting other's lists (how many source's names have we already received)
	wxString list_hostname; ///< hostname al que se le pidio la lista, para saber luego a quien pedirle los fuentes
public:
	ShareManager();
	~ShareManager();
	bool CheckServer();
	bool Exists(wxString name);
	bool Exists(mxSource *source);
	void Delete(wxString name);
	void Delete(mxSource *source);
	bool GetList(wxString host, wxListBox *list);
	bool AskFor(wxString name);
	void Insert(wxString name, mxSource *source, bool freezed=false);
	void Replace(wxString name, mxSource *source, bool freezed=false);
	void OnSocketEvent(wxSocketEvent *event);
	int GetMyList(wxControlWithItems *ctrl);
	bool Freeze(mxSource *source);
	int GetSharedList(wxArrayString &a); ///< append shared sources' titles (key from hash data) in a and return the count
};

extern ShareManager *share;

