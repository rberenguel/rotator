#version 300 es

precision highp float;

uniform vec4 mixColor;

out vec4 fragColor;

in vec2 vs_texCoord;

void main(void)
{
    float edgeWidth = 0.04;
    float blur = 0.02;
    vec2 edgeLow = smoothstep(vec2(edgeWidth), vec2(edgeWidth + blur), vs_texCoord);
    vec2 edgeHigh = smoothstep(vec2(1.0) - vec2(edgeWidth), vec2(1.0) - vec2(edgeWidth + blur), vs_texCoord);
    float edge = 1.2*edgeLow.x * edgeLow.y * edgeHigh.x * edgeHigh.y;
    fragColor = mixColor.a*vec4(mix(vec3(edge), mixColor.rgb, 1.0), 1.0);
    if(edge < 0.5){
        //fragColor = vec4(0.0);
        fragColor = vec4(203.0/255.0, 75.0/255.0, 22.0/255.0, 0.5);
    }
}
