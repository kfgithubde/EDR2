/***************************************************************
 * Name:      EDR2App.cpp
 * Purpose:   Code for Application Class
 * Author:    Klaus Friedewald ()
 * Created:   2023-06-28
 * Copyright: Klaus Friedewald ()
 * License:
 **************************************************************/

#include "EDR2App.h"

//(*AppHeaders
#include "EDR2Main.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(EDR2App);

bool EDR2App::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	EDR2Frame* Frame = new EDR2Frame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}
