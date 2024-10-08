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

#define MIKROPROCESOR 0
#define ELEKTRICNACRPALKA 1
#define TLACNAPOSODA 2
#define PRIJEMALO 3
#define PRISESEK 4
#define GRAF 5

#define MANJ -1
#define ENAKO 0
#define VECJE 1

#define TLAK0_LOG 2
#define TLAK1_LOG 5
#define DELOVANJE_LOG 0
#define MOC_LOG 2
#define VRTLJAJI_LOG 3
#define POZ_PRIJEMALO_LOG 7
#define POZ_PRISESEK_LOG 6
#define MASNI_TOK 1
#define BAT_POZ 8

#define KONST_MOC 0
#define KONST_VRTLJAJI 1

#define NI_REGULATORJA_TLAKA -1




std::vector<std::vector<double>> IzracunVolumna(std::vector<std::vector<int>> seznamElementov, std::vector<std::vector<double>> seznamResitevReset, std::vector<std::vector<double>> seznamLastnosti) {

	const double pi = M_PI;

	double V0_krmilni = .001;
	double V1_krmilni = .001;

	for (int i = 0; i < seznamResitevReset.size(); i++) {

		if (seznamElementov[i][2] == TLACNAPOSODA) {

			double V = seznamLastnosti[i][0];

			seznamResitevReset[i][3] = V;
		}
		else if (seznamElementov[i][2] == PRIJEMALO) {

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
		else if (seznamElementov[i][2] == PRISESEK) {

			double D = seznamLastnosti[i][0];
			double l = seznamLastnosti[i][1];

			double A = pi * pow(D, 2) / 4;

			double V = A * l;

			seznamResitevReset[i][3] = V;
		}
	}
	return seznamResitevReset;
}

std::vector<std::vector<double>> IzracunMase(PogojiOkolja pogojiOkolja, std::vector<std::vector<int>> seznamElementov, std::vector<std::vector<double>> seznamResitevReset) {

	double R = pogojiOkolja.plinskaKonstanta;
	double T = pogojiOkolja.temperaturaOzracja;

	for (int i = 0; i < seznamResitevReset.size(); i++) if (/*seznamResitevReset[i][1] == -1 &&*/ (seznamElementov[i][2] == TLACNAPOSODA || seznamElementov[i][2] == PRIJEMALO || seznamElementov[i][2] == PRISESEK)) {
		double p = seznamResitevReset[i][2];
		double V = seznamResitevReset[i][3];

		seznamResitevReset[i][1] = p * V / (R * T);

		if (seznamElementov[i][2] == PRIJEMALO) {
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
std::vector<double> IzracunPrijemala(PogojiOkolja pogojiOkolja, std::vector<double> lasti, std::vector<double> resi, double korak) {


	const double pi = M_PI;

	double g = pogojiOkolja.gravitacijskiPospesek;
	double pok = pogojiOkolja.tlakOzracja;

	double ti = korak;

	double koef_tr_st = .8; //////////////// preureditev - brez koeficienta direk sila
	double koef_tr_din = .6;


	double D = lasti[2];
	double d = lasti[3];
	double l = lasti[4];
	double m = .8; /////////////// izra�unat

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

std::vector<double> IzracunPriseska(PogojiOkolja pogojiOkolja, std::vector<double> lasti, std::vector<double> resi, double korak) {
	
	double pi = M_PI;


	double g = pogojiOkolja.gravitacijskiPospesek;
	double m = resi[4];

	double Fg = m * g;


	double p = resi[2];
	double pok = pogojiOkolja.tlakOzracja;
	double d = lasti[0];
	double k = .8; // Koeficient priseska ///////////////////// dodat v lastnosti

	double A = d * d * pi / 4;

	double Fp = (pok - p) * A * k;


	if (Fp > Fg) resi[5] = 1;
	else resi[5] = -1;

	return resi;
}


std::vector<std::vector<double>> IzracunPovezav(PogojiOkolja pogojiOkolja, std::vector<std::vector<int>> seznamElementov, std::vector<std::vector<double>> seznamLastnosti, std::vector<std::vector<double>> seznamResitev, std::vector<std::vector<int>> seznamPovezav, std::vector<std::vector<double>> seznamStikal, double korak, int cas) {

	std::vector<std::vector<double>> masniTok;
	masniTok.resize(seznamElementov.size());

	double pi = M_PI;

	double ti = korak; // Casovni korak [s]

	double g = pogojiOkolja.gravitacijskiPospesek;
	double R = pogojiOkolja.plinskaKonstanta; // Masna plinska konstanta [J/kgK]
	double T = pogojiOkolja.temperaturaOzracja; // Temperatura zraka [K]
	double gama = 1.4; // Razmerje sprecificnih toplot [/]
	double C = .6; // Koeficient prretoka [/]
	double d = .01; // Premer cevi [m]
	double A = pi * d * d / 4; // Prerez cevi [m^2]


	//- STIKALA
	for (int i = 0; i < seznamStikal.size(); i++) {

		if (seznamStikal[i][0] == -1) {

			if (seznamStikal[i][2] == MANJ) {

				if (cas * korak < seznamStikal[i][3]) seznamResitev[seznamStikal[i][4]][seznamStikal[i][5]] = seznamStikal[i][6];
			}

			else if (seznamStikal[i][2] == ENAKO) {

				if (cas * korak == seznamStikal[i][3]) seznamResitev[seznamStikal[i][4]][seznamStikal[i][5]] = seznamStikal[i][6];
			}

			else if (seznamStikal[i][2] == VECJE) {

				if (cas * korak > seznamStikal[i][3]) seznamResitev[seznamStikal[i][4]][seznamStikal[i][5]] = seznamStikal[i][6];
			}
		}

		else {
			if (seznamStikal[i][2] == MANJ) {

				if (seznamResitev[seznamStikal[i][0]][seznamStikal[i][1]] < seznamStikal[i][3]) seznamResitev[seznamStikal[i][4]][seznamStikal[i][5]] = seznamStikal[i][6];
			}

			else if (seznamStikal[i][2] == ENAKO) {

				if (seznamResitev[seznamStikal[i][0]][seznamStikal[i][1]] == seznamStikal[i][3]) seznamResitev[seznamStikal[i][4]][seznamStikal[i][5]] = seznamStikal[i][6];
			}

			else if (seznamStikal[i][2] == VECJE) {

				if (seznamResitev[seznamStikal[i][0]][seznamStikal[i][1]] > seznamStikal[i][3]) seznamResitev[seznamStikal[i][4]][seznamStikal[i][5]] = seznamStikal[i][6];
			}
		}
	}

	//- IZRACUN MASNEGA TOKA CEZ CRPALKO
	bool crpalkaBool = false;
	for (int i = 0; i < seznamPovezav.size(); i++) { if (seznamPovezav[i][4] == 1 && (seznamElementov[seznamPovezav[i][0]][2] == ELEKTRICNACRPALKA || seznamElementov[seznamPovezav[i][2]][2] == ELEKTRICNACRPALKA)) { crpalkaBool = true; break; } }
	
	if (crpalkaBool) {
		std::vector<std::vector<int>> crpalkaElementi; // [porabnik, dovodnik, smer_toka, crpalka]
		crpalkaElementi.resize(4);


		double V1, V2;
		double p1, p2;
		double m1, m2;
		double rho1, rho2;

		for (int i = 0; i < seznamPovezav.size(); i++) if (seznamPovezav[i][4] == 1) { ////////////// deluje samo za en prikljucek na vhod in izhod crpalke

			if (seznamElementov[seznamPovezav[i][0]][2] == ELEKTRICNACRPALKA && (seznamResitev[seznamPovezav[i][0]][0] == 1 || seznamResitev[seznamPovezav[i][0]][0] == -1)) {
				if (seznamResitev[seznamPovezav[i][2]][0] == 1) {
					if (seznamPovezav[i][1] == 1) crpalkaElementi[0].push_back(seznamPovezav[i][2]);
					else if (seznamPovezav[i][1] == 2) crpalkaElementi[1].push_back(seznamPovezav[i][2]);
					crpalkaElementi[3].push_back(seznamPovezav[i][0]);
				}
			}
			else if (seznamElementov[seznamPovezav[i][2]][2] == ELEKTRICNACRPALKA && (seznamResitev[seznamPovezav[i][2]][0] == 1 || seznamResitev[seznamPovezav[i][2]][0] == -1)) {
				if (seznamResitev[seznamPovezav[i][0]][0] == 1) {
					if (seznamPovezav[i][3] == 1) crpalkaElementi[0].push_back(seznamPovezav[i][0]);
					else if (seznamPovezav[i][3] == 2) crpalkaElementi[1].push_back(seznamPovezav[i][0]);
					crpalkaElementi[3].push_back(seznamPovezav[i][2]);
				}
			}
			if ((seznamElementov[seznamPovezav[i][0]][2] == ELEKTRICNACRPALKA && seznamResitev[seznamPovezav[i][0]][0] == 1) || (seznamElementov[seznamPovezav[i][2]][2] == ELEKTRICNACRPALKA && seznamResitev[seznamPovezav[i][2]][0] == 1))
				crpalkaElementi[2].push_back(1); /////////// ustvari vec podatkov, ko bi moral biti samo eden
			else if ((seznamElementov[seznamPovezav[i][0]][2] == ELEKTRICNACRPALKA && seznamResitev[seznamPovezav[i][0]][0] == -1) || (seznamElementov[seznamPovezav[i][2]][2] == ELEKTRICNACRPALKA && seznamResitev[seznamPovezav[i][2]][0] == -1))
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

				p1 = pogojiOkolja.tlakOzracja;
				rho1 = pogojiOkolja.gostotaOkoljskegaZraka;
			}
			if (!crpalkaElementi[1].empty()) {

				V2 = seznamResitev[crpalkaElementi[1][i]][3];
				p2 = seznamResitev[crpalkaElementi[1][i]][2];
				m2 = seznamResitev[crpalkaElementi[1][i]][1];
				rho2 = m2 / V2;
			}
			else {

				p2 = pogojiOkolja.tlakOzracja;
				rho2 = pogojiOkolja.gostotaOkoljskegaZraka;
			}

			double izkc = seznamLastnosti[crpalkaElementi[3][i]][0];
			double ncmax = seznamLastnosti[crpalkaElementi[3][i]][1] * (2 * pi);
			double Vc = seznamLastnosti[crpalkaElementi[3][i]][2] / (2 * pi);
			double Ac = seznamLastnosti[crpalkaElementi[3][i]][3];
			double lc = seznamLastnosti[crpalkaElementi[3][i]][4];

			double mtok = seznamResitev[crpalkaElementi[3][i]][1]; ///////////////////// koznost za konst. vrt.
			double Pc = seznamResitev[crpalkaElementi[3][i]][2];
			double nc = seznamResitev[crpalkaElementi[3][i]][3];


			double dT = (p1 - p2) * Ac * lc * crpalkaElementi[2][i];

			if (seznamLastnosti[crpalkaElementi[3][i]][5] == KONST_MOC) {

				if (dT >= 0) {
					nc = Pc * izkc / dT;
					if (nc > ncmax) nc = ncmax;
				}
				else if (dT < 0) nc = ncmax;

				nc *= crpalkaElementi[2][i];
			}
			else if (seznamLastnosti[crpalkaElementi[3][i]][5] == KONST_VRTLJAJI) {

				Pc = abs(nc * dT / izkc);
			}


			if (crpalkaElementi[2][i] == 1) {
				if (!crpalkaElementi[1].empty()) mtok = (m2 - m2 * (V2 / (V2 + Vc))) * nc;
				else mtok = rho2 * Vc * nc;
			}
			else if (crpalkaElementi[2][i] == -1) {
				if (!crpalkaElementi[0].empty()) mtok = (m1 - m1 * (V1 / (V1 + Vc))) * nc;
				else mtok = rho1 * Vc * nc;
			}


			seznamResitev[crpalkaElementi[3][i]][1] = mtok;
			seznamResitev[crpalkaElementi[3][i]][2] = Pc;
			seznamResitev[crpalkaElementi[3][i]][3] = nc;

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

			if (seznamElementov[seznamPovezav[i][0]][2] == ELEKTRICNACRPALKA || seznamElementov[seznamPovezav[i][2]][2] == ELEKTRICNACRPALKA || seznamElementov[seznamPovezav[i][0]][2] == PRISESEK || seznamElementov[seznamPovezav[i][2]][2] == PRISESEK) {

			}
			else {

				double V1, V2;
				double p1, p2;
				double m1, m2;

				if (seznamElementov[seznamPovezav[i][0]][2] == TLACNAPOSODA) {
					V1 = seznamResitev[seznamPovezav[i][0]][3];
					p1 = seznamResitev[seznamPovezav[i][0]][2];
					m1 = seznamResitev[seznamPovezav[i][0]][1];
				}
				else if (seznamElementov[seznamPovezav[i][0]][2] == PRIJEMALO) {
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

				if (seznamElementov[seznamPovezav[i][2]][2] == TLACNAPOSODA) {
					V2 = seznamResitev[seznamPovezav[i][2]][3];
					p2 = seznamResitev[seznamPovezav[i][2]][2];
					m2 = seznamResitev[seznamPovezav[i][2]][1];
				}
				else if (seznamElementov[seznamPovezav[i][2]][2] == PRIJEMALO) {
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

				if (seznamPovezav[i][5] == NI_REGULATORJA_TLAKA) {
					if (p1 > p2) mtok = -C * A * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama)));
					else if (p1 < p2) mtok = C * A * sqrt(2 * rho2 * p2 * (gama / (gama - 1)) * (pow(p1 / p2, 2 / gama) - pow(p1 / p2, (gama + 1) / gama)));
				}
				else { ////////////////////////// Mozne spremembe kar se tice tlacne razlike
					if (p1 > p2 && p2 < seznamPovezav[i][5]) mtok = -C * A * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama)));
					else if (p1 < p2 && p1 < seznamPovezav[i][5]) mtok = C * A * sqrt(2 * rho2 * p2 * (gama / (gama - 1)) * (pow(p1 / p2, 2 / gama) - pow(p1 / p2, (gama + 1) / gama)));
				}


				if (seznamElementov[seznamPovezav[i][0]][2] == TLACNAPOSODA) masniTok[seznamPovezav[i][0]].push_back(0);
				else if (seznamElementov[seznamPovezav[i][0]][2] == PRIJEMALO) masniTok[seznamPovezav[i][0]].push_back(seznamResitev[seznamPovezav[i][0]][7]);
				masniTok[seznamPovezav[i][0]].push_back(mtok);
				if (seznamElementov[seznamPovezav[i][2]][2] == TLACNAPOSODA) masniTok[seznamPovezav[i][2]].push_back(0);
				else if (seznamElementov[seznamPovezav[i][2]][2] == PRIJEMALO) masniTok[seznamPovezav[i][2]].push_back(seznamResitev[seznamPovezav[i][2]][7]);
				masniTok[seznamPovezav[i][2]].push_back(-mtok);
			}
		}

		//- ODZRACEVANJE
		else if (seznamPovezav[i][4] == 2) {

			if (seznamElementov[seznamPovezav[i][0]][2] == TLACNAPOSODA) {
				if (seznamResitev[seznamPovezav[i][0]][2] > seznamLastnosti[seznamPovezav[i][0]][1]) {
					double V1;
					double p1, p2;
					double m1;

					V1 = seznamResitev[seznamPovezav[i][0]][3];
					p1 = seznamResitev[seznamPovezav[i][0]][2];
					m1 = seznamResitev[seznamPovezav[i][0]][1];

					double rho1 = m1 / V1;

					p2 = pogojiOkolja.tlakOzracja;

					double mtok = 0;
					mtok = -C * 8 * A * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama)));

					masniTok[seznamPovezav[i][0]].push_back(0);
					masniTok[seznamPovezav[i][0]].push_back(mtok);
				}
			}
			else if (seznamElementov[seznamPovezav[i][0]][2] == PRIJEMALO) {
				if (seznamResitev[seznamPovezav[i][0]][7] == 0 && (seznamResitev[seznamPovezav[i][0]][5] > pogojiOkolja.tlakOzracja || seznamResitev[seznamPovezav[i][0]][5] < pogojiOkolja.tlakOzracja)) {
					double V1;
					double p1, p2;
					double m1;

					V1 = seznamResitev[seznamPovezav[i][0]][6];
					p1 = seznamResitev[seznamPovezav[i][0]][5];
					m1 = seznamResitev[seznamPovezav[i][0]][4];

					double rho1 = m1 / V1;

					p2 = pogojiOkolja.tlakOzracja;

					double rho2 = pogojiOkolja.gostotaOkoljskegaZraka;

					double mtok = 0;
					if (seznamResitev[seznamPovezav[i][0]][5] > pogojiOkolja.tlakOzracja) mtok = -C * 3 * A * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama))); //std::cout << "513, ";
					else if (seznamResitev[seznamPovezav[i][0]][5] < pogojiOkolja.tlakOzracja) mtok = C * 3 * A * sqrt(2 * rho2 * p2 * (gama / (gama - 1)) * (pow(p1 / p2, 2 / gama) - pow(p1 / p2, (gama + 1) / gama))); //std::cout << "514, ";

					masniTok[seznamPovezav[i][0]].push_back(1);
					masniTok[seznamPovezav[i][0]].push_back(mtok);
				}
				else if (seznamResitev[seznamPovezav[i][0]][7] == 1 && (seznamResitev[seznamPovezav[i][0]][2] > pogojiOkolja.tlakOzracja || seznamResitev[seznamPovezav[i][0]][2] < pogojiOkolja.tlakOzracja)) {
					double V1;
					double p1, p2;
					double m1;

					V1 = seznamResitev[seznamPovezav[i][0]][3];
					p1 = seznamResitev[seznamPovezav[i][0]][2];
					m1 = seznamResitev[seznamPovezav[i][0]][1];

					double rho1 = m1 / V1;

					p2 = pogojiOkolja.tlakOzracja;

					double rho2 = pogojiOkolja.gostotaOkoljskegaZraka;

					double mtok = 0;
					if (seznamResitev[seznamPovezav[i][0]][2] > pogojiOkolja.tlakOzracja) mtok = -C * 3 * A * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama)));
					else if (seznamResitev[seznamPovezav[i][0]][2] < pogojiOkolja.tlakOzracja) mtok = C * 3 * A * sqrt(2 * rho2 * p2 * (gama / (gama - 1)) * (pow(p1 / p2, 2 / gama) - pow(p1 / p2, (gama + 1) / gama)));

					masniTok[seznamPovezav[i][0]].push_back(0);
					masniTok[seznamPovezav[i][0]].push_back(mtok);
				}
			}
			else if (seznamElementov[seznamPovezav[i][0]][2] == PRISESEK) {
				if (seznamResitev[seznamPovezav[i][0]][4] < 0 || (seznamResitev[seznamPovezav[i][0]][5] == -1 && seznamResitev[seznamPovezav[i][0]][6] == -1)) {
					double V1;
					double p1, p2;
					double m1;

					V1 = seznamResitev[seznamPovezav[i][0]][3];
					p1 = seznamResitev[seznamPovezav[i][0]][2];
					m1 = seznamResitev[seznamPovezav[i][0]][1];

					double rho1 = m1 / V1;

					p2 = pogojiOkolja.tlakOzracja;

					double rho2 = pogojiOkolja.gostotaOkoljskegaZraka;

					double mtok = 0;
					if (seznamResitev[seznamPovezav[i][0]][2] > pogojiOkolja.tlakOzracja) mtok = -C * 20 * A * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama)));
					else if (seznamResitev[seznamPovezav[i][0]][2] < pogojiOkolja.tlakOzracja) mtok = C * 20 * A * sqrt(2 * rho2 * p2 * (gama / (gama - 1)) * (pow(p1 / p2, 2 / gama) - pow(p1 / p2, (gama + 1) / gama)));

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

					p2 = pogojiOkolja.tlakOzracja;

					double rho2 = pogojiOkolja.gostotaOkoljskegaZraka;

					double mtok = 0;
					if (seznamResitev[seznamPovezav[i][0]][2] > pogojiOkolja.tlakOzracja) mtok = -C * (A / 20) * sqrt(2 * rho1 * p1 * (gama / (gama - 1)) * (pow(p2 / p1, 2 / gama) - pow(p2 / p1, (gama + 1) / gama)));
					else if (seznamResitev[seznamPovezav[i][0]][2] < pogojiOkolja.tlakOzracja) mtok = C * (A / 20) * sqrt(2 * rho2 * p2 * (gama / (gama - 1)) * (pow(p1 / p2, 2 / gama) - pow(p1 / p2, (gama + 1) / gama)));

					masniTok[seznamPovezav[i][0]].push_back(0);
					masniTok[seznamPovezav[i][0]].push_back(mtok);
				}
			}
		}
	}

	//- IZRACUN TLAKA:
	for (int i = 0; i < masniTok.size(); i++) if (masniTok[i].size() > 0) {

		double m1 = 0, m2 = 0; // Masa
		double V1 = 0, V2 = 0; // Volumen
		double p1 = 0, p2 = 0; // Tlak

		for (int j = 0; j < masniTok[i].size(); j += 2) seznamResitev[i][1 + 3 * masniTok[i][j]] += masniTok[i][j + 1] * ti;

		if (seznamElementov[i][2] == TLACNAPOSODA || seznamElementov[i][2] == PRISESEK) {

			m1 = seznamResitev[i][1];
			V1 = seznamResitev[i][3];

			p1 = m1 * R * T / V1;

			seznamResitev[i][2] = p1;
		}
		else if (seznamElementov[i][2] == PRIJEMALO) {

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
		if (seznamElementov[i][2] == PRIJEMALO) seznamResitev[i] = IzracunPrijemala(pogojiOkolja, seznamLastnosti[i], seznamResitev[i], korak);
		else if (seznamElementov[i][2] == PRISESEK) seznamResitev[i] = IzracunPriseska(pogojiOkolja, seznamLastnosti[i], seznamResitev[i], korak);
	}

	return seznamResitev;
}


void ZapisMeritev(std::vector<std::vector<double>>* seznamGrafTock, std::vector<std::vector<int>>* seznamElementov, std::vector<std::vector<double>>* seznamLastnosti, std::vector<std::vector<double>>* seznamResitev, double korak) {

	int velikost = seznamElementov->size();

	seznamGrafTock->resize(velikost + 1);

	for (int i = 0; i < seznamElementov->size(); i++) 
		if ((*seznamElementov)[i][2] == GRAF && (*seznamLastnosti)[i][2] != -1 && (*seznamLastnosti)[i][3] != -1) 
			(*seznamGrafTock)[i].push_back((*seznamResitev)[(*seznamLastnosti)[i][2]][(*seznamLastnosti)[i][3]]);

	(*seznamGrafTock)[velikost].push_back(korak * (*seznamGrafTock)[velikost].size());
}




bool shranjeno = true;
bool drzanje = false;
bool drzanjeElementa = false;
int casDrzanja = 0;
short drzanjePovezav = 0;
short izbranElement = -1;
short izbranaPovezava = -1;

PogojiOkolja pogojiOkolja;
 
std::vector<std::vector<int>> seznamElementov;
// [x, y, element]
std::vector<std::vector<double>> seznamLastnosti;
// mikroprocesor []
// tlacna crpalka [izkoristek, notranja_torzija, volumen_na_vrtljaj, povrsina_rotorja, rocica_prijemalisca_sile, konst. moc/vrtljaji]
// tlacna posoda [volumen, varnostni ventil]
// prijemalo [/OUTDATED/, /OUTDATED/, D, d, l, %]
// prisesek [D, l]
// graf [debelina, visina, element, vrednost]
std::vector<std::vector<double>> seznamResitevReset;
// mikroprocesor [delovanje0 (0/1), delovanje1(0/1), delovanje2(0/1), delovanje3(0/1), delovanje4(0/1), delovanje5(0/1), delovanje6(0/1), delovanje7(0/1)]
// tlacna crpalka [delovanje (0/1), masni_tok, moc, obrati]
// tlacna posoda [delovanje (0/1), masa_zraka, tlak, volumen]
// prijemalo [delovanje (0/1), masa_zraka1, tlak1, volumen1, masa_zraka2, tlak2, volumen2, prikljucen_ventila_4/2, x, v, a]
// prisesek [delovanje (0/1), masa_zraka, tlak, volumen, utez, drzanje, pozicija]
// graf []
std::vector<std::vector<double>> seznamResitev;
// seznamResitev = seznamResitevReset

std::vector<std::vector<int>> seznamPovezav; /////////////////// premer cevi se nikjer ne uposteva
// [element1, prikljucen1, element2, prikljucek2, kabl/cev/odzracitev (0/1/2), regulator_tlaka(Pa), premer_cevi/odzracevanja(mm)]
std::vector<std::vector<double>> seznamStikal;
// [element1, velicina1, logicna_funkcija, vrednost1, element2, velicina2, vrednost2]
std::vector<std::vector<double>> seznamGrafTock;
// [cas, vrednost_graf1, vrednost_graf2]

wxChoice* choiceDod;
wxGauge* casSimulacije;
wxSpinCtrl* spinPovezava;

bool simbool = false;
double korak = .0005; // Casovni korak simulacije [s]


OknoSim::OknoSim(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* izbElement = new wxButton(panel, wxID_ANY, "Izbrisi izbrani element", wxPoint(5, 220), wxSize(190, -1));
	wxButton* izbVse = new wxButton(panel, wxID_ANY, "Izbrisi vse", wxPoint(5, 250), wxSize(190, -1));
	
	wxButton* risanjePovezav = new wxButton(panel, wxID_ANY, "Risanje povezav", wxPoint(5,280), wxSize(190,-1));
	spinPovezava = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(5, 310), wxSize(90, 53), wxSP_ARROW_KEYS | wxSP_WRAP, -1, -1, -1);
	wxButton* nastavitevPovezav = new wxButton(panel, wxID_ANY, "Nastavitve", wxPoint(105, 310), wxSize(90, -1));
	wxButton* brisanjePovezav = new wxButton(panel, wxID_ANY, "Brisi povezavo", wxPoint(105, 340), wxSize(90, -1));
	
	wxButton* shrani = new wxButton(panel, wxID_ANY, "Shrani", wxPoint(5, 380), wxSize(190, -1));
	wxButton* nalozi = new wxButton(panel, wxID_ANY, "Nalozi", wxPoint(5, 410), wxSize(190, -1));
	
	wxButton* simuliraj = new wxButton(panel, wxID_ANY, "Simuliraj", wxPoint(5, 500), wxSize(190, 40));
	wxButton* resetSim = new wxButton(panel, wxID_ANY, "Reset", wxPoint(5, 570), wxSize(190, -1));

	casSimulacije = new wxGauge(panel, wxID_ANY, 10000, wxPoint(5, 548), wxSize(190, -1), wxGA_HORIZONTAL, wxDefaultValidator, "cas");
	
	wxArrayString choices;
	choices.Add("Mikroprocesor");
	choices.Add("Elektricna Crpalka");
	choices.Add("Tlacna Posoda");
	choices.Add("Prijemalo");
	choices.Add("Prisesek");
	choices.Add("Graf");

	choiceDod = new wxChoice(panel, wxID_ANY, wxPoint(5, 0), wxSize(190, -1), choices/*, wxCB_SORT*/);
	choiceDod->SetSelection(0);


	this->Bind(wxEVT_CLOSE_WINDOW, &OknoSim::OnClose, this);
	panel->Bind(wxEVT_SIZE, &OknoSim::OnSizeChanged, this);
	panel->Bind(wxEVT_MOTION, &OknoSim::RefreshEvent, this);
	panel->Bind(wxEVT_LEFT_DOWN, &OknoSim::OnMouseDownEvent, this);
	panel->Bind(wxEVT_LEFT_UP, &OknoSim::OnMouseUpEvent, this);
	panel->Bind(wxEVT_LEFT_DCLICK, &OknoSim::OnMouseDoubleEvent, this);
	izbElement->Bind(wxEVT_BUTTON, &OknoSim::OnButtonIzbClicked, this);
	izbVse->Bind(wxEVT_BUTTON, &OknoSim::OnButtonIzbVseClicked, this);
	risanjePovezav->Bind(wxEVT_BUTTON, &OknoSim::OnRisanjePovezavClicked, this);
	spinPovezava->Bind(wxEVT_SPINCTRL, &OknoSim::OnSpinPovezavaChanged, this);
	nastavitevPovezav->Bind(wxEVT_BUTTON, &OknoSim::OnNastavitevPovezavClicked, this);
	brisanjePovezav->Bind(wxEVT_BUTTON, &OknoSim::OnBrisanjePovezavClicked, this);
	shrani->Bind(wxEVT_BUTTON, &OknoSim::OnShraniClicked, this);
	nalozi->Bind(wxEVT_BUTTON, &OknoSim::OnNaloziClicked, this);
	simuliraj->Bind(wxEVT_BUTTON, &OknoSim::OnSimulirajClicked, this);
	resetSim->Bind(wxEVT_BUTTON, &OknoSim::OnResetSimClicked, this);
	
	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(OknoSim::OnPaint));

	panel->SetDoubleBuffered(true);



	seznamElementov.push_back({ 220,20,MIKROPROCESOR });
	seznamElementov.push_back({ 360,420,ELEKTRICNACRPALKA });
	seznamElementov.push_back({ 590,330,TLACNAPOSODA });
	seznamElementov.push_back({ 870,230,PRIJEMALO });
	seznamElementov.push_back({ 510,540,PRISESEK });
	seznamElementov.push_back({ 830,30,GRAF });
	//seznamElementov.push_back({ 680,250,PRIJEMALO });

	seznamLastnosti.push_back({});
	seznamLastnosti.push_back({ .95,6,0.2,0.01,0.05,1 });
	seznamLastnosti.push_back({ .04,500000 });
	seznamLastnosti.push_back({ 700000,700000,0.032,0.012,0.03,0 });
	seznamLastnosti.push_back({ 0.1,0.2 });
	seznamLastnosti.push_back({ 240,100,2,2 });
	//seznamLastnosti.push_back({ 700000,700000,0.1,0.025,0.4,40 });

	seznamResitevReset.push_back({ 0,0,0,0,0,0,0,0 });
	seznamResitevReset.push_back({ 0,0,2000,20 });
	seznamResitevReset.push_back({ 1,-1,450000,-1 });
	seznamResitevReset.push_back({ 1,-1,pogojiOkolja.tlakOzracja,-1,-1,pogojiOkolja.tlakOzracja,-1,0,0,0,0 });
	seznamResitevReset.push_back({ 1,-1,pogojiOkolja.tlakOzracja,-1, 6, -1, -1 });
	seznamResitevReset.push_back({});
	//seznamResitevReset.push_back({ 1,-1,pogojiOkolja.tlakOzracja,-1,-1,pogojiOkolja.tlakOzracja,-1,0,0,0,0 });

	seznamResitevReset = IzracunVolumna(seznamElementov, seznamResitevReset, seznamLastnosti);
	seznamResitevReset = IzracunMase(pogojiOkolja, seznamElementov, seznamResitevReset);

	seznamResitev = seznamResitevReset;


	seznamPovezav.push_back({ 0,0,1,0,0,-1,-1 });
	seznamPovezav.push_back({ 0,1,2,0,0,-1,-1 });
	//seznamPovezav.push_back({ 0,2,5,0,0,-1,-1 });
	seznamPovezav.push_back({ 0,3,3,0,0,-1,-1 });
	seznamPovezav.push_back({ 0,4,4,0,0,-1,-1 });

	seznamPovezav.push_back({ 1,1,2,1,1,-1,12 });
	seznamPovezav.push_back({ 2,2,3,1,1,300000,12 });
	//seznamPovezav.push_back({ 5,1,2,2,1,450000,12 });
	seznamPovezav.push_back({ 1,2,4,1,1,-1,12 });

	seznamPovezav.push_back({ 2,3,-1,-1,2,-1,100 });
	seznamPovezav.push_back({ 3,3,-1,-1,2,-1,100 });
	seznamPovezav.push_back({ 4,3,-1,-1,2,-1,100 });
	//seznamPovezav.push_back({ 5,3,-1,-1,2,-1,100 });


	/*seznamStikal.push_back({2,2,-1,595000,1,0,1,0});
	seznamStikal.push_back({ 5,2,1,500000,5,7,1,0 });
	seznamStikal.push_back({ 5,5,1,500000,5,7,0,0 });
	seznamStikal.push_back({ 5,5,1,500000,4,6,-1,0 });
	seznamStikal.push_back({ 5,2,1,500000,4,6,1,0 });
	seznamStikal.push_back({ 2,2,-1,588000,1,0,-1,0 });
	seznamStikal.push_back({ 2,2,-1,500000,1,0,1,0 });
	seznamStikal.push_back({ 2,2,-1,500000,4,0,0,0 });*/

	//seznamStikal.push_back({ -1,0,VECJE,0,1,DELOVANJE_LOG,1,0 });
	seznamStikal.push_back({ 2,2,MANJ,300000,1,DELOVANJE_LOG,1,0 });
	seznamStikal.push_back({ 3,2,VECJE,300000,3,POZ_PRIJEMALO_LOG,1,0 });
	seznamStikal.push_back({ 3,5,VECJE,300000,3,POZ_PRIJEMALO_LOG,0,0 });
	seznamStikal.push_back({ 2,2,VECJE,450000,1,DELOVANJE_LOG,0,0 });
	//seznamStikal.push_back({ -1,0,VECJE,4,3,DELOVANJE_LOG,0,0 });
	

	wxStatusBar* statusBar = CreateStatusBar();
}


void OknoSim::OnClose(wxCloseEvent& evt) {

	if (evt.CanVeto() && shranjeno == false) {

		if (wxMessageBox("Dokument ni shranjen...\nCe zaprete aplikacijo boste izgubili svoje delo.", "Neshranjeno delo", wxICON_WARNING | wxOK | wxCANCEL) != wxOK) { //////////////// Mal grdo napisan

			evt.Veto();
			return;
		}
	}

	seznamGrafTock.clear();
	evt.Skip(); // Ali 'Destroy()'
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

			if (seznamElementov[i][2] == MIKROPROCESOR) {
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
			else if (seznamElementov[i][2] == ELEKTRICNACRPALKA) { 
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
			else if (seznamElementov[i][2] == TLACNAPOSODA) {
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
			else if (seznamElementov[i][2] == PRIJEMALO) { 
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
			else if (seznamElementov[i][2] == PRISESEK) { 
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
			else if (seznamElementov[i][2] == GRAF) {
				if (mousePos.x > seznamElementov[i][0] && mousePos.x < seznamElementov[i][0] + seznamLastnosti[i][0] && mousePos.y > seznamElementov[i][1] && mousePos.y < seznamElementov[i][1] + seznamLastnosti[i][1]) {
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
			seznamPovezav.push_back({ -1,-1,-1,-1,-1,-1 });
			drzanjePovezav = 1;
		}
	}

	casDrzanja = 0;
	
	Refresh();
}

void OknoSim::OnMouseUpEvent(wxMouseEvent& evt) {

	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());
	wxSize oknoSize = this->GetSize();


	if (drzanje && drzanjePovezav > 0) {

		drzanjePovezav = 0;
		seznamPovezav.erase(seznamPovezav.begin() + seznamPovezav.size() - 1);
	}

	
	if (izbranElement >= 0 && casDrzanja > 2 && mousePos.x > oknoSize.x - 115 && mousePos.y < 60 && drzanjeElementa) {
		
		casSimulacije->SetValue(0);

		for (int i = seznamPovezav.size() - 1; i >= 0; i--) if (seznamPovezav[i][0] == izbranElement || seznamPovezav[i][2] == izbranElement) seznamPovezav.erase(seznamPovezav.begin() + i); // brisanje cevi
		for (int i = seznamStikal.size() - 1; i >= 0; i--) if (seznamStikal[i][0] == izbranElement || seznamStikal[i][4] == izbranElement) seznamStikal.erase(seznamStikal.begin() + i); // brisanje stikal

		for (int i = 0; i < seznamPovezav.size(); i++) {
			if (seznamPovezav[i][0] > izbranElement) seznamPovezav[i][0]--;
			if (seznamPovezav[i][2] > izbranElement) seznamPovezav[i][2]--;
		}

		for (int i = 0; i < seznamStikal.size(); i++) {
			if (seznamStikal[i][0] > izbranElement) seznamStikal[i][0]--;
			if (seznamStikal[i][4] > izbranElement) seznamStikal[i][4]--;
			if (seznamStikal[i][7] > izbranElement) seznamStikal[i][7]--;
		}

		if (seznamElementov[izbranElement][2] == MIKROPROCESOR) for (int i = seznamStikal.size() - 1; i >= 0; i--) if (seznamStikal[i][seznamStikal[i].size() - 1] == izbranElement) seznamStikal.erase(seznamStikal.begin() + i);


		seznamElementov.erase(seznamElementov.begin() + izbranElement);
		seznamLastnosti.erase(seznamLastnosti.begin() + izbranElement);
		seznamResitevReset.erase(seznamResitevReset.begin() + izbranElement);
		seznamResitev = seznamResitevReset;
		seznamGrafTock.clear();

		izbranElement = -1;

		shranjeno = false;
	}
	else if (drzanje && mousePos.x > 200) {

		seznamElementov.push_back({ mousePos.x / 10 * 10,mousePos.y / 10 * 10,choiceDod->GetSelection() });

		if (choiceDod->GetSelection() == MIKROPROCESOR) {
			seznamLastnosti.push_back({});
			seznamResitevReset.push_back({ 0,0,0,0,0,0,0,0 });
		}
		else if (choiceDod->GetSelection() == ELEKTRICNACRPALKA) {
			seznamLastnosti.push_back({ .95,6,0.2,0.01,0.05,1 });
			seznamResitevReset.push_back({ 0,0,2000,20 });
		}
		else if (choiceDod->GetSelection() == TLACNAPOSODA) {
			seznamLastnosti.push_back({ 2,700000 });
			seznamResitevReset.push_back({ 1,-1,600000,-1 });
			seznamPovezav.push_back({ static_cast<int>(seznamElementov.size()) - 1,3,-1,-1,2,-1 });
		}
		else if (choiceDod->GetSelection() == PRIJEMALO) {
			seznamLastnosti.push_back({ 700000,700000,0.1,0.025,0.1,0 });
			seznamResitevReset.push_back({ 1,-1,pogojiOkolja.tlakOzracja,-1,-1,pogojiOkolja.tlakOzracja,-1,0,0,0,0 });
			seznamPovezav.push_back({ static_cast<int>(seznamElementov.size()) - 1,3,-1,-1,2,-1 });
		}
		else if (choiceDod->GetSelection() == PRISESEK) {
			seznamLastnosti.push_back({ 0.1,0.2 });
			seznamResitevReset.push_back({ 1,-1,pogojiOkolja.tlakOzracja,-1, 6, -1, 1 });
			seznamPovezav.push_back({ static_cast<int>(seznamElementov.size()) - 1,3,-1,-1,2,-1 });
		}
		else if (choiceDod->GetSelection() == GRAF) {
			seznamLastnosti.push_back({ 200,100,-1,-1 });
			seznamResitevReset.push_back({});
		}
		else {
			seznamLastnosti.push_back({});
			seznamResitevReset.push_back({});
		}

		seznamResitevReset = IzracunVolumna(seznamElementov, seznamResitevReset, seznamLastnosti);
		seznamResitevReset = IzracunMase(pogojiOkolja, seznamElementov, seznamResitevReset);
		seznamResitev = seznamResitevReset;

		shranjeno = false;
	}
	else if (izbranElement >= 0 && casDrzanja > 2 && mousePos.x > 200 && drzanjeElementa) {

		seznamElementov[izbranElement][0] = mousePos.x / 10 * 10;
		seznamElementov[izbranElement][1] = mousePos.y / 10 * 10;

		izbranElement = -1;

		shranjeno = false;
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

		if (seznamElementov[izbranElement][2] == MIKROPROCESOR) {
			NastavitevMikroProcesorja* procNast = new NastavitevMikroProcesorja();
			procNast->Show();
		}
		else if (seznamElementov[izbranElement][2] == ELEKTRICNACRPALKA) {
			NastavitevCrpalke* crpNast = new NastavitevCrpalke();
			crpNast->Show();
		}
		else if (seznamElementov[izbranElement][2] == TLACNAPOSODA) {
			NastavitevTlacnePosode* posNast = new NastavitevTlacnePosode();
			posNast->Show();
		}
		else if (seznamElementov[izbranElement][2] == PRIJEMALO) {
			NastavitevPrijemalke* prijNast = new NastavitevPrijemalke();
			prijNast->Show();
		}
		else if (seznamElementov[izbranElement][2] == PRISESEK) {
			NastavitevPriseska* prisNast = new NastavitevPriseska();
			prisNast->Show();
		}
		else if (seznamElementov[izbranElement][2] == GRAF) {
			NastavitevGrafa* grafNast = new NastavitevGrafa();
			grafNast->Show();
		}
	}

	Refresh();
}

void OknoSim::OnButtonIzbClicked(wxCommandEvent& evt) {

	if (izbranElement >= 0) {

		simbool = false;
		casSimulacije->SetValue(0);

		for (int i = seznamPovezav.size() - 1; i >= 0; i--) if (seznamPovezav[i][0] == izbranElement || seznamPovezav[i][2] == izbranElement) seznamPovezav.erase(seznamPovezav.begin() + i); // brisanje cevi
		for (int i = seznamStikal.size() - 1; i >= 0; i--) if (seznamStikal[i][0] == izbranElement || seznamStikal[i][4] == izbranElement) seznamStikal.erase(seznamStikal.begin() + i); // brisanje stikal

		for (int i = 0; i < seznamPovezav.size(); i++) {
			if (seznamPovezav[i][0] > izbranElement) seznamPovezav[i][0]--;
			if (seznamPovezav[i][2] > izbranElement) seznamPovezav[i][2]--;
		}

		for (int i = 0; i < seznamStikal.size(); i++) {
			if (seznamStikal[i][0] > izbranElement) seznamStikal[i][0]--;
			if (seznamStikal[i][4] > izbranElement) seznamStikal[i][4]--;
			if (seznamStikal[i][7] > izbranElement) seznamStikal[i][7]--;
		}

		if (seznamElementov[izbranElement][2] == MIKROPROCESOR) for (int i = seznamStikal.size() - 1; i >= 0; i--) if (seznamStikal[i][seznamStikal[i].size() - 1] == izbranElement) seznamStikal.erase(seznamStikal.begin() + i);


		seznamElementov.erase(seznamElementov.begin() + izbranElement);
		seznamLastnosti.erase(seznamLastnosti.begin() + izbranElement);
		seznamResitevReset.erase(seznamResitevReset.begin() + izbranElement);
		seznamResitev = seznamResitevReset;
		seznamGrafTock.clear();

		izbranElement = -1;

		shranjeno = false;

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
	seznamGrafTock.clear();

	casSimulacije->SetValue(0);
	izbranElement = -1;

	shranjeno = false;

	Refresh();
}

void OknoSim::OnRisanjePovezavClicked(wxCommandEvent& evt) {
	
	simbool = false;
	casSimulacije->SetValue(0);
	seznamResitev = seznamResitevReset;

	izbranElement = -1;

	if (drzanjePovezav == 0) {

		seznamPovezav.push_back({ -1,-1,-1,-1,-1,-1 });

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

		spinPovezava->SetValue(0);
		izbranaPovezava = spinPovezava->GetValue();

		shranjeno = false;
	}

	Refresh();
}

void OknoSim::OnSpinPovezavaChanged(wxCommandEvent& evt) {

	while (seznamPovezav[spinPovezava->GetValue()][4] == 2) {

		spinPovezava->SetValue(spinPovezava->GetValue() + 1);

		if (spinPovezava->GetValue() >= seznamPovezav.size()) spinPovezava->SetValue(0);
	}

	izbranaPovezava = spinPovezava->GetValue();


	Refresh();
}

void OknoSim::OnNastavitevPovezavClicked(wxCommandEvent& evt) {

	if (izbranaPovezava > 0 && seznamPovezav[izbranaPovezava - 1][4] == 1) {

		NastavitevPovezav* povNast = new NastavitevPovezav();
		povNast->Show();
	}

	else wxLogStatus("Izberite pnevmaticno cev");
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


			shranjeno = true;

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
					for (int j = 0; j < pon; j++) { double a; nalozi >> a; seznamStikal[i].push_back(a); }
				}
			}

			seznamResitev = seznamResitevReset;
		}
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
		seznamGrafTock.clear();
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
}

void OknoSim::OnResetSimClicked(wxCommandEvent& evt) {

	simbool = false;
	casSimulacije->SetValue(0);
	seznamResitev = seznamResitevReset;
	seznamGrafTock.clear();

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
		if (seznamElementov[izbranElement][2] == MIKROPROCESOR) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 10; oznacenElementSize.x = 120; oznacenElementSize.y = 160; }
		else if (seznamElementov[izbranElement][2] == ELEKTRICNACRPALKA) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 44; oznacenElementSize.x = 60; oznacenElementSize.y = 54; }
		else if (seznamElementov[izbranElement][2] == TLACNAPOSODA) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 10; oznacenElementSize.x = 140; oznacenElementSize.y = 60; }
		else if (seznamElementov[izbranElement][2] == PRIJEMALO) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 10; oznacenElementSize.x = 110; oznacenElementSize.y = 70; }
		else if (seznamElementov[izbranElement][2] == PRISESEK) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 30; oznacenElementSize.x = 100; oznacenElementSize.y = 40; }
		else if (seznamElementov[izbranElement][2] == GRAF) { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 10; oznacenElementSize.x = seznamLastnosti[izbranElement][0] + 20; oznacenElementSize.y = seznamLastnosti[izbranElement][1] + 20; }
		else { oznacenElementPoint.x -= 10; oznacenElementPoint.y -= 10; oznacenElementSize.x = 100; oznacenElementSize.y = 70; }

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
	else if (choiceDod->GetSelection() == MIKROPROCESOR) { predogled.x = sirinaOrodja / 2 - 50; }
	else if (choiceDod->GetSelection() == ELEKTRICNACRPALKA) { predogled.x = sirinaOrodja / 2 - 20; predogled.y += 34; }
	else if (choiceDod->GetSelection() == TLACNAPOSODA) { predogled.x = sirinaOrodja / 2 - 60; }
	else if (choiceDod->GetSelection() == PRIJEMALO) { predogled.x = sirinaOrodja / 2 - 45; }
	else if (choiceDod->GetSelection() == PRISESEK) { predogled.x = sirinaOrodja / 2 - 40; predogled.y += 20; }
	else if (choiceDod->GetSelection() == GRAF) { predogled.x = sirinaOrodja / 2 - 80; predogled.y += 20; }
	else { predogled.x = sirinaOrodja / 2 - 40; predogled.y += 20; }


	if (choiceDod->GetSelection() == MIKROPROCESOR) {
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
	else if (choiceDod->GetSelection() == ELEKTRICNACRPALKA) {
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
	else if (choiceDod->GetSelection() == TLACNAPOSODA) {
		dc.DrawRoundedRectangle(wxPoint(predogled.x, predogled.y), wxSize(120, 40), 20);
		dc.DrawLine(wxPoint(predogled.x + 30, predogled.y - 10), wxPoint(predogled.x + 30, predogled.y));

		dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
		dc.DrawLine(wxPoint(predogled.x - 10, predogled.y + 20), wxPoint(predogled.x, predogled.y + 20));
		dc.DrawLine(wxPoint(predogled.x + 120, predogled.y + 20), wxPoint(predogled.x + 130, predogled.y + 20));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}
	else if (choiceDod->GetSelection() == PRIJEMALO) {
		dc.DrawRectangle(wxPoint(predogled.x, predogled.y), wxSize(65, 50));
		dc.DrawRectangle(wxPoint(predogled.x + 40, predogled.y), wxSize(50, 10));
		dc.DrawRectangle(wxPoint(predogled.x + 40, predogled.y + 40), wxSize(50, 10));

		dc.DrawLine(wxPoint(predogled.x - 10, predogled.y + 10), wxPoint(predogled.x, predogled.y + 10));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
		dc.DrawLine(wxPoint(predogled.x - 10, predogled.y + 40), wxPoint(predogled.x, predogled.y + 40));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}
	else if (choiceDod->GetSelection() == PRISESEK) {
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
	else if (choiceDod->GetSelection() == GRAF) {
		dc.DrawText("Graf", wxPoint(predogled.x + 70, predogled.y - 16));
		dc.SetPen(wxPen(wxColour(204, 204, 204), 1, wxPENSTYLE_SOLID));
		dc.DrawRectangle(wxPoint(predogled.x, predogled.y), wxSize(160, 80));
		dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
	}
	else {
		dc.DrawRectangle(wxPoint(predogled.x, predogled.y), wxSize(80, 50));
		dc.DrawText("Neznan \nelement", wxPoint(predogled.x + 18, predogled.y + 10));
	}

	
	//- IZRIS POVEZAV
	for (int i = 0; i < seznamPovezav.size(); i++) {

		if (seznamPovezav[i][4] != 2) {
			wxPoint tocka1(0, 0);
			if (drzanjePovezav != 1 || i < seznamPovezav.size() - 1) {
				tocka1.x = seznamElementov[seznamPovezav[i][0]][0];
				tocka1.y = seznamElementov[seznamPovezav[i][0]][1];

				switch (seznamElementov[seznamPovezav[i][0]][2]) {

				case MIKROPROCESOR:
					tocka1.x += 110;
					tocka1.y += 10 + seznamPovezav[i][1] * 15;

					break;

				case ELEKTRICNACRPALKA:
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

				case TLACNAPOSODA:

					if (seznamPovezav[i][1] == 0) {
						tocka1.x += 30;
						tocka1.y -= 10;
					}
					else if (seznamPovezav[i][1] == 1) {
						tocka1.x -= 10 + 46;
						tocka1.y += 20;
					}
					else if (seznamPovezav[i][1] == 2) {
						tocka1.x += 130 + 46;
						tocka1.y += 20;
					}

					break;

				case PRIJEMALO:

					if (seznamPovezav[i][1] == 0) {
						tocka1.x -= 10;
						tocka1.y += 0;
					}
					else if (seznamPovezav[i][1] == 1) {
						tocka1.x -= 10 + 46;
						tocka1.y += 40 - 8;
					}

					break;

				case PRISESEK:

					if (seznamPovezav[i][1] == 0) {
						tocka1.x += 0;//50;
						tocka1.y -= 15;// 35;
					}
					else if (seznamPovezav[i][1] == 1) {
						tocka1.x += 40;
						tocka1.y -= 25 + 46;
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

				case MIKROPROCESOR:
					tocka2.x += 110;
					tocka2.y += 10 + seznamPovezav[i][3] * 15;

					break;

				case ELEKTRICNACRPALKA:
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

				case TLACNAPOSODA:

					if (seznamPovezav[i][3] == 0) {
						tocka2.x += 30;
						tocka2.y -= 10;
					}
					else if (seznamPovezav[i][3] == 1) {
						tocka2.x -= 10 + 46;
						tocka2.y += 20;
					}
					else if (seznamPovezav[i][3] == 2) {
						tocka2.x += 130 + 46;
						tocka2.y += 20;
					}

					break;

				case PRIJEMALO:

					if (seznamPovezav[i][3] == 0) {
						tocka2.x -= 10;
						tocka2.y += 0;
					}
					else if (seznamPovezav[i][3] == 1) {
						tocka2.x -= 10 + 46;
						tocka2.y += 40 - 8;
					}

					break;

				case PRISESEK:

					if (seznamPovezav[i][3] == 0) {
						tocka2.x += 0;//50;
						tocka2.y -= 15;//35;
					}
					else if (seznamPovezav[i][3] == 1) {
						tocka2.x += 40;
						tocka2.y -= 25 + 46;
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

			if (!(seznamPovezav[i][5] == -1)) {
				dc.DrawCircle((tocka1 + tocka2) / 2, 7);
				dc.DrawText(wxString::Format("%g bar", static_cast<double>(seznamPovezav[i][5])/100000), (tocka1 + tocka2) / 2 - wxPoint(-10, 7));
			}

			dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
		}
	}
	

	
	//- IZRIS ELEMENTOV
	if (simbool)  {
		for (int i = 0; i < 1; i++) seznamResitev = IzracunPovezav(pogojiOkolja, seznamElementov, seznamLastnosti, seznamResitev, seznamPovezav, seznamStikal, korak, casSimulacije->GetValue());
		ZapisMeritev(&seznamGrafTock, &seznamElementov, &seznamLastnosti, &seznamResitev, korak);
	}

	for (int i = 0; i < seznamElementov.size(); i++) {
		std::vector<int> xy = seznamElementov[i];
		int zamik = 0;

		switch (xy[2]) {

		case MIKROPROCESOR: //- Mikrokrmilnik

			dc.DrawRectangle(wxPoint(xy[0], xy[1]), wxSize(100, 140));
			dc.DrawText("0", wxPoint(xy[0] + 88, xy[1] + 5));
			dc.DrawLine(wxPoint(xy[0] + 110, xy[1] + 10), wxPoint(xy[0] + 100, xy[1] + 10));
			dc.DrawText("1", wxPoint(xy[0] + 88, xy[1] + 20));
			dc.DrawLine(wxPoint(xy[0] + 110, xy[1] + 25), wxPoint(xy[0] + 100, xy[1] + 25));
			dc.DrawText("2", wxPoint(xy[0] + 88, xy[1] + 35));
			dc.DrawLine(wxPoint(xy[0] + 110, xy[1] + 40), wxPoint(xy[0] + 100, xy[1] + 40));
			dc.DrawText("3", wxPoint(xy[0] + 88, xy[1] + 50));
			dc.DrawLine(wxPoint(xy[0] + 110, xy[1] + 55), wxPoint(xy[0] + 100, xy[1] + 55));
			dc.DrawText("4", wxPoint(xy[0] + 88, xy[1] + 65));
			dc.DrawLine(wxPoint(xy[0] + 110, xy[1] + 70), wxPoint(xy[0] + 100, xy[1] + 70));
			dc.DrawText("5", wxPoint(xy[0] + 88, xy[1] + 80));
			dc.DrawLine(wxPoint(xy[0] + 110, xy[1] + 85), wxPoint(xy[0] + 100, xy[1] + 85));
			dc.DrawText("6", wxPoint(xy[0] + 88, xy[1] + 95));
			dc.DrawLine(wxPoint(xy[0] + 110, xy[1] + 100), wxPoint(xy[0] + 100, xy[1] + 100));
			dc.DrawText("7", wxPoint(xy[0] + 88, xy[1] + 110));
			dc.DrawLine(wxPoint(xy[0] + 110, xy[1] + 115), wxPoint(xy[0] + 100, xy[1] + 115));

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

		case ELEKTRICNACRPALKA: //- Kompresor

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
				dc.DrawText(wxString::Format("vrt = %g s-1", seznamResitev[i][3]), wxPoint(xy[0], xy[1] + 60));

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

		case TLACNAPOSODA: //- Tlacna posoda

			dc.DrawRoundedRectangle(wxPoint(xy[0], xy[1]), wxSize(120, 40), 20);
			dc.DrawLine(wxPoint(xy[0] + 30, xy[1] - 10), wxPoint(xy[0] + 30, xy[1]));

			dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));

			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] + 20), wxPoint(xy[0], xy[1] + 20));
			dc.DrawLine(wxPoint(xy[0] + 130, xy[1] + 20), wxPoint(xy[0] + 120, xy[1] + 20));

			dc.DrawLine(wxPoint(xy[0] + 166, xy[1] + 20), wxPoint(xy[0] + 176, xy[1] + 20));
			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] + 20), wxPoint(xy[0] - 56, xy[1] + 20));

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

				dc.DrawRectangle(wxPoint(xy[0] - 15 - 46, xy[1] + 15), wxSize(10, 10));
				dc.DrawRectangle(wxPoint(xy[0] + 125 + 46, xy[1] + 15), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
			}
			
			// Risanje ventilov
			for (int j = 0; j < 2; j++) {
				dc.DrawRectangle(wxPoint(xy[0] + 130 - j * 176, xy[1] - 28 + 36 * seznamResitev[i][0]), wxSize(36 + 1, 36 + 1));
				dc.DrawLine(wxPoint(xy[0] + 130 - j * 176, xy[1] - 28 + (36 / 3 * 1) + 36 * seznamResitev[i][0]), wxPoint(xy[0] + 166 - j * 176, xy[1] - 28 + (36 / 3 * 1) + 36 * seznamResitev[i][0]));
				dc.DrawLine(wxPoint(xy[0] + 166 - j * 176, xy[1] - 28 + (36 / 3 * 1) + 36 * seznamResitev[i][0]), wxPoint(xy[0] + 166 - 5 - j * 176, xy[1] - 28 + (36 / 3 * 1) + 5 + 36 * seznamResitev[i][0]));
				dc.DrawLine(wxPoint(xy[0] + 166 - j * 176, xy[1] - 28 + (36 / 3 * 1) + 36 * seznamResitev[i][0]), wxPoint(xy[0] + 166 - 5 - j * 176, xy[1] - 28 + (36 / 3 * 1) - 5 + 36 * seznamResitev[i][0]));
				dc.DrawLine(wxPoint(xy[0] + 130 - j * 176, xy[1] - 28 + (36 / 3 * 2) + 36 * seznamResitev[i][0]), wxPoint(xy[0] + 140 - j * 176, xy[1] - 28 + (36 / 3 * 2) + 36 * seznamResitev[i][0]));
				dc.DrawLine(wxPoint(xy[0] + 140 - j * 176, xy[1] - 28 + (36 / 3 * 2) - 5 + 36 * seznamResitev[i][0]), wxPoint(xy[0] + 140 - j * 176, xy[1] - 28 + (36 / 3 * 2) + 5 + 36 * seznamResitev[i][0]));
				dc.DrawRectangle(wxPoint(xy[0] + 130 - j * 176, xy[1] + 8 + 36 * seznamResitev[i][0]), wxSize(36 + 1, 36 + 1));
				dc.DrawLine(wxPoint(xy[0] + 130 - j * 176, xy[1] + 8 + (36 / 3 * 2) + 36 * seznamResitev[i][0]), wxPoint(xy[0] + 166 - j * 176, xy[1] + 8 + (36 / 3 * 1) + 36 * seznamResitev[i][0]));
				dc.DrawLine(wxPoint(xy[0] + 130 - j * 176, xy[1] + 8 + (36 / 3 * 2) + 36 * seznamResitev[i][0]), wxPoint(xy[0] + 130 + 5 - j * 176, xy[1] + 8 + (36 / 3 * 2) + 5 + 36 * seznamResitev[i][0]));
				dc.DrawLine(wxPoint(xy[0] + 130 - j * 176, xy[1] + 8 + (36 / 3 * 2) + 36 * seznamResitev[i][0]), wxPoint(xy[0] + 130 + 5 - j * 176, xy[1] + 8 + (36 / 3 * 2) - 5 + 36 * seznamResitev[i][0]));
				dc.DrawLine(wxPoint(xy[0] + 130 - j * 176, xy[1] + 8 + (36 / 3 * 1) + 36 * seznamResitev[i][0]), wxPoint(xy[0] + 140 - j * 176, xy[1] + 8 + (36 / 3 * 1) + 36 * seznamResitev[i][0]));
				dc.DrawLine(wxPoint(xy[0] + 140 - j * 176, xy[1] + 8 + (36 / 3 * 1) - 5 + 36 * seznamResitev[i][0]), wxPoint(xy[0] + 140 - j * 176, xy[1] + 8 + (36 / 3 * 1) + 5 + 36 * seznamResitev[i][0]));
			}

			
			//dc.DrawRectangle(wxPoint(xy[0] - 46, xy[1] - 30 + 36 * seznamResitev[i][0]), wxSize(36 + 1, 36 + 1));
			//dc.DrawRectangle(wxPoint(xy[0] - 46, xy[1] + 6 + 36 * seznamResitev[i][0]), wxSize(36 + 1, 36 + 1));


			break;

		case PRIJEMALO: //- Prijemalka

			zamik = seznamResitev[i][8] / seznamLastnosti[i][4] * 10;
			
			dc.DrawRectangle(wxPoint(xy[0], xy[1]), wxSize(65, 50));
			dc.DrawRectangle(wxPoint(xy[0] + 40, xy[1] + zamik), wxSize(50, 10));
			dc.DrawRectangle(wxPoint(xy[0] + 40, xy[1] + 40 - zamik), wxSize(50, 10));

			dc.DrawLine(wxPoint(xy[0] - 0, xy[1]), wxPoint(xy[0] - 10, xy[1]));

			dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));

			dc.DrawLine(wxPoint(xy[0] - 0, xy[1] + 40), wxPoint(xy[0] - 10, xy[1] + 40));
			dc.DrawLine(wxPoint(xy[0] - 0, xy[1] + 24), wxPoint(xy[0] - 10, xy[1] + 24));

			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] + 32), wxPoint(xy[0] - 56, xy[1] + 32));

			dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0], xy[1] - 16));
			dc.DrawText(wxString::Format("p0 = %g bar", seznamResitev[i][2] / 100000), wxPoint(xy[0], xy[1] + 50));
			dc.DrawText(wxString::Format("p1 = %g bar", seznamResitev[i][5] / 100000), wxPoint(xy[0], xy[1] + 65));

			if (drzanjePovezav > 0) { // Risanje povezav
				dc.SetPen(wxPen(wxColour(51, 51, 153), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 153, 255), wxBRUSHSTYLE_SOLID));

				dc.DrawRectangle(wxPoint(xy[0] - 15, xy[1] - 5), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));

				dc.DrawRectangle(wxPoint(xy[0] - 15 - 46, xy[1] + 27), wxSize(10, 10));

				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));
				dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));
			}

			if (seznamResitev[i][9] > 0) dc.SetBrush(wxBrush(wxColour(51, 255, 51), wxBRUSHSTYLE_SOLID));
			else if (seznamResitev[i][9] < 0) dc.SetBrush(wxBrush(wxColour(255, 51, 51), wxBRUSHSTYLE_SOLID));
			else dc.SetBrush(wxBrush(wxColour(51, 51, 255), wxBRUSHSTYLE_SOLID));

			dc.DrawCircle(wxPoint(xy[0] + 10, xy[1] + 10), 5);

			dc.SetBrush(wxBrush(wxColour(255, 255, 255), wxBRUSHSTYLE_SOLID));

			// Risanje ventilov
			dc.DrawRectangle(wxPoint(xy[0] - 46, xy[1] - 22 + 36 * seznamResitev[i][7]), wxSize(36 + 1, 36 + 1));
			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] - 22 + (36 / 4 * 2) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 10, xy[1] - 22 + (36 / 4 * 1) + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] - 22 + (36 / 4 * 1) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 10 - 5, xy[1] - 22 + (36 / 4 * 1) + 5 + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] - 22 + (36 / 4 * 1) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 10 - 5, xy[1] - 22 + (36 / 4 * 1) - 5 + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] - 22 + (36 / 4 * 3) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 10, xy[1] - 22 + (36 / 4 * 3) + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] - 22 + (36 / 4 * 3) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 46 + 5, xy[1] - 22 + (36 / 4 * 3) + 5 + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] - 22 + (36 / 4 * 3) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 46 + 5, xy[1] - 22 + (36 / 4 * 3) - 5 + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] - 22 + (36 / 4 * 1) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 36, xy[1] - 22 + (36 / 4 * 1) + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 36, xy[1] - 22 + (36 / 4 * 1) - 5 + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 36, xy[1] - 22 + (36 / 4 * 1) + 5 + 36 * seznamResitev[i][7]));
			dc.DrawRectangle(wxPoint(xy[0] - 46, xy[1] + 14 + 36 * seznamResitev[i][7]), wxSize(36 + 1, 36 + 1));
			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] + 14 + (36 / 4 * 2) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 10, xy[1] + 14 + (36 / 4 * 3) + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] + 14 + (36 / 4 * 3) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 10 - 5, xy[1] + 14 + (36 / 4 * 3) + 5 + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 10, xy[1] + 14 + (36 / 4 * 3) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 10 - 5, xy[1] + 14 + (36 / 4 * 3) - 5 + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] + 14 + (36 / 4 * 1) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 10, xy[1] + 14 + (36 / 4 * 1) + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] + 14 + (36 / 4 * 1) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 46 + 5, xy[1] + 14 + (36 / 4 * 1) + 5 + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] + 14 + (36 / 4 * 1) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 46 + 5, xy[1] + 14 + (36 / 4 * 1) - 5 + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 46, xy[1] + 14 + (36 / 4 * 3) + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 36, xy[1] + 14 + (36 / 4 * 3) + 36 * seznamResitev[i][7]));
			dc.DrawLine(wxPoint(xy[0] - 36, xy[1] + 14 + (36 / 4 * 3) - 5 + 36 * seznamResitev[i][7]), wxPoint(xy[0] - 36, xy[1] + 14 + (36 / 4 * 3) + 5 + 36 * seznamResitev[i][7]));


			break;

		case PRISESEK: //- Prisesek

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

				dc.DrawLine(wxPoint(xy[0], xy[1] + zamik), wxPoint(xy[0], xy[1] - 15));

				dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));

				dc.DrawLine(wxPoint(xy[0] + 40, xy[1] - 25), wxPoint(xy[0] + 40, xy[1] - 15 + zamik));

				dc.DrawLine(wxPoint(xy[0] + 40, xy[1] - 25 - 46), wxPoint(xy[0] + 40, xy[1] - 15 - 46 + zamik));

				dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

				dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0] + 92, xy[1] - 24));
				dc.DrawText(wxString::Format("p = %g bar", seznamResitev[i][2] / 100000), wxPoint(xy[0] + 92, xy[1] - 8));

				if (drzanjePovezav > 0) { // Risanje povezav
					dc.SetPen(wxPen(wxColour(51, 51, 153), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 153, 255), wxBRUSHSTYLE_SOLID));

					dc.DrawRectangle(wxPoint(xy[0] - 5, xy[1] - 20), wxSize(10, 10));

					dc.SetPen(wxPen(wxColour(51, 153, 51), 1, wxPENSTYLE_SOLID));
					dc.SetBrush(wxBrush(wxColour(153, 255, 153), wxBRUSHSTYLE_SOLID));

					dc.DrawRectangle(wxPoint(xy[0] + 35, xy[1] - 30 - 46), wxSize(10, 10));

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

				dc.DrawRectangle(wxPoint(xy[0] - 8 + 36 * seznamResitev[i][0], xy[1] - 61), wxSize(36 + 1, 36 + 1));
				dc.DrawLine(wxPoint(xy[0] - 8 + (36 / 3 * 1) + 36 * seznamResitev[i][0], xy[1] - 61), wxPoint(xy[0] - 8 + (36 / 3 * 1) + 36 * seznamResitev[i][0], xy[1] - 25));
				dc.DrawLine(wxPoint(xy[0] - 8 + (36 / 3 * 1) + 36 * seznamResitev[i][0], xy[1] - 61), wxPoint(xy[0] - 8 - 5 + (36 / 3 * 1) + 36 * seznamResitev[i][0], xy[1] - 61 + 5));
				dc.DrawLine(wxPoint(xy[0] - 8 + (36 / 3 * 1) + 36 * seznamResitev[i][0], xy[1] - 61), wxPoint(xy[0] - 8 + 5 + (36 / 3 * 1) + 36 * seznamResitev[i][0], xy[1] - 61 + 5));
				dc.DrawLine(wxPoint(xy[0] - 8 + (36 / 3 * 2) + 36 * seznamResitev[i][0], xy[1] - 25 - 10), wxPoint(xy[0] - 8 + (36 / 3 * 2) + 36 * seznamResitev[i][0], xy[1] - 25));
				dc.DrawLine(wxPoint(xy[0] - 8 + (36 / 3 * 2) - 5 + 36 * seznamResitev[i][0], xy[1] - 25 - 10), wxPoint(xy[0] - 8 + (36 / 3 * 2) + 5 + 36 * seznamResitev[i][0], xy[1] - 25 - 10));
				dc.DrawRectangle(wxPoint(xy[0] + 24 + 36 * seznamResitev[i][0], xy[1] - 61), wxSize(36 + 1, 36 + 1));
				dc.DrawLine(wxPoint(xy[0] + 24 + (36 / 3 * 1) + 36 * seznamResitev[i][0], xy[1] - 61), wxPoint(xy[0] + 24 + (36 / 3 * 2) + 36 * seznamResitev[i][0], xy[1] - 25));
				dc.DrawLine(wxPoint(xy[0] + 24 + (36 / 3 * 2) + 36 * seznamResitev[i][0], xy[1] - 25), wxPoint(xy[0] + 24 - 5 + (36 / 3 * 2) + 36 * seznamResitev[i][0], xy[1] - 25 - 5));
				dc.DrawLine(wxPoint(xy[0] + 24 + (36 / 3 * 2) + 36 * seznamResitev[i][0], xy[1] - 25), wxPoint(xy[0] + 24 + 5 + (36 / 3 * 2) + 36 * seznamResitev[i][0], xy[1] - 25 - 5));
				dc.DrawLine(wxPoint(xy[0] + 24 + (36 / 3 * 1) + 36 * seznamResitev[i][0], xy[1] - 25 - 10), wxPoint(xy[0] + 24 + (36 / 3 * 1) + 36 * seznamResitev[i][0], xy[1] - 25));
				dc.DrawLine(wxPoint(xy[0] + 24 + (36 / 3 * 1) - 5 + 36 * seznamResitev[i][0], xy[1] - 25 - 10), wxPoint(xy[0] + 24 + (36 / 3 * 1) + 5 + 36 * seznamResitev[i][0], xy[1] - 25 - 10));

			}

			break;

		case GRAF: //- Graf

			dc.DrawText("Graf", wxPoint(xy[0] + seznamLastnosti[i][0] / 2 - 12, xy[1] - 16));
			dc.SetPen(wxPen(wxColour(204, 204, 204), 1, wxPENSTYLE_SOLID));
			dc.DrawRectangle(wxPoint(xy[0], xy[1]), wxSize(seznamLastnosti[i][0] + 1, seznamLastnosti[i][1] + 1));
			dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));


			if (!seznamGrafTock.empty() && seznamLastnosti[i][2] != -1 && seznamLastnosti[i][3] != -1) {
			
				double max = *std::max_element(seznamGrafTock[i].begin(), seznamGrafTock[i].end());
				double min = *std::min_element(seznamGrafTock[i].begin(), seznamGrafTock[i].end());

				int popacenje = 1;
				int dolzinaKrivilje = seznamGrafTock[seznamGrafTock.size() - 1].size();
				int omejitevGrafa = seznamLastnosti[i][0];

				while (dolzinaKrivilje > omejitevGrafa) {
					popacenje++;
					dolzinaKrivilje -= omejitevGrafa;
				}
				

				wxPointList* seznamTock = new wxPointList();

				for (int j = 0; j < seznamGrafTock[i].size() / popacenje; j++) {

					wxPoint* tocka;
					tocka = new wxPoint(xy[0] + seznamGrafTock[seznamGrafTock.size() - 1][j * popacenje] / korak / popacenje, xy[1] + seznamLastnosti[i][1] - seznamLastnosti[i][1] * ((seznamGrafTock[i][j * popacenje] - min) / (max - min)));
					seznamTock->Append(tocka);
				}

				dc.DrawLines(seznamTock);

				double pretvornik = 1; ////////////////// popravit
				if (seznamLastnosti[i][3] == 8) {
					max = max / seznamLastnosti[seznamLastnosti[i][2]][4] * 100;
					min = min / seznamLastnosti[seznamLastnosti[i][2]][4] * 100;
				}
				else {
					max /= 100000;
					min /= 100000;
				}

				dc.DrawText(wxString::Format("%g", round(max * 100) / 100), wxPoint(xy[0] - 23, xy[1] - 7));
				dc.DrawText(wxString::Format("%g", round(min * 100) / 100), wxPoint(xy[0] - 23, xy[1] + seznamLastnosti[i][1] - 7));
				dc.DrawText(wxString::Format("%g", seznamGrafTock[seznamGrafTock.size() - 1][0]), wxPoint(xy[0], xy[1] + seznamLastnosti[i][1] + 8));
				dc.DrawText(wxString::Format("%g", seznamGrafTock[seznamGrafTock.size() - 1][seznamGrafTock[seznamGrafTock.size() - 1].size() - 1]), wxPoint(xy[0] + seznamLastnosti[i][0], xy[1] + seznamLastnosti[i][1] + 8));
			}
			dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));

			break;


		default:

			dc.DrawRectangle(wxPoint(xy[0], xy[1]), wxSize(80, 50));
			dc.DrawText("Neznan \nelement", wxPoint(xy[0] + 18, xy[1] + 10));

			dc.DrawText(wxString::Format("Element %d", i + 1), wxPoint(xy[0], xy[1] - 16));

			break;
		}
	}
	
}





wxSpinCtrl* spinPremerPovezav;
wxCheckBox* checkRegulatorPovetav;
wxSpinCtrlDouble* spinRegulatorPovezav;

NastavitevPovezav::NastavitevPovezav() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitev cevi"), wxPoint(0, 0), wxSize(280, 240)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(5, 170), wxDefaultSize);
	wxButton* close = new wxButton(panel, wxID_ANY, "Close", wxPoint(90, 170), wxDefaultSize);

	spinPremerPovezav = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(150,5), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 5, 36, 12);

	checkRegulatorPovetav = new wxCheckBox(panel, wxID_ANY, "Regulator tlaka:", wxPoint(5, 53), wxDefaultSize);
	if (!(seznamPovezav[izbranaPovezava - 1][5] == NI_REGULATORJA_TLAKA)) checkRegulatorPovetav->SetValue(true);
	spinRegulatorPovezav = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(112, 50), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 10, 0, .1);
	if (!(checkRegulatorPovetav->IsChecked())) spinRegulatorPovezav->Disable();
	else { 
		spinRegulatorPovezav->Enable(); 
		spinRegulatorPovezav->SetValue(static_cast<double>(seznamPovezav[izbranaPovezava - 1][5]) / 100000);
	}

	
	apply->Bind(wxEVT_BUTTON, &NastavitevPovezav::OnApplyClicked, this);
	close->Bind(wxEVT_BUTTON, &NastavitevPovezav::OnCloseClicked, this);
	checkRegulatorPovetav->Bind(wxEVT_CHECKBOX, &NastavitevPovezav::OnRefresh, this);
	///////////////////// Dodat 'Bind' za 'wxEVT_CLOSE_WINDOW'
	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(NastavitevPovezav::OnPaint));
}


void NastavitevPovezav::OnApplyClicked(wxCommandEvent& evt) {

	if (checkRegulatorPovetav->IsChecked()) seznamPovezav[izbranaPovezava - 1][5] = spinRegulatorPovezav->GetValue() * 100000;
	else seznamPovezav[izbranaPovezava - 1][5] = NI_REGULATORJA_TLAKA;

	seznamPovezav[izbranaPovezava - 1][6] = spinPremerPovezav->GetValue();

	shranjeno = false;
}

void NastavitevPovezav::OnCloseClicked(wxCommandEvent& evt) {

	Destroy(); /////////////// Ali 'Close()' - Posebi gumb za to
}

void NastavitevPovezav::OnRefresh(wxCommandEvent& evt) {

	if (checkRegulatorPovetav->IsChecked()) spinRegulatorPovezav->Enable();
	else spinRegulatorPovezav->Disable();
}


void NastavitevPovezav::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();

	dc.DrawText("Premer pnevmaticne cevi:", wxPoint(5, 8));
	dc.DrawText("mm", wxPoint(200, 8));

	dc.DrawText("bar", wxPoint(180, 53));
}



wxChoice* choicePinBranje;
wxChoice* choiceVelicinaBranje;
wxChoice* choiceLogFun;
wxSpinCtrlDouble* spinVrednostBranje;
wxChoice* choicePinPisanje;
wxChoice* choiceVelicinaPisanje;
wxChoice* choiceVrednostPisanje;

NastavitevMikroProcesorja::NastavitevMikroProcesorja() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve Mikro Procesorja"), wxPoint(0, 0), wxSize(555, 300)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(10,230), wxDefaultSize);
	wxButton* close = new wxButton(panel, wxID_ANY, "Close", wxPoint(90, 230), wxDefaultSize);


	wxArrayString ArPinBranje;
	wxArrayString ArPinPisanje;
	ArPinBranje.Add(" cas");
	for (int j = 0; j < seznamPovezav.size(); j++) if (seznamPovezav[j][4] == 0) {
		if (seznamPovezav[j][0] == izbranElement) {

			if (seznamElementov[seznamPovezav[j][2]][2] == TLACNAPOSODA || seznamElementov[seznamPovezav[j][2]][2] == PRIJEMALO || seznamElementov[seznamPovezav[j][2]][2] == PRISESEK)
				ArPinBranje.Add(wxString::Format("pin %d", seznamPovezav[j][1]));
			if (seznamElementov[seznamPovezav[j][2]][2] == ELEKTRICNACRPALKA || seznamElementov[seznamPovezav[j][2]][2] == PRIJEMALO || seznamElementov[seznamPovezav[j][2]][2] == PRISESEK)
				ArPinPisanje.Add(wxString::Format("pin %d", seznamPovezav[j][1]));
		}
		else if (seznamPovezav[j][2] == izbranElement) {

			if (seznamElementov[seznamPovezav[j][0]][2] == TLACNAPOSODA || seznamElementov[seznamPovezav[j][0]][2] == PRIJEMALO || seznamElementov[seznamPovezav[j][0]][2] == PRISESEK)
				ArPinBranje.Add(wxString::Format("pin %d", seznamPovezav[j][3]));
			if (seznamElementov[seznamPovezav[j][0]][2] == ELEKTRICNACRPALKA || seznamElementov[seznamPovezav[j][0]][2] == PRIJEMALO || seznamElementov[seznamPovezav[j][0]][2] == PRISESEK)
				ArPinPisanje.Add(wxString::Format("pin %d", seznamPovezav[j][3]));
		}
	}

	choicePinBranje = new wxChoice(panel, wxID_ANY, wxPoint(20, 5), wxSize(51, -1), ArPinBranje);

	wxArrayString ArVelicinaBranje;
	ArVelicinaBranje.Add(" ");
	ArVelicinaBranje.Add(" ");

	choiceVelicinaBranje = new wxChoice(panel, wxID_ANY, wxPoint(75, 5), wxSize(56, -1), ArVelicinaBranje);
	choiceVelicinaBranje->Disable();

	wxArrayString ArLogFun;
	ArLogFun.Add(" <");
	ArLogFun.Add(" =");
	ArLogFun.Add(" >");
	
	choiceLogFun = new wxChoice(panel, wxID_ANY, wxPoint(135, 5), wxSize(36, -1), ArLogFun);
	choiceLogFun->Disable();
	
	spinVrednostBranje = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(175, 5), wxSize(86, -1), wxSP_ARROW_KEYS | wxSP_WRAP, -10000, 10000, 0, .01);
	spinVrednostBranje->Disable();


	choicePinPisanje = new wxChoice(panel, wxID_ANY, wxPoint(320, 5), wxSize(51, -1), ArPinPisanje);
	choicePinPisanje->Disable();

	wxArrayString ArVelicinaPisanje;
	ArVelicinaPisanje.Add(" ");
	ArVelicinaPisanje.Add(" ");

	choiceVelicinaPisanje = new wxChoice(panel, wxID_ANY, wxPoint(375, 5), wxSize(96, -1), ArVelicinaPisanje);
	choiceVelicinaPisanje->Disable();

	wxArrayString ArVrednostiPisanje;
	ArVrednostiPisanje.Add(" ");
	ArVrednostiPisanje.Add(" ");
	ArVrednostiPisanje.Add(" ");

	choiceVrednostPisanje = new wxChoice(panel, wxID_ANY, wxPoint(475, 5), wxSize(56, -1), ArVrednostiPisanje);
	choiceVrednostPisanje->Disable();

	wxButton* dodaj = new wxButton(panel, wxID_ANY, "Dodaj ukaz", wxPoint(5, 35), wxDefaultSize);
	wxButton* izbrisi = new wxButton(panel, wxID_ANY, "Izbrisi vse", wxPoint(5, 60), wxDefaultSize);



	apply->Bind(wxEVT_BUTTON, &NastavitevMikroProcesorja::OnApplyClicked, this);
	close->Bind(wxEVT_BUTTON, &NastavitevMikroProcesorja::OnCloseClicked, this);
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

	shranjeno = false;
}

void NastavitevMikroProcesorja::OnCloseClicked(wxCommandEvent& evt) {

	Destroy();
}

void NastavitevMikroProcesorja::OnDodajClicked(wxCommandEvent& evt) {

	if (choiceVrednostPisanje->GetSelection() >= 0) {
		int sezSize = seznamStikal.size();
		seznamStikal.resize(sezSize + 1);
		seznamStikal[sezSize].resize(8);


		if (choicePinBranje->GetSelection() == 0) {
			seznamStikal[sezSize][0] = -1;
			//seznamStikal[sezSize][1] = 2;
		}
		else {
			int pin;
			std::string strPin = static_cast<std::string>(choicePinBranje->GetString(choicePinBranje->GetSelection()));
			pin = std::stoi(strPin.substr(strPin.find(" ") + 1));
			for (int i = 0; i < seznamPovezav.size(); i++) {
				if ((seznamPovezav[i][0] == izbranElement && seznamPovezav[i][1] == pin) || (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin)) {
					int zamik = 0;
					if (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin) zamik = -2;

					seznamStikal[sezSize][0] = seznamPovezav[i][2 + zamik];

					if (seznamElementov[seznamStikal[sezSize][0]][2] == TLACNAPOSODA) {
						if (choiceVelicinaBranje->GetString(choiceVelicinaBranje->GetSelection()) == " p") seznamStikal[sezSize][1] = TLAK0_LOG;
					}
					else if (seznamElementov[seznamStikal[sezSize][0]][2] == PRIJEMALO) {
						if (choiceVelicinaBranje->GetString(choiceVelicinaBranje->GetSelection()) == " p0") seznamStikal[sezSize][1] = TLAK0_LOG;
						else if (choiceVelicinaBranje->GetString(choiceVelicinaBranje->GetSelection()) == " p1") seznamStikal[sezSize][1] = TLAK1_LOG;
					}
					else if (seznamElementov[seznamStikal[sezSize][0]][2] == PRISESEK) {
						if (choiceVelicinaBranje->GetString(choiceVelicinaBranje->GetSelection()) == " p") seznamStikal[sezSize][1] = TLAK0_LOG;
					}
				}
			}
		}

		if (choiceLogFun->GetSelection() == 0) seznamStikal[sezSize][2] = MANJ;
		else if (choiceLogFun->GetSelection() == 1) seznamStikal[sezSize][2] = ENAKO;
		else if (choiceLogFun->GetSelection() == 2) seznamStikal[sezSize][2] = VECJE;

		if (choicePinBranje->GetSelection() == 0) seznamStikal[sezSize][3] = spinVrednostBranje->GetValue() * korak;
		else seznamStikal[sezSize][3] = spinVrednostBranje->GetValue() * 100000;

		int pin;
		std::string strPin = static_cast<std::string> (choicePinPisanje->GetString(choicePinPisanje->GetSelection()));
		pin = std::stoi(strPin.substr(strPin.find(" ") + 1));
		for (int i = 0; i < seznamPovezav.size(); i++) {
			if ((seznamPovezav[i][0] == izbranElement && seznamPovezav[i][1] == pin) || (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin)) {
				int zamik = 0;
				if (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin) zamik = -2;

				seznamStikal[sezSize][4] = seznamPovezav[i][2 + zamik];

				if (seznamElementov[seznamStikal[sezSize][4]][2] == ELEKTRICNACRPALKA) {
					if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " delovanje") seznamStikal[sezSize][5] = DELOVANJE_LOG;
					if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " moc [W]") seznamStikal[sezSize][5] = MOC_LOG;
					if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " vrtljaji [s-1]") seznamStikal[sezSize][5] = VRTLJAJI_LOG;
				}
				else if (seznamElementov[seznamStikal[sezSize][4]][2] == PRIJEMALO) {
					if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " delovanje") seznamStikal[sezSize][5] = DELOVANJE_LOG;
					else if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " pozicija") seznamStikal[sezSize][5] = POZ_PRIJEMALO_LOG;
				}
				else if (seznamElementov[seznamStikal[sezSize][4]][2] == PRISESEK) {
					if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " delovanje") seznamStikal[sezSize][5] = DELOVANJE_LOG;
					else if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " pozicija") seznamStikal[sezSize][5] = POZ_PRISESEK_LOG;
				}
			}
		}

		seznamStikal[sezSize][6] = std::stoi(static_cast<std::string> (choiceVrednostPisanje->GetString(choiceVrednostPisanje->GetSelection())));

		seznamStikal[sezSize][7] = izbranElement;

		shranjeno = false;

		Refresh();
	}

	else wxMessageBox("Dolocite vse vrednosti", "Opomba", wxICON_WARNING | wxOK);
}

void NastavitevMikroProcesorja::OnIzbrisiClicked(wxCommandEvent& evt) {

	for (int i = seznamStikal.size() - 1; i >= 0; i--) if (seznamStikal[i][seznamStikal[i].size() - 1] == izbranElement) seznamStikal.erase(seznamStikal.begin() + i);

	shranjeno = false;

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

		std::string strPin = static_cast<std::string> (choicePinPisanje->GetString(choicePinPisanje->GetSelection()));
		int pin = std::stoi(strPin.substr(strPin.find(" ") + 1));
		for (int i = 0; i < seznamPovezav.size(); i++) {
			if ((seznamPovezav[i][0] == izbranElement && seznamPovezav[i][1] == pin) || (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin)) {
				int zamik = 0;
				if (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin) zamik = -2;

				if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == ELEKTRICNACRPALKA) {
					choiceVelicinaPisanje->SetString(0, " delovanje");
					if (seznamLastnosti[seznamPovezav[i][2 + zamik]][5] == KONST_MOC) choiceVelicinaPisanje->SetString(1, " moc [W]");
					else if (seznamLastnosti[seznamPovezav[i][2 + zamik]][5] == KONST_VRTLJAJI) choiceVelicinaPisanje->SetString(1, " vrtljaji [s-1]");

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
						else if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " vrtljaji [s-1]") {
							choiceVrednostPisanje->SetString(0, " 10");
							choiceVrednostPisanje->SetString(1, " 15");
							choiceVrednostPisanje->SetString(2, " 20");
						}
					}
				}
				else if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == PRIJEMALO) {
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
				else if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == PRISESEK) {
					choiceVelicinaPisanje->SetString(0, " delovanje");
					choiceVelicinaPisanje->SetString(1, " pozicija");

					if (choiceVelicinaPisanje->GetSelection() >= 0) {
						if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " delovanje") {
							choiceVrednostPisanje->SetString(0, " 0");
							choiceVrednostPisanje->SetString(1, " 1");
						}
						else if (choiceVelicinaPisanje->GetString(choiceVelicinaPisanje->GetSelection()) == " pozicija") {
							choiceVrednostPisanje->SetString(0, " -1");
							choiceVrednostPisanje->SetString(1, " 1");
						}
					}
				}
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

		if (choicePinBranje->GetSelection() == 0) {
			choiceVelicinaBranje->SetString(0, " t");
		}

		else {
			std::string strPin = static_cast<std::string>(choicePinBranje->GetString(choicePinBranje->GetSelection()));
			int pin = std::stoi(strPin.substr(strPin.find(" ") + 1));
			for (int i = 0; i < seznamPovezav.size(); i++) {
				if ((seznamPovezav[i][0] == izbranElement && seznamPovezav[i][1] == pin) || (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin)) {
					int zamik = 0;
					if (seznamPovezav[i][2] == izbranElement && seznamPovezav[i][3] == pin) zamik = -2;

					if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == TLACNAPOSODA) {
						choiceVelicinaBranje->SetString(0, " p");
					}
					else if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == PRIJEMALO) {
						choiceVelicinaBranje->SetString(0, " p0");
						choiceVelicinaBranje->SetString(1, " p1");
					}
					else if (seznamElementov[seznamPovezav[i][2 + zamik]][2] == PRISESEK) {
						choiceVelicinaBranje->SetString(0, " p");
					}
				}
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
		dc.DrawText("Zapis vseh ukazov:", wxPoint(380, 80));
		for (int i = 0; i < seznamStikal.size(); i++) 
			if (seznamStikal[i][7] == izbranElement)
				dc.DrawText(wxString::Format("%g | %g | %g | %g | %g | %g | %g | %g", seznamStikal[i][0], seznamStikal[i][1], seznamStikal[i][2], seznamStikal[i][3], seznamStikal[i][4], seznamStikal[i][5], seznamStikal[i][6], seznamStikal[i][7]), wxPoint(380, 100 + 20 * i));
		//dc.DrawText(wxString::Format("choicePinBranje %d", choicePinBranje->GetSelection()), wxPoint(380, 40));
	}

	wxPoint predogled(130, 80); /////////////////////// Spremenit visino
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

	dc.DrawText("If", wxPoint(5, 8));
	dc.DrawText("-->", wxPoint(295, 8));

	wxString enota;
	if (choicePinBranje->GetSelection() == 0) enota = "ms:";
	else if (choicePinBranje->GetSelection() > 0) enota = "bar:";
	else enota = "/:";
	dc.DrawText(enota, wxPoint(265, 8));


	for (int i = 0; i < 8; i++) {
		short najdena = 0;
		for(int j = 0; j < seznamPovezav.size(); j++) {
			if (seznamPovezav[j][0] == izbranElement && seznamPovezav[j][1] == i) {
				
				if (seznamElementov[seznamPovezav[j][2]][2] == TLACNAPOSODA) dc.DrawText(wxString::Format("pin %d: Branje", i), wxPoint(250, predogled.y + 3 + 15 * i));
				else if (seznamElementov[seznamPovezav[j][2]][2] == ELEKTRICNACRPALKA) dc.DrawText(wxString::Format("pin %d: Pisanje", i), wxPoint(250, predogled.y + 3 + 15 * i));
				else if (seznamElementov[seznamPovezav[j][2]][2] == PRIJEMALO) dc.DrawText(wxString::Format("pin %d: Branje/Pisanje", i), wxPoint(250, predogled.y + 3 + 15 * i));
				else if (seznamElementov[seznamPovezav[j][2]][2] == PRISESEK) dc.DrawText(wxString::Format("pin %d: Branje/Pisanje", i), wxPoint(250, predogled.y + 3 + 15 * i));
				else dc.DrawText(wxString::Format("pin %d: /", i), wxPoint(250, 103 + 15 * i));

			}
			else if (seznamPovezav[j][2] == izbranElement && seznamPovezav[j][3] == i) {
				
				if (seznamElementov[seznamPovezav[j][0]][2] == TLACNAPOSODA) dc.DrawText(wxString::Format("pin %d: Branje", i), wxPoint(250, predogled.y + 3 + 15 * i));
				else if (seznamElementov[seznamPovezav[j][0]][2] == ELEKTRICNACRPALKA) dc.DrawText(wxString::Format("pin %d: Pisanje", i), wxPoint(250, predogled.y + 3 + 15 * i));
				else if (seznamElementov[seznamPovezav[j][0]][2] == PRIJEMALO) dc.DrawText(wxString::Format("pin %d: Branje/Pisanje", i), wxPoint(250, predogled.y + 3 + 15 * i));
				else if (seznamElementov[seznamPovezav[j][0]][2] == PRISESEK) dc.DrawText(wxString::Format("pin %d: Branje/Pisanje", i), wxPoint(250, predogled.y + 3 + 15 * i));
				else dc.DrawText(wxString::Format("pin %d: /", i), wxPoint(250, 103 + 15 * i));

			}
			else najdena++;
		}
		if (najdena == seznamPovezav.size()) dc.DrawText(wxString::Format("pin %d: -", i), wxPoint(250, predogled.y + 3 + 15 * i));
	}
}



wxSpinCtrlDouble* izkCrpalke;
wxSpinCtrlDouble* notTorCrpalke;
wxSpinCtrlDouble* volVrtCrpalke;
wxSpinCtrlDouble* povRotCrpalke;
wxSpinCtrlDouble* rocPriSilCrpalke;
wxRadioBox* konstParameterCrpalke;

NastavitevCrpalke::NastavitevCrpalke() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve Crpalke"), wxPoint(0, 0), wxSize(400, 300)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(10, 230), wxDefaultSize);
	wxButton* close = new wxButton(panel, wxID_ANY, "Close", wxPoint(90, 230), wxDefaultSize);

	izkCrpalke = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(100, 5), wxSize(80, -1), wxSP_ARROW_KEYS | wxSP_WRAP, 0, 100, seznamLastnosti[izbranElement][0] * 100, .01);
	notTorCrpalke = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(100, 35), wxSize(80, -1), wxSP_ARROW_KEYS | wxSP_WRAP, 0, 100, seznamLastnosti[izbranElement][1], .1);
	volVrtCrpalke = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(100, 65), wxSize(80, -1), wxSP_ARROW_KEYS | wxSP_WRAP, 0, 10, seznamLastnosti[izbranElement][2], .01);
	povRotCrpalke = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(100, 95), wxSize(80, -1), wxSP_ARROW_KEYS | wxSP_WRAP, 0, 10, seznamLastnosti[izbranElement][3], .01);
	rocPriSilCrpalke = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(100, 125), wxSize(80, -1), wxSP_ARROW_KEYS | wxSP_WRAP, 0, 10, seznamLastnosti[izbranElement][4], .01);

	wxArrayString konstPar;
	konstPar.Add("Konst. moc");
	konstPar.Add("Konst. vrtljaji");
	konstParameterCrpalke = new wxRadioBox(panel, wxID_ANY, "Konstantna velicina:", wxPoint(250, 5), wxDefaultSize, konstPar, 1, wxRA_SPECIFY_COLS);
	konstParameterCrpalke->Select(seznamLastnosti[izbranElement][5]);


	apply->Bind(wxEVT_BUTTON, &NastavitevCrpalke::OnApplyClicked, this);
	close->Bind(wxEVT_BUTTON, &NastavitevCrpalke::OnCloseClicked, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(NastavitevCrpalke::OnPaint));
}


void NastavitevCrpalke::OnApplyClicked(wxCommandEvent& evt) {

	seznamLastnosti[izbranElement][0] = izkCrpalke->GetValue() / 100;
	seznamLastnosti[izbranElement][1] = notTorCrpalke->GetValue();
	seznamLastnosti[izbranElement][2] = volVrtCrpalke->GetValue();
	seznamLastnosti[izbranElement][3] = povRotCrpalke->GetValue();
	seznamLastnosti[izbranElement][4] = rocPriSilCrpalke->GetValue();
	seznamLastnosti[izbranElement][5] = konstParameterCrpalke->GetSelection();

	shranjeno = false;
}

void NastavitevCrpalke::OnCloseClicked(wxCommandEvent& evt) {

	Destroy();
}


void NastavitevCrpalke::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());


	wxPoint predogled(80, 200);
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


	dc.DrawText("Izkoristek: ", wxPoint(5, 8));
	dc.DrawText("Notranja torzija: ", wxPoint(5, 38));
	dc.DrawText("Volumen na vrt.: ", wxPoint(5, 68));
	dc.DrawText("Povrsina rotorja: ", wxPoint(5, 98));
	dc.DrawText("Rocica rotorja: ", wxPoint(5, 128));

	dc.DrawText("%", wxPoint(185, 8));
	dc.DrawText("Nm", wxPoint(185, 38));
	dc.DrawText("m^3", wxPoint(185, 68));
	dc.DrawText("m^2", wxPoint(185, 98));
	dc.DrawText("m", wxPoint(185, 128));
}



wxCheckBox* tlakVarnVentBool;
wxSpinCtrlDouble* tlakVarnVent;
wxSpinCtrlDouble* volumenTlacnePosode;
wxSpinCtrlDouble* zacTlakTlacnePosode;

NastavitevTlacnePosode::NastavitevTlacnePosode() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve Tlacne posode"), wxPoint(0, 0), wxSize(360, 300)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(10, 230), wxDefaultSize);
	wxButton* close = new wxButton(panel, wxID_ANY, "Close", wxPoint(90, 230), wxDefaultSize);

	tlakVarnVentBool = new wxCheckBox(panel, wxID_ANY, "Tlacni varnostni ventil", wxPoint(5, 5), wxDefaultSize);
	tlakVarnVent = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(180, 23), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, pogojiOkolja.tlakOzracja / 100000, 10, 7, .1);

	if (seznamLastnosti[izbranElement][1] >= 0) {
		tlakVarnVent->SetValue(seznamLastnosti[izbranElement][1]/100000);
		tlakVarnVentBool->SetValue(1);
	}
	else {
		tlakVarnVent->SetValue(0);
		tlakVarnVentBool->SetValue(0);
	}

	volumenTlacnePosode = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(180, 53), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0.01, 10, seznamLastnosti[izbranElement][0], 0.01);
	zacTlakTlacnePosode = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(180, 83), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 10, seznamResitevReset[izbranElement][2] / 100000, 0.1);


	apply->Bind(wxEVT_BUTTON, &NastavitevTlacnePosode::OnApplyClicked, this);
	close->Bind(wxEVT_BUTTON, &NastavitevTlacnePosode::OnCloseClicked, this);

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
	seznamResitevReset[izbranElement][2] = zacTlakTlacnePosode->GetValue() * 100000;
	
	seznamResitevReset = IzracunVolumna(seznamElementov, seznamResitevReset, seznamLastnosti);
	seznamResitevReset = IzracunMase(pogojiOkolja, seznamElementov, seznamResitevReset);

	seznamResitev = seznamResitevReset;

	shranjeno = false;
}

void NastavitevTlacnePosode::OnCloseClicked(wxCommandEvent& evt) {

	Destroy();
}


void NastavitevTlacnePosode::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());


	wxPoint predogled(60, 125);
	dc.DrawRoundedRectangle(wxPoint(predogled.x, predogled.y), wxSize(120, 40), 20);
	dc.DrawLine(wxPoint(predogled.x + 30, predogled.y - 10), wxPoint(predogled.x + 30, predogled.y));

	dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
	dc.DrawLine(wxPoint(predogled.x - 10, predogled.y + 20), wxPoint(predogled.x, predogled.y + 20));
	dc.DrawLine(wxPoint(predogled.x + 120, predogled.y + 20), wxPoint(predogled.x + 130, predogled.y + 20));
	dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));


	dc.DrawText("Tlak varnostnega ventila [bar]:", wxPoint(5, 25));
	dc.DrawText("Volumen Tlacne posode [m^3]: ", wxPoint(5, 55));
	dc.DrawText("Zacetni tlak v posodi [bar]: ", wxPoint(5, 85));
}



wxSpinCtrl* premerBataPrijemalke;
wxSpinCtrl* premerBatnicePrijemalke;
wxSpinCtrl* dolzinaHodaPrijemalke;
wxSpinCtrl* zacPozPrijemalke;

NastavitevPrijemalke::NastavitevPrijemalke() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve prijemala"), wxPoint(0, 0), wxSize(360, 300)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(10, 230), wxDefaultSize);
	wxButton* close = new wxButton(panel, wxID_ANY, "Close", wxPoint(90, 230), wxDefaultSize);

	premerBataPrijemalke = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(160, 3), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 10, 300, seznamLastnosti[izbranElement][2] * 1000);
	premerBatnicePrijemalke = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(160, 33), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 10, premerBataPrijemalke->GetValue(), seznamLastnosti[izbranElement][3] * 1000);
	dolzinaHodaPrijemalke = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(160, 63), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 10, 300, seznamLastnosti[izbranElement][4] * 1000);
	zacPozPrijemalke = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(160, 93), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 100, seznamLastnosti[izbranElement][5]);


	apply->Bind(wxEVT_BUTTON, &NastavitevPrijemalke::OnApplyClicked, this);
	close->Bind(wxEVT_BUTTON, &NastavitevPrijemalke::OnCloseClicked, this);
	premerBataPrijemalke->Bind(wxEVT_SPINCTRL, &NastavitevPrijemalke::OnRefresh, this);
	premerBatnicePrijemalke->Bind(wxEVT_SPINCTRL, &NastavitevPrijemalke::OnRefresh, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(NastavitevPrijemalke::OnPaint));
}


void NastavitevPrijemalke::OnApplyClicked(wxCommandEvent& evt) {

	seznamLastnosti[izbranElement][2] = static_cast<double> (premerBataPrijemalke->GetValue()) / 1000;
	seznamLastnosti[izbranElement][3] = static_cast<double> (premerBatnicePrijemalke->GetValue()) / 1000;
	seznamLastnosti[izbranElement][4] = static_cast<double> (dolzinaHodaPrijemalke->GetValue()) / 1000;
	seznamLastnosti[izbranElement][5] = static_cast<double> (zacPozPrijemalke->GetValue());

	seznamResitevReset = IzracunVolumna(seznamElementov, seznamResitevReset, seznamLastnosti);
	seznamResitevReset = IzracunMase(pogojiOkolja, seznamElementov, seznamResitevReset);

	seznamResitev = seznamResitevReset;

	shranjeno = false;
}

void NastavitevPrijemalke::OnCloseClicked(wxCommandEvent& evt) {

	Destroy();
}

void NastavitevPrijemalke::OnRefresh(wxCommandEvent& evt) {

	Refresh();
}


void NastavitevPrijemalke::OnPaint(wxPaintEvent& evt) {

	premerBatnicePrijemalke->SetRange(10, premerBataPrijemalke->GetValue());

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());


	wxPoint predogled(60, 130);
	dc.DrawRectangle(wxPoint(predogled.x, predogled.y), wxSize(65, 50));
	dc.DrawRectangle(wxPoint(predogled.x + 40, predogled.y), wxSize(50, 10));
	dc.DrawRectangle(wxPoint(predogled.x + 40, predogled.y + 40), wxSize(50, 10));

	dc.DrawLine(wxPoint(predogled.x - 10, predogled.y + 10), wxPoint(predogled.x, predogled.y + 10));
	dc.SetPen(wxPen(wxColour(0, 0, 0), 2, wxPENSTYLE_SOLID));
	dc.DrawLine(wxPoint(predogled.x - 10, predogled.y + 40), wxPoint(predogled.x, predogled.y + 40));
	dc.SetPen(wxPen(wxColour(0, 0, 0), 1, wxPENSTYLE_SOLID));


	dc.DrawText("Premer bata D [mm]: ", wxPoint(5, 5));
	dc.DrawText("Premer batnice d [mm]: ", wxPoint(5, 35));
	dc.DrawText("Hod bata l [mm]: ", wxPoint(5, 65));
	dc.DrawText("Zacetna pozicija bata D [%]: ", wxPoint(5, 95));
}



wxCheckBox* masaPriseskaBool;
wxSpinCtrlDouble* velikostMasePriseska;
wxSpinCtrl* premerPriseska;
wxSpinCtrl* velikostPriseska;

NastavitevPriseska::NastavitevPriseska() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitve priseska"), wxPoint(0, 0), wxSize(360, 300)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(10, 230), wxDefaultSize);
	wxButton* close = new wxButton(panel, wxID_ANY, "Close", wxPoint(90, 230), wxDefaultSize);

	masaPriseskaBool = new wxCheckBox(panel, wxID_ANY, "Dodaj ute�", wxPoint(5, 5), wxDefaultSize);
	if (seznamResitevReset[izbranElement][4] >= 0) masaPriseskaBool->SetValue(true);
	velikostMasePriseska = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(150, 25), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, 100, seznamResitevReset[izbranElement][4], .1);
	if (!masaPriseskaBool->IsChecked()) velikostMasePriseska->Disable();

	premerPriseska = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(150, 55), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 5, 500, seznamLastnosti[izbranElement][0] * 1000);
	velikostPriseska = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(150, 85), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 5, 200, seznamLastnosti[izbranElement][1] * 1000);


	apply->Bind(wxEVT_BUTTON, &NastavitevPriseska::OnApplyClicked, this);
	close->Bind(wxEVT_BUTTON, &NastavitevPriseska::OnCloseClicked, this);
	masaPriseskaBool->Bind(wxEVT_CHECKBOX, &NastavitevPriseska::OnRefresh, this);
	velikostMasePriseska->Bind(wxEVT_SPINCTRLDOUBLE, &NastavitevPriseska::OnRefresh, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(NastavitevPriseska::OnPaint));
}


void NastavitevPriseska::OnRefresh(wxCommandEvent& evt) {

	if (masaPriseskaBool->IsChecked()) velikostMasePriseska->Enable();
	else velikostMasePriseska->Disable();

	Refresh();
}

void NastavitevPriseska::OnApplyClicked(wxCommandEvent& evt) {

	seznamLastnosti[izbranElement][0] = static_cast<double> (premerPriseska->GetValue()) / 1000;
	seznamLastnosti[izbranElement][1] = static_cast<double> (velikostPriseska->GetValue()) / 1000;

	if (masaPriseskaBool->IsChecked()) seznamResitevReset[izbranElement][4] = velikostMasePriseska->GetValue();
	else seznamResitevReset[izbranElement][4] = -1;

	seznamResitevReset = IzracunVolumna(seznamElementov, seznamResitevReset, seznamLastnosti);
	seznamResitevReset = IzracunMase(pogojiOkolja, seznamElementov, seznamResitevReset);

	seznamResitev = seznamResitevReset;

	shranjeno = false;

	Refresh();
}

void NastavitevPriseska::OnCloseClicked(wxCommandEvent& evt) {

	Destroy();
}


void NastavitevPriseska::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();
	wxPoint mousePos = this->ScreenToClient(wxGetMousePosition());


	wxPoint predogled(60, 150);
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

	if (masaPriseskaBool->IsChecked()) {
		dc.DrawRoundedRectangle(wxPoint(predogled.x - 10, predogled.y), wxSize(100, 30), 5);
		dc.DrawText(wxString::Format("%g kg", velikostMasePriseska->GetValue()), wxPoint(predogled.x + 25, predogled.y + 9));
	}


	dc.DrawText("Masa utezi [kg]: ", wxPoint(5, 28));
	dc.DrawText("Premer priseska D [mm]: ", wxPoint(5, 58));
	dc.DrawText("Velikost priseska l [mm]: ", wxPoint(5, 88));
}



wxSpinCtrl* spinSirinaGraf;
wxSpinCtrl* spinVisinaGraf;
wxSpinCtrl* spinElementGraf;
wxChoice* choiceVelicinaGraf;

NastavitevGrafa::NastavitevGrafa() : wxFrame(nullptr, wxID_ANY, wxString::Format("Nastavitev Grafa"), wxPoint(0, 0), wxSize(280, 240)) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxButton* apply = new wxButton(panel, wxID_ANY, "Apply", wxPoint(5, 170), wxDefaultSize);
	wxButton* close = new wxButton(panel, wxID_ANY, "Close", wxPoint(90, 170), wxDefaultSize);

	spinElementGraf = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(100, 5), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 0, seznamElementov.size(), 0);
	if (seznamLastnosti[izbranElement][2] > -1) spinElementGraf->SetValue(seznamLastnosti[izbranElement][2] + 1);
	
	wxArrayString VelicineGraf;
	VelicineGraf.Add(" ");
	VelicineGraf.Add(" ");
	VelicineGraf.Add(" ");
	choiceVelicinaGraf = new wxChoice(panel, wxID_ANY, wxPoint(100, 35), wxSize(120, -1), VelicineGraf);
	if (seznamLastnosti[izbranElement][2] == -1) choiceVelicinaGraf->Disable();
	else {
		choiceVelicinaGraf->Select(0);
		if (!(seznamLastnosti[izbranElement][3] == -1)) {
			if (seznamElementov[seznamLastnosti[izbranElement][2]][2] == ELEKTRICNACRPALKA) {
				if (seznamLastnosti[izbranElement][3] == MASNI_TOK) choiceVelicinaGraf->SetString(0, " Masni tok");
			}
			else if (seznamElementov[seznamLastnosti[izbranElement][2]][2] == TLACNAPOSODA) {
				if (seznamLastnosti[izbranElement][3] == TLAK0_LOG) choiceVelicinaGraf->SetString(0, " Tlak");
			}
			else if (seznamElementov[seznamLastnosti[izbranElement][2]][2] == PRIJEMALO) {
				if (seznamLastnosti[izbranElement][3] == TLAK0_LOG) { choiceVelicinaGraf->SetString(0, " Tlak 0"); choiceVelicinaGraf->Select(0); }
				else if (seznamLastnosti[izbranElement][3] == TLAK1_LOG) { choiceVelicinaGraf->SetString(1, " Tlak 1"); choiceVelicinaGraf->Select(1); }
				else if (seznamLastnosti[izbranElement][3] == BAT_POZ) { choiceVelicinaGraf->SetString(2, " Pozicija bata"); choiceVelicinaGraf->Select(2); }
			}
			else if (seznamElementov[seznamLastnosti[izbranElement][2]][2] == PRISESEK) {
				if (seznamLastnosti[izbranElement][3] == TLAK0_LOG) choiceVelicinaGraf->SetString(0, " Tlak");
			}
		}
	}

	spinSirinaGraf = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(100, 85), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 60, 400, seznamLastnosti[izbranElement][0]);
	spinVisinaGraf = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(100, 115), wxDefaultSize, wxSP_ARROW_KEYS | wxSP_WRAP, 60, 400, seznamLastnosti[izbranElement][1]);


	apply->Bind(wxEVT_BUTTON, &NastavitevGrafa::OnApplyClicked, this);
	close->Bind(wxEVT_BUTTON, &NastavitevGrafa::OnCloseClicked, this);
	spinElementGraf->Bind(wxEVT_SPINCTRL, &NastavitevGrafa::OnRefresh, this);
	///////////////////// Dodat 'Bind' za 'wxEVT_CLOSE_WINDOW'
	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(NastavitevGrafa::OnPaint));
}


void NastavitevGrafa::OnApplyClicked(wxCommandEvent& evt) {

	seznamLastnosti[izbranElement][0] = spinSirinaGraf->GetValue();
	seznamLastnosti[izbranElement][1] = spinVisinaGraf->GetValue();

	if (spinElementGraf->GetValue() > 0) seznamLastnosti[izbranElement][2] = spinElementGraf->GetValue() - 1;

	if (!(choiceVelicinaGraf->GetStringSelection() == " ")) {
		if (choiceVelicinaGraf->GetStringSelection() == " Masni tok") seznamLastnosti[izbranElement][3] = MASNI_TOK;
		else if (choiceVelicinaGraf->GetStringSelection() == " Tlak") seznamLastnosti[izbranElement][3] = TLAK0_LOG;
		else if (choiceVelicinaGraf->GetStringSelection() == " Tlak 0") seznamLastnosti[izbranElement][3] = TLAK0_LOG;
		else if (choiceVelicinaGraf->GetStringSelection() == " Tlak 1") seznamLastnosti[izbranElement][3] = TLAK1_LOG;
		else if (choiceVelicinaGraf->GetStringSelection() == " Pozicija bata") seznamLastnosti[izbranElement][3] = BAT_POZ;
	}

	shranjeno = false;
}

void NastavitevGrafa::OnCloseClicked(wxCommandEvent& evt) {

	Destroy();
}

void NastavitevGrafa::OnRefresh(wxCommandEvent& evt) {

	if (spinElementGraf->GetValue() > 0) {
		
		choiceVelicinaGraf->Enable();

		choiceVelicinaGraf->SetString(0, " ");
		choiceVelicinaGraf->SetString(1, " ");
		choiceVelicinaGraf->SetString(2, " ");

		if (seznamElementov[spinElementGraf->GetValue() - 1][2] == ELEKTRICNACRPALKA) {
			choiceVelicinaGraf->SetString(0, " Masni tok");
		}
		else if (seznamElementov[spinElementGraf->GetValue() - 1][2] == TLACNAPOSODA) {
			choiceVelicinaGraf->SetString(0, " Tlak");
		}
		else if (seznamElementov[spinElementGraf->GetValue() - 1][2] == PRIJEMALO) {
			choiceVelicinaGraf->SetString(0, " Tlak 0");
			choiceVelicinaGraf->SetString(1, " Tlak 1");
			choiceVelicinaGraf->SetString(2, " Pozicija bata");
		}
		else if (seznamElementov[spinElementGraf->GetValue() - 1][2] == PRISESEK) {
			choiceVelicinaGraf->SetString(0, " Tlak");
		}
	}
	else {

		choiceVelicinaGraf->Disable();

		choiceVelicinaGraf->SetString(0, " ");
		choiceVelicinaGraf->SetString(1, " ");
		choiceVelicinaGraf->SetString(2, " ");
	}

	Refresh();
}


void NastavitevGrafa::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize velikostOkna = this->GetSize();

	dc.DrawText("Element:", wxPoint(5, 8));

	wxString el;
	if (spinElementGraf->GetValue() > 0) {
		if (seznamElementov[spinElementGraf->GetValue() - 1][2] == MIKROPROCESOR) el = "Mikroprocesor";
		else if (seznamElementov[spinElementGraf->GetValue() - 1][2] == ELEKTRICNACRPALKA) el = "Crpalka";
		else if (seznamElementov[spinElementGraf->GetValue() - 1][2] == TLACNAPOSODA) el = "Tlacna posoda";
		else if (seznamElementov[spinElementGraf->GetValue() - 1][2] == PRIJEMALO) el = "Prijemalo";
		else if (seznamElementov[spinElementGraf->GetValue() - 1][2] == PRISESEK) el = "Prisesek";
		else if (seznamElementov[spinElementGraf->GetValue() - 1][2] == GRAF) el = "Graf";
		else el = " /";
	}
	else el = " /";
	dc.DrawText(el, wxPoint(140, 8));

	dc.DrawText("Velicina:", wxPoint(5, 38));

	dc.DrawText("Sirina:", wxPoint(5, 88));
	dc.DrawText("Visina:", wxPoint(5, 118));
}