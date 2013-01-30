# version 120 


// Mine is an old machine.  For version 130 or higher, do 
// out vec4 color ;  
// out vec4 mynormal ; 
// out vec4 myvertex ;
// That is certainly more modern
varying vec3 ec_pos;
varying vec3 mynormal ; 
varying vec4 myvertex ; 

void main() {
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex ; 
	ec_pos = (gl_ModelViewMatrix*gl_Vertex).xyz;
    mynormal = gl_Normal ; 
    myvertex = gl_Vertex ; 
}

