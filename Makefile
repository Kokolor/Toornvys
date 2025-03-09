CPP = clang++
CPPFLAGS = -I include/ `llvm-config-19 --cxxflags`
OUT = tvyscc
CFILES = $(shell find . -type f -name '*.cpp')
OBJECTS = $(CFILES:.cpp=.o)

all: $(OUT)

$(OUT): $(OBJECTS)
	$(CPP) $(OBJECTS) -o $(OUT) `llvm-config-19 --ldflags --libs`
    
%.o: %.c
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(OUT)
