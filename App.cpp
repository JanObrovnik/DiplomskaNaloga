#include "App.h"
#include "MainFrame.h"
#include <wx/wx.h>

wxIMPLEMENT_APP(App);

bool App::OnInit() {

	MainFrame* mainFrame = new MainFrame("C++ GUI"); // ustvarimo frame z imenom

	mainFrame->SetClientSize(800, 600); // velikost aplikacije
	mainFrame->Center(); // centriramo na sredi ekrana

	mainFrame->Show(true); // prikažemo frame
	
	return true;
}
