CPP = g++
LD = g++
CPPFLAGS = -O3 -std=c++11 -fpermissive
LDFLAGS = -lGLU
TARGET = water
INCLUDE = -Iinclude/
OBJS = objs/main.o objs/Shader.o objs/Camera.o objs/Sphere.o
OS = $(shell uname)
LIB =  -lGL -lGLEW -lglfw


default: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) $(LIB) -o $(TARGET)

objs/Camera.o: src/Camera.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) src/Camera.cpp -o objs/Camera.o

objs/Shader.o: src/Shader.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) src/Shader.cpp -o objs/Shader.o

objs/main.o: src/main.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) src/main.cpp -o objs/main.o

objs/Sphere.o: src/Sphere.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) src/Sphere.cpp -o objs/Sphere.o

clean:
	rm -f $(OBJS) $(TARGET)
