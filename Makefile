CXX = g++
CXXFLAGS = -std=c++11 -Wall -Iinclude

TARGET = ThreadPool_test 
SRCDIR = src
INCLUDEDIR = include 
OBJDIR = obj

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

all : $(TARGET)

$(TARGET): $(OBJECTS) $(OBJDIR)/main.o
	$(CXX) $(OBJECTS) $(OBJDIR)/main.o -lpthread -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/main.o: test/main.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) &(TARGET)

.PHONY: all clean
