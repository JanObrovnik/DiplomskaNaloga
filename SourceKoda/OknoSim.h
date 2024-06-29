#pragma once
#include <wx/wx.h>

class OknoSim : public wxFrame {

public:
	OknoSim(const wxString& title);
private:
	void OnPaint(wxPaintEvent& evt);

	void RefreshEvent(wxMouseEvent& evt);

	void OnMouseDownEvent(wxMouseEvent& evt);
	void OnMouseUpEvent(wxMouseEvent& evt);
	void OnMouseDoubleEvent(wxMouseEvent& evt);

};

