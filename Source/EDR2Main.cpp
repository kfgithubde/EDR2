/***************************************************************
 * Name:      EDR2Main.cpp
 * Purpose:   Validation of automotive Event Data Recorders
 * Author:    Dr.-Ing. Klaus Friedewald)
 * Created:   2023-06-28
 * Copyright: Dr.-Ing. Klaus Friedewald
 * License:   GNU GENERAL PUBLIC LICENSE Version 3
 **************************************************************/

#include "EDR2Main.h"
#include "version.h"

//(*InternalHeaders(EDR2Frame)
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>
#include <wx/valtext.h>  // constants for input validation

#include <wx/stdpaths.h> // needed for token %:
#include <wx/filename.h>

#include <wx/xml/xml.h>    // Read inifiles

#include "EDRinput.hpp"  // common Block EDRINPUT
#include "./graph2d.h"

// ----------------   My own declarations ----------------------

#define EDR2WWW "<https://github.com/kfgithubde/EDR2>"

#define ASS_A 4 // First 3 Assessments for velocity, then acceleration

#define titlen 80
#define filnamlen 255

#define EDR_MSG_PLOT1 _("No accident EDR data") // Copy of text used in *.f08 ...
#define EDR_MSG_PLOT2 _("No crash MME data")    // ...as input to i18n gettext
#define EDR_MSG_PLOT3 _("No time data")


#define EDR_XMLSECT0 "EDR2" // rootnode
#define EDR_XMLSECT11 "General"           // 1st level section
    #define EDR_XML_LOCALE "Localization" // 2nd level sections
    #define EDR_XML_USRCONFIG "XMLfile"
#define EDR_XMLSECT12 "Charts"         // 1st level section
  #define EDR_XMLSECT21 "EDRpreview"   // 2nd level sections
  #define EDR_XMLSECT22 "MMEpreview"
  #define EDR_XMLSECT23 "Assessment"
    #define EDR_XML_TITLE_X "TitleX"  // 3nd level
    #define EDR_XML_TITLE_Y "TitleY"
    #define EDR_XML_TITLE_Z "TitleZ"
    #define EDR_XML_COLORMME "ColorMME"
    #define EDR_XML_ColorBOX "ColorBox"
    #define EDR_XML_LineEDR "LineTypeEDR"
    #define EDR_XML_SymbolEDR "SymbolEDR"

#define PROGDIRTOKEN "%:"
#define LOC_CATALOG "EDR2"
#define EDR_XML_FILNAM _("%:EDR2config.xml") // Usertemplate could be localized

wxString EDRprogDir;
char EDRxmlFileName [filnamlen];
wxLanguage UsrLang;
wxLocale m_Locale;

char EDRplotXtitle[titlen]="",EDRplotYtitle[titlen]="",EDRplotZtitle[titlen]="";
char MMEplotXtitle[titlen]="",MMEplotYtitle[titlen]="",MMEplotZtitle[titlen]="";
char ASSplotXtitle[titlen]="",ASSplotYtitle[titlen]="",ASSplotZtitle[titlen]="";

void ReadCrashData (const char filnam[], float yFac,
                    float xDatArr[], float yDatArr[]); // from Flex-Code ReadCrashData.l

extern "C" {
    void MMEplotRaw ( float tMME[],  float yMME[], const char title[]);
    void EDRplotAV ( float t[], float v[], float a[], const char title[]);
    void ASSplot ( float tEDR[], float vEDR[], float tMME[],  float aMME[] ,
                  float* dv, float* CornerF,
                   float* vBoxt,  float* vBoxV, const char title[]);
    void ASSplotA ( float tEDR[], float aEDR[],  float tMME[], float aMME[] ,
                  float* da, float* CornerF, const char title[]);
}

wxTextValidator NumValidator (wxFILTER_NUMERIC); // für wxTextCtrl: nur Zahleneingabe

// ----------------   My own helper functions ----------------------

extern "C" {
    float fac2SI (const char  unit[])
    {
      if ((strcmp ((char*)unit,_("s")) == 0) ||
          (strcmp ((char*)unit,_("m/s")) == 0) ||
          (strcmp ((char*)unit,_("m/(s*s)")) == 0) ) { // SI base
        return 1.;
      } else if (strcmp ((char*)unit,_("ms")) == 0) { // time
        return 0.001;
      } else if (strcmp ((char*)unit,_("km/h")) == 0) { // velocity
        return 1./3.6;
      } else if (strcmp ((char*)unit,_("mph")) == 0) {
        return 1.60934/3.6;
      } else if (strcmp ((char*)unit,_("g")) == 0) { // acceleration
        return 9.80665; // accuracy according SAE J211
      } else {
        return 0.;
      }
   }
}

extern "C" {
  void ftn_gettext (const char instring[],char outstring[],int* ls)
  {
    strncpy (outstring,wxGetTranslation (instring).data(),*ls-1);
    return;
  }
}

boolean XMLreadMainProgPar (const char * filname)
{
  wxXmlDocument xmlDoc;
  wxXmlNode *node, *node1, *node2, *NodeSect0;

  size_t iL;
  wxString strbuf;

    if (filname[0] != '\0') {
      if (!wxFileExists(filname)) {
         return false; // No input file: continue with defaults
      }
      if (!xmlDoc.Load(filname)) {
        return false; // unexpected file error: NOP
      }
      if (xmlDoc.GetRoot() == nullptr) {
        return false; // No root node
      }
      NodeSect0=  nullptr;
      if (xmlDoc.GetRoot()->GetName().IsSameAs(EDR_XMLSECT0)) {
        NodeSect0= xmlDoc.GetRoot() ;
      } else {
        node= xmlDoc.GetRoot()->GetChildren();
        while (node != nullptr) {
          if (node->GetName().IsSameAs(EDR_XMLSECT0)) {
            NodeSect0= node;
            break;
          }
          node= node->GetNext();
        }
      }
      if (NodeSect0 != nullptr) {
        node1= NodeSect0->GetChildren();
        while (node1 != nullptr) {
          if (node1->GetName().IsSameAs(EDR_XMLSECT11)) { // --- General ---
            node= node1->GetChildren ();
            while (node != nullptr) {
              if (node->GetName().IsSameAs(EDR_XML_LOCALE)) { // Select Language
                iL= node->GetNodeContent().length();
                if (iL > 0) {
                  strbuf= node->GetNodeContent().Truncate(titlen);
                  if (m_Locale.FindLanguageInfo (strbuf) != nullptr) {
                    UsrLang= (wxLanguage) m_Locale.FindLanguageInfo (strbuf)->Language;
                  }
                }
              } else if (node->GetName().IsSameAs(EDR_XML_USRCONFIG)) {
                iL= node->GetNodeContent().length();
                if (iL > 0) {
                  strbuf= node->GetNodeContent().Truncate(filnamlen);
                  strncpy (EDRxmlFileName,(const char*) strbuf.mb_str(wxConvLocal),filnamlen);
                }
              }
              node= node->GetNext();
            } // end dataloop EDR_XMLSECT11

          } else if (node1->GetName().IsSameAs(EDR_XMLSECT12)) { // --- Charts ---
            node2= node1->GetChildren();
            while (node2 != nullptr) {
              if (node2->GetName().IsSameAs(EDR_XMLSECT21)) { // EDRplot
                node= node2->GetChildren();
                while (node != nullptr) {
                  if (node->GetName().IsSameAs(EDR_XML_TITLE_X)) { // preview (left)
                    iL= node->GetNodeContent().length();
                    if (iL > 0) {
                      strbuf= node->GetNodeContent().Truncate(titlen);
                      strncpy (EDRplotXtitle,(const char*) strbuf.mb_str(wxConvLocal),titlen);
                    }
                  } else if (node->GetName().IsSameAs(EDR_XML_TITLE_Y)) {
                    iL= node->GetNodeContent().length();
                    if (iL > 0) {
                      strbuf= node->GetNodeContent().Truncate(titlen);
                      strncpy (EDRplotYtitle,(const char*) strbuf.mb_str(wxConvLocal),titlen);
                    }
                  } else if (node->GetName().IsSameAs(EDR_XML_TITLE_Z)) {
                    iL= node->GetNodeContent().length();
                    if (iL > 0) {
                      strbuf= node->GetNodeContent().Truncate(titlen);
                      strncpy (EDRplotZtitle,(const char*) strbuf.mb_str(wxConvLocal),titlen);
                    }
                  }
                  node= node->GetNext();
                } // end dataloop EDR_XMLSECT21

              } else if (node2->GetName().IsSameAs(EDR_XMLSECT22)) { // MMEplot
                node= node2->GetChildren();
                while (node != nullptr) {
                  if (node->GetName().IsSameAs(EDR_XML_TITLE_X)) { // preview (right)
                    iL= node->GetNodeContent().length();
                    if (iL > 0) {
                      strbuf= node->GetNodeContent().Truncate(titlen);
                      strncpy (MMEplotXtitle,(const char*) strbuf.mb_str(wxConvLocal),titlen);
                    }
                  } else if (node->GetName().IsSameAs(EDR_XML_TITLE_Y)) {
                    iL= node->GetNodeContent().length();
                    if (iL > 0) {
                      strbuf= node->GetNodeContent().Truncate(titlen);
                      strncpy (MMEplotYtitle,(const char*) strbuf.mb_str(wxConvLocal),titlen);
                    }
                  } else if (node->GetName().IsSameAs(EDR_XML_TITLE_Z)) {
                    iL= node->GetNodeContent().length();
                    if (iL > 0) {
                      strbuf= node->GetNodeContent().Truncate(titlen);
                      strncpy (MMEplotZtitle,(const char*) strbuf.mb_str(wxConvLocal),titlen);
                    }
                  }
                  node= node->GetNext();
                } // end dataloop EDR_XMLSECT22

               } else if (node2->GetName().IsSameAs(EDR_XMLSECT23)) { // ASSplot
                node= node2->GetChildren();
                while (node != nullptr) {
                  if (node->GetName().IsSameAs(EDR_XML_TITLE_X)) { // new window
                    iL= node->GetNodeContent().length();
                    if (iL > 0) {
                      strbuf= node->GetNodeContent().Truncate(titlen);
                      strncpy (ASSplotXtitle,(const char*) strbuf.mb_str(wxConvLocal),titlen);
                    }
                  } else if (node->GetName().IsSameAs(EDR_XML_TITLE_Y)) {
                    iL= node->GetNodeContent().length();
                    if (iL > 0) {
                      strbuf= node->GetNodeContent().Truncate(titlen);
                      strncpy (ASSplotYtitle,(const char*) strbuf.mb_str(wxConvLocal),titlen);
                    }
                  } else if (node->GetName().IsSameAs(EDR_XML_TITLE_Z)) {
                    iL= node->GetNodeContent().length();
                    if (iL > 0) {
                      strbuf= node->GetNodeContent().Truncate(titlen);
                      strncpy (ASSplotZtitle,(const char*) strbuf.mb_str(wxConvLocal),titlen);
                    }
                  } else if (node->GetName().IsSameAs(EDR_XML_COLORMME)) {
                    if (node->GetNodeContent().IsNumber()) {
                      edrinput_.iColorMME= wxAtoi(node->GetNodeContent());
                    }
                  } else if (node->GetName().IsSameAs(EDR_XML_ColorBOX)) {
                    if (node->GetNodeContent().IsNumber()) {
                      edrinput_.iColorBOX= wxAtoi(node->GetNodeContent());
                    }
                  } else if (node->GetName().IsSameAs(EDR_XML_LineEDR)) {
                    if (node->GetNodeContent().IsNumber()) {
                      edrinput_.iLineTypeEDR= wxAtoi(node->GetNodeContent());
                    }
                  } else if (node->GetName().IsSameAs(EDR_XML_SymbolEDR)) {
                    if (node->GetNodeContent().IsNumber()) {
                      edrinput_.iSymbolEDR= wxAtoi(node->GetNodeContent());
                    }
                  }
                  node= node->GetNext();
                } // end dataloop EDR_XMLSECT23

              }
              node2= node2->GetNext(); // continue searching in 2nd level
            }
          } // end dataloop EDR_XMLSECT12
          node1= node1->GetNext (); // search 1st level
        }
      }
    } // end if filename...
  return true;
 }



wxString ValUnitRFlabel (int iGrenz)
{
  if (iGrenz < ASS_A) {
    return wxT("km/h   Hz");
  } else {
    return wxT("g   Hz");
  }
}


// ---- End of my helper functions. Now helper from wxSmith ----

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(EDR2Frame)
const long EDR2Frame::ID_STATICTEXT1 = wxNewId();
const long EDR2Frame::ID_STATICTEXT4 = wxNewId();
const long EDR2Frame::ID_STATICTEXT5 = wxNewId();
const long EDR2Frame::ID_STATICTEXT6 = wxNewId();
const long EDR2Frame::ID_STATICTEXT7 = wxNewId();
const long EDR2Frame::ID_STATICTEXT2 = wxNewId();
const long EDR2Frame::ID_SPINCTRL2 = wxNewId();
const long EDR2Frame::ID_STATICTEXT3 = wxNewId();
const long EDR2Frame::ID_FILEPICKERCTRL1 = wxNewId();
const long EDR2Frame::ID_BUTTON1 = wxNewId();
const long EDR2Frame::ID_STATICTEXT50 = wxNewId();
const long EDR2Frame::ID_STATICTEXT51 = wxNewId();
const long EDR2Frame::ID_STATICTEXT52 = wxNewId();
const long EDR2Frame::ID_STATICTEXT53 = wxNewId();
const long EDR2Frame::ID_STATICTEXT54 = wxNewId();
const long EDR2Frame::ID_STATICTEXT55 = wxNewId();
const long EDR2Frame::ID_STATICTEXT56 = wxNewId();
const long EDR2Frame::ID_STATICTEXT57 = wxNewId();
const long EDR2Frame::ID_STATICTEXT58 = wxNewId();
const long EDR2Frame::ID_STATICTEXT59 = wxNewId();
const long EDR2Frame::ID_BUTTON3 = wxNewId();
const long EDR2Frame::ID_BUTTON9 = wxNewId();
const long EDR2Frame::ID_BUTTON10 = wxNewId();
const long EDR2Frame::ID_PANEL2 = wxNewId();
const long EDR2Frame::ID_STATICTEXT9 = wxNewId();
const long EDR2Frame::ID_STATICTEXT14 = wxNewId();
const long EDR2Frame::ID_STATICTEXT15 = wxNewId();
const long EDR2Frame::ID_STATICTEXT12 = wxNewId();
const long EDR2Frame::ID_STATICTEXT13 = wxNewId();
const long EDR2Frame::ID_STATICTEXT10 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL1 = wxNewId();
const long EDR2Frame::ID_STATICTEXT11 = wxNewId();
const long EDR2Frame::ID_FILEPICKERCTRL2 = wxNewId();
const long EDR2Frame::ID_BUTTON2 = wxNewId();
const long EDR2Frame::ID_STATICTEXT16 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL2 = wxNewId();
const long EDR2Frame::ID_STATICTEXT17 = wxNewId();
const long EDR2Frame::ID_FILEPICKERCTRL3 = wxNewId();
const long EDR2Frame::ID_BUTTON_OpenMMEy = wxNewId();
const long EDR2Frame::ID_STATICTEXT18 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL3 = wxNewId();
const long EDR2Frame::ID_STATICTEXT19 = wxNewId();
const long EDR2Frame::ID_FILEPICKERCTRL4 = wxNewId();
const long EDR2Frame::ID_BUTTON4 = wxNewId();
const long EDR2Frame::ID_BUTTON5 = wxNewId();
const long EDR2Frame::ID_BUTTON6 = wxNewId();
const long EDR2Frame::ID_BUTTON7 = wxNewId();
const long EDR2Frame::ID_PANEL3 = wxNewId();
const long EDR2Frame::ID_PANEL4 = wxNewId();
const long EDR2Frame::ID_PANEL5 = wxNewId();
const long EDR2Frame::ID_STATICTEXT42 = wxNewId();
const long EDR2Frame::ID_STATICTEXT43 = wxNewId();
const long EDR2Frame::ID_STATICTEXT44 = wxNewId();
const long EDR2Frame::ID_STATICTEXT45 = wxNewId();
const long EDR2Frame::ID_STATICTEXT46 = wxNewId();
const long EDR2Frame::ID_STATICTEXT30 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL4 = wxNewId();
const long EDR2Frame::ID_STATICTEXT31 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL5 = wxNewId();
const long EDR2Frame::ID_CHOICE1 = wxNewId();
const long EDR2Frame::ID_STATICTEXT32 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL6 = wxNewId();
const long EDR2Frame::ID_STATICTEXT33 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL7 = wxNewId();
const long EDR2Frame::ID_CHOICE2 = wxNewId();
const long EDR2Frame::ID_STATICTEXT34 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL8 = wxNewId();
const long EDR2Frame::ID_STATICTEXT35 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL9 = wxNewId();
const long EDR2Frame::ID_CHOICE3 = wxNewId();
const long EDR2Frame::ID_PANEL6 = wxNewId();
const long EDR2Frame::ID_STATICTEXT47 = wxNewId();
const long EDR2Frame::ID_STATICTEXT60 = wxNewId();
const long EDR2Frame::ID_STATICTEXT61 = wxNewId();
const long EDR2Frame::ID_STATICTEXT48 = wxNewId();
const long EDR2Frame::ID_STATICTEXT49 = wxNewId();
const long EDR2Frame::ID_STATICTEXT36 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL10 = wxNewId();
const long EDR2Frame::ID_STATICTEXT8 = wxNewId();
const long EDR2Frame::ID_STATICTEXT62 = wxNewId();
const long EDR2Frame::ID_STATICTEXT63 = wxNewId();
const long EDR2Frame::ID_STATICTEXT37 = wxNewId();
const long EDR2Frame::ID_CHOICE5 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL17 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL18 = wxNewId();
const long EDR2Frame::ID_STATICTEXT38 = wxNewId();
const long EDR2Frame::ID_STATICTEXT39 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL11 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL13 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL14 = wxNewId();
const long EDR2Frame::ID_STATICTEXT40 = wxNewId();
const long EDR2Frame::ID_STATICTEXT41 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL12 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL15 = wxNewId();
const long EDR2Frame::ID_TEXTCTRL16 = wxNewId();
const long EDR2Frame::ID_STATICTEXT20 = wxNewId();
const long EDR2Frame::ID_STATICTEXT66 = wxNewId();
const long EDR2Frame::ID_BUTTON11 = wxNewId();
const long EDR2Frame::ID_BUTTON12 = wxNewId();
const long EDR2Frame::ID_BUTTON13 = wxNewId();
const long EDR2Frame::ID_PANEL7 = wxNewId();
const long EDR2Frame::ID_PANEL1 = wxNewId();
const long EDR2Frame::idMenuHardCopy = wxNewId();
const long EDR2Frame::idMenuQuit = wxNewId();
const long EDR2Frame::idMenuAbout = wxNewId();
const long EDR2Frame::ID_MENUITEM1 = wxNewId();
const long EDR2Frame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EDR2Frame,wxFrame)
    //(*EventTable(EDR2Frame)
    //*)
END_EVENT_TABLE()

EDR2Frame::EDR2Frame(wxWindow* parent,wxWindowID id)
{

// ----------------   My own general initializations ----------------------

    edrinput_.iColorMME=7;     // gray
    edrinput_.iColorBOX= 2;    // red
    edrinput_.iLineTypeEDR= 3; // dotted
    edrinput_.iSymbolEDR= 6;   // diamond

    UsrLang= wxLANGUAGE_DEFAULT; // could be changed 2 times by XML

  wxFileName wxTmpFilNam; // get program directory
    wxTmpFilNam= wxStandardPaths::Get().GetExecutablePath();
    wxTmpFilNam.MakeAbsolute();
    EDRprogDir= wxTmpFilNam.GetPath(
                        wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR,
                        wxPATH_NATIVE);

  wxString tmp; // substitute "%:" with ProgDir
    tmp= EDR_XML_FILNAM; // original template before calling i18n!
    tmp.Replace (PROGDIRTOKEN, EDRprogDir, false);
    strncpy (EDRxmlFileName,(const char*) tmp.mb_str(wxConvLocal),filnamlen);

    winlbl0 ("","",EDRxmlFileName);      // set graph2d settings first
    XMLreadMainProgPar (EDRxmlFileName); // here EDRxmlFileName could change!

    tmp= EDRxmlFileName;
    tmp.Replace (PROGDIRTOKEN, EDRprogDir, false);
    strncpy (EDRxmlFileName,(const char*) tmp.mb_str(wxConvLocal),filnamlen);

    if (XMLreadMainProgPar(EDRxmlFileName) ) { // 2nd reading: now userspecific
      winlbl0 ("","",EDRxmlFileName);          // Graph2D only if succeeded
    }

    m_Locale.Init (UsrLang, wxLOCALE_LOAD_DEFAULT );  // Localisation has to be
    m_Locale.AddCatalogLookupPathPrefix (EDRprogDir); // made before building
    m_Locale.AddCatalog(LOC_CATALOG, UsrLang);        // the GUI

    // Flex (and datafiles too) depend on C-numberformat:
    setlocale (LC_NUMERIC, "C"); // decimalpoint, not comma

    //  Make (localized) presets only if not set by XML before

    if (!EDRplotXtitle[0]) strncpy (EDRplotXtitle,(const char*)
      _("X-Direction EDR: Acceleration and change in velocity")
                                         .mb_str(wxConvLocal),titlen);
    if (!EDRplotYtitle[0])strncpy (EDRplotYtitle,(const char*)
      _("Y-Direction EDR: Acceleration and change in velocity")
                                         .mb_str(wxConvLocal),titlen);
    if (!EDRplotZtitle[0])strncpy (EDRplotZtitle,(const char*)
      _("Z-Direction EDR: Acceleration and change in velocity")
                                         .mb_str(wxConvLocal),titlen);

    if (!MMEplotXtitle[0])strncpy (MMEplotXtitle,(const char*)
      _("X-Direction: MME data set unfiltered and filtered at 150Hz")
                                         .mb_str(wxConvLocal),titlen);
    if (!MMEplotYtitle[0])strncpy (MMEplotYtitle,(const char*)
      _("Y-Direction: MME data set unfiltered and filtered at 150Hz")
                                         .mb_str(wxConvLocal),titlen);
    if (!MMEplotZtitle[0])strncpy (MMEplotZtitle,(const char*)
      _("Z-Direction: MME data set unfiltered and filtered at 150Hz")
                                         .mb_str(wxConvLocal),titlen);

    if (!ASSplotXtitle[0])strncpy (ASSplotXtitle,(const char*)
      _("Assessment X-Direction, error corridor")
                                         .mb_str(wxConvLocal),titlen);
    if (!ASSplotYtitle[0])strncpy (ASSplotYtitle,(const char*)
      _("Assessment Y-Direction, error corridor")
                                         .mb_str(wxConvLocal),titlen);
    if (!ASSplotZtitle[0])strncpy (ASSplotZtitle,(const char*)
      _("Assessment Z-Direction, error corridor")
                                         .mb_str(wxConvLocal),titlen);


// ---- End of my general initializations. Now code basically from wxSmith ----


    //(*Initialize(EDR2Frame)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxMenu* Menu1;
    wxMenu* Menu2;
    wxMenuBar* MenuBar1;
    wxMenuItem* MenuItem1;
    wxMenuItem* MenuItem2;

    Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(1170,710));
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer1 = new wxFlexGridSizer(3, 2, 0, 3);
    Panel2 = new wxPanel(Panel1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED|wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 5, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    StaticText1 = new wxStaticText(Panel2, ID_STATICTEXT1, _("EDR Data               "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(Panel2, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText5 = new wxStaticText(Panel2, ID_STATICTEXT5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(Panel2, ID_STATICTEXT6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(Panel2, ID_STATICTEXT7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(Panel2, ID_STATICTEXT2, _("Record to Use"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    SpinEDRnum = new wxSpinCtrl(Panel2, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 5, 1, _T("ID_SPINCTRL2"));
    SpinEDRnum->SetValue(_T("1"));
    FlexGridSizer2->Add(SpinEDRnum, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(Panel2, ID_STATICTEXT3, _("File"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    FilePickerEDR = new wxFilePickerCtrl(Panel2, ID_FILEPICKERCTRL1, wxEmptyString, wxEmptyString, _T("*.csv"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
    FlexGridSizer2->Add(FilePickerEDR, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    OpenEDR = new wxButton(Panel2, ID_BUTTON1, _("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    OpenEDR->Disable();
    FlexGridSizer2->Add(OpenEDR, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText49 = new wxStaticText(Panel2, ID_STATICTEXT50, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT50"));
    StaticText49->SetMinSize(wxSize(-1,-1));
    FlexGridSizer2->Add(StaticText49, 1, wxALL, wxDLG_UNIT(Panel2,wxSize(5,0)).GetWidth());
    StaticText50 = new wxStaticText(Panel2, ID_STATICTEXT51, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT51"));
    FlexGridSizer2->Add(StaticText50, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText51 = new wxStaticText(Panel2, ID_STATICTEXT52, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT52"));
    FlexGridSizer2->Add(StaticText51, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText52 = new wxStaticText(Panel2, ID_STATICTEXT53, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT53"));
    FlexGridSizer2->Add(StaticText52, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText53 = new wxStaticText(Panel2, ID_STATICTEXT54, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT54"));
    FlexGridSizer2->Add(StaticText53, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText54 = new wxStaticText(Panel2, ID_STATICTEXT55, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT55"));
    FlexGridSizer2->Add(StaticText54, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(Panel2,wxSize(5,0)).GetWidth());
    StaticText55 = new wxStaticText(Panel2, ID_STATICTEXT56, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT56"));
    FlexGridSizer2->Add(StaticText55, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText56 = new wxStaticText(Panel2, ID_STATICTEXT57, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT57"));
    FlexGridSizer2->Add(StaticText56, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText57 = new wxStaticText(Panel2, ID_STATICTEXT58, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT58"));
    FlexGridSizer2->Add(StaticText57, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText58 = new wxStaticText(Panel2, ID_STATICTEXT59, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT59"));
    FlexGridSizer2->Add(StaticText58, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PlotEDRx = new wxButton(Panel2, ID_BUTTON3, _("Plot X"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    PlotEDRx->Disable();
    FlexGridSizer2->Add(PlotEDRx, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PlotEDRy = new wxButton(Panel2, ID_BUTTON9, _("Plot Y"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    PlotEDRy->Disable();
    FlexGridSizer2->Add(PlotEDRy, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PlotEDRz = new wxButton(Panel2, ID_BUTTON10, _("Plot Z"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
    PlotEDRz->Disable();
    FlexGridSizer2->Add(PlotEDRz, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel2->SetSizer(FlexGridSizer2);
    FlexGridSizer2->Fit(Panel2);
    FlexGridSizer2->SetSizeHints(Panel2);
    FlexGridSizer1->Add(Panel2, 1, wxALL|wxEXPAND, 5);
    Panel3 = new wxPanel(Panel1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED|wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer3 = new wxFlexGridSizer(0, 5, 0, 0);
    StaticText9 = new wxStaticText(Panel3, ID_STATICTEXT9, _("ISO-MME Data"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer3->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText14 = new wxStaticText(Panel3, ID_STATICTEXT14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    FlexGridSizer3->Add(StaticText14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText15 = new wxStaticText(Panel3, ID_STATICTEXT15, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
    FlexGridSizer3->Add(StaticText15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText12 = new wxStaticText(Panel3, ID_STATICTEXT12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
    FlexGridSizer3->Add(StaticText12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText13 = new wxStaticText(Panel3, ID_STATICTEXT13, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    FlexGridSizer3->Add(StaticText13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText10 = new wxStaticText(Panel3, ID_STATICTEXT10, _("Scale Factor X"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer3->Add(StaticText10, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextScaleMMEx = new wxTextCtrl(Panel3, ID_TEXTCTRL1, _("1.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer3->Add(TextScaleMMEx, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText11 = new wxStaticText(Panel3, ID_STATICTEXT11, _("File X"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    FlexGridSizer3->Add(StaticText11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FilePickerMMEx = new wxFilePickerCtrl(Panel3, ID_FILEPICKERCTRL2, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL2"));
    FlexGridSizer3->Add(FilePickerMMEx, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    OpenMMEx = new wxButton(Panel3, ID_BUTTON2, _("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    OpenMMEx->Disable();
    FlexGridSizer3->Add(OpenMMEx, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText16 = new wxStaticText(Panel3, ID_STATICTEXT16, _("Scale Factor Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
    FlexGridSizer3->Add(StaticText16, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextScaleMMEy = new wxTextCtrl(Panel3, ID_TEXTCTRL2, _("1.0"), wxDefaultPosition, wxDefaultSize, 0, NumValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer3->Add(TextScaleMMEy, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText17 = new wxStaticText(Panel3, ID_STATICTEXT17, _("File Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
    FlexGridSizer3->Add(StaticText17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FilePickerMMEy = new wxFilePickerCtrl(Panel3, ID_FILEPICKERCTRL3, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL3"));
    FlexGridSizer3->Add(FilePickerMMEy, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    OpenMMEy = new wxButton(Panel3, ID_BUTTON_OpenMMEy, _("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_OpenMMEy"));
    OpenMMEy->Disable();
    FlexGridSizer3->Add(OpenMMEy, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText18 = new wxStaticText(Panel3, ID_STATICTEXT18, _("Scale Factor Z"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
    FlexGridSizer3->Add(StaticText18, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextScaleMMEz = new wxTextCtrl(Panel3, ID_TEXTCTRL3, _("1.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    FlexGridSizer3->Add(TextScaleMMEz, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText19 = new wxStaticText(Panel3, ID_STATICTEXT19, _("File Z"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT19"));
    FlexGridSizer3->Add(StaticText19, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FilePickerMMEz = new wxFilePickerCtrl(Panel3, ID_FILEPICKERCTRL4, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL4"));
    FlexGridSizer3->Add(FilePickerMMEz, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    OpenMMEz = new wxButton(Panel3, ID_BUTTON4, _("Open"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    OpenMMEz->Disable();
    FlexGridSizer3->Add(OpenMMEz, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PlotMMEx = new wxButton(Panel3, ID_BUTTON5, _("Plot X"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    PlotMMEx->Disable();
    FlexGridSizer3->Add(PlotMMEx, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PlotMMEy = new wxButton(Panel3, ID_BUTTON6, _("Plot Y"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    PlotMMEy->Disable();
    FlexGridSizer3->Add(PlotMMEy, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PlotMMEz = new wxButton(Panel3, ID_BUTTON7, _("Plot Z"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    PlotMMEz->Disable();
    FlexGridSizer3->Add(PlotMMEz, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel3->SetSizer(FlexGridSizer3);
    FlexGridSizer3->Fit(Panel3);
    FlexGridSizer3->SetSizeHints(Panel3);
    FlexGridSizer1->Add(Panel3, 1, wxALL|wxEXPAND, 5);
    EDRplotPanel = new wxPanel(Panel1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    EDRplotPanel->SetMinSize(wxSize(340,260));
    EDRplotPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
    FlexGridSizer1->Add(EDRplotPanel, 1, wxALL|wxEXPAND, 5);
    MMEplotPanel = new wxPanel(Panel1, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
    MMEplotPanel->SetMinSize(wxSize(340,260));
    MMEplotPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
    FlexGridSizer1->Add(MMEplotPanel, 1, wxALL|wxEXPAND, 5);
    Panel4 = new wxPanel(Panel1, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED|wxTAB_TRAVERSAL, _T("ID_PANEL6"));
    FlexGridSizer4 = new wxFlexGridSizer(6, 5, 0, 0);
    StaticText41 = new wxStaticText(Panel4, ID_STATICTEXT42, _("Plot Window"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
    FlexGridSizer4->Add(StaticText41, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText42 = new wxStaticText(Panel4, ID_STATICTEXT43, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT43"));
    FlexGridSizer4->Add(StaticText42, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText43 = new wxStaticText(Panel4, ID_STATICTEXT44, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT44"));
    FlexGridSizer4->Add(StaticText43, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText44 = new wxStaticText(Panel4, ID_STATICTEXT45, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT45"));
    FlexGridSizer4->Add(StaticText44, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText45 = new wxStaticText(Panel4, ID_STATICTEXT46, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT46"));
    FlexGridSizer4->Add(StaticText45, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText30 = new wxStaticText(Panel4, ID_STATICTEXT30, _("t min"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT30"));
    FlexGridSizer4->Add(StaticText30, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextTmin = new wxTextCtrl(Panel4, ID_TEXTCTRL4, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
    FlexGridSizer4->Add(TextTmin, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText31 = new wxStaticText(Panel4, ID_STATICTEXT31, _("t max"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
    FlexGridSizer4->Add(StaticText31, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextTmax = new wxTextCtrl(Panel4, ID_TEXTCTRL5, _("-1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL5"));
    FlexGridSizer4->Add(TextTmax, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    UnitT = new wxChoice(Panel4, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    UnitT->SetSelection( UnitT->Append(_("s")) );
    UnitT->Append(_("ms"));
    FlexGridSizer4->Add(UnitT, 1, wxALL|wxEXPAND, 5);
    StaticText32 = new wxStaticText(Panel4, ID_STATICTEXT32, _("a min"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
    FlexGridSizer4->Add(StaticText32, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextAmin = new wxTextCtrl(Panel4, ID_TEXTCTRL6, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL6"));
    FlexGridSizer4->Add(TextAmin, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    amax = new wxStaticText(Panel4, ID_STATICTEXT33, _("a max"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
    FlexGridSizer4->Add(amax, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextAmax = new wxTextCtrl(Panel4, ID_TEXTCTRL7, _("-1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL7"));
    FlexGridSizer4->Add(TextAmax, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    UnitA = new wxChoice(Panel4, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    UnitA->SetSelection( UnitA->Append(_("m/(s*s)")) );
    UnitA->Append(_("g"));
    FlexGridSizer4->Add(UnitA, 1, wxALL|wxEXPAND, 5);
    StaticText33 = new wxStaticText(Panel4, ID_STATICTEXT34, _("v min"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT34"));
    FlexGridSizer4->Add(StaticText33, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextVmin = new wxTextCtrl(Panel4, ID_TEXTCTRL8, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL8"));
    FlexGridSizer4->Add(TextVmin, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText34 = new wxStaticText(Panel4, ID_STATICTEXT35, _("v max"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
    FlexGridSizer4->Add(StaticText34, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextVmax = new wxTextCtrl(Panel4, ID_TEXTCTRL9, _("-1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL9"));
    FlexGridSizer4->Add(TextVmax, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    UnitV = new wxChoice(Panel4, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    UnitV->SetSelection( UnitV->Append(_("m/s")) );
    UnitV->Append(_("km/h"));
    UnitV->Append(_("mph"));
    FlexGridSizer4->Add(UnitV, 1, wxALL|wxEXPAND, 5);
    Panel4->SetSizer(FlexGridSizer4);
    FlexGridSizer4->Fit(Panel4);
    FlexGridSizer4->SetSizeHints(Panel4);
    FlexGridSizer1->Add(Panel4, 1, wxALL|wxEXPAND, 5);
    Panel5 = new wxPanel(Panel1, ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED|wxTAB_TRAVERSAL, _T("ID_PANEL7"));
    FlexGridSizer5 = new wxFlexGridSizer(6, 5, 2, 0);
    StaticText46 = new wxStaticText(Panel5, ID_STATICTEXT47, _("Assessment"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT47"));
    FlexGridSizer5->Add(StaticText46, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText39 = new wxStaticText(Panel5, ID_STATICTEXT60, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT60"));
    FlexGridSizer5->Add(StaticText39, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText59 = new wxStaticText(Panel5, ID_STATICTEXT61, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT61"));
    FlexGridSizer5->Add(StaticText59, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText47 = new wxStaticText(Panel5, ID_STATICTEXT48, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT48"));
    FlexGridSizer5->Add(StaticText47, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText48 = new wxStaticText(Panel5, ID_STATICTEXT49, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT49"));
    FlexGridSizer5->Add(StaticText48, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText35 = new wxStaticText(Panel5, ID_STATICTEXT36, _("t Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
    FlexGridSizer5->Add(StaticText35, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextToffset = new wxTextCtrl(Panel5, ID_TEXTCTRL10, _("0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL10"));
    FlexGridSizer5->Add(TextToffset, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextUnitTass2 = new wxStaticText(Panel5, ID_STATICTEXT8, _("To Inialize"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer5->Add(StaticTextUnitTass2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText60 = new wxStaticText(Panel5, ID_STATICTEXT62, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT62"));
    FlexGridSizer5->Add(StaticText60, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText61 = new wxStaticText(Panel5, ID_STATICTEXT63, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT63"));
    FlexGridSizer5->Add(StaticText61, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText36 = new wxStaticText(Panel5, ID_STATICTEXT37, _("Limit Curve"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
    FlexGridSizer5->Add(StaticText36, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    GrenzKurve = new wxChoice(Panel5, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
    GrenzKurve->Append(_("V: SAE J1698"));
    GrenzKurve->Append(_("V: SAE J1698 adjusted"));
    GrenzKurve->SetSelection( GrenzKurve->Append(_("V: TP 563/UN R-160")) );
    GrenzKurve->Append(_("A: UN R-160"));
    FlexGridSizer5->Add(GrenzKurve, 1, wxALL|wxEXPAND, 5);
    TextCtrlRange = new wxTextCtrl(Panel5, ID_TEXTCTRL17, _("To Inialize"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL17"));
    FlexGridSizer5->Add(TextCtrlRange, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlFreq = new wxTextCtrl(Panel5, ID_TEXTCTRL18, _("To Inialize"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL18"));
    FlexGridSizer5->Add(TextCtrlFreq, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextUnitRF = new wxStaticText(Panel5, ID_STATICTEXT38, _("To Inialize"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
    FlexGridSizer5->Add(StaticTextUnitRF, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText38 = new wxStaticText(Panel5, ID_STATICTEXT39, _("t Box"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
    FlexGridSizer5->Add(StaticText38, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextTboxX = new wxTextCtrl(Panel5, ID_TEXTCTRL11, _("0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL11"));
    FlexGridSizer5->Add(TextTboxX, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextTboxY = new wxTextCtrl(Panel5, ID_TEXTCTRL13, _("0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL13"));
    FlexGridSizer5->Add(TextTboxY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextTboxZ = new wxTextCtrl(Panel5, ID_TEXTCTRL14, _("0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL14"));
    FlexGridSizer5->Add(TextTboxZ, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextUnitTass = new wxStaticText(Panel5, ID_STATICTEXT40, _("To Inialize"), wxPoint(-1,-1), wxDefaultSize, 0, _T("ID_STATICTEXT40"));
    FlexGridSizer5->Add(StaticTextUnitTass, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText40 = new wxStaticText(Panel5, ID_STATICTEXT41, _("v Box"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
    FlexGridSizer5->Add(StaticText40, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    TextVboxX = new wxTextCtrl(Panel5, ID_TEXTCTRL12, _("0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL12"));
    FlexGridSizer5->Add(TextVboxX, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextVboxY = new wxTextCtrl(Panel5, ID_TEXTCTRL15, _("0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL15"));
    FlexGridSizer5->Add(TextVboxY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextVboxZ = new wxTextCtrl(Panel5, ID_TEXTCTRL16, _("0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL16"));
    FlexGridSizer5->Add(TextVboxZ, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextUnitVass = new wxStaticText(Panel5, ID_STATICTEXT20, _("To Inialize"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT20"));
    FlexGridSizer5->Add(StaticTextUnitVass, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText64 = new wxStaticText(Panel5, ID_STATICTEXT66, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT66"));
    FlexGridSizer5->Add(StaticText64, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PlotAssX = new wxButton(Panel5, ID_BUTTON11, _("X Assessment"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON11"));
    FlexGridSizer5->Add(PlotAssX, 1, wxALL|wxEXPAND, 5);
    PlotAssY = new wxButton(Panel5, ID_BUTTON12, _("Y Assessment"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON12"));
    FlexGridSizer5->Add(PlotAssY, 1, wxALL|wxEXPAND, 5);
    PlotAssZ = new wxButton(Panel5, ID_BUTTON13, _("Z Assessment"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON13"));
    FlexGridSizer5->Add(PlotAssZ, 1, wxALL|wxEXPAND, 5);
    Panel5->SetSizer(FlexGridSizer5);
    FlexGridSizer5->Fit(Panel5);
    FlexGridSizer5->SetSizeHints(Panel5);
    FlexGridSizer1->Add(Panel5, 1, wxALL|wxEXPAND, 5);
    Panel1->SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(Panel1);
    FlexGridSizer1->SetSizeHints(Panel1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem3 = new wxMenuItem(Menu1, idMenuHardCopy, _("Hardcopy"), _("Make a Harcopy"), wxITEM_NORMAL);
    Menu1->Append(MenuItem3);
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuItem4 = new wxMenuItem(Menu2, ID_MENUITEM1, _("Licence"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuItem4);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    MainStatusBar = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -1 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    MainStatusBar->SetFieldsCount(1,__wxStatusBarWidths_1);
    MainStatusBar->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(MainStatusBar);

    Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnSpinEDRnumChange);
    Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&EDR2Frame::OnFilePickerEDRFileChanged);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnOpenEDRClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnPlotEDRxClick);
    Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnPlotEDRyClick);
    Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnPlotEDRzClick);
    Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextScaleMMEx);
    Connect(ID_FILEPICKERCTRL2,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&EDR2Frame::OnFilePickerMMExFileChanged);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnOpenMMExClick);
    Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextScaleMMEy);
    Connect(ID_FILEPICKERCTRL3,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&EDR2Frame::OnFilePickerMMEyFileChanged);
    Connect(ID_BUTTON_OpenMMEy,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnOpenMMEyClick);
    Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextScaleMMEz);
    Connect(ID_FILEPICKERCTRL4,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&EDR2Frame::OnFilePickerMMEzFileChanged);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnOpenMMEzClick);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnPlotMMExClick);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnPlotMMEyClick);
    Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnPlotMMEzClick);
    Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextTmin);
    Connect(ID_TEXTCTRL5,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextTmax);
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EDR2Frame::OnUnitTSelect);
    Connect(ID_TEXTCTRL6,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextAmin);
    Connect(ID_TEXTCTRL7,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextAmax);
    Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EDR2Frame::OnUnitASelect);
    Connect(ID_TEXTCTRL8,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextVmin);
    Connect(ID_TEXTCTRL9,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextVmax);
    Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EDR2Frame::OnUnitVSelect);
    Connect(ID_TEXTCTRL10,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextToffset);
    Connect(ID_CHOICE5,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EDR2Frame::OnGrenzKurveSelect);
    Connect(ID_TEXTCTRL11,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextTboxX);
    Connect(ID_TEXTCTRL13,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextTboxY);
    Connect(ID_TEXTCTRL14,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextTboxZ);
    Connect(ID_TEXTCTRL12,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextVboxX);
    Connect(ID_TEXTCTRL15,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextVboxY);
    Connect(ID_TEXTCTRL16,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EDR2Frame::OnTextVboxZ);
    Connect(ID_BUTTON11,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnPlotAssXClick);
    Connect(ID_BUTTON12,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnPlotAssYClick);
    Connect(ID_BUTTON13,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EDR2Frame::OnPlotAssZClick);
    Connect(idMenuHardCopy,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&EDR2Frame::OnMenuHardcopySelected);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&EDR2Frame::OnQuit);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&EDR2Frame::OnAbout);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&EDR2Frame::OnLicence);
    //*)

// ----------------   My own GUI initializations ----------------------

    edrinput_.iGrenzkurve= GrenzKurve->GetSelection() +1; // Defaults defined in EDR2frame.wxs
    StaticTextUnitRF->SetLabel (ValUnitRFlabel (edrinput_.iGrenzkurve));
    TextCtrlRange->SetValue (" ");
    TextCtrlFreq->SetValue (" ");

    strcpy ((char*)edrinput_.ASSunitT, UnitT->GetString(UnitT->GetSelection()) ); //...
    strcpy ((char*)edrinput_.ASSunitV, UnitV->GetString(UnitV->GetSelection()) ); //...
    strcpy ((char*)edrinput_.ASSunitA, UnitA->GetString(UnitA->GetSelection()) ); //...

    StaticTextUnitTass2->SetLabel((char*)edrinput_.ASSunitT);
    StaticTextUnitTass->SetLabel((char*)edrinput_.ASSunitT);
    StaticTextUnitVass->SetLabel((char*)edrinput_.ASSunitV);

    TextTmin->SetValue ("1");  // calls the eventhandler ...
    TextTmax->SetValue ("-1"); // ... and sets edrinput_.plotXXX
    TextAmin->SetValue ("1"); TextAmax->SetValue ("-1"); // min>max ->...
    TextVmin->SetValue ("1"); TextVmax->SetValue ("-1"); // ...plot autoscale

    TextToffset->SetValue ("0");


// ---------------- End of my initializations ---------------------

}

EDR2Frame::~EDR2Frame()
{
    //(*Destroy(EDR2Frame)
    //*)
}


/* Action routines for Menu Items */

void EDR2Frame::OnMenuHardcopySelected(wxCommandEvent& event)
{
  static int TCS_ID=3; // Hardcopy of the Assessmentplot
  bool TCSnotReady;
    TCSnotReady= WINSELECT (&TCS_ID);
    if (TCSnotReady) {
      return;
    }
    HDCOPY();
}

void EDR2Frame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void EDR2Frame::OnAbout(wxCommandEvent& event)
{
using namespace AutoVersion;
int tcsver[3], ag2ver[3];
wxString tmp1,tmp2, msg;

    tmp1.sprintf (_("Program Version: %d.%d%s Build %d\n"), MAJOR, MINOR, STATUS_SHORT, BUILD);
    TCSLEV (tcsver); AG2LEV(ag2ver);
    tmp2.sprintf (_("AG2: %d-%d / TCS: %d-%d\n"),ag2ver[0],ag2ver[1],tcsver[0],tcsver[1]);
    msg = (wxString) "\n" +
                     _("EDR2 can be used to compare the data of an automotive Event Data Recorder with the corresponding data obtained by full scale crash tests.")
                     + "\n\n" + tmp1 +
                     _("GCC Version: ") + __VERSION__ + "\n" +
                     tmp2 + wxbuildinfo(long_f) + "\n\n" +
                     _("Website: ") + EDR2WWW;
    wxMessageBox(msg, _("Welcome to EDR2"));
}

void EDR2Frame::OnLicence(wxCommandEvent& event)
{
wxString tmp1,tmp2, msg;

    msg = (wxString) "\n" +
                     _("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation;")
                     + _("either version 3 of the License, or at your option any later version.")
                     + "\n\n" +
                     _("This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.")
                     + "\n\n" +
                     _("You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.");
    wxMessageBox(msg, _("GNU General Public Licence"));
}



/* Action routines for reading ISO-MME files */

void EDR2Frame::OnFilePickerMMExFileChanged(wxFileDirPickerEvent& event)
{
    OpenMMEx->Enable(true);
}

void EDR2Frame::OnFilePickerMMEyFileChanged(wxFileDirPickerEvent& event)
{
    OpenMMEy->Enable(true);
}

void EDR2Frame::OnFilePickerMMEzFileChanged(wxFileDirPickerEvent& event)
{
    OpenMMEz->Enable(true);
}

void EDR2Frame::OnOpenMMExClick(wxCommandEvent& event)
{
double Scale;
wxString Str;
    Str= TextScaleMMEx->GetValue();
    if (!Str.ToCDouble (&Scale)) { // Erroneous Number String
      Scale= 1.0;
      TextScaleMMEx->SetValue ("1.0 ???");
    }
    ReadCrashData (FilePickerMMEx->GetPath().mb_str(), Scale, (float*)edrinput_.txMME, (float*)edrinput_.axMME);
    PlotMMEx->Enable(true);
}

void EDR2Frame::OnOpenMMEyClick(wxCommandEvent& event)
{
double Scale;
wxString Str;
    Str= TextScaleMMEy->GetValue();
    if (!Str.ToCDouble (&Scale)) { // Erroneous Number String
      Scale= 1.0;
      TextScaleMMEy->SetValue ("1.0 ???");
    }
    ReadCrashData (FilePickerMMEy->GetPath().mb_str(), Scale, (float*)edrinput_.tyMME, (float*)edrinput_.ayMME);
    PlotMMEy->Enable(true);
}

void EDR2Frame::OnOpenMMEzClick(wxCommandEvent& event)
{
double Scale;
wxString Str;
    Str= TextScaleMMEz->GetValue();
    if (!Str.ToCDouble (&Scale)) { // Erroneous Number String
      Scale= 1.0;
      TextScaleMMEz->SetValue ("1.0 ???");
    }
    ReadCrashData (FilePickerMMEz->GetPath().mb_str(), Scale, (float*)edrinput_.tzMME, (float*)edrinput_.azMME);
    PlotMMEz->Enable(true);
}

void EDR2Frame::OnPlotMMExClick(wxCommandEvent& event)
{
  static int TCS_ID=2; // ID of the plotwindow
  bool TCSnotReady;
    PlotMMEx->Enable(false); // prevent reentrance into plot routine
    TCSnotReady= winselect_ (&TCS_ID);
    if (TCSnotReady) {
      initt1 (3, this, (wxFrame*) MMEplotPanel, MainStatusBar);
    }

    MMEplotRaw (edrinput_.txMME, edrinput_.axMME, MMEplotXtitle);
    PlotMMEx->Enable (true); // a clean second call of the plot routine is o.k.
}

void EDR2Frame::OnPlotMMEyClick(wxCommandEvent& event)
{
  static int TCS_ID=2; // ID of the plotwindow
  bool TCSnotReady;
    PlotMMEy->Enable(false); // prevent reentrance into plot routine
    TCSnotReady= winselect_ (&TCS_ID);
    if (TCSnotReady) {
      initt1 (3, this, (wxFrame*) MMEplotPanel, MainStatusBar);
    }
    MMEplotRaw (edrinput_.tyMME, edrinput_.ayMME, MMEplotYtitle);
    PlotMMEy->Enable (true); // a clean second call of the plot routine is o.k.
}

void EDR2Frame::OnPlotMMEzClick(wxCommandEvent& event)
{
  static int TCS_ID=2; // ID of the plotwindow
  bool TCSnotReady;
    PlotMMEz->Enable(false); // prevent reentrance into plot routine
    TCSnotReady= winselect_ (&TCS_ID);
    if (TCSnotReady) {
      initt1 (3, this, (wxFrame*) MMEplotPanel, MainStatusBar);
    }
    MMEplotRaw (edrinput_.tzMME, edrinput_.azMME, MMEplotZtitle);
    PlotMMEz->Enable (true); // a clean second call of the plot routine is o.k.
}

void EDR2Frame::OnTextScaleMMEx(wxCommandEvent& event)
{
    OpenMMEx->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
}

void EDR2Frame::OnTextScaleMMEy(wxCommandEvent& event)
{
    OpenMMEy->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
}

void EDR2Frame::OnTextScaleMMEz(wxCommandEvent& event)
{
    OpenMMEz->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
}


/* Action routines for reading EDR files */


void EDR2Frame::OnSpinEDRnumChange(wxSpinEvent& event)
{
    OpenEDR->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
}

void EDR2Frame::OnFilePickerEDRFileChanged(wxFileDirPickerEvent& event)
{
    OpenEDR->Enable(true);
}

void EDR2Frame::OnOpenEDRClick(wxCommandEvent& event)
{
    edrinput_.nEDRrec= SpinEDRnum->GetValue();

    edrinput_.vBoxTx= 0.; edrinput_.vBoxTy= 0.; edrinput_.vBoxTz= 0.; // get first Assessmentbox...
    edrinput_.vBoxVx= 0.; edrinput_.vBoxVy= 0.; edrinput_.vBoxVz= 0.; // ...from EDR file

    ReadCrashData (FilePickerEDR->GetPath().mb_str(), 1.0, (float*)edrinput_.txEDR , (float*)edrinput_.vxEDR);

    edrinput_.vBoxTx*= fac2SI(edrinput_.EDRunitT); edrinput_.vBoxVx*= fac2SI(edrinput_.EDRunitV);
    edrinput_.vBoxTy*= fac2SI(edrinput_.EDRunitT); edrinput_.vBoxVy*= fac2SI(edrinput_.EDRunitV);
    edrinput_.vBoxTz*= fac2SI(edrinput_.EDRunitT); edrinput_.vBoxVz*= fac2SI(edrinput_.EDRunitV);

    char lab[20];
    sprintf(lab,"%f", edrinput_.vBoxTx/fac2SI(edrinput_.ASSunitT)); TextTboxX->SetLabel (lab);
    sprintf(lab,"%f", edrinput_.vBoxVx/fac2SI(edrinput_.ASSunitV)); TextVboxX->SetLabel (lab);
    sprintf(lab,"%f", edrinput_.vBoxTy/fac2SI(edrinput_.ASSunitT)); TextTboxY->SetLabel (lab);
    sprintf(lab,"%f", edrinput_.vBoxVy/fac2SI(edrinput_.ASSunitV)); TextVboxY->SetLabel (lab);
    sprintf(lab,"%f", edrinput_.vBoxTz/fac2SI(edrinput_.ASSunitT)); TextTboxZ->SetLabel (lab);
    sprintf(lab,"%f", edrinput_.vBoxVz/fac2SI(edrinput_.ASSunitV)); TextVboxZ->SetLabel (lab);

    PlotEDRx->Enable(true); PlotEDRy->Enable(true); PlotEDRz->Enable(true);
}

void EDR2Frame::OnPlotEDRxClick(wxCommandEvent& event)
{
  static int TCS_ID=1; // ID of the plotwindow
  bool TCSnotReady;
    PlotEDRx->Enable(false); // prevent reentrance into plot routine
    TCSnotReady= winselect_ (&TCS_ID);
    if (TCSnotReady) {
      initt1 (3, this, (wxFrame*) EDRplotPanel, MainStatusBar);
    }
    EDRplotAV (edrinput_.txEDR,edrinput_.vxEDR,edrinput_.axEDR, EDRplotXtitle);
    PlotEDRx->Enable (true); // a clean second call of the plot routine is o.k.
}

void EDR2Frame::OnPlotEDRyClick(wxCommandEvent& event)
{
  static int TCS_ID=1; // ID of the plotwindow
  bool TCSnotReady;
    PlotEDRy->Enable(false); // prevent reentrance into plot routine
    TCSnotReady= winselect_ (&TCS_ID);
    if (TCSnotReady) {
      initt1 (3, this, (wxFrame*) EDRplotPanel, MainStatusBar);
    }
    EDRplotAV (edrinput_.tyEDR,edrinput_.vyEDR,edrinput_.ayEDR, EDRplotYtitle);
    PlotEDRy->Enable (true); // a clean second call of the plot routine is o.k.
}

void EDR2Frame::OnPlotEDRzClick(wxCommandEvent& event)
{
  static int TCS_ID=1; // ID of the plotwindow
  bool TCSnotReady;
    PlotEDRz->Enable(false); // prevent reentrance into plot routine
    TCSnotReady= winselect_ (&TCS_ID);
    if (TCSnotReady) {
      initt1 (3, this, (wxFrame*) EDRplotPanel, MainStatusBar);
    }
    EDRplotAV (edrinput_.tzEDR,edrinput_.vzEDR,edrinput_.azEDR, EDRplotZtitle);
    PlotEDRz->Enable (true); // a clean second call of the plot routine is o.k.
}

// Action routines setting plot parameters

void EDR2Frame::OnTextTmin(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextTmin->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= 1.0;
    }
    edrinput_.plotTmin= NumIn* fac2SI (edrinput_.ASSunitT);
}

void EDR2Frame::OnTextTmax(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextTmax->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= -1.0;
    }
    edrinput_.plotTmax= NumIn* fac2SI (edrinput_.ASSunitT);
}

void EDR2Frame::OnTextAmin(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextAmin->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= 1.0;
    }
    edrinput_.plotAmin= NumIn* fac2SI (edrinput_.ASSunitA);
}

void EDR2Frame::OnTextAmax(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextAmax->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= -1.0;
    }
    edrinput_.plotAmax= NumIn* fac2SI (edrinput_.ASSunitA);
}

void EDR2Frame::OnTextVmin(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextVmin->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= 1.0;
    }
    edrinput_.plotVmin= NumIn* fac2SI (edrinput_.ASSunitV);
}

void EDR2Frame::OnTextVmax(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextVmax->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= -1.0;
    }
    edrinput_.plotVmax= NumIn* fac2SI (edrinput_.ASSunitV);
}

void EDR2Frame::OnUnitTSelect(wxCommandEvent& event)
{
    strcpy ((char*)edrinput_.ASSunitT, UnitT->GetString(UnitT->GetSelection()) );
    StaticTextUnitTass->SetLabel((char*)edrinput_.ASSunitT);   // update input mask
    StaticTextUnitTass2->SetLabel((char*)edrinput_.ASSunitT);

    char lab[20]; // convert input values to new units
    sprintf(lab,"%f", edrinput_.plotTmin/fac2SI(edrinput_.ASSunitT)); TextTmin->SetValue (lab);
    sprintf(lab,"%f", edrinput_.plotTmax /fac2SI(edrinput_.ASSunitT)); TextTmax ->SetValue (lab);
    sprintf(lab,"%f", edrinput_.tOffset /fac2SI(edrinput_.ASSunitT)); TextToffset ->SetValue (lab);
    sprintf(lab,"%f", edrinput_.vBoxTx/fac2SI(edrinput_.ASSunitT)); TextTboxX->SetValue (lab);
    sprintf(lab,"%f", edrinput_.vBoxTy/fac2SI(edrinput_.ASSunitT)); TextTboxY->SetValue (lab);
    sprintf(lab,"%f", edrinput_.vBoxTz/fac2SI(edrinput_.ASSunitT)); TextTboxZ->SetValue (lab);
}

void EDR2Frame::OnUnitASelect(wxCommandEvent& event)
{
    strcpy ((char*)edrinput_.ASSunitA, UnitA->GetString(UnitA->GetSelection()) );
    char lab[20]; // convert input values to new units
    sprintf(lab,"%f", edrinput_.plotAmin/fac2SI(edrinput_.ASSunitA)); TextAmin->SetValue (lab);
    sprintf(lab,"%f", edrinput_.plotAmax/fac2SI(edrinput_.ASSunitA)); TextAmax ->SetValue (lab);
}

void EDR2Frame::OnUnitVSelect(wxCommandEvent& event)
{
    strcpy ((char*)edrinput_.ASSunitV, UnitV->GetString(UnitV->GetSelection()) );
    StaticTextUnitVass->SetLabel((char*)edrinput_.ASSunitV);   // update input mask

    char lab[20]; // convert input values to new units
    sprintf(lab,"%f", edrinput_.plotVmin /fac2SI(edrinput_.ASSunitV)); TextVmin->SetValue (lab);
    sprintf(lab,"%f", edrinput_.plotVmax /fac2SI(edrinput_.ASSunitV)); TextVmax->SetValue (lab);
    sprintf(lab,"%f", edrinput_.vBoxVx/fac2SI(edrinput_.ASSunitV)); TextVboxX ->SetValue (lab);
    sprintf(lab,"%f", edrinput_.vBoxVy/fac2SI(edrinput_.ASSunitV)); TextVboxY->SetValue (lab);
    sprintf(lab,"%f", edrinput_.vBoxVz/fac2SI(edrinput_.ASSunitV)); TextVboxZ->SetValue (lab);
}

// Doing the assessment...

void EDR2Frame::OnGrenzKurveSelect(wxCommandEvent& event)
{
    edrinput_.iGrenzkurve= GrenzKurve->GetSelection() +1;
    StaticTextUnitRF->SetLabel (ValUnitRFlabel (edrinput_.iGrenzkurve));
    TextCtrlRange->SetValue (" ");
    TextCtrlFreq->SetValue (" ");
}

void EDR2Frame::OnTextToffset(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextToffset->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= 1.0;
    }
    edrinput_.tOffset= NumIn* fac2SI (edrinput_.ASSunitT);
}

void EDR2Frame::OnTextTboxX(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextTboxX->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= 0.0;
    }
    edrinput_.vBoxTx= NumIn* fac2SI (edrinput_.ASSunitT);
}

void EDR2Frame::OnTextTboxY(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextTboxY->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= 0.0;
    }
    edrinput_.vBoxTy= NumIn* fac2SI (edrinput_.ASSunitT);
}

void EDR2Frame::OnTextTboxZ(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextTboxZ->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= 0.0;
    }
    edrinput_.vBoxTz= NumIn* fac2SI (edrinput_.ASSunitT);
}

void EDR2Frame::OnTextVboxX(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextVboxX->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= 0.0;
    }
    edrinput_.vBoxVx= NumIn* fac2SI (edrinput_.ASSunitV);
}

void EDR2Frame::OnTextVboxY(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextVboxY->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= 0.0;
    }
    edrinput_.vBoxVy= NumIn* fac2SI (edrinput_.ASSunitV);
}

void EDR2Frame::OnTextVboxZ(wxCommandEvent& event)
{
double NumIn;
wxString StrIn;
    StrIn= TextVboxZ->GetValue();
    if (!StrIn.ToCDouble (&NumIn)) { // Erroneous Number String
      NumIn= 0.0;
    }
    edrinput_.vBoxVz= NumIn* fac2SI (edrinput_.ASSunitV);
}

void EDR2Frame::OnPlotAssXClick(wxCommandEvent& event)
{
  static int TCS_ID=3; // ID of the plotwindow
  bool TCSnotReady;
  float dy, CornerF;
  double tmpnum;
  static size_t emptylen=1;
    PlotAssX->Enable(false); // prevent reentrance into plot routine
    TCSnotReady= winselect_ (&TCS_ID);
    if (TCSnotReady) {
      initt1 (1, this, nullptr, nullptr);
    } else {
      STATST ((char*) " ",emptylen); // Clear messages from last plot
    }

    if (edrinput_.iGrenzkurve < ASS_A) {
      // Default USA and UN: SAE J1698-3 Delta=10 km/h and 150 Hz Butterworth filter
      if (!TextCtrlRange->GetValue().ToDouble(&tmpnum) ) TextCtrlRange->SetValue ("10.");
      if (!TextCtrlFreq->GetValue().ToDouble(&tmpnum) ) TextCtrlFreq->SetValue ("150.");

      TextCtrlRange->GetValue().ToDouble(&tmpnum);
      dy= tmpnum*fac2SI("km/h");
      TextCtrlFreq->GetValue().ToDouble(&tmpnum);
      CornerF=(float) tmpnum;
      ASSplot (edrinput_.txEDR, edrinput_.vxEDR, edrinput_.txMME, edrinput_.axMME,
              &dy, &CornerF, &edrinput_.vBoxTx,&edrinput_.vBoxVx, ASSplotXtitle);
    } else {
      // Default UN: Delta=5 g and 33 Hz Butterworth filter (CFC 20)
      if (!TextCtrlRange->GetValue().ToDouble(&tmpnum) ) TextCtrlRange->SetValue ("5.");
      if (!TextCtrlFreq->GetValue().ToDouble(&tmpnum) ) TextCtrlFreq->SetValue ("33.");

      TextCtrlRange->GetValue().ToDouble(&tmpnum);
      dy= tmpnum*fac2SI("g");
      TextCtrlFreq->GetValue().ToDouble(&tmpnum);
      CornerF=(float) tmpnum;
      ASSplotA (edrinput_.txEDR, edrinput_.axEDR, edrinput_.txMME, edrinput_.axMME,
               &dy, &CornerF, ASSplotXtitle);
    }
    PlotAssX->Enable(true);
}

void EDR2Frame::OnPlotAssYClick(wxCommandEvent& event)
{
  static int TCS_ID=3; // ID of the plotwindow
  bool TCSnotReady;
  float dy, CornerF;
  double tmpnum;
  static size_t emptylen=1;
    PlotAssY->Enable(false); // prevent reentrance into plot routine
    TCSnotReady= winselect_ (&TCS_ID);
    if (TCSnotReady) {
      initt1 (1, this, nullptr, nullptr);
    } else {
      STATST ((char*) " ",emptylen);
    }
    if (edrinput_.iGrenzkurve < ASS_A) {
      // Default USA and UN: SAE J1698-3 Delta=10 km/h and 150 Hz Butterworth filter
      if (!TextCtrlRange->GetValue().ToDouble(&tmpnum) ) TextCtrlRange->SetValue ("10.");
      if (!TextCtrlFreq->GetValue().ToDouble(&tmpnum) ) TextCtrlFreq->SetValue ("150.");

      TextCtrlRange->GetValue().ToDouble(&tmpnum);
      dy= tmpnum*fac2SI("km/h");
      TextCtrlFreq->GetValue().ToDouble(&tmpnum);
      CornerF=(float) tmpnum;
      ASSplot (edrinput_.tyEDR, edrinput_.vyEDR, edrinput_.tyMME, edrinput_.ayMME,
              &dy, &CornerF, &edrinput_.vBoxTy,&edrinput_.vBoxVy, ASSplotYtitle);
    } else {
      // Default UN: Delta=5 g and 33 Hz Butterworth filter (CFC 20)
      if (!TextCtrlRange->GetValue().ToDouble(&tmpnum) ) TextCtrlRange->SetValue ("5.");
      if (!TextCtrlFreq->GetValue().ToDouble(&tmpnum) ) TextCtrlFreq->SetValue ("33.");

      TextCtrlRange->GetValue().ToDouble(&tmpnum);
      dy= tmpnum*fac2SI("g");
      TextCtrlFreq->GetValue().ToDouble(&tmpnum);
      CornerF=(float) tmpnum;
      ASSplotA (edrinput_.tyEDR, edrinput_.ayEDR, edrinput_.tyMME, edrinput_.ayMME,
               &dy, &CornerF, ASSplotYtitle);
    }
    PlotAssY->Enable(true);
}

void EDR2Frame::OnPlotAssZClick(wxCommandEvent& event)
{
  static int TCS_ID=3; // ID of the plotwindow
  bool TCSnotReady;
  float dy, CornerF;
  double tmpnum;
  static size_t emptylen=1;

    PlotAssZ->Enable(false); // prevent reentrance into plot routine
    TCSnotReady= winselect_ (&TCS_ID);
    if (TCSnotReady) {
      initt1 (1, this, nullptr, nullptr);
    } else {
      STATST ((char*) " ",emptylen);
    }
    if (edrinput_.iGrenzkurve <ASS_A) {
      // Default USA and UN: SAE J1698-3 Delta=10 km/h and 150 Hz Butterworth filter
      if (!TextCtrlRange->GetValue().ToDouble(&tmpnum) ) TextCtrlRange->SetValue ("10.");
      if (!TextCtrlFreq->GetValue().ToDouble(&tmpnum) ) TextCtrlFreq->SetValue ("150.");

      TextCtrlRange->GetValue().ToDouble(&tmpnum);
      dy= tmpnum*fac2SI("km/h");
      TextCtrlFreq->GetValue().ToDouble(&tmpnum);
      CornerF=(float) tmpnum;
      ASSplot (edrinput_.tzEDR, edrinput_.vzEDR, edrinput_.tzMME, edrinput_.azMME,
               &dy, &CornerF, &edrinput_.vBoxTz,&edrinput_.vBoxVz, ASSplotZtitle);
    } else {
      // Default UN: Delta=0.5 g and 33 Hz Butterworth filter (CFC 20)
      if (!TextCtrlRange->GetValue().ToDouble(&tmpnum) ) TextCtrlRange->SetValue ("0.5");
      if (!TextCtrlFreq->GetValue().ToDouble(&tmpnum) ) TextCtrlFreq->SetValue ("33.");

      TextCtrlRange->GetValue().ToDouble(&tmpnum);
      dy= tmpnum*fac2SI("g");
      TextCtrlFreq->GetValue().ToDouble(&tmpnum);
      CornerF=(float) tmpnum;
      ASSplotA (edrinput_.tzEDR, edrinput_.azEDR, edrinput_.tzMME, edrinput_.azMME,
               &dy, &CornerF, ASSplotZtitle);
    }
    PlotAssZ->Enable(true);
}
