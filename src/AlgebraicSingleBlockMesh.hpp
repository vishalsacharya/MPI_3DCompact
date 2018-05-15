#include "AbstractSingleBlockMesh.hpp"
#ifndef _CALGEBRAICSINGLEBLOCKMESHH_
#define _CALGEBRAICSINGLEBLOCKMESHH_

#include "Macros.hpp"
#include "Utils.hpp"
#include "Derivatives.hpp"
#include "AbstractSingleBlockMesh.hpp"
#include "AbstractCSolver.hpp"

class AlgebraicSingleBlockMesh:public AbstractSingleBlockMesh{

    public:

	C2Decomp *c2d;
	AbstractCSolver *cs;
	Domain *d;

        int pxSize[3], pySize[3], pzSize[3]; 
        int pxStart[3], pyStart[3], pzStart[3];
        int pxEnd[3], pyEnd[3], pzEnd[3];

	double periodicXTranslation[3];
	double periodicYTranslation[3];
	double periodicZTranslation[3];

	bool periodicX;
	bool periodicY;
	bool periodicZ;

	AlgebraicSingleBlockMesh(C2Decomp *c2d, AbstractCSolver *cs, Domain *dom, int mpiRank){

	    this->mpiRank = mpiRank;

	    this->c2d = c2d;
	    this->d = dom;
	    this->derivX = cs->derivX;
	    this->derivY = cs->derivY;
	    this->derivZ = cs->derivZ;

	    d->getPencilDecompInfo(pxSize, pySize, pzSize, pxStart, pyStart, pzStart, pxEnd, pyEnd, pzEnd);

	    max_xi  = d->gLx;
	    max_eta = d->gLy;
	    max_zta = d->gLz;

	    Nx = d->gNx;
	    Ny = d->gNy;
	    Nz = d->gNz;

	    //Doing this for base y-pencil solvers...
	    c2d->allocY(x);
	    c2d->allocY(y);
	    c2d->allocY(z);



	    //Generate the mesh algebraically...
	    FOR_Z_YPEN{
		FOR_Y_YPEN{
		    FOR_X_YPEN{
			int ip = GETMAJIND_YPEN;
		
			int ii = GETGLOBALXIND_YPEN;
			int jj = GETGLOBALYIND_YPEN;
			int kk = GETGLOBALZIND_YPEN;

			double xi  = d->x[ii];
			double eta = d->y[jj];
			double zta = d->z[kk];
		
			//double nXi  = xi/max_xi;
			//double nEta = eta/max_eta;
			//double nZta = zta/max_zta;

			x[ip] = xi;
			y[ip] = eta;
			z[ip] = zta;  

		    }
		}
	    }

	    if(cs->bc->bcXType == BC::PERIODIC_SOLVE){
		periodicX = true;
		periodicXTranslation[0] = 1.0;
		periodicXTranslation[1] = 0.0;
		periodicXTranslation[2] = 0.0;
		IF_RANK0 cout << "Periodic x-face translation = {" << periodicXTranslation[0] << ", " << periodicXTranslation[1] << ", " << periodicXTranslation[2] << "}" << endl;;
	    }else{
		periodicX = false;
	    }

	    if(cs->bc->bcYType == BC::PERIODIC_SOLVE){
		periodicY = true;
		periodicYTranslation[0] = 0.0;
		periodicYTranslation[1] = 1.0;
		periodicYTranslation[2] = 0.0;
		IF_RANK0 cout << "Periodic y-face translation = {" << periodicYTranslation[0] << ", " << periodicYTranslation[1] << ", " << periodicYTranslation[2] << "}" << endl;;
	    }else{
		periodicY = false; 
	    }

	    if(cs->bc->bcZType == BC::PERIODIC_SOLVE){
		periodicZ = true;
		periodicZTranslation[0] = 0.0;
		periodicZTranslation[1] = 0.0;
		periodicZTranslation[2] = 1.0;

		IF_RANK0 cout << "Periodic z-face translation = {" << periodicZTranslation[0] << ", " << periodicZTranslation[1] << ", " << periodicZTranslation[2] << "}" << endl;;

	    }else{
		periodicZ = false;
	    }




	    c2d->allocY(J);
	    c2d->allocY(J11);
	    c2d->allocY(J12);
	    c2d->allocY(J13);
	    c2d->allocY(J21);
	    c2d->allocY(J22);
	    c2d->allocY(J23);
	    c2d->allocY(J31);
	    c2d->allocY(J32);
	    c2d->allocY(J33);

	}

 	void solveForJacobians();


};


void AlgebraicSingleBlockMesh::solveForJacobians(){

	double *xE11, *xE21;
	double *xE12, *xE22;
	double *xE13, *xE23;

	c2d->allocY(xE11);
	c2d->allocY(xE12);
	c2d->allocY(xE13);
	c2d->allocY(xE21);
	c2d->allocY(xE22);
	c2d->allocY(xE23);

	//Do the E2 derivatives first...
	if(periodicY){
	    double *Nm2x, *Nm1x, *Np1x, *Np2x;
	    Nm2x = new double[pySize[0]*pySize[2]];
	    Nm1x = new double[pySize[0]*pySize[2]];
	    Np1x = new double[pySize[0]*pySize[2]];
	    Np2x = new double[pySize[0]*pySize[2]];

	    double *Nm2y, *Nm1y, *Np1y, *Np2y;
	    Nm2y = new double[pySize[0]*pySize[2]];
	    Nm1y = new double[pySize[0]*pySize[2]];
	    Np1y = new double[pySize[0]*pySize[2]];
	    Np2y = new double[pySize[0]*pySize[2]];

	    FOR_X_YPEN{
		FOR_Z_YPEN{
		    int ii = i*pySize[2] + k;

		    int iim2 = i*pySize[2]*pySize[1] + k*pySize[1] + pySize[1]-2;		
		    int iim1 = i*pySize[2]*pySize[1] + k*pySize[1] + pySize[1]-1;		
		    int iip1 = i*pySize[2]*pySize[1] + k*pySize[1] + 0;		
		    int iip2 = i*pySize[2]*pySize[1] + k*pySize[1] + 1;		

		    Nm2x[ii] = x[iim2]-periodicYTranslation[0];
		    Nm1x[ii] = x[iim1]-periodicYTranslation[0];
		    Np1x[ii] = x[iip1]+periodicYTranslation[0];
		    Np2x[ii] = x[iip2]+periodicYTranslation[0];
	
		    Nm2y[ii] = y[iim2]-periodicYTranslation[1];
		    Nm1y[ii] = y[iim1]-periodicYTranslation[1];
		    Np1y[ii] = y[iip1]+periodicYTranslation[1];
		    Np2y[ii] = y[iip2]+periodicYTranslation[1];
	
		}
	    }

	    derivY->calc1stDerivField_TPB(x, xE12, Nm2x, Nm1x, Np1x, Np2x);
	    derivY->calc1stDerivField_TPB(y, xE22, Nm2y, Nm1y, Np1y, Np2y);

	    delete[] Nm2x;
	    delete[] Nm1x;
	    delete[] Np1x;
	    delete[] Np2x;

	    delete[] Nm2y;
	    delete[] Nm1y;
	    delete[] Np1y;
	    delete[] Np2y;

	}else{
	    derivY->calc1stDerivField(x, xE12);
	    derivY->calc1stDerivField(y, xE22);
	}
	

	getRange(xE12, "xE12", pySize[0], pySize[1], pySize[2], mpiRank);
	getRange(xE22, "xE22", pySize[0], pySize[1], pySize[2], mpiRank);
	
	//Transpose over to E1...
	double *tempX1, *tempX2, *tempX3, *tempX4;
	c2d->allocX(tempX1);
	c2d->allocX(tempX2);
	c2d->allocX(tempX3);
	c2d->allocX(tempX4);

	c2d->transposeY2X_MajorIndex(x, tempX1);
	c2d->transposeY2X_MajorIndex(y, tempX2);


	//Calculate E1 Derivatives..
	if(periodicX){
	    double *Nm2x, *Nm1x, *Np1x, *Np2x;
            Nm2x = new double[pxSize[1]*pxSize[2]];
            Nm1x = new double[pxSize[1]*pxSize[2]];
            Np1x = new double[pxSize[1]*pxSize[2]];
            Np2x = new double[pxSize[1]*pxSize[2]];

	    double *Nm2y, *Nm1y, *Np1y, *Np2y;
            Nm2y = new double[pxSize[1]*pxSize[2]];
            Nm1y = new double[pxSize[1]*pxSize[2]];
            Np1y = new double[pxSize[1]*pxSize[2]];
            Np2y = new double[pxSize[1]*pxSize[2]];

            FOR_Z_XPEN{
                FOR_Y_XPEN{
                    int ii = k*pxSize[1] + j;

                    int iim2 = k*pxSize[0]*pxSize[1] + j*pxSize[0] + pxSize[0]-2;
                    int iim1 = k*pxSize[0]*pxSize[1] + j*pxSize[0] + pxSize[0]-1;
                    int iip1 = k*pxSize[0]*pxSize[1] + j*pxSize[0] + 0;
                    int iip2 = k*pxSize[0]*pxSize[1] + j*pxSize[0] + 1;

                    Nm2x[ii] = tempX1[iim2]-periodicXTranslation[0];
                    Nm1x[ii] = tempX1[iim1]-periodicXTranslation[0];
                    Np1x[ii] = tempX1[iip1]+periodicXTranslation[0];
                    Np2x[ii] = tempX1[iip2]+periodicXTranslation[0];

                    Nm2y[ii] = tempX2[iim2]-periodicXTranslation[1];
                    Nm1y[ii] = tempX2[iim1]-periodicXTranslation[1];
                    Np1y[ii] = tempX2[iip1]+periodicXTranslation[1];
                    Np2y[ii] = tempX2[iip2]+periodicXTranslation[1];

                }
            }

            derivX->calc1stDerivField_TPB(tempX1, tempX3, Nm2x, Nm1x, Np1x, Np2x);
            derivX->calc1stDerivField_TPB(tempX2, tempX4, Nm2y, Nm1y, Np1y, Np2y);

            delete[] Nm2x;
            delete[] Nm1x;
            delete[] Np1x;
            delete[] Np2x;

            delete[] Nm2y;
            delete[] Nm1y;
            delete[] Np1y;
            delete[] Np2y;

	}else{
	    derivX->calc1stDerivField(tempX1, tempX3);
	    derivX->calc1stDerivField(tempX2, tempX4);
	}


	//Transpose back to E2...
	c2d->transposeX2Y_MajorIndex(tempX3, xE11);
	c2d->transposeX2Y_MajorIndex(tempX4, xE21);

	getRange(xE11, "xE11", pySize[0], pySize[1], pySize[2], mpiRank);
	getRange(xE21, "xE21", pySize[0], pySize[1], pySize[2], mpiRank);

	//Transpose over to E3...
	double *tempZ1, *tempZ2, *tempZ3, *tempZ4;
	c2d->allocZ(tempZ1);
	c2d->allocZ(tempZ2);
	c2d->allocZ(tempZ3);
	c2d->allocZ(tempZ4);

	c2d->transposeY2Z_MajorIndex(x, tempZ1);
	c2d->transposeY2Z_MajorIndex(y, tempZ2);

	//Calculate E3 Derivatives
	if(periodicZ){
	    double *Nm2x, *Nm1x, *Np1x, *Np2x;
            Nm2x = new double[pzSize[1]*pzSize[0]];
            Nm1x = new double[pzSize[1]*pzSize[0]];
            Np1x = new double[pzSize[1]*pzSize[0]];
            Np2x = new double[pzSize[1]*pzSize[0]];

	    double *Nm2y, *Nm1y, *Np1y, *Np2y;
            Nm2y = new double[pzSize[1]*pzSize[0]];
            Nm1y = new double[pzSize[1]*pzSize[0]];
            Np1y = new double[pzSize[1]*pzSize[0]];
            Np2y = new double[pzSize[1]*pzSize[0]];

            FOR_Y_ZPEN{
                FOR_X_ZPEN{
                    int ii = j*pzSize[0] + i;

                    int iim2 = j*pzSize[0]*pzSize[2] + i*pzSize[2] + pzSize[2]-2;
                    int iim1 = j*pzSize[0]*pzSize[2] + i*pzSize[2] + pzSize[2]-1;
                    int iip1 = j*pzSize[0]*pzSize[2] + i*pzSize[2] + 0;
                    int iip2 = j*pzSize[0]*pzSize[2] + i*pzSize[2] + 1;

                    Nm2x[ii] = tempZ1[iim2]-periodicZTranslation[0];
                    Nm1x[ii] = tempZ1[iim1]-periodicZTranslation[0];
                    Np1x[ii] = tempZ1[iip1]+periodicZTranslation[0];
                    Np2x[ii] = tempZ1[iip2]+periodicZTranslation[0];

                    Nm2y[ii] = tempZ2[iim2]-periodicZTranslation[1];
                    Nm1y[ii] = tempZ2[iim1]-periodicZTranslation[1];
                    Np1y[ii] = tempZ2[iip1]+periodicZTranslation[1];
                    Np2y[ii] = tempZ2[iip2]+periodicZTranslation[1];

                }
            }

            derivZ->calc1stDerivField_TPB(tempZ1, tempZ3, Nm2x, Nm1x, Np1x, Np2x);
            derivZ->calc1stDerivField_TPB(tempZ2, tempZ4, Nm2y, Nm1y, Np1y, Np2y);

            delete[] Nm2x;
            delete[] Nm1x;
            delete[] Np1x;
            delete[] Np2x;

            delete[] Nm2y;
            delete[] Nm1y;
            delete[] Np1y;
            delete[] Np2y;


	}else{
	    derivZ->calc1stDerivField(tempZ1, tempZ3);
	    derivZ->calc1stDerivField(tempZ2, tempZ4);
	}

	//Transpose over to E2
	c2d->transposeZ2Y_MajorIndex(tempZ3, xE13);
	c2d->transposeZ2Y_MajorIndex(tempZ4, xE23);

	getRange(xE13, "xE13", pySize[0], pySize[1], pySize[2], mpiRank);
	getRange(xE23, "xE23", pySize[0], pySize[1], pySize[2], mpiRank);


	//Now calculate intermediate components
	double *Va1, *Va2, *Va3;
	double *Vb1, *Vb2, *Vb3;
	double *Vc1, *Vc2, *Vc3;

	c2d->allocY(Va1);
	c2d->allocY(Va2);
	c2d->allocY(Va3);
	c2d->allocY(Vb1);
	c2d->allocY(Vb2);
	c2d->allocY(Vb3);
	c2d->allocY(Vc1);
	c2d->allocY(Vc2);
	c2d->allocY(Vc3);

	FOR_XYZ_YPEN{
	     Va1[ip] = xE11[ip]*y[ip]; 
	     Va2[ip] = xE12[ip]*y[ip]; 
	     Va3[ip] = xE13[ip]*y[ip]; 

	     Vb1[ip] = xE11[ip]*z[ip];
	     Vb2[ip] = xE12[ip]*z[ip];
	     Vb3[ip] = xE13[ip]*z[ip];

	     Vc1[ip] = xE21[ip]*z[ip];
	     Vc2[ip] = xE22[ip]*z[ip];
	     Vc3[ip] = xE23[ip]*z[ip];
	}

	getRange(Vb3, "Vb3", pySize[0], pySize[1], pySize[2], mpiRank);
	getRange(Vc1, "Vc1", pySize[0], pySize[1], pySize[2], mpiRank);

	//Only need the off-diagonals of the outer grad tensor of the vectors
	double *dVa12, *dVa13;
	double *dVa21, *dVa23;
	double *dVa31, *dVa32;

	c2d->allocY(dVa12);
	c2d->allocY(dVa13);
	c2d->allocY(dVa21);
	c2d->allocY(dVa23);
	c2d->allocY(dVa31);
	c2d->allocY(dVa32);

	double *dVb12, *dVb13;
	double *dVb21, *dVb23;
	double *dVb31, *dVb32;

	c2d->allocY(dVb12);
	c2d->allocY(dVb13);
	c2d->allocY(dVb21);
	c2d->allocY(dVb23);
	c2d->allocY(dVb31);
	c2d->allocY(dVb32);

	double *dVc12, *dVc13;
	double *dVc21, *dVc23;
	double *dVc31, *dVc32;

	c2d->allocY(dVc12);
	c2d->allocY(dVc13);
	c2d->allocY(dVc21);
	c2d->allocY(dVc23);
	c2d->allocY(dVc31);
	c2d->allocY(dVc32);

	//Start doing the E2 derivatives of all of this stuff
	if(periodicX){
	    double *Nm2a1, *Nm1a1, *Np1a1, *Np2a1;
	    Nm2a1 = new double[pySize[0]*pySize[2]];
	    Nm1a1 = new double[pySize[0]*pySize[2]];
	    Np1a1 = new double[pySize[0]*pySize[2]];
	    Np2a1 = new double[pySize[0]*pySize[2]];

	    double *Nm2a3, *Nm1a3, *Np1a3, *Np2a3;
	    Nm2a3 = new double[pySize[0]*pySize[2]];
	    Nm1a3 = new double[pySize[0]*pySize[2]];
	    Np1a3 = new double[pySize[0]*pySize[2]];
	    Np2a3 = new double[pySize[0]*pySize[2]];

	    double *Nm2b1, *Nm1b1, *Np1b1, *Np2b1;
	    Nm2b1 = new double[pySize[0]*pySize[2]];
	    Nm1b1 = new double[pySize[0]*pySize[2]];
	    Np1b1 = new double[pySize[0]*pySize[2]];
	    Np2b1 = new double[pySize[0]*pySize[2]];

	    double *Nm2b3, *Nm1b3, *Np1b3, *Np2b3;
	    Nm2b3 = new double[pySize[0]*pySize[2]];
	    Nm1b3 = new double[pySize[0]*pySize[2]];
	    Np1b3 = new double[pySize[0]*pySize[2]];
	    Np2b3 = new double[pySize[0]*pySize[2]];

	    double *Nm2c1, *Nm1c1, *Np1c1, *Np2c1;
	    Nm2c1 = new double[pySize[0]*pySize[2]];
	    Nm1c1 = new double[pySize[0]*pySize[2]];
	    Np1c1 = new double[pySize[0]*pySize[2]];
	    Np2c1 = new double[pySize[0]*pySize[2]];

	    double *Nm2c3, *Nm1c3, *Np1c3, *Np2c3;
	    Nm2c3 = new double[pySize[0]*pySize[2]];
	    Nm1c3 = new double[pySize[0]*pySize[2]];
	    Np1c3 = new double[pySize[0]*pySize[2]];
	    Np2c3 = new double[pySize[0]*pySize[2]];


	    FOR_X_YPEN{
		FOR_Z_YPEN{
		    int ii = i*pySize[2] + k;

		    int iim2 = i*pySize[2]*pySize[1] + k*pySize[1] + pySize[1]-2;		
		    int iim1 = i*pySize[2]*pySize[1] + k*pySize[1] + pySize[1]-1;		
		    int iip1 = i*pySize[2]*pySize[1] + k*pySize[1] + 0;		
		    int iip2 = i*pySize[2]*pySize[1] + k*pySize[1] + 1;		

		    Nm2a1[ii] = (y[iim2]-periodicYTranslation[1])*xE11[iim2];
		    Nm1a1[ii] = (y[iim1]-periodicYTranslation[1])*xE11[iim1];
		    Np1a1[ii] = (y[iip1]+periodicYTranslation[1])*xE11[iip1];
		    Np2a1[ii] = (y[iip2]+periodicYTranslation[1])*xE11[iip2];
	
		    Nm2a3[ii] = (y[iim2]-periodicYTranslation[1])*xE13[iim2];
		    Nm1a3[ii] = (y[iim1]-periodicYTranslation[1])*xE13[iim1];
		    Np1a3[ii] = (y[iip1]+periodicYTranslation[1])*xE13[iip1];
		    Np2a3[ii] = (y[iip2]+periodicYTranslation[1])*xE13[iip2];

		    Nm2b1[ii] = (z[iim2]-periodicYTranslation[2])*xE11[iim2];
		    Nm1b1[ii] = (z[iim1]-periodicYTranslation[2])*xE11[iim1];
		    Np1b1[ii] = (z[iip1]+periodicYTranslation[2])*xE11[iip1];
		    Np2b1[ii] = (z[iip2]+periodicYTranslation[2])*xE11[iip2];
	
		    Nm2b3[ii] = (z[iim2]-periodicYTranslation[2])*xE13[iim2];
		    Nm1b3[ii] = (z[iim1]-periodicYTranslation[2])*xE13[iim1];
		    Np1b3[ii] = (z[iip1]+periodicYTranslation[2])*xE13[iip1];
		    Np2b3[ii] = (z[iip2]+periodicYTranslation[2])*xE13[iip2];
	
		    Nm2c1[ii] = (z[iim2]-periodicYTranslation[2])*xE21[iim2];
		    Nm1c1[ii] = (z[iim1]-periodicYTranslation[2])*xE21[iim1];
		    Np1c1[ii] = (z[iip1]+periodicYTranslation[2])*xE21[iip1];
		    Np2c1[ii] = (z[iip2]+periodicYTranslation[2])*xE21[iip2];
	
		    Nm2c3[ii] = (z[iim2]-periodicYTranslation[2])*xE23[iim2];
		    Nm1c3[ii] = (z[iim1]-periodicYTranslation[2])*xE23[iim1];
		    Np1c3[ii] = (z[iip1]+periodicYTranslation[2])*xE23[iip1];
		    Np2c3[ii] = (z[iip2]+periodicYTranslation[2])*xE23[iip2];
	

		}
	    }

	    derivY->calc1stDerivField_TPB(Va1, dVa12, Nm2a1, Nm1a1, Np1a1, Np2a1);
	    derivY->calc1stDerivField_TPB(Va3, dVa32, Nm2a3, Nm1a3, Np1a3, Np2a3);

	    derivY->calc1stDerivField_TPB(Vb1, dVb12, Nm2b1, Nm1b1, Np1b1, Np2b1);
	    derivY->calc1stDerivField_TPB(Vb3, dVb32, Nm2b3, Nm1b3, Np1b3, Np2b3);

	    derivY->calc1stDerivField_TPB(Vc1, dVc12, Nm2c1, Nm1c1, Np1c1, Np2c1);
	    derivY->calc1stDerivField_TPB(Vc3, dVc32, Nm2c3, Nm1c3, Np1c3, Np2c3);


	    delete[] Nm2a1;
	    delete[] Nm1a1;
	    delete[] Np1a1;
	    delete[] Np2a1;

	    delete[] Nm2a3;
	    delete[] Nm1a3;
	    delete[] Np1a3;
	    delete[] Np2a3;

	    delete[] Nm2b1;
	    delete[] Nm1b1;
	    delete[] Np1b1;
	    delete[] Np2b1;

	    delete[] Nm2b3;
	    delete[] Nm1b3;
	    delete[] Np1b3;
	    delete[] Np2b3;

	    delete[] Nm2c1;
	    delete[] Nm1c1;
	    delete[] Np1c1;
	    delete[] Np2c1;

	    delete[] Nm2c3;
	    delete[] Nm1c3;
	    delete[] Np1c3;
	    delete[] Np2c3;

	}else{
	    derivY->calc1stDerivField(Va1, dVa12);
	    derivY->calc1stDerivField(Va3, dVa32);
	    derivY->calc1stDerivField(Vb1, dVb12);
	    derivY->calc1stDerivField(Vb3, dVb32);
	    derivY->calc1stDerivField(Vc1, dVc12);
  	    derivY->calc1stDerivField(Vc3, dVc32);
	}

	getRange(dVa12, "dVa12", pySize[0], pySize[1], pySize[2], mpiRank);
	getRange(dVa32, "dVa32", pySize[0], pySize[1], pySize[2], mpiRank);
	getRange(dVb12, "dVb12", pySize[0], pySize[1], pySize[2], mpiRank);
	getRange(dVb32, "dVb32", pySize[0], pySize[1], pySize[2], mpiRank);
	getRange(dVc12, "dVc12", pySize[0], pySize[1], pySize[2], mpiRank);
	getRange(dVc32, "dVc32", pySize[0], pySize[1], pySize[2], mpiRank);


	//Start doing the E1 derivatives...
	c2d->transposeY2X_MajorIndex(Va2, tempX1);
	c2d->transposeY2X_MajorIndex(Va3, tempX2);
	derivX->calc1stDerivField(tempX1, tempX3);
	derivX->calc1stDerivField(tempX2, tempX4);
	c2d->transposeX2Y_MajorIndex(tempX3, dVa21);
	c2d->transposeX2Y_MajorIndex(tempX4, dVa31);

	c2d->transposeY2X_MajorIndex(Vb2, tempX1);
	c2d->transposeY2X_MajorIndex(Vb3, tempX2);
	derivX->calc1stDerivField(tempX1, tempX3);
	derivX->calc1stDerivField(tempX2, tempX4);
	c2d->transposeX2Y_MajorIndex(tempX3, dVb21);
	c2d->transposeX2Y_MajorIndex(tempX4, dVb31);

	c2d->transposeY2X_MajorIndex(Vc2, tempX1);
	c2d->transposeY2X_MajorIndex(Vc3, tempX2);
	derivX->calc1stDerivField(tempX1, tempX3);
	derivX->calc1stDerivField(tempX2, tempX4);
	c2d->transposeX2Y_MajorIndex(tempX3, dVc21);
	c2d->transposeX2Y_MajorIndex(tempX4, dVc31);

	//Start doing the E3 derivatives...
	c2d->transposeY2Z_MajorIndex(Va1, tempZ1);
	c2d->transposeY2Z_MajorIndex(Va2, tempZ2);
	derivZ->calc1stDerivField(tempZ1, tempZ3);
	derivZ->calc1stDerivField(tempZ2, tempZ4);
	c2d->transposeZ2Y_MajorIndex(tempZ3, dVa13);
	c2d->transposeZ2Y_MajorIndex(tempZ4, dVa23);

	c2d->transposeY2Z_MajorIndex(Vb1, tempZ1);
	c2d->transposeY2Z_MajorIndex(Vb2, tempZ2);
	derivZ->calc1stDerivField(tempZ1, tempZ3);
	derivZ->calc1stDerivField(tempZ2, tempZ4);
	c2d->transposeZ2Y_MajorIndex(tempZ3, dVb13);
	c2d->transposeZ2Y_MajorIndex(tempZ4, dVb23);

	c2d->transposeY2Z_MajorIndex(Vc1, tempZ1);
	c2d->transposeY2Z_MajorIndex(Vc2, tempZ2);
	derivZ->calc1stDerivField(tempZ1, tempZ3);
	derivZ->calc1stDerivField(tempZ2, tempZ4);
	c2d->transposeZ2Y_MajorIndex(tempZ3, dVc13);
	c2d->transposeZ2Y_MajorIndex(tempZ4, dVc23);

	c2d->deallocXYZ(xE11);
	c2d->deallocXYZ(xE12);
	c2d->deallocXYZ(xE13);
	c2d->deallocXYZ(xE21);
	c2d->deallocXYZ(xE22);
	c2d->deallocXYZ(xE23);

	//Start calculating the Jacobian components [(dE/dx)/J]...
	FOR_XYZ_YPEN{
	    J11[ip] = dVc23[ip] - dVc32[ip];
	    J12[ip] = dVb32[ip] - dVb23[ip];
	    J13[ip] = dVa23[ip] - dVa32[ip];

	    J21[ip] = dVc31[ip] - dVc13[ip];
	    J22[ip] = dVb13[ip] - dVb31[ip];
	    J23[ip] = dVa31[ip] - dVa13[ip];

	    J31[ip] = dVc12[ip] - dVc21[ip];
	    J32[ip] = dVb21[ip] - dVb12[ip];
	    J33[ip] = dVa12[ip] - dVa12[ip];
	}

	//Free up all of this space here...
	c2d->deallocXYZ(dVa12);
	c2d->deallocXYZ(dVa13);
	c2d->deallocXYZ(dVa21);
	c2d->deallocXYZ(dVa23);
	c2d->deallocXYZ(dVa31);
	c2d->deallocXYZ(dVa32);

	c2d->deallocXYZ(dVb12);
	c2d->deallocXYZ(dVb13);
	c2d->deallocXYZ(dVb21);
	c2d->deallocXYZ(dVb23);
	c2d->deallocXYZ(dVb31);
	c2d->deallocXYZ(dVb32);

	c2d->deallocXYZ(dVc12);
	c2d->deallocXYZ(dVc13);
	c2d->deallocXYZ(dVc21);
	c2d->deallocXYZ(dVc23);
	c2d->deallocXYZ(dVc31);
	c2d->deallocXYZ(dVc32);

	c2d->deallocXYZ(tempX3);
	c2d->deallocXYZ(tempX4);
	c2d->deallocXYZ(tempZ3);
	c2d->deallocXYZ(tempZ4);


	//Now start computing the Jacobian determinant...
	double *preJdet1, *preJdet2, *preJdet3;
	double *Jdet1, *Jdet2, *Jdet3;
	c2d->allocY(preJdet1);
	c2d->allocY(preJdet2);
	c2d->allocY(preJdet3);
	c2d->allocY(Jdet1);
	c2d->allocY(Jdet2);
	c2d->allocY(Jdet3);

	FOR_XYZ_YPEN{
	    preJdet1[ip] = x[ip]*J11[ip] + y[ip]*J12[ip] + z[ip]*J13[ip];
	    preJdet2[ip] = x[ip]*J21[ip] + y[ip]*J22[ip] + z[ip]*J23[ip];
	    preJdet3[ip] = x[ip]*J31[ip] + y[ip]*J32[ip] + z[ip]*J33[ip];
	}

	//Compute the E2 component...
	derivY->calc1stDerivField(preJdet2, Jdet2);

	//Compute the E1 component....
	c2d->transposeY2X_MajorIndex(preJdet1, tempX1);
	derivX->calc1stDerivField(tempX1, tempX2);
	c2d->transposeX2Y_MajorIndex(tempX2, Jdet1);

 	//Compute the E3 component....
	c2d->transposeY2Z_MajorIndex(preJdet3, tempZ1);
	derivZ->calc1stDerivField(tempZ1, tempZ2);
	c2d->transposeZ2Y_MajorIndex(tempZ2, Jdet3);

	//Compute the Jacobian derivative...
	FOR_XYZ_YPEN{
	    J[ip] = (1.0/3.0)*(Jdet1[ip] + Jdet2[ip] + Jdet3[ip]);
	}

	//Free up all of the spaces we've been using...
	c2d->deallocXYZ(tempX1);
	c2d->deallocXYZ(tempX2);
	c2d->deallocXYZ(tempZ1);
	c2d->deallocXYZ(tempZ2);

	c2d->deallocXYZ(preJdet1);
	c2d->deallocXYZ(preJdet2);
	c2d->deallocXYZ(preJdet3);

	c2d->deallocXYZ(Jdet1);
	c2d->deallocXYZ(Jdet2);
	c2d->deallocXYZ(Jdet3);

}

#endif
