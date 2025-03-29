/***************************************************************
 * Name:      EDR2Main.h
 * Purpose:   Validation of automotive Event Data Recorders
 * Author:    Klaus Friedewald ()
 * Created:   2023-06-28
 * Copyright: Klaus Friedewald ()
 * License:   GNU GENERAL PUBLIC LICENSE Version 3
 **************************************************************/

#ifndef EDR2MAIN_H
#define EDR2MAIN_H

//(*Headers(EDR2Frame)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/textctrl.h>
//*)

class EDR2Frame: public wxFrame
{
    public:

        EDR2Frame(wxWindow* parent,wxWindowID id = -1);
        virtual ~EDR2Frame();

    private:

        //(*Handlers(EDR2Frame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnOpenMMEyClick(wxCommandEvent& event);
        void OnFilePickerMMEyFileChanged(wxFileDirPickerEvent& event);
        void OnFilePickerMMExFileChanged(wxFileDirPickerEvent& event);
        void OnPlotMMEyClick(wxCommandEvent& event);
        void OnTextScaleMMEy(wxCommandEvent& event);
        void OnTextScaleMMEx(wxCommandEvent& event);
        void OnTextScaleMMEz(wxCommandEvent& event);
        void OnFilePickerMMEzFileChanged(wxFileDirPickerEvent& event);
        void OnOpenMMExClick(wxCommandEvent& event);
        void OnOpenMMEzClick(wxCommandEvent& event);
        void OnPlotMMEzClick(wxCommandEvent& event);
        void OnPlotMMExClick(wxCommandEvent& event);
        void OnSpinEDRnumChange(wxSpinEvent& event);
        void OnFilePickerEDRFileChanged(wxFileDirPickerEvent& event);
        void OnOpenEDRClick(wxCommandEvent& event);
        void OnPlotEDRxClick(wxCommandEvent& event);
        void OnPlotEDRyClick(wxCommandEvent& event);
        void OnPlotEDRzClick(wxCommandEvent& event);
        void OnTextTmin(wxCommandEvent& event);
        void OnTextTmax(wxCommandEvent& event);
        void OnTextAmin(wxCommandEvent& event);
        void OnTextAmax(wxCommandEvent& event);
        void OnTextVmin(wxCommandEvent& event);
        void OnTextVmax(wxCommandEvent& event);
        void OnUnitTSelect(wxCommandEvent& event);
        void OnUnitASelect(wxCommandEvent& event);
        void OnUnitVSelect(wxCommandEvent& event);
        void OnGrenzKurveSelect(wxCommandEvent& event);
        void OnPlotAssXClick(wxCommandEvent& event);
        void OnAssTunitSelect(wxCommandEvent& event);
        void OnTextToffset(wxCommandEvent& event);
        void OnTextTboxX(wxCommandEvent& event);
        void OnAssVunitSelect(wxCommandEvent& event);
        void OnTextVboxX(wxCommandEvent& event);
        void OnTextTboxY(wxCommandEvent& event);
        void OnTextTboxZ(wxCommandEvent& event);
        void OnTextVboxY(wxCommandEvent& event);
        void OnTextVboxZ(wxCommandEvent& event);
        void OnPlotAssYClick(wxCommandEvent& event);
        void OnPlotAssZClick(wxCommandEvent& event);
        void OnMenuHardcopySelected(wxCommandEvent& event);
        void OnLicence(wxCommandEvent& event);
        //*)

        //(*Identifiers(EDR2Frame)
        static const long ID_STATICTEXT1;
        static const long ID_STATICTEXT4;
        static const long ID_STATICTEXT5;
        static const long ID_STATICTEXT6;
        static const long ID_STATICTEXT7;
        static const long ID_STATICTEXT2;
        static const long ID_SPINCTRL2;
        static const long ID_STATICTEXT3;
        static const long ID_FILEPICKERCTRL1;
        static const long ID_BUTTON1;
        static const long ID_STATICTEXT50;
        static const long ID_STATICTEXT51;
        static const long ID_STATICTEXT52;
        static const long ID_STATICTEXT53;
        static const long ID_STATICTEXT54;
        static const long ID_STATICTEXT55;
        static const long ID_STATICTEXT56;
        static const long ID_STATICTEXT57;
        static const long ID_STATICTEXT58;
        static const long ID_STATICTEXT59;
        static const long ID_BUTTON3;
        static const long ID_BUTTON9;
        static const long ID_BUTTON10;
        static const long ID_PANEL2;
        static const long ID_STATICTEXT9;
        static const long ID_STATICTEXT14;
        static const long ID_STATICTEXT15;
        static const long ID_STATICTEXT12;
        static const long ID_STATICTEXT13;
        static const long ID_STATICTEXT10;
        static const long ID_TEXTCTRL1;
        static const long ID_STATICTEXT11;
        static const long ID_FILEPICKERCTRL2;
        static const long ID_BUTTON2;
        static const long ID_STATICTEXT16;
        static const long ID_TEXTCTRL2;
        static const long ID_STATICTEXT17;
        static const long ID_FILEPICKERCTRL3;
        static const long ID_BUTTON_OpenMMEy;
        static const long ID_STATICTEXT18;
        static const long ID_TEXTCTRL3;
        static const long ID_STATICTEXT19;
        static const long ID_FILEPICKERCTRL4;
        static const long ID_BUTTON4;
        static const long ID_BUTTON5;
        static const long ID_BUTTON6;
        static const long ID_BUTTON7;
        static const long ID_PANEL3;
        static const long ID_PANEL4;
        static const long ID_PANEL5;
        static const long ID_STATICTEXT42;
        static const long ID_STATICTEXT43;
        static const long ID_STATICTEXT44;
        static const long ID_STATICTEXT45;
        static const long ID_STATICTEXT46;
        static const long ID_STATICTEXT30;
        static const long ID_TEXTCTRL4;
        static const long ID_STATICTEXT31;
        static const long ID_TEXTCTRL5;
        static const long ID_CHOICE1;
        static const long ID_STATICTEXT32;
        static const long ID_TEXTCTRL6;
        static const long ID_STATICTEXT33;
        static const long ID_TEXTCTRL7;
        static const long ID_CHOICE2;
        static const long ID_STATICTEXT34;
        static const long ID_TEXTCTRL8;
        static const long ID_STATICTEXT35;
        static const long ID_TEXTCTRL9;
        static const long ID_CHOICE3;
        static const long ID_PANEL6;
        static const long ID_STATICTEXT47;
        static const long ID_STATICTEXT60;
        static const long ID_STATICTEXT61;
        static const long ID_STATICTEXT48;
        static const long ID_STATICTEXT49;
        static const long ID_STATICTEXT36;
        static const long ID_TEXTCTRL10;
        static const long ID_STATICTEXT8;
        static const long ID_STATICTEXT62;
        static const long ID_STATICTEXT63;
        static const long ID_STATICTEXT37;
        static const long ID_CHOICE5;
        static const long ID_TEXTCTRL17;
        static const long ID_TEXTCTRL18;
        static const long ID_STATICTEXT38;
        static const long ID_STATICTEXT39;
        static const long ID_TEXTCTRL11;
        static const long ID_TEXTCTRL13;
        static const long ID_TEXTCTRL14;
        static const long ID_STATICTEXT40;
        static const long ID_STATICTEXT41;
        static const long ID_TEXTCTRL12;
        static const long ID_TEXTCTRL15;
        static const long ID_TEXTCTRL16;
        static const long ID_STATICTEXT20;
        static const long ID_STATICTEXT66;
        static const long ID_BUTTON11;
        static const long ID_BUTTON12;
        static const long ID_BUTTON13;
        static const long ID_PANEL7;
        static const long ID_PANEL1;
        static const long idMenuHardCopy;
        static const long idMenuQuit;
        static const long idMenuAbout;
        static const long ID_MENUITEM1;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(EDR2Frame)
        wxButton* OpenEDR;
        wxButton* OpenMMEx;
        wxButton* OpenMMEy;
        wxButton* OpenMMEz;
        wxButton* PlotAssX;
        wxButton* PlotAssY;
        wxButton* PlotAssZ;
        wxButton* PlotEDRx;
        wxButton* PlotEDRy;
        wxButton* PlotEDRz;
        wxButton* PlotMMEx;
        wxButton* PlotMMEy;
        wxButton* PlotMMEz;
        wxChoice* GrenzKurve;
        wxChoice* UnitA;
        wxChoice* UnitT;
        wxChoice* UnitV;
        wxFilePickerCtrl* FilePickerEDR;
        wxFilePickerCtrl* FilePickerMMEx;
        wxFilePickerCtrl* FilePickerMMEy;
        wxFilePickerCtrl* FilePickerMMEz;
        wxMenuItem* MenuItem3;
        wxMenuItem* MenuItem4;
        wxPanel* EDRplotPanel;
        wxPanel* MMEplotPanel;
        wxPanel* Panel1;
        wxPanel* Panel2;
        wxPanel* Panel3;
        wxPanel* Panel4;
        wxPanel* Panel5;
        wxSpinCtrl* SpinEDRnum;
        wxStaticText* StaticText10;
        wxStaticText* StaticText11;
        wxStaticText* StaticText12;
        wxStaticText* StaticText13;
        wxStaticText* StaticText14;
        wxStaticText* StaticText15;
        wxStaticText* StaticText16;
        wxStaticText* StaticText17;
        wxStaticText* StaticText18;
        wxStaticText* StaticText19;
        wxStaticText* StaticText1;
        wxStaticText* StaticText2;
        wxStaticText* StaticText30;
        wxStaticText* StaticText31;
        wxStaticText* StaticText32;
        wxStaticText* StaticText33;
        wxStaticText* StaticText34;
        wxStaticText* StaticText35;
        wxStaticText* StaticText36;
        wxStaticText* StaticText38;
        wxStaticText* StaticText39;
        wxStaticText* StaticText3;
        wxStaticText* StaticText40;
        wxStaticText* StaticText41;
        wxStaticText* StaticText42;
        wxStaticText* StaticText43;
        wxStaticText* StaticText44;
        wxStaticText* StaticText45;
        wxStaticText* StaticText46;
        wxStaticText* StaticText47;
        wxStaticText* StaticText48;
        wxStaticText* StaticText49;
        wxStaticText* StaticText4;
        wxStaticText* StaticText50;
        wxStaticText* StaticText51;
        wxStaticText* StaticText52;
        wxStaticText* StaticText53;
        wxStaticText* StaticText54;
        wxStaticText* StaticText55;
        wxStaticText* StaticText56;
        wxStaticText* StaticText57;
        wxStaticText* StaticText58;
        wxStaticText* StaticText59;
        wxStaticText* StaticText5;
        wxStaticText* StaticText60;
        wxStaticText* StaticText61;
        wxStaticText* StaticText64;
        wxStaticText* StaticText6;
        wxStaticText* StaticText7;
        wxStaticText* StaticText9;
        wxStaticText* StaticTextUnitRF;
        wxStaticText* StaticTextUnitTass2;
        wxStaticText* StaticTextUnitTass;
        wxStaticText* StaticTextUnitVass;
        wxStaticText* amax;
        wxStatusBar* MainStatusBar;
        wxTextCtrl* TextAmax;
        wxTextCtrl* TextAmin;
        wxTextCtrl* TextCtrlFreq;
        wxTextCtrl* TextCtrlRange;
        wxTextCtrl* TextScaleMMEx;
        wxTextCtrl* TextScaleMMEy;
        wxTextCtrl* TextScaleMMEz;
        wxTextCtrl* TextTboxX;
        wxTextCtrl* TextTboxY;
        wxTextCtrl* TextTboxZ;
        wxTextCtrl* TextTmax;
        wxTextCtrl* TextTmin;
        wxTextCtrl* TextToffset;
        wxTextCtrl* TextVboxX;
        wxTextCtrl* TextVboxY;
        wxTextCtrl* TextVboxZ;
        wxTextCtrl* TextVmax;
        wxTextCtrl* TextVmin;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // EDR2MAIN_H
