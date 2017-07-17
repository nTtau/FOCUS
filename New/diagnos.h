!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

SUBROUTINE diagnos
!------------------------------------------------------------------------------------------------------ 
! DATE: 07/13/2017
! diagonose the coil performance
!------------------------------------------------------------------------------------------------------   
  use globals, only: zero, one, sqrtmachprec, myid, ounit, IsQuiet, case_optimize, case_length, &
       coil, surf, Ncoils, Nteta, Nzeta, bnorm, bharm, tflux, ttlen, specw, ccsep
                     
  implicit none
  include "mpif.h"

  INTEGER           :: icoil, itmp, astat, ierr, mf=20, nf=20
  LOGICAL           :: lwbnorm = .True. , l_raw = .False.!if use raw coils data
  REAL              :: MaxCurv, AvgLength, MinCCdist, MinCPdist, tmp_dist
  REAL, parameter   :: infmax = 1.0E6
  REAL, allocatable :: Atmp(:,:), Btmp(:,:)

  if (myid == 0 .and. IsQuiet < 0) write(ounit, *) "----Coil diagnostic--------------------------------"

  if (case_optimize == 0) call AllocData(0) ! if not allocate data;
  call costfun(0)

  if (myid == 0) write(ounit, '("diagnos : "6(A12," ; "))') , &
       "Bnormal", "Bmn harmonics", "tor. flux", "coil length", "spectral", "c-c sep." 
  if (myid == 0) write(ounit, '("diagnos : "6(ES12.5," ; "))') bnorm, bharm, tflux, ttlen, specw, ccsep

  MaxCurv = zero
  do icoil = 1, Ncoils
     call curvature(icoil)
     if (coil(icoil)%maxcurv .ge. MaxCurv) then
        MaxCurv = coil(icoil)%maxcurv
        itmp = icoil !record the number
     endif
#ifdef DEBUG
     if(myid .eq. 0) write(ounit, '("diagnos : Maximum curvature of "I3 "-th coil is : " ES23.15)') &
          icoil, coil(icoil)%maxcurv
#endif
  enddo
  if(myid .eq. 0) write(ounit, '("diagnos : Maximum curvature of all the coils is  :" ES23.15 &
       " ; at coil " I3)') MaxCurv, itmp
  
  ! calculate the average length
  AvgLength = zero
  if ( (case_length == 1) .and. (sum(coil(1:Ncoils)%Lo) < sqrtmachprec) ) coil(1:Ncoils)%Lo = one
  call length(0)
  do icoil = 1, Ncoils
     AvgLength = AvgLength + coil(icoil)%L
  enddo
  AvgLength = AvgLength / Ncoils
  if(myid .eq. 0) write(ounit, '("diagnos : Average length of the coils is"8X" :" ES23.15)') AvgLength

  ! calculate the minimum distance of coil-coil separation
  ! coils are supposed to be placed in order
  minCCdist = infmax
  do icoil = 1, Ncoils

     if(Ncoils .eq. 1) exit !if only one coil
     itmp = icoil + 1
     if(icoil .eq. Ncoils) itmp = 1

     SALLOCATE(Atmp, (1:3,1:coil(icoil)%NS), zero)
     SALLOCATE(Btmp, (1:3,1:coil(itmp )%NS), zero)

     Atmp(1, 1:coil(icoil)%NS) = coil(icoil)%xx(1:coil(icoil)%NS)
     Atmp(2, 1:coil(icoil)%NS) = coil(icoil)%yy(1:coil(icoil)%NS)
     Atmp(3, 1:coil(icoil)%NS) = coil(icoil)%zz(1:coil(icoil)%NS)

     Btmp(1, 1:coil(itmp)%NS) = coil(itmp)%xx(1:coil(itmp)%NS)
     Btmp(2, 1:coil(itmp)%NS) = coil(itmp)%yy(1:coil(itmp)%NS)
     Btmp(3, 1:coil(itmp)%NS) = coil(itmp)%zz(1:coil(itmp)%NS)
     
     call mindist(Atmp, coil(icoil)%NS, Btmp, coil(itmp)%NS, tmp_dist)

     if (minCCdist .ge. tmp_dist) minCCdist=tmp_dist

     DALLOCATE(Atmp)
     DALLOCATE(Btmp)

  enddo

  if(myid .eq. 0) write(ounit, '("diagnos : The minimum coil-coil distance is "4X" :" ES23.15)') minCCdist

  ! calculate the minimum distance of coil-plasma separation
  minCPdist = infmax
  do icoil = 1, Ncoils

     SALLOCATE(Atmp, (1:3,1:coil(icoil)%NS), zero)
     SALLOCATE(Btmp, (1:3,1:(Nteta*Nzeta)), zero)

     Atmp(1, 1:coil(icoil)%NS) = coil(icoil)%xx(1:coil(icoil)%NS)
     Atmp(2, 1:coil(icoil)%NS) = coil(icoil)%yy(1:coil(icoil)%NS)
     Atmp(3, 1:coil(icoil)%NS) = coil(icoil)%zz(1:coil(icoil)%NS)

     Btmp(1, 1:(Nteta*Nzeta)) = reshape(surf(1)%xx(0:Nteta-1, 0:Nzeta-1), (/Nteta*Nzeta/))
     Btmp(2, 1:(Nteta*Nzeta)) = reshape(surf(1)%yy(0:Nteta-1, 0:Nzeta-1), (/Nteta*Nzeta/))
     Btmp(3, 1:(Nteta*Nzeta)) = reshape(surf(1)%zz(0:Nteta-1, 0:Nzeta-1), (/Nteta*Nzeta/))

     call mindist(Atmp, coil(icoil)%NS, Btmp, Nteta*Nzeta, tmp_dist)

     if (minCPdist .ge. tmp_dist) then 
        minCPdist=tmp_dist
        itmp = icoil
     endif

     DALLOCATE(Atmp)
     DALLOCATE(Btmp)

  enddo
  if(myid .eq. 0) write(ounit, '("diagnos : The minimum coil-plasma distance is    :" ES23.15 &
       " ; at coil " I3)') minCPdist, itmp


END SUBROUTINE diagnos

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

subroutine curvature(icoil)

  use globals, only : zero, pi2, ncpu, astat, ierr, myid, ounit, coil, NFcoil, Nseg, Ncoils
  implicit none
  include "mpif.h"

  INTEGER, INTENT(in) :: icoil

  REAL,allocatable    :: curv(:)

  SALLOCATE(curv, (0:coil(icoil)%NS), zero)

  curv = sqrt( (coil(icoil)%za*coil(icoil)%yt-coil(icoil)%zt*coil(icoil)%ya)**2  &
             + (coil(icoil)%xa*coil(icoil)%zt-coil(icoil)%xt*coil(icoil)%za)**2  & 
             + (coil(icoil)%ya*coil(icoil)%xt-coil(icoil)%yt*coil(icoil)%xa)**2 )& 
             / ((coil(icoil)%xt)**2+(coil(icoil)%yt)**2+(coil(icoil)%zt)**2)**(1.5)
  coil(icoil)%maxcurv = maxval(curv)

  return
end subroutine curvature

!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!

subroutine mindist(array_A, dim_A, array_B, dim_B, minimum)

  implicit none

  INTEGER, INTENT(IN ) :: dim_A, dim_B
  REAL   , INTENT(IN ) :: array_A(1:3,1:dim_A), array_B(1:3,1:dim_B)
  REAL   , INTENT(OUT) :: minimum

  INTEGER :: ipoint, jpoint, itmp, jtmp
  REAL, parameter :: infmax = 1.0E6
  REAL    :: distance

  minimum = infmax
  do ipoint = 1, dim_A
     do jpoint = 1, dim_B

        distance = ( array_A(1, ipoint) - array_B(1, jpoint) )**2 &
                  +( array_A(2, ipoint) - array_B(2, jpoint) )**2 &
                  +( array_A(3, ipoint) - array_B(3, jpoint) )**2
        if (distance .le. minimum) minimum = distance
        
     enddo
  enddo

  minimum = sqrt(minimum)
  
  return

end subroutine mindist
