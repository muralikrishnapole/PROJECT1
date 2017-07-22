
#ifndef  GLInstances_h
#define  GLInstances_h

#include <GL/glew.h>
//#include <SDL2/SDL.h>

//float * double2float( int n, double * ds ){ float*fs=float[n]; for(int i=0; i<n; i++){ fs[i]=(float)ds[i]; }; return fs; }
void double2float( int n, double * ds, float * fs ){ for(int i=0; i<n; i++){ fs[i]=(float)ds[i]; }; }

class GLMesh{ public:
    union{
        //struct{ GLuint vpos=0,vnor=0,vcol=0,vUVs=0; }; // union does not work with initializer
        struct{ GLuint vpos,vnor,vcol,vUVs; };
        GLuint buffs[4];
    };
    GLuint inds=0;
    int nVerts =0;
    int nInds  =0;
    GLenum draw_mode=GL_TRIANGLES;

    void init( int nVerts_, int nInds_, void * c_inds, void * c_vpos,  void * c_vnor, void * c_vcol, void * c_vUVs, GLenum usage=GL_STATIC_DRAW ){
        nVerts = nVerts_;
        nInds  = nInds_;
        if(nInds  ){ newElementBuffer( inds, nInds   *sizeof(GLuint),  c_inds, usage ); }else{ inds=0; };
        if(c_vpos ){ newArrayBuffer  ( vpos, nVerts*3*sizeof(GLfloat), c_vpos, usage ); }else{ vpos=0; };
        if(c_vnor ){ newArrayBuffer  ( vnor, nVerts*3*sizeof(GLfloat), c_vnor, usage ); }else{ vnor=0; };
        if(c_vcol ){ newArrayBuffer  ( vcol, nVerts*3*sizeof(GLfloat), c_vcol, usage ); }else{ vcol=0; };
        if(c_vUVs ){ newArrayBuffer  ( vUVs, nVerts*2*sizeof(GLfloat), c_vUVs, usage ); }else{ vUVs=0; };
    };

    void init_d( int nVerts_, int nInds_, int * c_inds, double * c_vpos,  double * c_vnor, double * c_vcol, double * c_vUVs, GLenum usage=GL_STATIC_DRAW ){
        nVerts = nVerts_;
        nInds  = nInds_;
        float * fs = new float[4*nVerts];
        if(nInds  ){ newElementBuffer( inds, nInds   *sizeof(GLuint),  c_inds, usage ); }else{ inds=0; };
        if(c_vpos ){ double2float( nVerts*3, c_vpos,fs); newArrayBuffer( vpos, nVerts*3*sizeof(GLfloat), fs, usage ); }else{ vpos=0; };
        if(c_vnor ){ double2float( nVerts*3, c_vnor,fs); newArrayBuffer( vnor, nVerts*3*sizeof(GLfloat), fs, usage ); }else{ vnor=0; };
        if(c_vcol ){ double2float( nVerts*3, c_vcol,fs); newArrayBuffer( vcol, nVerts*3*sizeof(GLfloat), fs, usage ); }else{ vcol=0; };
        if(c_vUVs ){ double2float( nVerts*2, c_vUVs,fs); newArrayBuffer( vUVs, nVerts*2*sizeof(GLfloat), fs, usage ); }else{ vUVs=0; };
        delete[] fs;
    };

    void deleteBuffs(){ for(int i=0; i<4; i++){ if( buffs[i] ){ glDeleteBuffers(1, &buffs[i] ); } }; }

    int preDraw (){
        int iarg = 0;
        if(vpos){ bindVertexAttribPointer( iarg, vpos, 3, GL_FLOAT, GL_FALSE );  iarg++; }
        if(vnor){ bindVertexAttribPointer( iarg, vnor, 3, GL_FLOAT, GL_FALSE );  iarg++; }
        if(vcol){ bindVertexAttribPointer( iarg, vcol, 3, GL_FLOAT, GL_FALSE );  iarg++; }
        if(vUVs){ bindVertexAttribPointer( iarg, vUVs, 2, GL_FLOAT, GL_FALSE );  iarg++; }
        return iarg;
    };
    void drawRaw( GLenum draw_mode ){
        if(nInds){ drawElements( draw_mode, inds, nInds ); }else{ glDrawArrays( draw_mode, 0, nVerts); };
    }
    void postDraw(int narg){ for( int i=0; i<narg; i++ ){ glDisableVertexAttribArray(i); } };
    void draw( GLenum draw_mode ){ int narg = preDraw(); drawRaw( draw_mode ); postDraw(narg); };
    void drawRaw(){ drawRaw( draw_mode ); };
    void draw   (){ draw   ( draw_mode ); };
    void drawPointsRaw( float sz ){ glPointSize(sz); glDrawArrays( GL_POINTS, 0, nVerts); } // mostly for debugging - ignores indexes
    void drawPoints   ( float sz ){ int narg = preDraw(); drawPointsRaw(sz); postDraw(narg);}
};

#endif
