#ifndef GL_ES
#version 120
#else
#version 100
precision highp float;
precision highp int;
precision lowp sampler2D;
#endif

attribute vec4 vertex;
attribute vec4 colour;
attribute vec4 uv0;

varying	vec4 oUv;
varying	vec4 oColor;
void main()
{
	oUv = uv0;
	oColor = colour;
	gl_Position = vertex;
}