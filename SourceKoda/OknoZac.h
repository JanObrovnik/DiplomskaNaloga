#pragma once
#include <wx/wx.h>

class OknoZac : public wxFrame {

public:
	OknoZac(const wxString& title);

private:
	void OnButton1Clicked(wxCommandEvent& evt);
	void OnButton2Clicked(wxCommandEvent& evt);
	void OnButton3Clicked(wxCommandEvent& evt);


	wxString m_textCtrl;
};

