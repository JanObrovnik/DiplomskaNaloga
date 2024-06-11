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
//res[p_l, p_d, x, v, a, V_l, V_d, n]
std::vector<double> izracun3(int n, int element, std::vector<double> last, std::vector<double> res) {


	const double pi = 3.14159265358979; //- pi

	double g = 9.81 * 1000; //- gravitacijski pospesek [mm/s^2]
	double pdel = last[8] / 10; //- delovni tlak [MPa]
	double pok = last[9] / 10; //- atmosferski tlak [MPa]

	double ti = .001; //- casovni korak [s]

	double koef_tr_st = .8; //- Koeficient staticnega trenja [/]
	double koef_tr_din = .6; //- Koeficient dinamicnega trenja [/]

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
	double m = .8 / 1000; //- masa batnice in bata [t] //////////////////////////// m = 2. / 1000 -> pika zlo pomembna
	m = m + last[12] / 1000 + last[13] / 1000;

	double Ftr_s = m * g * koef_tr_st; //- Sila staticnega trenja znotraj valja [N]
	double Ftr_d = m * g * koef_tr_din; //- Sila dinamicnega trenja znotraj valja [N]

	//Ftr_s = 100; Ftr_d = 20;

	double A0 = pi * pow(D, 2) / 4; //- površina celega bata [mm^2]
	double A1_l = pi * pow(d_l, 2) / 4; //- površina batnice [mm^2]
	double A2_l = A0 - A1_l; //- površina bata - batnice [mm^2]
	double A1_d = pi * pow(d_d, 2) / 4;
	double A2_d = A0 - A1_d;
	double A_l, A_d;
	if (last[0] < 0) A_l = A0;
	else A_l = A2_l;
	if (last[1] < 0) A_d = A0;
	else A_d = A2_d;

	double V_l0 = x0 * A0 + V_0_krmilni; //- zaèetni volumen na levi strani
	double V_d0 = (l - x0) * A0 + V_1_krmilni; //- zaèetni volumen na desni strani


	double p_l = res[0]/10;
	double p_d = res[1]/10;

	double x = res[2];
	double v = res[3];
	double a = res[4];

	double V_l, V_d;
	if (n == 0) {
		V_l = V_l0;
		V_d = V_d0;
		
	}
	else {
		V_l = res[5];
		V_d = res[6];
	}
	double V_l2 = V_l;
	double V_d2 = V_d;

	if (n > 0) {
		double F0, F0b, F0v, F1, F1b, F1v, dF; // 1.2

		F0 = A_l * p_l;
		F0b = (A0 - A_l) * pok;
		F0v = (l - x) * koef_vzm_leva;
		F1 = A_d * p_d;
		F1b = (A0 - A_d) * pok;
		F1v = x * koef_vzm_desna;

		dF = F0 + F0b + F0v - F1 - F1b - F1v;


		if (v == 0) {

			if (abs(dF) > Ftr_s) { // 2.0

				if (dF > 0) a = (dF - Ftr_s) / m; // 2.1
				else if (dF < 0) a = (dF + Ftr_s) / m;
			}
			else a = 0;
		}
		////////////////////////
		else if (v > 0) a = (dF - Ftr_d) / m;
		else if (v < 0) a = (dF + Ftr_d) / m;

		v = v + a * ti;

		if (abs(v) < .1) v = 0;

		double dx = v * ti;
		if ((x + dx) <= 0) {
			dx = x - 0;
			x = 0;
			v = 0;
		}
		else if ((x + dx) >= l) {
			dx = l - x;
			x = l;
			v = 0;
		}
		else {
			x = x + dx;
		}

		V_l = V_l + A_l * dx;
		V_d = V_d - A_d * dx;

		if (last[4] > 0) p_l = p_l * V_l2 / V_l;
		if (last[5] > 0) p_d = p_d * V_d2 / V_d;
	}

	res[0] = p_l*10;
	res[1] = p_d*10;
	res[2] = x;
	res[3] = v;
	res[4] = a;
	res[5] = V_l;
	res[6] = V_d;
	res[7] = n;

	return res;
}

std::vector<double> ventil(int element, std::vector<double> pi, double p_l, double p_d, double p_del, double p_ok) {

	if (p_d > p_l) pi[0] = 0; //////////////// spremenit if() v silo, da lah se vzmet in gumb uopostevas
	else if (p_l > p_d) pi[0] = 1;

	int poz = static_cast<int> (pi[0]);

	switch (element) {

	case 0: // ventil 3/2:

		switch (poz) {

		case 0: // celica 0 (desna):

			pi[2] = pi[3];

			break;

		case 1: // celica 1 (leva):

			pi[2] = pi[1];

			break;

		default:
			break;
		}
		break;


	case 1: // ventil 4/2:

		switch (poz) {

		case 0: // celica 0 (desna):

			pi[2] = pi[1];
			pi[4] = pi[3];

			break;

		case 1: // celica 1 (leva):

			pi[2] = pi[3];
			pi[4] = pi[1];

			break;

		default:
			break;
		}

		break;


	case 2: // ventil 5/2:

		switch (poz) {

		case 0: // celica 0 (desna):

			pi[2] = pi[1];
			pi[4] = pi[5];

			break;

		case 1: // celica 1 (leva):
			
			pi[2] = pi[3];
			pi[4] = pi[1];
			
			break;

		default:
			break;
		}

		break;


	default:
		break;
	}

	return pi;
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
//wxSpinCtrl* spinCtrl;

std::vector<std::vector<int>> seznam_valjev;
// seznam_valjev[x, y, element]
std::vector<std::vector<double>> seznam_lastnosti;
//last[batnica_leva, batnica_desna, vzmer_leva, vzmet_desna, tlak_izo_leva, tlak_izo_desna, zrak_prik_leva, zrak_prik_desna,
//     delovni_tlak, okoljski_tlak, zacetna_poz, hod_bata]
std::vector<std::vector<double>> res_reset;
std::vector<std::vector<double>> res;
//res[p_l, p_d, x]
std::vector<std::vector<std::vector<double>>> graf; // za risanje grafa

int oznacitev = -1;
bool sim = false;


MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* button_dod = new wxButton(panel, wxID_ANY, "Dodaj element", wxPoint(5, 48), wxSize(190, -1));
	wxButton* button_izb = new wxButton(panel, wxID_ANY, "Izbrisi element", wxPoint(5, 100), wxSize(190, -1));
	wxButton* button_izb_vse = new wxButton(panel, wxID_ANY, "Izbrisi vse", wxPoint(5, 130), wxSize(190, -1));
	wxButton* predlog = new wxButton(panel, wxID_ANY, "Pregled elementov", wxPoint(5, 360), wxSize(190, -1));
	wxButton* simuliraj = new wxButton(panel, wxID_ANY, "Simuliraj", wxPoint(5, 440), wxSize(190, 36));
	wxButton* pomozno_okno = new wxButton(panel, wxID_ANY, "Pomozno okno", wxPoint(5, 250), wxSize(190, 75));

	wxArrayString choices;
	choices.Add("Pnevmaticni valj");
	choices.Add("Vakumski prisesek");
	choices.Add("Pnevmaticno prijemalo");
	choices.Add("3/2 ventil");
	choices.Add("4/2 ventil");
	choices.Add("5/2 ventil");


	choice_dod = new wxChoice(panel, wxID_ANY, wxPoint(5, 20), wxSize(190, -1), choices/*, wxCB_SORT*/);
	choice_dod->SetSelection(0);
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


	seznam_lastnosti.push_back({ 1, 1, -1, -1, 1, 1, 1, -1, 6, 1, 0, 100, 0, 0, 10, 30, 40, 1, -1, -1 }); // Prikaz elementov
	seznam_lastnosti.push_back({ -1, 1, -1, -1, -1, -1, 1, -1, 6, 1, 0, 250, 0, 0, 20, 20, 40, 1, -1, -1 });
	seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 350, 0, 6, 20, 20, 40, 1, -1, -1 });

	res_reset.push_back({ 6., 1., 0, 0, 0, 0, 0, 0 });
	res_reset.push_back({ 6., 1., 0, 0, 0, 0, 0, 0 });
	res_reset.push_back({ 6., 1., 0, 0, 0, 0, 0, 0 });
	res = res_reset;

	seznam_valjev.push_back({ 300, 100, 0 });
	seznam_valjev.push_back({ 300, 200, 0 });
	seznam_valjev.push_back({ 300, 300, 0 });
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

		PomoznoOkno* dodatnoOkno = new PomoznoOkno();
		dodatnoOkno->Show();

		res = res_reset;
		slider->SetValue(0);
		graf.clear();

		Refresh();
	}
	else wxLogStatus("Kliknite na element");
}

void MainFrame::OnSizeChanged(wxSizeEvent& evt) {

	Refresh();
}

void MainFrame::OnButtonDodClicked(wxCommandEvent& evt) {
	sim = false;

	if ((dx != x || dy != y) && dx >= 200) {

		x = dx;
		y = dy;
		seznam_valjev.push_back({ x, y, choice_dod->GetSelection() });
		seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 250, 0, 0, 20, 20, 40, 1, -1, -1 });
		res_reset.push_back({ 6., 1., 0, 0, 0, 0, 0, 0 });
		res = res_reset;
		graf.clear();
		slider->SetValue(0);
	}
	else wxLogStatus("Izberi lokacijo z levik klikom");

	Refresh();
}

void MainFrame::OnButtonIzbClicked(wxCommandEvent& evt) {
	sim = false;

	if (oznacitev >= 0) {

		seznam_valjev.erase(seznam_valjev.begin() + oznacitev);
		seznam_lastnosti.erase(seznam_lastnosti.begin() + oznacitev);
		res_reset.erase(res_reset.begin() + oznacitev);
		res = res_reset;
		graf.clear();
		slider->SetValue(0);

		oznacitev = -1;
	}
	else wxLogStatus("Izberite element za izbrisat");

	Refresh();
}

void MainFrame::OnButtonIzbVseClicked(wxCommandEvent& evt) {
	sim = false;

	seznam_valjev.clear();
	seznam_lastnosti.clear();
	res_reset.clear();
	res.clear();
	graf.clear();
	slider->SetValue(0);

	Refresh();
}

void MainFrame::OnButtonPredVseClicked(wxCommandEvent& evt) {
	sim = false;

	seznam_valjev.clear();
	seznam_lastnosti.clear();
	res_reset.clear();
	res.clear();
	graf.clear();

	seznam_lastnosti.push_back({ 1, 1, -1, -1, 1, 1, 1, -1, 6, 1, 0, 100, 0, 0, 10, 30, 40, 1, -1, -1 });
	seznam_lastnosti.push_back({ -1, 1, -1, -1, -1, -1, 1, -1, 6, 1, 0, 250, 0, 0, 20, 20, 40, 1, -1, -1 });
	seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 350, 0, 6, 20, 20, 40, 1, -1, -1 });

	res_reset.push_back({ 6., 1., 0, 0, 0, 0, 0, 0 });
	res_reset.push_back({ 6., 1., 0, 0, 0, 0, 0, 0 });
	res_reset.push_back({ 6., 1., 0, 0, 0, 0, 0, 0 });
	res = res_reset;

	seznam_valjev.push_back({ 300, 100, 0 });
	seznam_valjev.push_back({ 300, 200, 0 });
	seznam_valjev.push_back({ 300, 300, 0 });

	slider->SetValue(0);

	Refresh();
}

void MainFrame::OnButtonSimClicked(wxCommandEvent& evt) {

	if (slider->GetValue() >= 1000) {
		slider->SetValue(0);
		res = res_reset;
		graf.clear();
		Refresh();
	}

	int i = slider->GetValue();

	if (sim == false) sim = true;
	else sim = false;

	while (sim && i <= 1000) {
		slider->SetValue(i);
		Refresh();
		wxYield();
		i++;
	}
	if (i >= 1000) {
		sim = false;
	}
}

void MainFrame::OnButtonPomClicked(wxCommandEvent& evt) {
	sim = false;

	if (oznacitev >= 0) {

		PomoznoOkno* dodatnoOkno = new PomoznoOkno();
		dodatnoOkno->Show();

		res = res_reset;
		slider->SetValue(0);
		graf.clear();

		Refresh();
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
	int deb_ven = 48;
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
	dc.DrawRectangle(wxPoint(0, visina_panel - visina_prikaza), wxSize(x_okno + 1, visina_prikaza));

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

	case 2:
		deb = 64;
		dc.DrawRectangle(wxPoint(54, visina_panel - visina_prikaza + 36), wxSize(deb + 1, vis + 1));
		dc.DrawRectangle(wxPoint(54 + deb / 4 * 3, visina_panel - visina_prikaza + 36 + vis / 10), wxSize(deb / 2 + 1, vis / 5 + 1));
		dc.DrawRectangle(wxPoint(54 + deb / 4 * 3, visina_panel - visina_prikaza + 36 + vis / 10 * 7), wxSize(deb / 2 + 1, vis / 5 + 1));
		deb = 80;
		break;

	case 3:

		dc.DrawRectangle(wxPoint(54, visina_panel - visina_prikaza + 36), wxSize(deb_ven + 1, deb_ven + 1)); // celica 0
		dc.DrawLine(wxPoint(54 + deb_ven / 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven / 3, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven / 3 * 2, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5), wxPoint(54 + deb_ven / 3 * 2, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven / 3 * 2 - 4, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5), wxPoint(54 + deb_ven / 3 * 2 + 4 + 1, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5));
		dc.DrawLine(wxPoint(54 + deb_ven / 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven / 3 + 6, visina_panel - visina_prikaza + 36 + 6));
		dc.DrawLine(wxPoint(54 + deb_ven / 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven / 3 - 6, visina_panel - visina_prikaza + 36 + 6));
		dc.DrawRectangle(wxPoint(54 + deb_ven, visina_panel - visina_prikaza + 36), wxSize(deb_ven + 1, deb_ven + 1)); // celica 1
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven + deb_ven / 3 * 2, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 3, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5), wxPoint(54 + deb_ven + deb_ven / 3, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 3 - 4, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5), wxPoint(54 + deb_ven + deb_ven / 3 + 4 + 1, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 3 * 2, visina_panel - visina_prikaza + deb_ven + 36), wxPoint(54 + deb_ven + deb_ven / 3 * 2 + 6, visina_panel - visina_prikaza + deb_ven + 36 - 6));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 3 * 2, visina_panel - visina_prikaza + deb_ven + 36), wxPoint(54 + deb_ven + deb_ven / 3 * 2 - 6, visina_panel - visina_prikaza + deb_ven + 36 - 6));

		break;

	case 4:

		dc.DrawRectangle(wxPoint(54, visina_panel - visina_prikaza + 36), wxSize(deb_ven + 1, deb_ven + 1)); // celica 0
		dc.DrawLine(wxPoint(54 + deb_ven / 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven / 3, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven / 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven / 3 + 6, visina_panel - visina_prikaza + 36 + 6));
		dc.DrawLine(wxPoint(54 + deb_ven / 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven / 3 - 6, visina_panel - visina_prikaza + 36 + 6));
		dc.DrawLine(wxPoint(54 + deb_ven / 3 * 2, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven / 3 * 2, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven / 3 * 2, visina_panel - visina_prikaza + deb_ven + 36), wxPoint(54 + deb_ven / 3 * 2 + 6, visina_panel - visina_prikaza + deb_ven + 36 - 6));
		dc.DrawLine(wxPoint(54 + deb_ven / 3 * 2, visina_panel - visina_prikaza + deb_ven + 36), wxPoint(54 + deb_ven / 3 * 2 - 6, visina_panel - visina_prikaza + deb_ven + 36 - 6));
		dc.DrawRectangle(wxPoint(54 + deb_ven, visina_panel - visina_prikaza + 36), wxSize(deb_ven + 1, deb_ven + 1)); // celica 1
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven + deb_ven / 3 * 2, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 3 * 2, visina_panel - visina_prikaza + deb_ven + 36), wxPoint(54 + deb_ven + deb_ven / 3 * 2 + 6, visina_panel - visina_prikaza + deb_ven + 36 - 6));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 3 * 2, visina_panel - visina_prikaza + deb_ven + 36), wxPoint(54 + deb_ven + deb_ven / 3 * 2 - 6, visina_panel - visina_prikaza + deb_ven + 36 - 6));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 3 * 2, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven + deb_ven / 3, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 3 * 2, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven + deb_ven / 3 * 2 + 6, visina_panel - visina_prikaza + 36 + 6));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 3 * 2, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven + deb_ven / 3 * 2 - 6, visina_panel - visina_prikaza + 36 + 6));

		break;

	case 5:

		dc.DrawRectangle(wxPoint(54, visina_panel - visina_prikaza + 36), wxSize(deb_ven + 1, deb_ven + 1)); // celica 0
		dc.DrawLine(wxPoint(54 + deb_ven / 4 * 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven / 4 * 3, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven / 4, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven / 2, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven / 4, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5), wxPoint(54 + deb_ven / 4, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven / 4 - 4, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5), wxPoint(54 + deb_ven / 4 + 4 + 1, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5));
		dc.DrawLine(wxPoint(54 + deb_ven / 4, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven / 4 + 6, visina_panel - visina_prikaza + 36 + 6));
		dc.DrawLine(wxPoint(54 + deb_ven / 4, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven / 4 - 6, visina_panel - visina_prikaza + 36 + 6));
		dc.DrawLine(wxPoint(54 + deb_ven / 4 * 3, visina_panel - visina_prikaza + deb_ven + 36), wxPoint(54 + deb_ven / 4 * 3 + 6, visina_panel - visina_prikaza + deb_ven + 36 - 6));
		dc.DrawLine(wxPoint(54 + deb_ven / 4 * 3, visina_panel - visina_prikaza + deb_ven + 36), wxPoint(54 + deb_ven / 4 * 3 - 6, visina_panel - visina_prikaza + deb_ven + 36 - 6));
		dc.DrawRectangle(wxPoint(54 + deb_ven, visina_panel - visina_prikaza + 36), wxSize(deb_ven + 1, deb_ven + 1)); // celica 1
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 4, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven + deb_ven / 4, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 4 * 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven + deb_ven / 2, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 4 * 3, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5), wxPoint(54 + deb_ven + deb_ven / 4 * 3, visina_panel - visina_prikaza + deb_ven + 36));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 4 * 3 - 4, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5), wxPoint(54 + deb_ven + deb_ven / 4 * 3 + 4 + 1, visina_panel - visina_prikaza + 36 + deb_ven / 6 * 5));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 4, visina_panel - visina_prikaza + deb_ven + 36), wxPoint(54 + deb_ven + deb_ven / 4 + 6, visina_panel - visina_prikaza + deb_ven + 36 - 6));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 4, visina_panel - visina_prikaza + deb_ven + 36), wxPoint(54 + deb_ven + deb_ven / 4 - 6, visina_panel - visina_prikaza + deb_ven + 36 - 6));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 4 * 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven + deb_ven / 4 * 3 + 6, visina_panel - visina_prikaza + 36 + 6));
		dc.DrawLine(wxPoint(54 + deb_ven + deb_ven / 4 * 3, visina_panel - visina_prikaza + 36), wxPoint(54 + deb_ven + deb_ven / 4 * 3 - 6, visina_panel - visina_prikaza + 36 + 6));

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

	graf.resize(seznam_valjev.size());

	//- IZRIS VALJEV
	for (int i = 0; i < seznam_valjev.size(); i++) {

		switch (seznam_valjev[i][2]) {

		case 0:
			
			if (res[1][2] < 5) { res[1][0] = 6; res[1][1] = 1; }
			else if (res[1][2] > seznam_lastnosti[1][11] - 5) { res[1][0] = 1; res[1][1] = 6; }
			if (res[2][2] < 5) { res[2][0] = 6; res[2][1] = 1; }
			else if (res[2][2] > seznam_lastnosti[2][11] - 5) { res[2][0] = 1; res[2][1] = 6; }

			if (sim == true) res[i] = izracun3(n, seznam_valjev[i][2], seznam_lastnosti[i], res[i]);
			
			graf[i].push_back(res[i]);


			dc.DrawRectangle(wxPoint(seznam_valjev[i][0], seznam_valjev[i][1]), wxSize(deb + 1, vis + 1)); // Ohišje

			if (seznam_lastnosti[i][4] < 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 16 * 1 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1)); // Priklucki
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 1, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 16 * 1 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1));
			}
			if (seznam_lastnosti[i][5] < 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 7, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 + 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 1 + deb / 8 * 7, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 16 * 1 + deb / 8 * 7 - 1, seznam_valjev[i][1] + vis - deb / 16 * 1 - 1));
			}

			dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb / 8 * 1 + res[i][2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1]), wxSize(deb / 8 + 1, vis + 1)); // Bat

			if (seznam_lastnosti[i][18] > 0) {
				dc.DrawCircle(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 - deb + res[i][2] * 50 / seznam_lastnosti[i][11] + 5, seznam_valjev[i][1] + vis / 5 * 3), 4); // Koncno stikalo
			}
			if (seznam_lastnosti[i][19] > 0) {
				dc.DrawCircle(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[i][2] * 50 / seznam_lastnosti[i][11] + deb / 8 * 7 - 5, seznam_valjev[i][1] + vis / 5 * 3), 4);
			}

			if (seznam_lastnosti[i][0] > 0) {
				dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 - deb + res[i][2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1] + vis / 5 * 2), wxSize(deb / 8 * 7 + 1, vis / 5 + 1)); // Batnica
			}
			if (seznam_lastnosti[i][1] > 0) {
				dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[i][2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1] + vis / 5 * 2), wxSize(deb / 8 * 7 + 1, vis / 5 + 1));
			}

			if (seznam_lastnosti[i][2] > 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + (deb - deb / 8 * 7 + res[i][2] * 50 / seznam_lastnosti[i][11]) * 1 / 3, seznam_valjev[i][1])); // Vzmet
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + (deb - deb / 8 * 7 + res[i][2] * 50 / seznam_lastnosti[i][11]) * 1 / 3, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + (deb - deb / 8 * 7 + res[i][2] * 50 / seznam_lastnosti[i][11]) * 2 / 3, seznam_valjev[i][1] + vis));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + (deb - deb / 8 * 7 + res[i][2] * 50 / seznam_lastnosti[i][11]) * 2 / 3, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 8 * 1 + res[i][2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1]));
			}
			if (seznam_lastnosti[i][3] > 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[i][2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[i][2] * 50 / seznam_lastnosti[i][11] + (deb - deb / 8 * 2 - res[i][2] * 50 / seznam_lastnosti[i][11]) * 1 / 3, seznam_valjev[i][1]));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[i][2] * 50 / seznam_lastnosti[i][11] + (deb - deb / 8 * 2 - res[i][2] * 50 / seznam_lastnosti[i][11]) * 1 / 3, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[i][2] * 50 / seznam_lastnosti[i][11] + (deb - deb / 8 * 2 - res[i][2] * 50 / seznam_lastnosti[i][11]) * 2 / 3, seznam_valjev[i][1] + vis));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + res[i][2] * 50 / seznam_lastnosti[i][11] + (deb - deb / 8 * 2 - res[i][2] * 50 / seznam_lastnosti[i][11]) * 2 / 3, seznam_valjev[i][1] + vis), wxPoint(seznam_valjev[i][0] + deb, seznam_valjev[i][1]));
			}
			if (seznam_lastnosti[i][12] > 0) {
				dc.DrawRoundedRectangle(wxPoint(seznam_valjev[i][0] - deb + deb / 8 * 2 + res[i][2] * 50 / seznam_lastnosti[i][11] - 40, seznam_valjev[i][1] + 5), wxSize(40 + 1, 40 + 1), 4);
				dc.DrawText(wxString::Format("%g kg", seznam_lastnosti[i][12]), wxPoint(seznam_valjev[i][0] - deb + deb / 8 * 2 + res[i][2] * 50 / seznam_lastnosti[i][11] - 40 + 5, seznam_valjev[i][1] + 5 + 13));
			}
			if (seznam_lastnosti[i][13] > 0) {
				dc.DrawRoundedRectangle(wxPoint(seznam_valjev[i][0] + deb + deb / 8 * 1 + res[i][2] * 50 / seznam_lastnosti[i][11], seznam_valjev[i][1] + 5), wxSize(40 + 1, 40 + 1), 4);
				dc.DrawText(wxString::Format("%g kg", seznam_lastnosti[i][13]), wxPoint(seznam_valjev[i][0] + deb + deb / 8 * 1 + res[i][2] * 50 / seznam_lastnosti[i][11] + 5, seznam_valjev[i][1] + 5 + 13));
			}

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] - 16)); // Ime

			dc.DrawText(wxString::Format("p1 = %g", res[i][0]), wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + vis)); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g", res[i][1]), wxPoint(seznam_valjev[i][0] + deb, seznam_valjev[i][1] + vis)); // Tlak v desnem

			
			dc.DrawText(wxString::Format("x = %g", res[i][2]), wxPoint(seznam_valjev[i][0] + 160, seznam_valjev[i][1]));
			dc.DrawText(wxString::Format("v = %g", res[i][3]), wxPoint(seznam_valjev[i][0] + 160, seznam_valjev[i][1] + 12));
			dc.DrawText(wxString::Format("a = %g", res[i][4]), wxPoint(seznam_valjev[i][0] + 160, seznam_valjev[i][1] + 24));
			dc.DrawText(wxString::Format("V_l = %g", res[i][5]), wxPoint(seznam_valjev[i][0] + 160, seznam_valjev[i][1] + 36));
			dc.DrawText(wxString::Format("V_d = %g", res[i][6]), wxPoint(seznam_valjev[i][0] + 160, seznam_valjev[i][1] + 48));
			dc.DrawText(wxString::Format("st: %g", graf[i][graf[i].size() - 1][7]), wxPoint(seznam_valjev[i][0] + 160, seznam_valjev[i][1] + 60));


			// Graf:
			if (seznam_lastnosti[i][17] > 0) {
				

				dc.SetPen(wxPen(wxColour(204, 204, 204), 1, wxPENSTYLE_SOLID));
				dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + 240, seznam_valjev[i][1]), wxSize(350 + 1, 80 + 1));
				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

				if (graf[i].size() > 1 && graf[i][graf[i].size() - 1][7] <= 400) {
					for (int j = 1; j < graf[i][graf[i].size() - 1][7]; j++) {
						dc.DrawLine(wxPoint(seznam_valjev[i][0] + 240 + graf[i][j - 1][7], seznam_valjev[i][1] + 80 - graf[i][j - 1][2] * 80 / seznam_lastnosti[i][11]), wxPoint(seznam_valjev[i][0] + 240 + graf[i][j][7], seznam_valjev[i][1] + 80 - graf[i][j][2] * 80 / seznam_lastnosti[i][11]));
					}
				}
				else if (graf[i][graf[i].size() - 1][7] > 400) {
					int zamik = graf[i][graf[i][graf[i].size() - 1][7] - 400][7] - 1;
					for (int j = graf[i][graf[i].size() - 1][7] - 400; j < graf[i][graf[i].size() - 1][7]; j++) {
						dc.DrawLine(wxPoint(seznam_valjev[i][0] + 240 + graf[i][j - 1][7] - zamik, seznam_valjev[i][1] + 80 - graf[i][j - 1][2] * 80 / seznam_lastnosti[i][11]), wxPoint(seznam_valjev[i][0] + 240 + graf[i][j][7] - zamik, seznam_valjev[i][1] + 80 - graf[i][j][2] * 80 / seznam_lastnosti[i][11]));
					}
				}
			}

			break;

		case 1:



			break;

		case 2:

			deb = 64;
			dc.DrawRectangle(wxPoint(seznam_valjev[i][0], seznam_valjev[i][1]), wxSize(deb + 1, vis + 1));
			dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb / 4 * 3, seznam_valjev[i][1] + vis / 10), wxSize(deb / 2 + 1, vis / 5 + 1));
			dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb / 4 * 3, seznam_valjev[i][1] + vis / 10 * 7), wxSize(deb / 2 + 1, vis / 5 + 1));
			deb = 80;

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] - 16)); // Ime

			break;

		case 3:

			dc.DrawRectangle(wxPoint(seznam_valjev[i][0], seznam_valjev[i][1]), wxSize(deb_ven + 1, deb_ven + 1));
			dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb_ven, seznam_valjev[i][1] + 36), wxSize(deb_ven + 1, deb_ven + 1));


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
wxCheckBox* izrisGrafa;

PomoznoOkno::PomoznoOkno() : wxFrame(nullptr, wxID_ANY, wxString::Format("Zacetno stanje elementa"), wxPoint(0, 0), wxSize(420, 540)) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
	wxSize size = this->GetSize();


	wxArrayString lastnosti;
	lastnosti.Add("Batnica");
	lastnosti.Add("Vzmet");
	lastnosti.Add("Tlaèno izoliran");
	lastnosti.Add("Masa");
	lastnosti.Add("Koncno stikalo");

	wxPoint pointLevaLastnost = wxPoint(10, 44);
	levaLastnost = new wxCheckListBox(panel, wxID_ANY, pointLevaLastnost, wxDefaultSize, lastnosti);
	if (seznam_lastnosti[oznacitev][0] > 0) levaLastnost->Check(0);
	if (seznam_lastnosti[oznacitev][2] > 0) levaLastnost->Check(1);
	if (seznam_lastnosti[oznacitev][4] > 0) levaLastnost->Check(2);
	if (seznam_lastnosti[oznacitev][12] > 0) levaLastnost->Check(3);
	if (seznam_lastnosti[oznacitev][18] > 0) levaLastnost->Check(4);
	wxPoint pointDesnaLastnost = wxPoint(size.x / 2, pointLevaLastnost.y);
	desnaLastnost = new wxCheckListBox(panel, wxID_ANY, pointDesnaLastnost, wxDefaultSize, lastnosti);
	if (seznam_lastnosti[oznacitev][1] > 0) desnaLastnost->Check(0);
	if (seznam_lastnosti[oznacitev][3] > 0) desnaLastnost->Check(1);
	if (seznam_lastnosti[oznacitev][5] > 0) desnaLastnost->Check(2);
	if (seznam_lastnosti[oznacitev][13] > 0) desnaLastnost->Check(3);
	if (seznam_lastnosti[oznacitev][19] > 0) desnaLastnost->Check(4);


	wxArrayString tlak;
	tlak.Add("Delovni tlak");
	tlak.Add("Okoljski tlak");

	wxPoint pointLevaTlak = wxPoint(pointLevaLastnost.x, pointLevaLastnost.y + 110);
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
	wxPoint pointPrikazGrafa = wxPoint(10, pointDelTlak.y + 60);
	izrisGrafa = new wxCheckBox(panel, wxID_ANY, "Prikaz Grafa", pointPrikazGrafa, wxDefaultSize);
	if (seznam_lastnosti[oznacitev][17] > 0) izrisGrafa->SetValue(true);
	else izrisGrafa->SetValue(false);

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

void PomoznoOkno::OnButtonClicked(wxCommandEvent& evt) {

	if (!(oznacitev < 0)) {
		// Za seznam_lastnosti:
		//last[batnica_leva, batnica_desna, vzmer_leva, vzmet_desna, tlak_izo_leva, tlak_izo_desna, zrak_prik_leva, zrak_prik_desna,
		//     delovni_tlak, okoljski_tlak, zacetna_poz, hod_bata, masa_leva, masa_desna, batnica_premer_leva, batnica_premer_desna,
		//	   bat_premer, izris_grafa, koncno_stikalo_leva, koncno_stikalo_desna]
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
		seznam_lastnosti[oznacitev][10] = static_cast<double>(zacPoz->GetValue());
		seznam_lastnosti[oznacitev][11] = static_cast<double>(hodBata->GetValue());
		if (levaLastnost->IsChecked(0) == true && levaLastnost->IsChecked(3)) seznam_lastnosti[oznacitev][12] = levaMasa->GetValue();
		else seznam_lastnosti[oznacitev][12] = 0;
		if (desnaLastnost->IsChecked(0) == true && desnaLastnost->IsChecked(3)) seznam_lastnosti[oznacitev][13] = desnaMasa->GetValue();
		else seznam_lastnosti[oznacitev][13] = 0;
		seznam_lastnosti[oznacitev][14] = levaBatnica->GetValue();
		seznam_lastnosti[oznacitev][15] = desnaBatnica->GetValue();
		seznam_lastnosti[oznacitev][16] = static_cast<double>(premerBata->GetValue());
		if (izrisGrafa->IsChecked() == true) seznam_lastnosti[oznacitev][17] = 1;
		else seznam_lastnosti[oznacitev][17] = -1;
		if (levaLastnost->IsChecked(4) == true && levaLastnost->IsChecked(0) == true) seznam_lastnosti[oznacitev][18] = 1;
		else seznam_lastnosti[oznacitev][18] = -1;
		if (desnaLastnost->IsChecked(4) == true && desnaLastnost->IsChecked(0) == true) seznam_lastnosti[oznacitev][19] = 1;
		else seznam_lastnosti[oznacitev][19] = -1;

		// Za res:
		//res[p_l, p_d, x, v, a, V_l, V_d]
		if (levaTlak->GetSelection() == 0) res_reset[oznacitev][0] = delTlak->GetValue();
		else res_reset[oznacitev][0] = okTlak->GetValue();
		if (desnaTlak->GetSelection() == 0) res_reset[oznacitev][1] = delTlak->GetValue();
		else res_reset[oznacitev][1] = okTlak->GetValue();
		res_reset[oznacitev][2] = static_cast<double>(zacPoz->GetValue()) * static_cast<double>(hodBata->GetValue()) / 100;
		res_reset[oznacitev][3] = 0;
		res_reset[oznacitev][4] = 0;
		res_reset[oznacitev][5] = 0;
		res_reset[oznacitev][6] = 0;

		res[oznacitev] = res_reset[oznacitev];
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

	dc.DrawLine(wxPoint(size.x / 2, 0), wxPoint(size.x / 2, 270));
	dc.DrawLine(wxPoint(0, 24), wxPoint(size.x, 24));
	dc.DrawLine(wxPoint(0, 270), wxPoint(size.x, 270));

	dc.DrawText("Leva stran valja:", wxPoint(10, 4));
	dc.DrawText("Lastnosti:", wxPoint(10, 30));
	dc.DrawText("Zraèni prikljuèek:", wxPoint(10, 140));
	dc.DrawText("Masa [kg] =", wxPoint(10, 210));
	dc.DrawText("Batnica [mm] =", wxPoint(10, 240));

	dc.DrawText("Desna stran valja:", wxPoint(size.x / 2 + 10, 4));
	dc.DrawText("Lastnosti:", wxPoint(size.x / 2 + 10, 30));
	dc.DrawText("Zraèni prikljuèek:", wxPoint(size.x / 2 + 10, 140));
	dc.DrawText("Masa [kg] =", wxPoint(size.x / 2 + 10, 210));
	dc.DrawText("Batnica [mm] =", wxPoint(size.x / 2 + 10, 240));

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

	if (levaLastnost->IsChecked(4) == true && levaLastnost->IsChecked(0) == true) {
		dc.DrawCircle(wxPoint(zamik + deb / 8 * 2 - deb + zacPom + 5, visina_panel - visina_prikaza + 36 + vis / 5 * 3), 4); // Koncno stikalo
	}
	if (desnaLastnost->IsChecked(4) == true && desnaLastnost->IsChecked(0) == true) {
		dc.DrawCircle(wxPoint(zamik + deb / 8 * 2 + zacPom + deb / 8 * 7 - 5, visina_panel - visina_prikaza + 36 + vis / 5 * 3), 4);
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