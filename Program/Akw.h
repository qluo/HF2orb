/** \ingroup HF */

/*! \file Akw.h
 *
 *  Calculate A(k,omega) and DOS
 *
 */
#ifndef AKW_H
#define AKW_H
#include "Utils.h"

namespace HF{
	template<typename EngineParamsType, typename HamType, typename LatticeType, typename FieldType>
	class Akw {
		typedef typename HamType::ComplexType ComplexType;
		typedef typename HamType::CMatrixType HamMatrixType;
		
	public:
		Akw(const EngineParamsType& engineParams, HamType& hamiltonian, LatticeType& lattice) :
			engineParams_(engineParams), hamiltonian_(hamiltonian), lattice_(lattice), lx_(engineParams_.lx), ly_(engineParams_.ly), sites_(lx_*ly_), hilbertSize_(hamiltonian_.getLength()), numOrb(hamiltonian_.getOrbs()), numSpin(hamiltonian_.getSpins()), Pi(3.1415926), nTBC(16)
		{
		}
		
		void calc_PBC_Print(std::ofstream& fout)
		{
			fout<<"#DOS"<<std::endl;

//			hamiltonian_.BuildHam();
//			hamiltonian_.printHam(fout);
//			hamiltonian_.Diagonalize();
//			hamiltonian_.printHam(fout);

			for(FieldType omega = -0.0;omega <= 0.5;omega += engineParams_.omegaStep){
				FieldType dos=0;
				for(int ikx=-lx_/2;ikx<lx_/2;ikx++){
					for(int iky=-ly_/2;iky<ly_/2;iky++){
//						int ikx=-lx_/2;
//						int iky=0;

						FieldType kx=double(ikx)/lx_;
						FieldType ky=double(iky)/ly_;
						FieldType akwTmp = calcAkwPBC(kx,ky,omega,fout);
						dos += akwTmp;
						fout<<2*kx<<" "<<2*ky<<" "<<omega<<" "<<akwTmp<<std::endl; // unit: Pi
					}
					fout<<std::endl;
				}
				fout<<std::endl;
				//fout<<omega<<" "<<dos<<std::endl;
			}
		}
		
		void calc_TBC_Print(std::ofstream& fout)
		{
			fout<<"#DOS"<<std::endl;
			hamiltonian_.BuildHam();
			for(FieldType omega = -2.0;omega <= 2.0;omega += engineParams_.omegaStep){
				FieldType dos=0;
//				for(int ikx=-lx_/2;ikx<lx_/2;ikx++){
				for(int ikx=lx_/2;ikx>-lx_/2;ikx--){
					for(int i_delta_kx=0;i_delta_kx<nTBC;i_delta_kx++){
//						for(int iky=-ly_/2;iky<ly_/2;iky++){
//						for(int iky=ly_/2;iky>-ly_/2;iky--){
//							for(int i_delta_ky=0;i_delta_ky<nTBC;i_delta_ky++){
//									int ikx=-lx_/2;
//									int i_delta_kx=0;
									int iky=0;
									int i_delta_ky=0;
									FieldType kx=double(ikx)/lx_;
									FieldType ky=double(iky)/ly_;
									FieldType phaseX = double(i_delta_kx)/(nTBC);
									FieldType phaseY = double(i_delta_ky)/(nTBC);

									FieldType akwTmp = calcAkwTBC(kx,ky,omega,phaseX,phaseY,fout);
									dos += akwTmp;
//									if(akwTmp > 100) std::cout<<2*(kx+phaseX/lx_)<<" "<<2*(ky+phaseY/ly_);
									fout<<2*(kx-phaseX/lx_)<<" "<<2*(ky-phaseY/ly_)<<" "<<omega<<" "<<akwTmp<<std::endl; // unit: Pi
//								}
//							}
//						fout<<std::endl;
						}
					}
					//fout<<"#Omega="<<omega<<"\n#DOS="<<dos<<"\n"<<std::endl;
					fout<<std::endl;
				}
				//cout<<"mus: "<<hamiltonian_.mus<<endl;
		}
		
	private:

		FieldType calcAkwPBC(FieldType& kx, FieldType& ky, FieldType& omega, std::ofstream& fout)
		{
			FieldType mu = engineParams_.mu;
			ComplexType sum = 0;
			ComplexType sum1 = 0;

			hamiltonian_.BuildHam();
			hamiltonian_.Diagonalize();

			for(int lambda=0;lambda<hilbertSize_;lambda++)
				for(int j=0;j<sites_;j++)
					for(int l=0;l<sites_;l++){
					// --------- Fourier Transform----------------
						int jx=0;
						int jy=0;
						int lx=0;
						int ly=0;
						calcComponents(j,jx,jy);
						calcComponents(l,lx,ly);
						ComplexType calcExp = ComplexType(cos(2.0*Pi*(kx*(jx-lx)+ky*(jy-ly))),sin(2.0*Pi*(kx*(jx-lx)+ky*(jy-ly)))); 
					//	fout<<calcExp<<std::endl;
					// --------------------------------------------
						for(int iOrb=0;iOrb<numOrb;iOrb++)
							for(int iSpin=0;iSpin<numSpin;iSpin++){
								int i_j=j+(iOrb+iSpin*numOrb)*sites_;
								int i_l=l+(iOrb+iSpin*numOrb)*sites_;
								sum1 = calcExp * conj(hamiltonian_.ham(i_j,lambda)) * hamiltonian_.ham(i_l,lambda) * deltaFun(omega-hamiltonian_.eigs[lambda]+mu);
								//if(real(sum1)>0.0001) fout <<i_j<<" "<<i_l<<" "<< sum1 <<std::endl;
								sum += sum1;
							}
					}
					//fout<<"#PBC test\n"<<hamiltonian_.ham;
			return real(sum)/sites_;
		}	

		FieldType calcAkwTBC(FieldType& kx, FieldType& ky, FieldType& omega, FieldType& phase_x, FieldType& phase_y, std::ofstream& fout)
		{
			// -------- to be implemented -------
			HamMatrixType hamTBC; 
			std::vector<FieldType> eigsTBC;
			FieldType mu = engineParams_.mu; 
			hamTBC.resize(hilbertSize_,hilbertSize_);
			bool test = creatHamTBC(hamTBC,phase_x,phase_y);
//			bool test = hamiltonian_.BuildHamTBC(hamTBC,phase_x,phase_y);
//			if((phase_x == 0) && (phase_y == 0)) fout<<hamTBC<<std::endl;
			if(test) diagonalize(hamTBC,eigsTBC);
//			if((phase_x == 0) && (phase_y == 0)) fout<<hamTBC<<std::endl;
			else throw std::runtime_error ("ERROR: creatHamTBC ");
			//if(engineParams_.density>0) mu = adjMu(eigsTBC); 		
			
			ComplexType sum = 0;
			ComplexType sum1 = 0;
			for(int lambda=0;lambda<hilbertSize_;lambda++)
				for(int j=0;j<sites_;j++)
					for(int l=0;l<sites_;l++){
					// --------- Fourier Transform ----------
						int jx=0;
						int jy=0;
						int lx=0;
						int ly=0;
						calcComponents(j,jx,jy);
						calcComponents(l,lx,ly);
						FieldType tkx = kx - phase_x/lx_;
						FieldType tky = ky - phase_y/ly_;
						ComplexType calcExp = ComplexType(cos(2.0*Pi*(tkx*(jx-lx)+tky*(jy-ly))),sin(2.0*Pi*(tkx*(jx-lx)+tky*(jy-ly)))); 
						//fout<<calcExp<<std::endl;
					// --------- Fourier Transform ----------
						for(int iOrb=0;iOrb<numOrb;iOrb++)
							for(int iSpin=0;iSpin<numSpin;iSpin++){
								int i_j=j+(iOrb+iSpin*numOrb)*sites_;
								int i_l=l+(iOrb+iSpin*numOrb)*sites_;
								sum1 = calcExp * conj(hamTBC(i_j,lambda)) * hamTBC(i_l,lambda) * deltaFun(omega-eigsTBC[lambda]+mu);
								//if(real(sum1)>0.0001) fout <<i_j<<" "<<i_l<<" "<< sum1 <<std::endl;
								sum += sum1;
							}
					}
					//fout<<"#TBC test\n"<<hamTBC;
			//delete hamTBC;
			//delete eigsTBC;
			return real(sum)/sites_;			
		}

		enum myCrossType {NEG=-1, NOT=0, POS=1};

		bool creatHamTBC(HamMatrixType& hamTBC, FieldType& phase_x, FieldType& phase_y)
		{	
//			std::cout<<"Hello, creatHamTBC"<<std::endl;
		  FieldType ph_x = phase_x;
		  FieldType ph_y = phase_y;	
			for (int ispin1=0;ispin1<numSpin;ispin1++)
			for (int ispin2=0;ispin2<numSpin;ispin2++) {
				for (int iorb1=0;iorb1<numOrb;iorb1++)
				for (int iorb2=0;iorb2<numOrb;iorb2++) {
					for(int isite=0;isite<sites_;isite++)
					for(int jsite=0;jsite<sites_;jsite++){
//						int isite = 7, jsite = 1;
						int i = isite+(iorb1+ispin1*numOrb)*sites_;
						int j = jsite+(iorb2+ispin2*numOrb)*sites_;

							myCrossType crossX = Is_Cross_X_Boundary(isite,jsite);
							myCrossType crossY = Is_Cross_Y_Boundary(isite,jsite);

							if ( (crossX==POS) && (crossY==POS) ) hamTBC(i,j) = hamiltonian_.ham(i,j)*(ComplexType(cos(2.0*Pi*(ph_x+ph_y)),sin(2.0*Pi*(ph_x+ph_y))));
							else if	( (crossX==NEG) && (crossY==NEG) ) hamTBC(i,j) = hamiltonian_.ham(i,j)*(ComplexType(cos(2.0*Pi*(ph_x+ph_y)),-sin(2.0*Pi*(ph_x+ph_y))));
							else if ( (crossX==POS) && (crossY==NEG) ) hamTBC(i,j) = hamiltonian_.ham(i,j)*(ComplexType(cos(2.0*Pi*(ph_x-ph_y)),sin(2.0*Pi*(ph_x-ph_y))));
							else if	( (crossX==NEG) && (crossY==POS) ) hamTBC(i,j) = hamiltonian_.ham(i,j)*(ComplexType(cos(2.0*Pi*(ph_x-ph_y)),-sin(2.0*Pi*(ph_x-ph_y))));

							else if ( (crossX==POS) && (crossY==NOT) ) hamTBC(i,j) = hamiltonian_.ham(i,j)*(ComplexType(cos(2.0*Pi*ph_x),sin(2.0*Pi*ph_x)));
							else if ( (crossX==NEG) && (crossY==NOT) ) hamTBC(i,j) = hamiltonian_.ham(i,j)*(ComplexType(cos(2.0*Pi*ph_x),-sin(2.0*Pi*ph_x)));

							else if ( (crossX==NOT) && (crossY==POS) ) hamTBC(i,j) = hamiltonian_.ham(i,j)*(ComplexType(cos(2.0*Pi*ph_y),sin(2.0*Pi*ph_y)));
							else if ( (crossX==NOT) && (crossY==NEG) ) hamTBC(i,j) = hamiltonian_.ham(i,j)*(ComplexType(cos(2.0*Pi*ph_y),-sin(2.0*Pi*ph_y)));

							else if ( (crossX==NOT) && (crossY==NOT) ) hamTBC(i,j) = hamiltonian_.ham(i,j);
					} // end for (site)
				} // end for (orb)
			} // end for (spin)
			
			return true;
		}


/*		
		bool creatHamTBC(HamMatrixType& hamTBC, FieldType& phase_x, FieldType& phase_y)
		{	
		  FieldType ph_x = phase_x;
		  FieldType ph_y = phase_y;	
			for (int ispin1=0;ispin1<numSpin;ispin1++)
			for (int ispin2=0;ispin2<numSpin;ispin2++) {
				for (int iorb1=0;iorb1<numOrb;iorb1++)
				for (int iorb2=0;iorb2<numOrb;iorb2++) {
					for(int isite=0;isite<sites_;isite++)
					for(int jsite=0;jsite<sites_;jsite++){
						int i = isite+(iorb1+ispin1*numOrb)*sites_;
						int j = jsite+(iorb2+ispin2*numOrb)*sites_;
						if(i>=j){
							bool crossX = Is_Cross_X_Boundary(isite,jsite);
							bool crossY = Is_Cross_Y_Boundary(isite,jsite);
							if(crossX && crossY) {
									hamTBC(i,j) = hamiltonian_.ham(i,j)*(ComplexType(cos(2.0*Pi*(ph_x+ph_y)),sin(2.0*Pi*(ph_x+ph_y))));
									hamTBC(j,i) = hamiltonian_.ham(j,i)*(ComplexType(cos(2.0*Pi*(ph_x+ph_y)),-sin(2.0*Pi*(ph_x+ph_y))));
							}
							else if(crossX) {
									hamTBC(i,j) = hamiltonian_.ham(i,j)*(ComplexType(cos(2.0*Pi*ph_x),sin(2.0*Pi*ph_x)));
									hamTBC(j,i) = hamiltonian_.ham(j,i)*(ComplexType(cos(2.0*Pi*ph_x),-sin(2.0*Pi*ph_x)));
							}
							else if(crossY){
									hamTBC(i,j) = hamiltonian_.ham(i,j)*(ComplexType(cos(2.0*Pi*ph_y),sin(2.0*Pi*ph_y)));
									hamTBC(j,i) = hamiltonian_.ham(j,i)*(ComplexType(cos(2.0*Pi*ph_y),-sin(2.0*Pi*ph_y)));
							}
							else {
								hamTBC(i,j) = hamiltonian_.ham(i,j);
								hamTBC(j,i) = hamiltonian_.ham(j,i);
							}
						} // end if(i>j)
					} // end for (site)
				} // end for (orb)
			} // end for (spin)
			
			return true;
		}
*/		
		void diagonalize(HamMatrixType& ham, std::vector<FieldType>& eigs)
		{
			char jobz='V';
			utils::diag(ham,eigs,jobz);
			//if (jobz!='V') sort(eigs.begin(), eigs.end(), std::less<FieldType>());
		}
		
		inline FieldType deltaFun(FieldType var)
		{
			return engineParams_.eps[2]/(pow(engineParams_.eps[2],2)+pow(var,2));
		}

		inline void calcComponents(int i, int& ix, int& iy)
		{
			iy = int(i/lx_);
			ix = i % lx_;
		}

		myCrossType Is_Cross_X_Boundary(int isite1, int isite2)
		{			
			std::vector<int> coor_site1(2);
			lattice_.index2Coor(coor_site1,isite1);
			std::vector<int> coor_site2(2);
			lattice_.index2Coor(coor_site2,isite2);
			//cout<<"("<<isite1<<","<<isite2<<")"<<" site1: "<<coor_site1[0]<<coor_site1[0]<<" site2: "<<coor_site2[0]<<coor_site2[0]<<endl;
			if((coor_site1[0]==0 || coor_site1[0]==1) && (coor_site2[0]==(lx_-1) || coor_site2[0]==(lx_-2))) return POS;
			else if((coor_site1[0]==(lx_-1) || coor_site1[0]==(lx_-2)) && (coor_site2[0]==0 || coor_site2[0]==1)) return NEG;
			else return NOT;
		}

		// ------- false: NOT on Boundary ------ true: on Y Boundary ------ //
		myCrossType Is_Cross_Y_Boundary(int isite1, int isite2)
		{			
			std::vector<int> coor_site1(2);
			lattice_.index2Coor(coor_site1,isite1);
			std::vector<int> coor_site2(2);
			lattice_.index2Coor(coor_site2,isite2);
			if((coor_site1[1]==0 || coor_site1[1]==1) && (coor_site2[1]==(ly_-1) || coor_site2[1]==(ly_-2))) return POS;
			else if((coor_site1[1]==(ly_-1) || coor_site1[1]==(ly_-2)) && (coor_site2[1]==0 || coor_site2[1]==1)) return NEG;
			else return NOT;
		}

/*		
		// ------- false: NOT on Boundary ------ true: on X Boundary ------ //
		bool Is_Cross_X_Boundary(int isite1, int isite2)
		{			
			std::vector<int> coor_site1(2);
			lattice_.index2Coor(coor_site1,isite1);
			std::vector<int> coor_site2(2);
			lattice_.index2Coor(coor_site2,isite2);
			//cout<<"("<<isite1<<","<<isite2<<")"<<" site1: "<<coor_site1[0]<<coor_site1[0]<<" site2: "<<coor_site2[0]<<coor_site2[0]<<endl;
			if(coor_site1[0]==0 && (coor_site2[0]==(lx_-1) || coor_site2[0]==(lx_-2))) return true;
			else if(coor_site1[0]==1 && coor_site2[0]==(lx_-1) ) return true;
			else if(coor_site1[0]==(lx_-2) && coor_site2[0]==0) return true;
			else if(coor_site1[0]==(lx_-1) && (coor_site2[0]==0 || coor_site2[0]==1)) return true;
			else return false;
		}

		// ------- false: NOT on Boundary ------ true: on Y Boundary ------ //
		bool Is_Cross_Y_Boundary(int isite1, int isite2)
		{			
			std::vector<int> coor_site1(2);
			lattice_.index2Coor(coor_site1,isite1);
			std::vector<int> coor_site2(2);
			lattice_.index2Coor(coor_site2,isite2);
			if(coor_site1[1]==0 && (coor_site2[1]==(ly_-1) || coor_site2[1]==(ly_-2))) return true;
			else if(coor_site1[1]==1 && coor_site2[1]==(ly_-1) ) return true;
			else if(coor_site1[1]==(ly_-2) && coor_site2[1]==0) return true;
			else if(coor_site1[1]==(ly_-1) && (coor_site2[1]==0 || coor_site2[0]==1)) return true;
			else return false;
			//if(coor_site1[1]==(ly_-1) && coor_site2[1]==0) return true;
			//else return false;
		}
*/		
		FieldType adjMu(std::vector<FieldType>& eigs)
		{	
			FieldType n0=engineParams_.density*sites_;
			int i_fs=int(n0)-1;
			return (eigs[i_fs]+eigs[i_fs+1])/2.0;			
		}
		
		const EngineParamsType& engineParams_;		
		HamType& hamiltonian_;	
		LatticeType& lattice_;
		int lx_;
		int ly_;
		int sites_;
		int hilbertSize_;
		const int numOrb;
		const int numSpin;
		const FieldType Pi;
		const int nTBC;

	}; // AKW
	
} // namespace HF

#endif
