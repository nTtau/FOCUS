
!title (coils) ! Initialize coils data using Fourier series.

!latex \briefly{Initialize the coils data with Fourier series.}

!latex \calledby{\link{focus}}
!latex \calls{\link{}}

!latex \subsection{overview}
!latex \bi
!latex \item[1.] If \inputvar{case\_coils=1}, then the Fourier series will be used for represent the coils.
!latex \item[2.] The basic equations about the Fourier representation is,
!latex  \be \label{fouriercoil}
!latex  x & = & X_{c,0} + \sum_ {n=1}^{N} \left [X_{c,n} \cos(nt)  +  X_{s,n} \sin(nt) \right ], \\ 
!latex  y & = & Y_{c,0} + \sum_ {n=1}^{N} \left [Y_{c,n} \cos(nt)  +  Y_{s,n} \sin(nt) \right ], \\
!latex  z & = & Z_{c,0} + \sum_ {n=1}^{N} \left [Z_{c,n} \cos(nt)  +  Z_{s,n} \sin(nt) \right ], 
!latex  \ee
!latex  \ei
!latex  \subsection{Initilization}
!latex  There are several ways to initialize the coils data.
!latex  \bi
!latex  \item[1.] \inputvar{case\_init = 0} : Toroidally placing \inputvar{Ncoils} circular coils with a 
!latex             radius of \inputvar{init\_radius} and current of \inputvar{init\_current}. The $i$th coil 
!latex             is placed at $\z = \frac{i-1}{Ncoils} \frac{2\pi}{Nfp}$.
!latex  \item[2.] \inputvar{case\_init = 1} : Read coils data from {\bf .ext.coil.xxx} files. xxx can vary 
!latex             from $001$ to $999$. Each file has such a format. \red{This is the most flexible way, and
!latex             each coil can be different.}            
!latex  \begin{raw}
!latex  #type of coils;   name
!latex      1      "Module 1"
!latex  #  Nseg       I   Ic  L   Lc  Lo
!latex     128  1.0E+07  0 6.28  1 3.14
!latex  # NFcoil
!latex  1
!latex  # Fourier harmonics for coils ( xc; xs; yc; ys; zc; zs)
!latex  3.00 0.30
!latex  0.00 0.00
!latex  0.00 0.00
!latex  0.00 0.00
!latex  0.00 0.00
!latex  0.00 0.30
!latex  \end{raw}
!latex  \ei
!latex  \bi
!latex  \item[3.] \inputvar{case\_init = -1} : Get coils data from a standard coils.ext file and 
!latex           then Fourier decomposed (normal Fourier tansformation and truncated with $NFcoil$ harmonics)
!latex  \ei
!latex   \subsection{Discretization}
!latex   \bi
!latex   \item[1.] Discretizing the coils data involves massive triangular functions in nested loops.
!latex   As shown in  \Eqn{fouriercoil}, the outside loop is for different discrete points and for each
!latex   point, a loop is needed to get the summation of the harmonics.
!latex   \item[2.] To avoid calling triangular functions every operations, it's a btter idea to allocate
!latex   the public triangular arrays.
!latex   \be
!latex   cmt(iD, iN) = \cos(iN \ \frac{iD}{D_i} 2\pi); iD = 0, coil(icoil)\%D; \ iN = 0,  coil(icoil)\%N \\
!latex   smt(iD, iN) = \sin(iN \ \frac{iD}{D_i} 2\pi); iD = 0, coil(icoil)\%D; \ iN = 0,  coil(icoil)\%N
!latex   \ee
!latex   \item[3.] Using the concept of vectorization, we can also finish this just through matrix 
!latex   operations. This is in \subroutine{fouriermatrix}.
!latex   \begin{raw}
!latex   subroutine fouriermatrix(xc, xs, xx, NF, ND)
!latex   nn(0:NF, 1:1) : matrix for N; iN
!latex   tt(1:1, 0:ND) : matrix for angle; iD/ND*2pi
!latex   nt(0:NF,0:ND) : grid for nt; nt = matmul(nn, tt)
!latex   xc(1:1, 0:NF) : cosin harmonics;
!latex   xs(1:1, 0:NF) : sin harmonics;
!latex   xx(1:1, 0:ND) : returned disrecte points;
!latex   
!latex   xx = xc * cos(nt) + xs * sin(nt)
!latex   \end{raw}
!latex   \item[4.] Actually, in real tests, the new method is not so fast. And parallelizations are actually
!latex   slowing the speed, both for the normal and vectorized method. 
!latex   \ei

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

subroutine rdcoils

  use globals

  implicit none

  include "mpif.h"

  LOGICAL   :: exist
  INTEGER   :: icoil, maxnseg, ifirst, NF, itmp, ip, icoef, total_coef
  REAL      :: Rmaj, zeta, totalcurrent, z0, r1, r2, z1, z2
  !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

  Nfixcur = 0 ! fixed coil current number
  Nfixgeo = 0 ! fixed coil geometry number

  if(myid == 0) write(ounit, *) "-----------INITIALIZE COILS----------------------------------"

  select case( case_init )

     !-------------read coils file--------------------------------------------------------------------------
  case(-1 )
     if (myid == 0) then
        write(ounit,'("rdcoils : Reading coils data (MAKEGRID format) from "A)') trim(inpcoils)
        call readcoils(inpcoils, maxnseg)
        write(ounit,'("        : Read ",i6," coils.")') Ncoils
        if (IsQuiet < 0) write(ounit, '(8X,": NFcoil = "I3" ; IsVaryCurrent = "I1 &
             " ; IsVaryGeometry = "I1)') NFcoil, IsVaryCurrent, IsVaryGeometry
     endif

     IlBCAST( Ncoils   ,      1, 0 )
     IlBCAST( maxnseg  ,      1, 0 )

     if( .not. allocated(coilsX) ) then !allocate arrays on other nodes;
        SALLOCATE( coilsX, (1:maxnseg,1:Ncoils), zero )    
        SALLOCATE( coilsY, (1:maxnseg,1:Ncoils), zero )
        SALLOCATE( coilsZ, (1:maxnseg,1:Ncoils), zero )
        SALLOCATE( coilsI, (          1:Ncoils), zero )
        SALLOCATE( coilseg,(          1:Ncoils),    0 )
        SALLOCATE( coilname,(         1:Ncoils),   '' )
     endif

     ! broadcast coils data;
     RlBCAST( coilsX, maxnseg*Ncoils, 0 )
     RlBCAST( coilsY, maxnseg*Ncoils, 0 )
     RlBCAST( coilsZ, maxnseg*Ncoils, 0 )
     RlBCAST( coilsI,         Ncoils, 0 )   
     IlBCAST( coilseg,        Ncoils, 0 )
     ClBCAST( coilname,       Ncoils, 0 )

     allocate(    coil(1:Ncoils) )
     allocate( FouCoil(1:Ncoils) )
     allocate(     DoF(1:Ncoils) )

     Ncoils = Ncoils / Npc ! Ncoils changed to unique number of coils;
     icoil = 0
     do icoil = 1, Ncoils

        !general coil parameters;
        coil(icoil)%NS =  Nseg  
        coil(icoil)%I  =  coilsI(icoil)
        coil(icoil)%Ic =  IsVaryCurrent
        coil(icoil)%L  =  target_length ! irrelevant until re-computed;
        coil(icoil)%Lc =  IsVaryGeometry
        coil(icoil)%Lo =  target_length
        coil(icoil)%name = trim(coilname(icoil))

        FATAL( rdcoils, coil(icoil)%Ic < 0 .or. coil(icoil)%Ic > 1, illegal )
        FATAL( rdcoils, coil(icoil)%Lc < 0 .or. coil(icoil)%Lc > 1, illegal )
        FATAL( rdcoils, coil(icoil)%Lo < zero                     , illegal )
        if(coil(icoil)%Ic == 0) Nfixcur = Nfixcur + 1
        if(coil(icoil)%Lc == 0) Nfixgeo = Nfixgeo + 1

        !Fourier representation related;
        FouCoil(icoil)%NF = NFcoil
        NF = NFcoil  ! alias
        SALLOCATE( FouCoil(icoil)%xc, (0:NF), zero )
        SALLOCATE( FouCoil(icoil)%xs, (0:NF), zero )
        SALLOCATE( FouCoil(icoil)%yc, (0:NF), zero )
        SALLOCATE( FouCoil(icoil)%ys, (0:NF), zero )
        SALLOCATE( FouCoil(icoil)%zc, (0:NF), zero )
        SALLOCATE( FouCoil(icoil)%zs, (0:NF), zero )        

        !if(myid .ne. modulo(icoil-1, ncpu)) cycle

       call Fourier( coilsX(1:coilseg(icoil),icoil), Foucoil(icoil)%xc, Foucoil(icoil)%xs, coilseg(icoil), NF)
       call Fourier( coilsY(1:coilseg(icoil),icoil), Foucoil(icoil)%yc, Foucoil(icoil)%ys, coilseg(icoil), NF)
       call Fourier( coilsZ(1:coilseg(icoil),icoil), Foucoil(icoil)%zc, Foucoil(icoil)%zs, coilseg(icoil), NF)

     enddo

     DALLOCATE( coilsX )
     DALLOCATE( coilsY )
     DALLOCATE( coilsZ )
     DALLOCATE( coilsI )
     DALLOCATE( coilseg)
     DALLOCATE(coilname)

     coil(1:Ncoils)%itype = case_coils

     !-------------individual coil file---------------------------------------------------------------------
  case( 0 )

     if( myid==0 ) then  !get file number;
        open( runit, file=trim(coilfile), status="old", action='read')
        read( runit,*)
        read( runit,*) Ncoils
        write(ounit,'("rdcoils : identified "i3" unique coils in "A" ;")') Ncoils, coilfile
     endif
                               
     IlBCAST( Ncoils        ,    1,  0 )
     allocate( FouCoil(1:Ncoils*Npc) )
     allocate(    coil(1:Ncoils*Npc) )
     allocate(     DoF(1:Ncoils*Npc) )

     if( myid==0 ) then
        do icoil = 1, Ncoils
           read( runit,*)
           read( runit,*)
           read( runit,*) coil(icoil)%itype, coil(icoil)%name
           if(coil(icoil)%itype /= 1) then
              STOP " wrong coil type in rdcoils"
              call MPI_ABORT(MPI_COMM_WORLD, 1, ierr)
           endif
           read( runit,*)
           read( runit,*) coil(icoil)%NS, coil(icoil)%I, coil(icoil)%Ic, &
                & coil(icoil)%L, coil(icoil)%Lc, coil(icoil)%Lo
           FATAL( rdcoils, coil(icoil)%NS < 0                        , illegal )
           FATAL( rdcoils, coil(icoil)%Ic < 0 .or. coil(icoil)%Ic > 1, illegal )
           FATAL( rdcoils, coil(icoil)%Lc < 0 .or. coil(icoil)%Lc > 2, illegal )
           FATAL( rdcoils, coil(icoil)%L  < zero                     , illegal )
           FATAL( rdcoils, coil(icoil)%Lc < zero                     , illegal )
           FATAL( rdcoils, coil(icoil)%Lo < zero                     , illegal )
           read( runit,*)
           read( runit,*) FouCoil(icoil)%NF
           FATAL( rdcoils, Foucoil(icoil)%NF  < 0                    , illegal )
           SALLOCATE( FouCoil(icoil)%xc, (0:FouCoil(icoil)%NF), zero )
           SALLOCATE( FouCoil(icoil)%xs, (0:FouCoil(icoil)%NF), zero )
           SALLOCATE( FouCoil(icoil)%yc, (0:FouCoil(icoil)%NF), zero )
           SALLOCATE( FouCoil(icoil)%ys, (0:FouCoil(icoil)%NF), zero )
           SALLOCATE( FouCoil(icoil)%zc, (0:FouCoil(icoil)%NF), zero )
           SALLOCATE( FouCoil(icoil)%zs, (0:FouCoil(icoil)%NF), zero )
           read( runit,*)
           read( runit,*) FouCoil(icoil)%xc(0:FouCoil(icoil)%NF)
           read( runit,*) FouCoil(icoil)%xs(0:FouCoil(icoil)%NF)
           read( runit,*) FouCoil(icoil)%yc(0:FouCoil(icoil)%NF)
           read( runit,*) FouCoil(icoil)%ys(0:FouCoil(icoil)%NF)
           read( runit,*) FouCoil(icoil)%zc(0:FouCoil(icoil)%NF)
           read( runit,*) FouCoil(icoil)%zs(0:FouCoil(icoil)%NF)

        enddo !end do icoil;

        close( runit )
     endif ! end of if( myid==0 );

     do icoil = 1, Ncoils

        IlBCAST( coil(icoil)%itype        , 1        ,  0 )
        ClBCAST( coil(icoil)%name         , 10       ,  0 )
        IlBCAST( coil(icoil)%NS           , 1        ,  0 )
        RlBCAST( coil(icoil)%I            , 1        ,  0 )
        IlBCAST( coil(icoil)%Ic           , 1        ,  0 )
        RlBCAST( coil(icoil)%L            , 1        ,  0 )
        IlBCAST( coil(icoil)%Lc           , 1        ,  0 )
        RlBCAST( coil(icoil)%Lo           , 1        ,  0 )
        IlBCAST( FouCoil(icoil)%NF        , 1        ,  0 )

        if (.not. allocated(FouCoil(icoil)%xc) ) then
           SALLOCATE( FouCoil(icoil)%xc, (0:FouCoil(icoil)%NF), zero )
           SALLOCATE( FouCoil(icoil)%xs, (0:FouCoil(icoil)%NF), zero )
           SALLOCATE( FouCoil(icoil)%yc, (0:FouCoil(icoil)%NF), zero )
           SALLOCATE( FouCoil(icoil)%ys, (0:FouCoil(icoil)%NF), zero )
           SALLOCATE( FouCoil(icoil)%zc, (0:FouCoil(icoil)%NF), zero )
           SALLOCATE( FouCoil(icoil)%zs, (0:FouCoil(icoil)%NF), zero ) 
        endif
        RlBCAST( FouCoil(icoil)%xc(0:FouCoil(icoil)%NF) , 1+FouCoil(icoil)%NF ,  0 )
        RlBCAST( FouCoil(icoil)%xs(0:FouCoil(icoil)%NF) , 1+FouCoil(icoil)%NF ,  0 )
        RlBCAST( FouCoil(icoil)%yc(0:FouCoil(icoil)%NF) , 1+FouCoil(icoil)%NF ,  0 )
        RlBCAST( FouCoil(icoil)%ys(0:FouCoil(icoil)%NF) , 1+FouCoil(icoil)%NF ,  0 )
        RlBCAST( FouCoil(icoil)%zc(0:FouCoil(icoil)%NF) , 1+FouCoil(icoil)%NF ,  0 )
        RlBCAST( FouCoil(icoil)%zs(0:FouCoil(icoil)%NF) , 1+FouCoil(icoil)%NF ,  0 )

        if(coil(icoil)%Ic == 0) Nfixcur = Nfixcur + 1
        if(coil(icoil)%Lc == 0) Nfixgeo = Nfixgeo + 1

     enddo

     !-------------toroidally placed circular coils---------------------------------------------------------
  case( 1 ) ! toroidally placed coils; 2017/03/13

     allocate( FouCoil(1:Ncoils*Npc) )
     allocate(    coil(1:Ncoils*Npc) )
     allocate(     DoF(1:Ncoils*Npc) )

     if (myid == 0)  then
        write(ounit,'("rdcoils : initializing "i3" unique circular coils;")') Ncoils
        if (IsQuiet < 1) write(ounit, '(8X,": Initialize "I4" circular coils with r="ES12.5"m ; I="&
             ES12.5" A")') Ncoils, init_radius, init_current
        if (IsQuiet < 0) write(ounit, '(8X,": NFcoil = "I3" ; IsVaryCurrent = "I1 &
             " ; IsVaryGeometry = "I1)') NFcoil, IsVaryCurrent, IsVaryGeometry
     endif

     do icoil = 1, Ncoils

        !general coil parameters;
        coil(icoil)%NS =  Nseg  
        coil(icoil)%I  =  init_current
        coil(icoil)%Ic =  IsVaryCurrent
        coil(icoil)%L  =  pi2*init_radius
        coil(icoil)%Lc =  IsVaryGeometry
        coil(icoil)%Lo =  target_length
        write(coil(icoil)%name,'("Mod_"I3.3)') icoil
        FATAL( rdcoils, coil(icoil)%Ic < 0 .or. coil(icoil)%Ic > 1, illegal )
        FATAL( rdcoils, coil(icoil)%Lc < 0 .or. coil(icoil)%Lc > 1, illegal )
        FATAL( rdcoils, coil(icoil)%Lo < zero                     , illegal )
        if(coil(icoil)%Ic == 0) Nfixcur = Nfixcur + 1
        if(coil(icoil)%Lc == 0) Nfixgeo = Nfixgeo + 1

        !Fourier representation related;
        FouCoil(icoil)%NF = NFcoil
        SALLOCATE( FouCoil(icoil)%xc, (0:NFcoil), zero )
        SALLOCATE( FouCoil(icoil)%xs, (0:NFcoil), zero )
        SALLOCATE( FouCoil(icoil)%yc, (0:NFcoil), zero )
        SALLOCATE( FouCoil(icoil)%ys, (0:NFcoil), zero )
        SALLOCATE( FouCoil(icoil)%zc, (0:NFcoil), zero )
        SALLOCATE( FouCoil(icoil)%zs, (0:NFcoil), zero )

        !initilize with circular coils;
        zeta = (icoil-1+half) * pi2 / (Ncoils*Npc)  ! put a half for a shift;

        call surfcoord( zero, zeta, r1, z1)
        call surfcoord(   pi, zeta, r2, z2)

        Rmaj = half * (r1 + r2)
        z0   = half * (z1 + z2)        
        
        FouCoil(icoil)%xc(0:1) = (/ Rmaj * cos(zeta), init_radius * cos(zeta) /)
        FouCoil(icoil)%xs(0:1) = (/ 0.0             , 0.0                     /)
        FouCoil(icoil)%yc(0:1) = (/ Rmaj * sin(zeta), init_radius * sin(zeta) /)
        FouCoil(icoil)%ys(0:1) = (/ 0.0             , 0.0                     /)
        FouCoil(icoil)%zc(0:1) = (/ z0              , 0.0                     /)
        Foucoil(icoil)%zs(0:1) = (/ 0.0             , init_radius             /)

     enddo ! end of do icoil;

     coil(1:Ncoils)%itype = case_coils

  end select

  FATAL( rdcoils, Nfixcur > Ncoils, error with fixed currents )
  FATAL( rdcoils, Nfixgeo > Ncoils, error with fixed geometry )

  !-----------------------allocate   coil data--------------------------------------------------
  do ip = 1, Npc
     do icoil = 1, Ncoils
        SALLOCATE( coil(icoil+(ip-1)*Ncoils)%xx, (0:coil(icoil)%NS), zero )
        SALLOCATE( coil(icoil+(ip-1)*Ncoils)%yy, (0:coil(icoil)%NS), zero )
        SALLOCATE( coil(icoil+(ip-1)*Ncoils)%zz, (0:coil(icoil)%NS), zero )
        SALLOCATE( coil(icoil+(ip-1)*Ncoils)%xt, (0:coil(icoil)%NS), zero )
        SALLOCATE( coil(icoil+(ip-1)*Ncoils)%yt, (0:coil(icoil)%NS), zero )
        SALLOCATE( coil(icoil+(ip-1)*Ncoils)%zt, (0:coil(icoil)%NS), zero )
        SALLOCATE( coil(icoil+(ip-1)*Ncoils)%xa, (0:coil(icoil)%NS), zero )
        SALLOCATE( coil(icoil+(ip-1)*Ncoils)%ya, (0:coil(icoil)%NS), zero )
        SALLOCATE( coil(icoil+(ip-1)*Ncoils)%za, (0:coil(icoil)%NS), zero )
        SALLOCATE( coil(icoil+(ip-1)*Ncoils)%dl, (0:coil(icoil)%NS), zero )
        SALLOCATE( coil(icoil+(ip-1)*Ncoils)%dd, (0:coil(icoil)%NS), zero )
     enddo
  enddo

  SALLOCATE( cosip, (0:Npc), one  )  ! cos(ip*pi/Np) ; default one ;
  SALLOCATE( sinip, (0:Npc), zero )  ! sin(ip*pi/Np) ; default zero;

  if (Npc >= 2) then
     do ip = 1, Npc-1
        cosip(ip) = cos(ip*pi2/Npc) ; sinip(ip) = sin(ip*pi2/Npc)
        do icoil = 1, Ncoils
           select case (coil(icoil)%itype)
           case( 1 )
              NF = FouCoil(icoil)%NF
              SALLOCATE( FouCoil(icoil+ip*Ncoils)%xc, (0:NF), zero )
              SALLOCATE( FouCoil(icoil+ip*Ncoils)%xs, (0:NF), zero )
              SALLOCATE( FouCoil(icoil+ip*Ncoils)%yc, (0:NF), zero )
              SALLOCATE( FouCoil(icoil+ip*Ncoils)%ys, (0:NF), zero )
              SALLOCATE( FouCoil(icoil+ip*Ncoils)%zc, (0:NF), zero )
              SALLOCATE( FouCoil(icoil+ip*Ncoils)%zs, (0:NF), zero )
           case default
              FATAL(discoil, .true., not supported coil types)
           end select
        enddo
     enddo
     
     call mapcoil ! map perodic coils;

  endif 

  !-----------------------normalize currents and geometries-------------------------------------
  !sum the total currents;
  totalcurrent = zero
  do icoil = 1, Ncoils*Npc
     totalcurrent = totalcurrent + coil(icoil)%I
  enddo

  if(myid == 0 .and. IsQuiet <= 0) then
     write(ounit,'("        : "i3" fixed currents ; "i3" fixed geometries.")') &
          & Nfixcur, Nfixgeo
     !write( ounit,'("        : total current G ="es23.15" ; 2 . pi2 . G = "es23.15" ;")') &
     !     & totalcurrent, totalcurrent * pi2 * two
  endif

  if (IsNormalize > 0) then
     Gnorm = 0
     Inorm = 0
     total_coef = 0 ! total number of coefficients
     do icoil = 1, Ncoils
        NF = FouCoil(icoil)%NF
        total_coef = total_coef + (6*NF + 3)
        do icoef = 0, NF
           Gnorm = Gnorm + FouCoil(icoil)%xs(icoef)**2 + FouCoil(icoil)%xc(icoef)**2
           Gnorm = Gnorm + FouCoil(icoil)%ys(icoef)**2 + FouCoil(icoil)%yc(icoef)**2
           Gnorm = Gnorm + FouCoil(icoil)%zs(icoef)**2 + FouCoil(icoil)%zc(icoef)**2
        enddo
        Inorm = Inorm + coil(icoil)%I**2
     enddo
     Gnorm = sqrt(Gnorm/total_coef) * weight_gnorm      ! quadratic mean
     Inorm = sqrt(Inorm/Ncoils) * weight_inorm          ! quadratic mean
     !Inorm = Inorm * 6  ! compensate for the fact that there are so many more spatial variables

     FATAL( rdcoils, abs(Gnorm) < machprec, cannot be zero )
     FATAL( rdcoils, abs(Inorm) < machprec, cannot be zero )
     
     if (myid == 0) then
        write(ounit, '("        : Currents   are normalized by " ES23.15)') Inorm
        write(ounit, '("        : Geometries are normalized by " ES23.15)') Gnorm
     endif

  else
     Inorm = one
     Gnorm = one
  endif

  !-----------------------allocate DoF arrays --------------------------------------------------  

  itmp = -1
  call AllocData(itmp)

  !-----------------------discretize coil data--------------------------------------------------

  if (myid == 0) then
     if (IsQuiet < 0) write(ounit, '(8X,": coils will be discretized in "I6" segments")') Nseg
  endif

  ifirst = 1
  call discoil(ifirst)
  ifirst = 0

  return

end subroutine rdcoils

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

subroutine mapcoil
!---------------------------------------------------------------------------------------------
! mapping periodic coils;
!---------------------------------------------------------------------------------------------
  use globals, only: zero, pi2, myid, ounit, ierr, coil, FouCoil, Ncoils, DoF, Npc, cosip, sinip
  implicit none
  include "mpif.h"

  INTEGER  :: ip, icoil, NF

  do ip = 1, Npc-1
     
     do icoil = 1, Ncoils

        coil(icoil+ip*Ncoils)%itype   = coil(icoil)%itype
        coil(icoil+ip*Ncoils)%NS      = coil(icoil)%NS
        coil(icoil+ip*Ncoils)%Ic      = coil(icoil)%Ic
        coil(icoil+ip*Ncoils)%Lc      = coil(icoil)%Lc
        coil(icoil+ip*Ncoils)%I       = coil(icoil)%I 
        coil(icoil+ip*Ncoils)%L       = coil(icoil)%L 
        coil(icoil+ip*Ncoils)%Lo      = coil(icoil)%Lo
        coil(icoil+ip*Ncoils)%maxcurv = coil(icoil)%maxcurv
        coil(icoil+ip*Ncoils)%name    = coil(icoil)%name

        select case (coil(icoil)%itype)
        case( 1 )
           Foucoil(icoil+ip*Ncoils)%NF = Foucoil(icoil)%NF
           Foucoil(icoil+ip*Ncoils)%xc = Foucoil(icoil)%xc * cosip(ip) - Foucoil(icoil)%yc * sinip(ip)
           Foucoil(icoil+ip*Ncoils)%xs = Foucoil(icoil)%xs * cosip(ip) - Foucoil(icoil)%ys * sinip(ip)
           Foucoil(icoil+ip*Ncoils)%yc = Foucoil(icoil)%yc * cosip(ip) + Foucoil(icoil)%xc * sinip(ip)
           Foucoil(icoil+ip*Ncoils)%ys = Foucoil(icoil)%ys * cosip(ip) + Foucoil(icoil)%xs * sinip(ip)
           Foucoil(icoil+ip*Ncoils)%zc = Foucoil(icoil)%zc
           Foucoil(icoil+ip*Ncoils)%zs = Foucoil(icoil)%zs
        case default
           FATAL(discoil, .true., not supported coil types)
        end select

     enddo
  enddo

  return

END subroutine mapcoil
           

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

subroutine discoil(ifirst)
!---------------------------------------------------------------------------------------------
! dicretize coils data;
! if ifirst = 1, it will update all the coils; otherwise, only update free coils;
! date: 20170314
!---------------------------------------------------------------------------------------------
  use globals, only: zero, pi2, myid, ounit, coil, FouCoil, Ncoils, DoF, Npc, cosip, sinip
  implicit none
  include "mpif.h"

  INTEGER, intent(in) :: ifirst

  INTEGER          :: icoil, iseg, mm, NS, NF, ierr, astat, ip
  REAL             :: tt
  REAL,allocatable :: cmt(:,:), smt(:,:)
  !-------------------------------------------------------------------------------------------
  !xx, xt, xa are 0, 1st and 2nd derivatives;
  if (Npc >= 2) call mapcoil ! map periodic coils;

  do icoil = 1, Ncoils*Npc

     if( (coil(icoil)%Lc + ifirst) /= 0) then  !first time or if Lc/=0, then need discretize;

        !reset to zero for all the coils;
        coil(icoil)%xx = zero
        coil(icoil)%yy = zero
        coil(icoil)%zz = zero
        coil(icoil)%xt = zero
        coil(icoil)%yt = zero
        coil(icoil)%zt = zero
        coil(icoil)%xa = zero
        coil(icoil)%ya = zero
        coil(icoil)%za = zero

        !if( myid.ne.modulo(icoil-1,ncpu) ) cycle ! parallelization loop;

        select case (coil(icoil)%itype)
        case( 1 )

           NS = coil(icoil)%NS; NF = FouCoil(icoil)%NF  ! allias variable for simplicity;
           SALLOCATE( cmt, (0:NS, 0:NF), zero )
           SALLOCATE( smt, (0:NS, 0:NF), zero )

           do iseg = 0, NS ; tt = iseg * pi2 / NS
              do mm = 0, NF
                 cmt(iseg,mm) = cos( mm * tt )
                 smt(iseg,mm) = sin( mm * tt )
              enddo
           enddo

           !-------------------------calculate coil data-------------------------------------------------  
           mm = 0
           coil(icoil)%xx(0:NS) = cmt(0:NS,mm) * Foucoil(icoil)%xc(mm)
           coil(icoil)%yy(0:NS) = cmt(0:NS,mm) * Foucoil(icoil)%yc(mm)
           coil(icoil)%zz(0:NS) = cmt(0:NS,mm) * Foucoil(icoil)%zc(mm)    
           do mm = 1, NF   
              coil(icoil)%xx(0:NS) = coil(icoil)%xx(0:NS) + (   cmt(0:NS,mm) * Foucoil(icoil)%xc(mm) &
                                                              + smt(0:NS,mm) * Foucoil(icoil)%xs(mm) )
              coil(icoil)%yy(0:NS) = coil(icoil)%yy(0:NS) + (   cmt(0:NS,mm) * Foucoil(icoil)%yc(mm) &
                                                              + smt(0:NS,mm) * Foucoil(icoil)%ys(mm) )
              coil(icoil)%zz(0:NS) = coil(icoil)%zz(0:NS) + (   cmt(0:NS,mm) * Foucoil(icoil)%zc(mm) &
                                                              + smt(0:NS,mm) * Foucoil(icoil)%zs(mm) )

              coil(icoil)%xt(0:NS) = coil(icoil)%xt(0:NS) + ( - smt(0:NS,mm) * Foucoil(icoil)%xc(mm) &
                                                              + cmt(0:NS,mm) * Foucoil(icoil)%xs(mm) ) * mm
              coil(icoil)%yt(0:NS) = coil(icoil)%yt(0:NS) + ( - smt(0:NS,mm) * Foucoil(icoil)%yc(mm) &
                                                              + cmt(0:NS,mm) * Foucoil(icoil)%ys(mm) ) * mm
              coil(icoil)%zt(0:NS) = coil(icoil)%zt(0:NS) + ( - smt(0:NS,mm) * Foucoil(icoil)%zc(mm) &
                                                              + cmt(0:NS,mm) * Foucoil(icoil)%zs(mm) ) * mm

              coil(icoil)%xa(0:NS) = coil(icoil)%xa(0:NS) + ( - cmt(0:NS,mm) * Foucoil(icoil)%xc(mm) &
                                                              - smt(0:NS,mm) * Foucoil(icoil)%xs(mm) ) * mm*mm
              coil(icoil)%ya(0:NS) = coil(icoil)%ya(0:NS) + ( - cmt(0:NS,mm) * Foucoil(icoil)%yc(mm) &
                                                              - smt(0:NS,mm) * Foucoil(icoil)%ys(mm) ) * mm*mm
              coil(icoil)%za(0:NS) = coil(icoil)%za(0:NS) + ( - cmt(0:NS,mm) * Foucoil(icoil)%zc(mm) &
                                                              - smt(0:NS,mm) * Foucoil(icoil)%zs(mm) ) * mm*mm
           enddo ! end of do mm; 

           if(ifirst /= 0) then
              ip = (icoil-1)/Ncoils  ! the integer is the period number;
              DoF(icoil)%xof(0:NS-1,      1:  NF+1) =  cosip(ip) * cmt(0:NS-1, 0:NF)  !x/xc
              DoF(icoil)%xof(0:NS-1,   NF+2:2*NF+1) =  cosip(ip) * smt(0:NS-1, 1:NF)  !x/xs
              DoF(icoil)%xof(0:NS-1, 2*NF+2:3*NF+2) = -sinip(ip) * cmt(0:NS-1, 0:NF)  !x/yc ; valid for ip>0 ;
              DoF(icoil)%xof(0:NS-1, 3*NF+3:4*NF+2) = -sinip(ip) * smt(0:NS-1, 1:NF)  !x/ys ; valid for ip>0 ;
              DoF(icoil)%yof(0:NS-1,      1:  NF+1) =  sinip(ip) * cmt(0:NS-1, 0:NF)  !y/xc ; valid for ip>0 ;
              DoF(icoil)%yof(0:NS-1,   NF+2:2*NF+1) =  sinip(ip) * smt(0:NS-1, 1:NF)  !y/xs ; valid for ip>0 ;
              DoF(icoil)%yof(0:NS-1, 2*NF+2:3*NF+2) =  cosip(ip) * cmt(0:NS-1, 0:NF)  !y/yc
              DoF(icoil)%yof(0:NS-1, 3*NF+3:4*NF+2) =  cosip(ip) * smt(0:NS-1, 1:NF)  !y/ys
              DoF(icoil)%zof(0:NS-1, 4*NF+3:5*NF+3) =              cmt(0:NS-1, 0:NF)  !z/zc
              DoF(icoil)%zof(0:NS-1, 5*NF+4:6*NF+3) =              smt(0:NS-1, 1:NF)  !z/zs
           endif

           coil(icoil)%dd = pi2 / NS  ! discretizing factor;

           DALLOCATE(cmt)
           DALLOCATE(smt)

        case default
           FATAL(discoil, .true., not supported coil types)
        end select

     endif

  enddo ! end of do icoil

  return
end subroutine discoil

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

SUBROUTINE discfou2
  !---------------------------------------------------------------------------------------------
  ! Discretize coil data from Fourier harmonics to xx, yy, zz
  ! calling fouriermatrix for single set
  ! DATE: 2017/03/18
  !---------------------------------------------------------------------------------------------  
  use globals, only: zero, pi2, myid, ncpu, ounit, coil, FouCoil, Ncoils
  implicit none
  include "mpif.h"

  INTEGER :: icoil, iorder, llmodnp, ierr, NS, NF
  !-------------------------call fouriermatr----------------------------------------------------  
  do icoil = 1, Ncoils

     NS = coil(icoil)%NS; NF = FouCoil(icoil)%NF  ! allias variable for simplicity;
     !reset to zero for all the coils;
     coil(icoil)%xx = zero
     coil(icoil)%yy = zero
     coil(icoil)%zz = zero
     coil(icoil)%xt = zero
     coil(icoil)%yt = zero
     coil(icoil)%zt = zero
     coil(icoil)%xa = zero
     coil(icoil)%ya = zero
     coil(icoil)%za = zero

     coil(icoil)%dd = pi2 / NS
     
     if( myid.ne.modulo(icoil-1,ncpu) ) cycle ! parallelization loop;

    iorder = 0
    call fouriermatrix( Foucoil(icoil)%xc, Foucoil(icoil)%xs, coil(icoil)%xx, NF, NS, iorder)
    call fouriermatrix( Foucoil(icoil)%yc, Foucoil(icoil)%ys, coil(icoil)%yy, NF, NS, iorder)
    call fouriermatrix( Foucoil(icoil)%zc, Foucoil(icoil)%zs, coil(icoil)%zz, NF, NS, iorder)

    iorder = 1  
    call fouriermatrix( Foucoil(icoil)%xc, Foucoil(icoil)%xs, coil(icoil)%xt, NF, NS, iorder)
    call fouriermatrix( Foucoil(icoil)%yc, Foucoil(icoil)%ys, coil(icoil)%yt, NF, NS, iorder)
    call fouriermatrix( Foucoil(icoil)%zc, Foucoil(icoil)%zs, coil(icoil)%zt, NF, NS, iorder)

    iorder = 2 
    call fouriermatrix( Foucoil(icoil)%xc, Foucoil(icoil)%xs, coil(icoil)%xa, NF, NS, iorder)
    call fouriermatrix( Foucoil(icoil)%yc, Foucoil(icoil)%ys, coil(icoil)%ya, NF, NS, iorder)
    call fouriermatrix( Foucoil(icoil)%zc, Foucoil(icoil)%zs, coil(icoil)%za, NF, NS, iorder)
   
  enddo
  !-------------------------broadcast coil data-------------------------------------------------  
  do icoil = 1, Ncoils ; llmodnp = modulo(icoil-1,ncpu)
     RlBCAST( coil(icoil)%xx(0:coil(icoil)%NS), coil(icoil)%NS+1, llmodnp )
     RlBCAST( coil(icoil)%yy(0:coil(icoil)%NS), coil(icoil)%NS+1, llmodnp )
     RlBCAST( coil(icoil)%zz(0:coil(icoil)%NS), coil(icoil)%NS+1, llmodnp )
     RlBCAST( coil(icoil)%xt(0:coil(icoil)%NS), coil(icoil)%NS+1, llmodnp )
     RlBCAST( coil(icoil)%yt(0:coil(icoil)%NS), coil(icoil)%NS+1, llmodnp )
     RlBCAST( coil(icoil)%zt(0:coil(icoil)%NS), coil(icoil)%NS+1, llmodnp )
     RlBCAST( coil(icoil)%xa(0:coil(icoil)%NS), coil(icoil)%NS+1, llmodnp )
     RlBCAST( coil(icoil)%ya(0:coil(icoil)%NS), coil(icoil)%NS+1, llmodnp )
     RlBCAST( coil(icoil)%za(0:coil(icoil)%NS), coil(icoil)%NS+1, llmodnp )
  enddo
  
  return
END SUBROUTINE discfou2

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

subroutine fouriermatrix( xc, xs, xx, NF, ND, order )
  !---------------------------------------------------------------------------------------------
  ! This subroutine uses matrix operations to discretize data from Fourier harmonics.
  ! It's supposed to be the fastest method.
  ! DATE: 2017/03/18
  !---------------------------------------------------------------------------------------------
  use globals, only : zero, pi2
  implicit none
  
  INTEGER, intent(in ) :: NF, ND, order
  REAL   , intent(in ) :: xc(0:NF), xs(0:NF)
  REAL   , intent(out) :: xx(0:ND)

  INTEGER              :: i
  REAL                 :: nn(0:NF, 1:1), tt(1:1, 0:ND), nt(0:NF, 0:ND), &
                       &  tc(1:1, 0:NF), ts(1:1, 0:NF), tx(1:1 , 0:ND), &
                       &  cnt(0:NF, 0:ND), snt(0:NF, 0:ND)

  !----------------------------data copy to matrix----------------------------------------------
  if ( size(xc) /= NF+1 ) STOP "Wrong input size for xc in subroutine fouriermatrix!"
  if ( size(xs) /= NF+1 ) STOP "Wrong input size for xs in subroutine fouriermatrix!"
  if ( size(xx) /= ND+1 ) STOP "Wrong input size for xx in subroutine fouriermatrix!"

  tc(1, 0:NF) = xc(0:NF) ! cos harmonics;
  ts(1, 0:NF) = xs(0:NF) ! sin harmonics;
  tx(1, 0:ND) = xx(0:ND) ! data coordinates;
  !----------------------------matrix assignmengt-----------------------------------------------
  nn(0:NF, 1) = (/ (i, i=0,NF) /) ! n;
  tt(1, 0:ND) = (/ (i*pi2/ND, i=0, ND) /) ! angle, t;
  nt = matmul(nn, tt)
  cnt = cos(nt)
  snt = sin(nt)
  !----------------------------select oder------------------------------------------------------
  select case (order)
  case (0)  ! 0-order

  case (1)  ! 1st-order
     do i = 0, ND
        cnt(0:NF, i) = - nn(0:NF, 1)     * snt(0:NF, i)
        snt(0:NF, i) =   nn(0:NF, 1)     * cnt(0:NF, i)
     enddo
  case (2)  ! 2nd-order
     do i = 0, ND
        cnt(0:NF, i) = -(nn(0:NF, 1)**2) * cnt(0:NF, i)
        snt(0:NF, i) = -(nn(0:NF, 1)**2) * snt(0:NF, i)
     enddo
  case default
     STOP "Invalid order in subroutine fouriermatrix"
  end select
  !----------------------------final multiplication---------------------------------------------
  tx = matmul(tc, cnt) + matmul(ts, snt)
  xx(0:ND) = tx(1, 0:ND)

  return
END subroutine fouriermatrix

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

SUBROUTINE readcoils(filename, maxnseg)
  use globals, only : zero, coilsX, coilsY, coilsZ, coilsI, coilseg, coilname, Ncoils, ounit, myid
  implicit none
  include "mpif.h"

  INTEGER, parameter         :: mcoil = 256, mseg = 1024 ! Largest coils and segments number
  INTEGER                    :: icoil, cunit, istat, astat, lstat, ierr, maxnseg, seg(1:mseg)
  REAL, dimension(mseg,mcoil):: x, y, z, I
  CHARACTER*100              :: filename
  CHARACTER*200              :: line
  REAL                       :: tmp
  CHARACTER (LEN=20), dimension(mcoil) :: name

  cunit = 99; I = 1.0; Ncoils= 1; maxnseg = 0; seg = 0;

  open(cunit,FILE=trim(filename),STATUS='old',IOSTAT=istat)
  if ( istat .ne. 0 ) then
     write(ounit,'("rdcoils : Reading coils data error in "A)') trim(filename)
     call MPI_ABORT( MPI_COMM_WORLD, 1, ierr )
  endif
     
  ! read coils and segments data
  read(cunit,*)
  read(cunit,*)
  read(cunit,*)

  do
     read(cunit,'(a)', IOSTAT = istat) line
     if(istat .ne. 0 .or. line(1:3) == 'end') exit !detect EOF or end

     seg(Ncoils) = seg(Ncoils) + 1
     read(line,*, IOSTAT = lstat) x(seg(Ncoils), Ncoils), y(seg(Ncoils), Ncoils), z(seg(Ncoils), Ncoils), &
          I(seg(Ncoils), Ncoils)
     if ( I(seg(Ncoils), Ncoils) == 0.0 ) then
        seg(Ncoils) = seg(Ncoils) - 1  !remove the duplicated last point
        read(line, *, IOSTAT = lstat) tmp, tmp, tmp, tmp, tmp, name(Ncoils)
        Ncoils = Ncoils + 1
     endif
  enddo

  close(cunit)

  Ncoils = Ncoils - 1
  maxnseg = maxval(seg)
  FATAL( readcoils, Ncoils  .le. 0 .or. Ncoils   >  mcoil, illegal )
  FATAL( readcoils, maxnseg .le. 0 .or. maxnseg  >  mseg , illegal )

#ifdef DEBUG
  write(ounit,'("rdcoils : Finding " I4 " coils and maximum segments number is " I6)') &
       Ncoils, maxnseg
  do icoil = 1, Ncoils
     write(ounit, '("rdcoils : Number of segments in coil " I4 " is " I6)') icoil, seg(icoil)
  enddo
#endif

  SALLOCATE( coilsX  , (1:maxnseg, 1:Ncoils), zero )
  SALLOCATE( coilsY  , (1:maxnseg, 1:Ncoils), zero )
  SALLOCATE( coilsZ  , (1:maxnseg, 1:Ncoils), zero )
  SALLOCATE( coilsI  , (1:Ncoils)           , zero )
  SALLOCATE( coilseg , (1:Ncoils)           ,    0 )
  SALLOCATE( coilname, (1:Ncoils)           , ''   )

  coilsX( 1:maxnseg, 1:Ncoils) = x( 1:maxnseg, 1:Ncoils)
  coilsY( 1:maxnseg, 1:Ncoils) = y( 1:maxnseg, 1:Ncoils)
  coilsZ( 1:maxnseg, 1:Ncoils) = z( 1:maxnseg, 1:Ncoils)
  coilsI(            1:Ncoils) = I( 1        , 1:Ncoils)
  coilseg(           1:Ncoils) = seg(          1:Ncoils)
  coilname(          1:Ncoils) = name(         1:Ncoils)

  return

end SUBROUTINE READCOILS

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

SUBROUTINE Fourier( X, XFC, XFS, Nsegs, NFcoil)
  use globals, only: ounit, zero, pi2, half, myid
  implicit none
  include "mpif.h"

  REAL    :: X(1:Nsegs), XFC(0:NFcoil), XFS(0:NFcoil)
  INTEGER :: Nsegs, NFcoil, ifou, iseg, funit, ierr
  REAL, allocatable:: A(:), B(:)

  allocate(A(0:Nsegs-1))
  allocate(B(0:Nsegs-1))

  FATAL(Fourier, Nsegs < 2*NFcoil, Nsegs too small)
  A = zero; B = zero

  do ifou = 0, Nsegs-1

     do iseg = 1, Nsegs
        A(ifou) = A(ifou) + X(iseg)*cos(ifou*pi2*(iseg-1)/Nsegs)
        B(ifou) = B(ifou) + X(iseg)*sin(ifou*pi2*(iseg-1)/Nsegs)
     enddo

  enddo

  A = 2.0/Nsegs * A; A(0) = half*A(0)
  B = 2.0/Nsegs * B

  XFC(0:NFcoil) = A(0:NFcoil)
  XFS(0:NFcoil) = B(0:NFcoil)

  deallocate(A, B)

  return

end SUBROUTINE Fourier

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!
