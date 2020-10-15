#version 330 core

in       vec2      fUV;
//uniform  vec2      Const;    // julia constant
uniform sampler2D  iChannel0; 
uniform float      iTime; 
uniform vec2       iResolution;
//uniform vec2 iMouse;

out vec4 gl_FragColor;

//Calculate normal vector.
vec2 N(float angle) {
    return vec2(sin(angle), cos(angle));
}

//Rotate around an arbitrary line by a given angle.
vec2 rotate(vec2 uv, vec2 cp, float a, bool side) {
    vec2 n = N(a * 3.14159);
    float d = dot(uv - cp, n);
    if (side) {
        uv -= n * max(0.0, d) * 2.0;
    } else {
        uv -= n * min(0.0, d) * 2.0;
    }
    return uv;
}

//Used if needing the distance for showing the mirroring line.
float dist(vec2 uv, vec2 cp, float a) {
    vec2 n = N(a * 3.14159);
    return dot(uv - cp, n);
}

//void mainImage( out vec4 fragColor, in vec2 fragCoord){
void main( ){
    // Shader Variables
    int iterations      = 5;     //Number of fractal iterations.
    float thickness     = 2.0;   //Thickness of the lines to be drawn.
    //bool mouseOn      = false; //Should click dragging affect the rotation?
    bool trippyInAndOut = true;  //Should the image trip in and out over time?
    bool trippyTexture  = true;  //Should the image use the texture in iChannel0, changing over time?
    bool showUVFolding  = false; //Use primary colors to show UV folding/mirroring?
    
    if (iterations <  1.0){ iterations = 1;   }
    if (thickness  <= 0.0){ thickness  = 1.0; }
    
    //vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;//Center Origin, Remap to -0.5 to +0.5, and square using aspect ratio.
    
    vec2 uv = 2*(fUV - 0.5)*iResolution.xy/ iResolution.x;
    
    //vec2 mouse = iMouse.xy / iResolution.xy;//Useful for finding exact mirroring angles to use, just place mouse.x in place of angle
    vec2 mouse = vec2(0.5,0.6);
    //if (!mouseOn) {
    //    mouse = vec2(1.0);//If not using mouse, lock to 1.0, 1.0 (because it is multiplied by a value below.)
    //}
    
	uv *= 1.25; //Zoom out.
    uv.y += tan((5.0 / 6.0) * 3.14159) * 0.5;// Re-Center
    
    vec3 col = vec3(0);//Set all black.
    
    uv.x = abs(uv.x);//Mirror on Y axis.
    
    uv = rotate(uv, vec2(0.5, 0.0), 5.0 / 6.0, true);//Rotate UV around a line passing through (0.5, 0.0) by (5.0 / 6.0) angle.

    //col += smoothstep(0.01, 0.0, abs(dist(uv, vec2(0.5, 0.0), mouse.x)));//Show the mirroring line.
   
    float scale = 1.0;//Set initial scale.
    uv.x += 0.5;            //Shift right by 1 /2 unit.
    for (int i = 0; i < iterations; i++) {//Loop through the number of iterations for the fractal.
        uv    *= 3.0;       // Scale UV space by a factor of 3
        scale *= 3.0;       // Keep track of total scale change.
        uv.x  -= 1.5;       // Shift left by 1.5 Units
        uv.x   = abs(uv.x); // Mirror on Y axis
        uv.x  -= 0.5;       // Shift left by 1/2 Unit
        uv     = rotate(uv, vec2(0.0, 0.0), mouse.y * 2.0 / 3.0, false);//Fold to create mirrored rotated segments. (The ^ part.)
    }
    
    if (trippyInAndOut) {
        //For trippy effect.
        uv *= cos(iTime * 0.5);
        uv = rotate(uv, vec2(0.0, 0.0), cos(iTime), false);
    }
    
    if (trippyTexture) {
        //Very trippy effect!
        uv /= scale;
        //col += texture(iChannel0, uv * 2.0 - iTime * 0.1).rgb;
        col += vec3(cos(uv*50.0),cos(uv.x/uv.y));
    } else {
        //Calculate the color based on the distance from the line. Until now, just shifting, scaling, mirroring UV space.
        //Remember uv space has been mirrored repeatedly to create the fractal outline. 
        //So we are only drwaing one line, but it is crumpled up.
        float d = length(uv - vec2(clamp(uv.x, -1.0, 1.0), 0));
        col += smoothstep(thickness / iResolution.y, 0.0, d / scale);//Smooth out and thicken the lines. and adjust based on scale.
    }
    
    if (showUVFolding) {
        col.rg += uv;   //Demonstrate UV space folding/mirroring.
    }
    
    gl_FragColor = vec4(col, 1.0);  //Output the color.
}


