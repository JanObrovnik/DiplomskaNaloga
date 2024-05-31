#include "MainFrame.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
//#include <algorithm>



std::vector<double> izracun(int n) {
	std::vector<double> res;
	

	const double pi = 3.14159265358979; //- pi

	double g = 9.81 * 1000; //- gravitacijski pospesek [mm/s^2]
	double pok = 1.01325 / 10; //- atmosferski tlak [MPa]

	double Ftr_s = 100; //- Sila staticnega trenja znotraj valja [N]
	double Ftr_d = 20; //- Sila dinamicnega trenja znotraj valja [N]

	//PODATKI ZA DIFERENÈNI VALJ
	double D = 40; //- premer bata [mm]
	double d = 20; //- premer batnice [mm]
	double l = 250; //- hod bata [mm]
	double x0 = 0; //- zaèetna pozicija bata [mm]
	double V_0_krmilni = 10000; //- krmilni volumen na zacetni strani [mm^3]
	double V_1_krmilni = 10000; //- krmilni volumen na koncni strani [mm^3]
	double m = .6; //- masa batnice in bata [kg]

	double A0 = pi * pow(D, 2) / 4; //- površina bata na strani 0 [mm^2]
	double A1 = pi * (pow(D, 2) - pow(d, 2)) / 4; //- površina bata na strani 1 [mm^2]

	double V0 = x0 * A0 + V_0_krmilni; //- zaèetni volumen na zacetni strani
	double V1 = (l - x0) * A1 + V_1_krmilni; //- zaèetni volumen na koncni strani

	//VSTOPNI PODATKI
	double p0 = 6. / 10; //- tlak v komori 0 [MPa]
	double p1 = pok; //- tlak v komori 0 [MPa]


	//Simulacija 1.1.:
	//premik bata, zracno izoliran

	double p01 = 0, p02 = 0, p11 = 0, p12 = 0;
	double V01, V02, V11, V12;
	double a = 0, v = 0, dv = 0, x = 0, dx = 0;
	double ti = .01; //- casovni korak [s]

	n = n*10; //- stevilo ponovitev simulacije
	for (int i = 0; i < n; i++) {

		if (i == 0) { // 1.0
			p01 = p0;
			p11 = p1;
			V01 = V02 = V0;
			V11 = V12 = V1;
		}

		p02 = p01 * V01 / V02; // 1.1
		p12 = p11 * V11 / V12;

		p01 = p02;
		p11 = p12;

		V01 = V02; // 1.1.2
		V11 = V12;


		double F0 = p02 * A0; // 1.2
		double F1 = p12 * A1;

		double dF = F0 - F1; // 1.3

		x = dx + v * ti; // 2.3

		v = dv + a * ti; // 2.2

		if (v == 0) {
			if (abs(dF) > Ftr_s) { // 2.0

				if (dF > 0) a = (dF - Ftr_s) / m; // 2.1
				else if (dF < 0) a = (dF + Ftr_s) / m;
			}
			else a = 0;
		}
		else if (x < 0) { x = 0; v = 0; }
		else if (x > l) { x = l; v = 0; }////////////////////////
		else if (v > 0) a = (dF - Ftr_d) / m;
		else if (v < 0) a = (dF + Ftr_d) / m;

		dx = x;
		dv = v;

		V02 = V0 + A0 * x; // 3.1
		V12 = V1 - A0 * x;
	}
	res.push_back(p01);
	res.push_back(p11);
	return res;
}



int x = 300;
int y = 250;
int dx = x;
int dy = y;
int velikost_x = 120;
int velikost_y = 60;

wxPanel* panel;
wxChoice* choice_dod;
wxChoice* choice_izb;
wxSlider* slider;

std::vector<std::vector<int>> seznam_valjev;

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* button_dod = new wxButton(panel, wxID_ANY, "Dodaj element", wxPoint(0, 40), wxSize(190, 100));
	wxButton* button_izb = new wxButton(panel, wxID_ANY, "Izbriši element", wxPoint(0, 160), wxSize(190, 100));
	wxButton* button_izb_vse = new wxButton(panel, wxID_ANY, "Izbriši vse", wxPoint(0, 280), wxSize(190, 100));

	wxArrayString choices;
	choices.Add("Element 0");
	choices.Add("Element 1");
	choices.Add("Element 2");
	choices.Add("Element 3");

	choice_dod = new wxChoice(panel, wxID_ANY, wxPoint(0, 20), wxSize(190, -1), choices, wxCB_SORT);
	choice_dod->SetSelection(0);
	choice_izb = new wxChoice(panel, wxID_ANY, wxPoint(0, 140), wxSize(190, -1), choices, wxCB_SORT);
	choice_izb->SetSelection(0);

	slider = new wxSlider(panel, wxID_ANY, 0, 0, 100, wxPoint(0, 400), wxSize(190, -1));

	panel->Bind(wxEVT_LEFT_DOWN, &MainFrame::OnMouseEvent, this);
	button_dod->Bind(wxEVT_BUTTON, &MainFrame::OnButtonDodClicked, this);
	button_izb->Bind(wxEVT_BUTTON, &MainFrame::OnButtonIzbClicked, this);
	button_izb_vse->Bind(wxEVT_BUTTON, &MainFrame::OnButtonIzbVseClicked, this);
	choice_dod->Bind(wxEVT_CHOICE, &MainFrame::OnChoicesClicked, this);
	slider->Bind(wxEVT_SLIDER, &MainFrame::OnSliderChanged, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(MainFrame::OnPaint));

	wxStatusBar* statusBar = CreateStatusBar();
	statusBar->SetDoubleBuffered(true); // da ne utripa izpis
}



void MainFrame::OnMouseEvent(wxMouseEvent& evt) {

	wxPoint mousePos = wxGetMousePosition(); // relativno na zaslon
	mousePos = this->ScreenToClient(mousePos); // pretvori zaslon in client (obratno pa "ClientToScreen")

	dx = round(static_cast<float>(mousePos.x) / 20) * 20;
	dy = round(static_cast<float>(mousePos.y) / 20) * 20;

	wxString message = wxString::Format("Mouse Event Detected! (x=%d y=%d)", dx, dy);
	wxLogStatus(message);
}

void MainFrame::OnButtonDodClicked(wxCommandEvent& evt) {

	if (dx != x || dy != y) {

		wxString naziv_gumb = wxString::Format("Gumb za element %d", choice_dod->GetSelection());

		//button_dodatek = new wxButton(panel, wxID_ANY, naziv_gumb, wxPoint(dx - velikost_x / 2, dy - velikost_y / 2), wxSize(velikost_x, velikost_y), wxWANTS_CHARS);
		//button_dodatek->Bind(wxEVT_BUTTON, &MainFrame::OnButtonDodatekClicked, this);
		
		x = dx;
		y = dy;
		seznam_valjev.push_back({ x, y});
		
	}
	else wxLogStatus("Izberi lokacijo z levik klikom");

	Refresh();
}

void MainFrame::OnButtonIzbClicked(wxCommandEvent& evt) {

	int n = choice_izb->GetSelection();

	if (n < seznam_valjev.size()) {

		seznam_valjev.erase(seznam_valjev.begin() + n);
		
		Refresh();
	}
	else wxLogStatus("Error");
}

void MainFrame::OnButtonIzbVseClicked(wxCommandEvent& evt) {

	seznam_valjev.clear();

	Refresh();
}

void MainFrame::OnChoicesClicked(wxCommandEvent& evt) {

	wxString naziv_element = wxString::Format("Item %d", choice_dod->GetSelection());

	wxLogStatus(naziv_element);

	Refresh();
}

void MainFrame::OnSliderChanged(wxCommandEvent& evt) {

	wxLogStatus("Slider change event");

	Refresh();
}

void MainFrame::OnPaint(wxPaintEvent& event) {

	wxPaintDC dc(this);


	int x_okno = 200; 
	int y_okno = 0;
	wxSize size = this->GetSize();
	int sirina_panel = size.x;
	int visina_panel = size.y;
	int sirina = sirina_panel - x_okno;
	int visina = visina_panel - y_okno;


	dc.SetPen(wxPen(wxColour(0,0,0), 1, wxPENSTYLE_SOLID));
	dc.DrawRectangle(x_okno, y_okno, sirina, visina);
	

	//- IZRISE TOCKE
	int ris_x = x_okno + 10;
	int ris_y = y_okno + 10;
	while (ris_x <= sirina + x_okno) {

		while (ris_y <= visina + y_okno) {

			dc.DrawPoint(ris_x, ris_y);

			ris_y = ris_y + 10;
		}
		ris_y = y_okno;
		ris_x = ris_x + 10;
	}
	//-

	
	//- IZRIS VALJEV
	int deb = 80;
	int vis = 50;
	float n = slider->GetValue();

	for (int i = 0; i < seznam_valjev.size(); i++) {
		dc.DrawRectangle(seznam_valjev[i][0], seznam_valjev[i][1], deb + 1, vis + 1); // Ohišje

		dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 1 + (n * 50 / slider->GetMax()), seznam_valjev[i][1], deb / 8 + 1, vis + 1); // Bat
		dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 2 + (n * 50 / slider->GetMax()), seznam_valjev[i][1] + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica
	
		dc.DrawText(wxString::Format("Item %d", i), seznam_valjev[i][0], seznam_valjev[i][1] - 16); // Ime

		dc.DrawText(wxString::Format("p1 = %g", izracun(n)[0]), seznam_valjev[i][0], seznam_valjev[i][1] + vis); // Tlak v levem
		dc.DrawText(wxString::Format("p2 = %g", izracun(n)[1]), seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis); // Tlak v desnem
	}
	//-
}



/*
dc.DrawRectangle(x, y, deb + 1, vis + 1); // Ohišje

dc.DrawRectangle(x + deb / 8 * 1 + n * 20, y, deb / 8 + 1, vis + 1); // Bat
dc.DrawRectangle(x + deb / 8 * 2 + n * 20, y + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica
*/


/*
povezava za dokumentacijo kontrol
https://docs.wxwidgets.org/3.0/group__group__class__ctrl.html

*/
