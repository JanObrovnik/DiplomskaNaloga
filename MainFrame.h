#pragma once
#include <wx/wx.h>

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title);

	void OnPaint(wxPaintEvent& evt);
private:
	void OnChoicesClicked(wxCommandEvent& evt);
	void OnSliderChanged(wxCommandEvent& evt);

	void OnButtonDodClicked(wxCommandEvent& evt);
	void OnButtonIzbClicked(wxCommandEvent& evt);
	void OnButtonIzbVseClicked(wxCommandEvent& evt);

	void OnMouseEvent(wxMouseEvent& evt);


	
};
