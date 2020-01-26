TARGET = ZGloom
LIBS = -lSDL2 -lSDL2_mixer -lxmp
CCX = g++
CXXFLAGS = -g -fpermissive

.PHONY: default all clean

default: $(TARGET)
all: default

HEADERS = $(wildcard *.h)
OBJS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

%.o: %.c $(HEADERS)
	$(CCX) $(CXXFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	$(CCX) $(OBJS) $(LIBS) -o $@

clean:
	rm -f *.o
	rm -f $(TARGET)
