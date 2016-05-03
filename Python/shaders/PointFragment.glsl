#version 330 core
precision highp float;

in block
{
    vec4 color;
    vec2 texCoord;
} In;
//in vec4 colour;

layout(location=0) out vec4 fragColour;

void main ()
{
//  // Quick fall-off computation
//  float r = length(In.texCoord*2.0-1.0)*3.0;
//  float i = exp(-r*r);
//  //if (i < 0.000001) discard;

//  fragColour = vec4(In.color.rgb, i);
fragColour=vec4(1);
 // fragColour=vec4(r);
}

