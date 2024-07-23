#include "OknoSim.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/time.h>
#include <wx/utils.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <string>
#include <fstream>
#include <numeric>
#include <wx/msgdlg.h> // wxMessageBox(wxT("Hello World!"));



std::vector<std::vector<double>> IzracunVolumna(std::vector<std::vector<int>> seznamElementov, std::vector<std::vector<double>> seznamResitevReset, std::vector<std::vector<double>> seznamLastnosti) {

	const double pi = 3.14159265358979;

	double V0_krmilni = .001;
	double V1_krmilni = .001;

	for (int i = 0; i < seznamResitevReset.size(); i++) {

		if (seznamElementov[i][2] == 2) {

			double V = seznamLastnosti[i][0];

			seznamResitevReset[i][3] = V;
		}
		else if (seznamElementov[i][2] == 3) {

			double D = seznamLastnosti[i][2];
			double d = seznamLastnosti[i][3];
			double l = seznamLastnosti[i][4];
			double x0 = seznamLastnosti[i][5] / 100 * l;
			double x1 = l - x0;

			double A0 = pi * pow(D, 2) / 4;
			double Ab = pi * pow(d, 2) / 4;
			double A1 = A0 - Ab;

			double V0 = A0 * x0 + V0_krmilni;
			double V1 = A1 * x1 + V1_krmilni;


			seznamResitevReset[i][3] = V0;
			seznamResitevReset[i][6] = V1;
			seznamResitevReset[i][8] = x0;
		}
		else if (seznamElementov[i][2] == 4) {

			double D = seznamLastnosti[i][0];
			double l = seznamLastnosti[i][1];

			double A = pi * pow(D, 2) / 4;

			double V = A * l;

			seznamResitevReset[i][3] = V;
		}
	}
	return seznamResitevReset;
}

std::vector<std::vector<double>> IzracunMase(std::vector<double> pogojiOkolja, std::vector<std::vector<int>> seznamElementov, std::vector<std::vector<double>> seznamResitevReset) {

	double R = pogojiOkolja[2];
	double T = pogojiOkolja[1];

	for (int i = 0; i < seznamResitevReset.size(); i++) if (seznamResitevReset[i][1] == -1 && (seznamElementov[i][2] == 2 || seznamElementov[i][2] == 3 || seznamElementov[i][2] == 4)) {
		double p = seznamResitevReset[i][2];
		double V = seznamResitevReset[i][3];

		seznamResitevReset[i][1] = p * V / (R * T);

		if (seznamElementov[i][2] == 3) {
			p = seznamResitevReset[i][5];
			V = seznamResitevReset[i][6];

			seznamResitevReset[i][4] = p * V / (R * T);

		}
	}
	return seznamResitevReset;
}

//lasti[bat_premer, batnica_premer, hod_bata, zacetna_poz]
//resi[deluje, p_d, x, v, a, V_l, V_d, n]
//pogojiOkolja[p_ok, T, R, g]
std::vector<double> IzracunPrijemala(std::vector<double> pogojiOkolja, std::vector<double> lasti, std::vector<double> resi, double korak) {


	const double pi = 3.14159265358979;

	double g = pogojiOkolja[3];
	double pok = pogojiOkolja[0];

	double ti = korak;

	double koef_tr_st = .8;
	double koef_tr_din = .6;


	double D = lasti[2];
	double d = lasti[3];
	double l = lasti[4];
	double m = .8;

	double Ftr_s = m * g * koef_tr_st;
	double Ftr_d = m * g * koef_tr_din;


	double A0 = pi * pow(D, 2) / 4;
	double Ab = pi * pow(d, 2) / 4;
	double A1 = A0 - Ab;


	double p0 = resi[2];
	double p1 = resi[5];

	double x = resi[8];
	double v = resi[9];
	double a = resi[10];

	double V0 = resi[3];
	double V1 = resi[6];


	double F0, F1, F1b, dF;

	F0 = A0 * p0;
	F1 = A1 * p1;
	F1b = Ab * pok;

	dF = F0 - F1 - F1b;


	if (v == 0) {

		if (abs(dF) > Ftr_s) {

			if (dF > 0) a = (dF - Ftr_s) / m;
			else if (dF < 0) a = (dF + Ftr_s) / m;
		}
		else a = 0;
	}
	else if (v > 0) a = (dF - Ftr_d) / m;
	else if (v < 0) a = (dF + Ftr_d) / m;

	v = v + a * ti;

	if (abs(v) < .0001) v = 0;

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

	V0 += A0 * dx;
	V1 -= A1 * dx;


	resi[2] = p0;
	resi[3] = V0;
	resi[5] = p1;
	resi[6] = V1;
	resi[8] = x;
	resi[9] = v;
	resi[10] = a;

	return resi;
}

std::vector<double> IzracunPriseska(std::vector<double> pogojiOkolja, std::vector<double> lasti, std::vector<double> resi, double korak) {
	
	double pi = 3.14159265358979;


	double g = pogojiOkolja[3];
	double m = resi[4];

	double Fg = m * g;


	double p = resi[2];
	double pok = pogojiOkolja[0];
	double d = lasti[0];

	double A = d * d * pi / 4;

	double Fp = (pok - p) * A;


	if (Fp > Fg) resi[5] = 1;
	else resi[5] = -1;

	return resi;
}


std::vector<std::vector<double>> IzracunPovezav(std::vector<double> pogojiOkolja, std::vector<std::vector<int>> seznamElementov, std::vector<std::vector<double>> seznamLastnosti, std::vector<std::vector<double>> seznamResitev, std::vector<std::vector<int>> seznamPovezav, std::vector<std::vector<double>> seznamStikal, double korak) {

	std::vector<std::vector<double>> masniTok;
	masniTok.resize(seznamElementov.size());

	double pi = 3.14159265358979;

	double ti = korak; // Casovni korak [s]

	double g = pogojiOkolja[3];
	double R = pogojiOkolja[2]; // Masna plinska konstanta [J/kgK]
	double T = pogojiOkolja[1]; // Temperatura zraka [K]
	double gama = 1.4; // Razmerje sprecificnih toplot [/]
	double C = .6; // Koeficient prretoka [/]
	double d = .01; // Premer cevi [m]
	double A = pi * d * d / 4; // Prerez cevi [m^2]


	//- STIKALA
	for (int i = 0; i < seznamStikal.size(); i++) {

		if (seznamStikal[i][2] < 0) {

			if (seznamResitev[seznamStikal[i][0]][seznamStikal[i][1]] < seznamStikal[i][3]) seznamResitev[seznamStikal[i][4]][seznamStikal[i][5]] = seznamStikal[i][6];
		}

		else if (seznamStikal[i][2] == 0) {

			if (seznamResitev[seznamStikal[i][0]][seznamStikal[i][1]] == seznamStikal[i][3]) seznamResitev[seznamStikal[i][4]][seznamStikal[i][5]] = seznamStikal[i][6];
		}

		else if (seznamStikal[i][2] > 0) {

			if (seznamResitev[seznamStikal[i][0]][seznamStikal[i][1]] > seznamStikal[i][3]) seznamResitev[seznamStikal[i][4]][seznamStikal[i][5]] = seznamStikal[i][6];
		}
	}


	//- IZRACUN MASNEGA TOKA CEZ CRPALKO
	bool crpalkaBool = false;
	for (int i = 0; i < seznamPovezav.size(); i++) { if (seznamPovezav[i][4] == 1 && (seznamElementov[seznamPovezav[i][0]][2] == 1 || seznamElementov[seznamPovezav[i][2]][2] == 1)) { crpalkaBool = true; break; } }

	if (crpalkaBool) {
		std::vector<std::vector<int>> crpalkaElementi; // [porabnik, dovodnik, smer_toka, crpalka]
		crpalkaElementi.resize(4);


		double V1, V2;
		double p1, p2;
		double m1, m2;
		double rho1, rho2;

		for (int i = 0; i < seznamPovezav.size(); i++) if (seznamPovezav[i][4] == 1) { ////////////// deluje samo za en prikljucek na vhod in izhod crpalke

			if (seznamElementov[seznamPovezav[i][0]][2] == 1 && (seznamResitev[seznamPovezav[i][0]][0] == 1 || seznamResitev[seznamPovezav[i][0]][0] == -1)) {
				if (seznamResitev[seznamPovezav[i][2]][0] == 1) {
					if (seznamPovezav[i][1] == 1) crpalkaElementi[0].push_back(seznamPovezav[i][2]);
					else if (seznamPovezav[i][1] == 2) crpalkaElementi[1].push_back(seznamPovezav[i][2]);
					crpalkaElementi[3].push_back(seznamPovezav[i][0]);
				}
			}
			else if (seznamElementov[seznamPovezav[i][2]][2] == 1 && (seznamResitev[seznamPovezav[i][2]][0] == 1 || seznamResitev[seznamPovezav[i][2]][0] == -1)) {
				if (seznamResitev[seznamPovezav[i][0]][0] == 1) {
					if (seznamPovezav[i][3] == 1) crpalkaElementi[0].push_back(seznamPovezav[i][0]);
					else if (seznamPovezav[i][3] == 2) crpalkaElementi[1].push_back(seznamPovezav[i][0]);
					crpalkaElementi[3].push_back(seznamPovezav[i][2]);
				}
			}
			if ((seznamElementov[seznamPovezav[i][0]][2] == 1 && seznamResitev[seznamPovezav[i][0]][0] == 1) || (seznamElementov[seznamPovezav[i][2]][2] == 1 && seznamResitev[seznamPovezav[i][2]][0] == 1))
				crpalkaElementi[2].push_back(1); /////////// ustvari vec podatkov, ko bi moral biti samo eden
			else if ((seznamElementov[seznamPovezav[i][0]][2] == 1 && seznamResitev[seznamPovezav[i][0]][0] == -1) || (seznamElementov[seznamPovezav[i][2]][2] == 1 && seznamResitev[seznamPovezav[i][2]][0] == -1))
				crpalkaElementi[2].push_back(-1); /////////// ustvari vec podatkov, ko bi moral biti samo eden
		}

		for (int i = 0; i < crpalkaElementi[0].size(); i++) {
			if (!crpalkaElementi[0].empty()) {

				V1 = seznamResitev[crpalkaElementi[0][i]][3];
				p1 = seznamResitev[crpalkaElementi[0][i]][2];
				m1 = seznamResitev[crpalkaElementi[0][i]][1];
				rho1 = m1 / V1;
			}
			else {

				p1 = pogojiOkolja[0];
				rho1 = pogojiOkolja[4];
			}
			if (!crpalkaElementi[1].empty()) {

				V2 = seznamResitev[crpalkaElementi[1][i]][3];
				p2 = seznamResitev[crpalkaElementi[1][i]][2];
				m2 = seznamResitev[crpalkaElementi[1][i]][1];
				rho2 = m2 / V2;
			}
			else {

				p2 = pogojiOkolja[0];
				rho2 = pogojiOkolja[4];
			}

			double izkc = seznamLastnosti[crpalkaElementi[3][i]][0];
			double ncmax = seznamLastnosti[crpalkaElementi[3][i]][1] * (2 * pi);
			double Vc = seznamLastnosti[crpalkaElementi[3][i]][2] / (2 * pi);
			double Ac = seznamLastnosti[crpalkaElementi[3][i]][3];
			double lc = seznamLastnosti[crpalkaElementi[3][i]][4];

			double mtok = 0;
			double Pc = seznamResitev[crpalkaElementi[3][i]][2];
			double nc = 0;


			double dT = (p1 - p2) * Ac * lc * crpalkaElementi[2][i];

			if (dT >= 0) {
				nc = Pc * izkc / dT;
				if (nc > ncmax) nc = ncmax;
			}
			else if (dT < 0) nc = ncmax;

			nc *= crpalkaElementi[2][i];


			if (crpalkaElementi[2][i] == 1) {
				if (!crpalkaElementi[1].empty()) mtok = (m2 - m2 * (V2 / (V2 + Vc))) * nc;
				else mtok = rho2 * Vc * nc;
			}
			else if (crpalkaElementi[2][i] == -1) {
				if (!crpalkaElementi[0].empty()) mtok = (m1 - m1 * (V1 / (V1 + Vc))) * nc;
				else mtok = rho1 * Vc * nc;
			}


			seznamResitev[crpalkaElementi[3][i]][1] = mtok;

			if (!crpalkaElementi[0].empty()) {

				masniTok[crpalkaElementi[0][i]].push_back(0);
				masniTok[crpalkaElementi[0][i]].push_back(mtok);
			}
			if (!crpalkaElementi[1].empty()) {

				masniTok[crpalkaElementi[1][i]].push_back(0);
				masniTok[crpalkaElementi[1][i]].push_back(-mtok);
			}
		}
	}

	//- IZRACUN MASNEGA TOKA
	for (int i = 0; i < seznamPovezav.size(); i++) {

		if (seznamPovezav[i][4] == 1 && (seznamResitev[seznamPovezav[i][0]][0] == 1 || seznamResitev[seznamPovezav[i][0]][0] == -1) && (seznamResitev[seznamPovezav[i][2]][0] == 1 || seznamResitev[seznamPovezav[i][2]][0] == -1)) {

			if (seznamElementov[seznamPovezav[i][0]][2] == 1 || seznamElementov[seznamPovezav[i][2]][2] == 1 || seznamElementov[seznamPovezav[i][0]][2] == 4 || seznamElementov[seznamPovezav[i][2]][2] == 4) { // Kompresor

			}
			else {

				double V1, V2;
				double p1, p2;
				double m1, m2;

				if (seznamElementov[seznamPovezav[i][0]][2] == 2) {
					V1 = seznamResitev[seznamPovezav[i][0]][3];
					p1 = seznamResitev[seznamPovezav[i][0]][2];
					m1 = seznamResitev[seznamPovezav[i][0]][1];
				}
				else if (seznamElementov[seznamPovezav[i][0]][2] == 3) {
					if (seznamResitev[seznamPovezav[i][0]][7] == 0) {
						V1 = seznamResitev[seznamPovezav[i][0]][3];
						p1 = seznamResitev[seznamPovezav[i][0]][2];
						m1 = seznamResitev[seznamPovezav[i][0]][1];
					}
					else if (seznamResitev[seznamPovezav[i][0]][7] == 1) {
						V1 = seznamResitev[seznamPovezav[i][0]][6];
						p1 = seznamResitev[seznamPovezav[i][0]][5];
						m1 = seznamResitev[seznamPovezav[i][0]][4];
					}
				}

				if (seznamElementov[seznamPovezav[i][2]][2] == 2) {
					V2 = seznamResitev[seznamPovezav[i][2]][3];
					p2 = seznamResitev[seznamPovezav[i][2]][2];
					m2 = seznamResitev[seznamPovezav[i][2]][1];
				}
				else if (seznamElementov[seznamPovezav[i][2]][2] == 3) {
					if (seznamResitev[seznamPovezav[i][2]][7] == 0) {
						V2 = seznamResitev[seznamPovezav[i][2]][3];
						p2 = seznamResitev[seznamPovezav[i][2]][2];
						m2 = seznamResitev[seznamPovezav[i][2]][1];
					}
					else if (seznamResitev[seznamPovezav[i][2]][7] == 1) {
						V2 = seznamResitev[seznamPovezav[i][2]][6];
						p2 = seznamResitev[seznamPovezav[i][2]][5];
						m2 = seznamResitev[seznamPovezav[i][2]][4];
					}
				}


				double rho1 = m1 / V1;
				double rho2 = m2 / V2;

				double mtok = 0;
				if (p1 > p2) mtok = -C * A * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama)));
				else if (p1 < p2) mtok = C * A * sqrt(2 * rho2 * p2 * (gama / (gama - 1)) * (pow(p1 / p2, 2 / gama) - pow(p1 / p2, (gama + 1) / gama)));

				if (seznamElementov[seznamPovezav[i][0]][2] == 2) masniTok[seznamPovezav[i][0]].push_back(0);
				else if (seznamElementov[seznamPovezav[i][0]][2] == 3) masniTok[seznamPovezav[i][0]].push_back(seznamResitev[seznamPovezav[i][0]][7]);
				masniTok[seznamPovezav[i][0]].push_back(mtok);
				if (seznamElementov[seznamPovezav[i][2]][2] == 2) masniTok[seznamPovezav[i][2]].push_back(0);
				else if (seznamElementov[seznamPovezav[i][2]][2] == 3) masniTok[seznamPovezav[i][2]].push_back(seznamResitev[seznamPovezav[i][2]][7]);
				masniTok[seznamPovezav[i][2]].push_back(-mtok);
			}
		}

		//- ODZRACEVANJE
		else if (seznamPovezav[i][4] == 2) {

			if (seznamElementov[seznamPovezav[i][0]][2] == 2) {
				if (seznamResitev[seznamPovezav[i][0]][2] > seznamLastnosti[seznamPovezav[i][0]][1]) {
					double V1;
					double p1, p2;
					double m1;

					V1 = seznamResitev[seznamPovezav[i][0]][3];
					p1 = seznamResitev[seznamPovezav[i][0]][2];
					m1 = seznamResitev[seznamPovezav[i][0]][1];

					double rho1 = m1 / V1;

					p2 = pogojiOkolja[0];

					double mtok = 0;
					mtok = -C * 8 * A * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama)));

					masniTok[seznamPovezav[i][0]].push_back(0);
					masniTok[seznamPovezav[i][0]].push_back(mtok);
				}
			}
			else if (seznamElementov[seznamPovezav[i][0]][2] == 3) {
				if (seznamResitev[seznamPovezav[i][0]][7] == 0 && (seznamResitev[seznamPovezav[i][0]][5] > pogojiOkolja[0] || seznamResitev[seznamPovezav[i][0]][5] < pogojiOkolja[0])) {
					double V1;
					double p1, p2;
					double m1;

					V1 = seznamResitev[seznamPovezav[i][0]][6];
					p1 = seznamResitev[seznamPovezav[i][0]][5];
					m1 = seznamResitev[seznamPovezav[i][0]][4];

					double rho1 = m1 / V1;

					p2 = pogojiOkolja[0];

					double rho2 = pogojiOkolja[4];

					double mtok = 0;
					if (seznamResitev[seznamPovezav[i][0]][5] > pogojiOkolja[0]) mtok = -C * 3 * A * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama))); //std::cout << "513, ";
					else if (seznamResitev[seznamPovezav[i][0]][5] < pogojiOkolja[0]) mtok = C * 3 * A * sqrt(2 * rho2 * p2 * (gama / (gama - 1)) * (pow(p1 / p2, 2 / gama) - pow(p1 / p2, (gama + 1) / gama))); //std::cout << "514, ";

					masniTok[seznamPovezav[i][0]].push_back(1);
					masniTok[seznamPovezav[i][0]].push_back(mtok);
				}
				else if (seznamResitev[seznamPovezav[i][0]][7] == 1 && (seznamResitev[seznamPovezav[i][0]][2] > pogojiOkolja[0] || seznamResitev[seznamPovezav[i][0]][2] < pogojiOkolja[0])) {
					double V1;
					double p1, p2;
					double m1;

					V1 = seznamResitev[seznamPovezav[i][0]][3];
					p1 = seznamResitev[seznamPovezav[i][0]][2];
					m1 = seznamResitev[seznamPovezav[i][0]][1];

					double rho1 = m1 / V1;

					p2 = pogojiOkolja[0];

					double rho2 = pogojiOkolja[4];

					double mtok = 0;
					if (seznamResitev[seznamPovezav[i][0]][2] > pogojiOkolja[0]) mtok = -C * 3 * A * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama)));
					else if (seznamResitev[seznamPovezav[i][0]][2] < pogojiOkolja[0]) mtok = C * 3 * A * sqrt(2 * rho2 * p2 * (gama / (gama - 1)) * (pow(p1 / p2, 2 / gama) - pow(p1 / p2, (gama + 1) / gama)));

					masniTok[seznamPovezav[i][0]].push_back(0);
					masniTok[seznamPovezav[i][0]].push_back(mtok);
				}
			}
			else if (seznamElementov[seznamPovezav[i][0]][2] == 4) {
				if (seznamResitev[seznamPovezav[i][0]][4] == 0 || (seznamResitev[seznamPovezav[i][0]][5] == -1 && seznamResitev[seznamPovezav[i][0]][6] == -1)) {
					double V1;
					double p1, p2;
					double m1;

					V1 = seznamResitev[seznamPovezav[i][0]][3];
					p1 = seznamResitev[seznamPovezav[i][0]][2];
					m1 = seznamResitev[seznamPovezav[i][0]][1];

					double rho1 = m1 / V1;

					p2 = pogojiOkolja[0];

					double rho2 = pogojiOkolja[4];

					double mtok = 0;
					if (seznamResitev[seznamPovezav[i][0]][2] > pogojiOkolja[0]) mtok = -C * 20 * A * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama)));
					else if (seznamResitev[seznamPovezav[i][0]][2] < pogojiOkolja[0]) mtok = C * 20 * A * sqrt(2 * rho2 * p2 * (gama / (gama - 1)) * (pow(p1 / p2, 2 / gama) - pow(p1 / p2, (gama + 1) / gama)));

					masniTok[seznamPovezav[i][0]].push_back(0);
					masniTok[seznamPovezav[i][0]].push_back(mtok);
				}
				else {
					double V1;
					double p1, p2;
					double m1;

					V1 = seznamResitev[seznamPovezav[i][0]][3];
					p1 = seznamResitev[seznamPovezav[i][0]][2];
					m1 = seznamResitev[seznamPovezav[i][0]][1];

					double rho1 = m1 / V1;

					p2 = pogojiOkolja[0];

					double rho2 = pogojiOkolja[4];

					double mtok = 0;
					if (seznamResitev[seznamPovezav[i][0]][2] > pogojiOkolja[0]) mtok = -C * (A / 20) * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama)));
					else if (seznamResitev[seznamPovezav[i][0]][2] < pogojiOkolja[0]) mtok = C * (A / 20) * sqrt(2 * rho2 * p2 * (gama / (gama - 1)) * (pow(p1 / p2, 2 / gama) - pow(p1 / p2, (gama + 1) / gama)));

					masniTok[seznamPovezav[i][0]].push_back(0);
					masniTok[seznamPovezav[i][0]].push_back(mtok);
				}
			}
		}
	}
	/*std::cout << std::endl;
	for (int i = 0; i < masniTok.size(); i++) {
		std::cout << i << ": ";
		for (int j = 0; j < masniTok[i].size(); j++) {
			std::cout << masniTok[i][j] << ", ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;*/

	//- IZRACUN TLAKA:
	for (int i = 0; i < masniTok.size(); i++) if (masniTok[i].size() > 0) {

		double m1 = 0, m2 = 0; // Masa
		double V1 = 0, V2 = 0; // Volumen
		double p1 = 0, p2 = 0; // Tlak

		for (int j = 0; j < masniTok[i].size(); j += 2) seznamResitev[i][1 + 3 * masniTok[i][j]] += masniTok[i][j + 1] * ti;

		if (seznamElementov[i][2] == 2 || seznamElementov[i][2] == 4) {

			m1 = seznamResitev[i][1];
			V1 = seznamResitev[i][3];

			p1 = m1 * R * T / V1;

			seznamResitev[i][2] = p1;
		}
		else if (seznamElementov[i][2] == 3) {

			m1 = seznamResitev[i][1];
			V1 = seznamResitev[i][3];

			p1 = m1 * R * T / V1;

			seznamResitev[i][2] = p1;

			m2 = seznamResitev[i][4];
			V2 = seznamResitev[i][6];

			p2 = m2 * R * T / V2;

			seznamResitev[i][5] = p2;
		}
	}

	//- IZRACUN PORABNIKOV:
	for (int i = 0; i < seznamElementov.size(); i++) {
		if (seznamElementov[i][2] == 3) seznamResitev[i] = IzracunPrijemala(pogojiOkolja, seznamLastnosti[i], seznamResitev[i], korak);
		else if (seznamElementov[i][2] == 4) seznamResitev[i] = IzracunPriseska(pogojiOkolja, seznamLastnosti[i], seznamResitev[i], korak);
	}

	return seznamResitev;
}



bool drzanje = false;
bool drzanjeElementa = false;
int casDrzanja = 0;
short drzanjePovezav = 0;
short izbranElement = -1;
short izbranaPovezava = -1;

std::vector<double> pogojiOkolja;
// [tlak ozracja, temperatura ozracja]
 
std::vector<std::vector<int>> seznamElementov;
// [x, y, element]
std::vector<std::vector<double>> seznamLastnosti;
// mikroprocesor []
// tlacna crpalka [izkoristek, torzija, volumen_na_vrtljaj, povrsina_rotorja, rocica_prijemalisca_sile]
// tlacna posoda [volumen, varnostni ventil]
// prijemalo [/, /, D, d, l, %]
// prisesek [D, l]
std::vector<std::vector<double>> seznamResitevReset;
// mikroprocesor [delovanje0 (0/1), delovanje1(0/1), delovanje2(0/1), delovanje3(0/1), delovanje4(0/1), delovanje5(0/1), delovanje6(0/1), delovanje7(0/1)]
// tlacna crpalka [delovanje (0/1), masni_tok, moc, obrati]
// tlacna posoda [delovanje (0/1), masa_zraka, tlak, volumen]
// prijemalo [delovanje (0/1), masa_zraka1, tlak1, volumen1, masa_zraka2, tlak2, volumen2, prikljucen_ventila_4/2, x, v, a]
// prisesek [delovanje (0/1), masa_zraka, tlak, volumen, utez, drzanje, pozicija]
std::vector<std::vector<double>> seznamResitev;
// seznamResitev = seznamResitevReset

std::vector<std::vector<int>> seznamPovezav;
// [element1, prikljucen1, element2, prikljucek2, kabl/cev (0/1)]
std::vector<std::vector<double>> seznamStikal;
// [element1, velicina1, logicna_funkcija, vrednost1, element2, velicina2, vrednost2]

wxChoice* choiceDod;
wxGauge* casSimulacije;
wxSpinCtrl* spinPovezava;

bool simbool = false;
double korak = .001; // Casovni korak simulacije [s]


OknoSim::OknoSim(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* izbElement = new wxButton(panel, wxID_ANY, "Izbrisi izbrani element", wxPoint(5, 220), wxSize(190, -1));
	wxButton* izbVse = new wxButton(panel, wxID_ANY, "Izbrisi vse", wxPoint(5, 250), wxSize(190, -1));
	wxButton* risanjePovezav = new wxButton(panel, wxID_ANY, "Risanje povezav", wxPoint(5,280), wxSize(190,-1));
	spinPovezava = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(5, 310), wxSize(90, -1), wxSP_ARROW_KEYS | wxSP_WRAP, -1, -1, -1);
	wxButton* brisanjePovezav = new wxButton(panel, wxID_ANY, "Brisi povezavo", wxPoint(105, 310), wxSize(90, -1));
	wxButton* shrani = new wxButton(panel, wxID_ANY, "Shrani", wxPoint(5, 350), wxSize(190, -1));
	wxButton* nalozi = new wxButton(panel, wxID_ANY, "Nalozi", wxPoint(5, 380), wxSize(190, -1));
	wxButton* simuliraj = new wxButton(panel, wxID_ANY, "Simuliraj", wxPoint(5, 500), wxSize(190, 40));
	wxButton* resetSim = new wxButton(panel, wxID_ANY, "Reset", wxPoint(5, 570), wxSize(190, -1));

	casSimulacije = new wxGauge(panel, wxID_ANY, 10000, wxPoint(5, 548), wxSize(190, -1), wxGA_HORIZONTAL, wxDefaultValidator, "cas");
	
	wxArrayString choices;
	choices.Add("Mikroprocesor");
	choices.Add("Elektricna Crpalka");
	choices.Add("Tlacna Posoda");
	choices.Add("Prijemalo");
	choices.Add("Prisesek");

	choiceDod = new wxChoice(panel, wxID_ANY, wxPoint(5, 0), wxSize(190, -1), choices/*, wxCB_SORT*/);
	choiceDod->SetSelection(0);


	panel->Bind(wxEVT_SIZE, &OknoSim::OnSizeChanged, this);
	panel->Bind(wxEVT_MOTION, &OknoSim::RefreshEvent, this);
	panel->Bind(wxEVT_LEFT_DOWN, &OknoSim::OnMouseDownEvent, this);
	panel->Bind(wxEVT_LEFT_UP, &OknoSim::OnMouseUpEvent, this);
	panel->Bind(wxEVT_LEFT_DCLICK, &OknoSim::OnMouseDoubleEvent, this);
	izbElement->Bind(wxEVT_BUTTON, &OknoSim::OnButtonIzbClicked, this);
	izbVse->Bind(wxEVT_BUTTON, &OknoSim::OnButtonIzbVseClicked, this);
	risanjePovezav->Bind(wxEVT_BUTTON, &OknoSim::OnRisanjePovezavClicked, this);
	spinPovezava->Bind(wxEVT_SPINCTRL, &OknoSim::OnSpinPovezavaChanged, this);
	brisanjePovezav->Bind(wxEVT_BUTTON, &OknoSim::OnBrisanjePovezavClicked, this);
	shrani->Bind(wxEVT_BUTTON, &OknoSim::OnShraniClicked, this);
	nalozi->Bind(wxEVT_BUTTON, &OknoSim::OnNaloziClicked, this);
	simuliraj->Bind(wxEVT_BUTTON, &OknoSim::OnSimulirajClicked, this);
	resetSim->Bind(wxEVT_BUTTON, &OknoSim::OnResetSimClicked, this);
	
	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(OknoSim::OnPaint));

	panel->SetDoubleBuffered(true);


	pogojiOkolja.push_back(101325);
	pogojiOkolja.push_back(293);
	pogojiOkolja.push_back(287);
	pogojiOkolja.push_back(9.81);
	pogojiOkolja.push_back(1.293);


	seznamElementov.push_back({ 220,20,0 });
	seznamElementov.push_back({ 360,540,1 });
	seznamElementov.push_back({ 510,430,2 });
	seznamElementov.push_back({ 700,350,3 });
	seznamElementov.push_back({ 600,540,4 });
	seznamElementov.push_back({ 680,250,3 });

	seznamLastnosti.push_back({});
	seznamLastnosti.push_back({ .95,6,0.2,0.01,0.05 });
	seznamLastnosti.push_back({ 2,700000 });
	seznamLastnosti.push_back({ 700000,700000,0.1,0.025,0.4,80 });
	seznamLastnosti.push_back({ 0.1,0.2 });
	seznamLastnosti.push_back({ 700000,700000,0.1,0.025,0.4,40 });

	seznamResitevReset.push_back({ 0,0,0,0,0,0,0,0 });
	seznamResitevReset.push_back({ 0,0,2000,0 });
	seznamResitevReset.push_back({ 1,-1,600000,-1 });
	seznamResitevReset.push_back({ 1,-1,pogojiOkolja[0],-1,-1,pogojiOkolja[0],-1,0,0,0,0 });
	seznamResitevReset.push_back({ 1,-1,pogojiOkolja[0],-1, 40, -1, 1 });
	seznamResitevReset.push_back({ 1,-1,pogojiOkolja[0],-1,-1,pogojiOkolja[0],-1,0,0,0,0 });

	seznamResitevReset = IzracunVolumna(seznamElementov, seznamResitevReset, seznamLastnosti);
	seznamResitevReset = IzracunMase(pogojiOkolja, seznamElementov, seznamResitevReset);

	seznamResitev = seznamResitevReset;


	seznamPovezav.push_back({ 0,0,1,0,0 });
	seznamPovezav.push_back({ 0,1,2,0,0 });
	seznamPovezav.push_back({ 0,2,5,0,0 });
	seznamPovezav.push_back({ 0,3,3,0,0 });
	seznamPovezav.push_back({ 0,4,4,0,0 });

	seznamPovezav.push_back({ 1,1,2,1,1 });
	seznamPovezav.push_back({ 2,2,3,1,1 });
	seznamPovezav.push_back({ 5,1,2,2,1 });
	seznamPovezav.push_back({ 1,2,4,1,1 });

	seznamPovezav.push_back({ 2,3,-1,-1,2 });
	seznamPovezav.push_back({ 3,3,-1,-1,2 });
	seznamPovezav.push_back({ 4,3,-1,-1,2 });
	seznamPovezav.push_back({ 5,3,-1,-1,2 });


	seznamStikal.push_back({ 2,2,-1,595000,1,0,1 });
	seznamStikal.push_back({ 5,2,1,500000,5,7,1 });
	seznamStikal.push_back({ 5,5,1,500000,5,7,0 });
	seznamStikal.push_back({ 5,5,1,500000,4,6,-1 });
	seznamStikal.push_back({ 5,2,1,500000,4,6,1 });
	seznamStikal.push_back({ 2,2,-1,588000,1,0,-1 });
	seznamStikal.push_back({ 2,2,-1,500000,1,0,1 });
	seznamStikal.push_back({ 2,2,-1,500000,4,0,0 });

	wxStatusBar* statusBar = CreateStatusBar();
}


void OknoSim::OnSizeChanged(wxSizeEvent& evt) {

	Refresh();
}

void OknoSim::RefreshEvent(wxMouseEvent& evt) {

	if (simbool == false) {
		wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

		if (drzanjeElementa && izbranElement >= 0 && casDrzanja > 2) {
			seznamElementov[izbranElement][0] = mousePos.x / 10 * 10;
			seznamElementov[izbranElement][1] = mousePos.y / 10 * 10;
		}

		casDrzanja++;

		Refresh();
	}
}

void OknoSim::OnMouseDownEvent(wxMouseEvent& evt) {

	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	if (mousePos.x < 200 && mousePos.y < 180) drzanje = true;
	else if (mousePos.x > 200) {

		for (int i = 0; i < seznamElementov.size(); i++) {
			int vrst;
			if (drzanjePovezav > 0) vrst = seznamPovezav[seznamPovezav.size() - 1][4];

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
					else if (mousePos.x > seznamElementov[i][0] + 15 && mousePos.x < seznamElementov[i][0] + 25 && mousePos.y > seznamElementov[i][1] + 5 && mousePos.y < seznamElementov[i][1] + 15) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 2; seznamPovezav[seznamPovezav.size() - 1][4] = 1; drzanjePovezav++; }
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
					if (mousePos.x > seznamElementov[i][0] + 25 && mousePos.x < seznamElementov[i][0] + 35 && mousePos.y > seznamElementov[i][1] - 15 && mousePos.y < seznamElementov[i][1] - 5) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 0; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] - 15 && mousePos.x < seznamElementov[i][0] - 5 && mousePos.y > seznamElementov[i][1] + 15 && mousePos.y < seznamElementov[i][1] + 25) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 1; seznamPovezav[seznamPovezav.size() - 1][4] = 1; drzanjePovezav++; }
					else if (mousePos.x > seznamElementov[i][0] + 125 && mousePos.x < seznamElementov[i][0] + 135 && mousePos.y > seznamElementov[i][1] + 15 && mousePos.y < seznamElementov[i][1] + 25) {
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
					if (mousePos.x > seznamElementov[i][0] + 45 && mousePos.x < seznamElementov[i][0] + 55 && mousePos.y > seznamElementov[i][1] - 40 && mousePos.y < seznamElementov[i][1] - 30) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 0; seznamPovezav[seznamPovezav.size() - 1][4] = 0; drzanjePovezav++;
					}
					else if (mousePos.x > seznamElementov[i][0] + 35 && mousePos.x < seznamElementov[i][0] + 45 && mousePos.y > seznamElementov[i][1] - 30 && mousePos.y < seznamElementov[i][1] - 20) {
						seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2] = i; seznamPovezav[seznamPovezav.size() - 1][(drzanjePovezav - 1) * 2 + 1] = 1; seznamPovezav[seznamPovezav.size() - 1][4] = 1; drzanjePovezav++;
					}
				}
				if (mousePos.x > seznamElementov[i][0] && mousePos.x < seznamElementov[i][0] + 80 && mousePos.y > seznamElementov[i][1] - 20 && mousePos.y < seznamElementov[i][1] + 10) {
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
		if (drzanjeElementa == false) {
			izbranElement = -1;
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
	wxSize oknoSize = this->GetSize();
	
	if (izbranElement >= 0 && casDrzanja > 2 && mousePos.x > oknoSize.x - 115 && mousePos.y < 60 && drzanjeElementa) {
		
		for (int i = seznamPovezav.size() - 1; i >= 0; i--) if (seznamPovezav[i][0] == izbranElement || seznamPovezav[i][2] == izbranElement) seznamPovezav.erase(seznamPovezav.begin() + i); // brisanje cevi
		for (int i = seznamStikal.size() - 1; i >= 0; i--) if (seznamStikal[i][0] == izbranElement || seznamStikal[i][4] == izbranElement) seznamStikal.erase(seznamStikal.begin() + i); // brisanje stikal

		for (int i = 0; i < seznamPovezav.size(); i++) {
			if (seznamPovezav[i][0] > izbranElement) seznamPovezav[i][0]--;
			if (seznamPovezav[i][2] > izbranElement) seznamPovezav[i][2]--;
		}

		for (int i = 0; i < seznamStikal.size(); i++) {
			if (seznamStikal[i][0] > izbranElement) seznamStikal[i][0]--;
			if (seznamStikal[i][4] > izbranElement) seznamStikal[i][4]--;
		}

		seznamElementov.erase(seznamElementov.begin() + izbranElement);
		seznamLastnosti.erase(seznamLastnosti.begin() + izbranElement);
		seznamResitevReset.erase(seznamResitevReset.begin() + izbranElement);
		seznamResitev = seznamResitevReset;

		casSimulacije->SetValue(0);
		izbranElement = -1;
	}
	else if (drzanje && mousePos.x > 200) {

		seznamElementov.push_back({ mousePos.x / 10 * 10,mousePos.y / 10 * 10,choiceDod->GetSelection() }); ////////////// treba dodat vse

		if (choiceDod->GetSelection() == 0) {
			seznamLastnosti.push_back({});
			seznamResitevReset.push_back({ 0,0,0,0,0,0,0,0 });
		}
		else if (choiceDod->GetSelection() == 1) {
			seznamLastnosti.push_back({ .95,6,0.2,0.01,0.05 });
			seznamResitevReset.push_back({ 1,0,2000,0 });
		}
		else if (choiceDod->GetSelection() == 2) {
			seznamLastnosti.push_back({ 2,700000 });
			seznamResitevReset.push_back({ 1,-1,600000,-1 });
			seznamPovezav.push_back({ static_cast<int>(seznamElementov.size()) - 1,3,-1,-1,2 });
		}
		else if (choiceDod->GetSelection() == 3) {
			seznamLastnosti.push_back({ 700000,700000,0.1,0.025,0.4,0 });
			seznamResitevReset.push_back({ 1,-1,pogojiOkolja[0],-1,-1,pogojiOkolja[0],-1,0,0,0,0 });
			seznamPovezav.push_back({ static_cast<int>(seznamElementov.size()) - 1,3,-1,-1,2 });
		}
		else if (choiceDod->GetSelection() == 4) {
			seznamLastnosti.push_back({ 0.1,0.2 });
			seznamResitevReset.push_back({ 1,-1,pogojiOkolja[0],-1, 40, -1, 1 });
			seznamPovezav.push_back({ static_cast<int>(seznamElementov.size()) - 1,3,-1,-1,2 });
		}
		else {
			seznamLastnosti.push_back({});
			seznamResitevReset.push_back({ 0,0,0,0 });
		}

		seznamResitevReset = IzracunVolumna(seznamElementov, seznamResitevReset, seznamLastnosti);
		seznamResitevReset = IzracunMase(pogojiOkolja, seznamElementov, seznamResitevReset);
		seznamResitev = seznamResitevReset;
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

		simbool = false;
		casSimulacije->SetValue(0);
		seznamResitev = seznamResitevReset;

		if (drzanjePovezav > 0) {
			seznamPovezav.erase(seznamPovezav.begin() + seznamPovezav.size() - 1);
			drzanjePovezav = 0;
		}

		if (seznamElementov[izbranElement][2] == 0) {
			NastavitevMikroProcesorja* procNast = new NastavitevMikroProcesorja();
			procNast->Show();
		}
		else if (seznamElementov[izbranElement][2] == 1) {
			NastavitevCrpalke* crpNast = new NastavitevCrpalke();
			crpNast->Show();
		}
		else if (seznamElementov[izbranElement][2] == 2) {
			NastavitevTlacnePosode* posNast = new NastavitevTlacnePosode();
			posNast->Show();
		}
		else if (seznamElementov[izbranElement][2] == 3) {
			NastavitevPrijemalke* prijNast = new NastavitevPrijemalke();
			prijNast->Show();
		}
		else if (seznamElementov[izbranElement][2] == 4) {
			NastavitevPriseska* prisNast = new NastavitevPriseska();
			prisNast->Show();
		}
	}

	Refresh();
}

void OknoSim::OnButtonIzbClicked(wxCommandEvent& evt) {

	if (izbranElement >= 0) {

		simbool = false;

		for (int i = seznamPovezav.size() - 1; i >= 0; i--) if (seznamPovezav[i][0] == izbranElement || seznamPovezav[i][2] == izbranElement) seznamPovezav.erase(seznamPovezav.begin() + i); // brisanje cevi

		for (int i = 0; i < seznamPovezav.size(); i++) {
			if (seznamPovezav[i][0] > izbranElement) seznamPovezav[i][0]--;
			if (seznamPovezav[i][2] > izbranElement) seznamPovezav[i][2]--;
		}


		seznamElementov.erase(seznamElementov.begin() + izbranElement);
		seznamLastnosti.erase(seznamLastnosti.begin() + izbranElement);
		seznamResitevReset.erase(seznamResitevReset.begin() + izbranElement);
		seznamResitev = seznamResitevReset;

		casSimulacije->SetValue(0);
		izbranElement = -1;

		Refresh();
	}
}

void OknoSim::OnButtonIzbVseClicked(wxCommandEvent& evt) {

	simbool = false;

	seznamStikal.clear();
	seznamPovezav.clear();
	seznamElementov.clear();
	seznamLastnosti.clear();
	seznamResitevReset.clear();
	seznamResitev = seznamResitevReset;

	casSimulacije->SetValue(0);
	izbranElement = -1;

	Refresh();
}

void OknoSim::OnRisanjePovezavClicked(wxCommandEvent& evt) {
	
	simbool = false;
	casSimulacije->SetValue(0);
	seznamResitev = seznamResitevReset;

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

void OknoSim::OnBrisanjePovezavClicked(wxCommandEvent& evt) {

	if (izbranaPovezava > 0) {

		seznamPovezav.erase(seznamPovezav.begin() + izbranaPovezava - 1);
	}

	Refresh();
}

void OknoSim::OnSpinPovezavaChanged(wxCommandEvent& evt) {

	izbranaPovezava = spinPovezava->GetValue(); /////////////// mogoc dat v Refresh()

	Refresh();
}

void OknoSim::OnShraniClicked(wxCommandEvent& evt) {

	simbool = false;
	drzanjePovezav = 0;
	seznamResitev = seznamResitevReset;
	casSimulacije->SetValue(0);

	Refresh();


	std::string ime = __DATE__;
	wxFileDialog* fileDialog = new wxFileDialog(this, "Shrani", wxEmptyString, "Simulacija " + ime, "Text files (*.txt)|*.txt", wxFD_SAVE);

	if (fileDialog->ShowModal() == wxID_OK) {

		std::string pot = static_cast<std::string>(fileDialog->GetPath());

		std::ofstream shrani;
		shrani.open(pot, std::ios::out);

		if (shrani.is_open()) {

			shrani << "Zapis zacetnih podatkov simulacije" << std::endl;
			shrani << __DATE__ << ", " << __TIME__ << std::endl << std::endl;


			shrani << "Stevilo elementov: " << seznamElementov.size() << std::endl;

			for (int i = 0; i < seznamElementov.size(); i++) {

				shrani << seznamElementov[i].size() << ": ";
				for (int j = 0; j < seznamElementov[i].size(); j++) shrani << seznamElementov[i][j] << " ";
				shrani << std::endl;

				shrani << seznamLastnosti[i].size() << ": ";
				for (int j = 0; j < seznamLastnosti[i].size(); j++) shrani << seznamLastnosti[i][j] << " ";
				shrani << std::endl;

				shrani << seznamResitev[i].size() << ": ";
				for (int j = 0; j < seznamResitev[i].size(); j++) shrani << seznamResitev[i][j] << " ";
				shrani << std::endl << std::endl;
			}
			shrani << std::endl;


			shrani << "Stevilo cevi: " << seznamPovezav.size() << std::endl;

			for (int i = 0; i < seznamPovezav.size(); i++) {

				shrani << seznamPovezav[i].size() << ": ";
				for (int j = 0; j < seznamPovezav[i].size(); j++) shrani << seznamPovezav[i][j] << " ";
				shrani << std::endl;
			}
			shrani << std::endl << std::endl;


			shrani << "Stevilo stikal: " << seznamStikal.size() << std::endl;

			for (int i = 0; i < seznamStikal.size(); i++) {

				shrani << seznamStikal[i].size() << ": ";
				for (int j = 0; j < seznamStikal[i].size(); j++) shrani << seznamStikal[i][j] << " ";
				shrani << std::endl;
			}
			shrani << std::endl << std::endl;

			shrani.close();
		}
	}
}

void OknoSim::OnNaloziClicked(wxCommandEvent& evt) {

	wxFileDialog* fileDialog = new wxFileDialog(this, "Odpri datoteko", wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_OPEN);

	if (fileDialog->ShowModal() == wxID_OK) {

		std::string pot = static_cast<std::string>(fileDialog->GetPath());

		std::ifstream nalozi;
		nalozi.open(pot, std::ios::in);

		if (nalozi.is_open()) {

			std::string bes;
			char ch;
			int st;

			nalozi >> bes >> bes >> bes >> bes;
			if (!(bes == "simulacije")) wxMessageBox(wxT("Neustrezna datoteka!"));

			else {

				simbool = false;

				seznamStikal.clear();
				seznamPovezav.clear();
				seznamElementov.clear();
				seznamLastnosti.clear();
				seznamResitevReset.clear();
				seznamResitev = seznamResitevReset;

				casSimulacije->SetValue(0);

				izbranElement = -1;

				Refresh();


				nalozi >> bes >> bes >> bes >> bes;
				nalozi >> bes >> bes;
				nalozi >> st;

				seznamElementov.resize(st);
				seznamLastnosti.resize(st);
				seznamResitevReset.resize(st);

				for (int i = 0; i < st; i++) {

					int pon;

					nalozi >> pon >> ch;
					for (int j = 0; j < pon; j++) { double a; nalozi >> a; seznamElementov[i].push_back(a); }

					nalozi >> pon >> ch;
					for (int j = 0; j < pon; j++) { double a; nalozi >> a; seznamLastnosti[i].push_back(a); }

					nalozi >> pon >> ch;
					for (int j = 0; j < pon; j++) { double a; nalozi >> a; seznamResitevReset[i].push_back(a); }
				}


				nalozi >> bes >> bes;
				nalozi >> st;

				seznamPovezav.resize(st);

				for (int i = 0; i < st; i++) {

					int pon;

					nalozi >> pon >> ch;
					for (int j = 0; j < pon; j++) { int a; nalozi >> a; seznamPovezav[i].push_back(a); }
				}


				nalozi >> bes >> bes;
				nalozi >> st;

				seznamStikal.resize(st);

				for (int i = 0; i < st; i++) {

					int pon;

					nalozi >> pon >> ch;
					for (int j = 0; j < pon; j++) { int a; nalozi >> a; seznamStikal[i].push_back(a); }
				}
			}

			seznamResitev = seznamResitevReset;
		}

		else wxLogStatus("Datoteka ni najdena");
	}

	fileDialog->Destroy();


	Refresh();
}

void OknoSim::OnSimulirajClicked(wxCommandEvent& evt) {

	if (drzanjePovezav > 0) {

		seznamPovezav.erase(seznamPovezav.begin() + seznamPovezav.size() - 1);
		drzanjePovezav = 0;
	}

	if (casSimulacije->GetValue() >= casSimulacije->GetRange() || casSimulacije->GetValue() == 0) {
		casSimulacije->SetValue(0);
		seznamResitev = seznamResitevReset;
		Refresh();
	}

	int i = casSimulacije->GetValue();

	if (simbool == false) simbool = true;
	else simbool = false;

	while (simbool && i <= casSimulacije->GetRange()) {
		casSimulacije->SetValue(i);
		Refresh();
		//Sleep(3);
		wxYield();
		i++;
	}
	if (i >= 1000) {
		simbool = false;
	}
}

void OknoSim::OnResetSimClicked(wxCommandEvent& evt) {

	simbool = false;
	casSimulacije->SetValue(0);
	seznamResitev = seznamResitevReset;

	Refresh();
}


void OknoSim::OnPaint(wxPaintEvent& evt) {

	spinPovezava->SetRange(0, seznamPovezav.size());

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());


	//- IZRIS OKNA
	int sirinaOrodja = 200;
	int visinaOrodja = 180;

	dc.DrawRectangle(wxPoint(0, 30), wxSize(sirinaOrodja + 1, visinaOrodja));
	dc.DrawRectangle(wxPoint(sirinaOrodja, 0), wxSize(velikostOkna.x - sirinaOrodja, velikostOkna.y));
	dc.DrawRectangle(wxPoint(velikostOkna.x - 100, 0), wxSize(100, 60));
	dc.DrawText("SMETI", wxPoint(velikostOkna.x - 65, 25));


	dc.DrawText(wxString::Format("t = %d ms", casSimulacije->GetValue() * 1), wxPoint(10, 480));
	if (false) { // ADMIN LOGS
		for (int i = 0; i < seznamPovezav.size(); i++) dc.DrawText(wxString::Format("%d | %d | %d | %d | %d", seznamPovezav[i][0], seznamPovezav[i][1], seznamPovezav[i][2], seznamPovezav[i][3], seznamPovezav[i][4]), wxPoint(205, 320 + 12 * i));
		for (int i = 0; i < seznamElementov.size(); i++) dc.DrawText(wxString::Format("%d: %d | %d | %d", i, seznamElementov[i][0], seznamElementov[i][1], seznamElementov[i][2]), wxPoint(90, 320 + 12 * i));
		for (int i = 0; i < seznamStikal.size(); i++) dc.DrawText(wxString::Format("%g | %g | %g | %g | %g | %g | %g", seznamStikal[i][0], seznamStikal[i][1], seznamStikal[i][2], seznamStikal[i][3], seznamStikal[i][4], seznamStikal[i][5], seznamStikal[i][6]), wxPoint(5, 600 + 12 * i));
		for (int i = 0; i < seznamStikal.size(); i++) dc.DrawText(wxString::Format("%g | %g | %g | %g", seznamResitev[seznamStikal[i][0]][seznamStikal[i][1]], seznamStikal[i][3], seznamResitev[seznamStikal[i][4]][seznamStikal[i][5]], seznamStikal[i][6]), wxPoint(5, 700 + 12 * i));
	}


	//- PRIKAZ IZBRANEGA ELEMENTA
	if (izbranElement >= 0) {

		wxPoint oznacenElementPoint(seznamElementov[izbranElement][0], seznamElementov[izbranElement][1]);
		wxSize oznacenElementSize(100, 70);
		if (seznamElementov[izbranElement][2] == 0) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 10; oznacenElementSize.x = 120; oznacenElementSize.y = 160; }
		else if (seznamElementov[izbranElement][2] == 1) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 44; oznacenElementSize.x = 60; oznacenElementSize.y = 54; }
		else if (seznamElementov[izbranElement][2] == 2) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 10; oznacenElementSize.x = 140; oznacenElementSize.y = 60; }
		else if (seznamElementov[izbranElement][2] == 3) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 10; oznacenElementSize.x = 110; oznacenElementSize.y = 70; }
		else if (seznamElementov[izbranElement][2] == 4) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 30; oznacenElementSize.x = 100; oznacenElementSize.y = 40; }

		dc.SetPen(wxPen(wxColour(153, 153, 255), 1, wxPENSTYLE_LONG_DASH));
		dc.DrawRectangle(oznacenElementPoint, oznacenElementSize);
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}


	//- IZRIS PRIKAZA ELEMENTA
	wxPoint predogled(10, 50);

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
		dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
		dc.DrawLine(wxPoint(predogled.x + 20, predogled.y + 10), wxPoint(predogled.x + 20, predogled.y));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}
	else if (choiceDod->GetSelection() == 2) {
		dc.DrawRoundedRectangle(wxPoint(predogled.x, predogled.y), wxSize(120, 40), 20);
		dc.DrawLine(wxPoint(predogled.x + 30, predogled.y - 10), wxPoint(predogled.x + 30, predogled.y));

		dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
		dc.DrawLine(wxPoint(predogled.x - 10, predogled.y + 20), wxPoint(predogled.x, predogled.y + 20));
		dc.DrawLine(wxPoint(predogled.x + 120, predogled.y + 20), wxPoint(predogled.x + 130, predogled.y + 20));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}
	else if (choiceDod->GetSelection() == 3) {
		dc.DrawRectangle(wxPoint(predogled.x, predogled.y), wxSize(65, 50));
		dc.DrawRectangle(wxPoint(predogled.x + 40, predogled.y), wxSize(50, 10));
		dc.DrawRectangle(wxPoint(predogled.x + 40, predogled.y + 40), wxSize(50, 10));

		dc.DrawLine(wxPoint(predogled.x - 10, predogled.y + 10), wxPoint(predogled.x, predogled.y + 10));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
		dc.DrawLine(wxPoint(predogled.x - 10, predogled.y + 40), wxPoint(predogled.x, predogled.y + 40));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}
	else if (choiceDod->GetSelection() == 4) {
		wxPoint* t1 = new wxPoint(predogled.x, predogled.y);
		wxPoint* t2 = new wxPoint(predogled.x + 80, predogled.y);
		wxPoint* t3 = new wxPoint(predogled.x + 65, predogled.y - 15);
		wxPoint* t4 = new wxPoint(predogled.x + 15, predogled.y - 15);

		wxPointList* tocke = new wxPointList();
		tocke->Append(t1);
		tocke->Append(t2);
		tocke->Append(t3);
		tocke->Append(t4);

		dc.DrawPolygon(tocke);

		dc.DrawLine(wxPoint(predogled.x + 50, predogled.y - 15), wxPoint(predogled.x + 50, predogled.y - 35));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
		dc.DrawLine(wxPoint(predogled.x + 40, predogled.y - 15), wxPoint(predogled.x + 40, predogled.y - 25));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}

	
	//- IZRIS POVEZAV
	for (int i = 0; i < seznamPovezav.size(); i++) {

		if (seznamPovezav[i][4] != 2) {
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
					else if (seznamPovezav[i][1] == 2) {
						tocka1.x += 20;
						tocka1.y += 10;
					}

					break;

				case 2:

					if (seznamPovezav[i][1] == 0) {
						tocka1.x += 30;
						tocka1.y -= 10;
					}
					else if (seznamPovezav[i][1] == 1) {
						tocka1.x -= 10;
						tocka1.y += 20;
					}
					else if (seznamPovezav[i][1] == 2) {
						tocka1.x += 130;
						tocka1.y += 20;
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

					if (seznamPovezav[i][1] == 0) {
						tocka1.x += 50;
						tocka1.y -= 35;
					}
					else if (seznamPovezav[i][1] == 1) {
						tocka1.x += 40;
						tocka1.y -= 25;
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
					else if (seznamPovezav[i][3] == 2) {
						tocka2.x += 20;
						tocka2.y += 10;
					}

					break;

				case 2:

					if (seznamPovezav[i][3] == 0) {
						tocka2.x += 30;
						tocka2.y -= 10;
					}
					else if (seznamPovezav[i][3] == 1) {
						tocka2.x -= 10;
						tocka2.y += 20;
					}
					else if (seznamPovezav[i][3] == 2) {
						tocka2.x += 130;
						tocka2.y += 20;
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

					if (seznamPovezav[i][3] == 0) {
						tocka2.x += 50;
						tocka2.y -= 35;
					}
					else if (seznamPovezav[i][3] == 1) {
						tocka2.x += 40;
						tocka2.y -= 25;
					}

					break;


				default:
					break;
				}
			}

			if (seznamPovezav[i][4] == 0) dc.SetPen(wxPen(wxColour(153, 153, 153), 1, wxPENSTYLE_SOLID));
			else if (seznamPovezav[i][4] == 1) dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
			
			if (i == izbranaPovezava - 1) dc.SetPen(wxPen(wxColour(0, 255, 0), 2, wxPENSTYLE_SOLID));

			dc.DrawLine(tocka1, wxPoint((tocka1.x + tocka2.x) / 2, tocka1.y));
			dc.DrawLine(wxPoint((tocka1.x + tocka2.x) / 2, tocka1.y), wxPoint((tocka1.x + tocka2.x) / 2, tocka2.y));
			dc.DrawLine(wxPoint((tocka1.x + tocka2.x) / 2, tocka2.y), tocka2);

			dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
		}

	}
	
	
	//- IZRIS ELEMENTOV
	/*if (casSimulacije->GetValue() == 0) { seznamResitev[1][0] = 1; }*/
	//if (casSimulacije->GetValue() == 3600) { seznamLastnosti[2][1] = 500000; seznamResitev[3][0] = 0; seznamResitev[5][0] = 0; } //////////////// podere program èe ni varnostnega ventila

	if (simbool) for (int i = 0; i < 1; i++) seznamResitev = IzracunPovezav(pogojiOkolja, seznamElementov, seznamLastnosti, seznamResitev, seznamPovezav, seznamStikal, korak);

	for (int i = 0; i < seznamElementov.size(); i++) {
		std::vector<int> xy = seznamElementov[i];
		int zamik = 0;

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

			if (drzanjePovezav > 0) { // Risanje povezav
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
				dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
				dc.DrawLine(wxPoint(xy[0] + 20, xy[1] + 10), wxPoint(xy[0] + 20, xy[1]));
				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

				dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0], xy[1] + 15));
				dc.DrawText(wxString::Format("m_tok = %g kg/s", seznamResitev[i][1]), wxPoint(xy[0], xy[1] + 30));
				dc.DrawText(wxString::Format("moc = %g W", seznamResitev[i][2]), wxPoint(xy[0], xy[1] + 45));

				if (seznamResitev[i][0] > 0) dc.SetBrush(wxBrush(wxColour(51, 255, 51), wxBRUSHSTYLE_SOLID));
				else if (seznamResitev[i][0] < 0) dc.SetBrush(wxBrush(wxColour(255, 51, 51), wxBRUSHSTYLE_SOLID));
				else dc.SetBrush(wxBrush(wxColour(51, 51, 255), wxBRUSHSTYLE_SOLID));

				dc.DrawCircle(wxPoint(xy[0] + 25, xy[1] - 8), 5);

				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));

				if (drzanjePovezav > 0) { // Risanje povezav
					dc.SetPen(wxPen(wxColour(51, 51, 153), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 153, 255), wxBRUSHSTYLE_SOLID));

					dc.DrawRectangle(wxPoint(xy[0] + 5, xy[1] + 5), wxSize(10, 10));

					dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));

					dc.DrawRectangle(wxPoint(xy[0] + 15, xy[1] - 39), wxSize(10, 10));
					dc.DrawRectangle(wxPoint(xy[0] + 15, xy[1] + 5), wxSize(10, 10));

					dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
				}
			}

			break;

		case 2:

			dc.DrawRoundedRectangle(wxPoint(xy[0], xy[1]), wxSize(120, 40), 20);
			dc.DrawLine(wxPoint(xy[0] + 30, xy[1] - 10), wxPoint(xy[0] + 30, xy[1]));

			dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));

			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] + 20), wxPoint(xy[0], xy[1] + 20));
			dc.DrawLine(wxPoint(xy[0] + 120, xy[1] + 20), wxPoint(xy[0] + 130, xy[1] + 20));

			dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0] + 60, xy[1] - 16));
			dc.DrawText(wxString::Format("p =  %g bar", seznamResitev[i][2] / 100000), wxPoint(xy[0] + 20, xy[1] + 55));

			if (seznamLastnosti[i][1] >= 0) {

				if (seznamResitev[i][2] > seznamLastnosti[i][1]) dc.SetBrush(wxBrush(wxColour(51, 255, 51), wxBRUSHSTYLE_SOLID));
				else dc.SetBrush(wxBrush(wxColour(51, 51, 255), wxBRUSHSTYLE_SOLID));

				dc.DrawLine(wxPoint(xy[0] + 30, xy[1] + 40), wxPoint(xy[0] + 30, xy[1] + 50));
				dc.DrawLine(wxPoint(xy[0] + 30, xy[1] + 50), wxPoint(xy[0] + 35, xy[1] + 50));
				dc.DrawCircle(wxPoint(xy[0] + 45, xy[1] + 50), 5);
				dc.DrawLine(wxPoint(xy[0] + 35, xy[1] + 50), wxPoint(xy[0] + 48, xy[1] + 58));
				dc.DrawLine(wxPoint(xy[0] + 35, xy[1] + 50), wxPoint(xy[0] + 48, xy[1] + 42));

				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
			}

			if (drzanjePovezav > 0) { // Risanje povezav
				dc.SetPen(wxPen(wxColour(51, 51, 153), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 153, 255), wxBRUSHSTYLE_SOLID));

				dc.DrawRectangle(wxPoint(xy[0] + 25, xy[1] - 15), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));

				dc.DrawRectangle(wxPoint(xy[0] - 15, xy[1] + 15), wxSize(10, 10));
				dc.DrawRectangle(wxPoint(xy[0] + 125, xy[1] + 15), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
			}

			break;

		case 3:

			zamik = seznamResitev[i][8] / seznamLastnosti[i][4] * 10;
			
			dc.DrawRectangle(wxPoint(xy[0], xy[1]), wxSize(65, 50));
			dc.DrawRectangle(wxPoint(xy[0] + 40, xy[1] + zamik), wxSize(50, 10));
			dc.DrawRectangle(wxPoint(xy[0] + 40, xy[1] + 40 - zamik), wxSize(50, 10));

			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] + 10), wxPoint(xy[0], xy[1] + 10));

			dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));

			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] + 40), wxPoint(xy[0], xy[1] + 40));

			dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0], xy[1] - 16));
			dc.DrawText(wxString::Format("p0 = %g bar", seznamResitev[i][2] / 100000), wxPoint(xy[0], xy[1] + 50));
			dc.DrawText(wxString::Format("p1 = %g bar", seznamResitev[i][5] / 100000), wxPoint(xy[0], xy[1] + 65));

			if (drzanjePovezav > 0) { // Risanje povezav
				dc.SetPen(wxPen(wxColour(51, 51, 153), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 153, 255), wxBRUSHSTYLE_SOLID));

				dc.DrawRectangle(wxPoint(xy[0] - 15, xy[1] + 5), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));

				dc.DrawRectangle(wxPoint(xy[0] - 15, xy[1] + 35), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
			}

			if (seznamResitev[i][9] > 0) dc.SetBrush(wxBrush(wxColour(51, 255, 51), wxBRUSHSTYLE_SOLID));
			else if (seznamResitev[i][9] < 0) dc.SetBrush(wxBrush(wxColour(255, 51, 51), wxBRUSHSTYLE_SOLID));
			else dc.SetBrush(wxBrush(wxColour(51, 51, 255), wxBRUSHSTYLE_SOLID));

			dc.DrawCircle(wxPoint(xy[0] + 10, xy[1] + 10), 5);

			dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));

			break;

		case 4:

			if (true) {
				int zamik = 10;
				if (seznamResitev[i][6] == -1) zamik = 0;

				wxPoint* t1 = new wxPoint(xy[0], xy[1] + zamik);
				wxPoint* t2 = new wxPoint(xy[0] + 80, xy[1] + zamik);
				wxPoint* t3 = new wxPoint(xy[0] + 65, xy[1] - 15 + zamik);
				wxPoint* t4 = new wxPoint(xy[0] + 15, xy[1] - 15 + zamik);

				wxPointList* tocke = new wxPointList();
				tocke->Append(t1);
				tocke->Append(t2);
				tocke->Append(t3);
				tocke->Append(t4);

				dc.DrawPolygon(tocke);

				dc.DrawLine(wxPoint(xy[0] + 50, xy[1] - 15 + zamik), wxPoint(xy[0] + 50, xy[1] - 35));
				dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));

				dc.DrawLine(wxPoint(xy[0] + 40, xy[1] - 15 + zamik), wxPoint(xy[0] + 40, xy[1] - 25));

				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

				dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0] + 80, xy[1] - 32));
				dc.DrawText(wxString::Format("p = %g bar", seznamResitev[i][2] / 100000), wxPoint(xy[0] + 80, xy[1] - 16));

				if (drzanjePovezav > 0) { // Risanje povezav
					dc.SetPen(wxPen(wxColour(51, 51, 153), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 153, 255), wxBRUSHSTYLE_SOLID));

					dc.DrawRectangle(wxPoint(xy[0] + 45, xy[1] - 40), wxSize(10, 10));

					dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));

					dc.DrawRectangle(wxPoint(xy[0] + 35, xy[1] - 30), wxSize(10, 10));

					dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
				}


				int dela = 0;
				for (int j = 0; j < seznamPovezav.size() - 1; j++) if (seznamPovezav[j][4] == 1) {
					if (seznamPovezav[j][0] == i) {
						if (seznamResitev[seznamPovezav[j][2]][0] == 1) dela = 1;
						else if (seznamResitev[seznamPovezav[j][2]][0] == -1) dela = -1;
					}
					else if (seznamPovezav[j][2] == i) {
						if (seznamResitev[seznamPovezav[j][0]][0] == 1) dela = 1;
						else if (seznamResitev[seznamPovezav[j][0]][0] == -1) dela = -1;
					}
				}
				
				if (seznamResitev[i][5] == 1) dc.SetBrush(wxBrush(wxColour(51, 255, 51), wxBRUSHSTYLE_SOLID));
				else if (seznamResitev[i][5] == -1 && seznamResitev[i][6] == 1) dc.SetBrush(wxBrush(wxColour(51, 51, 255), wxBRUSHSTYLE_SOLID));
				else if (seznamResitev[i][6] == -1) dc.SetBrush(wxBrush(wxColour(255, 51, 51), wxBRUSHSTYLE_SOLID));

				dc.DrawCircle(wxPoint(xy[0] + 20, xy[1] - 7 + zamik), 5);

				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));


				zamik = 10;
				if (seznamResitev[i][5] == 1 && seznamResitev[i][6] == -1) zamik = 0;
				
				if (seznamResitev[i][4] >= 0) {
					dc.DrawRoundedRectangle(wxPoint(xy[0] - 10, xy[1] + zamik), wxSize(100, 30), 5);
					dc.DrawText(wxString::Format("%g kg", seznamResitev[i][4]), wxPoint(xy[0] + 25, xy[1] + 9 + zamik));
				}

				if (false) { // ADMIN LOGS
					dc.DrawText(wxString::Format("%g dela", seznamResitev[i][5]), wxPoint(xy[0] + 25, xy[1] + 40 + zamik));
					dc.DrawText(wxString::Format("%g poz", seznamResitev[i][6]), wxPoint(xy[0] + 25, xy[1] + 55 + zamik));
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



wxChoice* choicePinBranje;
wxChoice* choiceVelicinaBranje;
wxChoice* choiceLogFun;
wxSpinCtrlDouble* spinVrednostBranje;
wxChoice* choicePinPisanje;
wxChoice* choiceVelicinaPisanje;
wxChoice* choiceVrednostPisanje;

NastavitevMikroProcesorja::NastavitevMikroProcesorja() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve Mikro Procesorja"), wxPoint(0, 0), wxSize(540, 300)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(10,230), wxDefaultSize);



	wxArrayString ArPinBranje;
	wxArrayString ArPinPisanje;
	for (int j = 0; j < seznamPovezav.size(); j++) if (seznamPovezav[j][4] == 0) {
		if (seznamPovezav[j][0] == izbranElement) {

			if (seznamElementov[seznamPovezav[j][2]][2] == 2 || seznamElementov[seznamPovezav[j][2]][2] == 3 || seznamElementov[seznamPovezav[j][2]][2] == 4)
				ArPinBranje.Add(wxString::Format(" %d", seznamPovezav[j][1]));
			if (seznamElementov[seznamPovezav[j][2]][2] == 1 || seznamElementov[seznamPovezav[j][2]][2] == 3 || seznamElementov[seznamPovezav[j][2]][2] == 4)
				ArPinPisanje.Add(wxString::Format(" %d", seznamPovezav[j][1]));
		}
		else if (seznamPovezav[j][2] == izbranElement) {

			if (seznamElementov[seznamPovezav[j][0]][2] == 2 || seznamElementov[seznamPovezav[j][0]][2] == 3 || seznamElementov[seznamPovezav[j][0]][2] == 4)
				ArPinBranje.Add(wxString::Format(" %d", seznamPovezav[j][3]));
			if (seznamElementov[seznamPovezav[j][0]][2] == 1 || seznamElementov[seznamPovezav[j][0]][2] == 3 || seznamElementov[seznamPovezav[j][0]][2] == 4)
				ArPinPisanje.Add(wxString::Format(" %d", seznamPovezav[j][3]));
		}
	}

	choicePinBranje = new wxChoice(panel, wxID_ANY, wxPoint(100, 5), wxSize(36, -1), ArPinBranje);

	wxArrayString ArVelicinaBranje;
	ArVelicinaBranje.Add(" ");
	ArVelicinaBranje.Add(" ");

	choiceVelicinaBranje = new wxChoice(panel, wxID_ANY, wxPoint(140, 5), wxSize(56, -1), ArVelicinaBranje);
	choiceVelicinaBranje->Disable();

	wxArrayString ArLogFun;
	ArLogFun.Add(" <");
	ArLogFun.Add(" =");
	ArLogFun.Add(" >");
	
	choiceLogFun = new wxChoice(panel, wxID_ANY, wxPoint(200, 5), wxSize(36, -1), ArLogFun);
	choiceLogFun->Disable();
	
	spinVrednostBranje = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(240, 5), wxSize(56, -1), wxSP_ARROW_KEYS | wxSP_WRAP, -1000, 1000, 0, .01);
	spinVrednostBranje->Disable();


	choicePinPisanje = new wxChoice(panel, wxID_ANY, wxPoint(300, 5), wxSize(36, -1), ArPinPisanje);
	choicePinPisanje->Disable();

	wxArrayString ArVelicinaPisanje;
	ArVelicinaPisanje.Add(" ");
	ArVelicinaPisanje.Add(" ");

	choiceVelicinaPisanje = new wxChoice(panel, wxID_ANY, wxPoint(340, 5), wxSize(96, -1), ArVelicinaPisanje);
	choiceVelicinaPisanje->Disable();

	wxArrayString ArVrednostiPisanje;
	ArVrednostiPisanje.Add(" ");
	ArVrednostiPisanje.Add(" ");
	ArVrednostiPisanje.Add(" ");

	choiceVrednostPisanje = new wxChoice(panel, wxID_ANY, wxPoint(440, 5), wxSize(56, -1), ArVrednostiPisanje);
	choiceVrednostPisanje->Disable();

	wxButton* dodaj = new wxButton(panel, wxID_ANY, "Dodaj ukaz", wxPoint(100, 35), wxDefaultSize);
	wxButton* izbrisi = new wxButton(panel, wxID_ANY, "Izbrisi vse", wxPoint(100, 60), wxDefaultSize);



	apply->Bind(wxEVT_BUTTON, &NastavitevMikroProcesorja::OnApplyClicked, this);
	choicePinBranje->Bind(wxEVT_CHOICE, &NastavitevMikroProcesorja::OnRefresh, this);
	choiceVelicinaBranje->Bind(wxEVT_CHOICE, &NastavitevMikroProcesorja::OnRefresh, this);
	choiceLogFun->Bind(wxEVT_CHOICE, &NastavitevMikroProcesorja::OnRefresh, this);
	spinVrednostBranje->Bind(wxEVT_SPINCTRLDOUBLE, &NastavitevMikroProcesorja::OnRefresh, this);
	choicePinPisanje->Bind(wxEVT_CHOICE, &NastavitevMikroProcesorja::OnRefresh, this);
	choiceVelicinaPisanje->Bind(wxEVT_CHOICE, &NastavitevMikroProcesorja::OnRefresh, this);
	choiceVrednostPisanje->Bind(wxEVT_CHOICE, &NastavitevMikroProcesorja::OnRefresh, this);
	dodaj->Bind(wxEVT_BUTTON, &NastavitevMikroProcesorja::OnDodajClicked, this);
	izbrisi->Bind(wxEVT_BUTTON, &NastavitevMikroProcesorja::OnIzbrisiClicked, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(NastavitevMikroProcesorja::OnPaint));
}


void NastavitevMikroProcesorja::OnApplyClicked(wxCommandEvent& evt) {

}

void NastavitevMikroProcesorja::OnDodajClicked(wxCommandEvent& evt) {

	if (choiceVrednostPisanje->GetSelection() >= 0) {
		int sezSize = seznamStikal.size();
		seznamStikal.resize(sezSize + 1);
		seznamStikal[sezSize].resize(7);


		int pin;
		pin = std::stoi(static_cast<std::string> (choicePinBranje->GetString(choicePinBranje->GetSelection())));
		for (int i = 0; i < seznamPovezav.size(); i++) {
			if ((seznamPovezav[i][0] == izbranElement && seznamPovezav[i][1] == pin) || (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin)) {
				int zamik = 0;
				if (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin) zamik = -2;

				seznamStikal[sezSize][0] = seznamPovezav[i][2 + zamik];
				
				if (seznamElementov[seznamStikal[sezSize][0]][2] == 2) {
					
					if (choiceVelicinaBranje->GetString(choiceVelicinaBranje->GetSelection()) == " p") seznamStikal[sezSize][1] = 2;
				}
				else if (seznamElementov[seznamStikal[sezSize][0]][2] == 3) {
					if (choiceVelicinaBranje->GetString(choiceVelicinaBranje->GetSelection()) == " p0") seznamStikal[sezSize][1] = 2;
					else if (choiceVelicinaBranje->GetString(choiceVelicinaBranje->GetSelection()) == " p1") seznamStikal[sezSize][1] = 5;
				}
				else if (seznamElementov[seznamStikal[sezSize][0]][2] == 4) {
					if (choiceVelicinaBranje->GetString(choiceVelicinaBranje->GetSelection()) == " p") seznamStikal[sezSize][1] = 2;
				}
				//break;
			}
		}

		if (choiceLogFun->GetSelection() == 0) seznamStikal[sezSize][2] = -1;
		else if (choiceLogFun->GetSelection() == 1) seznamStikal[sezSize][2] = 0;
		else if (choiceLogFun->GetSelection() == 2) seznamStikal[sezSize][2] = 1;

		seznamStikal[sezSize][3] = spinVrednostBranje->GetValue() * 100000; ///////////////// Mogoc ni za vse primere treba pretvorbe med bar in Pa

		pin = std::stoi(static_cast<std::string> (choicePinPisanje->GetString(choicePinPisanje->GetSelection())));
		for (int i = 0; i < seznamPovezav.size(); i++) {
			if ((seznamPovezav[i][0] == izbranElement && seznamPovezav[i][1] == pin) || (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin)) {
				int zamik = 0;
				if (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin) zamik = -2;

				seznamStikal[sezSize][4] = seznamPovezav[i][2 + zamik];

				if (seznamElementov[seznamStikal[sezSize][4]][2] == 1) {
					if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " delovanje") seznamStikal[sezSize][5] = 0;
					if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " moc [W]") seznamStikal[sezSize][5] = 2;
				}
				else if (seznamElementov[seznamStikal[sezSize][4]][2] == 3) {
					if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " delovanje") seznamStikal[sezSize][5] = 0;
					else if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " pozicija") seznamStikal[sezSize][5] = 7;
				}
				else if (seznamElementov[seznamStikal[sezSize][4]][2] == 4) {
					if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " delovanje") seznamStikal[sezSize][5] = 0;
					else if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " pozicija") seznamStikal[sezSize][5] = 6;
				}
				//break;
			}
		}

		seznamStikal[sezSize][6] = std::stoi(static_cast<std::string> (choiceVrednostPisanje->GetString(choiceVrednostPisanje->GetSelection())));
	}
	Refresh();
}

void NastavitevMikroProcesorja::OnIzbrisiClicked(wxCommandEvent& evt) {

	seznamStikal.clear();

	Refresh();
}

void NastavitevMikroProcesorja::OnRefresh(wxCommandEvent& evt) {

	if (choiceVelicinaPisanje->GetSelection() >= 0) {
		choiceVrednostPisanje->Enable();

	}
	if (choicePinPisanje->GetSelection() >= 0) {
		choiceVelicinaPisanje->Enable();

		choiceVelicinaPisanje->SetString(0, " ");
		choiceVelicinaPisanje->SetString(1, " ");

		choiceVrednostPisanje->SetString(0, " ");
		choiceVrednostPisanje->SetString(1, " ");
		choiceVrednostPisanje->SetString(2, " ");

		int pin = std::stoi(static_cast<std::string> (choicePinPisanje->GetString(choicePinPisanje->GetSelection())));
		for (int i = 0; i < seznamPovezav.size(); i++) {
			if ((seznamPovezav[i][0] == izbranElement && seznamPovezav[i][1] == pin) || (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin)) {
				int zamik = 0;
				if (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin) zamik = -2;

				if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == 1) {
					choiceVelicinaPisanje->SetString(0, " delovanje");
					choiceVelicinaPisanje->SetString(1, " moc [W]");

					if (choiceVelicinaPisanje->GetSelection() >= 0) {
						if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " delovanje") {
							choiceVrednostPisanje->SetString(0, " -1");
							choiceVrednostPisanje->SetString(1, " 0");
							choiceVrednostPisanje->SetString(2, " 1");
						}
						else if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " moc [W]") {
							choiceVrednostPisanje->SetString(0, " 2000");
							choiceVrednostPisanje->SetString(1, " 4000");
							choiceVrednostPisanje->SetString(2, " 6000");
						}
					}
				}
				else if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == 3) {
					choiceVelicinaPisanje->SetString(0, " delovanje");
					choiceVelicinaPisanje->SetString(1, " pozicija");

					if (choiceVelicinaPisanje->GetSelection() >= 0) {
						if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " delovanje") {
							choiceVrednostPisanje->SetString(0, " 0");
							choiceVrednostPisanje->SetString(1, " 1");
						}
						else if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " pozicija") {
							choiceVrednostPisanje->SetString(0, " 0");
							choiceVrednostPisanje->SetString(1, " 1");
						}
					}
				}
				else if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == 4) {
					choiceVelicinaPisanje->SetString(0, " delovanje");
					choiceVelicinaPisanje->SetString(1, " pozicija");

					if (choiceVelicinaPisanje->GetSelection() >= 0) {
						if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " delovanje") {
							choiceVrednostPisanje->SetString(0, " 0");
							choiceVrednostPisanje->SetString(1, " 1");
						}
						else if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " pozicija") {
							choiceVrednostPisanje->SetString(0, " 0");
							choiceVrednostPisanje->SetString(1, " 1");
						}
					}
				}
				//break;
			}
		}
	}
	if (choiceLogFun->GetSelection() >= 0) {
		choicePinPisanje->Enable();
		spinVrednostBranje->Enable();

	}
	if (choiceVelicinaBranje->GetSelection() >= 0) {
		choiceLogFun->Enable();

	}
	if (choicePinBranje->GetSelection() >= 0) {
		choiceVelicinaBranje->Enable();

		choiceVelicinaBranje->SetString(0, " ");
		choiceVelicinaBranje->SetString(1, " ");

		int pin = std::stoi(static_cast<std::string> (choicePinBranje->GetString(choicePinBranje->GetSelection())));
		for (int i = 0; i < seznamPovezav.size(); i++) {
			if ((seznamPovezav[i][0] == izbranElement && seznamPovezav[i][1] == pin) || (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin)) {
				int zamik = 0;
				if (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin) zamik = -2;

				if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == 2) {
					choiceVelicinaBranje->SetString(0, " p");
				}
				else if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == 3) {
					choiceVelicinaBranje->SetString(0, " p0");
					choiceVelicinaBranje->SetString(1, " p1");
				}
				else if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == 4) {
					choiceVelicinaBranje->SetString(0, " p");
				}
				//break;
			}
		}
	}

	Refresh();
}


void NastavitevMikroProcesorja::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	if (true) { // ADMIN LOGS
		dc.DrawText("Zapis vseh ukazov:", wxPoint(200, 80));
		for (int i = 0; i < seznamStikal.size(); i++) dc.DrawText(wxString::Format("%g | %g | %g | %g | %g | %g | %g", seznamStikal[i][0], seznamStikal[i][1], seznamStikal[i][2], seznamStikal[i][3], seznamStikal[i][4], seznamStikal[i][5], seznamStikal[i][6]), wxPoint(200, 100 + 20 * i));
	} //////////////// Za izris stikal daj wxArray

	for (int i = 0; i < 8; i++) {
		short najdena = 0;
		for(int j = 0; j < seznamPovezav.size(); j++) {
			if (seznamPovezav[j][0] == izbranElement && seznamPovezav[j][1] == i) {
				
				if (seznamElementov[seznamPovezav[j][2]][2] == 2) dc.DrawText(wxString::Format("%d: Branje", i), wxPoint(5, 5 + 15 * i));
				else if (seznamElementov[seznamPovezav[j][2]][2] == 1) dc.DrawText(wxString::Format("%d: Pisanje", i), wxPoint(5, 5 + 15 * i));
				else dc.DrawText(wxString::Format("%d: /", i), wxPoint(5, 5 + 15 * i));
				
			}
			else if (seznamPovezav[j][2] == izbranElement && seznamPovezav[j][3] == i) {
				
				if (seznamElementov[seznamPovezav[j][0]][2] == 2) dc.DrawText(wxString::Format("%d: Branje", i), wxPoint(5, 5 + 15 * i));
				else if (seznamElementov[seznamPovezav[j][0]][2] == 1) dc.DrawText(wxString::Format("%d: Pisanje", i), wxPoint(5, 5 + 15 * i));
				else dc.DrawText(wxString::Format("%d: /", i), wxPoint(5, 5 + 15 * i));
				
			}
			else najdena++;
		}
		if (najdena == seznamPovezav.size()) dc.DrawText(wxString::Format("%d: -", i), wxPoint(5, 5 + 15 * i));
	}
}



wxSpinCtrlDouble* masTokCrpalke;

NastavitevCrpalke::NastavitevCrpalke() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve Crpalke"), wxPoint(0, 0), wxSize(360, 300)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(10, 230), wxDefaultSize);

	masTokCrpalke = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(100, 5), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 20, 0, .1); ///////////// odstanit


	apply->Bind(wxEVT_BUTTON, &NastavitevCrpalke::OnApplyClicked, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(NastavitevCrpalke::OnPaint));
}


void NastavitevCrpalke::OnApplyClicked(wxCommandEvent& evt) {

	seznamResitevReset[izbranElement][1] = masTokCrpalke->GetValue();
	seznamResitev = seznamResitevReset;
}


void NastavitevCrpalke::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());


	dc.DrawText("Masni tok: ", wxPoint(5, 5));
}



wxCheckBox* tlakVarnVentBool;
wxSpinCtrlDouble* tlakVarnVent;
wxSpinCtrlDouble* volumenTlacnePosode;
wxSpinCtrlDouble* zacTlakTlacnePosode;

NastavitevTlacnePosode::NastavitevTlacnePosode() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve Tlacne posode"), wxPoint(0, 0), wxSize(360, 300)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(10, 230), wxDefaultSize);

	tlakVarnVentBool = new wxCheckBox(panel, wxID_ANY, "Tlacni varnostni ventil", wxPoint(5, 5), wxDefaultSize);
	tlakVarnVent = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(180, 23), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, pogojiOkolja[0]/100000, 10, 7, .1);

	if (seznamLastnosti[izbranElement][1] >= 0) {
		tlakVarnVent->SetValue(seznamLastnosti[izbranElement][1]/100000);
		tlakVarnVentBool->SetValue(1);
	}
	else {
		tlakVarnVent->SetValue(0);
		tlakVarnVentBool->SetValue(0);
	}

	volumenTlacnePosode = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(180, 53), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0.1, 10, seznamLastnosti[izbranElement][0], 0.1);
	zacTlakTlacnePosode = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(180, 83), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 10, seznamResitevReset[izbranElement][2] / 100000, 0.1);


	apply->Bind(wxEVT_BUTTON, &NastavitevTlacnePosode::OnApplyClicked, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(NastavitevTlacnePosode::OnPaint));
}


void NastavitevTlacnePosode::OnApplyClicked(wxCommandEvent& evt) {

	int obstaja = -1;
	for (int i = 0; i < seznamPovezav.size(); i++) if (seznamPovezav[i][0] == izbranElement && seznamPovezav[i][4] == 2) obstaja = i;
	if (tlakVarnVentBool->IsChecked()) {
		seznamLastnosti[izbranElement][1] = tlakVarnVent->GetValue() * 100000;
		if (obstaja == -1) seznamPovezav.push_back({ izbranElement,3,-1,-1,2 });
	}
	else {
		seznamLastnosti[izbranElement][1] = -1;
		if (obstaja >= 0) seznamPovezav.erase(seznamPovezav.begin() + obstaja);
	}

	seznamLastnosti[izbranElement][0] = volumenTlacnePosode->GetValue();
	seznamResitevReset[izbranElement][2] = zacTlakTlacnePosode->GetValue() * 100000; ///////////////// napacna stvar
	
	seznamResitev = seznamResitevReset;
}


void NastavitevTlacnePosode::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	dc.DrawText("Tlak varnostnega ventila [bar]:", wxPoint(5, 25));
	dc.DrawText("Volumen Tlacne posode [m^3]: ", wxPoint(5, 55));
	dc.DrawText("Zacetni tlak v posodi [bar]: ", wxPoint(5, 85));
}



wxSpinCtrl* premerBataPrijemalke;
wxSpinCtrl* premerBatnicePrijemalke;
wxSpinCtrl* dolzinaHodaPrijemalke;
wxSpinCtrl* zacPozPrijemalke;

NastavitevPrijemalke::NastavitevPrijemalke() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve prijemalke"), wxPoint(0, 0), wxSize(360, 300)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(10, 230), wxDefaultSize);

	premerBataPrijemalke = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(160, 3), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 10, 300, seznamLastnosti[izbranElement][2] * 1000);
	premerBatnicePrijemalke = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(160, 33), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 10, premerBataPrijemalke->GetValue(), seznamLastnosti[izbranElement][3] * 1000);
	dolzinaHodaPrijemalke = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(160, 63), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 10, 300, seznamLastnosti[izbranElement][4] * 1000);
	zacPozPrijemalke = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(160, 93), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 100, seznamLastnosti[izbranElement][5]);


	apply->Bind(wxEVT_BUTTON, &NastavitevPrijemalke::OnApplyClicked, this);
	premerBataPrijemalke->Bind(wxEVT_SPINCTRL, &NastavitevPrijemalke::OnRefresh, this);
	premerBatnicePrijemalke->Bind(wxEVT_SPINCTRL, &NastavitevPrijemalke::OnRefresh, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(NastavitevPrijemalke::OnPaint));
}


void NastavitevPrijemalke::OnApplyClicked(wxCommandEvent& evt) {

	seznamLastnosti[izbranElement][2] = static_cast<double> (premerBataPrijemalke->GetValue()) / 1000;
	seznamLastnosti[izbranElement][3] = static_cast<double> (premerBatnicePrijemalke->GetValue()) / 1000;
	seznamLastnosti[izbranElement][4] = static_cast<double> (dolzinaHodaPrijemalke->GetValue()) / 1000;
	seznamLastnosti[izbranElement][5] = static_cast<double> (zacPozPrijemalke->GetValue());
}

void NastavitevPrijemalke::OnRefresh(wxCommandEvent& evt) {

	Refresh();
}


void NastavitevPrijemalke::OnPaint(wxPaintEvent& evt) {

	premerBatnicePrijemalke->SetRange(10, premerBataPrijemalke->GetValue());

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	dc.DrawText("Premer bata D [mm]: ", wxPoint(5, 5));
	dc.DrawText("Premer batnice d [mm]: ", wxPoint(5, 35));
	dc.DrawText("Hod bata l [mm]: ", wxPoint(5, 65));
	dc.DrawText("Zacetna pozicija bata D [%]: ", wxPoint(5, 95));
}



wxSpinCtrl* premerPriseska;
wxSpinCtrl* velikostPriseska;

NastavitevPriseska::NastavitevPriseska() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve priseska"), wxPoint(0, 0), wxSize(360, 300)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	premerPriseska = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(150, 3), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 5, 500, seznamLastnosti[izbranElement][0] * 1000);
	velikostPriseska = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(150, 33), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 5, 200, seznamLastnosti[izbranElement][1] * 1000);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(10, 230), wxDefaultSize);


	apply->Bind(wxEVT_BUTTON, &NastavitevPriseska::OnApplyClicked, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(NastavitevPriseska::OnPaint));
}


void NastavitevPriseska::OnApplyClicked(wxCommandEvent& evt) {

	seznamLastnosti[izbranElement][0] = static_cast<double> (premerPriseska->GetValue()) / 1000;
	seznamLastnosti[izbranElement][1] = static_cast<double> (velikostPriseska->GetValue()) / 1000;
}


void NastavitevPriseska::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());

	dc.DrawText("Premer priseska D [mm]: ", wxPoint(5,5));
	dc.DrawText("Velikost priseska l [mm]: ", wxPoint(5,35));
}
