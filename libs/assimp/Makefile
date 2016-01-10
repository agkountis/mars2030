src = $(wildcard *.cpp) \
	  $(wildcard assimp/*.cpp) \
	  $(wildcard irrXML/*.cpp)
csrc = $(wildcard ConvertUTF/*.c)

obj = $(src:.cpp=.o) $(csrc:.c=.o)
lib_a = ../assimp.a

CFLAGS = -g -I.
CXXFLAGS = $(CFLAGS)

$(lib_a): $(obj)
	$(AR) rcs $@ $(obj)

.PHONY: clean
clean:
	rm -f $(obj) $(lib_a)
