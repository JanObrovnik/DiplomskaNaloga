#include "OknoSim.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/time.h>
#include <wx/utils.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <fstream>



 


OknoSim::OknoSim(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {

	wxPanel* panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

	wxSize size = panel->GetSize();
	


	panel->Bind(wxEVT_LEFT_DOWN, &OknoSim::OnMouseEvent, this);

	panel->Connect(wxEVT_PAINT, wxPaintEventHandler(OknoSim::OnPaint));




}


void OknoSim::OnMouseEvent(wxMouseEvent& evt) {

	wxLogStatus("0");
}


void OknoSim::OnPaint(wxPaintEvent& evt) {

	wxPaintDC dc(this);
	wxSize size = this->GetSize();


	wxPoint mikroProcesor(20,20);
	dc.DrawRectangle(wxPoint(mikroProcesor.x, mikroProcesor.y), wxSize(100, 140));
	dc.DrawText("0", wxPoint(mikroProcesor.x + 88, mikroProcesor.y + 5));
	dc.DrawLine(wxPoint(mikroProcesor.x + 100, mikroProcesor.y + 10), wxPoint(mikroProcesor.x + 110, mikroProcesor.y + 10));
	dc.DrawText("1", wxPoint(mikroProcesor.x + 88, mikroProcesor.y + 20));
	dc.DrawLine(wxPoint(mikroProcesor.x + 100, mikroProcesor.y + 25), wxPoint(mikroProcesor.x + 110, mikroProcesor.y + 25));
	dc.DrawText("2", wxPoint(mikroProcesor.x + 88, mikroProcesor.y + 35));
	dc.DrawLine(wxPoint(mikroProcesor.x + 100, mikroProcesor.y + 40), wxPoint(mikroProcesor.x + 110, mikroProcesor.y + 40));
	dc.DrawText("3", wxPoint(mikroProcesor.x + 88, mikroProcesor.y + 50));
	dc.DrawLine(wxPoint(mikroProcesor.x + 100, mikroProcesor.y + 55), wxPoint(mikroProcesor.x + 110, mikroProcesor.y + 55));
	dc.DrawText("4", wxPoint(mikroProcesor.x + 88, mikroProcesor.y + 65));
	dc.DrawLine(wxPoint(mikroProcesor.x + 100, mikroProcesor.y + 70), wxPoint(mikroProcesor.x + 110, mikroProcesor.y + 70));
	dc.DrawText("5", wxPoint(mikroProcesor.x + 88, mikroProcesor.y + 80));
	dc.DrawLine(wxPoint(mikroProcesor.x + 100, mikroProcesor.y + 85), wxPoint(mikroProcesor.x + 110, mikroProcesor.y + 85));
	dc.DrawText("6", wxPoint(mikroProcesor.x + 88, mikroProcesor.y + 95));
	dc.DrawLine(wxPoint(mikroProcesor.x + 100, mikroProcesor.y + 100), wxPoint(mikroProcesor.x + 110, mikroProcesor.y + 100));
	dc.DrawText("7", wxPoint(mikroProcesor.x + 88, mikroProcesor.y + 110));
	dc.DrawLine(wxPoint(mikroProcesor.x + 100, mikroProcesor.y + 115), wxPoint(mikroProcesor.x + 110, mikroProcesor.y + 115));

	wxPoint elektricnaCrpalka(180, 540);
	dc.DrawLine(wxPoint(elektricnaCrpalka.x, elektricnaCrpalka.y), wxPoint(elektricnaCrpalka.x + 40, elektricnaCrpalka.y));
	dc.DrawLine(wxPoint(elektricnaCrpalka.x + 40, elektricnaCrpalka.y), wxPoint(elektricnaCrpalka.x + 20, elektricnaCrpalka.y - 34));
	dc.DrawLine(wxPoint(elektricnaCrpalka.x + 20, elektricnaCrpalka.y - 34), wxPoint(elektricnaCrpalka.x, elektricnaCrpalka.y));

	wxPoint tlacnaPosoda(240, 320);
	dc.DrawRoundedRectangle(wxPoint(tlacnaPosoda.x, tlacnaPosoda.y), wxSize(120, 200), 6);
	dc.DrawRectangle(wxPoint(tlacnaPosoda.x + 10, tlacnaPosoda.y + 199), wxSize(100, 20));

	wxPoint prijemalo();

	wxPoint prisesek();


	//- Risanje crt:
	wxPoint point1(120, 60);
	wxPoint point2(780, 360);

	dc.DrawLine(point1, wxPoint((point1.x + point2.x) / 2, point1.y));
	dc.DrawLine(wxPoint((point1.x + point2.x) / 2, point1.y), wxPoint((point1.x + point2.x) / 2, point2.y));
	dc.DrawLine(wxPoint((point1.x + point2.x) / 2, point2.y), point2);
}
