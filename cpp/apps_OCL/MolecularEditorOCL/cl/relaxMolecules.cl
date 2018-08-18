
// https://www.khronos.org/registry/OpenCL/sdk/1.1/docs/man/xhtml/sampler_t.html



#define R2ELEC 1.0
#define R2SAFE          1e-4f
#define RSAFE           1.0e-4f
#define COULOMB_CONST   14.399644f  // [eV/e]



__constant sampler_t sampler_1 = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_LINEAR;

inline float3 rotMat( float3 v, float3 a, float3 b, float3 c ){ return (float3)(dot(v,a),dot(v,b),dot(v,c)); }
inline float3 rotMatT( float3 v,  float3 a, float3 b, float3 c  ){ return a*v.x + b*v.y + c*v.z; }


inline float3 rotQuat( float4 q, float3 v ){
    // http://www.geeks3d.com/20141201/how-to-rotate-a-vertex-by-a-quaternion-in-glsl/
    //return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
    //https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
    float3 t = 2 * cross( q.xyz, v );
    return v + t * q.w + cross( q.xyz , t );
    // https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
    //return 2.0 * dot(q.xyz, v) * q.xyz + ( q.w*q.w - dot(q.xyz,q.xyz) ) * v + 2.0 * q.w * cross(q.xyz, v);
}



float3 tipForce( float3 dpos, float4 stiffness, float4 dpos0 ){
    float r = sqrt( dot( dpos,dpos) );
    return  (dpos-dpos0.xyz) * stiffness.xyz              // harmonic 3D
         + dpos * ( stiffness.w * (r-dpos0.w)/r );  // radial
}

float4 interpFE( float3 pos, float3 dinvA, float3 dinvB, float3 dinvC, __read_only image3d_t imgIn ){
    const float4 coord = (float4)( dot(pos,dinvA),dot(pos,dinvB),dot(pos,dinvC), 0.0f );
    return read_imagef( imgIn, sampler_1, coord );
}

// this should be macro, to pass values by reference
void move_LeapFrog( float3 f, float3 p, float3 v, float2 RP ){
    v  =  f * RP.x + v*RP.y;
    p +=  v * RP.x;
}

#define FTDEC 0.5f
#define FTINC 1.1f
#define FDAMP 0.99f
#define N_RELAX_STEP_MAX  64
#define F2CONV  1e-8

// this should be macro, to pass values by reference
void move_FIRE( float3 f, float3 p, float3 v, float2 RP, float4 RP0 ){
    // RP0 = (t?,damp0,tmin,tmax)
    float ff   = dot(f,f);
    float vv   = dot(v,v);
    float vf   = dot(v,f);
    if( vf < 0 ){
        v      = 0.0f;
        RP.x   = max( RP.x*FTDEC, RP0.z );    // dt
        RP.y   = RP0.y;                      // damp
    }else{
        v      = v*(1-RP.y) + f*RP.y * sqrt(vv/ff);
        RP.x   = min( RP.x*FTINC, RP0.w );   // dt
        RP.y  *= FDAMP;                     // damp
    }
    // normal leap-frog times step
    v +=  f * RP.x;
    p +=  v * RP.x;
}



/*
Purpose,

    have N systems each composed of M molecules each composed of K atoms
    - assume M,K < local_size (~32)
    They are laying on grid
    we want to relax all the systems in paralel

    later we also want to find which configurations overlaps with previous

*/

__kernel void getForceRigidSystemSurfGrid(
    __read_only image3d_t  imgPauli,
    __read_only image3d_t  imgLondon,
    __read_only image3d_t  imgElec,
    // found - previously found molecular configurations
    __global  int2*    mol2atoms,    // mol2atoms[type.x:type.y]
    //__global  int2*  confs,        // pointer to poses ... since we have constant number of molecules, we dont need this
    __global  float8*  atomsInTypes, // atoms in molecule types
    __global  float8*  poses,        // pos, qrot
    __global  float8*  fposes,       // force acting on pos, qrot
    float4 dinvA,
    float4 dinvB,
    float4 dinvC,
    int nSystems,
    int nMols, // nMols should be approx local size
    float alpha
){

    __local float8 lATOMs[32]; // here we store atoms of molecule j

    //const int iG = get_global_id (0);
    const int nL      = get_local_size(0);
    // we asume there is nMol and nAtoms < local_size
    const int imol    = get_local_id(0);
    const int iL      = get_local_id(0);
    const int isystem = get_group_id(0);

    //if( isystem > nSystems ) return;  // perhaps not needed if global_size set properly
    //if( imol > nMols ) return;

    float4 forceE = (float4)(0.0,0.0,0.0,0.0);
    float4 torq   = (float4)(0.0,0.0,0.0,0.0); // WHAT IS DIFFERENCE BETWEEN dqrot/dforce and torq?

    const int molOffset = isystem * nMols;
    const int oimol     = molOffset+imol;
    const int iatomi    = mol2atoms[oimol].x;
    const int natomi    = mol2atoms[oimol].y;
    float4 mposi        = poses[oimol].lo;
    float4 qroti        = poses[oimol].hi;

    // ==== Molecule - Grid interaction

    // TODO: we may store transformed atoms of imol to local mem ?
    for(int ia=0; ia<natomi; ia++){ // atoms of molecule i
        float4 adposi = atomsInTypes[iatomi+ia].lo;
        adposi.xyz    = rotQuat( qroti, adposi.xyz );
        float3 aposi  = adposi.xyz + mposi.xyz;
        float3 REQi   = atomsInTypes[iatomi+ia].s456;

        // molecule grid interactions
        //float eps    = sqrt(REQi.y); //  THIS SHOULD BE ALREADY DONE
        float expar    = exp(-alpha*REQi.x);
        float cPauli   =    REQi.y*expar*expar;
        float cLondon  = -2*REQi.y*expar;
        float4 fe = (float4)(0.0,0.0,0.0,0.0);
        fe += cPauli   * interpFE( aposi, dinvA.xyz, dinvB.xyz, dinvC.xyz, imgPauli  );
        fe += cLondon  * interpFE( aposi, dinvA.xyz, dinvB.xyz, dinvC.xyz, imgLondon );
        fe += REQi.z   * interpFE( aposi, dinvA.xyz, dinvB.xyz, dinvC.xyz, imgElec   );

        forceE += fe;
        torq   += cross(adposi, fe);
    }

    // ==== Molecule - Molecule Interaction

    for(int jmol=0; jmol<nMols; jmol++){

        // transform atoms of jmol to world coords and store them local memory
        const int ojmol     = molOffset+jmol;
        const int iatomj = mol2atoms[ojmol].x;
        const int natomj = mol2atoms[ojmol].y;
        if(iL<natomj){
            float8 atomj = atomsInTypes[iatomj+iL];
            atomj.xyz    = rotQuat( poses[ojmol].hi, atomj.xyz ) + poses[ojmol].xyz;
            lATOMs[iL]   = atomj;
        }

        barrier(CLK_LOCAL_MEM_FENCE);
        if ( (jmol==imol) || (imol>nMols) ) continue; // prevent self-interaction and reading outside buffer
        for(int ia=0; ia<natomi; ia++){ // atoms of molecule i
            //float3 aposi = lATOMs[ia].xyz;
            //float3 REQi  = lATOMs[ia].s456;
            float4 adposi = atomsInTypes[iatomi+ia].lo;
            adposi.xyz    = rotQuat( qroti, adposi.xyz );
            float3 aposi  = adposi.xyz + mposi.xyz;
            float3 REQi   = atomsInTypes[iatomi+ia].s456;

            // molecule-molecule interaction
            float4 fe = (float4)(0.0,0.0,0.0,0.0);
            for(int ja=0; ja<natomj; ja++){            // atoms of molecule j
                float3 dp   = lATOMs[ja].xyz - aposi;  // already transformed
                float3 REQj = lATOMs[ja].s456;
                // force
                float r0    = REQj.x + REQi.x;
                float eps   = REQj.y * REQi.y; 
                float cElec = REQj.z * REQi.z * COULOMB_CONST;
                float r     = sqrt( dot(dp,dp)+R2SAFE );
                float expar = exp( alpha*(r-r0));
                float ir    = 1/r;
                float fr    = eps*2*alpha*( expar*expar - expar ) + cElec*ir*ir;
                fe.xyz     += dp *( fr*ir );
                fe.w       += eps*( expar*expar - 2*expar ) + cElec*ir; // Energy
            }

            forceE += fe;
            torq   += cross(adposi, fe);

        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    if (imol>nMols) return;

    fposes[oimol].lo = forceE;
    fposes[oimol].hi = torq;

}












