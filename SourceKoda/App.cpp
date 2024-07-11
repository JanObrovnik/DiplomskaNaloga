#include "App.h"
#include "OknoZac.h"
#include "MainFrame.h"
#include "OknoSim.h"
#include <wx/wx.h>

wxIMPLEMENT_APP(App);

bool App::OnInit() {

	//OknoZac* oknoZac = new OknoZac("Pnevmaticni simulator"); // ustvarimo frame z imenom
	OknoSim* oknoZac = new OknoSim("Pnevmaticni simulator"); // ustvarimo frame z imenom

	oknoZac->SetClientSize(900, 600); // velikost aplikacije 180, 180
	oknoZac->Center(); // centriramo na sredi ekrana

	oknoZac->Show(true); // prikažemo frame

	
	return true;
}
