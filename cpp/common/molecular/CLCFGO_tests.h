
#ifndef CLCFGO_tests_h
#define CLCFGO_tests_h

#include "integration.h"
#include "AOIntegrals.h"
//#include "AOrotations.h"
#include "Grid.h"
#include "CLCFGO.h"
#include "VecN.h"

#define iTEST_POS_DERIV  0
#define iTEST_SIZE_DERIV 1

void testDerivsCoulombModel( CLCFGO& solver, int n, double* xs, double* Es, double* Fs, int what  ){
    //initTestElectrons( );
    solver.toRho(0,1, 0);   
    solver.toRho(2,3, 1);
    for(int i=0; i<n; i++){
        DEBUG_iter=i;
        solver.clearAuxDens();
        solver.cleanForces();
        switch(what){
            case iTEST_POS_DERIV : solver.epos [0].x=xs[i];       break;
            case iTEST_SIZE_DERIV: solver.esize[0]  =fabs(xs[i]); break;
        }
        solver.toRho  (0,1, 0);                          // w0*w1 -> q0
        solver.toRho  (2,3, 1);                          // w2*w3 -> q1
        double E_  = solver.CoublombElement(0,1);        // E = C( q0, q1 )
        double E   = E_ * solver.rhoQ[0]*solver.rhoQ[1]; // E(q0,q1) * q0 * q1
        solver.fromRho( 0,1, 0 );                   // w0,w1 <- q0
        Es[i] = E;
        switch(what){
            case iTEST_POS_DERIV : Fs[i]=solver.efpos [0].x; break;
            case iTEST_SIZE_DERIV: Fs[i]=solver.efsize[0];   break;
        }
        //l_Q     [i] = solver.rhoQ[0];
        //l_dQ_ana[i] = solver.DEBUG_dQdp.x;
    }
}

void testDerivsTotal( CLCFGO& solver, int n, double* xs, double* Es, double* Fs, int what ){
    solver.reportOrbitals();
    for(int i=0; i<n; i++){
        //printf( "===== testDerivsTotal[%i]\n", i  );
        DEBUG_iter=i;
        //if(DEBUG_iter==DEBUG_log_iter){ printf("before switch(what) \n"); solver.reportOrbitals(); }
        switch(what){
            case iTEST_POS_DERIV : solver.epos [0].x=xs[i];        break;
            case iTEST_SIZE_DERIV: solver.esize[0]  =fabs(xs[i]);  break;
        }
        //if(DEBUG_iter==DEBUG_log_iter){ printf("before cleanForces() \n"); solver.reportOrbitals(); }
        solver.cleanForces();
        //if(DEBUG_iter==DEBUG_log_iter){ printf("after cleanForces() \n"); solver.reportOrbitals(); }
        double E  = solver.eval();
        //double xc = solver.rhoP[2].x; //This may be in aux struct
        Es[i] = E;
        switch(what){
            case iTEST_POS_DERIV : Fs[i]=solver.efpos [0].x; break;
            case iTEST_SIZE_DERIV: Fs[i]=solver.efsize[0];   break;
        }
    }
}

#endif