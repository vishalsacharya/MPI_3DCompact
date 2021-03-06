#ifndef _UTILSH_
#define _UTILSH_

#include <math.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "Macros.hpp"
#include "Domain.hpp"

using namespace std;

//Tridiagonal Matrix solver stuff
void solveTri(double a[], double b[], double c[], double d[], double x[], double *work, int n);
void cyclic(double *a, double *b, double *c, double alpha, double beta,
        double *r, int n, double *x);

//Transpose Stuff
void transposeMatrix(double *in, int Nx, int Ny, double *out);
void transposeMatrix_Fast1(const double *in, int n, int p, double *out, int block);
void transposeMatrix_Fast2(const double *in, int n, int p, double *out, int blocksize);

void transposeXYZtoYZX(const double *in, int Nx, int Ny, int Nz, double *out);
void transposeXYZtoYZX_Fast(const double *in, int Nx, int Ny, int Nz, double *out, int blocksize);
void transposeXYZtoZXY(const double *in, int Nx, int Ny, int Nz, double *out);
void transposeXYZtoZXY_Fast(const double *in, int Nx, int Ny, int Nz, double *out, int blocksize);
void transposeYZXtoZXY(const double *in, int Nx, int Ny, int Nz, double *out);
void transposeYZXtoZXY_Fast(const double *in, int Nx, int Ny, int Nz, double *out, int blocksize);
void transposeZXYtoXYZ(const double *in, int Nx, int Ny, int Nz, double *out);
void transposeZXYtoXYZ_Fast(const double *in, int Nx, int Ny, int Nz, double *out, int blocksize);
void transposeYZXtoXYZ(const double *in, int Nx, int Ny, int Nz, double *out);
void transposeYZXtoXYZ_Fast(const double *in, int Nx, int Ny, int Nz, double *out, int blocksize);

//Random number generation
double fRand(double fMin, double fMax);


void getRange(double *phi, string Var, int Nx, int Ny, int Nz, int mpiRank);
void getRangeValue(double *phi, int Nx, int Ny, int Nz, int mpiRank, double &globalDataMin, double &globalDataMax);

bool isPointInHexa(double p[3], double vertex[8][3]);
double getHexaVolume(double vertex[8][3]);
double getHexaVolumeWithPoint(double vertex[8][3], double P[3]);

//derivatives at boundaries, maybe go somewhere else?
inline double calcNeumann(double f1, double f2, double f3, double f4, double f5, double f6){
    return (f1*360.0 - f2*450.0 + f3*400.0 - f4*225.0 + f5*72.0 - f6*10.0)/147.0;
}

#endif
