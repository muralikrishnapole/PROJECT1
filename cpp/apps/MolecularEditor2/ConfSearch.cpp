
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "Draw.h"
#include "Draw3D.h"
#include "SDL_utils.h"
#include "Solids.h"

#include "fastmath.h"
#include "Vec3.h"
#include "Mat3.h"
#include "quaternion.h"

#include "raytrace.h"
#include "Molecule.h"
#include "MMFF.h"
#include "MMFFBuilder.h"

#include "IO_utils.h"

//#include "RBMMFF.h"
#include "DynamicOpt.h"

#include "AtomicConfiguration.h"
#include "DistanceHierarchy.h"

#include "AppSDL2OGL_3D.h"
#include "testUtils.h"


/*

TO DO:
 - save geom to .xyz
 - save geom to .pdb or .mol
 - add charges (read from .pdb)
 - torsion angles
 - add better model of substrate
 - include rigid body molecules (from MoleculerWorld )
 - Brute force non-bonded interactions by OpenCL
 - add some editation capabilities

 TODO Corrections:
 - vdW distances seems to be too close
 - some bonds too long
 - correct angular forcefield to repdesent kinked groups ( e.g. -OH )
*/

std::vector<Vec3d> iso_points;
int isoOgl;

Vec3d PPpos0 = (Vec3d){1.3,1.7, 1.5};

Vec3d testREQ,testPLQ;

// ==========================
// AppMolecularEditor2
// ==========================


void colorRB( float f ){ glColor3f( 0.5+f, 0.5, 0.5-f ); }



void printPoses( int n, double * poses ){
    for( int i=0; i<n; i++ ){
        int i8 = i*8;
        //printf( "force[%04i] %g,%g,%g,%g|%g,%g,%g,%g\n",i, opt.force[i8+0], opt.force[i8+1], opt.force[i8+2], opt.force[i8+3],    opt.force[i8+4], opt.force[i8+5], opt.force[i8+6], opt.force[i8+7]  );
        printf( "[%04i] %g,%g,%g,%g | %g,%g,%g,%g \n",i, poses[i8+0], poses[i8+1], poses[i8+2], poses[i8+3],    poses[i8+4], poses[i8+5], poses[i8+6], poses[i8+7]  );
    }
}

void drawPPRelaxTrj( int n, double dt, double damp, GridFF& gff, Vec3d pos, Vec3d PRQ ){
    Vec3d vel = (Vec3d){0.0,0.0,0.0};
    glBegin(GL_LINE_STRIP);
    for(int i=0; i<n; i++){
        Vec3d f = (Vec3d){0.0,0.0,0.0};
        gff.addForce( pos, PRQ, f);
        vel.mul(damp);
        vel.add_mul( f  , dt);
        pos.add_mul( vel, dt );
        glVertex3f(pos.x,pos.y,pos.z);
        //printf( " %i (%g,%g,%g) (%g,%g,%g) \n", i, pos.x,pos.y,pos.z,  f.x,f.y,f.z );
    }
    glEnd();
    //exit(0);
}

void drawGridForceAlongLine( int n, GridFF& gff, Vec3d pos0, Vec3d dpos, Vec3d PRQ, double fsc ){
    Vec3d pos = pos0;
	for( int i=0; i<n; i++ ){
        Vec3d f = (Vec3d){0.0,0.0,0.0};
        gff.addForce( pos, PRQ, f);
        //printf( " %i (%g,%g,%g) (%g,%g,%g) \n", i, pos.x,pos.y,pos.z,  f.x,f.y,f.z );
        Draw3D::drawVecInPos( f*fsc, pos );
        Draw3D::drawPointCross( pos, 0.1 );
        pos.add(dpos);
	}
}

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

void renderSubstrate( int n, Vec3d * points, GLenum mode ){
    //printf( "iso_points.size() %i \n", iso_points.size() );
    if( mode == GL_POINTS ){
        glBegin(GL_POINTS);
        for(int i=0; i<iso_points.size(); i++){ glVertex3f( iso_points[i].x, iso_points[i].y, iso_points[i].z      ); }
        glEnd();
    }
}

void renderSubstrate_( const GridShape& grid, Vec3d * FF, Vec3d * FFel, double isoval, bool sign ){
    //printf( "iso_points.size() %i \n", iso_points.size() );
    Vec3d * pos     = new Vec3d[grid.n.x * grid.n.y];
    Vec3d * normals = new Vec3d[grid.n.x * grid.n.y];
    //printf( " -- DEBUG 1 \n" );
    getIsoSurfZ( grid, isoval, sign, FF, pos, normals );

    //printf( " -- DEBUG 2 \n" );

    //glEnable(GL_LIGHTING);
    for ( int ib=1; ib<grid.n.y; ib++ ){
        glBegin(GL_TRIANGLE_STRIP);
        for ( int ia=0; ia<grid.n.x; ia++ ){
            int ip1 = (ib-1)*grid.n.x + ia;
            int ip2 = (ib  )*grid.n.x + ia;
            //printf( "iba (%i,%i) pos (%g,%g,%g)\n", ib,ia, pos[ip1].x,pos[ip1].y,pos[ip1].z );
            //glColor3f(pos[ip1].z*5-2,1.0f,1.0f); glNormal3f(normals[ip1].x,normals[ip1].y,normals[ip1].z); glVertex3f(pos[ip1].x,pos[ip1].y,pos[ip1].z);
            //glColor3f(pos[ip2].z*5-2,1.0f,1.0f); glNormal3f(normals[ip2].x,normals[ip2].y,normals[ip2].z); glVertex3f(pos[ip2].x,pos[ip2].y,pos[ip2].z);


            Vec3d gpos,fel1,fel2;
            grid.cartesian2grid( pos[ip1], gpos); fel1 = interpolate3DvecWrap( FFel, grid.n, gpos );
            grid.cartesian2grid( pos[ip2], gpos); fel2 = interpolate3DvecWrap( FFel, grid.n, gpos );


            //glColor3f(0.7f,0.7f,0.7f); glNormal3f(normals[ip1].x,normals[ip1].y,normals[ip1].z); glVertex3f(pos[ip1].x,pos[ip1].y,pos[ip1].z);
            //glColor3f(0.8f,0.7f,0.7f); glNormal3f(normals[ip2].x,normals[ip2].y,normals[ip2].z); glVertex3f(pos[ip2].x,pos[ip2].y,pos[ip2].z);

            //glColor3f( fel1.x, fel1.y, fel1.z ); glNormal3f(normals[ip1].x,normals[ip1].y,normals[ip1].z); glVertex3f(pos[ip1].x,pos[ip1].y,pos[ip1].z);
            //glColor3f( fel2.x, fel2.y, fel2.z ); glNormal3f(normals[ip2].x,normals[ip2].y,normals[ip2].z); glVertex3f(pos[ip2].x,pos[ip2].y,pos[ip2].z);

            colorRB( fel1.z ); glNormal3f(normals[ip1].x,normals[ip1].y,normals[ip1].z); glVertex3f(pos[ip1].x,pos[ip1].y,pos[ip1].z);
            colorRB( fel2.z ); glNormal3f(normals[ip2].x,normals[ip2].y,normals[ip2].z); glVertex3f(pos[ip2].x,pos[ip2].y,pos[ip2].z);
        }
        glEnd();
    }
    //printf( " -- DEBUG 3 \n" );
    delete [] pos;
    delete [] normals;
    //exit(0);
}

void viewSubstrate( int nx, int ny, int isoOgl, Vec3d a, Vec3d b ){
    glPushMatrix();
    for( int ix = -nx; ix<=nx; ix++ ){
        for( int iy = -ny; iy<=ny; iy++ ){
            Vec3d pos = a*ix + b*iy;
            glTranslatef(pos.x, pos.y, pos.z);
            glCallList(isoOgl);
            glTranslatef(-pos.x, -pos.y, -pos.z);
        }
    }
    glPopMatrix();
}

class AppMolecularEditor2 : public AppSDL2OGL_3D {
	public:
	Molecule    mol;
	MMFFparams  params;
    MMFF        world;
    MMFFBuilder builder;

    DistanceHierarchy<AtomicConfiguration> database;

    DynamicOpt  opt;

    int     fontTex;
    int     ogl_sph;

    char str[256];

    AtomicManipulator manipulator;

    Vec3d ray0;
    int ipicked  = -1, ibpicked = -1;
    int perFrame =  50;

    double drndv =  10.0;
    double drndp =  0.5;

    double  atomSize = 0.25;

    // ==== Functions

    void genNewManipul(int i);
    bool manipulation();

	virtual void draw   ()  override;
	virtual void drawHUD()  override;
	//virtual void mouseHandling( )  = override;
	virtual void eventHandling   ( const SDL_Event& event  ) override;
	virtual void keyStateHandling( const Uint8 *keys ) override;

	AppMolecularEditor2( int& id, int WIDTH_, int HEIGHT_ );

    void initRigidSubstrate();
};

void AppMolecularEditor2::initRigidSubstrate(){

    // ---- Rigid Substrate
    //world.substrate.init( (Vec3i){100,100,100}, (Mat3d){ 10.0,0.0f,0.0f,  0.0,10.0f,0.0f,  0.0,0.0f,10.0f }, (Vec3d){-5.0,-5.0,-5.0} );

    printf( "params.atypNames:\n" );
    for(auto kv : params.atypNames) { printf(" %s %i \n", kv.first.c_str(), kv.second ); }
    //exit(0);
    //world.substrate.grid.n    = (Vec3i){120,120,200};
    world.gridFF.grid.n    = (Vec3i){60,60,100};
    //world.substrate.grid.n    = (Vec3i){12,12,20};
    world.gridFF.grid.pos0 = (Vec3d){0.0d,0.0d,0.0d};
    world.gridFF.loadCell ( "inputs/cel.lvs" );
    //world.gridFF.loadCell ( "inputs/cel_2.lvs" );
    world.gridFF.grid.printCell();
    //world.gridFF.loadXYZ  ( "inputs/answer_Na_L1.xyz", params );
    //world.gridFF.loadXYZ  ( "inputs/NaCl_sym.xyz", params );
    world.gridFF.loadXYZ  ( "inputs/NaCl_wo4.xyz", params );
    //world.gridFF.loadXYZ  ( "inputs/NaCl_sym_Na_add.xyz", params );
    //world.gridFF.loadXYZ  ( "inputs/NaCl_sym_Cl_vac.xyz", params );
    //world.gridFF.loadXYZ  ( "inputs/NaCl_sym_Na_vac.xyz", params );
    //world.gridFF.loadXYZ  ( "inputs/Xe_instead_Na.xyz", params );
    //world.gridFF.loadXYZ( "inputs/Cl.xyz", params );
    world.translate( {0.0,0.0,4.5} );

    //testREQ = (Vec3d){ 2.181, 0.0243442, 0.0}; // Xe
    testREQ = (Vec3d){ 1.487, 0.0006808, 0.0}; // H
    testPLQ = REQ2PLQ( testREQ, -1.6 );//

    world.genPLQ();
    world.gridFF.allocateFFs();
    //world.gridFF.evalGridFFs( {0,0,0} );

    bool recalcFF = false;
    //bool recalcFF = true;
    if( recalcFF ){
        world.gridFF.evalGridFFs( {1,1,1} );
        if(world.gridFF.FFelec )  saveBin( "data/FFelec.bin",   world.gridFF.grid.getNtot()*sizeof(Vec3d), (char*)world.gridFF.FFelec );
        if(world.gridFF.FFPauli)  saveBin( "data/FFPauli.bin",  world.gridFF.grid.getNtot()*sizeof(Vec3d), (char*)world.gridFF.FFPauli );
        if(world.gridFF.FFLondon) saveBin( "data/FFLondon.bin", world.gridFF.grid.getNtot()*sizeof(Vec3d), (char*)world.gridFF.FFLondon );
    }else{
        if(world.gridFF.FFelec )  loadBin( "data/FFelec.bin",   world.gridFF.grid.getNtot()*sizeof(Vec3d), (char*)world.gridFF.FFelec );
        if(world.gridFF.FFPauli)  loadBin( "data/FFPauli.bin",  world.gridFF.grid.getNtot()*sizeof(Vec3d), (char*)world.gridFF.FFPauli );
        if(world.gridFF.FFLondon) loadBin( "data/FFLondon.bin", world.gridFF.grid.getNtot()*sizeof(Vec3d), (char*)world.gridFF.FFLondon );
    }


    //world.gridFF.evalGridFFs(int natoms, Vec3d * apos, Vec3d * REQs );

    int iatom = 11;
    printf( "testREQ   (%g,%g,%g) -> PLQ (%g,%g,%g) \n",        testREQ.x, testREQ.y, testREQ.z, testPLQ.x, testPLQ.y, testPLQ.z   );
    printf( "aREQs[%i] (%g,%g,%g) -> PLQ (%g,%g,%g) \n", iatom, world.aREQ[iatom].x, world.aREQ[iatom].y, world.aREQ[iatom].z, world.aPLQ[iatom].x, world.aPLQ[iatom].y, world.aPLQ[iatom].z );

   // exit(0);

    Vec3d * FFtot = new Vec3d[world.gridFF.grid.getNtot()];

    //world.gridFF.evalCombindGridFF_CheckInterp( (Vec3d){ 2.181, 0.0243442, 0.0}, FFtot );
    //saveXSF( "FFtot_z_CheckInterp.xsf", world.gridFF.grid, FFtot, 2, world.gridFF.natoms, world.gridFF.apos, world.gridFF.atypes );

    world.gridFF.evalCombindGridFF( testREQ, FFtot );
    //saveXSF( "FFtot_z.xsf", world.gridFF.grid, FFtot, 2, world.gridFF.natoms, world.gridFF.apos, world.gridFF.atypes );

    isoOgl = glGenLists(1);
    glNewList(isoOgl, GL_COMPILE);
        //getIsovalPoints_a( world.gridFF.grid, 0.1, FFtot, iso_points );
        //renderSubstrate( iso_points.size(), &iso_points[0], GL_POINTS );
        renderSubstrate_( world.gridFF.grid, FFtot, world.gridFF.FFelec, 0.01, true );
        Draw3D::drawAxis(1.0);
    glEndList();

    camPos.z = +5.0;

}

AppMolecularEditor2::AppMolecularEditor2( int& id, int WIDTH_, int HEIGHT_ ) : AppSDL2OGL_3D( id, WIDTH_, HEIGHT_ ) {

    fontTex = makeTexture( "common_resources/dejvu_sans_mono_RGBA_inv.bmp" );

    ogl_sph = glGenLists(1);
    glNewList( ogl_sph, GL_COMPILE );
        Draw3D::drawSphere_oct( 3, 1.0, {0.0,0.0,0.0} );
        //Draw3D::drawSphere_oct( 3, 0.25, {0.0,0.0,0.0} );
    glEndList();

    //qCamera.set( 0.0,0.0,0.0,1.0 );  // bottom view
    //qCamera.set( 0.0,0.0,1.0,0.0 );  // bottom view
    //qCamera.set( 0.0,1.0,0.0,0.0 );  // top view  x=-x, y=y,
    qCamera.set( 1.0,0.0,0.0,0.0 );    // top view  x=x, y=-y,
    //qCamera.set( 0.70710678118,0.0,0.0,0.70710678118 ); // side down
    //qCamera.set( -0.70710678118,0.0,0.0,0.70710678118 ); // x=x, z=y,  y=-y,
    //qCamera.set( 0.0, -0.70710678118,0.0,0.70710678118 ); // z=-x, y=y
    //qCamera.set( 0.0, +0.70710678118,0.0,0.70710678118 ); // z=+x, y=y
    //qCamera.set( 0.0,0.0, +0.70710678118, 0.70710678118 ); // y=-x, x=y
    //qCamera.set( 0.0,0.0, -0.70710678118, 0.70710678118 ); // y=x, x=-y

    //AtomType atyp;
    //atyp.fromString( "CA 6 4 4 1 2.00 0.09 0x11EEAA" );
    params.loadAtomTypes( "common_resources/AtomTypes.dat" );
    params.loadBondTypes( "common_resources/BondTypes.dat" );
    //for(auto kv : params.atypNames) { printf( ">>%s<< %i \n", kv.first.c_str(), kv.second ); };
    char str[1024];
    printf( "type %s \n", params.atypes[ params.atypNames.find( "C" )->second ].toString( str ) );
    printf( "type %s \n", params.atypes[ params.atypNames.find( "H" )->second ].toString( str ) );
    printf( "type %s \n", params.atypes[ params.atypNames.find( "O" )->second ].toString( str ) );
    printf( "type %s \n", params.atypes[ params.atypNames.find( "N" )->second ].toString( str ) );
    /*
    auto it = params.atypNames.find( "C" );
    if( it != params.atypNames.end() ){
        //printf( "type CA %i \n", it->second );
        printf( "type %i %s \n", it->second, params.atypes[ it->second ].toString( str ) );
    }else{
        printf("not found\n");
    }
    */

    mol.atypNames = &params.atypNames;

    //exit(0);

    // ---- Rigid Body Molecules
    //mol.loadXYZ( "inputs/water_ax.xyz" );                           printf( "DEBUG 3.1 \n" );
    //mol.loadXYZ( "inputs/water_ax_q0.xyz" );                        printf( "DEBUG 3.1 \n" );
    //mol.loadXYZ( "inputs/OH_ax.xyz" );                              printf( "DEBUG 3.1 \n" );
    mol.loadXYZ( "inputs/water_T5_ax.xyz" );    mol.printAtomInfo();                      printf( "DEBUG 3.1 \n" );
    params.assignREs( mol.natoms, mol.atomType, mol.REQs );
    Mat3d rot; rot.setOne();
    builder.insertMolecule( &mol, {0.0,0.0,4.0}, rot, true );          printf( "DEBUG 3.2 \n" );
    builder.insertMolecule( &mol, {4.0,0.0,4.0}, rot, true );
    //builder.insertMolecule( &mol, {0.0,4.0,4.0}, rot, true );
    //builder.insertMolecule( &mol, {4.0,4.0,4.0}, rot, true );

    mol.loadXYZ( "inputs/NaIon.xyz" ); mol.printAtomInfo();
    params.assignREs( mol.natoms, mol.atomType, mol.REQs );
    builder.insertMolecule( &mol, {4.0,6.0,5.0}, rot, false );
    builder.insertMolecule( &mol, {4.0,4.0,2.0}, rot, false );
    builder.insertMolecule( &mol, {4.0,8.0,2.0}, rot, false );

    mol.loadXYZ( "inputs/ClIon.xyz" ); mol.printAtomInfo();
    params.assignREs( mol.natoms, mol.atomType, mol.REQs );
    builder.insertMolecule( &mol, {2.0,6.0,2.0}, rot, false );
    builder.insertMolecule( &mol, {6.0,6.0,2.0}, rot, false );

    world.printAtomInfo();
    builder.toMMFF( &world, &params );                                 printf( "DEBUG 3.3 \n" );
    world.printAtomInfo(); //exit(0);
    //world.allocFragment( nFrag );
    //opt.bindArrays( 8*world.nFrag, (double*)world.poses, new double[8*world.nFrag], (double*)world.poseFs ); printf( "DEBUG 3.4 \n" );

    //opt.bindArrays( 8*world.nFrag, world.poses, world.poseVs, world.poseFs );
    world.allocateDyn(); printf( "DEBUG 3.4.1  \n" );
    world.initDyn();     printf( "DEBUG 3.4.2 \n" );
    opt.bindArrays( world.nDyn, world.dynPos, world.dynVel, world.dynForce ); printf( "DEBUG 3.4.3 \n" );
    opt.setInvMass( 1.0 );  printf( "DEBUG 3.5 \n" );
    opt.cleanVel  ( );      printf( "DEBUG 3.6 \n" );
    //exit(0);
    printf("POSE_pos   : \n"); printPoses( world.nFrag, world.poses  );
    printf("POSE_Force : \n"); printPoses( world.nFrag, world.poseFs );
    //exit(0);

    //world.printAtomInfo();
    //exit(0);

    /*
    world.apos      = mol.pos;
    world.bond2atom = mol.bond2atom;
    world.ang2bond  = mol.ang2bond;
    world.allocate( mol.natoms, mol.nbonds, mol.nang, 0 );
    world.ang_b2a();
    //params.fillBondParams( world.nbonds, world.bond2atom, mol.bondType, mol.atomType, world.bond_0, world.bond_k );
    */

    printf( "DEBUG 4 \n" );

    //printf( "bond 8 %g \n", world.bond_0[8] );
    //printf( "bond 9 %g \n", world.bond_0[9] );
    //Vec2i iat = bond2atom[8];
    //Vec2i iat = bond2atom[9];
    //exit(0);
    initRigidSubstrate();

    manipulator.bindAtoms(world.natoms, world.apos, world.aforce ); printf( "DEBUG 4.1 \n" );
    manipulator.realloc(1);                                         printf( "DEBUG 4.2 \n" );
    manipulator.goalSpan.set(5.0,5.0,1.0);                          printf( "DEBUG 4.3 \n" );
    manipulator.genGoals();                                         printf( "DEBUG 4.4 \n" );

    manipulator.nenabled = 10;
    manipulator.enabled = new int[manipulator.nenabled];
    std::memcpy( manipulator.enabled, (const int[]){0,1,2,3,4,5,6,7,8,9}, manipulator.nenabled*sizeof(int) );

    printf( "DEBUG 5 \n" );
    //exit(0);

}

void AppMolecularEditor2::draw(){
    glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//glTranslatef( 0.0, 0.0, -5.0 );

	glColor3f( 0.0f,0.0f,0.0f );
	//if(isoOgl)

	Draw3D::drawAxis(10);

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	viewSubstrate( 2, 2, isoOgl, world.gridFF.grid.cell.a, world.gridFF.grid.cell.b );

	//perFrame = 10;
	//delay = 100;

	//drawGridForceAlongLine( 100, world.gridFF, {1.0,1.1,2.5}, {0.1,0.1,0.0}, testPLQ, 50.0 );
	//drawGridForceAlongLine( 100, world.gridFF, {3.3,0.0,2.0}, {0.0,0.1,0.0}, testPLQ, 50.0 );
	//exit(0);

	//glColor3f( 1.0f,0.0f,0.0f );
	//drawPPRelaxTrj( 5000, 0.3, 0.95, world.gridFF, PPpos0, testPLQ );

    //return;

	//ibpicked = world.pickBond( ray0, camMat.c , 0.5 );

    ray0 = camPos + camMat.a*mouse_begin_x + camMat.b*mouse_begin_y;
    Draw3D::drawPointCross( ray0, 0.1 );
    //Draw3D::drawVecInPos( camMat.c, ray0 );
    if(ipicked>=0) Draw3D::drawLine( world.apos[ipicked], ray0);

	double F2;
	perFrame = 1;
	//delay = 100;
	for(int itr=0; itr<perFrame; itr++){

        world.cleanAtomForce();

        Vec3d d=(Vec3d){1.0,1.0,1.0};
        Vec3d shift = world.Collision_box.genRandomSample();
        //Vec3d shift = (Vec3d){ randf(-d.x,d.x),randf(-d.y,d.y),randf(-d.z,d.z) };
        //Vec3d shift = (Vec3d){ randf(0,d.x),randf(0,d.y),randf(0,d.z) };
        Quat4d qrot;  qrot.fromUniformS3( {randf(),randf(),randf()} );
        //Mat3d rot; qrot.toMatrix(rot);
        //rot.setOne();
        //world.tryPose( 0, 5, world.apos[0], world.apos[0]+shift, rot );
        world.tryFragPose( 0, false, shift, qrot );

        world.frags2atoms();       //printf( "DEBUG 5.2\n" );

        world.eval_FFgrid();
        //world.eval_MorseQ_On2(); //printf( "DEBUG 5.3\n" );
        world.eval_MorseQ_On2_fragAware();
        //world.eval_MorseQ_Frags(); //printf( "DEBUG 5.3\n" );

        //exit(0);
        if(ipicked>=0){
            Vec3d f = getForceSpringRay( world.apos[ipicked], camMat.c, ray0, -1.0 );
            //printf( "f (%g,%g,%g)\n", f.x, f.y, f.z );
            world.aforce[ipicked].add( f );
        };


        /*
        for(int i=0; i<world.natoms; i++){
            world.aforce[i].add( getForceHamakerPlane( world.apos[i], {0.0,0.0,1.0}, -3.0, 0.3, 2.0 ) );
            //printf( "%g %g %g\n",  world.aforce[i].x, world.aforce[i].y, world.aforce[i].z );
        }
        */

        //manipulator.forceToGoals();

        //world.tryPose( 0, 5, world.apos[0], shift, rot );
        //SDL_Delay(10);

        world.cleanPoseTemps();
        world.aforce2frags();      //printf( "DEBUG 5.4\n" );

        //exit(0);

        //for(int i=0; i<world.natoms; i++){ world.aforce[i].add({0.0,-0.01,0.0}); }
        //int ipivot = 0;
        //world.aforce[ipivot].set(0.0);
        //opt.move_LeapFrog(0.01);
        //opt.move_MDquench();

        for(int i=0; i<world.natoms; i++ ){
            Draw3D::drawVecInPos( world.aforce[i]*10.0, world.apos[i] );
        };

        world.toDym(true);
        //F2 = opt.move_FIRE();  //printf( "DEBUG 5.5\n" );
        world.checkPoseUnitary();
        world.fromDym();

        //printf("POSE_pos   : \n"); printPoses( world.nFrag, world.poses  );
        //printf("POSE_Force : \n"); printPoses( world.nFrag, world.poseFs );
        //exit(0);

    }

    glColor3f(0.0f,0.6f,0.0f); Draw3D::drawBBox ( world.Collision_box.a, world.Collision_box.b );
    //printf( "Box (%f,%f,%f)  (%f,%f,%f) \n", world.Try_box.a.x, world.Try_box.a.y, world.Try_box.a.z,    world.Try_box.b.x, world.Try_box.b.y, world.Try_box.b.z  );

    glColor3f(0.6f,0.6f,0.6f); plotSurfPlane( (Vec3d){0.0,0.0,1.0}, -3.0, {3.0,3.0}, {20,20} );
    //Draw3D::drawVecInPos( (Vec3d){0.0,0.0,1.0},  (Vec3d){0.0,0.0,0.0} );

    //printf( "==== frameCount %i  |F| %g \n", frameCount, sqrt(F2) );

    for(int i=0; i<world.nbonds; i++){
        Vec2i ib = world.bond2atom[i];
        glColor3f(0.0f,0.0f,0.0f);
        if(i==ibpicked) glColor3f(1.0f,0.0f,0.0f); ;
        Draw3D::drawLine(world.apos[ib.x],world.apos[ib.y]);
        sprintf(str,"%i\0",i);
        Draw3D::drawText(str, (world.apos[ib.x]+world.apos[ib.y])*0.5, fontTex, 0.02, 0);
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    for(int i=0; i<world.natoms; i++){
        //glColor3f(0.0f,0.0f,0.0f); Draw3D::drawPointCross(world.apos[i],0.2);
        //glColor3f(1.0f,0.0f,0.0f); Draw3D::drawVecInPos(world.aforce[i]*1000.0,world.apos[i]);

        //glCallList( ogl_sph );
        glEnable(GL_LIGHTING);
        Mat3d mat;
        mat.setOne();
        mat.mul( atomSize*params.atypes[world.atypes[i]].RvdW );
        //glColor3f(0.8f,0.8f,0.8f);
        Draw::setRGB( params.atypes[world.atypes[i]].color );
        Draw3D::drawShape(world.apos[i],mat,ogl_sph);
        glDisable(GL_LIGHTING);
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

   //exit(0);

   //printf( "camPos %g %g %g \n", camPos.x, camPos.y, camPos.z );

};

void  AppMolecularEditor2::keyStateHandling( const Uint8 *keys ){
    double dstep=0.1;
    if( keys[ SDL_SCANCODE_W ] ){ PPpos0.y +=dstep; }
    if( keys[ SDL_SCANCODE_S ] ){ PPpos0.y -=dstep; }
    if( keys[ SDL_SCANCODE_A ] ){ PPpos0.x +=dstep; }
    if( keys[ SDL_SCANCODE_D ] ){ PPpos0.x -=dstep; }
    if( keys[ SDL_SCANCODE_Q ] ){ PPpos0.z +=dstep; }
    if( keys[ SDL_SCANCODE_E ] ){ PPpos0.z -=dstep; }

    if( keys[ SDL_SCANCODE_X ] ){ camPos.z +=0.1; }
    if( keys[ SDL_SCANCODE_Z ] ){ camPos.z -=0.1; }

    AppSDL2OGL_3D::keyStateHandling( keys );
};

void AppMolecularEditor2::eventHandling ( const SDL_Event& event  ){
    //printf( "NonInert_seats::eventHandling() \n" );
    switch( event.type ){
        case SDL_KEYDOWN :
            switch( event.key.keysym.sym ){
                //case SDLK_p:  first_person = !first_person; break;
                //case SDLK_o:  perspective  = !perspective; break;
                //case SDLK_r:  world.fireProjectile( warrior1 ); break;

                case SDLK_v: for(int i=0; i<world.natoms; i++){ ((Vec3d*)opt.vel)[i].add(randf(-drndv,drndv),randf(-drndv,drndv),randf(-drndv,drndv)); } break;
                case SDLK_p: for(int i=0; i<world.natoms; i++){ world.apos[i].add(randf(-drndp,drndp),randf(-drndp,drndp),randf(-drndp,drndp)); } break;

                case SDLK_LEFTBRACKET:  if(ibpicked>=0) world.bond_0[ibpicked] += 0.1; break;
                case SDLK_RIGHTBRACKET: if(ibpicked>=0) world.bond_0[ibpicked] -= 0.1; break;

                //case SDLK_a: world.apos[1].rotate(  0.1, {0.0,0.0,1.0} ); break;
                //case SDLK_d: world.apos[1].rotate( -0.1, {0.0,0.0,1.0} ); break;
                //case SDLK_w: world.apos[1].mul( 1.1 ); break;
                //case SDLK_s: printf("saving ... "); save2xyz( "out.xyz", &world, &params ); printf("... DONE "); break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch( event.button.button ){
                case SDL_BUTTON_LEFT:
                    ipicked = pickParticle( world.natoms, world.apos, ray0, camMat.c , 0.5 );
                    printf("ipicked %i \n", ipicked);
                    break;
                case SDL_BUTTON_RIGHT:
                    ibpicked = world.pickBond( ray0, camMat.c , 0.5 );
                    printf("ibpicked %i \n", ibpicked);
                    break;
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

void AppMolecularEditor2::drawHUD(){
    glDisable ( GL_LIGHTING );

}

// ===================== MAIN

AppMolecularEditor2 * thisApp;

int main(int argc, char *argv[]){
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	//SDL_SetRelativeMouseMode( SDL_TRUE );
	int junk;
	thisApp = new AppMolecularEditor2( junk , 800, 600 );
	thisApp->loop( 1000000 );
	return 0;
}
















