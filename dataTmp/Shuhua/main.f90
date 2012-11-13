!*********************************************************
!*********************************************************
!**** 2008/2/10   f90 program  about eg t2g model      ***                                                                 
!**** eg use zheev dianize it, t29 use classical spin  ***
!**** total energy use MC simulation                   ***
!****      -------------------Shuhua Liang             ***
!****                                                  ***
!****                                                  ***
!*********************************************************
!*********************************************************





!***************************************************************************
Program main
!***************************************************************************

  use table_site
  use diag_mod
  implicit none
  real(8)::fermi  , aunit 
  open(unit=65,file="./input.dat")

   read(65,*)   beta
   read(65,*)   n_optimize
   read(65,*)   J_Hund
   read(65,*)   DeltaXY
   read(65,*)   k_phase
   read(65,*)   fix_chemical
   read(65,*)   nsize
   read(65,*)   temperature
   read(65,*)   J_x
   read(65,*)   J_y
   read(65,*)   J_next
   read(65,*)   MCstable
   read(65,*)   MCstep
   read(65,*)   MC_unit
   read(65,*)   muu
  close(65)
                                ! claim fermi function 
  ns=nsize*nsize
  ndim=4*ns 
!  Beta=real(1.0/temperature,8)


  mu=muu

   step=MCstep*MC_unit
   stable=MCstable*MC_unit
   unstable=step-stable


  Irand=574721
  accept=0
  reject=0
  MC_alpha=0.15    !acceptance 
  MC_beta=0.2     !acceptance

 allocate(ss_list(1:MCstable),den_list(0:1,1:MCstable),den_xy_list(1:MCstable))
 allocate(Nc(nsize,nsize))
 allocate(indx(ns),indy(ns))
 allocate(near(ns,4),nextnear(ns,4))
 allocate(Ham(ndim,ndim),Ham1(ndim,ndim),eigenstates(ndim,ndim),TTx(ndim,ndim),TTy(ndim,ndim))
 allocate(eigen(ndim), eigen1(ndim),twist_eigen(1:k_phase**2*ndim))
 allocate(efi(nsize,nsize),etheta(nsize,nsize))
 allocate(S_str(0:nsize-1,0:nsize-1),ave_S_str(0:nsize-1,0:nsize-1) )
 allocate(con_list(1:2,1:MCstable),list(1:MCstable))
 allocate(Spec_b(1:ndim,0:1))
 

   open(unit=34,file='./akw_xz.dat')
   open(unit=35,file='./akw_yz.dat')
   open(unit=16,file='./spin250.inp')
   
!   open(unit=99,file='./test.dat')

 call coordinate(Nc,indx,indy,near,nextnear,nsize,ns)
  efi=0.d0
  etheta=0.d0 


if (1==1) then
  do j=1,nsize
  do i=1,nsize
  efi(i,j)=0.0
  etheta(i,j)=(0.5+0.5*real((-1)**i,8))*pi
  enddo
  enddo
endif

if (1==1) then
   read(16,*) u
   read(16,*) u
  do j=1,nsize
  do i=1,nsize
 read(16,*)  etheta(i,j)
  enddo
  enddo
   read(16,*) u
   read(16,*) u
  do j=1,nsize
  do i=1,nsize
 read(16,*)  efi(i,j)
  enddo
  enddo
endif




 mu=muu
twist_mu=muu
MC_counting=0
 bbb=0
 ccc=0
 ddd=0
phase_1=0.0
phase_2=0.0
measure=1
count_MCstable=0
ss_list=0.0
den_list=0.0
den_xy_list=0.0
con_list=0.0
list=0.0


 call Hamiltonian2D(J_Hund,DeltaXY,ham,TTx,TTy,nsize,ndim,ns,near,nextnear,indx,indy,efi,etheta,phase_1,phase_2)
 call ZHEEVD95(ham,eigen,'V','U', IER)           !!!!!!!in module file"diag_mod"  
 call order(ham,eigen,nsize,ndim,ns,indx,indy,n_optimize)


!do ieig = 1, ndim
!	write(99,*) eigen(ieig);
!end do



  input_mu=twist_mu




!!!!!!!!!!!C_x_y
!!!!!!!!!!!C_x_y
!!!!!!!!!!!C_x_y
do i_1=1, k_phase
do i_2=1, k_phase

 phase_1=2.0*pi*real(i_1-1,8)/real(k_phase,8)
 phase_2=2.0*pi*real(i_2-1,8)/real(k_phase,8)
! phase_2=2.0*pi*real(i_2-1,8)/real(k_phase,8)

 call Hamiltonian2D(J_Hund,DeltaXY,ham,TTx,TTy,nsize,ndim,ns,near,nextnear,indx,indy,efi,etheta,phase_1,phase_2)
 call ZHEEVD95(ham,eigen,'V','U', IER)           !!!!!!!in module file"diag_mod"  


   do k1=0, nsize/2     
   do k2=0, nsize/2         
   x8=real(k1,8)*2.0/real(nsize,8)- phase_1/real(nsize,8)/pi
   x9=real(k2,8)*2.0/real(nsize,8)- phase_2/real(nsize,8)/pi
   if(x8>1.0 .or. x9>1.0 .or. x8<0.0 .or. x9<0.0) goto 2011

if(x9+x8>1.0) then
   ppp=1.0-x9
   qqq=1.0-x8
   x8=ppp
   x9=qqq
endif
   

if(x9>0.5) then
   ppp=1.0-x8
   qqq=1.0-x9
   x8=ppp
   x9=qqq
endif

!write(*,*) x8
   k_a=k1
   k_b=k2
 call Akw3(ham,kdim,eigen,nsize,ndim,ns,indx,indy,k_a,k_b,phase_1,phase_2,Spec_b) 

  x3=0
  x4=0
  x5=0
  do i=1, ndim
  if (eigen(i)<twist_mu+0.01 .and. eigen(i)>twist_mu-0.01) then
!write(*,*) "haha"
  beta=1000.0
  x1=beta*fermi( twist_mu, eigen(i),beta)*fermi( twist_mu, eigen(i),-beta)
  x2=x1

!  x1=exp(beta*(eigen(i)-muu))
!  x2=x1*beta/(1+x1)**2
  x3=x3+x2*spec_b(i,0)
  x4=x4+x2*spec_b(i,1)


  if (x3>0.0001)  write(34,*) x8, x9, x3
  if (x4>0.000001)  write(35,*) x8, x9, x4

  endif
  enddo
  2011 continue

  enddo
  enddo
  enddo
  enddo


 close(16)
 close(34)
 close(35)
!_______________________________________________________________(deacllocate)

deallocate(ss_list,den_list,den_xy_list)
deallocate(Nc)
deallocate(indx,indy)
deallocate(near,nextnear)
deallocate(Ham,Ham1,eigenstates,TTx,TTy)
deallocate(eigen, eigen1,twist_eigen)
deallocate(efi,etheta)
deallocate(S_str,ave_S_str)
deallocate(con_list, list)
deallocate(Spec_b)

 end program main




!****************************************************************
      function fermi( mu, e,beta)
!****************************************************************(H start)

     implicit none

     real(8)::x,y
     real(8)::fermi
     real(8)::e,beta
     real(8)::mu
      
      x =  beta*( e - mu )

      if (x>0.0) then 
         y = dexp(-x)/(dexp(-x)+1.d0)
      else 
         y = 1.d0/(dexp(x)+1.d0)
      end if
	if (x>50) y=0.0
	if (x<-50) y=1.0

      fermi = y

      end function fermi
!$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$(H end)







