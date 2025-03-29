! CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
!
! Create diagrams to
!   - check input files
!   - compare EDR-measurements with crashdata
!
! CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC



subroutine EDRplotAV (t,v,a, title)  bind (C, name='EDRplotAV')
    use Graph2D
    include 'EDRinput.fd' ! Common Block EDRinput
    real, intent(in) :: t(iLe), v(iLe),  a(iLe)
    character (kind=c_char), dimension(*), intent(in) :: title

    interface
      function fac2SI (UsedUnit) bind (C, name='fac2SI')
        use, intrinsic                        :: iso_c_binding
        character(kind= c_char), dimension(*) :: UsedUnit
        real (kind=c_float)                   :: fac2SI
      end function fac2SI

      subroutine ftn_gettext (instr, outstr, lstr) bind (C, name='ftn_gettext')
        use, intrinsic                        :: iso_c_binding
        character(kind= c_char), dimension(*) :: instr, outstr
        integer (kind= c_int)                 :: lstr
      end subroutine ftn_gettext
    end interface

    integer i
    real NoData(2)
    character*(lablen) ftnstr
    call statst (" "//char(0)) ! Clear Status Line
    if (t(1).lt.2) then
      call bell ()
      call ftn_gettext ("No time data"//char(0), ftnstr, len(ftnstr))
      call statst (ftnstr)
      return
    endif

    call initt(0)
    call binitt
    call slimx (100,900)


! plot velocity

    if (plotTmin.lt.plotTmax) call dlimx (plotTmin/fac2SI(EDRunitT), plotTmax/fac2SI(EDRunitT))
    if (plotVmin.lt.plotVmax) call dlimy (plotVmin/fac2SI(EDRunitA), plotVmax/fac2SI(EDRunitA))
    if (v(1).lt. 2) call dlimy (-1.,1.)

    call check(t,v)
    if (v(1) .ge. 2) then
      call dsplay(t,v)
    else
      NoData(1)= 1.
      NoData(2)= ag2infin() ! AG2 definition for invalid data
      call dsplay(NoData,NoData) ! plot grid without curve
    end if
    call dblsiz
    do i=1,lablen
      ftnstr(i:i)=title(i) ! convert C-string to Fortran (Fortran char 1 or 4 Byte)
      if (title(i) == c_null_char) exit
    end do
    i= i-1 ! drop char(0)
    if (i.gt.0) call notatec (100, 720, ftnstr (1:i))
    i= iStringLen(EDRunitT)
    if (i.gt.0)  call notatec (500, 50, EDRunitT(1:i))
    call movabs (20,600)
    i= iStringLen(EDRunitV)
    if (i.gt.0) call vlablc (EDRunitV(1:i))
    call nrmsiz

! plot acceleration

    if (a(1) .ge. 2.) then
      call yloc (800) ! move axis left
      call yfrm (4)   ! major tickmark to right
      call xlab (0)
      call xfrm (0)   ! prevent double marking of the axis
      call lincol(2)
      call line (3)   ! dash
      call symbl(6)
      call txtcol(2)
      call dinity
      if (plotAmin.lt.plotAmax) call dlimy (plotAmin/fac2SI(EDRunitA), plotAmax/fac2SI(EDRunitA))
      call check(t,a)
      call dsplay(t,a)
      call dblsiz
      call movabs (980,600)
      i= iStringLen(EDRunitA)
      if (i.gt.0) call vlablc (EDRunitA(1:i))
      call nrmsiz
    endif

    call iowait (0)   ! show the chart
    return
end subroutine EDRplotAV



subroutine MMEplotRaw (tMME, yMME, title ) bind (C, name='MMEplotRaw')
    use Graph2D
    include 'EDRinput.fd' ! Common Block EDRinput
    real, intent(in) :: tMME(iLi), yMME(iLi)
    character (kind=c_char), dimension(*), intent(in) :: title

    interface
      function fac2SI (UsedUnit) bind (C, name='fac2SI')
        use, intrinsic                        :: iso_c_binding
        character(kind= c_char), dimension(*) :: UsedUnit
        real (kind=c_float)                   :: fac2SI
      end function fac2SI

      subroutine ftn_gettext (instr, outstr, lstr) bind (C, name='ftn_gettext')
        use, intrinsic                        :: iso_c_binding
        character(kind= c_char), dimension(*) :: instr, outstr
        integer (kind= c_int)                 :: lstr
      end subroutine ftn_gettext
    end interface

    real tMMEfil(iLi),yMMEfil(iLi)
    real dt, frequ            ! workspace for the Butterworth filter
    real acof(3,1), bcof(3,1) ! ...

    integer i
    character*(lablen) ftnstr

    call statst (" ") ! Clear Status Line
    if (tMME(1) .lt. 2.) then
      call bell()
      call ftn_gettext ("No time data"//char(0), ftnstr, len(ftnstr))
      call statst (ftnstr)
      return
    end if

    i= nint(tMME(1))
    tMMEfil(1)= i ! use SI-units for filtering
    yMMEfil(1)= i
    tMMEfil(2:i+1)= tMME(2:i+1) * fac2SI(MMEunitT)
    yMMEfil(2:i+1)= yMME(2:i+1) * fac2SI(MMEunitA)
    dt= tMMEfil(4)-tMMEfil(3) ! stable timeinterval in s
    frequ= 1/dt ! Unit: Hz
    call design_butter (frequ, 150., 1, acof, bcof)   ! SAE J1698/3:
    call filter_2nd_order (yMMEfil(2),i,acof,bcof)    ! 2.Ordnung Butterworth 150 Hz

    tMMEfil(2:i+1)= tMMEfil(2:i+1) / fac2SI(MMEunitT) ! convert result to MME-units
    yMMEfil(2:i+1)= yMMEfil(2:i+1) / fac2SI(MMEunitA)

    call initt (0)
    call binitt
    call slimx (100,900)
    if (plotTmin.lt.plotTmax) call dlimx (plotTmin/fac2SI(MMEunitT),plotTmax/fac2SI(MMEunitT))
    if (plotAmin.lt.plotAmax) call dlimy (plotAmin/fac2SI(MMEunitA),plotAmax/fac2SI(MMEunitA))

    call check(tMME,yMME)
    call dsplay(tMME,yMME)

    call lincol(2)
    call cplot (tMMEfil, yMMEfil)
    call DefaultColour

    call dblsiz
    do i=1,lablen
      ftnstr(i:i)=title(i) ! convert c to ftn string
      if (title(i) == c_null_char) exit
    end do
    i= i-1 ! drop char(0)
    if (i.gt.0) call notatec (100, 720, ftnstr(1:i))

    i= iStringLen(MMEunitT)
    if (i.gt.0) call notatec (500, 50, MMEunitT(1:i))
    call movabs (20,600)
    i= iStringLen(MMEunitA)
    if (i.gt.0) call vlablc (MMEunitA(1:i))
    call nrmsiz ! prepare further calling of the subroutine

    call iowait (0) ! show the diagram
    return
end subroutine MMEplotRAW



subroutine ASSplot (tEDR, vEDR, tMME, aMME, dv,CornerF, vBoxT, vBoxV, title) bind (C, name='ASSplot')
    use Graph2D
    include 'EDRinput.fd' ! Common Block EDRinput, also IMPLICIT NONE

    real, intent(in) :: tEDR(iLi), vEDR(iLi)
    real, intent(in) :: tMME(iLi), aMME(iLi)
    real, intent(in) :: dv, CornerF
    real, intent(in) :: vBoxT, vBoxV
    character (kind=c_char), dimension(*), intent(in) :: title

    interface
      subroutine MMEcalcV (tIn,yIn, LowBound,UpBound, tOut,yOut)
        real, intent(in)  :: tIn(:),yIn(:)
        real, intent(in)  :: LowBound, UpBound
        real, intent(out) :: tOut(:),yOut(:)
      end subroutine

      function fac2SI (UsedUnit) bind (C, name='fac2SI')
        use, intrinsic                        :: iso_c_binding
        character(kind= c_char), dimension(*) :: UsedUnit
        real (kind=c_float)                   :: fac2SI
      end function fac2SI

      subroutine ftn_gettext (instr, outstr, lstr) bind (C, name='ftn_gettext')
        use, intrinsic                        :: iso_c_binding
        character(kind= c_char), dimension(*) :: instr, outstr
        integer (kind= c_int)                 :: lstr
      end subroutine ftn_gettext


    end interface

    real boxx, boxy  ! Acceptance window as defined in SAE-J1698
    parameter (boxx = 0.006, boxy = 20./3.6)
    real boxh, boxv

    real tIlow, tIup

    integer i
    real plotTminScale, plotTmaxScale
    real plotVminScale, plotVmaxScale
    real xMMEtemp(iLi), yMMEtemp (iLi)
    real tEDRscale(iLe), vEDRscale (iLe) ! Scale = Assesment Plot Units
    real tMMEvScale(iLi), vMMEvScale (iLi)
    real tMMEvUscale(iLi), vMMEvUscale (iLi), tMMEvLscale(iLi), vMMEvLscale (iLi) ! in AssUnits
    character*(lablen) ftnstr, ftnstr1

    real dt, frequ            ! workspace for the Butterworth filter
    real acof(3,1), bcof(3,1) ! ...
    real AdjustEndY

    call statst (" ") ! Clear Status Line
    if ((tEDR(1).lt.2).or.(vEDR(1).lt.2)) then
      call bell()
      call ftn_gettext ("No accident EDR data"//char(0), ftnstr, len(ftnstr))
      call statst (ftnstr)
      return
    end if
    if (tMME(1).lt.2) then
      call bell ()
      call ftn_gettext ("No crash MME data"//char(0), ftnstr, len(ftnstr))
      call statst (ftnstr)
      return
    end if

!
! Integrate MME acceleration to velocity
!
    tIlow = 0.  ! t=0: time of first contact
    tIup  = .3  ! Definition from SAE J1698

    call MMEcalcV (tMME,aMME, tIlow/fac2SI(MMEunitT),tIup/fac2SI(MMEunitT), xMMEtemp,yMMEtemp)      ! Integration in ISO-MME units (g*sec possible !)
    i= nint(xMMEtemp(1)) ! Number of points in integration interval
    xMMEtemp (2:i+1)= xMMEtemp(2:i+1) * fac2SI(MMEunitT) ! now in sec
    yMMEtemp (2:i+1)= yMMEtemp(2:i+1) * fac2SI(MMEunitA)*fac2SI(MMEunitT) ! and m/sec

    tMMEvScale(1)= i
    tMMEvScale(2:i+1)= xMMEtemp(2:i+1)/fac2SI(ASSunitT) ! scaled to plotunits
    vMMEvScale(1)= i
    vMMEvScale(2:i+1)= yMMEtemp(2:i+1)/fac2SI(ASSunitV)
!
! Upper limit yMMEvUscale: + 10 km/h in Assesment-Plot units
!
    tMMEvUscale= tMMEvScale
    vMMEvUscale(1)= vMMEvScale(1)
    vMMEvUscale(2:i+1)= vMMEvScale(2:i+1) + (dv/fac2SI(ASSunitV))

    if (iGrenzkurve.le.2) then
!
! -- iGrenzkurve=1/2: lower limit according to SAE-J1698
!                     yMMEvLscale: Butterworth-filter and - 10km/h
!
      i= nint(tMME(1))
      xMMEtemp(1)= i
      xMMEtemp(2:i+1)= tMME(2:i+1) * fac2SI(MMEunitT)
      yMMEtemp(1)=i
      yMMEtemp(2:i+1)= aMME(2:i+1) * fac2SI(MMEunitA)
      dt= xMMEtemp(4)-xMMEtemp(3) ! stable time intervall in s
      frequ= 1/dt ! sample rate in Hz
      call design_butter (frequ, CornerF, 1, acof, bcof)
      call filter_2nd_order (yMMEtemp(2),i,acof,bcof)

      call MMEcalcV (xMMEtemp,yMMEtemp, tIlow, tIup, tMMEvLscale,vMMEvLscale) ! in SI Units
      i= nint(tMMEvLscale(1))
      tMMEvLscale(2:i+1)= tMMEvLscale(2:i+1) / fac2SI(ASSunitT)
      vMMEvLscale(1)= i
      vMMEvLscale(2:i+1)= (vMMEvLscale(2:i+1) - dv) / fac2SI(ASSunitV)

      if (iGrenzkurve.eq.2) then
! -- Model 2: speed difference at the end is the same as in the beginning
!    (otherwise the lower limit can be higher than the upper limit!)
        AdjustEndY= vMMEvScale(nint(vMMEvScale(1))) - vMMEvLscale(nint(vMMEvLscale(1))) &
                                    - (dv/fac2SI(ASSunitV))
        AdjustEndY= AdjustEndY / vMMEvLscale(1)
        do i=2, nint(vMMEvLscale(1)) +1
          vMMEvLscale(i) = vMMEvLscale(i) + i*AdjustEndY
        end do
      end if
    else
!
!  -- iGrenzkurve > 2: lower/upper limit according to TP 563 (offset)
!
! lower limit yMMEvLscale: - 10 km/h
!
      i= nint(tMMEvScale(1))
      tMMEvLscale(1)= i
      tMMEvLscale(2:i+1)= tMMEvScale(2:i+1)
      vMMEvLscale(1)= i
      vMMEvLscale(2:i+1)= vMMEvScale(2:i+1) - (dv/fac2SI(ASSunitV))
!
! -- all MME related curves are now calculated
!
    end if

    i= nint(tEDR(1))
    tEDRscale(1)= i ! consider unit and offset, based on airbag deployment
    tEDRscale(2:i+1)= tEDR(2:i+1)*fac2SI(EDRunitT)/fac2SI(ASSunitT)
    tEDRscale(2:i+1)= tEDRscale(2:i+1) + tOffset/fac2SI(ASSunitT)
    i= nint(vEDR(1))
    vEDRscale(1)= i
    vEDRscale(2:i+1)= vEDR(2:i+1)*fac2SI(EDRunitV)/fac2SI(ASSunitV)

    call initt (0)
    call binitt
    call slimx (100,900)

    if (plotTmin.ge.plotTmax) then
      plotTminScale= tIlow / fac2SI(ASSunitT)    ! Default: integration limits
      plotTmaxScale= tIup / fac2SI(ASSunitT)
    else
      plotTminScale= plotTmin / fac2SI(ASSunitT) ! plotTmin in SI units
      plotTmaxScale= plotTmax / fac2SI(ASSunitT)
    end if

    if (plotVmin.ge.plotVmax) then
      plotVminScale=  ag2infin ()
      plotVmaxScale= -plotVminScale
    else
      plotVminScale= plotVmin / fac2SI(ASSunitV)
      plotVmaxScale= plotVmax / fac2SI(ASSunitV)
    end if
    call MNMX (vMMEvUscale, plotVminScale, plotVmaxScale)
    call MNMX (vMMEvLscale, plotVminScale, plotVmaxScale)


    call dlimx (plotTminScale,plotTmaxScale)
    call dlimy (plotVminScale,plotVmaxScale)
    call check (tMMEvScale,vMMEvScale)

    call dsplay (tMMEvLscale,vMMEvLscale)
    call cplot (tMMEvUscale,vMMEvUscale)
    call lincol (iColorMME)

    call cplot(tMMEvScale,vMMEvScale)

! plot acceptance window at maximum delta-V (EDR)

    call lincol (iColorBOX)
    boxh= boxx / fac2SI(ASSunitT)
    boxv= boxy / fac2SI(ASSunitV)
    call movea ((vBoxT + tOffset)/fac2SI(ASSunitT), vBoxV/fac2SI(ASSunitV))
    call mover (.5* boxh, .5* boxv)
    call drawr (-boxh, 0.)
    call drawr (0., -boxv)
    call drawr (boxh, 0.)
    call drawr (0., boxv)
    call DefaultColour ! reset lincol(2) auf xml-konfigurierten Wert

    call symbl(iSymbolEDR)
    call line (iLineTypeEDR)
    call cplot(tEDRscale, vEDRscale)

    call dblsiz

    write (ftnstr1, fmt='(1x,f5.1)', iostat=i) dv/fac2SI('km/h'//c_null_char)
    ftnstr1= ftnstr1(1:6) // ' km/h' ! limit always in km/h
    do i=1,lablen
      ftnstr(i:i)=title(i) ! convert c to ftn string
      if (title(i) == c_null_char) exit
    end do
    ftnstr= ftnstr(1:i-1) // ftnstr1(1:11)
    if (i.gt.0) call notatec (100, 720, ftnstr)

    call notatec (500, 50, ASSunitT)
    call movabs (20,600)
    call vlablc (ASSunitV)
    call nrmsiz

    call iowait(0) ! draw chart
    return
end subroutine ASSplot



subroutine MMEcalcV (tIn,yIn, LowBound,UpBound, tOut,yOut)
    implicit none
    real, intent(in)  :: tIn(:),yIn(:)
    real, intent(in)  :: LowBound, UpBound
    real, intent(out) :: tOut(:),yOut(:)

    integer i, iData
    real dt

    dt= tIn(4)-tIn(3) ! timestep
    idata= 2
    yOut(1)= 0. ! starting value only for the integration, will be overwritten
    do i=2, int(tIn(1)) +1
      tOut(iData)= tIn(i)
      if ((tOut(iData).ge.LowBound).and.(tOut(iData).le.UpBound)) then
        yOut(iData)= yOut(iData-1)+ yIn(i)*dt
        iData= iData +1
      endif
    enddo
    tOut(1)= iData-2
    yOut(1)= tOut(1) ! Finally a Teklong array again

    return
end subroutine MMEcalcV



subroutine ASSplotA (tEDR, aEDR, tMME, aMME, da,CornerF, title) bind (C, name='ASSplotA')
    use Graph2D
    include 'EDRinput.fd' ! Common Block EDRinput
    real, intent(in) :: tEDR(iLi), aEDR(iLi)
    real, intent(in) :: tMME(iLi), aMME(iLi)
    real, intent(in) :: da, CornerF
    character (kind=c_char), dimension(*), intent(in) :: title

    interface
      function fac2SI (UsedUnit) bind (C, name='fac2SI')
        use, intrinsic                        :: iso_c_binding
        character(kind= c_char), dimension(*) :: UsedUnit
        real (kind=c_float)                   :: fac2SI
      end function fac2SI

      subroutine ftn_gettext (instr, outstr, lstr) bind (C, name='ftn_gettext')
        use, intrinsic                        :: iso_c_binding
        character(kind= c_char), dimension(*) :: instr, outstr
        integer (kind= c_int)                 :: lstr
      end subroutine ftn_gettext
    end interface

    real tMMEfil(iLi),aMMEfil(iLi)
    real dt, frequ            ! workspace for the Butterworth filter
    real acof(3,1), bcof(3,1) ! ...

    integer i
    real plotAminScale, plotAmaxScale ! +
    real tEDRscale(iLe), aEDRscale (iLe) ! Scale = Assesment Plot Units
    real tMMEaUscale(iLi), aMMEaUscale (iLi), tMMEaLscale(iLi), aMMEaLscale (iLi) ! + in AssUnits
    character*(lablen) ftnstr, ftnstr1

    call statst (" ") ! Clear Status Line
    if ((tEDR(1).lt.2).or.(aEDR(1).lt.2)) then
      call bell()
      call ftn_gettext ("No accident EDR data"//char(0), ftnstr, len(ftnstr))
      call statst (ftnstr)
      return
    end if
    if (tMME(1) .lt. 2.) then
      call bell()
      call ftn_gettext ("No crash MME data"//char(0), ftnstr, len(ftnstr))
      call statst (ftnstr)
      return
    end if

    i= nint(tMME(1))
    tMMEfil(1)= i ! do filtering in SI-units
    aMMEfil(1)= i
    tMMEfil(2:i+1)= tMME(2:i+1) * fac2SI(MMEunitT)
    aMMEfil(2:i+1)= aMME(2:i+1) * fac2SI(MMEunitA)
    dt= tMMEfil(4)-tMMEfil(3) ! stable timeintervall in s
    frequ= 1/dt ! sample rate in Hz
    call design_butter (frequ, CornerF, 1, acof, bcof)! SAE J211: CornerF= CFC/.6
    call filter_2nd_order (aMMEfil(2),i,acof,bcof)    ! 2.Ordnung Butterworth

    tMMEfil(2:i+1)= tMMEfil(2:i+1) / fac2SI(ASSunitT) ! Ergebnis  in Assesment-Einheiten
    aMMEfil(2:i+1)= aMMEfil(2:i+1) / fac2SI(ASSunitA)

!
!  ------ Lower/upper limit according to UN-R160 by offset
!
    aMMEaLscale(1)= i
    aMMEaLscale(2:i+1)= aMMEfil(2:i+1) - (da/fac2SI(ASSunitA))
    aMMEaUscale(1)= i
    aMMEaUscale(2:i+1)= aMMEfil(2:i+1) + (da/fac2SI(ASSunitA))

    call initt (0)
    call binitt
    call slimx (100,900)

    if (plotTmin.lt.plotTmax) call dlimx (plotTmin/fac2SI(ASSunitT),plotTmax/fac2SI(ASSunitT))
    if (plotAmin.ge.plotAmax) then
      plotAminScale=  ag2infin ()
      plotAmaxScale= -plotAminScale
      call MNMX (aMMEaUscale, plotAminScale, plotAmaxScale)
      call MNMX (aMMEaLscale, plotAminScale, plotAmaxScale)
    else
      plotAminScale= plotAmin/fac2SI(ASSunitA)
      plotAmaxScale= plotAmax/fac2SI(ASSunitA)
    end if
    call dlimy (plotAminScale, plotAmaxScale)

    i= nint(tEDR(1))
    tEDRscale(1)= i
    tEDRscale(2:i+1)= (tEDR(2:i+1)*fac2SI(EDRunitT) + tOffset) / fac2SI(ASSunitT)
    i= nint(aEDR(1))
    aEDRscale(1)= i
    aEDRscale(2:i+1)= aEDR(2:i+1)*fac2SI(EDRunitA)/fac2SI(ASSunitA)

    call check(tMMEfil,aMMEfil)
    call dsplay (tMMEfil, aMMEaLscale)
    call cplot (tMMEfil, aMMEaUscale)
    call lincol(iColorMME)

    call cplot(tMMEfil,aMMEfil)

    call lincol(iColorBOX)
    call symbl(iSymbolEDR)
    call line (iLineTypeEDR)
    call cplot(tEDRscale, aEDRscale)

    call DefaultColour

    call dblsiz

    write (ftnstr1, fmt='(1x,f5.1)', iostat=i) da/fac2SI('g'//c_null_char) ! limit always in g
    ftnstr1= ftnstr1(1:6) // ' g'
    do i=1,lablen
      ftnstr(i:i)=title(i) ! convert c to ftn string
      if (title(i) == c_null_char) exit
    end do
    ftnstr= ftnstr(1:i-1) // ftnstr1(1:8)
    i= iTrimLen(ftnstr)
    call notatec (100, 720, ftnstr)

    call notatec (500, 50, ASSunitT)
    call movabs (20,600)
    call vlablc (ASSunitA)
    call nrmsiz ! prepare further calling of the subroutine

    call iowait (0) ! show the diagram

    return
end subroutine ASSplotA

