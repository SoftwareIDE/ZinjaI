#include "Application.h"

#include "mxWindow.h"
using namespace std;

bool mxApplication::OnInit() {
	new mxWindow;
	return true;
}
