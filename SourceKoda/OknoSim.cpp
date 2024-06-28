#include "OknoSim.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/time.h>
#include <wx/utils.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <fstream>



 


OknoSim::OknoSim(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxSize size = panel->GetSize();
	


	panel->Bind(wxEVT_LEFT_DOWN, &OknoSim::OnMouseEvent, this);



	wxStatusBar* statusBar = CreateStatusBar();
}


void OknoSim::OnMouseEvent(wxMouseEvent& evt) {

	wxLogStatus("0");
}
