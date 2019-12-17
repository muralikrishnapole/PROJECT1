
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>

#include "testUtils.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "Draw3D.h"
#include "SDL_utils.h"
#include "Solids.h"

#include "fastmath.h"
#include "Vec3.h"
#include "Mat3.h"

#include "raytrace.h"
//#include "Molecule.h"
#include "MMFFmini.h"
#include "NBFF.h"
#include "MMFFBuilder.h"
#include "DynamicOpt.h"
#include "QEq.h"

//#include "NBSRFF.h"
//#include "IO_utils.h"

#include "AppSDL2OGL_3D.h"


// ==========================
// TestAppSoftMolDyn
// ==========================

void plotSurfPlane( Vec3d normal, double c0, Vec2d d, Vec2i n ){
    Vec3d da,db;
    normal.getSomeOrtho( da,db );
    da.mul( d.a/da.norm() );
    db.mul( d.b/db.norm() );
    //glColor3f(1.0f,0.0f,0.0f); Draw3D::drawVecInPos(normal, {0.0,0.0,0.0} );
    //glColor3f(0.0f,1.0f,0.0f); Draw3D::drawVecInPos(da*10, {0.0,0.0,0.0} );
    //glColor3f(0.0f,0.0f,1.0f); Draw3D::drawVecInPos(db*10, {0.0,0.0,0.0} );
    Draw3D::drawRectGridLines( n*2, (da*-n.a)+(db*-n.b) + normal*c0, da, db );
}

void drawAngle(int i, MMFFmini& ff){
    //Vec2i b=ff.ang2bond[i];
    float sz=0.1;
    Vec3i a=ff.ang2atom[i];
    Draw3D::drawArrow(ff.apos[a.x],ff.apos[a.y],sz);
    Draw3D::drawArrow(ff.apos[a.y],ff.apos[a.z],sz);
    //if(b.i&SIGN_MASK){ Draw3D::drawArrow(apos[a.x],apos[a.y]); }else{ Draw3D::drawArrow(apos[a.x],apos[a.y]); };
    //if(b.j&SIGN_MASK){   };
}

void drawTors(int i, MMFFmini& ff){
    float sz=0.1;
    //Vec2i b=ff.ang2bond[i];
    Quat4i t=ff.tors2atom[i];
    Draw3D::drawArrow(ff.apos[t.x],ff.apos[t.y],sz);
    Draw3D::drawArrow(ff.apos[t.y],ff.apos[t.z],sz);
    Draw3D::drawArrow(ff.apos[t.z],ff.apos[t.w],sz);
    //if(b.i&SIGN_MASK){ Draw3D::drawArrow(apos[a.x],apos[a.y]); }else{ Draw3D::drawArrow(apos[a.x],apos[a.y]); };
    //if(b.j&SIGN_MASK){   };
}

class TestAppSoftMolDyn : public AppSDL2OGL_3D {
	public:
	Molecule    mol;
	//MMFFparams  params;
    MMFFmini    ff;
    NBFF       nff;
    MMFFBuilder builder;
    DynamicOpt  opt;

    int     fontTex;
    int     ogl_sph;

    char str[256];

    Vec3d ray0;
    int ipicked  = -1, ibpicked = -1;
    int perFrame =  50;

    double drndv =  10.0;
    double drndp =  0.5;

	virtual void draw   ();
	virtual void drawHUD();
	//virtual void mouseHandling( );
	virtual void eventHandling   ( const SDL_Event& event  );
	//virtual void keyStateHandling( const Uint8 *keys );

	TestAppSoftMolDyn( int& id, int WIDTH_, int HEIGHT_ );

};




bool bPrint = true;


inline double cos_func(double ca, double cb, double cab ){  return ( cab - ca*cb )/sqrt( (1-ca*ca)*(1-cb*cb) ); }

inline void ddot_num(Vec3d h, Vec3d h2, Vec3d& f, double d){
    h.normalize(); h2.normalize();
    h.x+=d; h.normalize(); f.x=h.dot(h2); h.x-=2*d; h.normalize(); f.x-=h.dot(h2); h.x+=d;
    h.y+=d; h.normalize(); f.y=h.dot(h2); h.y-=2*d; h.normalize(); f.y-=h.dot(h2); h.y+=d;
    h.z+=d; h.normalize(); f.z=h.dot(h2); h.z-=2*d; h.normalize(); f.z-=h.dot(h2); h.z+=d;
    f.mul(1/(d*2));
}

//inline double evalTorq(Vec3d& ha,Vec3d& hb,Vec3d& hab,   Vec3d& f1, Vec3d& f2, Vec3d& f2, Vec3d& f4 ){
inline double evalTorq(Vec3d& ha,Vec3d& hb,Vec3d& hab,   Vec3d& fa, Vec3d& fb, Vec3d& fab ){

    double invra  = 1.0d/ha .normalize();
    double invrb  = 1.0d/hb .normalize();
    double invrab = 1.0d/hab.normalize();

    //double invra3 = invra*invra*invra;
    //double invrb3 = invrb*invrb*invrb;
    //double invrc3 = invrc*invrc*invrc;

    // check
    Vec3d haT = ha - hab*hab.dot(ha); haT.normalize();
    Vec3d hbT = hb - hab*hab.dot(hb); hbT.normalize();
    double cT = haT.dot(hbT);


    double ca   = hab.dot(ha);
    double cb   = hab.dot(hb);
    double cab  = ha .dot(hb);
    double sa2  = (1-ca*ca);
    double sb2  = (1-cb*cb);
    double invs = 1/sqrt( sa2*sb2 );
    double c    = ( cab - ca*cb )*invs;  //  c = <  ha - <ha|hab>hab   | hb - <hb|hab>hab    >

    //double c   = cos_func(ca,cb,cab);

    //printf( "<fa|fb> %g cT %g cS %g \n", cs.x, cT, cS );

    // derivatives to get forces

    double invs3 = invs*invs*invs;
    double dcab  = invs;                           // dc/dcab = dc/d<ha|hb>
    double dca   = (1-cb*cb)*(ca*cab - cb)*invs3;  // dc/dca  = dc/d<ha|hab>
    double dcb   = (1-ca*ca)*(cb*cab - ca)*invs3;  // dc/dca  = dc/d<hb|hab>

    fa =Vec3dZero;
    fb =Vec3dZero;
    fab=Vec3dZero;

    Mat3Sd J;

    J.from_dhat(ha);    // -- by ha
    J.mad_ddot(hab,fa, dca ); // dca /dha = d<ha|hab>/dha
    J.mad_ddot(hb ,fa, dcab); // dcab/dha = d<ha|hb> /dha

    J.from_dhat(hb);    // -- by hb
    J.mad_ddot(hab,fb, dcb ); // dcb /dhb = d<hb|hab>/dha
    J.mad_ddot(ha ,fb, dcab); // dcab/dhb = d<hb|ha> /dha

    J.from_dhat(hab);         // -- by hab
    J.mad_ddot(ha,fab, dca);  // dca/dhab = d<ha|hab>/dhab
    J.mad_ddot(hb,fab, dcb);  // dcb/dhab = d<hb|hab>/dhab
    // derivative cab = <ha|hb>

    fa .mul(invra *invra *invra );
    fb .mul(invrb *invrb *invrb );
    fab.mul(invrab*invrab*invrab);

    if(bPrint){

        double d=0.001d;
        double dcaE  =( cos_func(ca+d,cb  ,cab  ) - cos_func(ca-d,cb  ,cab  ) )/(2*d);
        double dcbE  =( cos_func(ca  ,cb+d,cab  ) - cos_func(ca  ,cb-d,cab  ) )/(2*d);
        double dcabE =( cos_func(ca  ,cb  ,cab+d) - cos_func(ca  ,cb  ,cab-d) )/(2*d);


        Vec3d fE=Vec3dZero,f=Vec3dZero,ferr;
        Vec3d h;
        h=ha;

        ddot_num(ha,hab,fE,d); f=Vec3dZero; J.from_dhat(ha   );   J.dhat_dot(hab,f);
        printf( "d<a|ab>/da err %g : anal  (%g,%g,%g)  | num  (%g,%g,%g) \n", (f-fE).norm(), f.x,f.y,f.z,   fE.x,fE.y,fE.z );
        ddot_num(ha,hb,fE,d);  f=Vec3dZero; J.from_dhat(ha   );   J.dhat_dot(hb,f);
        printf( "d<a|b>/dha  err %g : anal  (%g,%g,%g)  | num  (%g,%g,%g) \n",(f-fE).norm(), f.x,f.y,f.z,   fE.x,fE.y,fE.z );

        ddot_num(hb,hab,fE,d); f=Vec3dZero; J.from_dhat(hb   );   J.dhat_dot(hab,f);
        printf( "d<b|ab>/da err %g : anal  (%g,%g,%g)  | num  (%g,%g,%g) \n",(f-fE).norm(), f.x,f.y,f.z,   fE.x,fE.y,fE.z );
        ddot_num(hb,ha,fE,d);  f=Vec3dZero; J.from_dhat(hb   );   J.dhat_dot(ha,f);
        printf( "d<a|b>/db  err %g : anal  (%g,%g,%g)  | num  (%g,%g,%g) \n",(f-fE).norm(), f.x,f.y,f.z,   fE.x,fE.y,fE.z );

        ddot_num(hab,ha,fE,d); f=Vec3dZero; J.from_dhat(hab   );   J.dhat_dot(ha,f);
        printf( "d<a|ab>/dab err %g : anal  (%g,%g,%g)  | num  (%g,%g,%g) \n",(f-fE).norm(), f.x,f.y,f.z,   fE.x,fE.y,fE.z );
        ddot_num(hab,hb,fE,d);  f=Vec3dZero; J.from_dhat(hab   );   J.dhat_dot(hb,f);
        printf( "d<b|ab>/dab err %g : anal  (%g,%g,%g)  | num  (%g,%g,%g) \n",(f-fE).norm(), f.x,f.y,f.z,   fE.x,fE.y,fE.z );

        printf( "dca,dcb,dcab : anal  (%g,%g,%g)  | num  (%g,%g,%g) \n", dca,dcb,dcab,   dcaE,dcbE,dcabE );
        printf( "c,cT   %g %g       ca,cb,cab (%g, %g, %g) \n",  c,cT,  ca,cb, cab );
        printf( " <ha|fa> %g <hb|fb> %g <hab|fa> %g <hab|fb> %g \n", ha.dot(fa), hb.dot(fb), hab.dot(fa), hab.dot(fb) );

    }
    //if(bPrint) printf( "n", fa.dot(fb), fa.dot(fb), fa.dot(fb), fa.dot(fb), );

    /*
    aforce[ia.x].add(fa*-1);
    aforce[ia.y].add(fa-fab);
    aforce[ia.z].add(fab-fb);
    aforce[ia.w].add(fb);
    */
    return c;
}

//template<typename T> std::function<T(const T&,const T&         )> F2;

TestAppSoftMolDyn::TestAppSoftMolDyn( int& id, int WIDTH_, int HEIGHT_ ) : AppSDL2OGL_3D( id, WIDTH_, HEIGHT_ ) {


    Vec3d ha      =(Vec3d){1.0, 1.0,-0.2};
    Vec3d hb      =(Vec3d){1.0,-1.0,+0.3};
    Vec3d hab     =(Vec3d){0.1, 0.2, 1.0};

    //ha.normalize();
    //hb.normalize();
    //hab.normalize();
    Vec3d fa,fb,fab;
    evalTorq( ha,hb,hab,  fa,fb,fab );
    bPrint=false;

    auto getEF_a  = [&](Vec3d p,Vec3d& f)->double{ return evalTorq( p,hb,hab,  f,fb,fab ); };
    auto getEF_b  = [&](Vec3d p,Vec3d& f)->double{ return evalTorq( ha,p,hab,  fa,f,fab ); };
    auto getEF_ab = [&](Vec3d p,Vec3d& f)->double{ return evalTorq( ha,hb,p,   fa,fb,f  ); };

    Vec3d f,fE;
    printf("ha  "); checkDeriv3d(getEF_a , ha, 0.001d,  fE, f );
    printf("hb  "); checkDeriv3d(getEF_b , hb, 0.001d,  fE, f );
    printf("hab "); checkDeriv3d(getEF_ab, hab,0.001d,  fE, f );

    //exit(0);
    /*
    double ds = 0.02;
    Vec3d& h  = ha0;
    Vec3d& f  =
    h   = ha;
    for(int i=0;i<3;i++){
        double E0,E1;
        h.array[i]-=ds*0.5;  E0= torqForce( ha,hb,hab,   fa,fb,fab );
        h.array[i]+=ds;      E1= torqForce( ha,hb,hab,   fa,fb,fab );
        double fE.array[i]=(E1-E0)/ds;
    }
    torqForce( ha,hb,hab, fa,fb,fab );

    printf( "fE(%g,%g,%g) fE(%g,%g,%g)\n", fE.x, fE.y, fE.z, f.x,f.y,f.z );
    */




    /*
    for(int i=0; i<n; i++){
        ha.x+=dx;
        double E  = torqForce( ha,hb,hab,   f1,f2,f2,f4 );
        double fx = (E-oE)/dx;
        printf( f1 );
    }
    */


    MMFFAtom a{1.0,0.0,0.0};
    printf( "print (%g,%g,%g)\n", a.pos.x, a.pos.y,a.pos.z );

    fontTex = makeTexture( "common_resources/dejvu_sans_mono_RGBA_inv.bmp" );

    double l0    = 1.5;
    double Kbond = 10.0;
    double Kang  = 3.0;
    double Ktors = 1.0;

    int tors_n = 3;


    //const int natom=4,nbond=3,nang=2,ntors=1;
    const int natom=4,nbond=3,nang=0,ntors=0;
    Vec3d apos0[] = {
        {-2.0,0.0,0.0},  // 0
        {-1.0,2.0,0.0},  // 1
        {+1.0,2.0,0.0},  // 2
        {+2.0,0.0,1.0},  // 3
    };
    Vec2i bong2atom[] = {
        {0,1},  // 0
        {1,2},  // 1
        {2,3},  // 2
    };
    Vec2i ang2bond[] = {
        {0,1},  // 0
        {1,2},  // 1
    };
    Vec3i tors2bond[] = {
        {0,1,2},  // 0
    };
    double a0s[] ={
        2.0,
        2.0,
        2.0,
    };
    DEBUG
/*
    const int natom=5+2,nbond=4+3,nang=6;
    Vec3d apos0[] = {
        { 0.5, 0.5, 0.5},  // 0
        {-1.0,+1.0,+1.0},  // 1
        {+1.0,-1.0,+1.0},  // 2
        {-1.0,-1.0,-1.0},  // 3
        {+1.0,+1.0,-1.0},  // 4

        {-1.0,-1.0,-2.0},  // 5
        {+1.0,+1.0,-2.0}   // 6

        //{1.0,0.0,0.0},  // 1
        //{0.0,1.0,0.0},  // 2
        //{0.0,0.0,1.0},  // 3
        //{-1.0,-1.0,-1.0}   // 4
    };
    Vec2i bong2atom[] = {
        {0,1},  // 0
        {0,2},  // 1
        {0,3},  // 2
        {0,4},  // 3

        {5,6},  // 4
        {3,5},  // 5
        {4,6}   // 6

    };
    Vec2i ang2bond[] = {
        {0,1},
        {0,2},
        {0,3},
        {1,2},
        {1,3},
        {2,3}
    };
    double a0s[] ={
        2.0,
        2.0,
        2.0,
        2.0,
        2.0,
        2.0
    };
    */

    // ============ Build molecule

    MMFFAtom brushAtom{ -1, -1,-1, Vec3dZero, MMFFAtom::defaultREQ };
    MMFFBond brushBond{ -1, -1,-1, 1.5, 25.0 };

    builder.capBond = MMFFBond{ -1, -1,-1, 1.07, 15.0 };

    printf( "----- Atoms \n" );
    for(int i=0;i<natom;i++){
        brushAtom.pos = apos0[i];
        builder.insertAtom(brushAtom, true);
    }
    printf( "----- Bonds \n" );
    for(int i=0;i<nbond;i++){
        brushBond.atoms=bong2atom[i];
        builder.insertBond(brushBond);
    }
    printf( "----- Confs \n" );
    for(int i=0;i<natom;i++){
        builder.makeSPConf(i,0,0);
        //builder.makeSPConf(i);
    }
    //exit(0);
    printf( "----- toMMFF \n" );
    builder.autoAngles( 2.5, 1.25 );
    printf( "----- toMMFF \n" );

    MMFFDihedral brushDihedral{ -1,   -1,-1,-1,    3, 0.5 };
    println(brushDihedral);
    //exit(0);

    builder.insertDihedralByAtom( {0,1,2,3}, brushDihedral );

    if( !builder.checkBondsSorted() ){
        if( !builder.sortBonds() ){
            printf( " ERROR in builder.sortBonds() => exit \n" );
            exit(0);
        }
    }
    //exit(0);
    builder.toMMFFmini( ff );

    //exit(0);
    /*
    ff.realloc(natom,nbond,nang,ntors);
    printf( "DEBUG 1 \n" );
    for(int i=0; i<ff.natoms; i++){
        ff.apos[i] = apos0[i];
    }
    printf( "DEBUG 2 \n" );
    for(int i=0; i<ff.nbonds; i++){
        ff.bond2atom[i] = bong2atom[i];
        ff.bond_k [i] = Kbond;
        ff.bond_l0[i] = l0;
    }
    printf( "DEBUG 3 \n" );
    for(int i=0; i<ff.nang; i++){
        ff.ang2bond[i] = ang2bond[i];
        double a0 = -a0s[i]/2.0; // NOTE: we use half-angle
        ff.ang_cs0[i] = { cos(a0), sin(a0) };
        ff.ang_k  [i] = Kang;
    }
    for(int i=0; i<ff.ntors; i++){
        ff.tors2bond[i] = tors2bond[i];
        ff.tors_k   [i] = Ktors;
        ff.tors_n   [i] = tors_n;
    }
    ff.angles_bond2atom  ();
    ff.torsions_bond2atom();
    printf( "DEBUG 4 \n" );
    */

    nff.bindOrRealloc( ff.natoms, ff.nbonds, ff.apos, ff.aforce, 0, ff.bond2atom );
    //nff.setREQs(0,nff.n, {1.4,0.0}  )
    nff.setREQs(0       ,nff.n,{1.9080,sqrt(0.003729),0});

    opt.bindOrAlloc( 3*ff.natoms, (double*)ff.apos, 0, (double*)ff.aforce, 0 );
    //opt.setInvMass( 1.0 );
    opt.cleanVel( );

    if( !checkParisSorted( nff.nmask, nff.pairMask ) ){
        printf( "ERROR: nff.pairMask is not sorted => exit \n" );
        exit(0);
    };

    ogl_sph = glGenLists(1);
    glNewList( ogl_sph, GL_COMPILE );
        //glEnable( GL_LIGHTING );
        //glColor3f( 0.8f, 0.8f, 0.8f );
        //Draw3D::drawSphere_oct(3, 0.5, {0.0,0.0,0.0} );
        Draw3D::drawSphere_oct( 2, 0.25, {0.0,0.0,0.0} );
    glEndList();

    //exit(0);

}

void TestAppSoftMolDyn::draw(){
    glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	/*
	//ibpicked = world.pickBond( ray0, camMat.c , 0.5 );
    ray0 = (Vec3d)(cam.rot.a*mouse_begin_x + cam.rot.b*mouse_begin_y);
    Draw3D::drawPointCross( ray0, 0.1 );
    //Draw3D::drawVecInPos( camMat.c, ray0 );
    if(ipicked>=0) Draw3D::drawLine( world.apos[ipicked], ray0);
    */


    /*
    for(int i=0; i<50; i++){
        ff.apos[1]={0.0,0.0};
        ff.apos[0]={1.0,0.0};
        double a = i*2*M_PI/50;
        ff.apos[2]={cos(a),sin(a)};

        ff.eval();

        glColor3f(1.0,1.0,1.0);
        Draw3D::drawVecInPos( (ff.apos[0]-ff.apos[1])*5.0, ff.apos[1] );
        Vec2d cs = ff.ang_cs0[0];
        cs.set_mul_cmplx(cs,cs);
        Draw3D::drawVecInPos( ((Vec3d){cs.x,-cs.y,0.0})*5.0, ff.apos[1] );
    */

    //ff.apos[0].set(-2.0,0.0,0.0);
    perFrame = 1;
    //perFrame = 50;
	for(int itr=0; itr<perFrame; itr++){
        printf( "======= frame %i \n", frameCount );

	    //printf( "DEBUG run 1 \n" );

	    // rotate arom[0]
        //ff.apos[0] = ff.apos[1] + (ff.apos[0]-ff.apos[1]).rotate( 2*M_PI/perFrame, ff.apos[2]-ff.apos[1] );

        double E=0;
        E += ff.eval();
        E += nff.evalLJQ_sortedMask();


        Vec3d cog,fsum,torq;
        checkForceInvariatns( ff.natoms, ff.aforce, ff.apos, cog, fsum, torq );
        printf( "fsum %g torq %g   cog (%g,%g,%g) \n", fsum.norm(), torq.norm(), cog.x, cog.y, cog.z );


        //for(int i=0; i<world.natoms; i++){ world.aforce[i].set(0.0d); }
        //printf( "DEBUG x.1 \n" );
        //world.eval_bonds(true);
        //world.eval_angles();
        //printf( "DEBUG x.2 \n" );
        //world.eval_angles();
        //printf( "DEBUG x.3 \n" );
        //world.eval_LJq_On2();

        /*
        //exit(0);
        if(ipicked>=0){
            Vec3d f = getForceSpringRay( world.apos[ipicked], (Vec3d)cam.rot.c, ray0, -1.0 );
            //printf( "f (%g,%g,%g)\n", f.x, f.y, f.z );
            world.aforce[ipicked].add( f );
        };


        for(int i=0; i<world.natoms; i++){
            world.aforce[i].add( getForceHamakerPlane( world.apos[i], {0.0,0.0,1.0}, -3.0, 0.3, 2.0 ) );
            //printf( "%g %g %g\n",  world.aforce[i].x, world.aforce[i].y, world.aforce[i].z );
        }
        */

        //exit(0);

        //for(int i=0; i<world.natoms; i++){ world.aforce[i].add({0.0,-0.01,0.0}); }
        //int ipivot = 0;
        //world.aforce[ipivot].set(0.0);
        //opt.move_LeapFrog(0.01);
        //opt.move_MDquench();

        //opt.move_GD(0.001);
        double f2 = opt.move_FIRE();
        //exit(0);
        //printf( "E %g |F| %g \n", E, sqrt(f2) );

    }


    glColor3f(0.0,1.0,0.0);
    //drawAngle(0, ff);
    //drawTors (0, ff);

    //glColor3f(0.6f,0.6f,0.6f); plotSurfPlane( (Vec3d){0.0,0.0,1.0}, -3.0, {3.0,3.0}, {20,20} );
    //Draw3D::drawVecInPos( (Vec3d){0.0,0.0,1.0},  (Vec3d){0.0,0.0,0.0} );

    //printf( "==== frameCount %i  |F| %g \n", frameCount, sqrt(F2) );
    //printf( "DEBUG run 2 \n" );
    // draw Bonds
    glColor3f(0.0f,0.0f,0.0f);

    for(int i=0; i<ff.nbonds; i++){
        Vec2i ib = ff.bond2atom[i];
        glColor3f(0.0f,0.0f,0.0f);
        //if(i==ibpicked) glColor3f(1.0f,0.0f,0.0f);
        Draw3D::drawLine(ff.apos[ib.x],ff.apos[ib.y]);
        //sprintf(str,"%i\0",i);
        //Draw3D::drawText(str, (world.apos[ib.x]+world.apos[ib.y])*0.5, fontTex, 0.02, 0,0);
        //Draw3D::drawText(str, (ff.apos[ib.x]+ff.apos[ib.y])*0.5, fontTex, 0.02, 0);
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    glColor3f(1.0f,1.0f,1.0f);

    for(int i=0; i<ff.nang; i++){
        Vec3i ia = ff.ang2atom[i];
        //glColor3f(0.0f,0.0f,0.0f);
        //if(i==ibpicked) glColor3f(1.0f,0.0f,0.0f);
        //Draw3D::drawTriangle(ff.apos[ia.x],ff.apos[ia.y],ff.apos[ia.z]);
        //sprintf(str,"%i\0",i);
        //Draw3D::drawText(str, (world.apos[ib.x]+world.apos[ib.y])*0.5, fontTex, 0.02, 0,0);
        //Draw3D::drawText(str, (ff.apos[ib.x]+ff.apos[ib.y])*0.5, fontTex, 0.02, 0);
    }


    //};
    // draw Atoms
    double fsc = 1.0;

    for(int i=0; i<ff.natoms; i++){
        //printf( "apos[%i] (%g,%g,%g)\n", i, ff.apos[i].x,ff.apos[i].y,ff.apos[i].z );
        //glColor3f(0.0f,0.0f,0.0f); Draw3D::drawPointCross(world.apos[i],0.2);
        //printf( "aforce[%i] (%g,%g,%g) \n", i,  ff.aforce[i].x, ff.aforce[i].y, ff.aforce[i].z );
        //glColor3f(0.0f,0.0f,1.0f); Draw3D::drawVecInPos(ff.aforce[i]*fsc,ff.apos[i]);
        glColor3f(0.0f,0.0f,0.0f); Draw3D::drawPointCross(ff.apos[i],0.1);
        /*
        //glCallList( ogl_sph );
        glEnable(GL_LIGHTING);
        Mat3d mat;
        mat.setOne();
        //mat.mul();
        glColor3f(0.8f,0.8f,0.8f);
        Draw3D::drawShape(ff.apos[i],mat,ogl_sph);
        glDisable(GL_LIGHTING);
        */
    }
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    /*
    printf("==========\n");
    for(int i=0; i<world.natoms; i++){
        printf("iatom %i (%g,%g,%g) (%g,%g,%g) \n", i, world.apos[i].x,world.apos[i].y,world.apos[i].z, world.aforce[i].x,world.aforce[i].y,world.aforce[i].z  );
    }
    if(frameCount>=10){STOP = true;}
    */

};


void TestAppSoftMolDyn::eventHandling ( const SDL_Event& event  ){
    //printf( "NonInert_seats::eventHandling() \n" );
    switch( event.type ){
        case SDL_KEYDOWN :
            switch( event.key.keysym.sym ){
                //case SDLK_p:  first_person = !first_person; break;
                //case SDLK_o:  perspective  = !perspective; break;
                //case SDLK_r:  world.fireProjectile( warrior1 ); break;

                case SDLK_LEFTBRACKET:  ff.i_DEBUG=(ff.i_DEBUG+1)%ff.nang; break;
                case SDLK_RIGHTBRACKET: ff.i_DEBUG=(ff.i_DEBUG+1)%ff.nang; break;

                //case SDLK_v: for(int i=0; i<world.natoms; i++){ ((Vec3d*)opt.vel)[i].add(randf(-drndv,drndv),randf(-drndv,drndv),randf(-drndv,drndv)); } break;
                //case SDLK_p: for(int i=0; i<world.natoms; i++){ world.apos[i].add(randf(-drndp,drndp),randf(-drndp,drndp),randf(-drndp,drndp)); } break;

                //case SDLK_LEFTBRACKET:  if(ibpicked>=0) world.bond_0[ibpicked] += 0.1; break;
                //case SDLK_RIGHTBRACKET: if(ibpicked>=0) world.bond_0[ibpicked] -= 0.1; break;

                //case SDLK_a: world.apos[1].rotate(  0.1, {0.0,0.0,1.0} ); break;
                //case SDLK_d: world.apos[1].rotate( -0.1, {0.0,0.0,1.0} ); break;

                //case SDLK_w: world.apos[1].mul( 1.1 ); break;
                //case SDLK_s: world.apos[1].mul( 0.9 ); break;

            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch( event.button.button ){
                //case SDL_BUTTON_LEFT:
                //    ipicked = pickParticle( world.natoms, world.apos, ray0, (Vec3d)cam.rot.c , 0.5 );
                //    break;
                //case SDL_BUTTON_RIGHT:
                //    ibpicked = world.pickBond( ray0, (Vec3d)cam.rot.c , 0.5 );
                //    printf("ibpicked %i \n", ibpicked);
                //    break;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            switch( event.button.button ){
                case SDL_BUTTON_LEFT:
                    ipicked = -1;
                    break;
                case SDL_BUTTON_RIGHT:
                    //ibpicked = -1;
                    break;
            }
            break;
    };
    AppSDL2OGL::eventHandling( event );
}

void TestAppSoftMolDyn::drawHUD(){
    glDisable ( GL_LIGHTING );

}

// ===================== MAIN

TestAppSoftMolDyn * thisApp;

int main(int argc, char *argv[]){
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	//SDL_SetRelativeMouseMode( SDL_TRUE );
	int junk;
	thisApp = new TestAppSoftMolDyn( junk , 800, 600 );
	thisApp->loop( 1000000 );
	return 0;
}
















