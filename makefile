OS := $(shell uname)
ifeq ($(OS), Linux)
	export MAKEFLAGS="-j $(grep -c ^processor /proc/cpuinfo)" \
else($(OS), Darwin)
	export MAKEFLAGS="-j $(sysctl-n hw.ncpu)" \
else
	#Windows? 
endif

CXXFLAGS := $(CXXFLAGS) -std=c++11 -pthread   #-O3

obj=progamma.o application.o manage_flags.o
VPATH=./source/
OBJPATH=./obj/

INCS=`root-config --cflags`
LIBS=`root-config --libs`


all: $(obj)
	$(CXX) $(OBJPATH)*.o -o proGamma $(CXXFLAGS) ${INCS} ${LIBS}
ifeq ($(wildcard configure_files*),) #cerco per la cartella "configure_files", se non esiste la creo
	@mkdir configure_files	
endif

#%.o: %.cpp functions.h
#	$(CXX) -c $< -o $(OBJPATH)$@ $(CXXFLAGS) ${INCS}

%.o: %.cpp	
	$(CXX) -c $< -o $(OBJPATH)$@ $(CXXFLAGS) ${INCS}

.PHONY: clean
clean:
	rm $(OBJPATH)*.o proGamma
