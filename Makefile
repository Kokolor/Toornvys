CPP = clang++
CPPFLAGS = -I include/
OUT = tvyscc
CFILES = $(shell find . -type f -name '*.cpp')
OBJECTS = $(CFILES:.cpp=.o)

all: $(OUT)

$(OUT): $(OBJECTS)
	$(CPP) $(OBJECTS) -o $(OUT)
    
%.o: %.c
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(OUT)
