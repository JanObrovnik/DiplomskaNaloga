#pragma once
#include <wx/wx.h>

class OknoSim : public wxFrame {

public:
	OknoSim(const wxString& title);
private:
	void OnMouseEvent(wxMouseEvent& evt);

};

