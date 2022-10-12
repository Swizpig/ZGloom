TARGET = ZGloom
LIBS = -lSDL2 -lSDL2_mixer -lxmp
CXX = g++
CXXFLAGS = -g

.PHONY: default all clean

default: $(TARGET)
all: default

HEADERS = $(wildcard *.h)
OBJS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

%.o: %.c $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -o $@

clean:
	rm -f *.o
	rm -f $(TARGET)
