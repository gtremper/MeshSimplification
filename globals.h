/* Globals needed by the parser and drawer */

#ifndef GLOBALS_H
#define GLOBALS_H



extern int width, height;
extern float fovy; 
extern int numLights;
extern const int MAXLIGHTS;
extern vec4 light_position[];
extern vec4 light_specular[];
extern GLuint shaderprogram;



#endif