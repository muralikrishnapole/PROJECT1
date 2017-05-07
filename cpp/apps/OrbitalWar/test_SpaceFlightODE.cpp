
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "Vec3.h"
#include "Mat3.h"

#include "Draw2D.h"
#include "Draw3D.h"

//#include "DynamicOpt.h"
#include "spline_hermite.h"
#include "CubicBSpline.h"
#include "ODEintegrator.h"

#include "SDL_utils.h"
#include "AppSDL2OGL_3D.h"
#include "GUI.h"
#include "testUtils.h"

#include "SpaceLaunchODE.h"

/*
// TODO : in case we need object ?
SpaceLaunch * plaunch1 = NULL;
void getODEDerivs( double t, int n, double * Ys, double * dYs ){
    plaunch1->getODEDerivs(t,n,Ys,dYs);
}
*/

void splinePixel( double t, double& x, double& y, double& z ){
    int    i = (int) t;
    double u = t - i;
    /*
    double y1 = thetaCPs[i+1];
    double y2 = thetaCPs[i+2];
    x = t+1.0;
    y = Spline_Hermite::val( u, y1, y2, (y2-thetaCPs[i])*0.5d, (thetaCPs[i+3]-y1)*0.5d ) * 5.0;
    z = 0;
    */
    Vec3d p = getSpline3d( u, dirCPs+i );
    x=p.x; y=p.y; z=p.z;
};

// ==== Class

class TestApp_SpaceFlightODE : public AppSDL2OGL_3D {
	public:
    bool RUNNING = true;
    ODEintegrator_RKF45  odeint;
    //SpaceLaunch          launch1;

    Vec3d  *pos=NULL,*vel=NULL,*acc=NULL;
    double *mass=NULL;
    Vec3d  opos;

	virtual void draw   ();
	virtual void drawHUD();
	//virtual void mouseHandling( );
	//virtual void eventHandling   ( const SDL_Event& event  );
	//virtual void keyStateHandling( const Uint8 *keys );
    //virtual void mouseHandling( );

	TestApp_SpaceFlightODE( int& id, int WIDTH_, int HEIGHT_ );

};

TestApp_SpaceFlightODE::TestApp_SpaceFlightODE( int& id, int WIDTH_, int HEIGHT_ ) : AppSDL2OGL_3D( id, WIDTH_, HEIGHT_ ) {

    odeint.reallocate( 7 );
    odeint.dt_max    = 0.005;
    odeint.dt_min    = 0.0001;
    odeint.dt_adapt  = 0.001;
    odeint.getDerivs = getODEDerivs;
    //plaunch1         = &launch1;

    ((Vec3d*)(odeint.invMaxYerr  ))->set(1/1e-1);
    ((Vec3d*)(odeint.invMaxYerr+3))->set(1/1e-2);
    odeint.invMaxYerr[6] = 1/1e+1;

    pos  = (Vec3d*)(odeint.Y   );
    vel  = (Vec3d*)(odeint.Y+3 );
    acc  = (Vec3d*)(odeint.dY+3);
    mass = odeint.Y+6;

    pos->set(1.0,0.0,0.0); vel->set(0.0,0.0,0.0);
    //pos->set(0.0,100.0e+3,0.0); vel->set(7.9e+3,0.0,0.0); satelite
    *mass = mass_initial;

    for(int i=1; i<nCP; i++){
        double t = (i-1)*0.5;
        //double   theta = M_PI_2/(1.0d + t*t*1.5 );
        double   theta = M_PI_2*exp( -0.5*t*t );
        if(i==0) theta = M_PI_2;
        thetaCPs [i] = theta;
        thrustCPs[i] = thrust_full;
        dirCPs   [i].set( cos(theta), sin(theta), 0.0 );
    }


    glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    zoom = 0.2;
}

void TestApp_SpaceFlightODE::draw(){
    //printf( " ==== frame %i \n", frameCount );
    //glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	/*
	// --- fade out
	glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.5f,0.5f,0.5f, 0.05f);Draw2D::drawRectangle( {-20.0,-20.0},{20.0,20.0}, true);
	//glDisable(GL_BLEND);
	//glClear( GL_DEPTH_BUFFER_BIT );
    */

	// --- draw control spline
    //glColor3f(1.0f,1.0f,1.0f); Draw3D::drawCurve( 0.0, 8.0, 100, splinePixel );
    //for(int i=0; i<nCP; i++){ Draw3D::drawPointCross( {i,thetaCPs[i]*5.0,0.0}, 0.1 ); }
    //for(int i=0; i<nCP; i++){ Draw3D::drawPointCross( dirCPs[i], 0.1 ); }

    //RUNNING = 0;

    if(RUNNING){
        int nstep = 0;
        //odeint.step( 0.1d );
        //odeint.adaptive_step_RKF45( );
        //nstep = odeint.integrate_adaptive( odeint.dt_adapt, odeint.t+0.2d );
        nstep = odeint.integrate_adaptive( odeint.dt_adapt, odeint.t+5.0d );

        //odeint.step_RKF45( 0.1d );
        //odeint.step_euler( 0.1d );

        //printf( "%i %i (%g,%g,%g) %g %g\n ", frameCount, nstep, pos->x,pos->y,pos->z, odeint.t, odeint.dt_adapt );
        printf( " %g : (%g,%g,%g) %g %g \n ", odeint.t, pos->x,pos->y,pos->z, vel->norm(), *mass );
        //glColor3f(0.0f,0.0f,0.0f); Draw3D::drawLine    ( opos      , *pos ); Draw3D::drawPointCross( *pos, 0.1 );
        //glColor3f(0.0f,0.0f,1.0f); Draw3D::drawVecInPos( (*vel)*2.0, *pos );

        if( (*pos-planet_pos).norm2()<(planet_R*planet_R) ){
            printf("crashed !!!\n");
            RUNNING = false;
        }
    }

    glColor3f(0.0f,0.0f,0.0f); Draw3D::drawLine    ( opos*view_scale      , (*pos)*view_scale );
    //Draw3D::drawPointCross( *pos*view_scale, 0.1 );
    //glColor3f(0.0f,0.0f,1.0f); Draw3D::drawVecInPos( (*vel)*2.0*view_scale, (*pos)*view_scale );
    opos = *pos;

    // --- draw planet
    //glColor3f(0.0f,0.0f,0.0f); Draw3D::drawPointCross( {0.0,0.0,0.0}, 0.5 );
    glColor3f(0.0f,0.0f,0.0f); Draw3D::drawPointCross( {0.0,0.0,0.0}, 0.1 );
    glColor3f(0.0f,0.0f,0.0f); Draw3D::drawPointCross( planet_pos*view_scale, 0.5 );
    glColor3f(0.0f,0.0f,0.0f); Draw3D::drawCircleAxis( 64, planet_pos*view_scale, {0.0,planet_R*view_scale,0.0}, {0.0,0.0,1.0} );

	glDisable ( GL_LIGHTING );

};

void TestApp_SpaceFlightODE::drawHUD(){
    glDisable ( GL_LIGHTING );

    float tsc = 1.0;
    float sc  = 100.0;
    for(int i=0; i<nCP; i++){
        double st = uT*(i-1)*tsc;
        glColor3f(0.0f,0.0f,0.0f); Draw3D::drawPointCross( {st,thetaCPs[i]  *sc*2,0.0}, 5.0 );
        glColor3f(1.0f,0.0f,0.0f); Draw3D::drawPointCross( {st,dirCPs  [i].x*sc,0.0}, 5.0 );
        glColor3f(0.0f,0.0f,1.0f); Draw3D::drawPointCross( {st,dirCPs  [i].y*sc,0.0}, 5.0 );
    }

    int icp; double u;
    spline_sampe( odeint.t, u, icp );
    if((icp+3)<nCP){
        double st = odeint.t*tsc;
        double theta = getSpline  ( u, thetaCPs+icp );
        Vec3d  dir   = getSpline3d( u, dirCPs   +icp );
        //double thrust = getSpline  ( u, thrustCPs+icp );
        glColor3f(0.0f,0.0f,0.0f); Draw3D::drawPointCross( {st,theta*sc*2,0.0}, 5.0 );
        glColor3f(1.0f,0.0f,0.0f); Draw3D::drawPointCross( {st,dir.x*sc,0.0}, 5.0 );
        glColor3f(0.0f,0.0f,1.0f); Draw3D::drawPointCross( {st,dir.y*sc,0.0}, 5.0 );
    }



    //exit(0);
}

// ===================== MAIN

TestApp_SpaceFlightODE * thisApp;

int main(int argc, char *argv[]){
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	//SDL_SetRelativeMouseMode( SDL_TRUE );
	int junk;
	thisApp = new TestApp_SpaceFlightODE( junk , 800, 600 );
	thisApp->loop( 1000000 );
	return 0;
}
















