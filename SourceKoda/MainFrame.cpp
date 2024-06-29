#include "MainFrame.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/time.h>
#include <wx/utils.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <fstream>
//#include <algorithm>


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
	double m = .8 / 1000; //- masa batnice in bata [t] // m = 2. / 1000 -> pika zlo pomembna
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
		if (koef_vzm_leva > 0) F0v = F0v + 60;
		F1 = A_d * p_d;
		F1b = (A0 - A_d) * pok;
		F1v = x * koef_vzm_desna;
		if (koef_vzm_desna > 0) F1v = F1v + 60;

		dF = F0 + F0b + F0v - F1 - F1b - F1v;


		if (v == 0) {

			if (abs(dF) > Ftr_s) { // 2.0

				if (dF > 0) a = (dF - Ftr_s) / m; // 2.1
				else if (dF < 0) a = (dF + Ftr_s) / m;
			}
			else a = 0;
		}
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

std::vector<double> ventil(std::vector<double> last, std::vector<double> res, double p_del = 6., double p_ok = 1.) {
	
	double Flg = 0, Fdg = 0, Flp = 0, Fdp = 0, Flv = 0, Fdv = 0, Fls = 0, Fds = 0;
	if (last[1] > 0) Flg = 1000;
	if (last[2] > 0) Fdg = 1000;
	if (res[6] > 0) Flp = res[6] * 100;
	if (res[7] > 0) Fdp = res[7] * 100;
	if (last[5] > 0) Flv = 250;
	if (last[6] > 0) Fdv = 250;
	if (last[7] > 0) Fls = -2000; //////////// mogoc leps ta minus nardit
	if (last[8] > 0) Fds = -2000; //////////// mogoc leps ta minus nardit

	if (Flg + Flp + Flv + Fls < Fdg + Fdp + Fdv + Fds) res[0] = 0;
	else if (Flg + Flp + Flv + Fls > Fdg + Fdp + Fdv + Fds) res[0] = 1;

	int poz = static_cast<int> (res[0]);
	int naziv = static_cast<int> (last[0]);

	switch (naziv) {

	case 32: // ventil 3/2:

		switch (poz) {

		case 0: // celica 0 (desna):

			res[2] = res[3];

			break;

		case 1: // celica 1 (leva):

			res[2] = res[1];

			break;

		default:
			break;
		}
		break;


	case 42: // ventil 4/2:

		switch (poz) {

		case 0: // celica 0 (desna):

			res[2] = res[1];
			res[4] = res[3];

			break;

		case 1: // celica 1 (leva):

			res[2] = res[3];
			res[4] = res[1];

			break;

		default:
			break;
		}

		break;


	case 52: // ventil 5/2:

		switch (poz) {

		case 0: // celica 0 (desna):

			res[2] = res[1];
			res[4] = res[5];

			break;

		case 1: // celica 1 (leva):

			res[2] = res[3];
			res[4] = res[1];
			
			break;

		default:
			break;
		}

		break;


	default:
		break;
	}

	return res;
}

//pointer[element_valja, element_ventila]
void cevka(std::vector<std::vector<double*>> pointer) {

	for (int i = 0; i < pointer.size(); i++) {

		*(pointer[i][0]) = *(pointer[i][1]);
	}
}

//pointer[element_valja, element_ventila]
void stikalo(std::vector<std::vector<double*>> pointer, std::vector<std::vector<int>> dol) {

	for (int i = 0; i < pointer.size(); i++) {

		if (abs(*pointer[i][0] - dol[i][0]) < 16) *(pointer[i][1]) = 1;
		else *(pointer[i][1]) = -1;
	}
}

int prvaStevka(int n) {

	while (n >= 10) n = n / 10;

	return n;
}

int zadnjaStevka(int n) {

	n = n % 10;

	return n;
}



int x = 300;
int y = 250;
int dx = x;
int dy = y;
int velikost_x = 120;
int velikost_y = 60;
short st_elementov = 0;

wxPanel* panel;
wxChoice* choice_dod;
//wxChoice* choice_izb;
wxSlider* slider;
//wxSpinCtrl* spinCtrl;

//seznam_valjev[x, y, element]
std::vector<std::vector<int>> seznam_valjev;
//last[batnica_leva, batnica_desna, vzmer_leva, vzmet_desna, tlak_izo_leva, tlak_izo_desna, zrak_prik_leva, zrak_prik_desna,
//     delovni_tlak, okoljski_tlak, zacetna_poz, hod_bata, masa_leva, masa_desna, batnica_premer_leva, batnica_premer_desna,
//	   bat_premer, izris_grafa, koncno_stikalo_leva, koncno_stikalo_desna, prikaz_vrednosti]
std::vector<std::vector<double>> seznam_lastnosti;
//res[p_l, p_d, x, v, a, V_l, V_d]
std::vector<std::vector<double>> res_reset;
std::vector<std::vector<double>> res;
//graf[res] - za risanje grafa
std::vector<std::vector<std::vector<double>>> graf;
//seznam_cevi[naziv_valja, del_valja, naziv_ventila, del_ventila]
std::vector<std::vector<int>> seznam_cevi;
//seznam_stikal[naziv_valja, naziv_ventila, del_ventila]
std::vector<std::vector<int>> seznam_stikal;

int oznacitev = -1;
bool sim = false;
int risCevi = 0;


MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* button_dod = new wxButton(panel, wxID_ANY, "Dodaj element", wxPoint(5, 48), wxSize(190, -1));
	wxButton* button_izb = new wxButton(panel, wxID_ANY, "Izbrisi element", wxPoint(5, 100), wxSize(190, -1));
	wxButton* button_izb_vse = new wxButton(panel, wxID_ANY, "Izbrisi vse", wxPoint(5, 130), wxSize(190, -1));
	wxButton* pov_cev = new wxButton(panel, wxID_ANY, "Povezava cevi", wxPoint(5, 160), wxSize(190, -1));
	wxButton* cev_izb = new wxButton(panel, wxID_ANY, "Izbrisi cevi", wxPoint(5, 190), wxSize(190, -1));
	wxButton* predlog = new wxButton(panel, wxID_ANY, "Pregled elementov", wxPoint(5, 355), wxSize(190, -1));
	wxButton* simuliraj = new wxButton(panel, wxID_ANY, "Simuliraj", wxPoint(5, 420), wxSize(190, 56));
	wxButton* pomozno_okno = new wxButton(panel, wxID_ANY, "Nastavitve elementa", wxPoint(5, 240), wxSize(190, -1));
	wxButton* shrani = new wxButton(panel, wxID_ANY, "Shrani", wxPoint(5, 270), wxSize(190, -1));
	wxButton* nalozi = new wxButton(panel, wxID_ANY, "Nalozi", wxPoint(5, 300), wxSize(190, -1));

	wxArrayString choices;
	choices.Add("Pnevmaticni valj");
	choices.Add("Vakumski prisesek");
	choices.Add("Pnevmaticno prijemalo");
	choices.Add("-");
	choices.Add("Idealni kompresor");
	choices.Add("Potni ventil");
	st_elementov = choices.size();


	choice_dod = new wxChoice(panel, wxID_ANY, wxPoint(5, 20), wxSize(190, -1), choices/*, wxCB_SORT*/);
	choice_dod->SetSelection(0);
	slider = new wxSlider(panel, wxID_ANY, 0, 0, 1000, wxPoint(5, 380), wxSize(190, -1), wxSL_VALUE_LABEL);

	panel->Bind(wxEVT_LEFT_DOWN, &MainFrame::OnMouseEvent, this);
	panel->Bind(wxEVT_LEFT_DCLICK, &MainFrame::OnDoubleMouseEvent, this);
	panel->Bind(wxEVT_MOTION, &MainFrame::OnMouseMoveEvent, this);
	panel->Bind(wxEVT_LEFT_UP, &MainFrame::OnMouseUpEvent, this);
	panel->Bind(wxEVT_SIZE, &MainFrame::OnSizeChanged, this);
	button_dod->Bind(wxEVT_BUTTON, &MainFrame::OnButtonDodClicked, this);
	button_izb->Bind(wxEVT_BUTTON, &MainFrame::OnButtonIzbClicked, this);
	button_izb_vse->Bind(wxEVT_BUTTON, &MainFrame::OnButtonIzbVseClicked, this);
	pov_cev->Bind(wxEVT_BUTTON, &MainFrame::OnCevClicked, this);
	cev_izb->Bind(wxEVT_BUTTON, &MainFrame::OnCevIzbClicked, this);
	predlog->Bind(wxEVT_BUTTON, &MainFrame::OnButtonPredVseClicked, this);
	simuliraj->Bind(wxEVT_BUTTON, &MainFrame::OnButtonSimClicked, this);
	pomozno_okno->Bind(wxEVT_BUTTON, &MainFrame::OnButtonPomClicked, this);
	shrani->Bind(wxEVT_BUTTON, &MainFrame::OnShraniClicked, this);
	nalozi->Bind(wxEVT_BUTTON, &MainFrame::OnNaloziClicked, this);
	choice_dod->Bind(wxEVT_CHOICE, &MainFrame::OnChoicesClicked, this);
	slider->Bind(wxEVT_SLIDER, &MainFrame::OnSliderChanged, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(MainFrame::OnPaint));

	wxStatusBar* statusBar = CreateStatusBar();
	statusBar->SetDoubleBuffered(true); // da ne utripa izpis
	panel->SetDoubleBuffered(true);
	

	seznam_valjev.push_back({ 320, 50, 0 }); // Prikaz elementov
	seznam_valjev.push_back({ 320, 150, 0 });
	seznam_valjev.push_back({ 320, 250, 0 });

	seznam_lastnosti.push_back({ 1, 1, -1, -1, 1, 1, 1, -1, 6, 1, 0, 100, 0, 0, 10, 30, 40, 1, -1, -1, 1 });
	seznam_lastnosti.push_back({ -1, 1, -1, -1, -1, -1, 1, -1, 6, 1, 0, 250, 0, 0, 20, 20, 40, 1, -1, 1, 1 });
	seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 350, 0, 6, 20, 20, 40, 1, -1, -1, 1 });

	res_reset.push_back({ 6., 1., 0, 0, 0, 0, 0, 0 });
	res_reset.push_back({ 0, 0, 0, 0, 0, 0, 0, 0 });
	res_reset.push_back({ 0, 0, 0, 0, 0, 0, 0, 0 });

	
	seznam_valjev.push_back({ 320, 390, 5 });
	seznam_lastnosti.push_back({ 52, -1, -1, -1, -1, -1, -1, -1, -1 });
	res_reset.push_back({ 0, 1., 1., 1., 1., 1., 0, 0 });


	seznam_valjev.push_back({ 320, 470, 4 });
	seznam_lastnosti.push_back({ 6 });
	res_reset.push_back({ 6. });
	res = res_reset;

	
	seznam_cevi.push_back({ 1,0,3,2 });
	seznam_cevi.push_back({ 1,1,3,4 });
	seznam_cevi.push_back({ 2,0,3,2 });
	seznam_cevi.push_back({ 2,1,3,4 });
	seznam_cevi.push_back({ 3,1,4,0 });

	seznam_stikal.push_back({ 1, 3, 7, 0, 0 });
	seznam_stikal.push_back({ 1, 3, 8, 250, 1 });
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
		else if (risCevi == 1) {
			if (seznam_valjev[i][2] == 0) {
				if (seznam_lastnosti[i][4] < 0 && mousePos.x > seznam_valjev[i][0] && mousePos.x < seznam_valjev[i][0] + 10 && mousePos.y > seznam_valjev[i][1] + 50 && mousePos.y < seznam_valjev[i][1] + 60) {
					bool risi = true;
					for (int j = 0; j < seznam_cevi.size(); j++) if (seznam_cevi[j][0] == i && seznam_cevi[j][1] == 0) risi = false;

					if (risi) {
						seznam_cevi[seznam_cevi.size() - 1][0] = i;
						seznam_cevi[seznam_cevi.size() - 1][1] = 0;

						risCevi = 2;
					}
					else wxLogStatus("Dovoljena je samo ena povezava na valj.");

					break;
				}
				else if (seznam_lastnosti[i][5] < 0 && mousePos.x > seznam_valjev[i][0] + 70 && mousePos.x < seznam_valjev[i][0] + 80 && mousePos.y > seznam_valjev[i][1] + 50 && mousePos.y < seznam_valjev[i][1] + 60) {
					bool risi = true;
					for (int j = 0; j < seznam_cevi.size(); j++) if (seznam_cevi[j][0] == i && seznam_cevi[j][1] == 1) risi = false;

					if (risi) {
						seznam_cevi[seznam_cevi.size() - 1][0] = i;
						seznam_cevi[seznam_cevi.size() - 1][1] = 1;

						risCevi = 2;
					}
					else wxLogStatus("Dovoljena je samo ena povezava na valj.");

					break;
				}
			}
			else if (seznam_valjev[i][2] == 5) {
				if (seznam_lastnosti[i][3] >= 0 && mousePos.x > seznam_valjev[i][0] - 24 && mousePos.x < seznam_valjev[i][0] + 48 && mousePos.y > seznam_valjev[i][1] && mousePos.y < seznam_valjev[i][1] + 48) {
					bool risi = true;
					for (int j = 0; j < seznam_cevi.size(); j++) if (seznam_cevi[j][0] == i && seznam_cevi[j][1] == 6) risi = false;

					if (risi) {
						seznam_cevi[seznam_cevi.size() - 1][0] = i;
						seznam_cevi[seznam_cevi.size() - 1][1] = 6;

						risCevi = 2;
						break;
					}
					else wxLogStatus("Dovoljena je samo ena povezava.");
				}
				else if (seznam_lastnosti[i][4] >= 0 && mousePos.x > seznam_valjev[i][0] + 96 && mousePos.x < seznam_valjev[i][0] + 168 && mousePos.y > seznam_valjev[i][1] && mousePos.y < seznam_valjev[i][1] + 48) {
						bool risi = true;
						for (int j = 0; j < seznam_cevi.size(); j++) if (seznam_cevi[j][0] == i && seznam_cevi[j][1] == 7) risi = false;

						if (risi) {
							seznam_cevi[seznam_cevi.size() - 1][0] = i;
							seznam_cevi[seznam_cevi.size() - 1][1] = 7;

							risCevi = 2;
							break;
						}
						else wxLogStatus("Dovoljena je samo ena povezava.");
				}
				else if (prvaStevka(seznam_lastnosti[i][0]) == 3) {
					if (mousePos.x > seznam_valjev[i][0] + 48 && mousePos.x < seznam_valjev[i][0] + 96 && mousePos.y > seznam_valjev[i][1] + 48 && mousePos.y < seznam_valjev[i][1] + 58) {
						bool risi = true;
						for (int j = 0; j < seznam_cevi.size(); j++) if (seznam_cevi[j][0] == i && seznam_cevi[j][1] == 1) risi = false;

						if (risi) {
							seznam_cevi[seznam_cevi.size() - 1][0] = i;
							seznam_cevi[seznam_cevi.size() - 1][1] = 1;

							risCevi = 2;
							break;
						}
						else wxLogStatus("Dovoljena je samo ena povezava.");
					}
				}
				else if (prvaStevka(seznam_lastnosti[i][0]) == 4) {
					if (mousePos.x > seznam_valjev[i][0] + 48 && mousePos.x < seznam_valjev[i][0] + 96 && mousePos.y > seznam_valjev[i][1] + 48 && mousePos.y < seznam_valjev[i][1] + 58) {
						bool risi = true;
						for (int j = 0; j < seznam_cevi.size(); j++) if (seznam_cevi[j][0] == i && seznam_cevi[j][1] == 1) risi = false;

						if (risi) {
							seznam_cevi[seznam_cevi.size() - 1][0] = i;
							seznam_cevi[seznam_cevi.size() - 1][1] = 1;

							risCevi = 2;
							break;
						}
						else wxLogStatus("Dovoljena je samo ena povezava.");
					}
				}
				else if (prvaStevka(seznam_lastnosti[i][0]) == 5) {
					if (mousePos.x > seznam_valjev[i][0] + 48 && mousePos.x < seznam_valjev[i][0] + 96 && mousePos.y > seznam_valjev[i][1] + 48 && mousePos.y < seznam_valjev[i][1] + 58) {
						bool risi = true;
						for (int j = 0; j < seznam_cevi.size(); j++) if (seznam_cevi[j][0] == i && seznam_cevi[j][1] == 1) risi = false;

						if (risi) {
							seznam_cevi[seznam_cevi.size() - 1][0] = i;
							seznam_cevi[seznam_cevi.size() - 1][1] = 1;

							risCevi = 2;
							break;
						}
						else wxLogStatus("Dovoljena je samo ena povezava.");
					}
				}
			}
		}
		else if (risCevi == 2) {
			if (seznam_valjev[i][2] == 5) {
				if (prvaStevka(seznam_lastnosti[i][0]) == 3) {
					if (mousePos.x > seznam_valjev[i][0] + 48 && mousePos.x < seznam_valjev[i][0] + 96 && mousePos.y > seznam_valjev[i][1] - 10 && mousePos.y < seznam_valjev[i][1]) {
						seznam_cevi[seznam_cevi.size() - 1][2] = i;
						seznam_cevi[seznam_cevi.size() - 1][3] = 2;

						risCevi = 0;
						break;
					}
				}
				else if (prvaStevka(seznam_lastnosti[i][0]) == 4) {
					if (mousePos.x > seznam_valjev[i][0] + 48 && mousePos.x < seznam_valjev[i][0] + 72 && mousePos.y > seznam_valjev[i][1] - 10 && mousePos.y < seznam_valjev[i][1]) {
						seznam_cevi[seznam_cevi.size() - 1][2] = i;
						seznam_cevi[seznam_cevi.size() - 1][3] = 4;

						risCevi = 0;
						break;
					}
					else if (mousePos.x > seznam_valjev[i][0] + 72 && mousePos.x < seznam_valjev[i][0] + 96 && mousePos.y > seznam_valjev[i][1] - 10 && mousePos.y < seznam_valjev[i][1]) {
						seznam_cevi[seznam_cevi.size() - 1][2] = i;
						seznam_cevi[seznam_cevi.size() - 1][3] = 2;

						risCevi = 0;
						break;
					}
				}
				else if (prvaStevka(seznam_lastnosti[i][0]) == 5) {
					if (mousePos.x > seznam_valjev[i][0] + 48 && mousePos.x < seznam_valjev[i][0] + 72 && mousePos.y > seznam_valjev[i][1] - 10 && mousePos.y < seznam_valjev[i][1]) {
						seznam_cevi[seznam_cevi.size() - 1][2] = i;
						seznam_cevi[seznam_cevi.size() - 1][3] = 4;

						risCevi = 0;
						break;
					}
					else if (mousePos.x > seznam_valjev[i][0] + 72 && mousePos.x < seznam_valjev[i][0] + 96 && mousePos.y > seznam_valjev[i][1] - 10 && mousePos.y < seznam_valjev[i][1]) {
						seznam_cevi[seznam_cevi.size() - 1][2] = i;
						seznam_cevi[seznam_cevi.size() - 1][3] = 2;

						risCevi = 0;
						break;
					}
				}
			}
			else if (seznam_valjev[i][2] == 4) {
				if (mousePos.x > seznam_valjev[i][0] && mousePos.x < seznam_valjev[i][0] + 40 && mousePos.y > seznam_valjev[i][1] - 10 && mousePos.y < seznam_valjev[i][1]) {
					seznam_cevi[seznam_cevi.size() - 1][2] = i;
					seznam_cevi[seznam_cevi.size() - 1][3] = 0;

					risCevi = 0;
					break;
				}
			}
		}
		
		else if (seznam_valjev[i][2] == 5) { // gumbi
			if (seznam_lastnosti[i][1] >= 0 && mousePos.x > seznam_valjev[i][0] - 16 && mousePos.x < seznam_valjev[i][0] + 48 && mousePos.y > seznam_valjev[i][1] && mousePos.y < seznam_valjev[i][1] + 48) {
				seznam_lastnosti[i][1] = 1;
				break;
			}
			else if (seznam_lastnosti[i][2] >= 0 && mousePos.x > seznam_valjev[i][0] + 96 && mousePos.x < seznam_valjev[i][0] + 160 && mousePos.y > seznam_valjev[i][1] && mousePos.y < seznam_valjev[i][1] + 48) {
				seznam_lastnosti[i][2] = 1;
				break;
			}
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


	/*wxString message = wxString::Format("Mouse Event Detected! (x=%d y=%d)", dx, dy);
	wxLogStatus(message);*/

	Refresh();
}

void MainFrame::OnDoubleMouseEvent(wxMouseEvent& evt) {

	if (oznacitev >= 0) {
		sim = false;
		risCevi = 0;

		if (seznam_valjev[oznacitev][2] == 0) {
		
			PomoznoOkno* dodatnoOkno = new PomoznoOkno();
			dodatnoOkno->Show();
		}
		else if (seznam_valjev[oznacitev][2] == 4) {

			KompresorNastavitve* kompNast = new KompresorNastavitve();
			kompNast->Show();
		}
		else if (seznam_valjev[oznacitev][2] == 5) {

			VentilNastavitve* ventNast = new VentilNastavitve();
			ventNast->Show();
		}
		else wxLogStatus("Ta element nima nastavitev");

		res = res_reset;
		slider->SetValue(0);
		graf.clear();

		Refresh();
	}
	else wxLogStatus("Kliknite na element");
}

void MainFrame::OnMouseMoveEvent(wxMouseEvent& evt) {

	if (sim == false) Refresh();
}

void MainFrame::OnMouseUpEvent(wxMouseEvent& evt) {

	for (int i = 0; i < seznam_valjev.size(); i++) if (seznam_valjev[i][2] == 5) {
		if (seznam_lastnosti[i][1] == 1) seznam_lastnosti[i][1] = 0;
		else if (seznam_lastnosti[i][2] == 1) seznam_lastnosti[i][2] = 0;
	}

	Refresh();
}

void MainFrame::OnSizeChanged(wxSizeEvent& evt) {

	Refresh();
}

void MainFrame::OnButtonDodClicked(wxCommandEvent& evt) {
	sim = false;

	if ((dx != x || dy != y) && dx >= 200) {

		x = dx;
		y = dy;

		if (choice_dod->GetSelection() == 0) {
			seznam_valjev.push_back({ x, y, choice_dod->GetSelection() });
			seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 250, 0, 0, 20, 20, 40, -1, -1, 1, -1 });
			res_reset.push_back({ 6., 1., 0, 0, 0, 0, 0, 0 });
		}
		else if (choice_dod->GetSelection() == 4) {
			seznam_valjev.push_back({ x, y, choice_dod->GetSelection() });
			seznam_lastnosti.push_back({ 6 });
			res_reset.push_back({ 6. });
		}
		else if (choice_dod->GetSelection() == 5) {
			seznam_valjev.push_back({ x, y, choice_dod->GetSelection() });
			seznam_lastnosti.push_back({ 52, -1, -1, -1, -1, -1, -1, -1, -1 });
			res_reset.push_back({ 0, 1., 1., 1., 1., 1., 0, 0 });
		}
		else {
			seznam_valjev.push_back({ x, y, choice_dod->GetSelection() });
			seznam_lastnosti.push_back({  });
			res_reset.push_back({  });
		}

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

		for (int i = seznam_cevi.size() - 1; i >= 0; i--) if (seznam_cevi[i][0] == oznacitev || seznam_cevi[i][2] == oznacitev) seznam_cevi.erase(seznam_cevi.begin() + i); // brisanje cevi

		for (int i = 0; i < seznam_cevi.size(); i++) {
			if (seznam_cevi[i][0] > oznacitev) seznam_cevi[i][0]--;
			if (seznam_cevi[i][2] > oznacitev) seznam_cevi[i][2]--;
		}


		for (int i = seznam_stikal.size() - 1; i >= 0; i--) if (seznam_stikal[i][0] == oznacitev || seznam_stikal[i][1] == oznacitev) seznam_stikal.erase(seznam_stikal.begin() + i); // brisanje stikal

		for (int i = 0; i < seznam_stikal.size(); i++) {
			if (seznam_stikal[i][0] > oznacitev) seznam_stikal[i][0]--;
			if (seznam_stikal[i][1] > oznacitev) seznam_stikal[i][1]--;
		}


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

	seznam_stikal.clear();
	seznam_cevi.clear();
	seznam_valjev.clear();
	seznam_lastnosti.clear();
	res_reset.clear();
	res = res_reset;
	graf.clear();

	slider->SetValue(0);

	oznacitev = -1;

	Refresh();
}

void MainFrame::OnCevClicked(wxCommandEvent& evt) {
	sim = false;
	oznacitev = -1;

	res = res_reset;
	graf.clear();
	slider->SetValue(0);

	if (risCevi == 0) {
		risCevi = 1;
		seznam_cevi.push_back({ 0, 0, 0, 0 });
	}
	else {
		risCevi = 0;
		seznam_cevi.erase(seznam_cevi.begin() + (seznam_cevi.size() - 1));
	}

	Refresh();
}

void MainFrame::OnCevIzbClicked(wxCommandEvent& evt) {
	sim = false;
	oznacitev = -1;

	res = res_reset;
	graf.clear();
	slider->SetValue(0);

	risCevi = 0;
	seznam_cevi.clear();

	Refresh();
}

void MainFrame::OnButtonPredVseClicked(wxCommandEvent& evt) {
	sim = false;

	seznam_stikal.clear();
	seznam_cevi.clear();
	seznam_valjev.clear();
	seznam_lastnosti.clear();
	res_reset.clear();
	res.clear();
	graf.clear();
	seznam_cevi.clear();

	seznam_valjev.push_back({ 320, 100, 0 }); // Valji
	seznam_valjev.push_back({ 320, 200, 0 });
	seznam_valjev.push_back({ 320, 300, 0 });

	seznam_lastnosti.push_back({ 1, 1, -1, -1, 1, 1, 1, -1, 6, 1, 0, 100, 0, 0, 10, 30, 40, 1, -1, -1, 1 });
	seznam_lastnosti.push_back({ -1, 1, -1, -1, -1, -1, 1, -1, 6, 1, 0, 250, 0, 0, 20, 20, 40, 1, -1, 1, 1 });
	seznam_lastnosti.push_back({ -1, 1, -1, 1, -1, -1, 1, -1, 6, 1, 0, 350, 0, 6, 20, 20, 40, 1, -1, -1, 1 });

	res_reset.push_back({ 6., 1., 0, 0, 0, 0, 0, 0 });
	res_reset.push_back({ 0, 0, 0, 0, 0, 0, 0, 0 });
	res_reset.push_back({ 0, 0, 0, 0, 0, 0, 0, 0 });
	res = res_reset;


	seznam_valjev.push_back({ 320, 440, 5 }); // Ventil
	seznam_lastnosti.push_back({ 52, -1, -1, -1, -1, -1, -1, -1, -1 });
	res_reset.push_back({ 0, 1., 1., 1., 1., 1., 0, 0 });


	seznam_valjev.push_back({ 320, 520, 4 }); // Kompresor
	seznam_lastnosti.push_back({ 6 });
	res_reset.push_back({ 6. });


	seznam_cevi.push_back({ 1,0,3,2 });
	seznam_cevi.push_back({ 1,1,3,4 });
	seznam_cevi.push_back({ 2,0,3,2 });
	seznam_cevi.push_back({ 2,1,3,4 });
	seznam_cevi.push_back({ 3,1,4,0 });

	seznam_stikal.push_back({ 1, 3, 8, 250, 0 });
	seznam_stikal.push_back({ 1, 3, 7, 0, 1 });

	res = res_reset;

	slider->SetValue(0);

	Refresh();
}

void MainFrame::OnButtonSimClicked(wxCommandEvent& evt) {

	if (slider->GetValue() >= 1000 || slider->GetValue() == 0) {
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
		Sleep(1);
		wxYield();
		i++;
	}
	if (i >= 1000) {
		sim = false;
	}
}

void MainFrame::OnButtonPomClicked(wxCommandEvent& evt) {

	if (oznacitev >= 0) {
		sim = false;
		risCevi = 0;

		if (seznam_valjev[oznacitev][2] == 0) {

			PomoznoOkno* valjNast = new PomoznoOkno();
			valjNast->Show();
		}
		else if (seznam_valjev[oznacitev][2] == 4) {

			KompresorNastavitve* kompNast = new KompresorNastavitve();
			kompNast->Show();
		}
		else if (seznam_valjev[oznacitev][2] == 5) {

			VentilNastavitve* ventNast = new VentilNastavitve();
			ventNast->Show();
		}
		else wxLogStatus("Za ta element ni nastavitev");

		res = res_reset;
		slider->SetValue(0);
		graf.clear();

		Refresh();
	}
	else wxLogStatus("Kliknite na element");
}

void MainFrame::OnShraniClicked(wxCommandEvent& evt) {

	sim = false;
	risCevi = 0;
	res = res_reset;
	slider->SetValue(0);
	graf.clear();

	Refresh();


	std::ofstream shrani;
	shrani.open("Shranjen_Primer_V1.txt", std::ios::out);

	if (shrani.is_open()) {

		shrani << "Zapis zacetnih podatkov simulacije" << std::endl;
		shrani << __DATE__ << ", " << __TIME__ << std::endl << std::endl;


		shrani << "Stevilo elementov: " << seznam_valjev.size() << std::endl;

		for (int i = 0; i < seznam_valjev.size(); i++) {

			shrani << seznam_valjev[i].size() << ": ";
			for (int j = 0; j < seznam_valjev[i].size(); j++) shrani << seznam_valjev[i][j] << " ";
			shrani << std::endl;

			shrani << seznam_lastnosti[i].size() << ": ";
			for (int j = 0; j < seznam_lastnosti[i].size(); j++) shrani << seznam_lastnosti[i][j] << " ";
			shrani << std::endl;

			shrani << res[i].size() << ": ";
			for (int j = 0; j < res[i].size(); j++) shrani << res[i][j] << " ";
			shrani << std::endl << std::endl;
		}
		shrani << std::endl;


		shrani << "Stevilo cevi: " << seznam_cevi.size() << std::endl;

		for (int i = 0; i < seznam_cevi.size(); i++) {

			shrani << seznam_cevi[i].size() << ": ";
			for (int j = 0; j < seznam_cevi[i].size(); j++) shrani << seznam_cevi[i][j] << " ";
			shrani << std::endl;
		}
		shrani << std::endl << std::endl;


		shrani << "Stevilo stikal: " << seznam_stikal.size() << std::endl;

		for (int i = 0; i < seznam_stikal.size(); i++) {

			shrani << seznam_stikal[i].size() << ": ";
			for (int j = 0; j < seznam_stikal[i].size(); j++) shrani << seznam_stikal[i][j] << " ";
			shrani << std::endl;
		}
		shrani << std::endl << std::endl;

		shrani.close();
	}
}

void MainFrame::OnNaloziClicked(wxCommandEvent& evt) {

	std::ifstream nalozi;
	nalozi.open("Shranjen_Primer_V1.txt", std::ios::in);

	if (nalozi.is_open()) {

		sim = false;

		seznam_stikal.clear();
		seznam_cevi.clear();
		seznam_valjev.clear();
		seznam_lastnosti.clear();
		res_reset.clear();
		res = res_reset;
		graf.clear();

		slider->SetValue(0);

		oznacitev = -1;

		Refresh();


		std::string bes;
		char ch;
		int st;

		nalozi >> bes >> bes >> bes >> bes;
		nalozi >> bes >> bes >> bes >> bes;
		nalozi >> bes >> bes;
		nalozi >> st;

		seznam_valjev.resize(st);
		seznam_lastnosti.resize(st);
		res_reset.resize(st);
		
		for (int i = 0; i < st; i++) {

			int pon;

			nalozi >> pon >> ch;
			for (int j = 0; j < pon; j++) { double a; nalozi >> a; seznam_valjev[i].push_back(a); }

			nalozi >> pon >> ch;
			for (int j = 0; j < pon; j++) { double a; nalozi >> a; seznam_lastnosti[i].push_back(a); }

			nalozi >> pon >> ch;
			for (int j = 0; j < pon; j++) { double a; nalozi >> a; res_reset[i].push_back(a); }
		}


		nalozi >> bes >> bes;
		nalozi >> st;

		seznam_cevi.resize(st);

		for (int i = 0; i < st; i++) {

			int pon;

			nalozi >> pon >> ch;
			for (int j = 0; j < pon; j++) { int a; nalozi >> a; seznam_cevi[i].push_back(a); }
		}


		nalozi >> bes >> bes;
		nalozi >> st;

		seznam_stikal.resize(st);

		for (int i = 0; i < st; i++) {

			int pon;

			nalozi >> pon >> ch;
			for (int j = 0; j < pon; j++) { int a; nalozi >> a; seznam_stikal[i].push_back(a); }
		}


		res = res_reset;
	}
	else wxLogStatus("Datoteka ni najdena");


	Refresh();
}

void MainFrame::OnChoicesClicked(wxCommandEvent& evt) {

	Refresh();
}

void MainFrame::OnSliderChanged(wxCommandEvent& evt) {
	sim = false;

	Refresh();
}


void MainFrame::OnPaint(wxPaintEvent& event) {

	wxPaintDC dc(this);


	int deb = 80;
	int vis = 50;
	int deb_ven = 48;
	float n = slider->GetValue();
	int zamik = 0;


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

	/*int ris_x = x_okno + 10; // Glavni razlog za poèasno delovanje programa
	int ris_y = y_okno + 10;
	while (ris_x <= sirina + x_okno) {

		while (ris_y <= visina + y_okno) {

			dc.DrawPoint(ris_x, ris_y);

			ris_y = ris_y + 10;
		}
		ris_y = y_okno;
		ris_x = ris_x + 10;
	}*/
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
		dc.DrawText(wxString::Format("Element %d", seznam_valjev.size() + 1), wxPoint(54, visina_panel - visina_prikaza + 20));
		deb = 80;

		break;

	case 4:

		deb = 40;
		dc.DrawLine(wxPoint(80, visina_panel - visina_prikaza + 70), wxPoint(80 + deb, visina_panel - visina_prikaza + 70));
		dc.DrawLine(wxPoint(80 + deb, visina_panel - visina_prikaza + 70), wxPoint(80 + deb / 2, visina_panel - visina_prikaza + 70 - deb / 3 * 2));
		dc.DrawLine(wxPoint(80 + deb / 2, visina_panel - visina_prikaza + 70 - deb / 3 * 2), wxPoint(80, visina_panel - visina_prikaza + 70));
		dc.DrawText(wxString::Format("Element %d", seznam_valjev.size() + 1), wxPoint(80, visina_panel - visina_prikaza + 32 + deb));
		deb = 80;

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


	if (seznam_valjev.size() > 0) graf.resize(seznam_valjev.size()); // Za pravilno delovanje grafa


	//- STIKALO
	std::vector<std::vector<double*>> stk;
	std::vector<std::vector<int>> dol;

	for (int i = 0; i < seznam_stikal.size(); i++) if (seznam_stikal[i][1] != -1 && seznam_stikal[i][2] != -1) {
		stk.push_back({ &res[seznam_stikal[i][0]][2], &seznam_lastnosti[seznam_stikal[i][1]][seznam_stikal[i][2]] });
		dol.push_back({ seznam_stikal[i][3] });
	}

	stikalo(stk, dol);
	//-


	//- VENTIL
	for (int i = 0; i < seznam_valjev.size(); i++) if (seznam_valjev[i][2] == 5) {

		res[i] = ventil(seznam_lastnosti[i], res[i]);
	}
	//-



	//- CEVI
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());
	if (risCevi == 2) { 
		int zamik = 0;
		int zamik_vis = 0;
		if (seznam_valjev[seznam_cevi[seznam_cevi.size() - 1][0]][2] == 0) {
			if (seznam_cevi[seznam_cevi.size() - 1][1] == 0) zamik = 5;
			else if (seznam_cevi[seznam_cevi.size() - 1][1] == 1) zamik = deb - 5;
			zamik_vis = vis;
		}
		else if (seznam_valjev[seznam_cevi[seznam_cevi.size() - 1][0]][2] == 5) {
			if (seznam_cevi[seznam_cevi.size() - 1][1] == 6) {
				zamik += - 18;
				zamik_vis = deb_ven / 2;
				if (res[seznam_cevi[seznam_cevi.size() - 1][0]][0] == 1) zamik += deb_ven;
			}
			else if (seznam_cevi[seznam_cevi.size() - 1][1] == 7) {
				zamik += 2 * deb_ven + 18;
				zamik_vis = deb_ven / 2;
				if (res[seznam_cevi[seznam_cevi.size() - 1][0]][0] == 1) zamik += deb_ven;
			}
			else {
				if (prvaStevka(seznam_lastnosti[seznam_cevi[seznam_cevi.size() - 1][0]][0]) == 3) zamik = deb_ven / 3;
				else if (prvaStevka(seznam_lastnosti[seznam_cevi[seznam_cevi.size() - 1][0]][0]) == 4) zamik = deb_ven / 3;
				else if (prvaStevka(seznam_lastnosti[seznam_cevi[seznam_cevi.size() - 1][0]][0]) == 5) zamik = deb_ven / 2;
				zamik += deb_ven;
				zamik_vis = deb_ven;
			}
		}
		dc.DrawLine(wxPoint(seznam_valjev[seznam_cevi[seznam_cevi.size() - 1][0]][0] + zamik, seznam_valjev[seznam_cevi[seznam_cevi.size() - 1][0]][1] + zamik_vis), wxPoint(mousePos.x, mousePos.y));
	}

	std::vector<std::vector<double*>> cev;

	int pon = seznam_cevi.size();
	if (risCevi > 0) pon = pon - 1;

	for (int i = 0; i < pon; i++) {

		cev.push_back({ &res[seznam_cevi[i][0]][seznam_cevi[i][1]], &res[seznam_cevi[i][2]][seznam_cevi[i][3]] });

		wxPoint point1(seznam_valjev[seznam_cevi[i][0]][0], seznam_valjev[seznam_cevi[i][0]][1]);
		if (seznam_valjev[seznam_cevi[i][0]][2] == 0) {
			point1.y = point1.y + vis;
			if (seznam_cevi[i][1] == 0) point1.x = point1.x + 5;
			else if (seznam_cevi[i][1] == 1) point1.x = point1.x + deb - 5;
		}
		else if (seznam_valjev[seznam_cevi[i][0]][2] == 5) {
			if (seznam_cevi[i][1] == 6) {
				point1.y = point1.y + deb_ven / 2;
				point1.x = point1.x - 18;
				if (res[seznam_cevi[i][0]][0] == 1) point1.x = point1.x + deb_ven;
			}
			else if (seznam_cevi[i][1] == 7) {
				point1.y = point1.y + deb_ven / 2;
				point1.x = point1.x + 2 * deb_ven + 18;
				if (res[seznam_cevi[i][0]][0] == 1) point1.x = point1.x + deb_ven;
			}
			else {
				point1.y = point1.y + deb_ven;
				if (prvaStevka(seznam_lastnosti[seznam_cevi[i][0]][0]) == 3 || prvaStevka(seznam_lastnosti[seznam_cevi[i][0]][0]) == 4) point1.x = point1.x + deb_ven + deb_ven / 3;
				else if (prvaStevka(seznam_lastnosti[seznam_cevi[i][0]][0]) == 5) point1.x = point1.x + deb_ven + deb_ven / 2;
			}
		}

		wxPoint point2(seznam_valjev[seznam_cevi[i][2]][0], seznam_valjev[seznam_cevi[i][2]][1]);
		if (seznam_valjev[seznam_cevi[i][2]][2] == 5) {
			point2.x = point2.x + deb_ven;
			if (prvaStevka(seznam_lastnosti[seznam_cevi[i][2]][0]) == 3) {
				if (seznam_cevi[i][3] == 2) point2.x = point2.x + deb_ven / 3;
				else if (seznam_cevi[i][3] == 4) { seznam_cevi.erase(seznam_cevi.begin() + i); i--; pon--; }
			}
			else if (prvaStevka(seznam_lastnosti[seznam_cevi[i][2]][0]) == 4) {
				if (seznam_cevi[i][3] == 4) point2.x = point2.x + deb_ven / 3;
				else if (seznam_cevi[i][3] == 2) point2.x = point2.x + deb_ven / 3 * 2;
			}
			else if (prvaStevka(seznam_lastnosti[seznam_cevi[i][2]][0]) == 5) {
				if (seznam_cevi[i][3] == 4) point2.x = point2.x + deb_ven / 4;
				else if (seznam_cevi[i][3] == 2) point2.x = point2.x + deb_ven / 4 * 3;
			}
		}
		else if (seznam_valjev[seznam_cevi[i][2]][2] == 4) {
			point2.x = point2.x + 20;
		}

		dc.DrawLine(point1, point2);
	}

	cevka(cev);
	//-


	//- ADMIN "LOGS"
	if (false) {
		if (seznam_cevi.size() > 0) dc.DrawText(wxString::Format("seznam = %d | %d | %d | %d ", seznam_cevi[0][0], seznam_cevi[0][1], seznam_cevi[0][2], seznam_cevi[0][3]), wxPoint(240, 20));
		if (seznam_cevi.size() > 1) dc.DrawText(wxString::Format("seznam = %d | %d | %d | %d ", seznam_cevi[1][0], seznam_cevi[1][1], seznam_cevi[1][2], seznam_cevi[1][3]), wxPoint(240, 35));
		if (seznam_cevi.size() > 2) dc.DrawText(wxString::Format("seznam = %d | %d | %d | %d ", seznam_cevi[2][0], seznam_cevi[2][1], seznam_cevi[2][2], seznam_cevi[2][3]), wxPoint(240, 50));
		if (seznam_cevi.size() > 3) dc.DrawText(wxString::Format("seznam = %d | %d | %d | %d ", seznam_cevi[3][0], seznam_cevi[3][1], seznam_cevi[3][2], seznam_cevi[3][3]), wxPoint(240, 65));

		if (seznam_cevi.size() > 0) dc.DrawText(wxString::Format("seznam = %g | %g ", res[seznam_cevi[0][0]][seznam_cevi[0][1]], res[seznam_cevi[0][2]][seznam_cevi[0][3]]), wxPoint(420, 20));
		if (seznam_cevi.size() > 1) dc.DrawText(wxString::Format("seznam = %g | %g ", res[seznam_cevi[1][0]][seznam_cevi[1][1]], res[seznam_cevi[1][2]][seznam_cevi[1][3]]), wxPoint(420, 35));
		if (seznam_cevi.size() > 2) dc.DrawText(wxString::Format("seznam = %g | %g ", res[seznam_cevi[2][0]][seznam_cevi[2][1]], res[seznam_cevi[2][2]][seznam_cevi[2][3]]), wxPoint(420, 50));
		if (seznam_cevi.size() > 3) dc.DrawText(wxString::Format("seznam = %g | %g ", res[seznam_cevi[3][0]][seznam_cevi[3][1]], res[seznam_cevi[3][2]][seznam_cevi[3][3]]), wxPoint(420, 65));

		dc.DrawText(wxString::Format("stikala = %d ", seznam_stikal.size()), wxPoint(600, 5));
		dc.DrawText(wxString::Format("valji = %d ", seznam_valjev.size()), wxPoint(600, 20));
		dc.DrawText(wxString::Format("last. = %d ", seznam_lastnosti.size()), wxPoint(600, 35));
		dc.DrawText(wxString::Format("res = %d ", res.size()), wxPoint(600, 50));
		dc.DrawText(wxString::Format("res_reset. = %d ", res_reset.size()), wxPoint(600, 65));
		dc.DrawText(wxString::Format("st. cevi = %d ", seznam_cevi.size()), wxPoint(600, 80));

		for (int i = 0; i < seznam_stikal.size(); i++) dc.DrawText(wxString::Format("seznam stikal: %d | %d | %d | %d | %d", seznam_stikal[i][0], seznam_stikal[i][1], seznam_stikal[i][2], seznam_stikal[i][3], seznam_stikal[i][4]), wxPoint(700, 10 + i * 15));
	}
	//-


	//- IZRIS VALJEV
	for (int i = 0; i < seznam_valjev.size(); i++) {

		switch (seznam_valjev[i][2]) {

		case 0:

			if (sim == true) res[i] = izracun3(n, seznam_valjev[i][2], seznam_lastnosti[i], res[i]);

			graf[i].push_back(res[i]); /////////////// bat in graf sta nesinhronizirana ////////////////// zaradi Refresh(); ko je sim == false, se ustvarijo praznine nekje v Graf
			

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

			dc.DrawText(wxString::Format("p1 = %g bar", res[i][0]), wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + vis)); // Tlak v levem
			dc.DrawText(wxString::Format("p2 = %g bar", res[i][1]), wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + vis + 12)); // Tlak v desnem

			// Vrednosti
			if (seznam_lastnosti[i][20] == 1) {

				dc.DrawText(wxString::Format("st: %g", graf[i][graf[i].size() - 1][7]), wxPoint(seznam_valjev[i][0] - 110, seznam_valjev[i][1] - 12));
				dc.DrawText(wxString::Format("x = %g", res[i][2]), wxPoint(seznam_valjev[i][0] - 110, seznam_valjev[i][1]));
				dc.DrawText(wxString::Format("v = %g", res[i][3]), wxPoint(seznam_valjev[i][0] - 110, seznam_valjev[i][1] + 12));
				dc.DrawText(wxString::Format("a = %g", res[i][4]), wxPoint(seznam_valjev[i][0] - 110, seznam_valjev[i][1] + 24));
				dc.DrawText(wxString::Format("V_l = %g", res[i][5]), wxPoint(seznam_valjev[i][0] - 110, seznam_valjev[i][1] + 36));
				dc.DrawText(wxString::Format("V_d = %g", res[i][6]), wxPoint(seznam_valjev[i][0] - 110, seznam_valjev[i][1] + 48));
			}
			
			// Stikala:
			for (int j = 0; j < seznam_stikal.size(); j++) if (seznam_stikal[j][0] == i) {

				dc.DrawCircle(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + deb / 8 * 7 - 5 + seznam_stikal[j][3] * (deb / 8 * 5) / seznam_lastnosti[i][11], seznam_valjev[i][1] + vis / 5 * 3 + 8), 5);
				dc.DrawCircle(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + deb / 8 * 7 - 5 + seznam_stikal[j][3] * (deb / 8 * 5) / seznam_lastnosti[i][11], seznam_valjev[i][1] + vis / 5 * 3 + 8), 2);
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + deb / 8 * 7 - 5 + seznam_stikal[j][3] * (deb / 8 * 5) / seznam_lastnosti[i][11], seznam_valjev[i][1] + vis / 5 * 3 + 13),
							wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + deb / 8 * 7 - 5 + seznam_stikal[j][3] * (deb / 8 * 5) / seznam_lastnosti[i][11], seznam_valjev[i][1] + vis / 5 * 3 + 22));
				dc.DrawText(wxString::Format("%d_%d", seznam_stikal[j][0], seznam_stikal[j][4]), wxPoint(seznam_valjev[i][0] + deb / 8 * 2 + deb / 8 * 7 - 5 + seznam_stikal[j][3] * (deb / 8 * 5) / seznam_lastnosti[i][11] - 4, seznam_valjev[i][1] + vis / 5 * 3 + 24));
			}

			// Cevi
			if (risCevi == 1) {

				dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));
				if (seznam_lastnosti[i][4] < 0) dc.DrawRectangle(wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + vis + 1), wxSize(10 + 1, 10 + 1));
				if (seznam_lastnosti[i][5] < 0) dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb - 10, seznam_valjev[i][1] + vis + 1), wxSize(10 + 1, 10 + 1));
				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
			}


			// Graf:
			if (seznam_lastnosti[i][17] > 0) {

				dc.SetPen(wxPen(wxColour(204, 204, 204), 1, wxPENSTYLE_SOLID));
				dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + 190, seznam_valjev[i][1]), wxSize(350 + 1, 80 + 1));
				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

				if (graf[i].size() > 1 && graf[i][graf[i].size() - 1][7] <= 400) {
					for (int j = 1; j < graf[i][graf[i].size() - 1][7]; j++) {
						dc.DrawLine(wxPoint(seznam_valjev[i][0] + 190 + graf[i][j - 1][7], seznam_valjev[i][1] + 80 - graf[i][j - 1][2] * 80 / seznam_lastnosti[i][11]), wxPoint(seznam_valjev[i][0] + 190 + graf[i][j][7], seznam_valjev[i][1] + 80 - graf[i][j][2] * 80 / seznam_lastnosti[i][11]));
					}
				}
				else if (graf[i][graf[i].size() - 1][7] > 400) {
					int zamik = graf[i][graf[i][graf[i].size() - 1][7] - 400][7] - 1;
					for (int j = graf[i][graf[i].size() - 1][7] - 400; j < graf[i][graf[i].size() - 1][7]; j++) {
						dc.DrawLine(wxPoint(seznam_valjev[i][0] + 190 + graf[i][j - 1][7] - zamik, seznam_valjev[i][1] + 80 - graf[i][j - 1][2] * 80 / seznam_lastnosti[i][11]), wxPoint(seznam_valjev[i][0] + 190 + graf[i][j][7] - zamik, seznam_valjev[i][1] + 80 - graf[i][j][2] * 80 / seznam_lastnosti[i][11]));
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

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] - 16)); // Ime
			deb = 80;

			break;

		case 4:

			deb = 40;
			dc.DrawLine(wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + deb / 3 * 2), wxPoint(seznam_valjev[i][0] + deb, seznam_valjev[i][1] + deb / 3 * 2));
			dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb, seznam_valjev[i][1] + deb / 3 * 2), wxPoint(seznam_valjev[i][0] + deb / 2, seznam_valjev[i][1]));
			dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb / 2, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + deb / 3 * 2));

			dc.DrawText(wxString::Format("p = %g bar", res[i][0]), wxPoint(seznam_valjev[i][0] + deb - 5, seznam_valjev[i][1]));
			
			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(seznam_valjev[i][0], seznam_valjev[i][1] + 44 / 3 * 2)); // Ime
			deb = 80;
			
			if (risCevi == 2) {
				dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));
				dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + 15, seznam_valjev[i][1] - 11), wxSize(10 + 1, 10 + 1));
				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
			}

			break;

		case 5:

			//- ADMIN "LOGS"
			if (false) {
				dc.DrawText(wxString::Format("Naziv: %g", seznam_lastnosti[i][0]), wxPoint(seznam_valjev[i][0] + 120, seznam_valjev[i][1]));
				dc.DrawText(wxString::Format("Gumb L: %g", seznam_lastnosti[i][1]), wxPoint(seznam_valjev[i][0] + 120, seznam_valjev[i][1] + 12));
				dc.DrawText(wxString::Format("Gumb D: %g", seznam_lastnosti[i][2]), wxPoint(seznam_valjev[i][0] + 120, seznam_valjev[i][1] + 24));
				dc.DrawText(wxString::Format("Pnev L: %g", seznam_lastnosti[i][3]), wxPoint(seznam_valjev[i][0] + 120, seznam_valjev[i][1] + 36));
				dc.DrawText(wxString::Format("Pnev D: %g", seznam_lastnosti[i][4]), wxPoint(seznam_valjev[i][0] + 120, seznam_valjev[i][1] + 48));
				dc.DrawText(wxString::Format("Vzme L: %g", seznam_lastnosti[i][5]), wxPoint(seznam_valjev[i][0] + 120, seznam_valjev[i][1] + 60));
				dc.DrawText(wxString::Format("Vzme D: %g", seznam_lastnosti[i][6]), wxPoint(seznam_valjev[i][0] + 120, seznam_valjev[i][1] + 72));
				dc.DrawText(wxString::Format("Stik L: %g", seznam_lastnosti[i][7]), wxPoint(seznam_valjev[i][0] + 120, seznam_valjev[i][1] + 84));
				dc.DrawText(wxString::Format("Stik D: %g", seznam_lastnosti[i][8]), wxPoint(seznam_valjev[i][0] + 120, seznam_valjev[i][1] + 96));
				dc.DrawText(wxString::Format("res: %g", res[i][0]), wxPoint(seznam_valjev[i][0] + 200, seznam_valjev[i][1]));
				dc.DrawText(wxString::Format("res: %g", res[i][1]), wxPoint(seznam_valjev[i][0] + 200, seznam_valjev[i][1] + 12));
				dc.DrawText(wxString::Format("res: %g", res[i][2]), wxPoint(seznam_valjev[i][0] + 200, seznam_valjev[i][1] + 24));
				dc.DrawText(wxString::Format("res: %g", res[i][3]), wxPoint(seznam_valjev[i][0] + 200, seznam_valjev[i][1] + 36));
				dc.DrawText(wxString::Format("res: %g", res[i][4]), wxPoint(seznam_valjev[i][0] + 200, seznam_valjev[i][1] + 48));
				dc.DrawText(wxString::Format("res: %g", res[i][5]), wxPoint(seznam_valjev[i][0] + 200, seznam_valjev[i][1] + 60));
				dc.DrawText(wxString::Format("res: %g", res[i][6]), wxPoint(seznam_valjev[i][0] + 200, seznam_valjev[i][1] + 72));
				dc.DrawText(wxString::Format("res: %g", res[i][7]), wxPoint(seznam_valjev[i][0] + 200, seznam_valjev[i][1] + 84));
			}

			if (res[i][0] == 1) zamik = deb_ven;
			else zamik = 0;

			dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + zamik, seznam_valjev[i][1]), wxSize(deb_ven + 1, deb_ven + 1)); // celica 0
			dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb_ven + zamik, seznam_valjev[i][1]), wxSize(deb_ven + 1, deb_ven + 1)); // celica 1
			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(seznam_valjev[i][0] + zamik, seznam_valjev[i][1] - 16)); // naziv


			if (prvaStevka(seznam_lastnosti[i][0]) == 3) {

				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven / 3 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5), wxPoint(seznam_valjev[i][0] + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 3 * 2 - 4 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5), wxPoint(seznam_valjev[i][0] + deb_ven / 3 * 2 + 4 + 1 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven / 3 + 6 + zamik, seznam_valjev[i][1] + 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven / 3 - 6 + zamik, seznam_valjev[i][1] + 6));

				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 - 4 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 + 4 + 1 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1] + deb_ven), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + 6 + zamik, seznam_valjev[i][1] + deb_ven - 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1] + deb_ven), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 - 6 + zamik, seznam_valjev[i][1] + deb_ven - 6));

				if (risCevi == 1) {
					dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 - 5, seznam_valjev[i][1] + deb_ven), wxSize(10 + 1, 10 + 1));
					dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
					dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				}

				if (risCevi == 2) {
					dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 - 5, seznam_valjev[i][1] - 11), wxSize(10 + 1, 10 + 1));
					dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
					dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				}
			}
			else if (prvaStevka(seznam_lastnosti[i][0]) == 4) {

				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven / 3 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven / 3 + 6 + zamik, seznam_valjev[i][1] + 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven / 3 - 6 + zamik, seznam_valjev[i][1] + 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1] + deb_ven), wxPoint(seznam_valjev[i][0] + deb_ven / 3 * 2 + 6 + zamik, seznam_valjev[i][1] + deb_ven - 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1] + deb_ven), wxPoint(seznam_valjev[i][0] + deb_ven / 3 * 2 - 6 + zamik, seznam_valjev[i][1] + deb_ven - 6));

				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1] + deb_ven), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + 6 + zamik, seznam_valjev[i][1] + deb_ven - 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1] + deb_ven), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 - 6 + zamik, seznam_valjev[i][1] + deb_ven - 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + 6 + zamik, seznam_valjev[i][1] + 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 - 6 + zamik, seznam_valjev[i][1] + 6));

				if (risCevi == 1) {
					dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 - 5, seznam_valjev[i][1] + deb_ven), wxSize(10 + 1, 10 + 1));
					dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
					dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				}

				if (risCevi == 2) {
					dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 - 5, seznam_valjev[i][1] - 11), wxSize(10 + 1, 10 + 1));
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 3 * 2 - 5, seznam_valjev[i][1] - 11), wxSize(10 + 1, 10 + 1));
					dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
					dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				}
			}
			else if (prvaStevka(seznam_lastnosti[i][0]) == 5) {

				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 4 * 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven / 4 * 3 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 4 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven / 2 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 4 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5), wxPoint(seznam_valjev[i][0] + deb_ven / 4 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 4 - 4 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5), wxPoint(seznam_valjev[i][0] + deb_ven / 4 + 4 + 1 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 4 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven / 4 + 6 + zamik, seznam_valjev[i][1] + 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 4 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven / 4 - 6 + zamik, seznam_valjev[i][1] + 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 4 * 3 + zamik, seznam_valjev[i][1] + deb_ven), wxPoint(seznam_valjev[i][0] + deb_ven / 4 * 3 + 6 + zamik, seznam_valjev[i][1] + deb_ven - 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven / 4 * 3 + zamik, seznam_valjev[i][1] + deb_ven), wxPoint(seznam_valjev[i][0] + deb_ven / 4 * 3 - 6 + zamik, seznam_valjev[i][1] + deb_ven - 6));

				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 * 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 2 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 * 3 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 * 3 + zamik, seznam_valjev[i][1] + deb_ven));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 * 3 - 4 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 * 3 + 4 + 1 + zamik, seznam_valjev[i][1] + deb_ven / 6 * 5));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 + zamik, seznam_valjev[i][1] + deb_ven), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 + 6 + zamik, seznam_valjev[i][1] + deb_ven - 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 + zamik, seznam_valjev[i][1] + deb_ven), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 - 6 + zamik, seznam_valjev[i][1] + deb_ven - 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 * 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 * 3 + 6 + zamik, seznam_valjev[i][1] + 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 * 3 + zamik, seznam_valjev[i][1]), wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 * 3 - 6 + zamik, seznam_valjev[i][1] + 6));

				if (risCevi == 1) {
					dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 2 - 5, seznam_valjev[i][1] + deb_ven), wxSize(10 + 1, 10 + 1));
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] - 18 - 5 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 5), wxSize(10 + 1, 10 + 1));
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 18 - 5 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 5), wxSize(10 + 1, 10 + 1));
					dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
					dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				}

				if (risCevi == 2) {
					dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 - 5, seznam_valjev[i][1] - 11), wxSize(10 + 1, 10 + 1));
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + deb_ven + deb_ven / 4 * 3 - 5, seznam_valjev[i][1] - 11), wxSize(10 + 1, 10 + 1));
					dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
					dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				}
			}

			if (seznam_lastnosti[i][1] >= 0) {
				dc.DrawRectangle(wxPoint(seznam_valjev[i][0] - deb_ven / 4 + zamik, seznam_valjev[i][1] + 5), wxSize(deb_ven / 4 + 1, deb_ven / 6 + 1));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] - deb_ven / 4 + zamik, seznam_valjev[i][1] + 6 - 4), wxPoint(seznam_valjev[i][0] - deb_ven / 4 + zamik, seznam_valjev[i][1] + deb_ven / 6 + 9));
			}
			if (seznam_lastnosti[i][2] >= 0) {
				dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + zamik, seznam_valjev[i][1] + 5), wxSize(deb_ven / 4 + 1, deb_ven / 6 + 1));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + deb_ven / 4 + zamik, seznam_valjev[i][1] + 6 - 4), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + deb_ven / 4 + zamik, seznam_valjev[i][1] + deb_ven / 6 + 9));
			}
			if (seznam_lastnosti[i][3] >= 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + zamik, seznam_valjev[i][1] + deb_ven / 2), wxPoint(seznam_valjev[i][0] - 6 + zamik, seznam_valjev[i][1] + deb_ven / 2));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] - 6 + zamik, seznam_valjev[i][1] + deb_ven / 2), wxPoint(seznam_valjev[i][0] - 12 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] - 12 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 6), wxPoint(seznam_valjev[i][0] - 12 + zamik, seznam_valjev[i][1] + deb_ven / 2 + 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] - 12 + zamik, seznam_valjev[i][1] + deb_ven / 2 + 6), wxPoint(seznam_valjev[i][0] - 6 + zamik, seznam_valjev[i][1] + deb_ven / 2));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] - 12 + zamik, seznam_valjev[i][1] + deb_ven / 2), wxPoint(seznam_valjev[i][0] - 18 + zamik, seznam_valjev[i][1] + deb_ven / 2));
			}
			if (seznam_lastnosti[i][4] >= 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + zamik, seznam_valjev[i][1] + deb_ven / 2), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 6 + zamik, seznam_valjev[i][1] + deb_ven / 2));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 6 + zamik, seznam_valjev[i][1] + deb_ven / 2), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 12 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 12 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 6), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 12 + zamik, seznam_valjev[i][1] + deb_ven / 2 + 6));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 12 + zamik, seznam_valjev[i][1] + deb_ven / 2 + 6), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 6 + zamik, seznam_valjev[i][1] + deb_ven / 2));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 12 + zamik, seznam_valjev[i][1] + deb_ven / 2), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 18 + zamik, seznam_valjev[i][1] + deb_ven / 2));
			}
			if (seznam_lastnosti[i][5] > 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + zamik, seznam_valjev[i][1] + deb_ven - 2), wxPoint(seznam_valjev[i][0] - 4 + zamik, seznam_valjev[i][1] + deb_ven - 10));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] - 4 + zamik, seznam_valjev[i][1] + deb_ven - 10), wxPoint(seznam_valjev[i][0] - 8 + zamik, seznam_valjev[i][1] + deb_ven - 2));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] - 8 + zamik, seznam_valjev[i][1] + deb_ven - 2), wxPoint(seznam_valjev[i][0] - 12 + zamik, seznam_valjev[i][1] + deb_ven - 10));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] - 12 + zamik, seznam_valjev[i][1] + deb_ven - 10), wxPoint(seznam_valjev[i][0] - 16 + zamik, seznam_valjev[i][1] + deb_ven - 2));
			}
			if (seznam_lastnosti[i][6] > 0) {
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + zamik, seznam_valjev[i][1] + deb_ven - 2), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 4 + zamik, seznam_valjev[i][1] + deb_ven - 10));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 4 + zamik, seznam_valjev[i][1] + deb_ven - 10), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 8 + zamik, seznam_valjev[i][1] + deb_ven - 2));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 8 + zamik, seznam_valjev[i][1] + deb_ven - 2), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 12 + zamik, seznam_valjev[i][1] + deb_ven - 10));
				dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 12 + zamik, seznam_valjev[i][1] + deb_ven - 10), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 16 + zamik, seznam_valjev[i][1] + deb_ven - 2));
			}
			//- Stikala
			for (int j = 0; j < seznam_stikal.size(); j++) if (seznam_stikal[j][1] == i) {
				if (seznam_stikal[j][2] == 8) {
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] - 12 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 3), wxSize(12 + 1, 6 + 1));
					dc.DrawCircle(wxPoint(seznam_valjev[i][0] - 12 + zamik, seznam_valjev[i][1] + deb_ven / 2), 5);
					dc.DrawCircle(wxPoint(seznam_valjev[i][0] - 12 + zamik, seznam_valjev[i][1] + deb_ven / 2), 2);
					dc.DrawText(wxString::Format("%d_%d", seznam_stikal[j][0], seznam_stikal[j][4]), wxPoint(seznam_valjev[i][0] - 24 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 24));
					if (seznam_lastnosti[i][8] >= 0) {
						dc.DrawLine(wxPoint(seznam_valjev[i][0] - 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 8), wxPoint(seznam_valjev[i][0] - 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 + 12));
						dc.DrawLine(wxPoint(seznam_valjev[i][0] - 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 8), wxPoint(seznam_valjev[i][0] - 17 + zamik - 5, seznam_valjev[i][1] + deb_ven / 2 - 8 + 5));
						dc.DrawLine(wxPoint(seznam_valjev[i][0] - 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 8 + 5), wxPoint(seznam_valjev[i][0] - 17 + zamik - 5, seznam_valjev[i][1] + deb_ven / 2 - 8 + 10));
						dc.DrawLine(wxPoint(seznam_valjev[i][0] - 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 8 + 10), wxPoint(seznam_valjev[i][0] - 17 + zamik - 5, seznam_valjev[i][1] + deb_ven / 2 - 8 + 15));
						dc.DrawLine(wxPoint(seznam_valjev[i][0] - 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 8 + 15), wxPoint(seznam_valjev[i][0] - 17 + zamik - 5, seznam_valjev[i][1] + deb_ven / 2 - 8 + 20));
					}
				}

				if (seznam_stikal[j][2] == 7) {
					dc.DrawRectangle(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + zamik, seznam_valjev[i][1] + deb_ven / 2 - 3), wxSize(12 + 1, 6 + 1));
					dc.DrawCircle(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 12 + zamik, seznam_valjev[i][1] + deb_ven / 2), 5);
					dc.DrawCircle(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 12 + zamik, seznam_valjev[i][1] + deb_ven / 2), 2);
					dc.DrawText(wxString::Format("%d_%d", seznam_stikal[j][0], seznam_stikal[j][4]), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + zamik + 8, seznam_valjev[i][1] + deb_ven / 2 - 24));
					if (seznam_lastnosti[i][7] >= 0) {
						dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 8), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 + 12));
						dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 8), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 17 + zamik + 5, seznam_valjev[i][1] + deb_ven / 2 - 8 + 5));
						dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 8 + 5), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 17 + zamik + 5, seznam_valjev[i][1] + deb_ven / 2 - 8 + 10));
						dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 8 + 10), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 17 + zamik + 5, seznam_valjev[i][1] + deb_ven / 2 - 8 + 15));
						dc.DrawLine(wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 17 + zamik, seznam_valjev[i][1] + deb_ven / 2 - 8 + 15), wxPoint(seznam_valjev[i][0] + 2 * deb_ven + 17 + zamik + 5, seznam_valjev[i][1] + deb_ven / 2 - 8 + 20));
					}
				}
			}

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
wxCheckBox* izpisVrednosti;

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
	wxPoint pointIzpisVrednosti = wxPoint(pointPrikazGrafa.x, pointPrikazGrafa.y + 20);
	izpisVrednosti = new wxCheckBox(panel, wxID_ANY, "Izpis vrednosti", pointIzpisVrednosti, wxDefaultSize);
	if (seznam_lastnosti[oznacitev][20] > 0) izpisVrednosti->SetValue(true);
	else izpisVrednosti->SetValue(false);

	wxButton* button = new wxButton(panel, wxID_ANY, "Aplly", wxPoint(size.x / 2 - 40, size.y - 64), wxSize(80, -1));
	wxButton* stkNast = new wxButton(panel, wxID_ANY, "Nastavite\n koncnih stikal", wxPoint(size.x / 2 - 60, size.y - 104), wxSize(120, 40));


	button->Bind(wxEVT_BUTTON, &PomoznoOkno::OnButtonClicked, this);
	stkNast->Bind(wxEVT_BUTTON, &PomoznoOkno::OnStikaloClicked, this);
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
		//	   bat_premer, izris_grafa, koncno_stikalo_leva, koncno_stikalo_desna, izpis_vrednosti]
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
		if (izpisVrednosti->IsChecked() == true) seznam_lastnosti[oznacitev][20] = 1;
		else seznam_lastnosti[oznacitev][20] = -1;

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

void PomoznoOkno::OnStikaloClicked(wxCommandEvent& evt) {
	sim = false;

	if (oznacitev >= 0) {

		StikaloNastavitev* stikaloOkno = new StikaloNastavitev();
		stikaloOkno->Show();

		res = res_reset;
		slider->SetValue(0);
		graf.clear();

		Refresh();
	}
	else wxLogStatus("Izberite element");

	Refresh();
}

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



int st_Stikal = 0;

wxSlider* stikaloSlider;

StikaloNastavitev::StikaloNastavitev() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve ventila"), wxPoint(0, 0), wxSize(400, 236)) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
	wxSize size = this->GetSize();


	wxButton* stkBrisi = new wxButton(panel, wxID_ANY, "Izbrisi stikala", wxPoint(size.x / 2 - 200 + 2, 170), wxSize(120, -1));
	wxButton* stkAplly = new wxButton(panel, wxID_ANY, "Dodaj stikalo", wxPoint(size.x / 2 - 70 + 2, 170), wxSize(120, -1));
	wxButton* stkZapri = new wxButton(panel, wxID_ANY, "Zapri", wxPoint(size.x / 2 + 60 + 2, 170), wxSize(120, -1));

	stikaloSlider = new wxSlider(panel, wxID_ANY, 0, 0, seznam_lastnosti[oznacitev][11], wxPoint(size.x / 2 - 100, 115), wxSize(200, -1), wxSL_VALUE_LABEL);


	panel->Bind(wxEVT_SIZE, &StikaloNastavitev::OnSizeChanged, this);
	stkBrisi->Bind(wxEVT_BUTTON, &StikaloNastavitev::OnBrisiClicked, this);
	stkAplly->Bind(wxEVT_BUTTON, &StikaloNastavitev::OnApllyClicked, this);
	stikaloSlider->Bind(wxEVT_SLIDER, &StikaloNastavitev::OnSliderChanged, this);

	
	st_Stikal = 0;
	for (int i = 0; i < seznam_stikal.size(); i++) if (seznam_stikal[i][0] == oznacitev) st_Stikal++;


	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(StikaloNastavitev::OnPaint));

	panel->SetDoubleBuffered(true);
}


void StikaloNastavitev::OnSizeChanged(wxSizeEvent& evt) {

	Refresh();
}

void StikaloNastavitev::OnBrisiClicked(wxCommandEvent& evt) {

	for (int i = 0; i < seznam_stikal.size(); i++) {
		if (seznam_stikal[i][0] == oznacitev) {
			seznam_stikal.erase(seznam_stikal.begin() + i);
			i--;
		}
	}

	Refresh();
}

void StikaloNastavitev::OnApllyClicked(wxCommandEvent& evt) {

	seznam_stikal.push_back({ oznacitev, -1, -1, stikaloSlider->GetValue(), st_Stikal });
	st_Stikal++;

	Refresh();
}

void StikaloNastavitev::OnSliderChanged(wxCommandEvent& evt) {

	Refresh();
}


void StikaloNastavitev::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);

	wxSize size = this->GetSize();

	int deb = 80;
	int vis = 50;


	dc.DrawRectangle(wxPoint(0, 0), wxSize(size.x, size.y / 3 * 1.6));

	int pomik = stikaloSlider->GetValue() * (deb / 8 * 5) / seznam_lastnosti[oznacitev][11];

	dc.DrawRectangle(wxPoint(size.x / 2 - deb / 2, size.y / 6.7 * 1), wxSize(deb + 1, vis + 1)); // Ohišje
	dc.DrawLine(wxPoint(size.x / 2 - deb / 2, size.y / 6.7 * 1 + vis), wxPoint(size.x / 2 - deb / 2 + 5, size.y / 6.7 * 1 + vis - 5)); // Ventili
	dc.DrawLine(wxPoint(size.x / 2 - deb / 2 + 5, size.y / 6.7 * 1 + vis - 5), wxPoint(size.x / 2 - deb / 2 + deb / 8 * 1, size.y / 6.7 * 1 + vis));
	dc.DrawLine(wxPoint(size.x / 2 - deb / 2 + deb / 8 * 7, size.y / 6.7 * 1 + vis), wxPoint(size.x / 2 - deb / 2 + deb / 8 * 7 + deb / 16, size.y / 6.7 * 1 + vis - deb / 16));
	dc.DrawLine(wxPoint(size.x / 2 - deb / 2 + deb / 8 * 7 + deb / 16, size.y / 6.7 * 1 + vis - deb / 16), wxPoint(size.x / 2 - deb / 2 + deb, size.y / 6.7 * 1 + vis));
	dc.DrawCircle(wxPoint(size.x / 2 - deb / 2 + deb / 8 * 2 + deb / 8 * 7 - 5 + pomik, size.y / 6.7 * 1 + vis / 5 * 3), 4); // Stikalo na batnici
	dc.DrawRectangle(wxPoint(size.x / 2 - deb / 2 + deb / 8 * 1 + pomik, size.y / 6.7 * 1), wxSize(deb / 8 + 1, vis + 1)); // Bat
	dc.DrawRectangle(wxPoint(size.x / 2 - deb / 2 + deb / 8 * 2 + pomik, size.y / 6.7 * 1 + vis / 5 * 2), wxSize(deb / 8 * 7 + 1, vis / 5 + 1)); // Batnica
	dc.DrawLine(wxPoint(size.x / 2 - deb / 2 + deb / 8 * 2, size.y / 6.7 * 1 + vis), wxPoint(size.x / 2 - deb / 2 + deb / 8 * 2 + (deb - deb / 8 * 2) * 1 / 3, size.y / 6.7 * 1)); // Vzmet
	dc.DrawLine(wxPoint(size.x / 2 - deb / 2 + deb / 8 * 2 + (deb - deb / 8 * 2) * 1 / 3, size.y / 6.7 * 1), wxPoint(size.x / 2 - deb / 2 + deb / 8 * 2 + (deb - deb / 8 * 2) * 2 / 3, size.y / 6.7 * 1 + vis));
	dc.DrawLine(wxPoint(size.x / 2 - deb / 2 + deb / 8 * 2 + (deb - deb / 8 * 2) * 2 / 3, size.y / 6.7 * 1 + vis), wxPoint(size.x / 2 - deb / 2 + deb, size.y / 6.7 * 1));
	dc.DrawText(wxString::Format("Element %d", oznacitev), wxPoint(size.x / 2 - deb / 2, size.y / 6.7 * 1 - 16));

	for (int i = 0; i < seznam_stikal.size(); i++) if (seznam_stikal[i][0] == oznacitev) {

		dc.DrawCircle(wxPoint(size.x / 2 - deb / 2 + deb / 8 * 2 + deb / 8 * 7 - 5 + seznam_stikal[i][3] * (deb / 8 * 5) / seznam_lastnosti[oznacitev][11], size.y / 6.7 * 1 + vis / 5 * 3 + 8), 5);
		dc.DrawCircle(wxPoint(size.x / 2 - deb / 2 + deb / 8 * 2 + deb / 8 * 7 - 5 + seznam_stikal[i][3] * (deb / 8 * 5) / seznam_lastnosti[oznacitev][11], size.y / 6.7 * 1 + vis / 5 * 3 + 8), 2);
		dc.DrawLine(wxPoint(size.x / 2 - deb / 2 + deb / 8 * 2 + deb / 8 * 7 - 5 + seznam_stikal[i][3] * (deb / 8 * 5) / seznam_lastnosti[oznacitev][11], size.y / 6.7 * 1 + vis / 5 * 3 + 13), 
					wxPoint(size.x / 2 - deb / 2 + deb / 8 * 2 + deb / 8 * 7 - 5 + seznam_stikal[i][3] * (deb / 8 * 5) / seznam_lastnosti[oznacitev][11], size.y / 6.7 * 1 + vis / 5 * 3 + 22));
	}
	
	//- ADMIN "LOGS"
	if (false) {
		dc.DrawText(wxString::Format("oznacitev = %d", oznacitev), wxPoint(10, 10));
		dc.DrawText(wxString::Format("st. stikal = %d", st_Stikal), wxPoint(10, 25));
		for (int i = 0; i < seznam_stikal.size(); i++) dc.DrawText(wxString::Format("seznam stikal: %d | %d | %d | %d | %d", seznam_stikal[i][0], seznam_stikal[i][1], seznam_stikal[i][2], seznam_stikal[i][3], seznam_stikal[i][4]), wxPoint(240, 10 + i * 15));
	}
}


wxArrayString koncnaStikla;
wxCheckListBox* levaLastnostVentil;
wxCheckListBox* desnaLastnostVentil;
wxChoice* nazivVentila;
wxChoice* stikLeva;
wxChoice* stikDesna;


VentilNastavitve::VentilNastavitve() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve ventila"), wxPoint(0, 0), wxSize(540, 240)) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
	wxSize size = this->GetSize();

	wxButton* aplly = new wxButton(panel, wxID_ANY, "Aplly", wxPoint(size.x / 2 - 60, size.y - 100), wxSize(120, 40));


	wxArrayString lastnosti;
	lastnosti.Add("Gumb");
	lastnosti.Add("Tlacno");
	lastnosti.Add("Vzmet");

	wxPoint pointVentil = wxPoint(10, 10);
	levaLastnostVentil = new wxCheckListBox(panel, wxID_ANY, pointVentil, wxDefaultSize, lastnosti);
	if (seznam_lastnosti[oznacitev][1] >= 0) levaLastnostVentil->Check(0);
	if (seznam_lastnosti[oznacitev][3] >= 0) levaLastnostVentil->Check(1);
	if (seznam_lastnosti[oznacitev][5] > 0) levaLastnostVentil->Check(2);

	pointVentil = wxPoint(pointVentil.x + size.x / 3 * 2, pointVentil.y);
	desnaLastnostVentil = new wxCheckListBox(panel, wxID_ANY, pointVentil, wxDefaultSize, lastnosti);
	if (seznam_lastnosti[oznacitev][2] >= 0) desnaLastnostVentil->Check(0);
	if (seznam_lastnosti[oznacitev][4] >= 0) desnaLastnostVentil->Check(1);
	if (seznam_lastnosti[oznacitev][6] > 0) desnaLastnostVentil->Check(2);

	wxArrayString nazivi;
	nazivi.Add("3/2 ventil");
	nazivi.Add("4/2 ventil");
	nazivi.Add("5/2 ventil");

	nazivVentila = new wxChoice(panel, wxID_ANY, wxPoint(size.x / 2 - 55, pointVentil.y), wxSize(96, -1), nazivi);
	if (seznam_lastnosti[oznacitev][0] == 32) nazivVentila->SetSelection(0);
	else if (seznam_lastnosti[oznacitev][0] == 42) nazivVentila->SetSelection(1);
	else if (seznam_lastnosti[oznacitev][0] == 52) nazivVentila->SetSelection(2);
	else nazivVentila->SetSelection(-1);

	
	koncnaStikla.Clear();
	std::sort(seznam_stikal.begin(), seznam_stikal.end());
	for (int i = 0; i < seznam_stikal.size(); i++) koncnaStikla.Add(wxString::Format("Stikalo %d_%d (%d mm)", seznam_stikal[i][0] + 1, seznam_stikal[i][4], seznam_stikal[i][3]));
	
	pointVentil = wxPoint(pointVentil.x - size.x / 3 * 2, pointVentil.y + 100);
	stikLeva = new wxChoice(panel, wxID_ANY, pointVentil, wxDefaultSize, koncnaStikla/*, wxCB_SORT*/);
	for (int i = 0; i < seznam_stikal.size(); i++) if (seznam_stikal[i][1] == oznacitev && seznam_stikal[i][2] == 8) stikLeva->SetSelection(i);

	wxButton* brisiLeva = new wxButton(panel, wxID_ANY, "Odstrani", wxPoint(pointVentil.x, pointVentil.y + 25), wxDefaultSize);

	pointVentil = wxPoint(pointVentil.x + size.x / 3 * 2, pointVentil.y);
	stikDesna = new wxChoice(panel, wxID_ANY, pointVentil, wxDefaultSize, koncnaStikla/*, wxCB_SORT */ );
	for (int i = 0; i < seznam_stikal.size(); i++) if (seznam_stikal[i][1] == oznacitev && seznam_stikal[i][2] == 7) stikDesna->SetSelection(i);

	wxButton* brisiDesna = new wxButton(panel, wxID_ANY, "Odstrani", wxPoint(pointVentil.x, pointVentil.y + 25), wxDefaultSize);


	panel->Bind(wxEVT_SIZE, &VentilNastavitve::OnSizeChanged, this);
	aplly->Bind(wxEVT_BUTTON, &VentilNastavitve::OnApllyClicked, this);
	levaLastnostVentil->Bind(wxEVT_CHECKLISTBOX, &VentilNastavitve::OnNastavitveChanged, this);
	desnaLastnostVentil->Bind(wxEVT_CHECKLISTBOX, &VentilNastavitve::OnNastavitveChanged, this);
	nazivVentila->Bind(wxEVT_CHOICE, &VentilNastavitve::OnNastavitveChanged, this);
	stikLeva->Bind(wxEVT_CHOICE, &VentilNastavitve::OnNastavitveChanged, this);
	stikDesna->Bind(wxEVT_CHOICE, &VentilNastavitve::OnNastavitveChanged, this);
	brisiLeva->Bind(wxEVT_BUTTON, &VentilNastavitve::OnBrisiLevaChanged, this);
	brisiDesna->Bind(wxEVT_BUTTON, &VentilNastavitve::OnBrisiDesnaChanged, this);



	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(VentilNastavitve::OnPaint));

	panel->SetDoubleBuffered(true);
}

void VentilNastavitve::OnApllyClicked(wxCommandEvent& evt) {
	
	if (seznam_valjev[oznacitev][2] == 5) {

		if (levaLastnostVentil->IsChecked(0) == true) seznam_lastnosti[oznacitev][1] = 0;
		else seznam_lastnosti[oznacitev][1] = -1;
		if (desnaLastnostVentil->IsChecked(0) == true) seznam_lastnosti[oznacitev][2] = 0;
		else seznam_lastnosti[oznacitev][2] = -1;
		if (levaLastnostVentil->IsChecked(1) == true && stikLeva->GetSelection() < 0) seznam_lastnosti[oznacitev][3] = 0;
		else seznam_lastnosti[oznacitev][3] = -1;
		if (desnaLastnostVentil->IsChecked(1) == true && stikDesna->GetSelection() < 0) seznam_lastnosti[oznacitev][4] = 0;
		else seznam_lastnosti[oznacitev][4] = -1;
		if (levaLastnostVentil->IsChecked(2) == true) seznam_lastnosti[oznacitev][5] = 1;
		else seznam_lastnosti[oznacitev][5] = -1;
		if (desnaLastnostVentil->IsChecked(2) == true) seznam_lastnosti[oznacitev][6] = 1;
		else seznam_lastnosti[oznacitev][6] = -1;

		for (int i = 0; i < seznam_stikal.size(); i++) if (seznam_stikal[i][1] == oznacitev && seznam_stikal[i][2] == 8) {
			seznam_stikal[i][1] = -1;
			seznam_stikal[i][2] = -1;
		}
		if (stikLeva->GetSelection() >= 0) {
			seznam_stikal[stikLeva->GetSelection()][1] = oznacitev;
			seznam_stikal[stikLeva->GetSelection()][2] = 8;
			for (int i = 0; i < seznam_cevi.size(); i++) if (seznam_cevi[i][1] == 6) seznam_cevi.erase(seznam_cevi.begin() + i);
		}
		for (int i = 0; i < seznam_stikal.size(); i++) if (seznam_stikal[i][1] == oznacitev && seznam_stikal[i][2] == 7) {
			seznam_stikal[i][1] = -1;
			seznam_stikal[i][2] = -1;
		}
		if (stikDesna->GetSelection() >= 0) {
			seznam_stikal[stikDesna->GetSelection()][1] = oznacitev;
			seznam_stikal[stikDesna->GetSelection()][2] = 7;
			for (int i = 0; i < seznam_cevi.size(); i++) if (seznam_cevi[i][1] == 7) seznam_cevi.erase(seznam_cevi.begin() + i);
		}

		if (nazivVentila->GetSelection() == 0) seznam_lastnosti[oznacitev][0] = 32;
		else if (nazivVentila->GetSelection() == 1) seznam_lastnosti[oznacitev][0] = 42;
		else if (nazivVentila->GetSelection() == 2) seznam_lastnosti[oznacitev][0] = 52;
	}
	else wxLogStatus("Izberite ventil za spremembo nastavitev");

	Refresh();
}

void VentilNastavitve::OnSizeChanged(wxSizeEvent& evt) {

	Refresh();
}

void VentilNastavitve::OnNastavitveChanged(wxCommandEvent& evt) {

	Refresh();
}

void VentilNastavitve::OnBrisiLevaChanged(wxCommandEvent& evt) {

	stikLeva->SetSelection(-1);

	for (int i = 0; i < seznam_stikal.size(); i++) if (seznam_stikal[i][1] == oznacitev && seznam_stikal[i][2] == 8) {
		seznam_lastnosti[oznacitev][8] = -1;
		
		seznam_stikal[i][1] = -1;
		seznam_stikal[i][2] = -1;
	}

	Refresh();
}

void VentilNastavitve::OnBrisiDesnaChanged(wxCommandEvent& evt) {

	stikDesna->SetSelection(-1);

	for (int i = 0; i < seznam_stikal.size(); i++) if (seznam_stikal[i][1] == oznacitev && seznam_stikal[i][2] == 7) {
		seznam_lastnosti[oznacitev][7] = -1;

		seznam_stikal[i][1] = -1;
		seznam_stikal[i][2] = -1;
	}

	Refresh();
}


void VentilNastavitve::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);

	wxSize size = this->GetSize();

	//- ADMIN "LOGS"
	if (true) {
		dc.DrawText(wxString::Format("izbor = %d", stikLeva->GetSelection()), wxPoint(120, 10));
		for (int i = 0; i < seznam_stikal.size(); i++) dc.DrawText(wxString::Format("seznam stikal: %d | %d | %d | %d | %d", seznam_stikal[i][0], seznam_stikal[i][1], seznam_stikal[i][2], seznam_stikal[i][3], seznam_stikal[i][4]), wxPoint(200, 10 + i * 15));
	}

	int deb_ven = 48;

	dc.DrawRectangle(wxPoint(size.x / 2 - deb_ven, size.y / 2 - deb_ven / 2), wxSize(deb_ven + 1, deb_ven + 1)); // celica 0
	dc.DrawRectangle(wxPoint(size.x / 2, size.y / 2 - deb_ven / 2), wxSize(deb_ven + 1, deb_ven + 1)); // celica 1

	if (nazivVentila->GetSelection() == 0) { // 3/2
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 - deb_ven + deb_ven / 3, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5), wxPoint(size.x / 2 - deb_ven + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 3 * 2 - 4, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5), wxPoint(size.x / 2 - deb_ven + deb_ven / 3 * 2 + 4 + 1, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 - deb_ven + deb_ven / 3 + 6, size.y / 2 - deb_ven / 2 + 6));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 - deb_ven + deb_ven / 3 - 6, size.y / 2 - deb_ven / 2 + 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 3, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5), wxPoint(size.x / 2 + deb_ven / 3, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 3 - 4, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5), wxPoint(size.x / 2 + deb_ven / 3 + 4 + 1, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2 + deb_ven), wxPoint(size.x / 2 + deb_ven / 3 * 2 + 6, size.y / 2 - deb_ven / 2 + deb_ven - 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2 + deb_ven), wxPoint(size.x / 2 + deb_ven / 3 * 2 - 6, size.y / 2 - deb_ven / 2 + deb_ven - 6));
	}
	else if (nazivVentila->GetSelection() == 1) { // 4/2
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 - deb_ven + deb_ven / 3, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 - deb_ven + deb_ven / 3 + 6, size.y / 2 - deb_ven / 2 + 6));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 - deb_ven + deb_ven / 3 - 6, size.y / 2 - deb_ven / 2 + 6));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 - deb_ven + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2 + deb_ven), wxPoint(size.x / 2 - deb_ven + deb_ven / 3 * 2 + 6, size.y / 2 - deb_ven / 2 + deb_ven - 6));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2 + deb_ven), wxPoint(size.x / 2 - deb_ven + deb_ven / 3 * 2 - 6, size.y / 2 - deb_ven / 2 + deb_ven - 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2 + deb_ven), wxPoint(size.x / 2 + deb_ven / 3 * 2 + 6, size.y / 2 - deb_ven / 2 + deb_ven - 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2 + deb_ven), wxPoint(size.x / 2 + deb_ven / 3 * 2 - 6, size.y / 2 - deb_ven / 2 + deb_ven - 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 + deb_ven / 3, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 + deb_ven / 3 * 2 + 6, size.y / 2 - deb_ven / 2 + 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 3 * 2, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 + deb_ven / 3 * 2 - 6, size.y / 2 - deb_ven / 2 + 6));
	}
	else if (nazivVentila->GetSelection() == 2) { // 5/2
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 4 * 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 - deb_ven + deb_ven / 4 * 3, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 4, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 - deb_ven + deb_ven / 2, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 4, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5), wxPoint(size.x / 2 - deb_ven + deb_ven / 4, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 4 - 4, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5), wxPoint(size.x / 2 - deb_ven + deb_ven / 4 + 4 + 1, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 4, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 - deb_ven + deb_ven / 4 + 6, size.y / 2 - deb_ven / 2 + 6));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 4, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 - deb_ven + deb_ven / 4 - 6, size.y / 2 - deb_ven / 2 + 6));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 4 * 3, size.y / 2 - deb_ven / 2 + deb_ven), wxPoint(size.x / 2 - deb_ven + deb_ven / 4 * 3 + 6, size.y / 2 - deb_ven / 2 + deb_ven - 6));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven + deb_ven / 4 * 3, size.y / 2 - deb_ven / 2 + deb_ven), wxPoint(size.x / 2 - deb_ven + deb_ven / 4 * 3 - 6, size.y / 2 - deb_ven / 2 + deb_ven - 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 4, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 + deb_ven / 4, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 4 * 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 + deb_ven / 2, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 4 * 3, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5), wxPoint(size.x / 2 + deb_ven / 4 * 3, size.y / 2 - deb_ven / 2 + deb_ven));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 4 * 3 - 4, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5), wxPoint(size.x / 2 + deb_ven / 4 * 3 + 4 + 1, size.y / 2 - deb_ven / 2 + deb_ven / 6 * 5));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 4, size.y / 2 - deb_ven / 2 + deb_ven), wxPoint(size.x / 2 + deb_ven / 4 + 6, size.y / 2 - deb_ven / 2 + deb_ven - 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 4, size.y / 2 - deb_ven / 2 + deb_ven), wxPoint(size.x / 2 + deb_ven / 4 - 6, size.y / 2 - deb_ven / 2 + deb_ven - 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 4 * 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 + deb_ven / 4 * 3 + 6, size.y / 2 - deb_ven / 2 + 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven / 4 * 3, size.y / 2 - deb_ven / 2), wxPoint(size.x / 2 + deb_ven / 4 * 3 - 6, size.y / 2 - deb_ven / 2 + 6));
	}

	if (levaLastnostVentil->IsChecked(0) == true) {
		dc.DrawRectangle(wxPoint(size.x / 2 - deb_ven - deb_ven / 4, size.y / 2 - deb_ven / 2 + 5), wxSize(deb_ven / 4 + 1, deb_ven / 6 + 1));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven - deb_ven / 4, size.y / 2 - deb_ven / 2 + 6 - 4), wxPoint(size.x / 2 - deb_ven - deb_ven / 4, size.y / 2 + deb_ven / 4 - deb_ven / 2 + 5));
	}
	if (desnaLastnostVentil->IsChecked(0) == true) {
		dc.DrawRectangle(wxPoint(size.x / 2 + deb_ven, size.y / 2 - deb_ven / 2 + 5), wxSize(deb_ven / 4 + 1, deb_ven / 6 + 1));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven + deb_ven / 4, size.y / 2 - deb_ven / 2 + 6 - 4), wxPoint(size.x / 2 + deb_ven + deb_ven / 4, size.y / 2 + deb_ven / 4 - deb_ven / 2 + 5));
	}
	if (levaLastnostVentil->IsChecked(1) == true && stikLeva->GetSelection() < 0) {
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven, size.y / 2), wxPoint(size.x / 2 - deb_ven - 6, size.y / 2));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven - 6, size.y / 2), wxPoint(size.x / 2 - deb_ven - 12, size.y / 2 - 6));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven - 12, size.y / 2 - 6), wxPoint(size.x / 2 - deb_ven - 12, size.y / 2 + 6));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven - 12, size.y / 2 + 6), wxPoint(size.x / 2 - deb_ven - 6, size.y / 2));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven - 12, size.y / 2), wxPoint(size.x / 2 - deb_ven - 18, size.y / 2));
	}
	if (desnaLastnostVentil->IsChecked(1) == true && stikDesna->GetSelection() < 0) {
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven, size.y / 2), wxPoint(size.x / 2 + deb_ven + 6, size.y / 2));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven + 6, size.y / 2), wxPoint(size.x / 2 + deb_ven + 12, size.y / 2 - 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven + 12, size.y / 2 - 6), wxPoint(size.x / 2 + deb_ven + 12, size.y / 2 + 6));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven + 12, size.y / 2 + 6), wxPoint(size.x / 2 + deb_ven + 6, size.y / 2));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven + 12, size.y / 2), wxPoint(size.x / 2 + deb_ven + 18, size.y / 2));
	}
	if (levaLastnostVentil->IsChecked(2) == true) {
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven, size.y / 2 + deb_ven / 2 - 2), wxPoint(size.x / 2 - deb_ven - 4, size.y / 2 + deb_ven / 2 - 10));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven - 4, size.y / 2 + deb_ven / 2 - 10), wxPoint(size.x / 2 - deb_ven - 8, size.y / 2 + deb_ven / 2 - 2));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven - 8, size.y / 2 + deb_ven / 2 - 2), wxPoint(size.x / 2 - deb_ven - 12, size.y / 2 + deb_ven / 2 - 10));
		dc.DrawLine(wxPoint(size.x / 2 - deb_ven - 12, size.y / 2 + deb_ven / 2 - 10), wxPoint(size.x / 2 - deb_ven - 16, size.y / 2 + deb_ven / 2 - 2));
	}
	if (desnaLastnostVentil->IsChecked(2) == true) {
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven, size.y / 2 + deb_ven / 2 - 2), wxPoint(size.x / 2 + deb_ven + 4, size.y / 2 + deb_ven / 2 - 10));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven + 4, size.y / 2 + deb_ven / 2 - 10), wxPoint(size.x / 2 + deb_ven + 8, size.y / 2 + deb_ven / 2 - 2));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven + 8, size.y / 2 + deb_ven / 2 - 2), wxPoint(size.x / 2 + deb_ven + 12, size.y / 2 + deb_ven / 2 - 10));
		dc.DrawLine(wxPoint(size.x / 2 + deb_ven + 12, size.y / 2 + deb_ven / 2 - 10), wxPoint(size.x / 2 + deb_ven + 16, size.y / 2 + deb_ven / 2 - 2));
	}

	if (stikLeva->GetSelection() >= 0) {
		dc.DrawRectangle(wxPoint(size.x / 2 - deb_ven - 12, size.y / 2 - 3), wxSize(12 + 1, 6 + 1));
		dc.DrawCircle(wxPoint(size.x / 2 - deb_ven - 12, size.y / 2), 5);
		dc.DrawCircle(wxPoint(size.x / 2 - deb_ven - 12, size.y / 2), 2);
		levaLastnostVentil->Check(1, false);
	}
	if (stikDesna->GetSelection() >= 0) {
		dc.DrawRectangle(wxPoint(size.x / 2 + deb_ven, size.y / 2 - 3), wxSize(12 + 1, 6 + 1));
		dc.DrawCircle(wxPoint(size.x / 2 + deb_ven + 12, size.y / 2), 5);
		dc.DrawCircle(wxPoint(size.x / 2 + deb_ven + 12, size.y / 2), 2);
		desnaLastnostVentil->Check(1, false);
	}
}


wxSpinCtrlDouble* kompresorTlak;

KompresorNastavitve::KompresorNastavitve() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve ventila"), wxPoint(0, 0), wxSize(300, 200)) {

	panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	kompresorTlak = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(90, 10), wxSize(120, -1), wxSP_ARROW_KEYS | wxSP_WRAP | wxTE_PROCESS_ENTER, 0, 20, seznam_lastnosti[oznacitev][0], .01);

	wxButton* aplly = new wxButton(panel, wxID_ANY, "Aplly", wxPoint(90, 110), wxSize(120, 40));


	kompresorTlak->Bind(wxEVT_SPINCTRLDOUBLE, &KompresorNastavitve::OnNastavitveChanged, this);
	kompresorTlak->Bind(wxEVT_TEXT_ENTER, &KompresorNastavitve::OnNastavitveChanged, this);
	aplly->Bind(wxEVT_BUTTON, &KompresorNastavitve::OnApllyClicked, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(KompresorNastavitve::OnPaint));
}

void KompresorNastavitve::OnApllyClicked(wxCommandEvent& evt) {

	if (oznacitev >= 0) {

		seznam_lastnosti[oznacitev][0] = kompresorTlak->GetValue();
		res_reset[oznacitev][0] = kompresorTlak->GetValue();

		res = res_reset;

		Refresh();
	}
	else wxLogStatus("Oznacite Kompresor za dolocitev tlaka");
}

void KompresorNastavitve::OnNastavitveChanged(wxCommandEvent& evt) {

	Refresh();
}


void KompresorNastavitve::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);

	dc.DrawLine(wxPoint(130, 90), wxPoint(170, 90));
	dc.DrawLine(wxPoint(170, 90), wxPoint(150, 90 - 40 * 2 / 3));
	dc.DrawLine(wxPoint(150, 90 - 40 * 2 / 3), wxPoint(130, 90));
	dc.DrawText(wxString::Format("p = %g", kompresorTlak->GetValue()), wxPoint(170, 65));
}

