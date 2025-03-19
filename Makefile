TARGET = DMXEngine
PREF_SRC = ./src/
CSRC := $(wildcard $(PREF_SRC)*.c)
CSRC := $(CSRC) $(wildcard $(PREF_SRC)mathc/*.c)
COBJ := $(patsubst %.c, %.o, $(CSRC))
COBJ := $(subst src,build, $(COBJ))
LIB := $(LIB)

DIR := ${CURDIR}

FLAGS := -I$(DIR)\include

CC = gcc $(FLAGS)
LD = gcc

build: $(TARGET)
	@echo sources = $(CSRC)
	@echo objects = $(COBJ)
	rm build/*.o
$(TARGET): $(COBJ)
	$(LD) $(COBJ) -L$(DIR)\lib\glfw-3.4 -L$(DIR)\lib\vulkan -lvulkan-1 -lopengl32 -lglfw3dll -o $(TARGET).exe
%.o:
	$(CC) -c $(subst .o,.c,$(subst build,src, $@)) -o $@
debug:
	@echo sources = $(CSRC)
	@echo objects = $(COBJ)
	@echo $(subst .o,.c,$(subst build,src, $(COBJ)))