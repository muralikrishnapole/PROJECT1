
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>

#include "Vec2.h"

#include "testUtils.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "Draw.h"
#include "Draw2D.h"
#include "AppSDL2OGL.h"

#include "convolve2D.h"
#include "OCL.h"

#define R2SAFE  1.0e-2
#define F2MAX   10.0

class TestApp_clConvolve2D : public AppSDL2OGL {
	public:
	int per_frame = 32;

	int err;
	OCLsystem cl;
	OCLtask *task1,*task2;

	//bool use_GPU = false;
	int method = 1;

	GLuint texture1 = 0;

	//virtual void loop( int nframes );
	virtual void eventHandling( const SDL_Event& event );
	virtual void draw   ();
	//virtual void drawHUD();

	void switchMethod( int i );

	TestApp_clConvolve2D( int& id, int WIDTH_, int HEIGHT_ );

};

// ==================== Implementation

TestApp_clConvolve2D::TestApp_clConvolve2D( int& id, int WIDTH_, int HEIGHT_ ) : AppSDL2OGL( id, WIDTH_, HEIGHT_ ) {

    genXOR2D(nx, ny, buff);
    texture1 = Draw::makeTexture<Draw::float2RGBA>( nx, ny, buff );

    // --- OpenCL
    cl.init();
    cl.newBuffer( "I",   ntot, sizeof(float), (float*)buff , CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR );
    cl.newBuffer( "O",   ntot, sizeof(float), (float*)buff_, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR );
    err = cl.buildProgram( "cl/convolve2D.cl" );     OCL_checkError(err, "cl.buildProgram");

    task1 = new OCLtask( &cl, cl.newKernel("blur2D_naive"), 2, 0, 0 );
    //task1 = new OCLtask( &cl, cl.newKernel("blur2D_local"), 2, 0, 0 );
    task1->global[0] = nx-2; task1->global[1] = ny-2;
    task1->local [0] = 16;   task1->local [1] = 16;
    task1->args = { BUFFarg(0), BUFFarg(1) };
    task1->print_arg_list();
    //exit(0);

}

void TestApp_clConvolve2D::draw(){
    long t1,t2,t3;
    glClearColor( 0.5f, 0.5f, 0.5f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //delay = 100;
    double f2err;
    t1=getCPUticks();
    switch(method){
        case 1:
            for(int itr=0; itr<per_frame; itr++){
                blur(nx, ny, buff , buff_ );
                blur(nx, ny, buff_, buff  );
            }
            break;
        case 2:
            for(int itr=0; itr<per_frame; itr++){
                task1->args[0].setBuff(0); task1->args[1].setBuff(1); task1->enque();   //exit(0);
                task1->args[0].setBuff(1); task1->args[1].setBuff(0); task1->enque();
            }
            clFinish(cl.commands);
            cl.download(0);
            break;
    }
    t1 = getCPUticks() - t1;

    printf( " %i method %i Time %g [Mticks] \n", frameCount, method,  t1*1e-6 );

    glDeleteTextures( 1, &texture1 );
    Draw::makeTexture<Draw::float2RGBA>( nx, ny, buff );
    if(texture1) Draw2D::renderImage(texture1,{-5.0,-5.0,5.0,5.0});

};

void TestApp_clConvolve2D::switchMethod( int i ){
    method=i;
    genXOR2D(nx, ny, buff);
    cl.upload(0);
}

void TestApp_clConvolve2D::eventHandling( const SDL_Event& event ){
    switch( event.type ){
        case SDL_KEYDOWN :
            switch( event.key.keysym.sym ){
                case SDLK_KP_1:  switchMethod( 1 ); break;
                case SDLK_KP_2:  switchMethod( 2 ); break;
            } break;
    };
    AppSDL2OGL::eventHandling( event );
};


// ===================== MAIN

TestApp_clConvolve2D * app;

int main(int argc, char *argv[]){
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	int junk;
	app = new TestApp_clConvolve2D( junk , 800, 600 );
	app->loop( 1000000 );
	return 0;
}















