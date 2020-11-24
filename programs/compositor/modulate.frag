//MIT License
//
//Copyright (c) 2020 Andrey Vasiliev
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#ifndef GL_ES
#define VERSION 120
#version VERSION
#else
#define VERSION 100
#version VERSION
#endif
#include "header.frag"

#include "srgb.glsl"

in vec2 oUv0;
uniform sampler2D SceneSampler;
#ifdef BLOOM
uniform sampler2D BloomSampler;
#endif
#ifdef SSAO
uniform sampler2D SsaoSampler;
#endif
#ifdef MANUAL_SRGB
uniform float exposure;
#endif

void main()
{
    vec3 scene = texture2D(SceneSampler, oUv0).rgb;
#ifdef SSAO
    scene.rgb *= texture2D(SsaoSampler, oUv0).r;
#endif
#ifdef BLOOM
    scene.rgb += texture2D(BloomSampler, oUv0).rgb;
#endif
#ifdef MANUAL_SRGB
    const float gamma = 2.2;
    vec3 mapped = vec3(1.0) - exp(-scene.rgb * exposure);
    scene.rgb = pow(mapped, vec3(1.0 / gamma));
#endif

    gl_FragColor = vec4(scene.rgb, 1.0);
}