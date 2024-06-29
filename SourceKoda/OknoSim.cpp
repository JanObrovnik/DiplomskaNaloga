#include "OknoSim.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/time.h>
#include <wx/utils.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <fstream>



bool drzanje = false;
 
std::vector<std::vector<int>> seznamElementov;
std::vector<std::vector<double>> seznamLastnosti;
std::vector<std::vector<double>> seznamResitevReset;
std::vector<std::vector<double>> seznamResitev;

std::vector<std::vector<int>> seznamPovezav;

wxChoice* choiceDod;


OknoSim::OknoSim(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
	
	wxArrayString choices;
	choices.Add("Pnevmaticni valj");
	choices.Add("Vakumski prisesek");
	choices.Add("Pnevmaticno prijemalo");
	choices.Add("-");
	choices.Add("Idealni kompresor");
	choices.Add("Potni ventil");

	choiceDod = new wxChoice(panel, wxID_ANY, wxPoint(5, 0), wxSize(190, -1), choices/*, wxCB_SORT*/);


	panel->Bind(wxEVT_MOTION, &OknoSim::RefreshEvent, this);
	panel->Bind(wxEVT_LEFT_DOWN, &OknoSim::OnMouseDownEvent, this);
	panel->Bind(wxEVT_LEFT_UP, &OknoSim::OnMouseUpEvent, this);
	panel->Bind(wxEVT_LEFT_DCLICK, &OknoSim::OnMouseDoubleEvent, this);
	
	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(OknoSim::OnPaint));

	panel->SetDoubleBuffered(true);


	seznamElementov.push_back({ 220,20,0 });
	seznamElementov.push_back({ 380,540,1 });
	seznamElementov.push_back({ 480,320,2 });

	seznamPovezav.push_back({ 0,0,1,0,0 });
	seznamPovezav.push_back({ 0,1,2,0,0 });
	seznamPovezav.push_back({ 1,1,2,1,1 });
}


void OknoSim::RefreshEvent(wxMouseEvent& evt) {

	Refresh();
}

void OknoSim::OnMouseDownEvent(wxMouseEvent& evt) {

	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	if (mousePos.x < 200 && mousePos.y < 120 && choiceDod->GetSelection() >= 0) drzanje = true;


	Refresh();
}

void OknoSim::OnMouseUpEvent(wxMouseEvent& evt) {

	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	if (drzanje && mousePos.x > 200) {

		seznamElementov.push_back({ mousePos.x,mousePos.y,choiceDod->GetSelection() });
	}
	drzanje = false;

	
	Refresh();
}

void OknoSim::OnMouseDoubleEvent(wxMouseEvent& evt) {

	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	seznamElementov.push_back({ mousePos.x,mousePos.y,1 });

	
	Refresh();
}


void OknoSim::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();

	//- IZRIS OKNA
	dc.DrawRectangle(wxPoint(0, 30), wxSize(201, 120));
	dc.DrawRectangle(wxPoint(200, 0), wxSize(velikostOkna.x - 200, velikostOkna.y));


	//- IZRIS RISANJA POVEZAV


	//- IZRIS POVEZAV
	for (int i = 0; i < seznamPovezav.size(); i++) {

		wxPoint tocka1(0, 0);
		tocka1.x = seznamElementov[seznamPovezav[i][0]][0];
		tocka1.y = seznamElementov[seznamPovezav[i][0]][1];

		switch (seznamElementov[seznamPovezav[i][0]][2]) {

		case 0:
			tocka1.x += 110;
			tocka1.y += 10 + seznamPovezav[i][1] * 15;

			break;

		case 1:
			if (seznamPovezav[i][1] == 0) {
				tocka1.x += 10;
				tocka1.y += 10;
			}
			else if (seznamPovezav[i][1] == 1) {
				tocka1.x += 20;
				tocka1.y -= 34;
			}

			break;

		case 2:
			
			if (seznamPovezav[i][1] == 0) {
				tocka1.x -= 10;
				tocka1.y += 20;
			}
			else if (seznamPovezav[i][1] == 1) {
				tocka1.x -= 10;
				tocka1.y += 180;
			}
			else if (seznamPovezav[i][1] == 2) {
				tocka1.x += 130;
				tocka1.y += 180;
			}

			break;


		default:
			break;
		}


		wxPoint tocka2(0, 0);
		tocka2.x = seznamElementov[seznamPovezav[i][2]][0];
		tocka2.y = seznamElementov[seznamPovezav[i][2]][1];

		switch (seznamElementov[seznamPovezav[i][2]][2]) {

		case 0:
			tocka2.x += 110;
			tocka2.y += 10 + seznamPovezav[i][3] * 15;

			break;

		case 1:
			if (seznamPovezav[i][3] == 0) {
				tocka2.x += 10;
				tocka2.y += 10;
			}
			else if (seznamPovezav[i][3] == 1) {
				tocka2.x += 20;
				tocka2.y -= 34;
			}

			break;

		case 2:
			
			if (seznamPovezav[i][3] == 0) {
				tocka2.x -= 10;
				tocka2.y += 20;
			}
			else if (seznamPovezav[i][3] == 1) {
				tocka2.x -= 10;
				tocka2.y += 180;
			}
			else if (seznamPovezav[i][3] == 2) {
				tocka2.x += 130;
				tocka2.y += 180;
			}

			break;


		default:
			break;
		}

		if (seznamPovezav[i][4] == 0) {
			dc.SetPen(wxPen(wxColour(153, 153, 153), 1, wxPENSTYLE_SOLID));

			dc.DrawLine(tocka1, wxPoint((tocka1.x + tocka2.x) / 2, tocka1.y));
			dc.DrawLine(wxPoint((tocka1.x + tocka2.x) / 2, tocka1.y), wxPoint((tocka1.x + tocka2.x) / 2, tocka2.y));
			dc.DrawLine(wxPoint((tocka1.x + tocka2.x) / 2, tocka2.y), tocka2);

			dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
		}

		else if (seznamPovezav[i][4] == 1) {
			dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));

			dc.DrawLine(tocka1, wxPoint((tocka1.x + tocka2.x) / 2, tocka1.y));
			dc.DrawLine(wxPoint((tocka1.x + tocka2.x) / 2, tocka1.y), wxPoint((tocka1.x + tocka2.x) / 2, tocka2.y));
			dc.DrawLine(wxPoint((tocka1.x + tocka2.x) / 2, tocka2.y), tocka2);

			dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
		}
	}


	//- IZRIS DRZANJA ELEMENTA
	if (drzanje) {

		wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

		dc.DrawRectangle(mousePos, wxSize(80, 50));
	}

	//- IZRIS ELEMENTOV
	for (int i = 0; i < seznamElementov.size(); i++) {
		std::vector<int> xy = seznamElementov[i];

		switch (xy[2]) {

		case 0:

			dc.DrawRectangle(wxPoint(xy[0], xy[1]), wxSize(100, 140));
			dc.DrawText("0", wxPoint(xy[0] + 88, xy[1] + 5));
			dc.DrawLine(wxPoint(xy[0] + 100, xy[1] + 10), wxPoint(xy[0] + 110, xy[1] + 10));
			dc.DrawText("1", wxPoint(xy[0] + 88, xy[1] + 20));
			dc.DrawLine(wxPoint(xy[0] + 100, xy[1] + 25), wxPoint(xy[0] + 110, xy[1] + 25));
			dc.DrawText("2", wxPoint(xy[0] + 88, xy[1] + 35));
			dc.DrawLine(wxPoint(xy[0] + 100, xy[1] + 40), wxPoint(xy[0] + 110, xy[1] + 40));
			dc.DrawText("3", wxPoint(xy[0] + 88, xy[1] + 50));
			dc.DrawLine(wxPoint(xy[0] + 100, xy[1] + 55), wxPoint(xy[0] + 110, xy[1] + 55));
			dc.DrawText("4", wxPoint(xy[0] + 88, xy[1] + 65));
			dc.DrawLine(wxPoint(xy[0] + 100, xy[1] + 70), wxPoint(xy[0] + 110, xy[1] + 70));
			dc.DrawText("5", wxPoint(xy[0] + 88, xy[1] + 80));
			dc.DrawLine(wxPoint(xy[0] + 100, xy[1] + 85), wxPoint(xy[0] + 110, xy[1] + 85));
			dc.DrawText("6", wxPoint(xy[0] + 88, xy[1] + 95));
			dc.DrawLine(wxPoint(xy[0] + 100, xy[1] + 100), wxPoint(xy[0] + 110, xy[1] + 100));
			dc.DrawText("7", wxPoint(xy[0] + 88, xy[1] + 110));
			dc.DrawLine(wxPoint(xy[0] + 100, xy[1] + 115), wxPoint(xy[0] + 110, xy[1] + 115));

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0], xy[1] - 16));

			break;

		case 1:

			dc.DrawLine(wxPoint(xy[0], xy[1]), wxPoint(xy[0] + 40, xy[1]));
			dc.DrawLine(wxPoint(xy[0] + 40, xy[1]), wxPoint(xy[0] + 20, xy[1] - 34));
			dc.DrawLine(wxPoint(xy[0] + 20, xy[1] - 34), wxPoint(xy[0], xy[1]));
			dc.DrawLine(wxPoint(xy[0] + 10, xy[1] + 10), wxPoint(xy[0] + 10, xy[1]));

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0], xy[1] - 50));

			break;

		case 2:

			dc.DrawRoundedRectangle(wxPoint(xy[0], xy[1]), wxSize(120, 200), 6);
			dc.DrawRectangle(wxPoint(xy[0] + 10, xy[1] + 199), wxSize(100, 21));
			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] + 20), wxPoint(xy[0] + 10, xy[1] + 20));

			dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] + 180), wxPoint(xy[0] + 10, xy[1] + 180));
			dc.DrawLine(wxPoint(xy[0] + 110, xy[1] + 180), wxPoint(xy[0] + 130, xy[1] + 180));
			dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));


			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0], xy[1] - 16));

			break;

		case 3:



			break;

		case 4:



			break;


		default:

			dc.DrawRectangle(wxPoint(xy[0], xy[1]), wxSize(80, 50));
			dc.DrawText("Neznan \nelement", wxPoint(xy[0] + 18, xy[1] + 10));

			break;
		}
	}
}
