#pragma once
#include <wx/wx.h>

class OknoSim : public wxFrame {

public:
	OknoSim(const wxString& title);
private:
	void OnPaint(wxPaintEvent& evt);

	void OnMouseEvent(wxMouseEvent& evt);

};

