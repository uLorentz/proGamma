#for multithreaded compiling
OS := $(shell uname) 
ifeq ($(OS), Linux) 
	export MAKEFLAGS="-j $(grep -c ^processor /proc/cpuinfo)" \
else($(OS), Darwin)
	export MAKEFLAGS="-j $(sysctl-n hw.ncpu)" \
else
	#Windows? 
endif

CXXFLAGS := $(CXXFLAGS) -std=c++11 -pthread  -O3
#it allows to put object files in a subfolder
obj =$(patsubst %.o,$(OBJPATH)%.o, progamma.o dataget.o application.o rooting.o manage_flags.o)

VPATH=./source/
OBJPATH=./obj/

INCS=`root-config --cflags`
LIBS=`root-config --libs`

all: $(obj)
	$(CXX) $(OBJPATH)*.o -o proGamma $(CXXFLAGS) ${INCS} ${LIBS}
ifeq ($(wildcard configure_files*),) #search for "configure_files" folder and create it if it doesn't exist
	@mkdir configure_files	
endif

$(OBJPATH)%.o: %.cpp
ifeq ($(wildcard $(OBJPATH)*),) #search for obj path and create it if it doesn't exist 
	@mkdir -p $(OBJPATH)
endif
	$(CXX) -c $< -o $@ $(CXXFLAGS) ${INCS}

.PHONY: clean
clean:
	rm $(OBJPATH)*.o proGamma
