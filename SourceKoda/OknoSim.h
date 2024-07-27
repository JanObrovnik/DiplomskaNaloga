#pragma once
#include <wx/wx.h>

class OknoSim : public wxFrame {

public:
	OknoSim(const wxString& title);
private:
	void OnPaint(wxPaintEvent& evt);

	void OnSizeChanged(wxSizeEvent& evt);

	void RefreshEvent(wxMouseEvent& evt);
	void OnMouseDownEvent(wxMouseEvent& evt);
	void OnMouseUpEvent(wxMouseEvent& evt);
	void OnMouseDoubleEvent(wxMouseEvent& evt);

	void OnRisanjePovezavClicked(wxCommandEvent& evt);
	void OnSpinPovezavaChanged(wxCommandEvent& evt);
	void OnBrisanjePovezavClicked(wxCommandEvent& evt);
	void OnSimulirajClicked(wxCommandEvent& evt);
	void OnResetSimClicked(wxCommandEvent& evt);
	void OnShraniClicked(wxCommandEvent& evt);
	void OnNaloziClicked(wxCommandEvent& evt);
	void OnButtonIzbClicked(wxCommandEvent& evt);
	void OnButtonIzbVseClicked(wxCommandEvent& evt);
};

class NastavitevMikroProcesorja : public wxFrame {

public:
	NastavitevMikroProcesorja();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnApplyClicked(wxCommandEvent& evt);
	void OnDodajClicked(wxCommandEvent& evt);
	void OnIzbrisiClicked(wxCommandEvent& evt);

	void OnRefresh(wxCommandEvent& evt);
};

class NastavitevCrpalke : public wxFrame {

public:
	NastavitevCrpalke();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnApplyClicked(wxCommandEvent& evt);
};

class NastavitevTlacnePosode : public wxFrame {

public:
	NastavitevTlacnePosode();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnApplyClicked(wxCommandEvent& evt);
};

class NastavitevPrijemalke : public wxFrame {

public:
	NastavitevPrijemalke();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnApplyClicked(wxCommandEvent& evt);

	void OnRefresh(wxCommandEvent& evt);
};

class NastavitevPriseska : public wxFrame {

public:
	NastavitevPriseska();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnApplyClicked(wxCommandEvent& evt);

	void OnRefresh(wxCommandEvent& evt);
};
