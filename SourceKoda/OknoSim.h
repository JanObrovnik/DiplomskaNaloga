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
	void OnSimulirajClicked(wxCommandEvent& evt);
};

class NastavitevMikroProcesorja : public wxFrame {

public:
	NastavitevMikroProcesorja();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnApplyClicked(wxCommandEvent& evt);
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
};

class NastavitevPriseska : public wxFrame {

public:
	NastavitevPriseska();
private:
	void OnPaint(wxPaintEvent& evt);

	void OnApplyClicked(wxCommandEvent& evt);
};
