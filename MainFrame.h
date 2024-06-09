#pragma once
#include <wx/wx.h>

class MainFrame : public wxFrame {

public:
	MainFrame(const wxString& title);

	//void OnButtonRefClicked(wxCommandEvent& evt);
private:
	void OnPaint(wxPaintEvent& evt);

	void OnChoicesClicked(wxCommandEvent& evt);
	void OnSliderChanged(wxCommandEvent& evt);

	void OnButtonDodClicked(wxCommandEvent& evt);
	void OnButtonIzbClicked(wxCommandEvent& evt);
	void OnButtonIzbVseClicked(wxCommandEvent& evt);
	void OnButtonPredVseClicked(wxCommandEvent& evt);
	void OnButtonSimClicked(wxCommandEvent& evt);
	void OnButtonPomClicked(wxCommandEvent& evt);

	void OnMouseEvent(wxMouseEvent& evt);
	void OnDoubleMouseEvent(wxMouseEvent& evt);

	void OnSizeChanged(wxSizeEvent& evt);
};

class PomoznoOkno : public wxFrame {

public:
	PomoznoOkno();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnButtonClicked(wxCommandEvent& evt);
	void OnNastavitveClicked(wxCommandEvent& evt);
	void OnPremerClicked(wxCommandEvent& evt);

	void OnSizeChanged(wxSizeEvent& evt);
};
