CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I. -I$(HOME)/.local/include
LDFLAGS = -L. -L$(HOME)/.local/lib -lreadline

SRC = main.cpp parser.cpp builtin.cpp ls.cpp fg_bg.cpp pinfo.cpp search.cpp io_redirection.cpp pipeline.cpp autocomplete.cpp history.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = mysh

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp fun.h
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
