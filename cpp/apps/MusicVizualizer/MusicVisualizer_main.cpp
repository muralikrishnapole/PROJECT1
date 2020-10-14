


/*


===============
FEATURS TO DO
===============

1) Reaction-Diffusion
    Belousov–Zhabotinsky_reaction  - https://www.shadertoy.com/view/XtcGD2
    Turing Pattern - https://en.wikipedia.org/wiki/Turing_pattern
    Multi-Pass Multi-Frame-Buffer
2) Fluid Dynamics multi-pass
     Chimera's Breath - https://www.shadertoy.com/view/4tGfDW
    Multi-Substance - https://www.shadertoy.com/view/WtffRM
3) Kaleidoscope
4) Body-Of rotation render for spectrum
5) L-system branching acording to music (spectrum, complex)
6) Diffusion random walks  with same spectrum a the music - https://matousstieber.wordpress.com/
4) Particle flow harmonies (sinus flow)-  https://www.shadertoy.com/view/4sGSDw



* Fractal Coloring Techniques - Coloring/Texturing Nature/Flower/Fiebers/Frost JuliaSet
    https://www.shadertoy.com/view/XsfXzs
    http://jussiharkonen.com/gallery/coloring-techniques/
    http://jussiharkonen.com/files/on_fractal_coloring_techniques%28lo-res%29.pdf
    https://commons.wikimedia.org/wiki/File:Stripe_Average_Coloring_-_Mandelbrot_set_zoom_(_wake_1over3_).png
* fractal shake - Dancing Membrane - https://www.shadertoy.com/view/llXBRH
* easy adaptive sampling : Day107 -  dynamic Ornamental fractal pattern  - https://www.shadertoy.com/view/tt2BRK
*/


//#define SPEED_TEST


#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
//#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>
#include <GL/glew.h>

#include "testUtils.h"

#include "fastmath.h"
#include "Vec3.h"
#include "Mat3.h"
#include "quaternion.h"
#include "VecN.h"

//#include "Body.h"
//#include "DynamicControl.h"
//#include "FieldPatch.h"
#include "Solids.h"

#include "Mesh.h"
#include "Solids.h"
#include "GLfunctions.h"
#include "GLobjects.h"
#include "GLObject.h"
#include "Shader.h"
#include "DrawOGL3.h"
#include "SceneOGL3.h"
#include "ScreenSDL2OGL3.h"
#include "AppSDL2OGL3.h"


#define _DEBUG_VIEW_ 1
#include "DebugView.h"  //do we need it ?
DEBUG_VIEW_DEFINE()

#include "SDL_utils.h"
#include "IO_utils.h"


#include "MusicUtils.h"
#include "MusicRendererOGL3.h"



using namespace Music;


// ====================================
//      MusicVisualizerGUI
// ====================================

class MusicVisualizerGUI : public AppSDL2OGL3, public SceneOGL3 { public:

    //DynamicControl rollControl;

    Spectrum waveform;

    float dtJul = 0.2;
    Vec2f CJul = Vec2fZero;

    float camDist = 100.0;
	int perFrame = 10;
	//double dt = 0.001;

	//int fontTex_DEBUG;

    RenderStack layers;
    //Camera cam;


    Shader *shDebug=0,*shJulia=0,*shReactDiff=0,*shTex=0;
    Shader *shFluid1=0,*shFluid2=0;
    GLMesh *histMesh=0, *waveMesh=0, *glTxDebug=0;


	// ==== function declarations

	//void reallocateTrj(int n);

	//void renderSkyBox( float x0, float y0, float z0 );

	//virtual void camera     ();
	//virtual void cameraHUD();

	//virtual void drawHUD();

	//virtual void update();
    virtual void eventHandling   ( const SDL_Event& event  );

    virtual void draw( Camera& cam );

	MusicVisualizerGUI(int W, int H);

    void draw_Spectrum     ( Camera& cam );
    void draw_Julia        ( Camera& cam );
    void draw_ReactDiffuse ( Camera& cam );
    void draw_Fluid        ( Camera& cam );
};

MusicVisualizerGUI::MusicVisualizerGUI(int W, int H):AppSDL2OGL3(W,H),SceneOGL3(){

 for( ScreenSDL2OGL3* screen: screens ) screen->scenes.push_back( this );

    DEBUG_VIEW_INIT()

    // https://learnopengl.com/Advanced-Lighting/HDR
    // https://stackoverflow.com/questions/11211698/framebuffer-object-with-float-texture-clamps-values
    //glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);   GL_DEBUG;
    //glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE); GL_DEBUG;
    //glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE); GL_DEBUG;


    //frameBuff1.init( WIDTH, HEIGHT );
    //texTest = makeTestTextureRGBA( 256, 256);

    const char *keys[2]{"SOLVER","RENDER"};
    char** srcs         = fileGetSections( "common_resources/shaders/Fluid.glslf", 2, keys, "//#BEGIN_SHADER:" );
    char* srx_texture3D = filetobuf( "common_resources/shaders/texture3D.glslv" );
    printf("\n//==========%s \n\n %s \n\n", "SOLVER", srcs[0] );
    printf("\n//==========%s \n\n %s \n\n", "RENDER", srcs[1] );
    shFluid1 = new Shader( srx_texture3D , srcs[0], false );
    shFluid2 = new Shader( srx_texture3D , srcs[1], false );

    shTex   = new Shader( "common_resources/shaders/texture3D.glslv" , "common_resources/shaders/texture.glslf" , true );
    shDebug = new Shader( "common_resources/shaders/const3D.glslv"   , "common_resources/shaders/const3D.glslf" , true );
    //sh1     = new Shader( "common_resources/shaders/shade3D.glslv"   , "common_resources/shaders/shade3D.glslf" , true );
    //shJulia = new Shader( "common_resources/shaders/texture3D.glslv" , "common_resources/shaders/Julia.glslf"   , true );
    shJulia = new Shader( "common_resources/shaders/texture3D.glslv" , "common_resources/shaders/Julia_curvature.glslf"   , true );

    shReactDiff = new Shader( "common_resources/shaders/texture3D.glslv" , "common_resources/shaders/BelousovZhabotinsky.glslf"   , true );
    GL_DEBUG;

    /*
    GLMeshBuilder mshDebug;
    mshDebug.addLine      ( (Vec3f){0.0,0.0,0.0}, {10.0,10.0,10.0}, {1.0,0.0,0.0} );
    mshDebug.addPointCross( {0.0,0.0,0.0}, 1.0, {0.0,0.0,1.0} );
    glDebug = mshDebug.makeLineMesh();

    GLMeshBuilder mshbuild;
    Parabola2Mesh ( {20,10}, {-1.5,-0.5*M_PI}, {1.0,0},      2.0, 4.0, 0.0, false, mshbuild );
    Hyperbola2Mesh( {20,20}, {-1.5,0.0}, {1.0,M_PI},    1.5, 2.0, 4.0, 0.0, false, mshbuild );
    //mshbuild.moveSub( 0, {1.0,2.0,3.0} );
    //mshbuild.rotateSub( 0, {0.0,0.0,0.0}, {1.0,0.0,0.0}, M_PI*0.5 );
    //mshbuild.scaleSub( 0, {1.0,0.5,0.25} );
    glmesh      = mshbuild.makeGLmesh();
    msh_normals = mshbuild.normals2GLmesh(0.1);
    */

    Camera& cam = screens[0]->cam;
    cam.aspect  = screens[0]->HEIGHT/(float)screens[0]->WIDTH;
    cam.zmax    = 1000.0;
    // Prespective
    //cam.zmin = 1.0;
    //cam.zoom = 5.00f;
    // Orthographic
    cam.persp =  false;
    //cam.zoom  =  0.9036f; // Q??? : Not Sure Why there is this factor 0.9036f instead of 1.0 ?????
    cam.zoom  =  0.90310f;
    cam.zmin  = -1000.0;
    //zoomStep = 0.001;


    makeBilboard( glTxDebug );


    layers.screenQuad = glTxDebug;
    layers.makeBuffers( 4, screen[0].WIDTH, screen[0].HEIGHT );
    layers.shaders.push_back( shTex   );
    layers.shaders.push_back( shJulia );
    layers.shaders.push_back( shReactDiff );

    shFluid1->use(); GL_DEBUG;
    shFluid1->setUniformf    ("dt",   0.15);
    shFluid1->setUniformVec2f("iResolution", (Vec2f){layers.buffers[0]->W,layers.buffers[0]->H});
    shFluid1->setUniformf    ("vorticity", 0.09 );

    shFluid2->use(); GL_DEBUG;
    shFluid2->setUniformi( "iChannel0", 0 ); GL_DEBUG;
    shFluid2->setUniformi( "iChannel1", 1 ); GL_DEBUG;
    shFluid2->setUniformf    ("dt",   0.15);
    shFluid2->setUniformVec2f("iResolution", (Vec2f){layers.buffers[0]->W,layers.buffers[0]->H});

    shJulia->use(); GL_DEBUG;
    shJulia->setUniformVec2f("iResolution", (Vec2f){layers.buffers[0]->W,layers.buffers[0]->H});
    //return 0;


    int result = 0;
    int flags = MIX_INIT_MP3;

    const char *music_file_name = "common_resources/02-Lazenca-SaveUs.mp3";
    //const char *music_file_name = "Yanni - Reflections Of Passion.mp3";

    Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Failed to init SDL\n");
        exit(1);
    }
    if (flags != (result = Mix_Init(flags ))) {
        printf("Could not initialize mixer (result: %d).\n", result);
        printf("Mix_Init: %s\n", Mix_GetError());
        exit(1);
    }
    Mix_Music *music = Mix_LoadMUS( music_file_name );


    waveform.realloc( 1024, 16 );
    waveform.mixer_info();
    waveform.clearHist();

    Vec3f ps[3*waveform.nwave];
    //for(int i=0; i<waveform.nwave; i++){ ps[i].set( i*0.1, waveform.wave[i]*0.0001, 0 ); }
    for(int i=0; i<waveform.nwave; i++){ ps[i].set( 0.0, 0, 0 ); }
    waveMesh = polyLineMesh( waveform.nwave, (float*)ps );
    histMesh = polyLineMesh( waveform.nhist, (float*)ps );

    Mix_SetPostMix( postmix_Spectrum, (void*)&waveform );
    Mix_PlayMusic(music, 1);



    glActiveTexture(GL_TEXTURE0  );
    glActiveTexture(GL_TEXTURE0+1);
    glActiveTexture(GL_TEXTURE0+2);
    glActiveTexture(GL_TEXTURE0+3);
    glActiveTexture(GL_TEXTURE0+4);
    glActiveTexture(GL_TEXTURE0+5);
    glActiveTexture(GL_TEXTURE0+6);
    glActiveTexture(GL_TEXTURE0+7);

    //camStep = 2.0;
};


void MusicVisualizerGUI::draw_Spectrum( Camera& cam ){
    shDebug->use();
    cam.lookAt( (Vec3f){0.0,0.0,0.0}, 20.0 );
    setCamera( *shDebug, cam );
    shDebug->setModelPoseT( (Vec3d){-0.8f,0.0,0.0}, Mat3dIdentity );
    Vec2d* wave = (Vec2d*)(waveform.wave);
    float dx = 0.0015;
    float dy = 0.0000003;
    plotBuffStereo ( *waveMesh, *shDebug, waveform.nwave, waveform.Fwave, dx, dy );
    shDebug->setUniformVec4f("baseColor", (Quat4f){0.0,1.0,0.0,1.0});
    plotBuff( *histMesh, waveform.nhist, waveform.hist, 0.5*dx*waveform.nwave/waveform.nhist, dy );
}

void MusicVisualizerGUI::draw_Julia( Camera& cam ){
    shJulia->use();
    float scJulC = 0.00000002;
    CJul.mul(1-dtJul);
    CJul.add(
        dtJul*waveform.hist[0               ]*scJulC,
        dtJul*waveform.hist[waveform.nhist/2]*scJulC
    );
    //float Cx=0.285,Cy=0.01;
    //float Cx=-0.4,Cy=0.6;
    float Cx=-0.8,Cy=0.256;
    //float Cx=-0.8,Cy=0.156;
    Vec2f C = { CJul.x+Cx, CJul.y+Cy  };

    //CJul.set(Cx,Cy);
    printf("C %g %g | %g %g \n", CJul.y, CJul.y, Cx,Cy );
    shJulia->setUniformVec2f( "Const", C );
    setCamera(*shJulia, cam);
    shJulia->setModelPoseT( (Vec3d){-0.5/cam.aspect,-0.5,0.0}, {1./cam.aspect,0.,0.,  0.,1.,0.,  0.,1.,0.} );
    glTxDebug->draw();
}

void MusicVisualizerGUI::draw_ReactDiffuse( Camera& cam ){

    shReactDiff->use();
    setCamera(*shReactDiff, cam);
    shReactDiff->setModelPoseT( (Vec3d){-0.5/cam.aspect,-0.5,0.0}, {1./cam.aspect,0.,0.,  0.,1.,0.,  0.,1.,0.} );

    // Render 0 -> 1
    layers.bindOutput( 1    );
    layers.bindInput ( 0, 0 );
    if(frameCount==1)layers.fillRandomRGB(0);
    glTxDebug->draw();

    // Render 1 -> 0
    layers.bindOutput( 0    );
    layers.bindInput ( 1, 0 );
    if(frameCount==1)layers.fillRandomRGB(1);
    glTxDebug->draw();

}


void MusicVisualizerGUI::draw_Fluid( Camera& cam ){

    // Try to use this extension allowing to use same texture as both input and output
    // https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_shader_framebuffer_fetch.txt
    // https://stackoverflow.com/questions/64304026/rendering-to-custom-framebuffer-using-same-texture-both-as-input-and-output?noredirect=1#comment113708604_64304026

    int perFrame = 5;

    int iout,iin,iin2;
    for(int i=0; i<perFrame; i++){
        int iter = i + frameCount*perFrame;

        // === 1] --- Shader 1
        shFluid1->use();
        setCamera(*shFluid1, cam);
        shFluid1->setModelPoseT( (Vec3d){-0.5/cam.aspect,-0.5,0.0}, {1./cam.aspect,0.,0.,  0.,1.,0.,  0.,1.,0.} );

        shFluid1->setUniformf ("time", iter*0.001);

        if( iter&1 ){ iout=0; iin=1; }else{ iout=1; iin=0; }; // split odd and even frames
        layers.bindOutput( iout   );
        layers.bindInput ( iin, 0 );
        glTxDebug->draw();

        // === 2] --- Shader 2
        shFluid2->use();
        setCamera(*shFluid2, cam);
        shFluid2->setModelPoseT( (Vec3d){-0.5/cam.aspect,-0.5,0.0}, {1./cam.aspect,0.,0.,  0.,1.,0.,  0.,1.,0.} );

        iin=iout;
        if( iter&1 ){ iout=2; iin2=3; }else{ iout=3; iin2=2; };
        layers.bindOutput( iout    );
        layers.bindInput ( iin,  0 );
        layers.bindInput ( iin2, 1 );
        glTxDebug->draw();
    }

    // === 3] --- just plot velocity buffer
    shTex->use();
    setCamera(*shTex, cam );
    shTex->setModelPoseT( (Vec3d){-0.5/cam.aspect,-0.5,0.0}, {1./cam.aspect,0.,0.,  0.,1.,0.,  0.,1.,0.} );

    layers.unbindOutput();
    layers.bindInput(iout,0);
    glTxDebug->draw();

}

void MusicVisualizerGUI::draw( Camera& cam ){

    //waveform.spectrumHistSmearing();
    //waveform.update( 5.1 );
    waveform.need_refresh = false;
    waveform.update( 5.0 );
    //printf("\e[1;1H\e[2J"); // clear screen //https://stackoverflow.com/questions/2347770/how-do-you-clear-the-console-screen-in-c
    //waveform.printSpectrum();

    //cam = cam_;

    //int narg;
    glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    //glEnable(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    //draw_ReactDiffuse(cam);
    //draw_Fluid(cam);

    layers.unbindOutput();

    /*
    // Render Layers
    shTex->use();
    setCamera(*shTex, cam );
    //shTex->setModelPoseT( (Vec3d){-4.,-4.,0.0}, Mat3dIdentity*8.0 );
    shTex->setModelPoseT( (Vec3d){-0.5/cam.aspect,-0.5,0.0}, {1./cam.aspect,0.,0.,  0.,1.,0.,  0.,1.,0.} );
    //printf( "cam.zoom %g \n", cam.zoom );
    //layers.render( 0, -1, 1, (const int[]){0} ); // renders using shader[0] to default_screen using 1 input buffer #0
    layers.bindInput(0,0);
    glTxDebug->draw();
    */

    shDebug->use();
    setCamera(*shDebug, cam);
    shDebug->setUniformVec4f("baseColor", {0.0,1.0,0.0,1.0});
    //shDebug->setModelPoseT( (Vec3d){-0.5,-0.5,0.0}, Mat3dIdentity*1.0 );
    shDebug->setModelPoseT( (Vec3d){-0.5/cam.aspect,-0.5,0.0}, {1./cam.aspect,0.,0.,  0.,1.,0.,  0.,1.,0.} );
    glTxDebug->draw(GL_LINE_LOOP);

    draw_Julia(cam);
    //draw_Spectrum(cam);

};


/*
void MusicVisualizerGUI::update(){
    AppSDL2OGL3::update();

    //mouseButtons = SDL_GetMouseState(&mx,&my);
    bool RMB = mouseButtons&SDL_BUTTON(SDL_BUTTON_RIGHT);

    if      ( keys[ SDL_SCANCODE_A ] ){   }
	else if ( keys[ SDL_SCANCODE_D ] ){   }

    if      ( keys[ SDL_SCANCODE_W ] ){  }
	else if ( keys[ SDL_SCANCODE_S ] ){   }

    if      ( keys[ SDL_SCANCODE_E ] ){ }
	else if ( keys[ SDL_SCANCODE_Q ] ){}

    //if( keys[SDL_SCANCODE_W]||keys[SDL_SCANCODE_S]||keys[SDL_SCANCODE_A]||keys[SDL_SCANCODE_D]||keys[SDL_SCANCODE_E]||keys[SDL_SCANCODE_Q] ){
    if( keys[SDL_SCANCODE_W]||keys[SDL_SCANCODE_S]||keys[SDL_SCANCODE_E]||keys[SDL_SCANCODE_Q] ){
        controler.bActive = false;
        Mat3d rot;
        //rot.setT(myCraft->rotMat);
        rot.set(myCraft->rotMat);
        controler.goalDir = rot.c;
        controler.goalUp  = rot.b;
    }else{
        controler.bActive = true;
    };


    Mat3d rot; rot.setT(myCraft->rotMat);
	//world->update_world(); // ALL PHYSICS COMPUTATION DONE HERE
}
*/


void MusicVisualizerGUI::eventHandling( const SDL_Event& event  ){
    switch( event.type ){
        case SDL_KEYDOWN :
        switch( event.key.keysym.sym ){
            case SDLK_ESCAPE   : SDL_Quit(); exit(1); break;
            case SDLK_SPACE    : STOP = !STOP; printf( STOP ? " STOPED\n" : " UNSTOPED\n"); break;
        }; break;
        case SDL_QUIT: SDL_Quit(); exit(1); break;

        case SDL_MOUSEBUTTONDOWN:
            switch( event.button.button ){
                case SDL_BUTTON_RIGHT:
                   // mouseSteer = true;
                    break;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            switch( event.button.button ){
                case SDL_BUTTON_RIGHT:
                  //  mouseSteer = false;
                 //   pilot->resetSteer();
                    break;
            }
            break;
    }

};

MusicVisualizerGUI * app;

int main(int argc, char *argv[]){
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	//SDL_SetRelativeMouseMode( SDL_TRUE );
	SDL_ShowCursor(SDL_DISABLE);
    SDL_DisplayMode dm;
    SDL_GetDesktopDisplayMode(0, &dm);
    app = new MusicVisualizerGUI( dm.w-150, dm.h-100 );
    app->loop( 1000000 );
    app->quit();
    return 0;
}

