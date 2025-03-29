C Unterprogramme zur Filterberechnung aus USAARL Report 95-13
C

C --- subroutine design_butter (samhz,corner,nsect,acof,bcof) ---
C
C Subroutine to design low-pass Butterworth digital filters. The filter is 
C obtained by using the bilinear transformation to transform analog filter
C equations to digital domain. Filtering is accomplished by a cascade of
C second-order sections which are defined by the order of the filter.
C Implementation in the time-domain is recursive. Arguments are:
C
C Input:
C
C    samhz . . .    given sampling rate (Hz) of digital signal.
C 
C    corner . . .   given filter corner frequency (Hz) where the magnitude
C                   is -3 dB (half-power point).
C 
C    nsect . . .    given number of 2nd-order sections (pole-pairs). The 
C                   number of poles of the filter will be 2 x nsect.
C
C Output:
C 
C    acof . . .     coefficients (AO,Al,A2) of 2nd-order filter sections 
C    bcof . . .     coefficients (BO,Bl,B2) of 2nd-order filter sections 
C
C Implementation: 
C
C    Recursive filtering through each and-order section is performed by 
C    the difference equation:
C 
C Y(n) = A0 * X(n) + Al * X(n-1) + A2 * X(n-2) -Bl Y(n-1) -B2 * Y(n-2)
C 
      subroutine design_butter (samhz,corner,nsect,acof,bcof)
      implicit none
      integer nsect 
      real samhz,corner
      real acof(3,*), bcof(3,*)
      integer npoles, m
      real wc,fact,sector,wedge,ang, um,vm, xm,ym,den,sum
      real pi /3.1415926535/ 

      wc= corner / samhz 
      fact= tan( pi*wc ) 
      npoles= 2*nsect 
      sector= pi / npoles 
      wedge= sector / 2. 

      do m = 1, nsect
       ang = wedge * (2*m - 1) 
       
       xm= -fact * cos( ang ) 
       ym= fact * sin( ang )
       
       den = ( 1. -xm)**2 + ym**2
        
       um= (1. -xm*xm -ym*ym )/ den 
       vm= (2. * ym )/ den
        
       bcof(1,m) = 1. 
       bcof(2,m) = -2. * um 
       bcof(3,m) =um*um+vm*vm 

       sum = bcof(1,m) + bcof(2,m) + bcof(3,m) 

       acof(1,m) = sum/4. 
       acof(2,m) = sum/2. 
       acof(3,m) = sum/4.
      enddo
      
      return
      end



C --- subroutine filter_2nd_order (x,npt,a,b) ---
C
C Subroutine for recursive application of second-order filter to a time
C domain signal. 
C
C Inside this routine, filtering is forward. Backward filtering may be
C accomplished by reversing the signal prior to calling this routine,
C then restoring the order upon return of the filtered signal. 
C
C The first two points are reflected about the initial point to produce
C a reasonable starting method. Other initial conditions may dictate
C other starting methods. 
C
C By sliding the filter window along the time axis, the need for auxillary
C storage is eliminated, allowing the full utilization of computer memory.
C 
C This routine illustrates the correct usage of the filter coeffients
C in the difference equation: 
C
C    Y(n) = AO*X(n) + Al*X(n-1) + A2*X(n-2) -Bl*Y(n-1) -B2*Y(n-2)
C 
C Arguments:
C 
C    x() . . .        upon entry, an array containing the unfiltered signal,
C                     and replaced by the filtered signal upon return.
C 
C    npt . . .        number of samples in the x() signal array. 
C
C    a() . . .        array containing AO, Al, and A2 coefficients of filter
C    b() . . .        array containing BO, Bl, and B2 coefficinets of filter 
C                     Note: BO must be supplied even through not used.
C
      subroutine filter_2nd_order (x,npt,a,b)
      implicit none
      integer npt
      real x(*),a(*),b(*)
      integer n
      real a0,a1,a2,b1,b2, xn0,xn1,xn2, yn0,yn1,yn2, yn

      a0= a(1)
      a1= a(1)
      a2= a(3)
      b1= b(2)
      b2= b(3) 
      
      xn0=  x(1)
      xn1=  2*xn0  -x(2)  
      xn2=  2*xn0  -x(3)  
      
      yn1=  xn2  
      yn2=  xn1
      
      yn0= a0*xn0 + a1*xn1 + a2*xn2 - b1*yn1 - b2*yn2
      x(1)= yn0  

      xn2= xn1  
      xn1= xn0  
      xn0= x(2)
        
      yn2= yn1  
      yn1= yn0  

      yn0= a0*xn0 + a1*xn1 + a2*xn2 - b1*yn1 - b2*yn2  
      x(2)= yn0
        
      yn1=  x(2)  
      yn2=  x(1)  
      
      do  n= 3,npt  
       yn= a0*x(n) + a1*x(n-1) +  a2*x(n-2) - b1*yn1 - b2*yn2

       x(n-2)=  yn2  
       yn2=  yn1  
       yn1=  yn 
      enddo
      return
      end 



C --- subroutine design_J211 (samhz,corner,nsect,acof,bcof) ---
C
C Subroutine to design low-pass digital filters using equations recommendes
C by the SAE J211 guideline(1994, draft). All J211 channel class filters
C (CFC 60, 180, 300 and 1000) are derived from 2 4-th order Butterworth
C filter, modified to keep the filter response inside a desired freuency
C response corridor. Arguments are:
C
C Input:
C
C    samhz . . .    given sampling rate (Hz) of digital signal.
C 
C    corner . . .   given filter corner frequency (Hz) where the magnitude
C                   is -3 dB (half-power point). This is equal to the class
C                   CFC of the filter, divided by the 0.6 factor.
C 
C    nsect . . .    given number of 2nd-order sections (pole-pairs). For the
C                   J211 filters, there are 2 identical sections.
C
C Output:
C 
C    acof . . .     coefficients (AO,Al,A2) of 2nd-order filter sections 
C    bcof . . .     coefficients (BO,Bl,B2) of 2nd-order filter sections 
C
C Implementation: 
C
C    Recursive filtering through each 2nd-order section is performed by 
C    the difference equation:
C 
C Y(n) = A0 * X(n) + Al * X(n-1) + A2 * X(n-2) -Bl Y(n-1) -B2 * Y(n-2)
C 
      subroutine design_J211 (samhz,corner,nsect,acof,bcof)
      implicit none
      integer nsect 
      real samhz,corner
      real acof(3,*), bcof(3,*)

      integer class, m
      real sr2, ts, arg, wa,wa2, den, a0,a1,a2, b0,b1,b2
      double precision wd
      real pi /3.1415926535/ 

      class= 0.6 * corner
      if (nint(corner) .eq. 1650) class = 1000
      
      ts= 1.00 / samhz
      sr2= sqrt (2.)
      
      wd= 2.d0*pi*class*2.0775
      
      arg= wd*ts / 2.
      wa= tan(arg)
      wa2= wa*wa
      
      den= (1.+sr2*wa+wa2)
      
      a0= wa2/den
      a1= 2.*a0
      a2= a0
      
      b0= 1.0
      b1= 2.* (wa2-1.) / den
      b2= (1.-sr2*wa+wa2) / den

      do m = 1, nsect
       acof(1,m) = a0 
       acof(2,m) = a1
       acof(3,m) = a2

       bcof(1,m) = 1. 
       bcof(2,m) = b1
       bcof(3,m) = b2
      enddo
      
      return
      end

