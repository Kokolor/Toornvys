CPP = clang++
CPPFLAGS = -I include/ `llvm-config --cxxflags`
OUT = tvyscc
CFILES = $(shell find . -type f -name '*.cpp')
OBJECTS = $(CFILES:.cpp=.o)

all: $(OUT)

$(OUT): $(OBJECTS)
	$(CPP) $(OBJECTS) -o $(OUT) `llvm-config --ldflags --libs`
    
%.o: %.c
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(OUT)
