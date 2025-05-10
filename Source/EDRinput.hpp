/** ****************************************************************************
\file    EDRinput.hpp
\brief   Common Block EDRINPUT
\version 1.3
\author  Dr.-Ing. Klaus Friedewald
\~german
         C Header passend zu EDRinput.fd
\~english
         C header belonging to EDRinput.fd
 \~

***************************************************************************** */

#define iLi 20000 // Do not forget EDRinput.fd!
#define iLe 200
#define lablen 80

extern "C" {
  extern struct EDRINPUT {
    int
      nmme;
    float
      dtMME, tStartMME,
      txMME[iLi], tyMME[iLi], tzMME[iLi],
      axMME[iLi], ayMME[iLi], azMME[iLi];

    int
      nEDRrec;
    float
      txEDR[iLe], tyEDR[iLe], tzEDR[iLe],
      vxEDR[iLe], vyEDR[iLe], vzEDR[iLe],
      axEDR[iLe], ayEDR[iLe], azEDR[iLe];

    float
      plotTmin, plotTmax,
      plotVmin, plotVmax,
      plotAmin, plotAmax,
      tOffset,
      vBoxTx,vBoxVx, vBoxTy,vBoxVy, vBoxTz,vBoxVz;

    int
      iGrenzkurve,
      iColorMME,iColorBOX, iLineTypeEDR,iSymbolEDR;

    char
      MMEunitT[lablen], MMEunitA[lablen], MMEunitV[lablen],
      EDRunitT[lablen], EDRunitA[lablen], EDRunitV[lablen],
      ASSunitT[lablen], ASSunitA[lablen], ASSunitV[lablen];

  } edrinput_; // use gfortran FTN77 name mangling
}

