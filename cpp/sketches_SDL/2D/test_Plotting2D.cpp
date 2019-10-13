
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>

#include "fastmath.h"
#include "Vec2.h"
#include "geom2D.h"


#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "Draw2D.h"
#include "AppSDL2OGL.h"
#include "SDL_utils.h"

#include "Plot2D.h"
#include "PlotScreen2D.h"

#include "testUtils.h"

// ======================  TestApp

/*

Results Sin-Cos approx

cos_sin()     MaxErr: 4.21802e-10 RMSE: 1.06433e-10
cos_sin(2,1)  MaxErr: 1.88677e-05 RMSE: 5.66393e-06
cos_sin(2,2)  MaxErr: 6.16719e-07 RMSE: 1.77133e-07
cos_sin(2,3)  MaxErr: 1.94899e-08 RMSE: 5.53645e-09
cos_sin(4,1)  MaxErr: 5.14595e-08 RMSE: 1.36165e-08
cos_sin(4,2)  MaxErr: 4.21802e-10 RMSE: 1.06433e-10
cos_sin(4,3)  MaxErr: 3.33356e-12 RMSE: 8.31634e-13
cos_sin(6,1)  MaxErr: 8.7447e-11 RMSE: 2.09387e-11
cos_sin(6,2)  MaxErr: 1.80411e-13 RMSE: 4.09557e-14
cos_sin(6,3)  MaxErr: 4.60743e-15 RMSE: 1.68924e-15
junk;         : 3.369 ticks/call ( 3.36924e+07 1e+07 ) | 9.75056e+06
cos();sin()   : 81.565 ticks/call ( 8.15651e+08 1e+07 ) | 45488.8
sincos()      : 80.077 ticks/call ( 8.00774e+08 1e+07 ) | 45488.8
cos_sin( )    : 20.017 ticks/call ( 2.0017e+08 1e+07 ) | 45488.8
cos_sin(2,1)  : 17.144 ticks/call ( 1.71439e+08 1e+07 ) | 45487.3
cos_sin(2,2)  : 20.379 ticks/call ( 2.0379e+08 1e+07 ) | 45488.8
cos_sin(2,3)  : 23.491 ticks/call ( 2.34906e+08 1e+07 ) | 45488.8
cos_sin(4,1)  : 19.112 ticks/call ( 1.9112e+08 1e+07 ) | 45488.8
cos_sin(4,2)  : 22.704 ticks/call ( 2.2704e+08 1e+07 ) | 45488.8
cos_sin(4,3)  : 25.384 ticks/call ( 2.5384e+08 1e+07 ) | 45488.8
cos_sin(6,1)  : 20.930 ticks/call ( 2.09297e+08 1e+07 ) | 45488.8
cos_sin(6,2)  : 23.869 ticks/call ( 2.38689e+08 1e+07 ) | 45488.8
cos_sin(6,3)  : 28.037 ticks/call ( 2.80372e+08 1e+07 ) | 45488.8


*/


class TestAppPlotting : public AppSDL2OGL{
	public:

    Plot2D plot1;
    int fontTex;

	virtual void draw   ();
    //virtual void eventHandling( const SDL_Event& event );

	TestAppPlotting( int& id, int WIDTH_, int HEIGHT_ );
};

TestAppPlotting::TestAppPlotting( int& id, int WIDTH_, int HEIGHT_ ) : AppSDL2OGL( id, WIDTH_, HEIGHT_ ) {

    fontTex = makeTexture( "common_resources/dejvu_sans_mono_RGBA_inv.bmp" );

    Func1d myFunc = &sin;

    plot1.init();
    plot1.fontTex = fontTex;

    /*
    DataLine2D * line1 = new DataLine2D(100);
    line1->linspan(-3*M_PI,2*M_PI);
    line1->yfunc = myFunc;

    DataLine2D * line2 = new DataLine2D(400);
    line2->linspan(-M_PI,3*M_PI);
    for(int i=0; i<line2->n; i++){
        double x     = line2->xs[i];
        line2->ys[i] = sin(x*x);
    }
    line2->clr = 0xFF00FF00;
    */


    int nsamp = 1000;
    DataLine2D * lcos     = new DataLine2D(nsamp,-3*M_PI,5*M_PI); //lcos.pointStyle();
    DataLine2D * lsin     = new DataLine2D(nsamp,-3*M_PI,5*M_PI);
    DataLine2D * lcos_ref = new DataLine2D(nsamp,-3*M_PI,5*M_PI);
    DataLine2D * lsin_ref = new DataLine2D(nsamp,-3*M_PI,5*M_PI);

    DataLine2D * lcos_err = new DataLine2D(nsamp,-3*M_PI,5*M_PI);
    DataLine2D * lsin_err = new DataLine2D(nsamp,-3*M_PI,5*M_PI);

    for(int i=0; i<nsamp; i++){
        lcos_ref->ys[i] = cos(lcos_ref->xs[i]);
        lsin_ref->ys[i] = sin(lsin_ref->xs[i]);
        //cos_sin( lcos_ref->xs[i], lcos->ys[i], lsin->ys[i] );
        cos_sin( lcos_ref->xs[i], lcos->ys[i], lsin->ys[i], 2,1 );
        //cos_sin( lcos_ref->xs[i], lcos->ys[i], lsin->ys[i], 2,2 );
        //cos_sin( lcos_ref->xs[i], lcos->ys[i], lsin->ys[i], 2,3 );
        //cos_sin( lcos_ref->xs[i], lcos->ys[i], lsin->ys[i], 4,1 );
        //cos_sin( lcos_ref->xs[i], lcos->ys[i], lsin->ys[i], 4,2 );
        //cos_sin( lcos_ref->xs[i], lcos->ys[i], lsin->ys[i], 4,3 );
        //cos_sin( lcos_ref->xs[i], lcos->ys[i], lsin->ys[i], 6,1 );
        //cos_sin( lcos_ref->xs[i], lcos->ys[i], lsin->ys[i], 6,2 );
        //cos_sin( lcos_ref->xs[i], lcos->ys[i], lsin->ys[i], 6,3 );
        lcos_err->ys[i] = log10( fabs( lcos->ys[i] - lcos_ref->ys[i] ) );
        lsin_err->ys[i] = log10( fabs( lsin->ys[i] - lsin_ref->ys[i] ) );
        //printf( "[%i]: x %g err %g %g \n", i, lcos_ref->xs[i], lcos_err->ys[i], lsin_err->ys[i] );
    }

    lcos_ref->clr = 0xFFFF0000;
    lsin_ref->clr = 0xFF0000FF;
    lcos->clr     = 0xFFFFFF00;
    lsin->clr     = 0xFF00FFFF;
    lcos_err->clr = 0xFFFF7F00;
    lsin_err->clr = 0xFF007FFF;

    plot1.lines.push_back( lcos_ref );
    plot1.lines.push_back( lsin_ref );
    plot1.lines.push_back( lcos     );
    plot1.lines.push_back( lsin     );
    plot1.lines.push_back( lcos_err );
    plot1.lines.push_back( lsin_err );
    plot1.render();


    const int n = 1000;
    const int m = 10000;
    double xs[n];
    for(int i=0; i<n; i++){ xs[i]=randf(-30.0,30.0); }
    //VecN::arange(n,-30.0,60./n,xs);

    double c=0,s=0;
    double dn=1./n;

    TEST_ERROR_PROC_N( "cos_sin()    ", {double x=xs[i]; cos_sin(x,c,s); c-=cos(x); s-=sin(x); STORE_ERROR(c); STORE_ERROR(s); }, n );

    TEST_ERROR_PROC_N( "cos_sin(2,1) ", {double x=xs[i]; cos_sin(x,c,s,2,1); c-=cos(x); s-=sin(x); STORE_ERROR(c); STORE_ERROR(s); }, n );
    TEST_ERROR_PROC_N( "cos_sin(2,2) ", {double x=xs[i]; cos_sin(x,c,s,2,2); c-=cos(x); s-=sin(x); STORE_ERROR(c); STORE_ERROR(s); }, n );
    TEST_ERROR_PROC_N( "cos_sin(2,3) ", {double x=xs[i]; cos_sin(x,c,s,2,3); c-=cos(x); s-=sin(x); STORE_ERROR(c); STORE_ERROR(s); }, n );

    TEST_ERROR_PROC_N( "cos_sin(4,1) ", {double x=xs[i]; cos_sin(x,c,s,4,1); c-=cos(x); s-=sin(x); STORE_ERROR(c); STORE_ERROR(s); }, n );
    TEST_ERROR_PROC_N( "cos_sin(4,2) ", {double x=xs[i]; cos_sin(x,c,s,4,2); c-=cos(x); s-=sin(x); STORE_ERROR(c); STORE_ERROR(s); }, n );
    TEST_ERROR_PROC_N( "cos_sin(4,3) ", {double x=xs[i]; cos_sin(x,c,s,4,3); c-=cos(x); s-=sin(x); STORE_ERROR(c); STORE_ERROR(s); }, n );

    TEST_ERROR_PROC_N( "cos_sin(6,1) ", {double x=xs[i]; cos_sin(x,c,s,6,1); c-=cos(x); s-=sin(x); STORE_ERROR(c); STORE_ERROR(s); }, n );
    TEST_ERROR_PROC_N( "cos_sin(6,2) ", {double x=xs[i]; cos_sin(x,c,s,6,2); c-=cos(x); s-=sin(x); STORE_ERROR(c); STORE_ERROR(s); }, n );
    TEST_ERROR_PROC_N( "cos_sin(6,3) ", {double x=xs[i]; cos_sin(x,c,s,6,3); c-=cos(x); s-=sin(x); STORE_ERROR(c); STORE_ERROR(s); }, n );

    SPEED_TEST_PROC_NM( "junk;        ", {double x=xs[i];s+=x;c+=x;sum=c+s;}, n, m );

    SPEED_TEST_PROC_NM( "cos();sin()  ", {double x=xs[i];sum+=cos(x)+sin(x);      }, n, m );
    SPEED_TEST_PROC_NM( "sincos()     ", {sincos (xs[i],&c,&s);sum+=c+s; }, n, m );
    SPEED_TEST_PROC_NM( "cos_sin( )   ", {cos_sin(xs[i],c,s);sum+=c+s; }, n, m );

    SPEED_TEST_PROC_NM( "cos_sin(2,1) ", {cos_sin(xs[i],c,s,2,1);sum+=c+s; }, n, m );
    SPEED_TEST_PROC_NM( "cos_sin(2,2) ", {cos_sin(xs[i],c,s,2,2);sum+=c+s; }, n, m );
    SPEED_TEST_PROC_NM( "cos_sin(2,3) ", {cos_sin(xs[i],c,s,2,3);sum+=c+s; }, n, m );

    SPEED_TEST_PROC_NM( "cos_sin(4,1) ", {cos_sin(xs[i],c,s,4,1);sum+=c+s; }, n, m );
    SPEED_TEST_PROC_NM( "cos_sin(4,2) ", {cos_sin(xs[i],c,s,4,2);sum+=c+s; }, n, m );
    SPEED_TEST_PROC_NM( "cos_sin(4,3) ", {cos_sin(xs[i],c,s,4,3);sum+=c+s; }, n, m );

    SPEED_TEST_PROC_NM( "cos_sin(6,1) ", {cos_sin(xs[i],c,s,6,1);sum+=c+s; }, n, m );
    SPEED_TEST_PROC_NM( "cos_sin(6,2) ", {cos_sin(xs[i],c,s,6,2);sum+=c+s; }, n, m );
    SPEED_TEST_PROC_NM( "cos_sin(6,3) ", {cos_sin(xs[i],c,s,6,3);sum+=c+s; }, n, m );

}

void TestAppPlotting::draw(){
    glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST );

	//plot1.drawAxes();
    plot1.view();
};

// ===================== MAIN

TestAppPlotting * testApp;

int main(int argc, char *argv[]){

	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	int junk;
	testApp = new TestAppPlotting( junk , 800, 600 );
	testApp->loop( 1000000 );
	return 0;
}
















