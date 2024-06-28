#include "App.h"
#include "OknoZac.h"
#include "MainFrame.h"
#include "OknoSim.h"
#include <wx/wx.h>

wxIMPLEMENT_APP(App);

bool App::OnInit() {

	OknoZac* oknoZac = new OknoZac("Pnevmaticni simulator"); // ustvarimo frame z imenom

	oknoZac->SetClientSize(180, 180); // velikost aplikacije
	oknoZac->Center(); // centriramo na sredi ekrana

	oknoZac->Show(true); // prikažemo frame

	
	return true;
}
