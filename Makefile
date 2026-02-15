binary		:= SNLS.ELF
ps2			?= 0
window		?= 1
debug			?= 0
flags			:= -g --std=c23 -DDEBUG=0 -D_GNU_SOURCE -Wstringop-overflow=0
libraries	:= -lm -lc
includes		:= -Iinclude -I. -Iinclude/general

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
			   -I$(PS2SDK)/ports/include/libpng16/ -I$(PS2SDK)/ports/include/SDL2

	linkfile	:= -T$(PS2SDK)/ee/startup/linkfile
else
	libraries	+= $(shell pkg-config --cflags --libs raylib freetype2 libpng)

	includes    	+= $(shell pkg-config --cflags freetype2 libpng)

	source		+= src/platform/pc
	prefix		:=
endif

ifeq	($(window), 1)
	flags	+= -DSNLS_WITH_WINDOW
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
					src/tools \
					src/dynarec

c_source 	+= $(foreach c_src, $(source), $(wildcard $(c_src)/*.c))
#c_source	+= $(foreach c_src, $(source), $(wildcard $(c_src)/*.cpp))
c_objects	+= $(patsubst %.c,bin/%.o,$(c_source))
d_files		+= $(patsubst %.c,bin/%.d,$(c_source))
#c_objects	+= $(patsubst %.cpp,bin/%.o,$(c_source))
#c_objects	+= $(patsubst %.cpp,bin/%.o,$(cpp_source))

# not used
# objects		:= $(c_objects)


all: check $(binary)
 
check:
	@if pkg-config freetype2 raylib libpng; then \
		continue; \
	else \
		echo "Error, some package wasn't found"; \
		exit 1; \
	fi

$(binary): $(c_objects)
	@echo -e '\n\t Linking to $@...\n'
	$(compiler) $(linkfile) -o $@ $^ $(includes) $(libraries)

bin/%.o: %.c
	@mkdir -p $(@D)
	@echo -e '\n\t Building $@\n'
	$(compiler) -c $^ -o $@ $(includes) $(flags) $(libraries)


#bin/%.o: %.cpp
#	@mkdir -p $(@D)
#	@echo Building $@
#	$(compiler_g++) -c $^ -o $@ $(includes) $(flags) $(libraries)

clean:
	rm -rf bin $(binary)
