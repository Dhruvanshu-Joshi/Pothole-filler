CFLAGS = -g -Wall -I../../../csrc/.  `sdl2-config --cflags`  -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections

SRC = $(shell ls ../../../csrc/*.c) $(shell ls ../common/*.c ) main.c 

OBJ = $(SRC:.c=.o)

helloworld: $(OBJ) 
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) `sdl2-config --libs` -o u8g2_sdl

clean:	
	-rm $(OBJ) u8g2_sdl

