
!title (boundary) ! The plasma boundary is read from file 

!latex \briefly{A Fourier representation for the plasma boundary is read from file }

!latex \calledby{\link{xfocus}}
!latex \calls{\link{}}

!latex \section{General representation (stellarator)}
!latex \subsection{overview}
!latex The general representation for plasma boundary is in \subroutine{generic}. The basic fomulation
!latex is
!latex \be
!latex \ds R &= \sum R_{mn}^c \, \cos(m\t - n\z) + R_{mn}^s \, \sin(m\t - n\z) \nonumber \\
!latex \ds Z &= \sum Z_{mn}^c \, \cos(m\t - n\z) + Z_{mn}^s \, \sin(m\t - n\z) \nonumber
!latex \ee
!latex Usually, if we imply stellarator symmetry, then $R_{mn}^s$ and $Z_{mn}^c$ would be zero.
!latex 
!latex The positive driection for poloidal angle $\t$ is \red{counterclockwise} and for toroidal angle is also
!latex \red{counterclockwise} from the top view. The positive surface normal should be pointed outwards.
!latex \subsection{Variables}
!latex The Fourier harmonics of the plasma boundary are  reqired in \verb+plasma.boundary+, 
!latex and the format of this file is as follows: \begin{verbatim}
!latex Nfou       ! integer: number of Fourier harmonics for the plasma boundary;
!latex Nfp        ! integer: number of field periodicity;
!latex NBnf       ! integer: number of Fourier harmonics for Bn;
!latex ---------------------------------------------------------
!latex bim(1:bmn) ! integer: poloidal mode identification;
!latex bin(1:bmn) ! integer: toroidal mode identification;
!latex Bnim(1:bmn)! integer: poloidal mode identification, for Bn;
!latex Bnin(1:bmn)! integer: toroidal mode identification, for Bn;
!latex ---------------------------------------------------------
!latex Rbc(1:bmn) ! real   : cylindrical R cosine harmonics;
!latex Rbs(1:bmn) ! real   : cylindrical R   sine harmonics;
!latex Zbc(1:bmn) ! real   : cylindrical Z cosine harmonics;
!latex Zbs(1:bmn) ! real   : cylindrical Z   sine harmonics;
!latex Bns(1:nbf) ! real   : B normal sin harmonics;
!latex Bnc(1:nbf) ! real   : B normal cos harmonics; \end{verbatim}
!latex Note that immediately after reading (and broadcasting) 
!latex \verb+bin+, the field periodicity factor is included, i.e. \verb+bin = bin * Nfp+.
!latex \subsection{Sample file}
!latex Example of the plasma.boundary file:
!latex  { \begin{verbatim}
!latex #Nfou Nfp NBnf
!latex 4 2 1
!latex #plasma boundary
!latex # n m Rbc Rbs Zbc Zbs
!latex 0 0  3.00 0.0 0.0  0.00
!latex 0 1  0.30 0.0 0.0 -0.30
!latex 1 0  0.00 0.0 0.0 -0.06
!latex 1 1 -0.06 0.0 0.0 -0.06
!latex #Bn harmonics
!latex # n m bnc bns
!latex 0 0 0.0 0.0
!latex \end{verbatim}
!latex }
!latex \section{Knotran}
!latex The input surface file for knotrans is descriped in \subroutine{knotxx}.
!latex \section{Tokamak}
!latex This part is reserved for later accomplishment of the interface for tokamaks.

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

subroutine generic
  
  use globals, only : zero, half, pi2, myid, ncpu, ounit, runit, ext, IsQuiet, IsSymmetric, &
                      Nfou, Nfp, NBnf, bim, bin, Bnim, Bnin, Rbc, Rbs, Zbc, Zbs, Bnc, Bns,  &
                      Nteta, Nzeta, surf
  
  implicit none
  
  include "mpif.h"
  
!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!
  
  LOGICAL :: exist
  INTEGER :: iosta, astat, ierr, ii, jj, imn
  REAL    :: RR(0:2), ZZ(0:2), szeta, czeta, xx(1:3), xt(1:3), xz(1:3), ds(1:3), &
             teta, zeta, arg, dd, tmp
  
  !-------------read plasma.boundary---------------------------------------------------------------------  
  inquire( file="plasma.boundary", exist=exist)  
  FATAL( surface, .not.exist, plasma.boundary does not exist ) 
  if( myid == 0 ) then
     open(runit, file="plasma.boundary", status='old', action='read')
     read(runit,*) !empty line
     read(runit,*) Nfou, Nfp, NBnf !read dimensions
  endif
  
  !Broadcast the values
  IlBCAST( Nfou , 1, 0 )
  IlBCAST( Nfp  , 1, 0 )
  IlBCAST( NBnf , 1, 0 )  
  FATAL( surface, Nfou <= 0, invalid )
  FATAL( surface, Nfp  <= 0, invalid )
  FATAL( surface, NBnf <  0, invalid )

  !allocate arrays
  SALLOCATE( bim, (1:Nfou), 0 )
  SALLOCATE( bin, (1:Nfou), 0 )  
  SALLOCATE( Rbc, (1:Nfou), zero )
  SALLOCATE( Rbs, (1:Nfou), zero )
  SALLOCATE( Zbc, (1:Nfou), zero )
  SALLOCATE( Zbs, (1:Nfou), zero )

  if( myid == 0 ) then
   read(runit,*) !empty line
   read(runit,*) !empty line
   do imn = 1, Nfou
      read(runit,*) bin(imn), bim(imn), Rbc(imn), Rbs(imn), Zbc(imn), Zbs(imn)
   enddo
  endif  

  IlBCAST( bim(1:Nfou), Nfou, 0 )
  IlBCAST( bin(1:Nfou), Nfou, 0 )
  
  if (IsSymmetric  ==  0) then 
     bin(1:Nfou) = bin(1:Nfou) * Nfp  !Disarde periodicity
     Nfp = 1                          !reset Nfp to 1
  endif
  
  
  RlBCAST( Rbc(1:Nfou), Nfou, 0 )
  RlBCAST( Rbs(1:Nfou), Nfou, 0 )
  RlBCAST( Zbc(1:Nfou), Nfou, 0 )
  RlBCAST( Zbs(1:Nfou), Nfou, 0 )

  !read Bnormal ditributions
  if( NBnf  >   0) then
     SALLOCATE( Bnim, (1:NBnf), 0    )
     SALLOCATE( Bnin, (1:NBnf), 0    )
     SALLOCATE( Bnc , (1:NBnf), zero )
     SALLOCATE( Bns , (1:NBnf), zero )

     if( myid == 0 ) then
        read(runit,*) !empty line
        read(runit,*) !empty line
        do imn = 1, NBnf 
           read(runit,*) Bnin(imn), Bnim(imn), Bnc(imn), Bns(imn)
        enddo
     endif

     IlBCAST( Bnim(1:NBnf), NBnf, 0 )
     IlBCAST( Bnin(1:NBnf), NBnf, 0 )

     !if (IsSymmetric  ==  0) Bnin(1:NBnf) = Bnin(1:NBnf) * Nfp ! Disarde periodicity;
     ! This should be consistent with bnftran; Before fully constructed the stellarator symmetry,
     ! it's turned off;
     
     RlBCAST( Bnc(1:NBnf) , NBnf, 0 )
     RlBCAST( Bns(1:NBnf) , NBnf, 0 )
  endif

  if( myid == 0 ) close(runit,iostat=iosta)
  
  IlBCAST( iosta, 1, 0 )
  
  FATAL( surface, iosta.ne.0, error closing plasma.boundary )
  
  !-------------output for check-------------------------------------------------------------------------
  if( myid == 0 .and. IsQuiet <= 0) then
     write(ounit, *) "-----------Reading surface-----------------------------------"
     write(ounit, '("surface : Nfou = " I06 " ; Nfp = " I06 " ; NBnf = " I06 " ;" )') Nfou, Nfp, NBnf
  endif

  if( myid == 0 .and. IsQuiet <= -2) then !very detailed output;
     write(ounit,'("        : " 10x " : bim ="10i13   )') bim(1:Nfou)
     write(ounit,'("        : " 10x " : bin ="10i13   )') bin(1:Nfou)
     write(ounit,'("        : " 10x " : Rbc ="10es13.5)') Rbc(1:Nfou)
     write(ounit,'("        : " 10x " : Rbs ="10es13.5)') Rbs(1:Nfou)
     write(ounit,'("        : " 10x " : Zbc ="10es13.5)') Zbc(1:Nfou)
     write(ounit,'("        : " 10x " : Zbs ="10es13.5)') Zbs(1:Nfou)
     if(Nbnf > 0) then
        write(ounit,'("        : " 10x " : Bnim ="10i13  )') Bnim(1:NBnf)
        write(ounit,'("        : " 10x " : Bnin ="10i13  )') Bnin(1:NBnf)
        write(ounit,'("        : " 10x " : Bnc ="10es13.5)') Bnc (1:NBnf)
        write(ounit,'("        : " 10x " : Bns ="10es13.5)') Bns (1:NBnf)
     endif
  endif

  !-------------discretize surface data------------------------------------------------------------------  
  allocate( surf(1:1) ) ! can allow for myltiple plasma boundaries 
                        ! if multiple currents are allowed; 14 Apr 16;
  
  surf(1)%Nteta = Nteta ! not used yet; used for multiple surfaces; 20170307;
  surf(1)%Nzeta = Nzeta ! not used yet; used for multiple surfaces; 20170307;
  
  SALLOCATE( surf(1)%xx, (0:Nteta-1,0:Nzeta-1), zero ) !x coordinates;
  SALLOCATE( surf(1)%yy, (0:Nteta-1,0:Nzeta-1), zero ) !y coordinates
  SALLOCATE( surf(1)%zz, (0:Nteta-1,0:Nzeta-1), zero ) !z coordinates
  SALLOCATE( surf(1)%nx, (0:Nteta-1,0:Nzeta-1), zero ) !unit nx;
  SALLOCATE( surf(1)%ny, (0:Nteta-1,0:Nzeta-1), zero ) !unit ny;
  SALLOCATE( surf(1)%nz, (0:Nteta-1,0:Nzeta-1), zero ) !unit nz;
  SALLOCATE( surf(1)%ds, (0:Nteta-1,0:Nzeta-1), zero ) !jacobian;
  SALLOCATE( surf(1)%xt, (0:Nteta-1,0:Nzeta-1), zero ) !dx/dtheta;
  SALLOCATE( surf(1)%yt, (0:Nteta-1,0:Nzeta-1), zero ) !dy/dtheta;
  SALLOCATE( surf(1)%zt, (0:Nteta-1,0:Nzeta-1), zero ) !dz/dtheta;
  SALLOCATE( surf(1)%pb, (0:Nteta-1,0:Nzeta-1), zero ) !target Bn;
 
! The center point value was used to discretize grid;
  do ii = 0, Nteta-1; teta = ( ii + half ) * pi2 / Nteta
   do jj = 0, Nzeta-1; zeta = ( jj + half ) * pi2 / Nzeta
    
    RR(0:2) = zero ; ZZ(0:2) = zero
    
    do imn = 1, Nfou ; arg = bim(imn) * teta - bin(imn) * zeta
     
     RR(0) =  RR(0) +     Rbc(imn) * cos(arg) + Rbs(imn) * sin(arg)
     ZZ(0) =  ZZ(0) +     Zbc(imn) * cos(arg) + Zbs(imn) * sin(arg)
     
     RR(1) =  RR(1) + ( - Rbc(imn) * sin(arg) + Rbs(imn) * cos(arg) ) * bim(imn)
     ZZ(1) =  ZZ(1) + ( - Zbc(imn) * sin(arg) + Zbs(imn) * cos(arg) ) * bim(imn)
     
     RR(2) =  RR(2) - ( - Rbc(imn) * sin(arg) + Rbs(imn) * cos(arg) ) * bin(imn)
     ZZ(2) =  ZZ(2) - ( - Zbc(imn) * sin(arg) + Zbs(imn) * cos(arg) ) * bin(imn)
     
    enddo ! end of do imn; 30 Oct 15;
    
    szeta = sin(zeta)
    czeta = cos(zeta)
    
    xx(1:3) = (/   RR(0) * czeta,   RR(0) * szeta, ZZ(0) /)
    xt(1:3) = (/   RR(1) * czeta,   RR(1) * szeta, ZZ(1) /)
    xz(1:3) = (/   RR(2) * czeta,   RR(2) * szeta, ZZ(2) /) + (/ - RR(0) * szeta,   RR(0) * czeta, zero  /)

    ds(1:3) = -(/ xt(2) * xz(3) - xt(3) * xz(2), & ! minus sign for theta counterclockwise direction;
                  xt(3) * xz(1) - xt(1) * xz(3), &
                  xt(1) * xz(2) - xt(2) * xz(1) /)

    dd = sqrt( sum( ds(1:3)*ds(1:3) ) )

    ! x, y, z coordinates for the surface;
    surf(1)%xx(ii,jj) = xx(1)
    surf(1)%yy(ii,jj) = xx(2)
    surf(1)%zz(ii,jj) = xx(3)

    ! dx/dt, dy/dt, dz/dt (dt for d theta)
    surf(1)%xt(ii,jj) = xt(1)
    surf(1)%yt(ii,jj) = xt(2)
    surf(1)%zt(ii,jj) = xt(3)

    ! surface normal vectors and ds for the jacobian;
    surf(1)%nx(ii,jj) = ds(1) / dd
    surf(1)%ny(ii,jj) = ds(2) / dd
    surf(1)%nz(ii,jj) = ds(3) / dd
    surf(1)%ds(ii,jj) =         dd

   enddo ! end of do jj; 14 Apr 16;
  enddo ! end of do ii; 14 Apr 16;

  !calculate target Bn with input harmonics; 05 Jan 17;
  if(NBnf >  0) then

     do jj = 0, Nzeta-1 ; zeta = ( jj + half ) * pi2 / Nzeta
        do ii = 0, Nteta-1 ; teta = ( ii + half ) * pi2 / Nteta 
           do imn = 1, NBnf
              arg = Bnim(imn) * teta - Bnin(imn) * zeta
              surf(1)%pb(ii,jj) = surf(1)%pb(ii,jj) + Bnc(imn)*cos(arg) + Bns(imn)*sin(arg)
           enddo
        enddo
     enddo

  endif
  
  
  return
  
end subroutine generic

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

subroutine surfcoord( theta, zeta, r, z)
  use globals, only: zero, Nfou, Nfp, bim, bin, Rbc, Rbs, Zbc, Zbs
  implicit none
  include "mpif.h"

  REAL, INTENT(in ) :: theta, zeta
  REAL, INTENT(out) :: r, z

  INTEGER           :: imn
  REAL              :: arg, carg, sarg
  !-------------calculate r, z coodinates for theta, zeta------------------------------------------------  
  if( .not. allocated(bim) ) STOP  "please allocate surface data first!"

  r = zero; z = zero  
  do imn = 1, Nfou
     arg = bim(imn) * theta - bin(imn) * zeta
     R =  R + Rbc(imn) * cos(arg) + Rbs(imn) * sin(arg)
     Z =  Z + Zbc(imn) * cos(arg) + Zbs(imn) * sin(arg)
  enddo

  return
end subroutine surfcoord

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!
