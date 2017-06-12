
!title (initialize) ! Read input file, initialize global variables.

!latex \briefly{Reads input files broadcasts, and allocates/initializes global variables.}

!latex \calledby{\link{focus}, \link{globals}}
!latex \calls{\link{}}

!latex \section{Input namelist}
!latex  The \emph{focusin} namelist is the only input namelist needed for FOCUS running. It should be
!latex  written from the file \emph{example.input}. Here are the details for the variables. \\
!latex  \bi
!latex  \item \inputvar{IsQuiet = -1} \\
!latex    \textit{Information displayed to the user} \\
!latex    \bi
!latex    \item[-1:] more details;
!latex    \item[0:]  essential;
!latex    \item[1:] concise.
!latex    \ei
!latex 
!latex  \item \inputvar{IsSymmetric = 0} \\
!latex    \textit{Enforce stellarator symmetry or not} \\
!latex    \bi
!latex    \item[0:] no stellarator symmetry enforced;
!latex    \item[1:] periodicty enforced;
!latex    \item[2:] fully stellarator symmetry enforced. 
!latex    \ei
!latex 
!latex  \par \begin{tikzpicture} \draw[dashed] (0,1) -- (10,1); \end{tikzpicture}
!latex 
!latex  \item \inputvar{case\_surface = 0} \\
!latex    \textit{Specify the input plasma boundary format} \\
!latex    \bi
!latex    \item[0:] general VMEC-like format (Rbc, Rbs, Zbc, Zbs), seen in \link{rdsurf};
!latex    \item[1:] read axis for knots, seen in \link{rdknot};
!latex    \item[2:] read wout file from VMEC outputs, seen in \link{rdvmec}.
!latex    \ei
!latex 
!latex  \item \inputvar{knotsurf = 0.2} \\
!latex    \textit{Minor plasma radius for knototrans, only valid for \inputvar{case\_surface = 1}} 
!latex 
!latex  \item \inputvar{ellipticity = 0.0} \\
!latex    \textit{Ellipticity of plasma for knototrans, only valid for \inputvar{case\_surface = 1}} 
!latex 
!latex  \item \inputvar{Nteta = 64} \\
!latex    \textit{Poloidal resolution for discritizing the plasma} 
!latex 
!latex  \item \inputvar{Nzeta = 64} \\
!latex    \textit{Toroidal resolution for discritizing the plasma} 
!latex 
!latex  \par \begin{tikzpicture} \draw[dashed] (0,1) -- (10,1); \end{tikzpicture}
!latex 
!latex  \item \inputvar{case\_init = 0} \\
!latex    \textit{Specify the initializing method for coils, seen in \link{rdcoils}} \\
!latex    \bi
!latex    \item[-1:] read the standard \emph{coils.example} file and transform to FOCUS format;
!latex    \item[0:] read FOCUS format data in \emph{example.focus};
!latex    \item[1:] toroidally spaced \inputvar{Ncoils} circular coils with radius of \inputvar{init\_radius};
!latex    \ei
!latex 
!latex  \item \inputvar{case\_coils = 1} \\
!latex    \textit{Specify representation used for the initial coils, seen in \link{rdcoils}} \\
!latex    \bi
!latex    \item[0:] using piecewise linear representation;
!latex    \item[1:] using Fourier series representation;
!latex    \ei
!latex 
!latex  \item \inputvar{Ncoils = 0} \\
!latex    \textit{Number of coils initilized, only valid for \inputvar{case\_init = 1}}
!latex 
!latex  \item \inputvar{init\_current = 1.0E6} \\
!latex    \textit{Initial coil current, only valid for \inputvar{case\_init = 1}} 
!latex 
!latex  \item \inputvar{init\_radius = 1.0} \\
!latex    \textit{Initial coil current, only valid for \inputvar{case\_init = 1}} 
!latex 
!latex  \item \inputvar{IsVaryCurrent = 1} \\
!latex    \textit{Keep coil currents fixed or not, overriden by \emph{example.focus}} \\
!latex    \bi
!latex    \item[0:] coil currents being fixed;
!latex    \item[1:] coil currents can be varied;
!latex    \ei
!latex 
!latex  \item \inputvar{IsVaryGeometry = 1} \\
!latex    \textit{Keep coil geometries fixed or not, overriden by \emph{example.focus}} \\
!latex    \bi
!latex    \item[0:] coil geometries being fixed;
!latex    \item[1:] coil geometries can be varied;
!latex    \ei
!latex 
!latex  \item \inputvar{NFcoil = 4} \\
!latex    \textit{Number of Fourier coefficients for coils, only valid for \inputvar{case\_coils = 1},
!latex           overriden by \emph{example.focus}} 
!latex 
!latex  \item \inputvar{Nseg = 128} \\
!latex    \textit{Number of segments for discritizing coils, only valid for \inputvar{case\_coils = 1},
!latex           overriden by \emph{example.focus}} 
!latex 
!latex  \par \begin{tikzpicture} \draw[dashed] (0,1) -- (10,1); \end{tikzpicture}
!latex 
!latex  \item \inputvar{IsNormalize = 1} \\
!latex    \textit{Normalizing coil parameters or not} \\
!latex    \bi
!latex    \item[0:] keep raw data (normalized to 1.0);
!latex    \item[1:] currents being normalized to averaged absolute current,
!latex              coil geometry parameters being normalized to major radius;
!latex    \ei
!latex 
!latex  \item \inputvar{IsNormWeight = 1} \\
!latex    \textit{each constraints normalized to initial value or not} \\
!latex    \bi
!latex    \item[0:] keep raw value for constraints;
!latex    \item[1:] $w = w/f_0$ weights normalized to the initial values of each constraints;
!latex    \ei
!latex 
!latex  \item \inputvar{case\_bnormal = 0} \\
!latex    \textit{ Bn error normalized to $|B|$ or not} \\
!latex    \bi
!latex    \item[0:] keep raw Bn error;
!latex    \item[1:] Bn residue normalized to local $|B|$;
!latex    \ei
!latex 
!latex  \item \inputvar{case\_coillen = 0} \\
!latex    \textit{ options for constructing coil length constraint, seen in \link{coillen}} \\
!latex    \bi
!latex    \item[1:] quadratic format, converging the target\_length;
!latex    \item[2:] exponential format, as short as possible;
!latex    \ei
!latex 
!latex  \item \inputvar{weight\_bnorm = 1.0} \\
!latex    \textit{weight for Bn error, if zero, turned off; seen in \link{bnormal}} 
!latex 
!latex  \item \inputvar{weight\_bharm = 0.0} \\
!latex    \textit{weight for Bn Fourier harmonics error, if zero, turned off; seen in \link{bmnharm}} 
!latex 
!latex  \item \inputvar{weight\_tflux = 0.0} \\
!latex    \textit{weight for toroidal flux error, if zero, turned off; seen in \link{torflux}} 
!latex 
!latex  \item \inputvar{target\_tflux = 0.0} \\
!latex    \textit{target value for the toroidal flux, if zero, automatically set to initial $\Psi_{ave}$; 
!latex    seen in \link{solvers}} 
!latex 
!latex  \item \inputvar{weight\_ttlen = 0.0} \\
!latex    \textit{weight for coils length error, if zero, turned off; seen in \link{coillen}} 
!latex 
!latex  \item \inputvar{target\_tflux = 0.0} \\
!latex    \textit{target value (or for normalization) of the coils length, if zero, automatically 
!latex    set to initial actual length; seen in \link{rdcoils}} 
!latex 
!latex  \item \inputvar{weight\_specw = 0.0} \\
!latex    \textit{weight for spectral condensation error, if zero, turned off; seen in \link{specwid}} 
!latex 
!latex  \item \inputvar{weight\_ccsep = 0.0} \\
!latex    \textit{weight for coil-coil separation constraint, if zero, turned off; seen in \link{coilsep}} 
!latex
!latex  \par \begin{tikzpicture} \draw[dashed] (0,1) -- (10,1); \end{tikzpicture}


!latex  \ei

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

subroutine initial

  use globals
  implicit none
  include "mpif.h"

  LOGICAL :: exist
  INTEGER :: icpu

  !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

  machprec = epsilon(pi)         ! get the machine precision
  sqrtmachprec = sqrt(machprec)  ! sqrt of machine precision
  vsmall = ten * machprec        ! very small number
  small = thousand * machprec    ! small number

  !-------------read input namelist----------------------------------------------------------------------
  if( myid == 0 ) then ! only the master node reads the input; 25 Mar 15;
     call getarg(1,ext) ! get argument from command line
     write(ounit, '("initial : machine_prec   = ", ES12.5, " ; sqrtmachprec   = ", ES12.5,   &
          & " ; ext = ", A)') machprec, sqrtmachprec, trim(ext)     
  endif

  !-------------IO files name ---------------------------------------------------------------------------
  ClBCAST( ext           ,  100,  0 )
  inputfile = trim(ext)//".input"
  surffile  = "plasma.boundary"
  knotfile  = trim(ext)//".knot"
  coilfile  = trim(ext)//".focus"
  harmfile  = trim(ext)//".harmonics"
  hdf5file  = "focus_"//trim(ext)//".h5"
  inpcoils  = "coils."//trim(ext)
  outcoils  = trim(ext)//"coils."

  !-------------read the namelist-----------------------------------------------------------------------
  if( myid == 0 ) then
     inquire(file=trim(inputfile), EXIST=exist) ! inquire if inputfile existed;
     FATAL( initial, .not.exist, input file ext.input not provided )
  endif

  do icpu = 1, ncpu
     call MPI_BARRIER( MPI_COMM_WORLD, ierr )
     if (myid == icpu-1) then                              ! each cpu read the namelist in turn;
        open(runit, file=trim(inputfile), status="old", action='read')
        read(runit, focusin)
        close(runit)
     endif ! end of if( myid == 0 )
  enddo

  !-------------show the namelist for checking----------------------------------------------------------

  if (myid == 0 .and. IsQuiet < 1) then ! Not quiet to output more informations;

     write(ounit, *) "-----------INPUT NAMELIST------------------------------------"
     write(ounit, '(3A, I1)')"initial : Read focusin from ", trim(inputfile),  " ; IsQuiet = ", IsQuiet

     select case (IsSymmetric)
     case (0)
        if (IsQuiet < 0) write(ounit, '(8X,": IsSymmetric = "I1, &
             " ; No stellarator symmetry or periodicity enforced.")') IsSymmetric
     case default
        FATAL( initial, IsSymmetric /= 0, Stellarator symmetry is not activated yet)
     end select

     select case (case_surface)
     case (0)
        inquire( file=trim(surffile), exist=exist )
        FATAL( initial, .not.exist, plasma boundary file not provided )
        if (IsQuiet < 0)  write(ounit, '(8X,": case_surface = "I1, &
             " ; FOCUS will read VMEC-like Fourier harmonics in "A)') case_surface, trim(surffile)
     case (1)
        inquire( file=trim(knotfile), exist=exist )
        FATAL( initial, .not.exist, axis file not provided )
        FATAL( initial, knotsurf < zero, illegal minor radius)
        if (IsQuiet < 0)  write(ounit, '(8X,": case_surface = "I1, &
             " ; FOCUS will read axis info in "A)') case_surface, trim(knotfile)
        if (IsQuiet < 0)  write(ounit, '(8X,": knotsurf = " ES12.5 &
             " ; ellipticity = " ES12.5)') knotsurf, ellipticity
     case default
        FATAL( initial, .true., selected surface type is not supported )
     end select

     FATAL( initial, Nteta   <=   0, illegal surface resolution )
     FATAL( initial, Nzeta   <=   0, illegal surface resolution )
     write(ounit, '(8X,": plasma boundary will be discretized in "I6" X "I6)') Nteta, Nzeta

     select case( case_init )
     case(-1 )
        inquire( file=trim(inpcoils), exist=exist )
        FATAL( initial, .not.exist, coils file coils.ext not provided )
        FATAL( initial, NFcoil <= 0    , no enough harmonics )
        FATAL( initial, Nseg   <= 0    , no enough segments  )
        write(ounit, '(8X,": read coils data in" A )') trim(inpcoils)
        if (IsQuiet < 0) write(ounit, '(8X,": NFcoil = "I3" ; IsVaryCurrent = "I1 &
             " ; IsVaryGeometry = "I1)') NFcoil, IsVaryCurrent, IsVaryGeometry
        write(ounit, '(8X,": coils will be discretized in "I6" segments")') Nseg
     case( 0 )
        inquire( file=trim(coilfile), exist=exist )
        FATAL( initial, .not.exist, FOCUS coil file ext.focus not provided )
        write(ounit, '(8X,": read coil parameters in" A )') trim(coilfile)
     case( 1 )
        FATAL( initial, Ncoils < 1, should provide the No. of coils)
        FATAL( initial, init_current == zero, invalid coil current)
        FATAL( initial, init_radius < zero, invalid coil radius)
        FATAL( initial, NFcoil <= 0    , no enough harmonics )
        FATAL( initial, Nseg   <= 0    , no enough segments  )
        write(ounit, '(8X,": Initialize "I4" circular coils with r="ES12.5"m ; I="ES12.5" A")') &
             Ncoils, init_radius, init_current
        if (IsQuiet < 0) write(ounit, '(8X,": NFcoil = "I3" ; IsVaryCurrent = "I1 &
             " ; IsVaryGeometry = "I1)') NFcoil, IsVaryCurrent, IsVaryGeometry
        write(ounit, '(8X,": coils will be discretized in "I6" segments")') Nseg
     case default
        FATAL( initial, .true., selected case_init is not supported )
     end select

     FATAL( initial, case_coils /= 1, only fourier representation is valid )
     if (IsQuiet < 0) write(ounit, '(8X,": case_coils = "I1 &
          "using Fourier series as the basic representation.")') case_coils

     write(ounit, '(8X,": IsNormalize = "I1" ; IsNormWeight = "I1)') IsNormalize, IsNormWeight

     select case ( case_bnormal )
     case ( 0 )
        write(ounit, '(8X,": case_bnormal = "I1" ; no normalization on Bn")') case_bnormal
     case ( 1 )
        write(ounit, '(8X,": case_bnormal = "I1" ; Bn normalized to |B|")') case_bnormal
     case default
        FATAL( initial, .true., selected case_bnormal is not supported )
     end select

     select case ( case_length )
     case ( 1 )
        write(ounit, '(8X,": case_bnormal = "I1" ; quadratic format of length constraint")') case_length
     case ( 2 )
        write(ounit, '(8X,": case_bnormal = "I1" ; exponential format of length constraint")') case_length
     case default
        FATAL( initial, .true., selected case_length is not supported )
     end select

     FATAL( initial, weight_bnorm  < zero, illegal )
     FATAL( initial, weight_bharm  < zero, illegal )
     FATAL( initial, weight_tflux  < zero, illegal )
     FATAL( initial, weight_ttlen  < zero, illegal )
     FATAL( initial, weight_specw  < zero, illegal )
     FATAL( initial, weight_ccsep  < zero, illegal )
     write(ounit, '(8X,": weights are set as: "6A12)') "bnorm", "bharm", "tflux", "ttlen", "specw", "ccsep"
     write(ounit, '(8X,": "20X,6ES12.5)') weight_bnorm, weight_bharm, weight_tflux, weight_ttlen, &
          weight_specw, weight_ccsep

     FATAL( initial, target_length  < zero, illegal )
     write(ounit, '(8X,": target_tflux = "ES12.5" ; target_length = "ES12.5)') target_tflux, target_length

     if (solver_DF == 1) then
        FATAL( initial, DF_maxiter <= 0, must be positive )
        write(ounit, '(8X,": Differential flow is used")')
        if (IsQuiet < 0) write(ounit,'(8X,": DF_tausta = "ES12.5" ; DF_tauend = "ES12.5 &
             " ; DF_xtol = "ES12.5" ; DF_maxiter = "I6)') DF_tausta, DF_tauend, DF_xtol, DF_maxiter
     endif
                       
     if (solver_CG == 1) then
        FATAL( initial, CG_maxiter <= 0, must be positive )
        FATAL( Initial, CG_wolfe_c1 <= zero .or. CG_wolfe_c1 >= one, should be 0<c1<1 )
        FATAL( Initial, CG_wolfe_c2 <= zero .or. CG_wolfe_c2 >= one, should be 0<c2<1 )
        FATAL( Initial, CG_wolfe_c1 >= CG_wolfe_c2, should be c1<c2)
        write(ounit, '(8X,": Nonlinear Conjugate Gradient method is used")')
        if (IsQuiet < 0) write(ounit,'(8X,": CG_wolfe_c1 = "ES12.5" ; CG_wolfe_c2 = "ES12.5" ; CG_xtol = " &
             ES12.5" ; CG_maxiter = "ES12.5)') CG_wolfe_c1, CG_wolfe_c2, CG_xtol, CG_maxiter
     endif

     if (solver_HN == 1) then
        FATAL( initial, HN_maxiter <= 0, must be positive )
        write(ounit, '(8X,": Hybrid Newton method is used")')
        if (IsQuiet < 0) write(ounit,'(8X,": HN_factor = "ES12.5" ; HN_xtol = " &
             ES12.5" ; HN_maxiter = "I6)') HN_factor, HN_xtol, HN_maxiter
     endif

     if (solver_TN == 1) then
        FATAL( initial, TN_maxiter <= 0, must be positive )
        FATAL( Initial, TN_cr <= zero .or. TN_cr > one, should be 0<cr<=1 )
        write(ounit, '(8X,": Truncated Newton method is used")')
        if (IsQuiet < 0) write(ounit,'(8X,": TN_cr = "ES12.5" ; TN_reorder = "I1" ; TN_xtol = " &
             ES12.5" ; TN_maxiter = "I6)') TN_cr, TN_reorder, TN_xtol, TN_maxiter
     endif

     select case ( case_postproc )
     case ( 0 )
        write(ounit,'(8X,": no extra post-processings")')
     case ( 1 )
        write(ounit,'(8X,": coil evaluation would be performed")')
     case default
        FATAL( initial, .true., selected case_postproc is not supported )
     end select
        
     FATAL( initial, save_freq < 0, should not be negative )
     write(ounit, '(8X,": files saving setteings: freq = "I4" ; coils = "I1" ; harmonics = "I1 &
          " ; filaments = " I1)') save_freq, save_coils, save_harmonics, save_filaments
     if (IsQuiet < 0) then
        write(ounit,'(8X,5A)') ": ", trim(coilfile), " and ", trim(hdf5file), "will be stored."
        if (save_coils /= 0) write(ounit,'(8X, 3A)') ": new coils file ", trim(outcoils), "will be updated."
        if (save_harmonics /= 0) write(ounit,'(8X,3A)')": Bmn harmonics file ",  trim(harmfile), "will be updated."
      endif

  endif

  FATAL( initial, ncpu >= 1000 , too macy cpus, modify nodelabel)
  write(nodelabel,'(i3.3)') myid ! nodelabel is global; 30 Oct 15;

  ! initialize iteration and total iterations;
  iout = 0
  Nouts = solver_DF*DF_maxiter + solver_CG*CG_maxiter + solver_HN*HN_maxiter + solver_TN*TN_maxiter

  discretefactor = (pi2/Nteta) * (pi2/Nzeta)
  
  !save weights before normalized
  tmpw_bnorm = weight_bnorm
  tmpw_bharm = weight_bharm
  tmpw_tflux = weight_tflux
  tmpt_tflux = target_tflux
  tmpw_ttlen = weight_ttlen
  tmpw_specw = weight_specw
  tmpw_ccsep = weight_ccsep

  return

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

end subroutine initial
