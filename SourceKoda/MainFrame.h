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
	void OnCevClicked(wxCommandEvent& evt);
	void OnCevIzbClicked(wxCommandEvent& evt);
	void OnButtonPredVseClicked(wxCommandEvent& evt);
	void OnButtonSimClicked(wxCommandEvent& evt);
	void OnButtonPomClicked(wxCommandEvent& evt);
	void OnShraniClicked(wxCommandEvent& evt);
	void OnNaloziClicked(wxCommandEvent& evt);

	void OnMouseEvent(wxMouseEvent& evt);
	void OnDoubleMouseEvent(wxMouseEvent& evt);
	void OnMouseMoveEvent(wxMouseEvent& evt);
	void OnMouseUpEvent(wxMouseEvent& evt);

	void OnSizeChanged(wxSizeEvent& evt);
};

class PomoznoOkno : public wxFrame {

public:
	PomoznoOkno();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnButtonClicked(wxCommandEvent& evt);
	PomoznoOkno(wxPaintEvent& evt);
	void OnNastavitveClicked(wxCommandEvent& evt);
	void OnPremerClicked(wxCommandEvent& evt);
	void OnStikaloClicked(wxCommandEvent& evt);

	void OnSizeChanged(wxSizeEvent& evt);
};

class StikaloNastavitev : public wxFrame {

public:
	StikaloNastavitev();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnBrisiClicked(wxCommandEvent& evt);
	void OnApllyClicked(wxCommandEvent& evt);
	void OnSliderChanged(wxCommandEvent& evt);

	void OnSizeChanged(wxSizeEvent& evt);
};

class VentilNastavitve : public wxFrame {

public:
	VentilNastavitve();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnApllyClicked(wxCommandEvent& ect);
	void OnNastavitveChanged(wxCommandEvent& evt);
	void OnBrisiLevaChanged(wxCommandEvent& evt);
	void OnBrisiDesnaChanged(wxCommandEvent& evt);

	void OnSizeChanged(wxSizeEvent& evt);
};

class KompresorNastavitve : public wxFrame {

public:
	KompresorNastavitve();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnNastavitveChanged(wxCommandEvent& evt);
	void OnApllyClicked(wxCommandEvent& evt);
};
