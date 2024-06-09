#include "MainFrame.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/time.h>
#include <wx/utils.h>
#include <stdlib.h>
#include <time.h>
#include <vector>



//last[batnica_leva, batnica_desna, vzmer_leva, vzmet_desna, tlak_izo_leva, tlak_izo_desna, zrak_prik_leva, zrak_prik_desna,
//     delovni_tlak, okoljski_tlak, zacetna_poz, hod_bata, masa_leva, masa_desna, batnica_premer_leva, batnica_premer_desna,
//	   bat_premer]
std::vector<double> izracun2(int n, int element, std::vector<double> last) {

	std::vector<double> res;


	const double pi = 3.14159265358979; //- pi

	double g = 9.81 * 1000; //- gravitacijski pospesek [mm/s^2]
	double pdel = last[8] / 10; //- delovni tlak [MPa]
	double pok = last[9] / 10; //- atmosferski tlak [MPa]

	double Ftr_s = 100; //- Sila staticnega trenja znotraj valja [N]
	double Ftr_d = 20; //- Sila dinamicnega trenja znotraj valja [N] /////////////////spremenit v koef_tr in izracunat z mase

	double koef_vzm_leva, koef_vzm_desna; //- Koeficient vzmeti [N/mm]
	if (last[2] < 0) koef_vzm_leva = 0.;
	else koef_vzm_leva = 2.4;
	if (last[3] < 0) koef_vzm_desna = 0.;
	else koef_vzm_desna = 2.4;

	//PODATKI ZA VALJ
	double D = last[16]; //- premer bata [mm]
	double d_l = last[14]; //- premer leve batnice [mm]
	double d_d = last[15]; //- premer desne batnice [mm]
	double l = last[11]; //- hod bata [mm]
	double x0 = last[10] / 100 * l; //- zaèetna pozicija bata [mm]
	double V_0_krmilni = 10000; //- krmilni volumen na zacetni strani [mm^3]
	double V_1_krmilni = 10000; //- krmilni volumen na koncni strani [mm^3]
	double m = .6; //- masa batnice in bata [kg]
	m = m + last[12] + last[13];

	double A0 = pi * pow(D, 2) / 4; //- površina celega bata [mm^2]
	double A1_l = pi * pow(d_l, 2) / 4; //- površina batnice [mm^2]
	double A2_l = A0 - A1_l; //- površina bata - batnice [mm^2]
	double A1_d = pi * pow(d_d, 2) / 4; //- površina batnice [mm^2]
	double A2_d = A0 - A1_d; //- površina bata - batnice [mm^2]

	double V_l, V_d;
	if (last[0] < 0) V_l = x0 * A0 + V_0_krmilni; //- zaèetni volumen na zacetni strani
	else V_l = x0 * A2_l + V_0_krmilni;
	if (last[1] < 0) V_d = (l - x0) * A0 + V_1_krmilni; //- zaèetni volumen na koncni strani
	else V_d = (l - x0) * A2_d + V_1_krmilni;

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
				V01 = V02 = V_l;
				V11 = V12 = V_d;
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
			else { F0 = A2_l * p01; F0b = A1_l * pok; }
			F0v = (l - x) * koef_vzm_leva;
			if (last[1] < 0) { F1 = A0 * p11; F1b = 0; }
			else { F1 = A2_d * p11; F1b = A1_d * pok; }
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


			if (last[0] < 0) V02 = V02 + A0 * dx; // 3.1
			else V02 = V02 + A2_l * dx;
			if (last[1] < 0) V12 = V12 - A0 * dx;
			else V12 = V12 - A2_d * dx;
		}
		res[0] = p01;
		res[1] = p11;
		res[2] = x;
	}

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
//wxChoice* choice_izb;
wxSlider* slider;
wxSpinCtrl* spinCtrl;

std::vector<std::vector<int>> seznam_valjev;
// seznam_valjev[x, y, element]
std::vector<std::vector<double>> seznam_lastnosti;
//last[batnica_leva, batnica_desna, vzmer_leva, vzmet_desna, tlak_izo_leva, tlak_izo_desna, zrak_prik_leva, zrak_prik_desna,
//     delovni_tlak, okoljski_tlak, zacetna_poz, hod_bata]
int oznacitev = -1;
bool sim = false;


MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* button_dod = new wxButton(panel, wxID_ANY, "Dodaj element", wxPoint(5, 48), wxSize(190, -1));
	wxButton* button_izb = new wxButton(panel, wxID_ANY, "Izbriši element", wxPoint(5, 128), wxSize(190, -1));
	wxButton* button_izb_vse = new wxButton(panel, wxID_ANY, "Izbriši vse", wxPoint(5, 160), wxSize(190, -1));
	wxButton* predlog = new wxButton(panel, wxID_ANY, "Pregled elementov", wxPoint(5, 360), wxSize(190, -1));
	wxButton* simuliraj = new wxButton(panel, wxID_ANY, "Simuliraj", wxPoint(5, 440), wxSize(190, 36));
	wxButton* pomozno_okno = new wxButton(panel, wxID_ANY, "Pomozno okno", wxPoint(5, 250), wxSize(190, 75));

	wxArrayString choices;
	choices.Add("Pnevmaticni valj");
	choices.Add("Vakumski prisesek");
	choices.Add("Pnevmaticno prijemalo");

	choice_dod = new wxChoice(panel, wxID_ANY, wxPoint(5, 20), wxSize(190, -1), choices/*, wxCB_SORT*/);
	choice_dod->SetSelection(0);
	spinCtrl = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(5, 100), wxSize(190, -1), wxSP_ARROW_KEYS|wxSP_WRAP);
	spinCtrl->SetRange(0, 0);
	slider = new wxSlider(panel, wxID_ANY, 0, 0, 1000, wxPoint(5, 400), wxSize(190, -1), wxSL_VALUE_LABEL);

	panel->Bind(wxEVT_LEFT_DOWN, &MainFrame::OnMouseEvent, this);
	panel->Bind(wxEVT_LEFT_DCLICK, &MainFrame::OnDoubleMouseEvent, this);
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


	seznam_lastnosti.push_back({ 1, 1, -1, -1, 1, 1, 1, -1, 6, 1, 10, 100, 0, 0, 10, 30, 40 }); // Prikaz elementov
	seznam_lastnosti.push_back({ -1, 1, -1, -1, -1, -1, 1, -1, 6, 1, 0, 250, 0, 0, 20, 20, 40 });
	seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 350, 0, 6, 20, 20, 40 });

	seznam_valjev.push_back({ 300, 100, 0 });
	seznam_valjev.push_back({ 300, 200, 0 });
	seznam_valjev.push_back({ 300, 300, 0 });

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

void MainFrame::OnDoubleMouseEvent(wxMouseEvent& evt) {

	wxPoint mousePos = wxGetMousePosition();
	mousePos = this->ScreenToClient(mousePos);

	if (oznacitev >= 0) {
		sim = false;
		//if (seznam_valjev[oznacitev][0] < mousePos.x && seznam_valjev[oznacitev][0] + 80 > mousePos.x && seznam_valjev[oznacitev][1] < mousePos.y && seznam_valjev[oznacitev][1] + 50 > mousePos.y) {
		PomoznoOkno* dodatnoOkno = new PomoznoOkno();
		dodatnoOkno->Show();
		//}
	}
	else wxLogStatus("Kliknite na element");
	wxLogStatus("DClick");
}

void MainFrame::OnSizeChanged(wxSizeEvent& evt) {

	Refresh();
}

void MainFrame::OnButtonDodClicked(wxCommandEvent& evt) {
	sim = false;

	if ((dx != x || dy != y) && dx >= 200) {

		wxString naziv_gumb = wxString::Format("Gumb za element %d", choice_dod->GetSelection());

		//button_dodatek = new wxButton(panel, wxID_ANY, naziv_gumb, wxPoint(dx - velikost_x / 2, dy - velikost_y / 2), wxSize(velikost_x, velikost_y), wxWANTS_CHARS);
		//button_dodatek->Bind(wxEVT_BUTTON, &MainFrame::OnButtonDodatekClicked, this);
		
		x = dx;
		y = dy;
		seznam_valjev.push_back({ x, y, choice_dod->GetSelection() });
		seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 250, 0, 0, 20, 20, 40 });
		spinCtrl->SetRange(0, seznam_valjev.size());
	}
	else wxLogStatus("Izberi lokacijo z levik klikom");

	Refresh();
}

void MainFrame::OnButtonIzbClicked(wxCommandEvent& evt) {
	sim = false;

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
	sim = false;

	seznam_valjev.clear();
	seznam_lastnosti.clear();
	spinCtrl->SetRange(0, seznam_valjev.size());

	Refresh();
}

void MainFrame::OnButtonPredVseClicked(wxCommandEvent& evt) {
	sim = false;

	seznam_valjev.clear();
	seznam_lastnosti.clear();

	seznam_lastnosti.push_back({ -1, 1, -1, -1, 1, 1, 1, -1, 6, 1, 10, 100, 0, 0, 20, 20, 40 });
	seznam_lastnosti.push_back({ -1, 1, -1, -1, -1, -1, 1, -1, 6, 1, 0, 250, 0, 0, 20, 20, 40 });
	seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 350, 0, 6, 20, 20, 40 });

	seznam_valjev.push_back({ 300, 100, 0 });
	seznam_valjev.push_back({ 300, 200, 0 });
	seznam_valjev.push_back({ 300, 300, 0 });

	spinCtrl->SetRange(0, seznam_valjev.size());

	Refresh();
}

void MainFrame::OnButtonSimClicked(wxCommandEvent& evt) {
	
	if (sim == false) sim = true;
	else sim = false;

	int i = slider->GetValue();
	while (sim && i <= 1000) {
		slider->SetValue(i);
		Refresh();
		wxYield();
		i++;
	}
	if (i >= 1000) sim = false;
}

void MainFrame::OnButtonPomClicked(wxCommandEvent& evt) {
	sim = false;

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
	sim = false;

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
	dc.DrawRectangle(wxPoint(0, visina_panel - visina_prikaza), wxSize(x_okno+1, visina_prikaza));

	switch (choice_dod->GetSelection()) {

		case 0: // Valj

			dc.DrawRectangle(wxPoint(54, visina_panel - visina_prikaza + 36), wxSize(deb + 1, vis + 1)); // Ohišje
			dc.DrawLine(wxPoint(54, visina_panel - visina_prikaza + vis + 36), wxPoint(54 + deb / 16 * 1 + 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1));
			dc.DrawLine(wxPoint(54 + deb / 8 * 1, visina_panel - visina_prikaza + vis + 36), wxPoint(54 + deb / 16 * 1 - 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1));
			dc.DrawLine(wxPoint(54 + deb / 8 * 7, visina_panel - visina_prikaza + vis + 36), wxPoint(54 + deb / 16 * 1 + deb / 8 * 7 + 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1));
			dc.DrawLine(wxPoint(54 + deb / 8 * 1 + deb / 8 * 7, visina_panel - visina_prikaza + vis + 36), wxPoint(54 + deb / 16 * 1 + deb / 8 * 7 - 1, visina_panel - visina_prikaza + vis + 36 - deb / 16 * 1 - 1));
			dc.DrawRectangle(wxPoint(54 + deb / 8 * 1, visina_panel - visina_prikaza + 36), wxSize(deb / 8 + 1, vis + 1)); // Bat
			dc.DrawRectangle(wxPoint(54 + deb / 8 * 2, visina_panel - visina_prikaza + 36 + vis / 5 * 2), wxSize(deb / 8 * 7 + 1, vis / 5 + 1)); // Batnica
			dc.DrawLine(wxPoint(54 + deb / 8 * 2, visina_panel - visina_prikaza + 36 + vis), wxPoint(54 + deb / 8 * 2 + (deb - deb / 8 * 2) * 1 / 3, visina_panel - visina_prikaza + 36)); // Vzmet
			dc.DrawLine(wxPoint(54 + deb / 8 * 2 + (deb - deb / 8 * 2) * 1 / 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb / 8 * 2 + (deb - deb / 8 * 2) * 2 / 3, visina_panel - visina_prikaza + 36 + vis));
			dc.DrawLine(wxPoint(54 + deb / 8 * 2 + (deb - deb / 8 * 2) * 2 / 3, visina_panel - visina_prikaza + 36 + vis), wxPoint(54 + deb, visina_panel - visina_prikaza + 36));
			dc.DrawText(wxString::Format("Element %d", seznam_valjev.size() + 1), wxPoint(54, visina_panel - visina_prikaza + 20));
			
			break;

		default:

			dc.DrawRectangle(wxPoint(54, visina_panel - visina_prikaza + 36), wxSize(deb + 1, vis + 1));
			dc.DrawText("Neznan \nelement", wxPoint(54 + 18, visina_panel - visina_prikaza + 46));
			dc.DrawText(wxString::Format("Element %d", seznam_valjev.size() + 1), wxPoint(54, visina_panel - visina_prikaza + 20));

			break;
	}
	//-


	//- OZNAÈITEV ELEMENTA
	if (oznacitev >= 0) {

		dc.SetPen(wxPen(wxColour(153, 153, 255), 1, wxPENSTYLE_LONG_DASH));
		dc.DrawRectangle(wxPoint(seznam_valjev[oznacitev][0] - 10, seznam_valjev[oznacitev][1] - 10), wxSize(100 + 1, 70 + 1));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}
	//-
	

	//- IZRIS VALJEV
	for (int i = 0; i < seznam_valjev.size(); i++) {

		std::vector<double> res;

		switch (seznam_valjev[i][2]) {
		
		case 0:
			res = izracun2(n, seznam_valjev[i][2], seznam_lastnosti[i]);

			dc.DrawRectangle(wxPoint(seznam_valjev[i][0], seznam_valjev[i][1]), wxSize(deb + 1, vis + 1)); // Ohišje

			if (seznam_lastnosti[i][4] < 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 16 * 1 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1)); // Priklucki
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 1, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 16 * 1 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1));
			}
			if (seznam_lastnosti[i][5] < 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 7, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 1 + deb / 8 * 7, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1));
			}
			
			dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb / 8 * 1 + res[2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1]), wxSize(deb / 8 + 1, vis + 1)); // Bat

			if (seznam_lastnosti[i][0] > 0) {
				dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 - deb + res[2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1] + vis / 5 * 2), wxSize(deb / 8 * 7 + 1, vis / 5 + 1)); // Batnica
			}
			if (seznam_lastnosti[i][1] > 0) {
				dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1] + vis / 5 * 2), wxSize(deb / 8 * 7 + 1, vis / 5 + 1));
			}

			if (seznam_lastnosti[i][2] > 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + (deb - deb / 8 * 7 + res[2] * 50 / seznam_lastnosti[i][11]) * 1 / 3, seznam_valjev[i][1])); // Vzmet
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + (deb - deb / 8 * 7 + res[2] * 50 / seznam_lastnosti[i][11]) * 1 / 3, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + (deb - deb / 8 * 7 + res[2] * 50 / seznam_lastnosti[i][11]) * 2 / 3, seznam_valjev[i][1] + vis));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + (deb - deb / 8 * 7 + res[2] * 50 / seznam_lastnosti[i][11]) * 2 / 3, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 8 * 1 + res[2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1]));
			}
			if (seznam_lastnosti[i][3] > 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[2] * 50 / seznam_lastnosti[i][11] + (deb - deb / 8 * 2 - res[2] * 50 / seznam_lastnosti[i][11]) * 1 / 3, seznam_valjev[i][1]));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[2] * 50 / seznam_lastnosti[i][11] + (deb - deb / 8 * 2 - res[2] * 50 / seznam_lastnosti[i][11]) * 1 / 3, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[2] * 50 / seznam_lastnosti[i][11] + (deb - deb / 8 * 2 - res[2] * 50 / seznam_lastnosti[i][11]) * 2 / 3, seznam_valjev[i][1] + vis));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[2] * 50 / seznam_lastnosti[i][11] + (deb - deb / 8 * 2 - res[2] * 50 / seznam_lastnosti[i][11]) * 2 / 3, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb, seznam_valjev[i][1]));
			}
			if (seznam_lastnosti[i][12] > 0){
				dc.DrawRoundedRectangle(wxPoint(seznam_valjev[i][0] - deb + deb / 8 * 2 + res[2] * 50 / seznam_lastnosti[i][11] - 40, seznam_valjev[i][1] + 5), wxSize(40 + 1, 40 + 1), 4);
				dc.DrawText(wxString::Format("%g kg", seznam_lastnosti[i][12]), wxPoint(seznam_valjev[i][0] - deb + deb / 8 * 2 + res[2] * 50 / seznam_lastnosti[i][11] - 40 + 5, seznam_valjev[i][1] + 5 + 13));
			}
			if (seznam_lastnosti[i][13] > 0) {
				dc.DrawRoundedRectangle(wxPoint(seznam_valjev[i][0] + deb + deb / 8 * 1 + res[2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1] + 5), wxSize(40 + 1, 40 + 1), 4);
				dc.DrawText(wxString::Format("%g kg", seznam_lastnosti[i][13]), wxPoint(seznam_valjev[i][0] + deb + deb / 8 * 1 + res[2] * 50 / seznam_lastnosti[i][11] + 5, seznam_valjev[i][1] + 5 + 13));
			}

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] - 16)); // Ime

			dc.DrawText(wxString::Format("p1 = %g", 10 * res[0]), wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + vis)); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g", 10 * res[1]), wxPoint(seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis)); // Tlak v desnem

			break;
		
		default:

			dc.DrawRectangle(wxPoint(seznam_valjev[i][0], seznam_valjev[i][1]), wxSize(deb + 1, vis + 1));
			dc.DrawText("Neznan \nelement", wxPoint(seznam_valjev[i][0] + 18, seznam_valjev[i][1] + 10));

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] - 16));

			break;
		}
	}
	//-
}



wxCheckListBox* levaLastnost;
wxCheckListBox* desnaLastnost;
wxRadioBox* levaTlak;
wxRadioBox* desnaTlak;
wxSpinCtrlDouble* levaMasa;
wxSpinCtrlDouble* desnaMasa;
wxSpinCtrlDouble* levaBatnica;
wxSpinCtrlDouble* desnaBatnica;
wxSpinCtrlDouble* delTlak;
wxSpinCtrlDouble* okTlak;
wxSpinCtrl* zacPoz;
wxSpinCtrl* hodBata;
wxSpinCtrl* premerBata;

PomoznoOkno::PomoznoOkno() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve elementov"), wxPoint(0,0), wxSize(420,540)) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
	wxSize size = this->GetSize();


	wxArrayString lastnosti;
	lastnosti.Add("Batnica");
	lastnosti.Add("Vzmet");
	lastnosti.Add("Tlaèno izoliran");
	lastnosti.Add("Masa");

	wxPoint pointLevaLastnost = wxPoint(10, 44);
	levaLastnost = new wxCheckListBox(panel, wxID_ANY, pointLevaLastnost, wxDefaultSize, lastnosti);
	if (seznam_lastnosti[oznacitev][0] > 0) levaLastnost->Check(0);
	if (seznam_lastnosti[oznacitev][2] > 0) levaLastnost->Check(1);
	if (seznam_lastnosti[oznacitev][4] > 0) levaLastnost->Check(2);
	if (seznam_lastnosti[oznacitev][12] > 0) levaLastnost->Check(3);
	wxPoint pointDesnaLastnost = wxPoint(size.x / 2, pointLevaLastnost.y);
	desnaLastnost = new wxCheckListBox(panel, wxID_ANY, pointDesnaLastnost, wxDefaultSize, lastnosti);
	if (seznam_lastnosti[oznacitev][1] > 0) desnaLastnost->Check(0);
	if (seznam_lastnosti[oznacitev][3] > 0) desnaLastnost->Check(1);
	if (seznam_lastnosti[oznacitev][5] > 0) desnaLastnost->Check(2);
	if (seznam_lastnosti[oznacitev][13] > 0) desnaLastnost->Check(3);

	wxArrayString tlak;
	tlak.Add("Delovni tlak");
	tlak.Add("Okoljski tlak");

	wxPoint pointLevaTlak = wxPoint(pointLevaLastnost.x, pointLevaLastnost.y + 100);
	levaTlak = new wxRadioBox(panel, wxID_ANY, "", pointLevaTlak, wxDefaultSize, tlak);
	if (seznam_lastnosti[oznacitev][6] > 0) levaTlak->SetSelection(0);
	else levaTlak->SetSelection(1);
	wxPoint pointDesnaTlak = wxPoint(size.x / 2, pointLevaTlak.y);
	desnaTlak = new wxRadioBox(panel, wxID_ANY, "", pointDesnaTlak, wxDefaultSize, tlak);
	if (seznam_lastnosti[oznacitev][7] > 0) desnaTlak->SetSelection(0);
	else desnaTlak->SetSelection(1);

	wxPoint pointLevaMasa = wxPoint(pointLevaTlak.x + 110, pointLevaTlak.y + 56);
	levaMasa = new wxSpinCtrlDouble(panel, wxID_ANY, "", pointLevaMasa, wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP | wxTE_PROCESS_ENTER, 0, 20, seznam_lastnosti[oznacitev][12], .1);
	wxPoint pointDesnaMasa = wxPoint(pointLevaMasa.x + size.x / 2 - 10, pointLevaMasa.y);
	desnaMasa = new wxSpinCtrlDouble(panel, wxID_ANY, "", pointDesnaMasa, wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP | wxTE_PROCESS_ENTER, 0, 20, seznam_lastnosti[oznacitev][13], .1);
	wxPoint pointLevaBatnica = wxPoint(pointLevaMasa.x, pointLevaMasa.y + 30);
	levaBatnica = new wxSpinCtrlDouble(panel, wxID_ANY, "", pointLevaBatnica, wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP | wxTE_PROCESS_ENTER, 0, seznam_lastnosti[oznacitev][16], seznam_lastnosti[oznacitev][14], .5);
	wxPoint pointDesnaBatnica = wxPoint(pointLevaBatnica.x + size.x / 2 - 10, pointLevaBatnica.y);
	desnaBatnica = new wxSpinCtrlDouble(panel, wxID_ANY, "", pointDesnaBatnica, wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP | wxTE_PROCESS_ENTER, 0, seznam_lastnosti[oznacitev][16], seznam_lastnosti[oznacitev][15], .5);

	wxPoint pointDelTlak = wxPoint(120, 280);
	delTlak = new wxSpinCtrlDouble(panel, wxID_ANY, "", pointDelTlak, wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 10, seznam_lastnosti[oznacitev][8], .1);
	wxPoint pointOkTlak = wxPoint(pointDelTlak.x, pointDelTlak.y + 30);
	okTlak = new wxSpinCtrlDouble(panel, wxID_ANY, "", pointOkTlak, wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 10, seznam_lastnosti[oznacitev][9], .1);
	wxPoint pointZacPoz = wxPoint(pointDelTlak.x + 200, pointDelTlak.y);
	zacPoz = new wxSpinCtrl(panel, wxID_ANY, "", pointZacPoz, wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP | wxTE_PROCESS_ENTER, 0, 100, seznam_lastnosti[oznacitev][10]);
	wxPoint pointHodBata = wxPoint(pointDelTlak.x + 200, pointDelTlak.y + 30);
	hodBata = new wxSpinCtrl(panel, wxID_ANY, "", pointHodBata, wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 1000, seznam_lastnosti[oznacitev][11]);
	wxPoint pointPremerBata = wxPoint(pointDelTlak.x + 200, pointDelTlak.y + 60);
	premerBata = new wxSpinCtrl(panel, wxID_ANY, "", pointPremerBata, wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP | wxTE_PROCESS_ENTER, 0, 200, seznam_lastnosti[oznacitev][16]);

	wxButton* button = new wxButton(panel, wxID_ANY, "Aplly", wxPoint(size.x / 2 - 40, size.y - 64), wxSize(80, 20));
	//wxButton* button2 = new wxButton(panel, wxID_ANY, "Try", wxPoint(size.x / 2 - 40, size.y - 86), wxSize(80, 20));


	button->Bind(wxEVT_BUTTON, &PomoznoOkno::OnButtonClicked, this);
	//button2->Bind(wxEVT_BUTTON, &MainFrame::OnButtonRefClicked, this);
	panel->Bind(wxEVT_SIZE, &PomoznoOkno::OnSizeChanged, this);
	levaLastnost->Bind(wxEVT_CHECKLISTBOX, &PomoznoOkno::OnNastavitveClicked, this);
	desnaLastnost->Bind(wxEVT_CHECKLISTBOX, &PomoznoOkno::OnNastavitveClicked, this);
	levaTlak->Bind(wxEVT_RADIOBOX, &PomoznoOkno::OnNastavitveClicked, this);
	desnaTlak->Bind(wxEVT_RADIOBOX, &PomoznoOkno::OnNastavitveClicked, this);
	levaMasa->Bind(wxEVT_SPINCTRLDOUBLE, &PomoznoOkno::OnNastavitveClicked, this);
	levaMasa->Bind(wxEVT_TEXT_ENTER, &PomoznoOkno::OnNastavitveClicked, this);
	desnaMasa->Bind(wxEVT_SPINCTRLDOUBLE, &PomoznoOkno::OnNastavitveClicked, this);
	desnaMasa->Bind(wxEVT_TEXT_ENTER, &PomoznoOkno::OnNastavitveClicked, this);
	zacPoz->Bind(wxEVT_SPINCTRL, &PomoznoOkno::OnNastavitveClicked, this);
	zacPoz->Bind(wxEVT_TEXT_ENTER, &PomoznoOkno::OnNastavitveClicked, this);
	levaBatnica->Bind(wxEVT_SPINCTRLDOUBLE, &PomoznoOkno::OnPremerClicked, this);
	levaBatnica->Bind(wxEVT_TEXT_ENTER, &PomoznoOkno::OnPremerClicked, this);
	desnaBatnica->Bind(wxEVT_SPINCTRLDOUBLE, &PomoznoOkno::OnPremerClicked, this);
	desnaBatnica->Bind(wxEVT_TEXT_ENTER, &PomoznoOkno::OnPremerClicked, this);
	premerBata->Bind(wxEVT_SPINCTRL, &PomoznoOkno::OnPremerClicked, this);
	premerBata->Bind(wxEVT_TEXT_ENTER, &PomoznoOkno::OnPremerClicked, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(PomoznoOkno::OnPaint));

	panel->SetDoubleBuffered(true);
}
//last[batnica_leva, batnica_desna, vzmer_leva, vzmet_desna, tlak_izo_leva, tlak_izo_desna, zrak_prik_leva, zrak_prik_desna,
//     delovni_tlak, okoljski_tlak, zacetna_poz, hod_bata, masa_leva, masa_desna, batnica_premer_leva, batnica_premer_desna,
//	   bat_premer]
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
		if (levaLastnost->IsChecked(0) == true && levaLastnost->IsChecked(3)) seznam_lastnosti[oznacitev][12] = levaMasa->GetValue();
		else seznam_lastnosti[oznacitev][12] = 0;
		if (desnaLastnost->IsChecked(0) == true && desnaLastnost->IsChecked(3)) seznam_lastnosti[oznacitev][13] = desnaMasa->GetValue();
		else seznam_lastnosti[oznacitev][13] = 0;
		seznam_lastnosti[oznacitev][14] = levaBatnica->GetValue();
		seznam_lastnosti[oznacitev][15] = desnaBatnica->GetValue();
		seznam_lastnosti[oznacitev][16] = premerBata->GetValue();
	}
	else wxLogStatus("Izberite element za spremembo nastavitev");

	Refresh(); 
}

/*void MainFrame::OnButtonRefClicked(wxCommandEvent& evt) {

	Refresh();
}*/

void PomoznoOkno::OnNastavitveClicked(wxCommandEvent& evt) {

	Refresh();
}

void PomoznoOkno::OnPremerClicked(wxCommandEvent& evt) {

	levaBatnica->SetRange(0, premerBata->GetValue());
	desnaBatnica->SetRange(0, premerBata->GetValue());
}

void PomoznoOkno::OnSizeChanged(wxSizeEvent& evt) {

	Refresh();
}

void PomoznoOkno::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);

	wxSize size = this->GetSize();

	dc.DrawLine(wxPoint(size.x / 2, 0), wxPoint(size.x / 2, 260));
	dc.DrawLine(wxPoint(0, 24), wxPoint(size.x, 24));
	dc.DrawLine(wxPoint(0, 260), wxPoint(size.x, 260));

	dc.DrawText("Leva stran valja:", wxPoint(10, 4));
	dc.DrawText("Lastnosti:", wxPoint(10, 30));
	dc.DrawText("Zraèni prikljuèek:", wxPoint(10, 130));
	dc.DrawText("Masa [kg] =", wxPoint(10, 200));
	dc.DrawText("Batnica [mm] =", wxPoint(10, 230));
	
	dc.DrawText("Desna stran valja:", wxPoint(size.x / 2 + 10, 4));
	dc.DrawText("Lastnosti:", wxPoint(size.x / 2 + 10, 30));
	dc.DrawText("Zraèni prikljuèek:", wxPoint(size.x / 2 + 10, 130));
	dc.DrawText("Masa [kg] =", wxPoint(size.x / 2 + 10, 200));
	dc.DrawText("Batnica [mm] =", wxPoint(size.x / 2 + 10, 230));
	
	dc.DrawText("Delovni tlak [bar] = ", wxPoint(10, 280));
	dc.DrawText("Okoljski tlak [bar] = ", wxPoint(10, 310));
	dc.DrawText("Zacetna pozicija [%] = ", wxPoint(200, 280));
	dc.DrawText("Hod bata [mm] = ", wxPoint(200, 310));
	dc.DrawText("Premer bata [mm] = ", wxPoint(200, 340));

	
	int deb = 80;
	int vis = 50;
	int x_okno = 200;
	int y_okno = 0;
	int sirina_panel = size.x;
	int visina_panel = size.y;
	int sirina = sirina_panel - x_okno;
	int visina = visina_panel - y_okno;
	int visina_prikaza = 160;
	int zamik = sirina_panel / 2 - deb / 2;
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
	if (levaLastnost->IsChecked(3) == true && levaLastnost->IsChecked(0) == true && levaMasa->GetValue() > 0) {
		dc.DrawRoundedRectangle(wxPoint(zamik - deb + deb / 8 * 2 + zacPom - 40, visina_panel - visina_prikaza + 36 + 5), wxSize(40 + 1, 40 + 1), 4);
		dc.DrawText(wxString::Format("%g kg", levaMasa->GetValue()), wxPoint(zamik - deb + deb / 8 * 2 + zacPom - 40 + 5, visina_panel - visina_prikaza + 36 + 5 + 13));
	}
	if (desnaLastnost->IsChecked(3) == true && desnaLastnost->IsChecked(0) == true && desnaMasa->GetValue() > 0) {
		dc.DrawRoundedRectangle(wxPoint(zamik + deb + deb / 8 * 1 + zacPom, visina_panel - visina_prikaza + 36 + 5), wxSize(40 + 1, 40 + 1), 4);
		dc.DrawText(wxString::Format("%g kg", desnaMasa->GetValue()), wxPoint(zamik + deb + deb / 8 * 1 + zacPom + 5, visina_panel - visina_prikaza + 36 + 5 + 13));
	}
}

/*
povezava za dokumentacijo kontrol
https://docs.wxwidgets.org/3.0/group__group__class__ctrl.html

*/