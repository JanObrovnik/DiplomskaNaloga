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
bool drzanjeElementa = false;
int casDrzanja = 0;
short drzanjePovezav = 0;
short izbranElement = -1;
 
std::vector<std::vector<int>> seznamElementov;
std::vector<std::vector<double>> seznamLastnosti;
std::vector<std::vector<double>> seznamResitevReset;
std::vector<std::vector<double>> seznamResitev;

std::vector<std::vector<int>> seznamPovezav;

wxChoice* choiceDod;


OknoSim::OknoSim(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* risanjePovezav = new wxButton(panel, wxID_ANY, "Risanje povezav", wxPoint(5,280), wxSize(190,-1));
	
	wxArrayString choices;
	choices.Add("Mikroprocesor");
	choices.Add("Elektricna Crpalka");
	choices.Add("Tlacna Posoda");
	choices.Add("Prijemalo");
	choices.Add("Prisesek");

	choiceDod = new wxChoice(panel, wxID_ANY, wxPoint(5, 0), wxSize(190, -1), choices/*, wxCB_SORT*/);
	choiceDod->SetSelection(0);


	panel->Bind(wxEVT_MOTION, &OknoSim::RefreshEvent, this);
	panel->Bind(wxEVT_LEFT_DOWN, &OknoSim::OnMouseDownEvent, this);
	panel->Bind(wxEVT_LEFT_UP, &OknoSim::OnMouseUpEvent, this);
	panel->Bind(wxEVT_LEFT_DCLICK, &OknoSim::OnMouseDoubleEvent, this);
	risanjePovezav->Bind(wxEVT_BUTTON, &OknoSim::OnRisanjePovezavClicked, this);
	
	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(OknoSim::OnPaint));

	panel->SetDoubleBuffered(true);


	seznamElementov.push_back({ 220,20,0 });
	seznamElementov.push_back({ 380,540,1 });
	seznamElementov.push_back({ 480,320,2 });
	seznamElementov.push_back({ 700,350,3 });
	seznamElementov.push_back({ 700,500,4 });


	seznamPovezav.push_back({ 0,0,1,0,0 });
	seznamPovezav.push_back({ 0,1,2,0,0 });
	seznamPovezav.push_back({ 1,1,2,1,1 });
	seznamPovezav.push_back({ 2,2,3,1,1 });
	seznamPovezav.push_back({ 2,2,4,1,1 });
}


void OknoSim::RefreshEvent(wxMouseEvent& evt) {

	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	if (drzanjeElementa && izbranElement >= 0 && casDrzanja > 2 ) {
		seznamElementov[izbranElement][0] = mousePos.x / 10 * 10;
		seznamElementov[izbranElement][1] = mousePos.y / 10 * 10;
	}

	casDrzanja++;

	Refresh();
}

void OknoSim::OnMouseDownEvent(wxMouseEvent& evt) {

	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	if (mousePos.x < 200 && mousePos.y < 240) drzanje = true;
	else if (mousePos.x > 200) {

		for (int i = 0; i < seznamElementov.size(); i++) {
			int vrst = seznamPovezav[seznamPovezav.size() - 1][4];

			if (seznamElementov[i][2] == 0) {
				if (drzanjePovezav > 0) {
					if (mousePos.x > seznamElementov[i][0] + 100 && mousePos.x < seznamElementov[i][0] + 115 && mousePos.y > seznamElementov[i][1] + 5 && mousePos.y < seznamElementov[i][1] + 15) { 
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 0; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] + 100 && mousePos.x < seznamElementov[i][0] + 115 && mousePos.y > seznamElementov[i][1] + 20 && mousePos.y < seznamElementov[i][1] + 30) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 1; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] + 100 && mousePos.x < seznamElementov[i][0] + 115 && mousePos.y > seznamElementov[i][1] + 35 && mousePos.y < seznamElementov[i][1] + 45) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 2; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++;	}
					else if (mousePos.x > seznamElementov[i][0] + 100 && mousePos.x < seznamElementov[i][0] + 115 && mousePos.y > seznamElementov[i][1] + 50 && mousePos.y < seznamElementov[i][1] + 60) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 3; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] + 100 && mousePos.x < seznamElementov[i][0] + 115 && mousePos.y > seznamElementov[i][1] + 65 && mousePos.y < seznamElementov[i][1] + 75) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 4; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] + 100 && mousePos.x < seznamElementov[i][0] + 115 && mousePos.y > seznamElementov[i][1] + 80 && mousePos.y < seznamElementov[i][1] + 90) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 5; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] + 100 && mousePos.x < seznamElementov[i][0] + 115 && mousePos.y > seznamElementov[i][1] + 95 && mousePos.y < seznamElementov[i][1] + 105) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 6; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] + 100 && mousePos.x < seznamElementov[i][0] + 115 && mousePos.y > seznamElementov[i][1] + 110 && mousePos.y < seznamElementov[i][1] + 120) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 7; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] + 100 && mousePos.x < seznamElementov[i][0] + 115 && mousePos.y > seznamElementov[i][1] + 125 && mousePos.y < seznamElementov[i][1] + 135) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 8; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
				}
				if (mousePos.x > seznamElementov[i][0] && mousePos.x < seznamElementov[i][0] + 100 && mousePos.y > seznamElementov[i][1] && mousePos.y < seznamElementov[i][1] + 140) {
					if (izbranElement == i) izbranElement = -1;
					else {
						izbranElement = i;
						drzanjeElementa = true;
					}
				}
			}
			else if (seznamElementov[i][2] == 1) { 
				if (drzanjePovezav > 0) {
					if (mousePos.x > seznamElementov[i][0] + 5 && mousePos.x < seznamElementov[i][0] + 15 && mousePos.y > seznamElementov[i][1] + 5 && mousePos.y < seznamElementov[i][1] + 15) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 0; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] + 15 && mousePos.x < seznamElementov[i][0] + 25 && mousePos.y > seznamElementov[i][1] - 39 && mousePos.y < seznamElementov[i][1] - 29) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 1; seznamPovezav[seznamPovezav.size() - 1][4] = 1; drzanjePovezav++; }
				}
				if (mousePos.x > seznamElementov[i][0] && mousePos.x < seznamElementov[i][0] + 40 && mousePos.y > seznamElementov[i][1] - 34 && mousePos.y < seznamElementov[i][1]) {
					if (izbranElement == i) izbranElement = -1;
					else {
						izbranElement = i;
						drzanjeElementa = true;
					}
				}
			}
			else if (seznamElementov[i][2] == 2) {
				if (drzanjePovezav > 0) {
					if (mousePos.x > seznamElementov[i][0] - 15 && mousePos.x < seznamElementov[i][0] - 5 && mousePos.y > seznamElementov[i][1] + 15 && mousePos.y < seznamElementov[i][1] + 25) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 0; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] - 15 && mousePos.x < seznamElementov[i][0] - 5 && mousePos.y > seznamElementov[i][1] + 175 && mousePos.y < seznamElementov[i][1] + 185) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 1; seznamPovezav[seznamPovezav.size() - 1][4] = 1; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] + 115 && mousePos.x < seznamElementov[i][0] + 125 && mousePos.y > seznamElementov[i][1] + 175 && mousePos.y < seznamElementov[i][1] + 185) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 2; seznamPovezav[seznamPovezav.size() - 1][4] = 1; drzanjePovezav++; }
				}
				if (mousePos.x > seznamElementov[i][0] && mousePos.x < seznamElementov[i][0] + 120 && mousePos.y > seznamElementov[i][1] && mousePos.y < seznamElementov[i][1] + 200) {
					if (izbranElement == i) izbranElement = -1;
					else {
						izbranElement = i;
						drzanjeElementa = true;
					}
				}
			}
			else if (seznamElementov[i][2] == 3) { 
				if (drzanjePovezav > 0) {
					if (mousePos.x > seznamElementov[i][0] - 15 && mousePos.x < seznamElementov[i][0] - 5 && mousePos.y > seznamElementov[i][1] + 5 && mousePos.y < seznamElementov[i][1] + 15) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 0; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] - 15 && mousePos.x < seznamElementov[i][0] - 5 && mousePos.y > seznamElementov[i][1] + 35 && mousePos.y < seznamElementov[i][1] + 45) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 1; seznamPovezav[seznamPovezav.size() - 1][4] = 1; drzanjePovezav++; }
				}
				if (mousePos.x > seznamElementov[i][0] && mousePos.x < seznamElementov[i][0] + 90 && mousePos.y > seznamElementov[i][1] && mousePos.y < seznamElementov[i][1] + 50) {
					if (izbranElement == i) izbranElement = -1;
					else {
						izbranElement = i;
						drzanjeElementa = true;
					}
				}
			}
			else if (seznamElementov[i][2] == 4) { 
				if (drzanjePovezav > 0) {
					if (mousePos.x > seznamElementov[i][0] + 0 && mousePos.x < seznamElementov[i][0] + 0 && mousePos.y > seznamElementov[i][1] - 25 && mousePos.y < seznamElementov[i][1] - 15) { /////////// Še ne obstaja
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 0; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] + 35 && mousePos.x < seznamElementov[i][0] + 45 && mousePos.y > seznamElementov[i][1] - 25 && mousePos.y < seznamElementov[i][1] - 15) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 1; seznamPovezav[seznamPovezav.size() - 1][4] = 1; drzanjePovezav++; }
				}
				if (mousePos.x > seznamElementov[i][0] && mousePos.x < seznamElementov[i][0] + 80 && mousePos.y > seznamElementov[i][1] - 20 && mousePos.y < seznamElementov[i][1]) {
					if (izbranElement == i) izbranElement = -1;
					else {
						izbranElement = i;
						drzanjeElementa = true;
					}
				}
			}
			else {
				if (mousePos.x > seznamElementov[i][0] && mousePos.x < seznamElementov[i][0] + 80 && mousePos.y > seznamElementov[i][1] && mousePos.y < seznamElementov[i][1] + 50) {
					if (izbranElement == i) izbranElement = -1;
					else {
						izbranElement = i;
						drzanjeElementa = true;
					}
				}
			}
			if (drzanjePovezav == 3 && seznamPovezav[seznamPovezav.size() - 1][4] != vrst) {
				seznamPovezav.erase(seznamPovezav.begin() + seznamPovezav.size() - 1);
				drzanjePovezav = 0;
			}
		}
		if (drzanjePovezav == 3) {
			seznamPovezav.push_back({ -1,-1,-1,-1,-1 });
			drzanjePovezav = 1;
		}
	}

	casDrzanja = 0;
	
	Refresh();
}

void OknoSim::OnMouseUpEvent(wxMouseEvent& evt) {

	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	if (drzanje && mousePos.x > 200) {

		seznamElementov.push_back({ mousePos.x/10*10,mousePos.y/10*10,choiceDod->GetSelection() });
	}
	else if (izbranElement >= 0 && casDrzanja > 2 && mousePos.x > 200 && drzanjeElementa) {

		seznamElementov[izbranElement][0] = mousePos.x / 10 * 10;
		seznamElementov[izbranElement][1] = mousePos.y / 10 * 10;

		izbranElement = -1;
	}

	drzanje = false;
	drzanjeElementa = false;
	
	Refresh();
}

void OknoSim::OnMouseDoubleEvent(wxMouseEvent& evt) {

	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	if (izbranElement >= 0) {

		seznamPovezav.erase(seznamPovezav.begin() + seznamPovezav.size() - 1);
		drzanjePovezav = 0;
	}

	
	Refresh();
}

void OknoSim::OnRisanjePovezavClicked(wxCommandEvent& evt) {

	izbranElement = -1;

	if (drzanjePovezav == 0) {

		seznamPovezav.push_back({ -1,-1,-1,-1,-1 });

		drzanjePovezav = 1;
	}
	else {

		seznamPovezav.erase(seznamPovezav.begin() + seznamPovezav.size() - 1);

		drzanjePovezav = 0;
	}

	Refresh();
}


void OknoSim::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());
	
	if (true) { // ADMIN LOGS
		for (int i = 0; i < seznamPovezav.size(); i++) dc.DrawText(wxString::Format("%d | %d | %d | %d | %d", seznamPovezav[i][0], seznamPovezav[i][1], seznamPovezav[i][2], seznamPovezav[i][3], seznamPovezav[i][4]), wxPoint(20, 400 + 12 * i));
	}

	//- IZRIS OKNA
	int sirinaOrodja = 200;
	int visinaOrodja = 240;

	dc.DrawRectangle(wxPoint(0, 30), wxSize(sirinaOrodja + 1, visinaOrodja));
	dc.DrawRectangle(wxPoint(sirinaOrodja, 0), wxSize(velikostOkna.x - sirinaOrodja, velikostOkna.y));


	//- PRIKAZ IZBRANEGA ELEMENTA
	if (izbranElement >= 0) {
		wxLogStatus(wxString::Format("%d - %d", izbranElement, seznamElementov[izbranElement][2]));
		wxPoint oznacenElementPoint(seznamElementov[izbranElement][0], seznamElementov[izbranElement][1]);
		wxSize oznacenElementSize(100, 70);
		if (seznamElementov[izbranElement][2] == 0) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 10; oznacenElementSize.x = 120; oznacenElementSize.y = 160; }
		else if (seznamElementov[izbranElement][2] == 1) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 44; oznacenElementSize.x = 60; oznacenElementSize.y = 54; }
		else if (seznamElementov[izbranElement][2] == 2) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 10; oznacenElementSize.x = 140; oznacenElementSize.y = 240; }
		else if (seznamElementov[izbranElement][2] == 3) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 10; oznacenElementSize.x = 110; oznacenElementSize.y = 70; }
		else if (seznamElementov[izbranElement][2] == 4) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 30; oznacenElementSize.x = 100; oznacenElementSize.y = 40; }

		dc.SetPen(wxPen(wxColour(153, 153, 255), 1, wxPENSTYLE_LONG_DASH));
		dc.DrawRectangle(oznacenElementPoint, oznacenElementSize);
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}
	

	//- IZRIS PRIKAZA ELEMENTA
	wxPoint predogled(10, 40);

	if (drzanje) { 
		predogled = this->ScreenToClient(wxGetMousePosition()); 
		predogled.x = predogled.x / 10 * 10;
		predogled.y = predogled.y / 10 * 10;
	}
	else if (choiceDod->GetSelection() == 0) { predogled.x = sirinaOrodja / 2 - 50; }
	else if (choiceDod->GetSelection() == 1) { predogled.x = sirinaOrodja / 2 - 20; predogled.y += 34; }
	else if (choiceDod->GetSelection() == 2) { predogled.x = sirinaOrodja / 2 - 60; }
	else if (choiceDod->GetSelection() == 3) { predogled.x = sirinaOrodja / 2 - 45; }
	else if (choiceDod->GetSelection() == 4) { predogled.x = sirinaOrodja / 2 - 40; predogled.y += 20; }
		
	
	if (choiceDod->GetSelection() == 0) {
		dc.DrawRectangle(predogled, wxSize(100, 140));
		dc.DrawText("0", wxPoint(predogled.x + 88, predogled.y + 5));
		dc.DrawLine(wxPoint(predogled.x + 100, predogled.y + 10), wxPoint(predogled.x + 110, predogled.y + 10));
		dc.DrawText("1", wxPoint(predogled.x + 88, predogled.y + 20));
		dc.DrawLine(wxPoint(predogled.x + 100, predogled.y + 25), wxPoint(predogled.x + 110, predogled.y + 25));
		dc.DrawText("2", wxPoint(predogled.x + 88, predogled.y + 35));
		dc.DrawLine(wxPoint(predogled.x + 100, predogled.y + 40), wxPoint(predogled.x + 110, predogled.y + 40));
		dc.DrawText("3", wxPoint(predogled.x + 88, predogled.y + 50));
		dc.DrawLine(wxPoint(predogled.x + 100, predogled.y + 55), wxPoint(predogled.x + 110, predogled.y + 55));
		dc.DrawText("4", wxPoint(predogled.x + 88, predogled.y + 65));
		dc.DrawLine(wxPoint(predogled.x + 100, predogled.y + 70), wxPoint(predogled.x + 110, predogled.y + 70));
		dc.DrawText("5", wxPoint(predogled.x + 88, predogled.y + 80));
		dc.DrawLine(wxPoint(predogled.x + 100, predogled.y + 85), wxPoint(predogled.x + 110, predogled.y + 85));
		dc.DrawText("6", wxPoint(predogled.x + 88, predogled.y + 95));
		dc.DrawLine(wxPoint(predogled.x + 100, predogled.y + 100), wxPoint(predogled.x + 110, predogled.y + 100));
		dc.DrawText("7", wxPoint(predogled.x + 88, predogled.y + 110));
		dc.DrawLine(wxPoint(predogled.x + 100, predogled.y + 115), wxPoint(predogled.x + 110, predogled.y + 115));
	}
	else if (choiceDod->GetSelection() == 1) {
		wxPoint* t1 = new wxPoint(predogled.x, predogled.y);
		wxPoint* t2 = new wxPoint(predogled.x + 40, predogled.y);
		wxPoint* t3 = new wxPoint(predogled.x + 20, predogled.y - 34);

		wxPointList* tocke = new wxPointList();
		tocke->Append(t1);
		tocke->Append(t2);
		tocke->Append(t3);

		dc.DrawPolygon(tocke);

		dc.DrawLine(wxPoint(predogled.x + 10, predogled.y + 10), wxPoint(predogled.x + 10, predogled.y));
	}
	else if (choiceDod->GetSelection() == 2) {
		dc.DrawRoundedRectangle(wxPoint(predogled.x, predogled.y), wxSize(120, 200), 6);
		dc.DrawRectangle(wxPoint(predogled.x + 10, predogled.y + 199), wxSize(100, 21));
		dc.DrawLine(wxPoint(predogled.x - 10, predogled.y + 20), wxPoint(predogled.x + 10, predogled.y + 20));

		dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
		dc.DrawLine(wxPoint(predogled.x - 10, predogled.y + 180), wxPoint(predogled.x + 10, predogled.y + 180));
		dc.DrawLine(wxPoint(predogled.x + 110, predogled.y + 180), wxPoint(predogled.x + 130, predogled.y + 180));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}
	else if (choiceDod->GetSelection() == 3) {
		dc.DrawRectangle(wxPoint(predogled.x, predogled.y), wxSize(65, 50));
		dc.DrawRectangle(wxPoint(predogled.x + 40, predogled.y), wxSize(50, 10));
		dc.DrawRectangle(wxPoint(predogled.x + 40, predogled.y + 40), wxSize(50, 10));
	}
	else if (choiceDod->GetSelection() == 4) {
		wxPoint* t1 = new wxPoint(predogled.x, predogled.y);
		wxPoint* t2 = new wxPoint(predogled.x + 80, predogled.y);
		wxPoint* t3 = new wxPoint(predogled.x + 70, predogled.y - 10);
		wxPoint* t4 = new wxPoint(predogled.x + 10, predogled.y - 10);

		wxPointList* tocke = new wxPointList();
		tocke->Append(t1);
		tocke->Append(t2);
		tocke->Append(t3);
		tocke->Append(t4);

		dc.DrawPolygon(tocke);

		dc.DrawLine(wxPoint(predogled.x + 40, predogled.y - 10), wxPoint(predogled.x + 40, predogled.y - 20));
	}


	//- IZRIS POVEZAV
	for (int i = 0; i < seznamPovezav.size(); i++) {

		wxPoint tocka1(0, 0);
		if (drzanjePovezav != 1 || i < seznamPovezav.size() - 1) {
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

			case 3:

				if (seznamPovezav[i][1] == 0) {
					tocka1.x -= 10;
					tocka1.y += 10;
				}
				else if (seznamPovezav[i][1] == 1) {
					tocka1.x -= 10;
					tocka1.y += 40;
				}

				break;

			case 4:

				if (seznamPovezav[i][1] == 1) {
					tocka1.x += 40;
					tocka1.y -= 20;
				}

				break;


			default:
				break;
			}
		}


		wxPoint tocka2(0, 0);
		if (drzanjePovezav == 2 && i == seznamPovezav.size() - 1) tocka2 = mousePos;
		else if (drzanjePovezav != 1 || i < seznamPovezav.size() - 1) {
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

			case 3:

				if (seznamPovezav[i][3] == 0) {
					tocka2.x -= 10;
					tocka2.y += 10;
				}
				else if (seznamPovezav[i][3] == 1) {
					tocka2.x -= 10;
					tocka2.y += 40;
				}

				break;

			case 4:

				if (seznamPovezav[i][3] == 1) {
					tocka2.x += 40;
					tocka2.y -= 20;
				}

				break;


			default:
				break;
			}
		}

		if (seznamPovezav[i][4] == 0) dc.SetPen(wxPen(wxColour(153, 153, 153), 1, wxPENSTYLE_SOLID));
		else if (seznamPovezav[i][4] == 1) dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));	

		dc.DrawLine(tocka1, wxPoint((tocka1.x + tocka2.x) / 2, tocka1.y));
		dc.DrawLine(wxPoint((tocka1.x + tocka2.x) / 2, tocka1.y), wxPoint((tocka1.x + tocka2.x) / 2, tocka2.y));
		dc.DrawLine(wxPoint((tocka1.x + tocka2.x) / 2, tocka2.y), tocka2);

		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

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

			if (drzanjePovezav > 0) {
				dc.SetPen(wxPen(wxColour(51, 51, 153), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 153, 255), wxBRUSHSTYLE_SOLID));

				dc.DrawRectangle(wxPoint(xy[0] + 105, xy[1] + 5), wxSize(10, 10));
				dc.DrawRectangle(wxPoint(xy[0] + 105, xy[1] + 20), wxSize(10, 10));
				dc.DrawRectangle(wxPoint(xy[0] + 105, xy[1] + 35), wxSize(10, 10));
				dc.DrawRectangle(wxPoint(xy[0] + 105, xy[1] + 50), wxSize(10, 10));
				dc.DrawRectangle(wxPoint(xy[0] + 105, xy[1] + 65), wxSize(10, 10));
				dc.DrawRectangle(wxPoint(xy[0] + 105, xy[1] + 80), wxSize(10, 10));
				dc.DrawRectangle(wxPoint(xy[0] + 105, xy[1] + 95), wxSize(10, 10));
				dc.DrawRectangle(wxPoint(xy[0] + 105, xy[1] + 110), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
			}

			break;

		case 1:

			if (true) {
				wxPoint* t1 = new wxPoint(xy[0], xy[1]);
				wxPoint* t2 = new wxPoint(xy[0] + 40, xy[1]);
				wxPoint* t3 = new wxPoint(xy[0] + 20, xy[1] - 34);

				wxPointList* tocke = new wxPointList();
				tocke->Append(t1);
				tocke->Append(t2);
				tocke->Append(t3);

				dc.DrawPolygon(tocke);

				dc.DrawLine(wxPoint(xy[0] + 10, xy[1] + 10), wxPoint(xy[0] + 10, xy[1]));

				dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0], xy[1] - 50));

				if (drzanjePovezav > 0) {
					dc.SetPen(wxPen(wxColour(51, 51, 153), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 153, 255), wxBRUSHSTYLE_SOLID));

					dc.DrawRectangle(wxPoint(xy[0] + 5, xy[1] + 5), wxSize(10, 10));

					dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));

					dc.DrawRectangle(wxPoint(xy[0] + 15, xy[1] - 39), wxSize(10, 10));

					dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
				}
			}

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

			if (drzanjePovezav > 0) {
				dc.SetPen(wxPen(wxColour(51, 51, 153), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 153, 255), wxBRUSHSTYLE_SOLID));

				dc.DrawRectangle(wxPoint(xy[0] - 15, xy[1] + 15), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));

				dc.DrawRectangle(wxPoint(xy[0] - 15, xy[1] + 175), wxSize(10, 10));
				dc.DrawRectangle(wxPoint(xy[0] + 125, xy[1] + 175), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
			}

			break;

		case 3:

			dc.DrawRectangle(wxPoint(xy[0], xy[1]), wxSize(65, 50));
			dc.DrawRectangle(wxPoint(xy[0] + 40, xy[1]), wxSize(50, 10));
			dc.DrawRectangle(wxPoint(xy[0] + 40, xy[1] + 40), wxSize(50, 10));

			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] + 10), wxPoint(xy[0], xy[1] + 10));

			dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));

			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] + 40), wxPoint(xy[0], xy[1] + 40));

			dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0], xy[1] - 16));

			if (drzanjePovezav > 0) {
				dc.SetPen(wxPen(wxColour(51, 51, 153), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 153, 255), wxBRUSHSTYLE_SOLID));

				dc.DrawRectangle(wxPoint(xy[0] - 15, xy[1] + 5), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));

				dc.DrawRectangle(wxPoint(xy[0] - 15, xy[1] + 35), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
			}

			break;

		case 4:

			if (true) {
				wxPoint* t1 = new wxPoint(xy[0], xy[1]);
				wxPoint* t2 = new wxPoint(xy[0] + 80, xy[1]);
				wxPoint* t3 = new wxPoint(xy[0] + 70, xy[1] - 10);
				wxPoint* t4 = new wxPoint(xy[0] + 10, xy[1] - 10);

				wxPointList* tocke = new wxPointList();
				tocke->Append(t1);
				tocke->Append(t2);
				tocke->Append(t3);
				tocke->Append(t4);

				dc.DrawPolygon(tocke);

				dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));

				dc.DrawLine(wxPoint(xy[0] + 40, xy[1] - 10), wxPoint(xy[0] + 40, xy[1] - 20));

				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

				dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0], xy[1]));

				if (drzanjePovezav > 0) {
					dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));

					dc.DrawRectangle(wxPoint(xy[0] + 35, xy[1] - 25), wxSize(10, 10));

					dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
				}
			}

			break;


		default:

			dc.DrawRectangle(wxPoint(xy[0], xy[1]), wxSize(80, 50));
			dc.DrawText("Neznan \nelement", wxPoint(xy[0] + 18, xy[1] + 10));

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0], xy[1] - 16));

			break;
		}
	}
}
