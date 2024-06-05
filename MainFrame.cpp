#include "MainFrame.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
//#include <algorithm>



std::vector<double> izracun(int n, int element) {
	std::vector<double> res;
	

	const double pi = 3.14159265358979; //- pi

	double g = 9.81 * 1000; //- gravitacijski pospesek [mm/s^2]
	double pok = 1.01325 / 10; //- atmosferski tlak [MPa]

	double Ftr_s = 100; //- Sila staticnega trenja znotraj valja [N]
	double Ftr_d = 20; //- Sila dinamicnega trenja znotraj valja [N]

	double koef_vzm = 2.4; //- Koeficient vzmeti [N/mm]

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

	double p01 = 0, p02 = 0, p11 = 0, p12 = 0;
	double V01, V02, V11, V12;
	double a = 0, v = 0, dv = 0, x = 0, dx = 0;
	double ti = .01; //- casovni korak [s]


	res.push_back(p0);
	res.push_back(p1);
	res.push_back(x);

	n = n; //- stevilo ponovitev simulacije

	if (n != 0) {
		switch (element) {

		case 0:

			//Simulacija 1.1.:
			//premik bata, zracno izoliran

			for (int i = 0; i < n; i++) {

				if (i == 0) { // 1.0
					p01 = p0;
					p11 = p1;
					V01 = V02 = V0;
					V11 = V12 = V1;
				}

				p01 = p01 * V01 / V02; // 1.1
				p11 = p11 * V11 / V12;


				V01 = V02; // 1.1.2
				V11 = V12;


				double F0 = p01 * A0; // 1.2
				double F1 = p11 * A1;
				double F2 = pok * (A0 - A1);

				double dF = F0 - F1 - F2; // 1.3

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
			res[0] = p01;
			res[1] = p11;
			res[2] = x;
			break;

		case 1:

			//Simulacija 2.1.:
			//premik bata

			for (int i = 0; i < n; i++) {

				if (i == 0) { // 1.0
					p01 = p0;
					p11 = p1;
					V01 = V02 = V0;
					V11 = V12 = V1;
				}

				p01 = p0; // 1.1
				p11 = pok;


				V01 = V02; // 1.1.2
				V11 = V12;


				double F0 = p01 * A0; // 1.2
				double F1 = p11 * A1;
				double F2 = pok * (A0 - A1);

				double dF = F0 - F1 - F2; // 1.3

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
			res[0] = p01;
			res[1] = p11;
			res[2] = x;
			break;

		case 2:

			//Simulacija 1.1.:
			//premik bata, zracno izoliran

			for (int i = 0; i < n; i++) {

				if (i == 0) { // 1.0
					p01 = p0;
					p11 = p1;
					V01 = V02 = V0;
					V11 = V12 = V1;
				}

				p01 = p0; // 1.1
				p11 = pok;


				V01 = V02; // 1.1.2
				V11 = V12;


				double F0 = p01 * A0; // 1.2
				double F1 = p11 * A1;
				double F2 = pok * (A0 - A1);
				double Fv = koef_vzm * x;// Koef. vzmeti * pomik

				double dF = F0 - F1 - F2 - Fv; // 1.3

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
			res[0] = p01;
			res[1] = p11;
			res[2] = x;
			break;

		default:
			wxLogStatus("Izbranega elementa ni v knjiznici");
			break;
		}
	}

	/*res.push_back(p01);
	res.push_back(p11);
	res.push_back(x);*/
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
wxSpinCtrl* spinCtrl;


std::vector<std::vector<int>> seznam_valjev;

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* button_dod = new wxButton(panel, wxID_ANY, "Dodaj element", wxPoint(0, 48), wxSize(190, -1));
	wxButton* button_izb = new wxButton(panel, wxID_ANY, "Izbriši element", wxPoint(0, 128), wxSize(190, -1));
	wxButton* button_izb_vse = new wxButton(panel, wxID_ANY, "Izbriši vse", wxPoint(0, 160), wxSize(190, -1));
	wxButton* predlog = new wxButton(panel, wxID_ANY, "Pregled elementov", wxPoint(0, 360), wxSize(190, -1));
	wxButton* simuliraj = new wxButton(panel, wxID_ANY, "Simuliraj", wxPoint(0, 440), wxSize(190, -1));
	wxButton* pomozno_okno = new wxButton(panel, wxID_ANY, "Pomozno okno", wxPoint(0, 250), wxSize(190, 75));

	wxArrayString choices;
	choices.Add("Izoliran valj");
	choices.Add("Diferencialni valj");
	choices.Add("Enosmerni valj");
	choices.Add("Vakumski prisesek");
	choices.Add("Pnevmatièno prijemalo");

	choice_dod = new wxChoice(panel, wxID_ANY, wxPoint(0, 20), wxSize(190, -1), choices/*, wxCB_SORT*/);
	choice_dod->SetSelection(0);
	spinCtrl = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(0, 100), wxSize(190, -1), wxSP_ARROW_KEYS|wxSP_WRAP);
	spinCtrl->SetRange(0, 0);
	slider = new wxSlider(panel, wxID_ANY, 0, 0, 1000, wxPoint(0, 400), wxSize(190, -1), wxSL_VALUE_LABEL);

	panel->Bind(wxEVT_LEFT_DOWN, &MainFrame::OnMouseEvent, this);
	panel->Bind(wxEVT_SIZE, &MainFrame::OnSizeChanged, this);
	button_dod->Bind(wxEVT_BUTTON, &MainFrame::OnButtonDodClicked, this);
	button_izb->Bind(wxEVT_BUTTON, &MainFrame::OnButtonIzbClicked, this);
	button_izb_vse->Bind(wxEVT_BUTTON, &MainFrame::OnButtonIzbVseClicked, this);
	predlog->Bind(wxEVT_BUTTON, &MainFrame::OnButtonPredVseClicked, this);
	simuliraj->Bind(wxEVT_BUTTON, &MainFrame::OnButtonSimClicked, this);
	pomozno_okno->Bind(wxEVT_BUTTON, &MainFrame::OnButtonPomClicked, this);
	choice_dod->Bind(wxEVT_CHOICE, &MainFrame::OnChoicesClicked, this);
	slider->Bind(wxEVT_SLIDER, &MainFrame::OnSliderChanged, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(MainFrame::OnPaint));

	wxStatusBar* statusBar = CreateStatusBar();
	statusBar->SetDoubleBuffered(true); // da ne utripa izpis
	panel->SetDoubleBuffered(true);
}



void MainFrame::OnMouseEvent(wxMouseEvent& evt) {

	wxPoint mousePos = wxGetMousePosition(); // relativno na zaslon
	mousePos = this->ScreenToClient(mousePos); // pretvori zaslon in client (obratno pa "ClientToScreen")

	dx = round(static_cast<float>(mousePos.x) / 20) * 20;
	dy = round(static_cast<float>(mousePos.y) / 20) * 20;

	wxString message = wxString::Format("Mouse Event Detected! (x=%d y=%d)", dx, dy);
	wxLogStatus(message);
}

void MainFrame::OnSizeChanged(wxSizeEvent& evt) {

	Refresh();
}

void MainFrame::OnButtonDodClicked(wxCommandEvent& evt) {

	if ((dx != x || dy != y) && dx >= 200) {

		wxString naziv_gumb = wxString::Format("Gumb za element %d", choice_dod->GetSelection());

		//button_dodatek = new wxButton(panel, wxID_ANY, naziv_gumb, wxPoint(dx - velikost_x / 2, dy - velikost_y / 2), wxSize(velikost_x, velikost_y), wxWANTS_CHARS);
		//button_dodatek->Bind(wxEVT_BUTTON, &MainFrame::OnButtonDodatekClicked, this);
		
		x = dx;
		y = dy;
		seznam_valjev.push_back({ x, y, choice_dod->GetSelection() });
		spinCtrl->SetRange(0, seznam_valjev.size());
	}
	else wxLogStatus("Izberi lokacijo z levik klikom");

	Refresh();
}

void MainFrame::OnButtonIzbClicked(wxCommandEvent& evt) {

	int n = spinCtrl->GetValue() - 1;

	if (n < seznam_valjev.size()) {

		seznam_valjev.erase(seznam_valjev.begin() + n);
		spinCtrl->SetRange(0, seznam_valjev.size());
		spinCtrl->SetValue(0);
		
		Refresh();
	}
	else wxLogStatus("Error");
}

void MainFrame::OnButtonIzbVseClicked(wxCommandEvent& evt) {

	seznam_valjev.clear();
	spinCtrl->SetRange(0, seznam_valjev.size());

	Refresh();
}

void MainFrame::OnButtonPredVseClicked(wxCommandEvent& evt) {

	seznam_valjev.clear();

	seznam_valjev.push_back({ 300, 100, 0 });
	seznam_valjev.push_back({ 300, 200, 1 });
	seznam_valjev.push_back({ 300, 300, 2 });

	spinCtrl->SetRange(0, seznam_valjev.size());

	Refresh();
}

void MainFrame::OnButtonSimClicked(wxCommandEvent& evt) {

	slider->SetValue(420);
	
	Refresh();
}

void MainFrame::OnButtonPomClicked(wxCommandEvent& evt) {

	PomoznoOkno* dodatnoOkno = new PomoznoOkno();
	dodatnoOkno->Show();
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


	int deb = 80;
	int vis = 50;
	float n = slider->GetValue();


	int x_okno = 200; 
	int y_okno = 0;
	wxSize size = this->GetSize();
	int sirina_panel = size.x;
	int visina_panel = size.y;
	int sirina = sirina_panel - x_okno;
	int visina = visina_panel - y_okno;
	int visina_prikaza = 120;

	//- IZRIS DELOVNEGA OBMOÈJA
	dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	
	dc.DrawText("Dodaj", wxPoint(0, 0));
	dc.DrawText("Izbrisi", wxPoint(0, 80));
	dc.DrawText("Simuliraj", wxPoint(0, 340));

	dc.DrawRectangle(x_okno, y_okno, sirina, visina);

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


	//- IZRIS ELEMENTA
	dc.DrawRectangle(0, visina_panel - visina_prikaza, x_okno+1, visina_prikaza);

	switch (choice_dod->GetSelection()) {

		case 0: // Izoliran bat
			dc.DrawRectangle(54, visina_panel - visina_prikaza + 36, deb + 1, vis + 1); // Ohišje
			dc.DrawRectangle(54 + deb / 8 * 1, visina_panel - visina_prikaza + 36, deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(54 + deb / 8 * 2, visina_panel - visina_prikaza + 36 + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica
			break;

		case 1: // Diferencialni bat
			dc.DrawRectangle(54, visina_panel - visina_prikaza + 36, deb + 1, vis + 1); // Ohišje
			dc.DrawLine(54, visina_panel - visina_prikaza + vis + 36, 54 + deb / 16 * 1 + 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1);
			dc.DrawLine(54 + deb / 8 * 1, visina_panel - visina_prikaza + vis + 36, 54 + deb / 16 * 1 - 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1);
			dc.DrawLine(54 + deb / 8 * 7, visina_panel - visina_prikaza + vis + 36, 54 + deb / 16 * 1 + deb / 8 * 7 + 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1);
			dc.DrawLine(54 + deb / 8 * 1 + deb / 8 * 7, visina_panel - visina_prikaza + vis + 36, 54 + deb / 16 * 1 + deb / 8 * 7 - 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1);
			dc.DrawRectangle(54 + deb / 8 * 1, visina_panel - visina_prikaza + 36, deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(54 + deb / 8 * 2, visina_panel - visina_prikaza + 36 + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica
			break;

		case 2: // 
			dc.DrawRectangle(54, visina_panel - visina_prikaza + 36, deb + 1, vis + 1); // Ohišje
			dc.DrawLine(54, visina_panel - visina_prikaza + vis + 36, 54 + deb / 16 * 1 + 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1);
			dc.DrawLine(54 + deb / 8 * 1, visina_panel - visina_prikaza + vis + 36, 54 + deb / 16 * 1 - 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1);
			dc.DrawLine(54 + deb / 8 * 7, visina_panel - visina_prikaza + vis + 36, 54 + deb / 16 * 1 + deb / 8 * 7 + 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1);
			dc.DrawLine(54 + deb / 8 * 1 + deb / 8 * 7, visina_panel - visina_prikaza + vis + 36, 54 + deb / 16 * 1 + deb / 8 * 7 - 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1);
			dc.DrawRectangle(54 + deb / 8 * 1, visina_panel - visina_prikaza + 36, deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(54 + deb / 8 * 2, visina_panel - visina_prikaza + 36 + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica
			dc.DrawLine(wxPoint(54 + deb / 8 * 2, visina_panel - visina_prikaza + 36 + vis), wxPoint(54 + deb / 8 * 2 + (deb - deb / 8 * 2) * 1 / 3, visina_panel - visina_prikaza + 36)); // Vzmet
			dc.DrawLine(wxPoint(54 + deb / 8 * 2 + (deb - deb / 8 * 2) * 1 / 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb / 8 * 2 + (deb - deb / 8 * 2) * 2 / 3, visina_panel - visina_prikaza + 36 + vis));
			dc.DrawLine(wxPoint(54 + deb / 8 * 2 + (deb - deb / 8 * 2) * 2 / 3, visina_panel - visina_prikaza + 36 + vis), wxPoint(54 + deb, visina_panel - visina_prikaza + 36));
			break;

		case 3: // 
			dc.DrawRectangle(54, visina_panel - visina_prikaza + 36, deb + 1, vis + 1); // Ohišje
			dc.DrawRectangle(54 + deb / 8 * 1, visina_panel - visina_prikaza + 36, deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(54 + deb / 8 * 2, visina_panel - visina_prikaza + 36 + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica
			break;

		default:
			break;
	}
	//-

	
	//- IZRIS VALJEV
	
	for (int i = 0; i < seznam_valjev.size(); i++) {

		switch (seznam_valjev[i][2]) {
		
		case 0:

			dc.DrawRectangle(seznam_valjev[i][0], seznam_valjev[i][1], deb + 1, vis + 1); // Ohišje

			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 1 + (50 * izracun(n, seznam_valjev[i][2])[2] / 250), seznam_valjev[i][1], deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 2 + (50 * izracun(n, seznam_valjev[i][2])[2] / 250), seznam_valjev[i][1] + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica

			dc.DrawText(wxString::Format("Element %d", i + 1), seznam_valjev[i][0], seznam_valjev[i][1] - 16); // Ime

			dc.DrawText(wxString::Format("p1 = %g", 10 * izracun(n, seznam_valjev[i][2])[0]), seznam_valjev[i][0], seznam_valjev[i][1] + vis); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g", 10 * izracun(n, seznam_valjev[i][2])[1]), seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis); // Tlak v desnem

			break;

		case 1:

			dc.DrawRectangle(seznam_valjev[i][0], seznam_valjev[i][1], deb + 1, vis + 1); // Ohišje

			dc.DrawLine(seznam_valjev[i][0], seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1); // "Ventil"
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1 + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);

			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 1 + (50 * izracun(n, seznam_valjev[i][2])[2] / 250), seznam_valjev[i][1], deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 2 + (50 * izracun(n, seznam_valjev[i][2])[2] / 250), seznam_valjev[i][1] + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica

			dc.DrawText(wxString::Format("Element %d", i + 1), seznam_valjev[i][0], seznam_valjev[i][1] - 16); // Ime

			dc.DrawText(wxString::Format("p1 = %g", 10 * izracun(n, seznam_valjev[i][2])[0]), seznam_valjev[i][0], seznam_valjev[i][1] + vis); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g", 10 * izracun(n, seznam_valjev[i][2])[1]), seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis); // Tlak v desnem
			
			break;

		case 2:

			dc.DrawRectangle(seznam_valjev[i][0], seznam_valjev[i][1], deb + 1, vis + 1); // Ohišje

			dc.DrawLine(seznam_valjev[i][0], seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1); // "Ventil"
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1 + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);

			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 1 + (50 * izracun(n, seznam_valjev[i][2])[2] / 250), seznam_valjev[i][1], deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 2 + (50 * izracun(n, seznam_valjev[i][2])[2] / 250), seznam_valjev[i][1] + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica

			dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * izracun(n, seznam_valjev[i][2])[2] / 250), seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * izracun(n, seznam_valjev[i][2])[2] / 250) + (deb - deb / 8 * 2 - izracun(n, seznam_valjev[i][2])[2] * 50 / 250) * 1 / 3, seznam_valjev[i][1])); // Vzmet
			dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * izracun(n, seznam_valjev[i][2])[2] / 250) + (deb - deb / 8 * 2 - izracun(n, seznam_valjev[i][2])[2] * 50 / 250) * 1 / 3, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * izracun(n, seznam_valjev[i][2])[2] / 250) + (deb - deb / 8 * 2 - izracun(n, seznam_valjev[i][2])[2] * 50 / 250) * 2 / 3, seznam_valjev[i][1] + vis));
			dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * izracun(n, seznam_valjev[i][2])[2] / 250) + (deb - deb / 8 * 2 - izracun(n, seznam_valjev[i][2])[2] * 50 / 250) * 2 / 3, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb, seznam_valjev[i][1]));

			dc.DrawText(wxString::Format("Element %d", i + 1), seznam_valjev[i][0], seznam_valjev[i][1] - 16); // Ime

			dc.DrawText(wxString::Format("p1 = %g", 10 * izracun(n, seznam_valjev[i][2])[0]), seznam_valjev[i][0], seznam_valjev[i][1] + vis); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g", 10 * izracun(n, seznam_valjev[i][2])[1]), seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis); // Tlak v desnem

			break;

			//dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 1 + (n * 50 / slider->GetMax()), seznam_valjev[i][1], deb / 8 + 1, vis + 1); // Bat prikaz
			//dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 2 + (n * 50 / slider->GetMax()), seznam_valjev[i][1] + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica prikaz
		
		default:
			break;
		}
	}
	//-
}

PomoznoOkno::PomoznoOkno() : wxFrame(nullptr, wxID_ANY, "title", wxPoint(0,0), wxSize(320,320)) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* button = new wxButton(panel, wxID_ANY, "Button", wxPoint(20, 20), wxSize(69, 69));

	button->Bind(wxEVT_BUTTON, &PomoznoOkno::OnButtonClicked, this);

	wxStatusBar* statusBar = CreateStatusBar();
	statusBar->SetDoubleBuffered(true);
}

void PomoznoOkno::OnButtonClicked(wxCommandEvent& evt) {

	wxLogStatus("Delaa");
}



/*
povezava za dokumentacijo kontrol
https://docs.wxwidgets.org/3.0/group__group__class__ctrl.html

*/