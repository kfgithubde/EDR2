/* FLEX input file: parse Bosch CDR-Retrieval and ISO-MME datafiles */

/* Get rid of libfl.a */
%option noyywrap
%option batch

%{
#include "ReadCrashData.h"

#include "EDRinput.hpp"

%}

%s RDMME
%s SYSRDMODE
%s RD2NUMMODE
%s RD3NUMMODE

%%

 /* Start state = (All States) for ISO-MME files */

^(?i:\"*"Test object number") {return MMEINTRO;}
^(?i:"Unit")[ \t]*":"* {return MMEUNIT;}
^(?i:"Sampling interval")[ \t]*":"* {return SAMPLINTVL;}
^(?i:"Time of first sample")[ \t]*":"* {return TFIRSTSAMP;}
^(?i:"Number of samples")[ \t]*":"* {return NUMSAMPL;}

 /* Start state = (All States) for EDR files */

^(?i:\"*"CDR FILE INFORMATION") {return EDRINTRO;}
^(?i:\"*"SYSTEM STATUS AT EVENT (RECORD") {return EDRSYSREC;}
^(?i:\"*"LONGITUDINAL CRASH PULSE (RECORD") {return EDRXREC;}
^(?i:\"*"LATERAL CRASH PULSE (RECORD") {return EDRYREC;}
^(?i:\"*"NORMAL ACCELERATION (RECORD") {return EDRZREC;}
(?i:\"*"(km/h)") {return EDRVUNIT;}
(?i:\"*"(g)") {return EDRAUNIT;}
^(?i:\"*"Time (msec)") {return RDWHAT;}

 /* Start state = <INITIAL> for all files */

<INITIAL>"["[0-9]+"]" {} // Skip [nn]_ in Bosch CDR V23
<INITIAL>[-+]?([0-9]*\.?[0-9]+|[0-9]+\.)([Ee][-+]?[0-9]+)?[,]? {return NUMBER;}

 /* Start states for MME files */

<RDMME>:.*$ {return MMETAIL;}
<RDMME>^[ \t]*[-+]?([0-9]*\.?[0-9]+|[0-9]+\.)([Ee][-+]?[0-9]+)? {return MMEVALUE;}

 /* Start states for EDR files */
<SYSRDMODE>^(?i:\"*"Maximum Delta-V, Longitudinal (km/h)")\"*[ \t]*","* {return MAXDELTAVX;}
<SYSRDMODE>^(?i:\"*"Time, Maximum Delta-V, Longitudinal (msec)")\"*[ \t]*","* {return TMAXDELTAVX;}
<SYSRDMODE>^(?i:\"*"Maximum Delta-V, Lateral (km/h)")\"*[ \t]*","* {return MAXDELTAVY;}
<SYSRDMODE>^(?i:\"*"Time, Maximum Delta-V, Lateral (msec)")\"*[ \t]*","* {return TMAXDELTAVY;}
<SYSRDMODE>[-+]?([0-9]*\.?[0-9]+|[0-9]+\.)([Ee][-+]?[0-9]+)?[,]? {return NUMBER;}
<SYSRDMODE>^[ \t]*\n {BEGIN(INITIAL);}

<RD2NUMMODE>[-+]?([0-9]*\.?[0-9]+|[0-9]+\.)([Ee][-+]?[0-9]+)?[,]? {return NUMBER2;}
<RD3NUMMODE>[-+]?([0-9]*\.?[0-9]+|[0-9]+\.)([Ee][-+]?[0-9]+)?[,]? {return NUMBER3;}


%%
void ErrorInInputfile ()
{
  printf ("!!! ReadCrashData: Parsing ERROR !!! \n");
}



void ReadCrashData (const char filnam[], float yFac, float xDatArr[], float yDatArr[])
{

enum {LONGITUDINAL, LATERAL, NORMAL} axis;
enum {VELOCITY, ACCELERATION} TypeDat2, TypeDat3;

static int nVal, nDropVal; // consider small DatArr's
int ncurrent;

int tok, tmptok1, tmptok2;
float d1, d2, d3;
char*  c;
char NumBuf[80];

char *TokenString; // pointer for substrings


  FILE *f= fopen (filnam,"r");
  if (!f) {
    ErrorInInputfile();
    // error handler
  }
  yyrestart (f);

  while (tok = yylex()) {
    switch (tok) {
      //
      // Actions for ISO-MME files
      //
      case MMEINTRO:
        BEGIN (INITIAL);
        strcpy ((char*)edrinput_.MMEunitT, "s"); // ISO default
        strcpy ((char*)edrinput_.MMEunitA, "m/(s*s)"); // can be overwritten
        strcpy ((char*)edrinput_.MMEunitV, "m/s"); // ISO default
        xDatArr[0]=  yDatArr[0] = 1.0;
        xDatArr[1]=  yDatArr[0]= 0.0;
        nVal= 1; // Don't overwrite first Element
        nDropVal= 0; // just to be sure
        break;
      case MMEUNIT:
        BEGIN (RDMME);
        yyless (yyleng-1); // return ":" to inputbuffer
        yylex(); // get unit string via MMETAIL
        TokenString= &yytext[1]; // drop ":"
        strcpy ((char*)edrinput_.MMEunitA,TokenString);
        BEGIN (INITIAL);
        break;
      case SAMPLINTVL:
        if (yylex() != NUMBER) ErrorInInputfile();
        edrinput_.dtMME= atof(yytext);
        xDatArr[1]-= edrinput_.dtMME;
        break;
      case TFIRSTSAMP:
        if (yylex() != NUMBER) ErrorInInputfile();
        edrinput_.tStartMME= atof(yytext);
        xDatArr[1]+= edrinput_.tStartMME;
        break;
      case NUMSAMPL:
        if (yylex() != NUMBER) ErrorInInputfile();
        edrinput_.nmme= atoi(yytext);
        nDropVal= edrinput_.nmme / iLi;
        BEGIN (RDMME);
        break;
      case MMEVALUE:
        nVal--;
        if (nVal < 0) {
          if (nDropVal>0) { // arithmetic mean
            yDatArr[( (int)xDatArr[0])] = yDatArr[( (int)xDatArr[0])] / (float)(nDropVal+1);
          }
          xDatArr[0]++;
          xDatArr[( (int)xDatArr[0])]= xDatArr[( (int)xDatArr[0])-1];
          yDatArr[( (int)xDatArr[0])]= 0.0;
          nVal= nDropVal;
        }
        xDatArr[( (int)xDatArr[0])] += edrinput_.dtMME;
        yDatArr[0]= xDatArr[0];
        yDatArr[( (int)yDatArr[0])] += atof(yytext)*yFac;
        break;
      case MMETAIL:
        printf ("---------> MMETAIL");
        break; // action for debugging reasons
      //
      // Actions for CDR files
      //
      case EDRINTRO:
        BEGIN (INITIAL);
        edrinput_.txEDR[0]= 0; edrinput_.vxEDR[0]= 0; edrinput_.axEDR[0]= 0;
        edrinput_.tyEDR[0]= 0; edrinput_.vyEDR[0]= 0; edrinput_.ayEDR[0]= 0;
        edrinput_.tzEDR[0]= 0; edrinput_.vzEDR[0]= 0; edrinput_.azEDR[0]= 0;
        break;
      case EDRSYSREC: // Paragraph SYSREADMODE
        printf ("---------> EDRSYSREC\n");
        if (yylex() != NUMBER) ErrorInInputfile();
        ncurrent= atoi(yytext);
        if (ncurrent == edrinput_.nEDRrec) {
          BEGIN (SYSRDMODE);
        }
        break;
      case TMAXDELTAVX:
        strcpy ((char*)edrinput_.EDRunitT, "ms"); // also used for matching!
        if (yylex() != NUMBER) ErrorInInputfile();
        edrinput_.vBoxTx= atof(yytext);
        printf ("---------> TMAXDELTAV= %f \n", atof (yytext));
        break;
      case MAXDELTAVX:
        strcpy ((char*)edrinput_.EDRunitV, "km/h"); // used for matching!
        if (yylex() != NUMBER) ErrorInInputfile();
        edrinput_.vBoxVx= atof(yytext);
        printf ("---------> MAXDELTAV= %f \n", atof (yytext));
        break;
      case TMAXDELTAVY:
        strcpy ((char*)edrinput_.EDRunitT, "ms"); // also used for matching!
        if (yylex() != NUMBER) ErrorInInputfile();
        edrinput_.vBoxTy= atof(yytext);
        printf ("---------> TMAXDELTAV= %f \n", atof (yytext));
        break;
      case MAXDELTAVY:
        strcpy ((char*)edrinput_.EDRunitV, "km/h"); // used for matching!
        if (yylex() != NUMBER) ErrorInInputfile();
        edrinput_.vBoxVy= atof(yytext);
        printf ("---------> MAXDELTAV= %f \n", atof (yytext));
        break;
      case EDRXREC: // Paragraph Crashpulse (RD?NUMMODE)
        printf ("---------> EDRXREC\n");
        BEGIN (INITIAL);
        if (yylex() != NUMBER) ErrorInInputfile();
        ncurrent= atoi(yytext);
        axis = LONGITUDINAL;
        if (ncurrent == edrinput_.nEDRrec) {
          edrinput_.txEDR[0]= 0; // in case of no EDRINTRO
        }
        break;
      case EDRYREC:
        printf ("---------> EDRYREC\n");
        BEGIN (INITIAL);
        if (yylex() != NUMBER) ErrorInInputfile();
        ncurrent= atoi(yytext);
        axis = LATERAL;
        if (ncurrent == edrinput_.nEDRrec) edrinput_.tyEDR[0]= 0;
        break;
      case EDRZREC:
        printf ("---------> EDRZREC\n");
        BEGIN (INITIAL);
        if (yylex() != NUMBER) ErrorInInputfile();
        ncurrent= atoi(yytext);
        axis = NORMAL;
        if (ncurrent == edrinput_.nEDRrec) edrinput_.tzEDR[0]= 0;
        break;
      case RDWHAT:
        strcpy ((char*)edrinput_.EDRunitT, "ms");   // used for defining RDWHAT
        strcpy ((char*)edrinput_.EDRunitA, "g");    // ...               EDRAUNIT
        strcpy ((char*)edrinput_.EDRunitV, "km/h"); // ...               EDRVUNIT
        BEGIN (INITIAL);
        if (ncurrent == edrinput_.nEDRrec) {
          tmptok1= yylex();
            if (tmptok1 == EDRVUNIT) {
              TypeDat2= VELOCITY;
            } else if (tmptok1 == EDRAUNIT) {
              TypeDat2= ACCELERATION;
            } else {
              break; // leave case RDWHAT (no interpretation of ROLL RATE etc.)
            }
          tmptok2= yylex();
          if (tmptok2 == EDRVUNIT) {
            TypeDat3= VELOCITY;
            BEGIN (RD3NUMMODE);
          } else if (tmptok2 == EDRAUNIT) {
            TypeDat3= ACCELERATION;
            BEGIN (RD3NUMMODE);
          } else {
            BEGIN (RD2NUMMODE);
            d1= atof(yytext); // first data point already read
            c= NumBuf+ sprintf(NumBuf, " %g ", d1);
            while (c > NumBuf) unput (*--c); // return data
          }
        }
        break;
      case NUMBER2:
        d1= atof (yytext);
        if (yylex() != NUMBER2) {
          BEGIN (INITIAL);
          c= NumBuf+ sprintf(NumBuf, " %g ", d1);
          while (c > NumBuf) unput (*--c); // return data
          break;
        }
        d2= atof (yytext);
        switch (axis) {
          case LONGITUDINAL:
            if ((int)edrinput_.txEDR[0] >= iLe) {fclose (f); return;} // avoid buffer overflow
            edrinput_.txEDR[0]++;
            edrinput_.txEDR[((int)edrinput_.txEDR[0])]= d1;
            switch (TypeDat2) {
              case VELOCITY:
                edrinput_.vxEDR[0]= edrinput_.txEDR[0];
                edrinput_.vxEDR[(int)edrinput_.vxEDR[0]]= d2;
                break;
              case ACCELERATION:
                edrinput_.axEDR[0]= edrinput_.txEDR[0];
                edrinput_.axEDR[(int)edrinput_.axEDR[0]]= d2;
                break;
            }
            break;
          case LATERAL:
            if ((int)edrinput_.tyEDR[0] >= iLe) {fclose (f); return;} // avoid buffer overflow
            edrinput_.tyEDR[0]++;
            edrinput_.tyEDR[(int)edrinput_.tyEDR[0]]= d1;
            switch (TypeDat2) {
              case VELOCITY:
                edrinput_.vyEDR[0]= edrinput_.tyEDR[0];
                edrinput_.vyEDR[(int)edrinput_.vyEDR[0]]= d2;
                break;
              case ACCELERATION:
                edrinput_.ayEDR[0]= edrinput_.tyEDR[0];
                edrinput_.ayEDR[(int)edrinput_.ayEDR[0]]= d2;
                break;
            }
            break;
          case NORMAL:
            if ((int)edrinput_.tzEDR[0] >= iLe) {fclose (f); return;} // avoid buffer overflow
            edrinput_.tzEDR[0]++;
            edrinput_.tzEDR[(int)edrinput_.tzEDR[0]]= d1;
            switch (TypeDat2) {
              case VELOCITY:
                edrinput_.vzEDR[0]= edrinput_.tzEDR[0];
                edrinput_.vzEDR[(int)edrinput_.vzEDR[0]]= d2;
                break;
              case ACCELERATION:
                edrinput_.azEDR[0]= edrinput_.tzEDR[0];
                edrinput_.azEDR[(int)edrinput_.azEDR[0]]= d2;
                break;
            }
            break;
        }
        break;
      case NUMBER3:
        d1= atof (yytext);
        if (yylex() != NUMBER3) {
          c= NumBuf+ sprintf(NumBuf, " %g ", d1);
          while (c > NumBuf) unput (*--c); // return data
          BEGIN (INITIAL);
        }
        d2= atof (yytext);
        yylex();
        d3= atof (yytext);

        switch (axis) {
          case LONGITUDINAL:
            if ((int)edrinput_.txEDR[0] >= iLe) {fclose (f); return;} // avoid buffer overflow
            edrinput_.txEDR[0]++;
            edrinput_.txEDR[((int)edrinput_.txEDR[0])]= d1;
            switch (TypeDat2) {
              case VELOCITY:
                edrinput_.vxEDR[0]= edrinput_.txEDR[0];
                edrinput_.vxEDR[(int)edrinput_.vxEDR[0]]= d2;
                break;
              case ACCELERATION:
                edrinput_.axEDR[0]= edrinput_.txEDR[0];
                edrinput_.axEDR[(int)edrinput_.axEDR[0]]= d2;
                break;
            }
            switch (TypeDat3) {
              case VELOCITY:
                edrinput_.vxEDR[0]= edrinput_.txEDR[0];
                edrinput_.vxEDR[(int)edrinput_.vxEDR[0]]= d3;
                break;
              case ACCELERATION:
                edrinput_.axEDR[0]= edrinput_.txEDR[0];
                edrinput_.axEDR[(int)edrinput_.axEDR[0]]= d3;
                break;
            }
            break;
          case LATERAL:
           if ((int)edrinput_.tyEDR[0] >= iLe) {fclose (f); return;} // avoid buffer overflow
           edrinput_.tyEDR[0]++;
            edrinput_.tyEDR[(int)edrinput_.tyEDR[0]]= d1;
            switch (TypeDat2) {
              case VELOCITY:
                edrinput_.vyEDR[0]= edrinput_.tyEDR[0];
                edrinput_.vyEDR[(int)edrinput_.vyEDR[0]]= d2;
                break;
              case ACCELERATION:
                edrinput_.ayEDR[0]= edrinput_.tyEDR[0];
                edrinput_.ayEDR[(int)edrinput_.ayEDR[0]]= d2;
                break;
            }
            switch (TypeDat3) {
              case VELOCITY:
                edrinput_.vyEDR[0]= edrinput_.tyEDR[0];
                edrinput_.vyEDR[(int)edrinput_.vyEDR[0]]= d3;
                break;
              case ACCELERATION:
                edrinput_.ayEDR[0]= edrinput_.tyEDR[0];
                edrinput_.ayEDR[(int)edrinput_.ayEDR[0]]= d3;
                break;
            }
            break;
          case NORMAL:
            if ((int)edrinput_.tzEDR[0] >= iLe) {fclose (f); return;} // avoid buffer overflow
            edrinput_.tzEDR[0]++;
            edrinput_.tzEDR[(int)edrinput_.tzEDR[0]]= d1;
            switch (TypeDat2) {
              case VELOCITY:
                edrinput_.vzEDR[0]= edrinput_.tzEDR[0];
                edrinput_.vzEDR[(int)edrinput_.vzEDR[0]]= d2;
                break;
              case ACCELERATION:
                edrinput_.azEDR[0]= edrinput_.tzEDR[0];
                edrinput_.azEDR[(int)edrinput_.azEDR[0]]= d2;
                break;
            }
            switch (TypeDat3) {
              case VELOCITY:
                edrinput_.vzEDR[0]= edrinput_.tzEDR[0];
                edrinput_.vzEDR[(int)edrinput_.vzEDR[0]]= d3;
                break;
              case ACCELERATION:
                edrinput_.azEDR[0]= edrinput_.tzEDR[0];
                edrinput_.azEDR[(int)edrinput_.azEDR[0]]= d3;
                break;
            }
            break;
        }
      //
      // General actions
      //
      case NUMBER:
        printf ("---------> NUMBER= %f \n", atof (yytext));
        break; // action for debugging reasons
      case DBGCALL:
        printf ("---------> DBGCALL\n");
        break; // action for debugging reasons
    }
  }

  fclose (f);
}

