
#ifndef FTRFF_h
#define FTRFF_h

#include "fastmath.h"
#include "Vec2.h"
#include "Vec3.h"
#include "quaternion.h"
#include "Forces.h"

/*
Rigid Atom Reactive Force-field
===============================

Problem:  binding of several atoms to one center
Solution: bonds repel for opposite side (back-side)

Electron Pairs
 - To prevent many atoms 

Passivation atoms
 - except "scafold atoms" (C,O,N) there are termination atoms (-H, Cl) wich r



Optimization:
    - We can separate rotation and movement of atoms, rotation should be much faster
    - This makes only sense if we use forcefield with cheap evaluation of torque for fixed distance 
        - we can perhaps save sqrt() calculation 


*/

#define N_BOND_MAX 4
#define R1SAFE    1e-8

static const double sp3_hs[] = {
-0.57735026919, -0.57735026919, -0.57735026919,
+0.57735026919, +0.57735026919, -0.57735026919,
-0.57735026919, +0.57735026919, +0.57735026919,
+0.57735026919, -0.57735026919, +0.57735026919
};

static const double sp2_hs[] = {
+1.000000000000, -0.00000000000,  0.00000000000,
-0.500000000000, +0.86602540378,  0.00000000000,
-0.500000000000, -0.86602540378,  0.00000000000,
 0.00000000000,   0.00000000000,  1.00000000000     // electron - can be repulsive
};

static const double sp1_hs[] = {
+1.000000000000,  0.00000000000,  0.00000000000,
-1.000000000000,  0.00000000000,  0.00000000000,
 0.000000000000,  1.00000000000,  0.00000000000,    // electron - can be repulsive
 0.000000000000,  0.00000000000,  1.00000000000     // electron - can be repulsive
};


inline void overlapFE(double r, double amp, double beta, double& e, double& fr ){ 
    //https://www.wolframalpha.com/input/?i=exp(b*x)*(1%2Bb*x%2B(b*x)%5E2%2F3)+derivative+by+x
    double x     = r*beta;
    double expar = amp*exp(-x);
    e  =  expar*(1 +   x + 0.33333333*x*x ); 
    fr = (expar*(6 + 5*x +            x*x )*beta*0.33333333)/r;
}

template<typename T>
void rotateVectors(int n, const Quat4TYPE<T>& qrot, Vec3TYPE<T>* h0s, Vec3TYPE<T>* hs ){
    Mat3TYPE<T> mrot; 
    qrot.toMatrix(mrot);
    for( int j=0; j<n; j++ ){
        Vec3TYPE<T> h;
        mrot.dot_to_T    ( h0s[j], h );
        hs[j] = h;
        //ps[j].set_add_mul( pos, p_, r0 );
    }
}

struct RigidAtomType{
    int    nbond = 4;  // number bonds
    double rbond0 = 0.5;
    double acore =  4.0;
    double bcore = -0.7;
    double abond = -2.0;
    double bbond = -1.1;
    Vec3d* bh0s = (Vec3d*)sp3_hs;

    inline void combine(const RigidAtomType& a, const RigidAtomType& b ){
        nbond  = a.nbond;
        acore  = a.acore  * b.acore ;  // TODO
        bcore  = a.bcore  + b.bcore ;
        abond  = -(a.abond  * b.abond); 
        bbond  = a.bbond  + b.bbond ;
        rbond0 = a.rbond0 + b.rbond0;
    }

    void print(){
        printf( "nbond %i rbond0 %g\n", nbond, rbond0 );
        printf( "abond %g bbond  %g\n", abond, bbond );
        printf( "acore %g bcore  %g\n", acore, bcore );
    }
};

struct RigidAtom{
    RigidAtomType* type = 0;
    Vec3d  pos;
    Quat4d qrot;
    //Quat4d frot;
    Vec3d torq;
    Vec3d force;

    inline void cleanForceTorq(){ torq=Vec3dZero; force=Vec3dZero; };
    inline void setPose(const Vec3d& pos_, const Quat4d& qrot_ ){ pos=pos_; qrot=qrot_; };

    inline void moveRotGD(double dt){   
        qrot.dRot_exact( dt, torq );  qrot.normalize();          // costly => debug
        //dRot_taylor2( dt, torq );   qrot.normalize_taylor3();  // fast => op
    }

    inline void movePosGD(double dt){ pos.add_mul(force,dt); }

};


class RARFF{ public:

    double invMassRot = 2.0;

    int natom    =0;
    RigidAtomType* types=0;
    RigidAtom*     atoms=0;

    void realloc(int natom_){
        natom=natom_;
        _realloc(atoms,natom);
    }

    inline double pairEF( const Vec3d& dij, const RigidAtomType& type, Vec3d* bhs, Vec3d& force, Vec3d& torq ){
        double rij = dij.norm() + R1SAFE;

        double eij = type.acore*exp(type.bcore*rij);
        //Vec3d  fij = dij*( -eij*type.bcore );
        //printf( "fij (%g,%g,%g)\n", fij.x,fij.y,fij.z );
        //force.add(fij);

        //force=dij*(( -eij*type.bcore )/rij);
        force=dij*(( eij*type.bcore )/rij);
        torq =Vec3dZero;

        double E = eij;
        //double E = 0;

        //printf("\n", type.nbond );

        // bonds interactions
        
        for(int ib=0; ib<type.nbond; ib++){
        //for(int ib=0; ib<1; ib++){

                Vec3d  db = bhs[ib]*type.rbond0;

                Vec3d  d  = dij - db;
                //double r  = d.norm() + R1SAF

                const double R2BOND = 0.1;
                double r  = sqrt( d.norm2() + R2BOND );

                double e,fr;
                e  = type.abond*exp(type.bbond*r);
                fr = e*type.bbond/r;
                //overlapFE(r,type.abond,-type.bbond,e,fr);

                //printf( "ib %i rij %g r %g eij %g e %g abond  %g  acore %g \n", ib, rij, r, eij, e, type.abond, type.acore  );
                E += e;
                Vec3d f =  d*fr;
                force.add( f );
                torq .add_cross( f, db            );
        }
        

        //exit(0);
        return E;
    }

    void cleanAtomForce(){ for(int i=0; i<natom; i++){ atoms[i].cleanForceTorq(); } }

    double interEF(){
        //Vec3d bps[N_BOND_MAX];
        Vec3d bhs[N_BOND_MAX];
        double E = 0;

        for(int i=0; i<natom; i++){
        //for(int i=0; i<1; i++){
        //for(int i=1; i<2; i++){
            RigidAtom&     atomi = atoms[i];
            RigidAtomType& typei = *atomi.type;
            //int            nbi   = typei.nbond;
            Vec3d           pi   = atomi.pos;

            rotateVectors<double>(N_BOND_MAX, atomi.qrot, typei.bh0s, bhs );

            for(int j=0; j<natom; j++){
                RigidAtom&     atomj = atoms[j];
                //RigidAtomType& typej = *atomi.type;

                RigidAtomType pairType;
                pairType.combine( typei, *atomi.type );

                Vec3d  dij = atomj.pos - pi;

                Vec3d force=Vec3dZero;
                Vec3d torq =Vec3dZero;
                E += pairEF(dij,pairType,bhs,force,torq);

                atomi.torq .add(torq);
                atomi.force.add(force);
                atomj.force.sub(force);

                /*
                // core-core interactions

                double acore = typei.acore * typej.acore ;  // TODO
                double bcore = typei.bcore + typej.bcore ;
                double abond = typei.abond * typej.abond ; 
                double bbond = typei.bbond + typej.bbond ;

                Vec3d  dij = atomj.pos - pi;
                double rij = dij.norm();

                double eij = acore*exp(bcore*rij);
                Vec3d  fij = dij*( eij*bcore );

                atomi.force.sub(fij);
                atomj.force.add(fij);

                double rbond0 = typei.rbond0 + typej.rbond0;

                // bonds interactions
                for(int ib=0; ib<nbi; ib++){

                        Vec3d  db = bhs[ib]*rbond0;

                        Vec3d  d  = dij + db;
                        double r  = d.norm();

                        double e = abond*exp(bbond*r);
                        Vec3d f  = d*( e*bbond );

                        atomi.torq .add_cross(f,db);
                        atomi.force.add(f);
                        atomj.force.sub(f);

                }
                */
            }
        }
    }

    void move(double dt){
        for(int i=0; i<natom; i++){
            atoms[i].moveRotGD(dt*invMassRot);
            atoms[i].movePosGD(dt);
        }
    }

};



#endif
