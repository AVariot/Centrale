CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2 -Isrc -I.
LDFLAGS  := -lpthread

TARGET        := app
CLIENT_TARGET := client
BUILDDIR      := build

SRCS := main.cpp $(shell find src -type f -name '*.cpp' -not -path 'src/client/*')
OBJS := $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

fclean: clean
	rm -rf log/*

re: clean all
