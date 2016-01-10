csrc = $(wildcard src/*.c) \
	   $(wildcard libs/imago/*.c) \
	   $(wildcard libs/ogg/*.c) \
	   $(wildcard libs/vorbis/*.c) \
	   $(wildcard libs/vmath/*.c) \
	   $(wildcard libs/libdrawtext/*.c)

cppsrc = $(wildcard src/*.cpp)

ccsrc = $(wildcard src/audio/*.cc) \
		$(wildcard libs/vmath/*.cc) \
		$(wildcard libs/ncf/*.cc)

obj = $(csrc:.c=.o) $(ccsrc:.cc=.o) $(cppsrc:.cpp=.o)

bin = mars2030

#opt = -O3 -ffast-math
dbg = -g
warn = -Wall
incpaths = -Isrc -Isrc/audio -Ilibs -Ilibs/vorbis -Ilibs/vmath -Ilibs/imago -Ilibs/libdrawtext -I/usr/include/freetype2 -I/usr/local/include/freetype2
libpaths = -L/usr/local/lib

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -lGLEW
	libal = -framework OpenAL
	warn += -Wno-deprecated-declarations
        libs = -framework LibOVR
else
	libgl = -lGL -lGLU -lGLEW
	libal = -lopenal
	libs = -lX11 -lOVR
endif

CFLAGS = -pedantic $(warn) $(dbg) $(opt) $(incpaths) `pkg-config --cflags sdl2`
CXXFLAGS = $(CFLAGS)
LDFLAGS = $(libgl) $(libal) $(libs) -lm -lpthread -ldl -lz -lpng -lfreetype -ljpeg `pkg-config --libs sdl2` -lassimp

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
