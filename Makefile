binary		:= SNLS.ELF
ps2		:= 0 
debug		:= 0
flags		:= -g -DDEBUG=0 --std=c11 
includes	:= -Iinclude -I. -Iinclude/general

ifeq ($(ps2), 1)
	source		+= src/platform/ps2
	prefix		:= mips64r5900el-ps2-elf-
	flags		+= -D_EE -Dfor_ps2
	libraries	+= -L$(PS2SDK)/ee/lib -L$(PS2SDK)/ports/lib -L$(GSKIT)/lib \
			   -ldebug -lkernel -lps2_drivers -lgskit -ldma -ldmakit \
			   -lps2stuff -lps2gl  -lc -lm -lstdc++ -llibpng16_static \
			   -lz

	includes	+= -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include \
			   -I$(GSKIT)/include -I$(PS2SDK)/ports/include/GL \
			   -I$(PS2SDK)/ports/include -I$(PS2SDK)/ports/include/freetype2 \
			   -I$(PS2SDK)/ports/include/libpng16/

	linkfile	:= -T$(PS2SDK)/ee/startup/linkfile
else
	libraries	+= -lc \
			   $(shell pkg-config --cflags --libs sdl2 gl libpng)

	includes    	+= -I/usr/include/freetype2 \
			   -I/usr/include/ -I/usr/include/GL \
			   -I/usr/include/libpng16 \
			   $(shell pkg-config --cflags sdl2 gl libpng)

	source		+= src/platform/pc
	prefix		:=
endif

ifeq  ($(debug), 1)
	flags		+= -fsanitize=address
	libraries	+= -lasan
endif

compiler	:= $(prefix)gcc
compiler_g++	:= $(prefix)g++

source		+= src \
		   src/core \
		   src/platform/cross \
		   src/emulator \
		   src/emulator/main \
		   src/tools

c_source 	+= $(foreach c_src, $(source), $(wildcard $(c_src)/*.c))
#c_source	+= $(foreach c_src, $(source), $(wildcard $(c_src)/*.cpp))
c_objects	+= $(patsubst %.c,build/%.o,$(c_source))
d_files		+= $(patsubst %.c,build/%.d,$(c_source))
#c_objects	+= $(patsubst %.cpp,build/%.o,$(c_source))
#c_objects	+= $(patsubst %.cpp,build/%.o,$(cpp_source))

# not used
# objects		:= $(c_objects)


all: $(binary)

$(binary): $(c_objects)
	@echo -e '\n\t Linking to $@...\n'
	$(compiler) $(linkfile) -o $@ $^ $(includes) $(libraries)

build/%.o: %.c 
	@mkdir -p $(@D)
	@echo -e '\n\t Building $@\n'
	$(compiler) -c $^ -o $@ $(includes) $(flags) $(libraries)


#build/%.o: %.cpp
#	@mkdir -p $(@D)
#	@echo Building $@
#	$(compiler_g++) -c $^ -o $@ $(includes) $(flags) $(libraries)

clean:
	rm -rf build

