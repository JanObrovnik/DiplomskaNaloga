#include "OknoZac.h"
#include "MainFrame.h"
#include "OknoSim.h"
#include <wx/wx.h>



OknoZac::OknoZac(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* Gumb1 = new wxButton(panel, wxID_ANY, "Simulacija 1", wxPoint(40, 20), wxSize(100, 60));
	wxButton* Gumb2 = new wxButton(panel, wxID_ANY, "Simulacija 2", wxPoint(40, 100), wxSize(100, 60));

	Gumb1->Bind(wxEVT_BUTTON, &OknoZac::OnButton1Clicked, this);
	Gumb2->Bind(wxEVT_BUTTON, &OknoZac::OnButton2Clicked, this);

	wxStatusBar* statusBar = CreateStatusBar();
}

void OknoZac::OnButton1Clicked(wxCommandEvent& evt) {

	MainFrame* mainFrame = new MainFrame("Pnevmaticni simulator 1");

	mainFrame->SetClientSize(900, 600);
	mainFrame->Center();

	mainFrame->Show(true);
}

void OknoZac::OnButton2Clicked(wxCommandEvent& evt) {

	OknoSim* oknoSim = new OknoSim("Pnevmaticni simulator 2");

	oknoSim->SetClientSize(900, 600);
	oknoSim->Center();

	oknoSim->Show(true);
}
