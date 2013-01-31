CC = g++
ifeq ($(shell sw_vers 2>/dev/null | grep Mac | awk '{ print $$2}'),Mac)
	CFLAGS = -g -DGL_GLEXT_PROTOTYPES -I./include/ -I./lib/mac -I/usr/X11/include -DOSX
	LDFLAGS = -framework GLUT -framework OpenGL -L./lib/mac/ \
    	-L"/System/Library/Frameworks/OpenGL.framework/Libraries" \
    	-lGL -lGLU -lm -lstdc++ -lGLEW
else
	CFLAGS = -g -DGL_GLEXT_PROTOTYPES -I./include/ -I/usr/X11R6/include -I/sw/include \
					 -I/usr/sww/include -I/usr/sww/pkg/Mesa/include
	LDFLAGS = -L./lib/nix -L/usr/X11R6/lib -L/sw/lib -L/usr/sww/lib \
						-L/usr/sww/bin -L/usr/sww/pkg/Mesa/lib -lglut -lGLU -lGL -lX11 -lGLEW -lfreeimage
endif
INCFLAGS = -I./glm-0.9.4.1
RM = /bin/rm -f 
all: viewer
viewer: main.o shaders.o mesh.o parser.o shaders.h mesh.h
	$(CC) $(CFLAGS) -o viewer shaders.o main.o mesh.o parser.o $(INCFLAGS) $(LDFLAGS) 
main.o: main.cpp shaders.h mesh.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c main.cpp
shaders.o: shaders.cpp shaders.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c shaders.cpp
mesh.o: mesh.cpp mesh.h 
	$(CC) $(CFLAGS) $(INCFLAGS) -c mesh.cpp 
parser.o: parser.cpp mesh.h 
	$(CC) $(CFLAGS) $(INCFLAGS) -c parser.cpp 
clean: 
	$(RM) *.o viewer


 
