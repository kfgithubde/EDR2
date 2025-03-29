/***************************************************************
 * Name:      EDR2App.h
 * Purpose:   Defines Application Class
 * Author:    Klaus Friedewald ()
 * Created:   2023-06-28
 * Copyright: Klaus Friedewald ()
 * License:
 **************************************************************/

#ifndef EDR2APP_H
#define EDR2APP_H

#include <wx/app.h>

class EDR2App : public wxApp
{
    public:
        virtual bool OnInit();
};

#endif // EDR2APP_H
