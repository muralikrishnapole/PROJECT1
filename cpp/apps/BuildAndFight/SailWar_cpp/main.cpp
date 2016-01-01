
/*
TO DO :
	*	each of a few runs ends up as segmentation fault:
		./test: line 6: 11233 Segmentation fault      (core dumped) ./program.x
		try to use valgrind ?
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// ========= include from common
#include "fastmath.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Mat3.h"
#include "quaternion.h"
#include "raytrace.h"

#include "drawMath.h"
#include "drawMath2D.h"
//#include "drawUtils.h"

#include "Body.h"
#include "Body2D.h"
#include "AeroSurf2D.h"

// ========= include from local app

#include "GameWorld.h"
#include "Projectile.h"
#include "Gun.h"
#include "Yacht2D.h"
#include "Frigate2D.h"

// ===============================
// ===== GLOBAL VARIABLES
// ===============================

bool  isLib             = false;
bool  STOP          	= false;
bool  loopEnd           = false;
int   frameCount		=	0;

SDL_Event		 event;

GameWorld world;

//std::vector<Projectile*> projectiles( 100 );

#include "GameScreen.h"

GameScreen* thisScreen;

// ===============================
// ===== FUNCTIONS
// ===============================

void quit(){
	SDL_Quit();
	exit(1);
};

void escape(){
	if(isLib){
		printf( " ending loop \n");
		loopEnd = true;
	}else{
		printf( " exiting \n");
		quit();
	}
};


void update(){

	//ship1.applySailForces(  { 0.0, 0.0 },  { 0.0, 1.0 }  );
	//ship1.move( dt );

/*
	for( int i=0; i<perFrame; i++ ){
		ship1.clean_temp( );
		ship1.applySailForces(  windSpeed,  watterSpeed );
		ship1.move( dt );
	}
*/

	//STOP = true;
};

void setup(){

}

void inputHanding(){
	while(SDL_PollEvent(&event)){ // be carefull to include all event handling instide the while loop !!!
		if( event.type == SDL_KEYDOWN ){
			switch( event.key.keysym.sym ){
				case SDLK_ESCAPE:   escape(); break;
				case SDLK_SPACE:    STOP = !STOP; printf( STOP ? " STOPED\n" : " UNSTOPED\n"); break;
				case SDLK_KP_MINUS: thisScreen->zoom*=VIEW_ZOOM_STEP; break;
				case SDLK_KP_PLUS:  thisScreen->zoom/=VIEW_ZOOM_STEP; break;
				case SDLK_KP_1:     world.ship1.fire_left ( &world.projectiles ); break;
				case SDLK_KP_2:     world.ship1.fire_right( &world.projectiles ); break;
			}

/*
			if( event.key.keysym.sym == SDLK_ESCAPE   ) { escape(); }
			if( event.key.keysym.sym == SDLK_SPACE    ) { STOP = !STOP; printf( STOP ? " STOPED\n" : " UNSTOPED\n"); }
			if( event.key.keysym.sym == SDLK_KP_MINUS ) { thisScreen->zoom*=VIEW_ZOOM_STEP; }
			if( event.key.keysym.sym == SDLK_KP_PLUS  ) { thisScreen->zoom/=VIEW_ZOOM_STEP; }
*/
		}
		if( event.type == SDL_QUIT){ quit();  };

	} // while(SDL_PollEvent(&event))


	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if( keys[ SDL_SCANCODE_LEFT  ] ){  world.ship1.rudder.setAngle( world.ship1.rudder.phi + 0.01 );  }
	if( keys[ SDL_SCANCODE_RIGHT ] ){  world.ship1.rudder.setAngle( world.ship1.rudder.phi - 0.01 );  }

	if( keys[ SDL_SCANCODE_UP  ]  ){  world.ship1.mast.setAngle( world.ship1.mast.phi + 0.01 );  }
	if( keys[ SDL_SCANCODE_DOWN ] ){  world.ship1.mast.setAngle( world.ship1.mast.phi - 0.01 );  }

	SDL_GetMouseState( &thisScreen->mouseX, &thisScreen->mouseY );
	//printf( "frame %i mouseX moyseY  %i %i   \n", frameCount, mouseX, mouseY );
}


void loop(int n ){
	loopEnd = false;
	for( int iframe=0; iframe<n; iframe++ ){
		inputHanding();
		if(!STOP){
			update();
			thisScreen->update();
		}
		//printf(" %i \n", iframe );
		SDL_Delay( 10 );
		frameCount++;
		if(loopEnd) break;
	}
}

// FUNCTION ======  main
int main(int argc, char *argv[]){

	// creating windows
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	int sid;
	//thisScreen  = new Screen2D( sid, 800,600);
	thisScreen  = new GameScreen( sid, 800,600 );

	setup();

	printf( " setup done \n" );

	//loop( 1 );
	loop( 1000000 );

	return 0;
}


// ==========================================================
// ===== Export this functions to Dynamic library for Python
// ==========================================================

extern "C"{ }

