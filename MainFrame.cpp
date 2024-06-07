#include "MainFrame.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/time.h>
#include <wx/utils.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <iostream>
//#include <algorithm>



//last[batnica_leva, batnica_desna, vzmer_leva, vzmet_desna, tlak_izo_leva, tlak_izo_desna, zrak_prik_leva, zrak_prik_desna,
//     delovni_tlak, okoljski_tlak, zacetna_poz, hod_bata]
std::vector<double> izracun1(int n, int element, std::vector<double> last) {

	std::vector<double> res;


	const double pi = 3.14159265358979; //- pi

	double g = 9.81 * 1000; //- gravitacijski pospesek [mm/s^2]
	double pdel = last[8] / 10; //- delovni tlak [MPa]
	double pok = last[9] / 10; //- atmosferski tlak [MPa]
	
	double Ftr_s = 100; //- Sila staticnega trenja znotraj valja [N]
	double Ftr_d = 20; //- Sila dinamicnega trenja znotraj valja [N]

	double koef_vzm_leva, koef_vzm_desna; //- Koeficient vzmeti [N/mm]
	if (last[2] < 0) koef_vzm_leva = 0.;
	else koef_vzm_leva = 2.4;
	if (last[3] < 0) koef_vzm_desna = 0.;
	else koef_vzm_desna = 2.4;

	//PODATKI ZA DIFERENÈNI VALJ
	double D = 40; //- premer bata [mm] ////////////dodatek
	double d = 20; //- premer batnice [mm] ////////////dodatek
	double l = last[11]; //- hod bata [mm]
	double x0 = last[10] / 100 * l; //- zaèetna pozicija bata [mm]
	double V_0_krmilni = 10000; //- krmilni volumen na zacetni strani [mm^3]
	double V_1_krmilni = 10000; //- krmilni volumen na koncni strani [mm^3]
	double m = .6; //- masa batnice in bata [kg]

	double A0 = pi * pow(D, 2) / 4; //- površina celega bata [mm^2]
	double A1 = pi * pow(d, 2) / 4; //- površina batnice [mm^2]
	double A2 = A0 - A1; //- površina bata brez batnice [mm^2]

	double V0, V1;
	if (last[0] < 0) V0 = x0 * A0 + V_0_krmilni; //- zaèetni volumen na zacetni strani
	else V0 = x0 * A2 + V_0_krmilni;
	if (last[1] < 0) V1 = (l - x0) * A0 + V_1_krmilni; //- zaèetni volumen na koncni strani
	else V1 = (l - x0) * A2 + V_1_krmilni;

	//VSTOPNI PODATKI
	double p0, p1;
	if (last[6] < 0) p0 = pok; //- tlak v komori 0 [MPa]
	else p0 = pdel;
	if (last[7] < 0) p1 = pok; //- tlak v komori 0 [MPa]
	else p1 = pdel;

	double p01 = 0, p02 = 0, p11 = 0, p12 = 0;
	double V01, V02, V11, V12;
	double a = 0, v = 0, dv = 0, x = x0, dx = 0;
	double ti = .01; //- casovni korak [s]
	
	
	res.push_back(p0);
	res.push_back(p1);
	res.push_back(x0);

	n = n; //- stevilo ponovitev simulacije

	if (n != 0) {

		//Simulacija 1.1.:
		//premik bata, zracno izoliran

		for (int i = 0; i < n; i++) {

			if (i == 0) { // 1.0
				p01 = p0;
				p11 = p1;
				V01 = V02 = V0;
				V11 = V12 = V1;
			}


			if (last[4] < 0) { // 1.1
				if (last[6] < 0) p01 = pok;
				else p01 = pdel;
			}
			else p01 = p01 * V01 / V02;

			if (last[5] < 0) {
				if (last[7] < 0) p11 = pok;
				else p11 = pdel;
			}
			else p11 = p11 * V11 / V12;


			V01 = V02; // 1.1.2
			V11 = V12;


			double F0, F0b, F0v, F1, F1b, F1v, dF; // 1.2
			if (last[0] < 0) { F0 = A0 * p01; F0b = 0; }
			else { F0 = A2 * p01; F0b = A1 * pok; }
			F0v = (l - x) * koef_vzm_leva;
			if (last[1] < 0) { F1 = A0 * p11; F1b = 0; }
			else { F1 = A2 * p11; F1b = A1 * pok; }
			F1v = x * koef_vzm_desna;

			dF = F0 + F0b + F0v - F1 - F1b - F1v; // 1.3


			dx = v * ti;
			x = x + dx; // 2.3

			v = v + a * ti; // 2.2

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

			x = x;
			v = v;

			//V02 = V0 + A0 * x;
			//V12 = V1 - A0 * x;

			//V02 = V02 + A0 * dx;
			//V12 = V12 - A0 * dx;
			if (last[0] < 0) V02 = V02 + A0 * dx; // 3.1
			else V02 = V02 + A2 * dx;
			if (last[1] < 0) V12 = V12 - A0 * dx;
			else V12 = V12 - A2 * dx;
		}
		res[0] = p01;
		res[1] = p11;
		res[2] = x;
	}

	return res;
}

/*
std::vector<double> izracun0(int n, int element) {
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
			//wxLogStatus("Izbranega elementa ni v knjiznici");
			break;
		}
	}

	return res;
}
*/


int x = 300;
int y = 250;
int dx = x;
int dy = y;
int velikost_x = 120;
int velikost_y = 60;

wxPanel* panel;
wxChoice* choice_dod;
//wxChoice* choice_izb;
wxSlider* slider;
wxSpinCtrl* spinCtrl;

std::vector<std::vector<int>> seznam_valjev;
// seznam_valjev[x, y, element]
std::vector<std::vector<double>> seznam_lastnosti;
//last[batnica_leva, batnica_desna, vzmer_leva, vzmet_desna, tlak_izo_leva, tlak_izo_desna, zrak_prik_leva, zrak_prik_desna,
//     delovni_tlak, okoljski_tlak, zacetna_poz, hod_bata]
int oznacitev = -1;


MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* button_dod = new wxButton(panel, wxID_ANY, "Dodaj element", wxPoint(5, 48), wxSize(190, -1));
	wxButton* button_izb = new wxButton(panel, wxID_ANY, "Izbriši element", wxPoint(5, 128), wxSize(190, -1));
	wxButton* button_izb_vse = new wxButton(panel, wxID_ANY, "Izbriši vse", wxPoint(5, 160), wxSize(190, -1));
	wxButton* predlog = new wxButton(panel, wxID_ANY, "Pregled elementov", wxPoint(5, 360), wxSize(190, -1));
	wxButton* simuliraj = new wxButton(panel, wxID_ANY, "Simuliraj", wxPoint(5, 440), wxSize(190, 36));
	wxButton* pomozno_okno = new wxButton(panel, wxID_ANY, "Pomozno okno", wxPoint(5, 250), wxSize(190, 75));

	wxArrayString choices;
	choices.Add("Izoliran valj");
	choices.Add("Diferencialni valj");
	choices.Add("Enosmerni valj");
	choices.Add("Vakumski prisesek");
	choices.Add("Pnevmatièno prijemalo");

	choice_dod = new wxChoice(panel, wxID_ANY, wxPoint(5, 20), wxSize(190, -1), choices/*, wxCB_SORT*/);
	choice_dod->SetSelection(0);
	spinCtrl = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(5, 100), wxSize(190, -1), wxSP_ARROW_KEYS|wxSP_WRAP);
	spinCtrl->SetRange(0, 0);
	slider = new wxSlider(panel, wxID_ANY, 0, 0, 1000, wxPoint(5, 400), wxSize(190, -1), wxSL_VALUE_LABEL);

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

	/*seznam_valjev.push_back({300, 100, 0});
	seznam_valjev.push_back({ 300, 200, 1 });
	seznam_valjev.push_back({ 300, 300, 2 });*/

	seznam_lastnosti.push_back({ -1, 1, -1, -1, 1, 1, 1, -1, 6, 1, 0, 250 });
	seznam_lastnosti.push_back({ -1, 1, -1, -1, -1, -1, 1, -1, 6, 1, 0, 250 });
	seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 250 });

	seznam_valjev.push_back({ 500, 100, 3 });
	seznam_valjev.push_back({ 500, 200, 4 });
	seznam_valjev.push_back({ 500, 300, 5 });

	spinCtrl->SetRange(0, seznam_valjev.size());
}



void MainFrame::OnMouseEvent(wxMouseEvent& evt) {

	wxPoint mousePos = wxGetMousePosition(); // relativno na zaslon
	mousePos = this->ScreenToClient(mousePos); // pretvori zaslon in client (obratno pa "ClientToScreen")


	bool je = false;
	for (int i = 0; i < seznam_valjev.size(); i++) {

		if (seznam_valjev[i][0] < mousePos.x && seznam_valjev[i][0] + 80 > mousePos.x && seznam_valjev[i][1] < mousePos.y && seznam_valjev[i][1] + 50 > mousePos.y) {

			je = true;

			if (i == oznacitev) oznacitev = -1;
			else {

				oznacitev = i;

				choice_dod->SetSelection(seznam_valjev[i][2]);
			}

			break;
		}
	}

	if (je == false) {

		dx = round(static_cast<float>(mousePos.x) / 10) * 10;
		dy = round(static_cast<float>(mousePos.y) / 10) * 10;

		if (oznacitev >= 0 && dx >= 200) {
			seznam_valjev[oznacitev][0] = dx; 
			seznam_valjev[oznacitev][1] = dy;
		}

		oznacitev = -1;
	}


	wxString message = wxString::Format("Mouse Event Detected! (x=%d y=%d)", dx, dy);
	wxLogStatus(message);

	Refresh();
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
		seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 250 });
		spinCtrl->SetRange(0, seznam_valjev.size());
	}
	else wxLogStatus("Izberi lokacijo z levik klikom");

	Refresh();
}

void MainFrame::OnButtonIzbClicked(wxCommandEvent& evt) {

	int n = spinCtrl->GetValue() - 1;

	if (n < seznam_valjev.size()) {

		seznam_valjev.erase(seznam_valjev.begin() + n);
		seznam_lastnosti.erase(seznam_lastnosti.begin() + n);
		spinCtrl->SetRange(0, seznam_valjev.size());
		spinCtrl->SetValue(0);
		
		Refresh();
	}
	else wxLogStatus("Error");
}

void MainFrame::OnButtonIzbVseClicked(wxCommandEvent& evt) {

	seznam_valjev.clear();
	seznam_lastnosti.clear();
	spinCtrl->SetRange(0, seznam_valjev.size());

	Refresh();
}

void MainFrame::OnButtonPredVseClicked(wxCommandEvent& evt) {

	seznam_valjev.clear();
	seznam_lastnosti.clear();

	/*seznam_valjev.push_back({300, 100, 0});
	seznam_valjev.push_back({ 300, 200, 1 });
	seznam_valjev.push_back({ 300, 300, 2 });*/

	seznam_lastnosti.push_back({ -1, 1, -1, -1, 1, 1, 1, -1, 6, 1, 0, 250 });
	seznam_lastnosti.push_back({ -1, 1, -1, -1, -1, -1, 1, -1, 6, 1, 0, 250 });
	seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 250 });

	seznam_valjev.push_back({ 500, 100, 3 });
	seznam_valjev.push_back({ 500, 200, 4 });
	seznam_valjev.push_back({ 500, 300, 5 });

	spinCtrl->SetRange(0, seznam_valjev.size());

	Refresh();
}

void MainFrame::OnButtonSimClicked(wxCommandEvent& evt) {

	for (int i = 0; i <= 100; i++) {

		slider->SetValue(i * 10);
		Refresh();
		wxMilliSleep(10);
	}
	slider->SetValue(0);
	Refresh();
}

void MainFrame::OnButtonPomClicked(wxCommandEvent& evt) {

	if (oznacitev >= 0) {

		PomoznoOkno* dodatnoOkno = new PomoznoOkno();
		dodatnoOkno->Show();
	}
	else wxLogStatus("Izberite element");
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

	//- IZRIS DELOVNEGA OBMOCJA
	dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	
	dc.DrawText("Dodaj", wxPoint(5, 0));
	dc.DrawText("Izbrisi", wxPoint(5, 80));
	dc.DrawText("Simuliraj", wxPoint(5, 340));

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


	//- OZNAÈITEV ELEMENTA
	if (oznacitev >= 0) {

		dc.SetPen(wxPen(wxColour(153, 153, 255), 1, wxPENSTYLE_LONG_DASH));
		dc.DrawRectangle(seznam_valjev[oznacitev][0] - 10, seznam_valjev[oznacitev][1] - 10, 100 + 1, 70 + 1);
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}
	//-
	

	//- IZRIS VALJEV
	for (int i = 0; i < seznam_valjev.size(); i++) {

		std::vector<double> res;
		//std::vector<double> res = izracun0(n, seznam_valjev[i][2]);

		switch (seznam_valjev[i][2]) {
		
		/*case 0:

			dc.DrawRectangle(seznam_valjev[i][0], seznam_valjev[i][1], deb + 1, vis + 1); // Ohišje

			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 1 + (50 * res[2] / 250), seznam_valjev[i][1], deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250), seznam_valjev[i][1] + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica

			dc.DrawText(wxString::Format("Element %d", i + 1), seznam_valjev[i][0], seznam_valjev[i][1] - 16); // Ime

			dc.DrawText(wxString::Format("p1 = %g", 10 * res[0]), seznam_valjev[i][0], seznam_valjev[i][1] + vis); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g", 10 * res[1]), seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis); // Tlak v desnem

			break;

		case 1:

			dc.DrawRectangle(seznam_valjev[i][0], seznam_valjev[i][1], deb + 1, vis + 1); // Ohišje

			dc.DrawLine(seznam_valjev[i][0], seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1); // "Ventil"
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1 + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);

			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 1 + (50 * res[2] / 250), seznam_valjev[i][1], deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250), seznam_valjev[i][1] + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica

			dc.DrawText(wxString::Format("Element %d", i + 1), seznam_valjev[i][0], seznam_valjev[i][1] - 16); // Ime

			dc.DrawText(wxString::Format("p1 = %g", 10 * res[0]), seznam_valjev[i][0], seznam_valjev[i][1] + vis); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g", 10 * res[1]), seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis); // Tlak v desnem
			
			break;

		case 2:

			dc.DrawRectangle(seznam_valjev[i][0], seznam_valjev[i][1], deb + 1, vis + 1); // Ohišje

			dc.DrawLine(seznam_valjev[i][0], seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1); // "Ventil"
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1 + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);

			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 1 + (50 * res[2] / 250), seznam_valjev[i][1], deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250), seznam_valjev[i][1] + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica

			dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250), seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250) + (deb - deb / 8 * 2 - res[2] * 50 / 250) * 1 / 3, seznam_valjev[i][1])); // Vzmet
			dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250) + (deb - deb / 8 * 2 - res[2] * 50 / 250) * 1 / 3, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250) + (deb - deb / 8 * 2 - res[2] * 50 / 250) * 2 / 3, seznam_valjev[i][1] + vis));
			dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250) + (deb - deb / 8 * 2 - res[2] * 50 / 250) * 2 / 3, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb, seznam_valjev[i][1]));

			dc.DrawText(wxString::Format("Element %d", i + 1), seznam_valjev[i][0], seznam_valjev[i][1] - 16); // Ime

			dc.DrawText(wxString::Format("p1 = %g", 10 * res[0]), seznam_valjev[i][0], seznam_valjev[i][1] + vis); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g", 10 * res[1]), seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis); // Tlak v desnem

			break;*/

		case 3:
			res = izracun1(n, seznam_valjev[i][2], seznam_lastnosti[i]);

			dc.DrawRectangle(seznam_valjev[i][0], seznam_valjev[i][1], deb + 1, vis + 1); // Ohišje

			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 1 + (50 * res[2] / 250), seznam_valjev[i][1], deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250), seznam_valjev[i][1] + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica

			dc.DrawText(wxString::Format("Element %d", i + 1), seznam_valjev[i][0], seznam_valjev[i][1] - 16); // Ime

			dc.DrawText(wxString::Format("p1 = %g", 10 * res[0]), seznam_valjev[i][0], seznam_valjev[i][1] + vis); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g", 10 * res[1]), seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis); // Tlak v desnem

			break;

		case 4:
			res = izracun1(n, seznam_valjev[i][2], seznam_lastnosti[i]);

			dc.DrawRectangle(seznam_valjev[i][0], seznam_valjev[i][1], deb + 1, vis + 1); // Ohišje

			dc.DrawLine(seznam_valjev[i][0], seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1); // "Ventil"
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1 + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);

			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 1 + (50 * res[2] / 250), seznam_valjev[i][1], deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250), seznam_valjev[i][1] + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica

			dc.DrawText(wxString::Format("Element %d", i + 1), seznam_valjev[i][0], seznam_valjev[i][1] - 16); // Ime

			dc.DrawText(wxString::Format("p1 = %g", 10 * res[0]), seznam_valjev[i][0], seznam_valjev[i][1] + vis); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g", 10 * res[1]), seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis); // Tlak v desnem

			break;

		case 5:
			res = izracun1(n, seznam_valjev[i][2], seznam_lastnosti[i]);

			dc.DrawRectangle(seznam_valjev[i][0], seznam_valjev[i][1], deb + 1, vis + 1); // Ohišje

			dc.DrawLine(seznam_valjev[i][0], seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1); // "Ventil"
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);
			dc.DrawLine(seznam_valjev[i][0] + deb / 8 * 1 + deb / 8 * 7, seznam_valjev[i][1] + vis, seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1);

			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 1 + (50 * res[2] / 250), seznam_valjev[i][1], deb / 8 + 1, vis + 1); // Bat
			dc.DrawRectangle(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250), seznam_valjev[i][1] + vis / 5 * 2, deb / 8 * 7 + 1, vis / 5 + 1); // Batnica

			dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250), seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250) + (deb - deb / 8 * 2 - res[2] * 50 / 250) * 1 / 3, seznam_valjev[i][1])); // Vzmet
			dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250) + (deb - deb / 8 * 2 - res[2] * 50 / 250) * 1 / 3, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250) + (deb - deb / 8 * 2 - res[2] * 50 / 250) * 2 / 3, seznam_valjev[i][1] + vis));
			dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + (50 * res[2] / 250) + (deb - deb / 8 * 2 - res[2] * 50 / 250) * 2 / 3, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb, seznam_valjev[i][1]));

			dc.DrawText(wxString::Format("Element %d", i + 1), seznam_valjev[i][0], seznam_valjev[i][1] - 16); // Ime

			dc.DrawText(wxString::Format("p1 = %g", 10 * res[0]), seznam_valjev[i][0], seznam_valjev[i][1] + vis); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g", 10 * res[1]), seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis); // Tlak v desnem

			break;
		
		default:
			break;
		}
	}
	//-
}



wxCheckListBox* levaLastnost;
wxCheckListBox* desnaLastnost;
wxRadioBox* levaTlak;
wxRadioBox* desnaTlak;
wxSpinCtrlDouble* delTlak;
wxSpinCtrlDouble* okTlak;
wxSpinCtrl* zacPoz;
wxSpinCtrl* hodBata;

PomoznoOkno::PomoznoOkno() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve elementa"), wxPoint(0,0), wxSize(420,480)) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
	wxSize size = this->GetSize();

	wxButton* button = new wxButton(panel, wxID_ANY, "Aplly", wxPoint(size.x / 2 - 40, size.y - 64), wxSize(80, 20));

	wxArrayString lastnosti;
	lastnosti.Add("Batnica");
	lastnosti.Add("Vzmet");
	lastnosti.Add("Tlaèno izoliran");
	levaLastnost = new wxCheckListBox(panel, wxID_ANY, wxPoint(10, 44), wxDefaultSize, lastnosti);
	desnaLastnost = new wxCheckListBox(panel, wxID_ANY, wxPoint(size.x / 2 + 10, 44), wxDefaultSize, lastnosti);

	wxArrayString tlak;
	tlak.Add("Delovni tlak");
	tlak.Add("Okoljski tlak");
	levaTlak = new wxRadioBox(panel, wxID_ANY, "", wxPoint(10, 124), wxDefaultSize, tlak);
	desnaTlak = new wxRadioBox(panel, wxID_ANY, "", wxPoint(size.x / 2 + 10, 124), wxDefaultSize, tlak);

	delTlak = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(120, 180), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 10, 6, .1);
	okTlak = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(120, 210), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 10, 1.01325, .1);
	zacPoz = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(320, 180), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP | wxTE_PROCESS_ENTER, 0, 100, 0);
	hodBata = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(320, 210), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 1000, 250);

	button->Bind(wxEVT_BUTTON, &PomoznoOkno::OnButtonClicked, this);
	panel->Bind(wxEVT_SIZE, &PomoznoOkno::OnSizeChanged, this);
	levaLastnost->Bind(wxEVT_CHECKLISTBOX, &PomoznoOkno::OnNastavitveClicked, this);
	desnaLastnost->Bind(wxEVT_CHECKLISTBOX, &PomoznoOkno::OnNastavitveClicked, this);
	levaTlak->Bind(wxEVT_RADIOBOX, &PomoznoOkno::OnNastavitveClicked, this);
	desnaTlak->Bind(wxEVT_RADIOBOX, &PomoznoOkno::OnNastavitveClicked, this);
	zacPoz->Bind(wxEVT_SPINCTRL, &PomoznoOkno::OnNastavitveClicked, this);
	zacPoz->Bind(wxEVT_TEXT_ENTER, &PomoznoOkno::OnNastavitveClicked, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(PomoznoOkno::OnPaint));

	panel->SetDoubleBuffered(true);
}
//last[batnica_leva, batnica_desna, vzmer_leva, vzmet_desna, tlak_izo_leva, tlak_izo_desna, zrak_prik_leva, zrak_prik_desna,
//     delovni_tlak, okoljski_tlak, zacetna_poz, hod_bata]
void PomoznoOkno::OnButtonClicked(wxCommandEvent& evt) {

	if (!(oznacitev < 0)) {
		if (levaLastnost->IsChecked(0) == true) seznam_lastnosti[oznacitev][0] = 1;
		else seznam_lastnosti[oznacitev][0] = -1;
		if (desnaLastnost->IsChecked(0) == true) seznam_lastnosti[oznacitev][1] = 1;
		else seznam_lastnosti[oznacitev][1] = -1;
		if (levaLastnost->IsChecked(1) == true) seznam_lastnosti[oznacitev][2] = 1;
		else seznam_lastnosti[oznacitev][2] = -1;
		if (desnaLastnost->IsChecked(1) == true) seznam_lastnosti[oznacitev][3] = 1;
		else seznam_lastnosti[oznacitev][3] = -1;
		if (levaLastnost->IsChecked(2) == true) seznam_lastnosti[oznacitev][4] = 1;
		else seznam_lastnosti[oznacitev][4] = -1;
		if (desnaLastnost->IsChecked(2) == true) seznam_lastnosti[oznacitev][5] = 1;
		else seznam_lastnosti[oznacitev][5] = -1;
		if (levaTlak->GetSelection() == 0) seznam_lastnosti[oznacitev][6] = 1;
		else seznam_lastnosti[oznacitev][6] = -1;
		if (desnaTlak->GetSelection() == 0) seznam_lastnosti[oznacitev][7] = 1;
		else seznam_lastnosti[oznacitev][7] = -1;
		seznam_lastnosti[oznacitev][8] = delTlak->GetValue();
		seznam_lastnosti[oznacitev][9] = okTlak->GetValue();
		seznam_lastnosti[oznacitev][10] = zacPoz->GetValue();
		seznam_lastnosti[oznacitev][11] = hodBata->GetValue();
	}
	else wxLogStatus("Izberite element za spremembo nastavitev");

	Refresh(); 
}

void PomoznoOkno::OnNastavitveClicked(wxCommandEvent& evt) {

	Refresh();
}

void PomoznoOkno::OnSizeChanged(wxSizeEvent& evt) {

	Refresh();
}

void PomoznoOkno::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);

	wxSize size = this->GetSize();

	dc.DrawLine(wxPoint(size.x / 2, 0), wxPoint(size.x / 2, 176));
	dc.DrawLine(wxPoint(0, 24), wxPoint(size.x, 24));
	dc.DrawLine(wxPoint(0, 176), wxPoint(size.x, 176));

	dc.DrawText("Leva stran valja:", wxPoint(10, 4));
	dc.DrawText("Lastnosti:", wxPoint(10, 30));
	dc.DrawText("Zraèni prikljuèek:", wxPoint(10, 110));
	
	dc.DrawText("Desna stran valja:", wxPoint(size.x / 2 + 10, 4));
	dc.DrawText("Lastnosti:", wxPoint(size.x / 2 + 10, 30));
	dc.DrawText("Zraèni prikljuèek:", wxPoint(size.x / 2 + 10, 110));
	
	dc.DrawText("Delovni tlak [bar] = ", wxPoint(10, 180));
	dc.DrawText("Okoljski tlak [bar] = ", wxPoint(10, 210));
	dc.DrawText("Zacetna pozicija [%] = ", wxPoint(200, 180));
	dc.DrawText("Hod bata [mm] = ", wxPoint(200, 210));

	
	int deb = 80;
	int vis = 50;
	int x_okno = 200;
	int y_okno = 0;
	int sirina_panel = size.x;
	int visina_panel = size.y;
	int sirina = sirina_panel - x_okno;
	int visina = visina_panel - y_okno;
	int visina_prikaza = 160;
	int zamik = sirina_panel / 2 - deb / 8;
	int zacPom = zacPoz->GetValue() * (deb * 5 / 8) / 100;


	dc.DrawRectangle(wxPoint(zamik, visina_panel - visina_prikaza + 36), wxSize(deb + 1, vis + 1)); // Ohišje

	if (levaLastnost->IsChecked(2) == false) {
		dc.DrawLine(wxPoint(zamik, visina_panel - visina_prikaza + vis + 36), wxPoint(zamik + deb / 16 * 1 + 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1)); // Priklucki
		dc.DrawLine(wxPoint(zamik + deb / 8 * 1, visina_panel - visina_prikaza + vis + 36), wxPoint(zamik + deb / 16 * 1 - 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1));
	}
	if (desnaLastnost->IsChecked(2) == false) {
		dc.DrawLine(wxPoint(zamik + deb / 8 * 7, visina_panel - visina_prikaza + vis + 36), wxPoint(zamik + deb / 16 * 1 + deb / 8 * 7 + 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1));
		dc.DrawLine(wxPoint(zamik + deb / 8 * 1 + deb / 8 * 7, visina_panel - visina_prikaza + vis + 36), wxPoint(zamik + deb / 16 * 1 + deb / 8 * 7 - 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1));
	}

	dc.DrawRectangle(wxPoint(zamik + deb / 8 * 1 + zacPom, visina_panel - visina_prikaza + 36), wxSize(deb / 8 + 1, vis + 1)); // Bat

	if (levaLastnost->IsChecked(0) == true) {
		dc.DrawRectangle(wxPoint(zamik + deb / 8 * 2 - deb + zacPom, visina_panel - visina_prikaza + 36 + vis / 5 * 2), wxSize(deb / 8 * 7 + 1, vis / 5 + 1)); // Batnica
	}
	if (desnaLastnost->IsChecked(0) == true) {
		dc.DrawRectangle(wxPoint(zamik + deb / 8 * 2 + zacPom, visina_panel - visina_prikaza + 36 + vis / 5 * 2), wxSize(deb / 8 * 7 + 1, vis / 5 + 1));
	}

	if (levaLastnost->IsChecked(1) == true) {
		dc.DrawLine(wxPoint(zamik, visina_panel - visina_prikaza + 36 + vis), wxPoint(zamik + (deb - deb / 8 * 7 + zacPom) * 1 / 3, visina_panel - visina_prikaza + 36)); // Vzmet
		dc.DrawLine(wxPoint(zamik + (deb - deb / 8 * 7 + zacPom) * 1 / 3, visina_panel - visina_prikaza + 36), wxPoint(zamik + (deb - deb / 8 * 7 + zacPom) * 2 / 3, visina_panel - visina_prikaza + 36 + vis));
		dc.DrawLine(wxPoint(zamik + (deb - deb / 8 * 7 + zacPom) * 2 / 3, visina_panel - visina_prikaza + 36 + vis), wxPoint(zamik + deb / 8 * 1 + zacPom, visina_panel - visina_prikaza + 36));
	}
	if (desnaLastnost->IsChecked(1) == true) {
		dc.DrawLine(wxPoint(zamik + deb / 8 * 2 + zacPom, visina_panel - visina_prikaza + 36 + vis), wxPoint(zamik + deb / 8 * 2 + zacPom + (deb - deb / 8 * 2 - zacPom) * 1 / 3, visina_panel - visina_prikaza + 36));
		dc.DrawLine(wxPoint(zamik + deb / 8 * 2 + zacPom + (deb - deb / 8 * 2 - zacPom) * 1 / 3, visina_panel - visina_prikaza + 36), wxPoint(zamik + deb / 8 * 2 + zacPom + (deb - deb / 8 * 2 - zacPom) * 2 / 3, visina_panel - visina_prikaza + 36 + vis));
		dc.DrawLine(wxPoint(zamik + deb / 8 * 2 + zacPom + (deb - deb / 8 * 2 - zacPom) * 2 / 3, visina_panel - visina_prikaza + 36 + vis), wxPoint(zamik + deb, visina_panel - visina_prikaza + 36));
	}
}

/*
povezava za dokumentacijo kontrol
https://docs.wxwidgets.org/3.0/group__group__class__ctrl.html

*/