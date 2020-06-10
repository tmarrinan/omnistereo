############
CXX= g++
CXX_FLAGS= -std=c++11

MACHINE= $(shell uname -s)

ifeq ($(MACHINE),Darwin)
	INC= -I/usr/local/include -I${HOME}/local/include -I./include
	LIB= -L/usr/local/lib -L${HOME}/local/lib -lglfw -lglad
else
	INC= -I/usr/include -I./include
	LIB= -L/usr/lib64 -lGL -lglfw -lglad
endif

SRCDIR= src
OBJDIR= obj
BINDIR= bin

OBJS= $(addprefix $(OBJDIR)/, main.o)
EXEC= $(addprefix $(BINDIR)/, omnistereo)

mkdirs:= $(shell mkdir -p $(OBJDIR) $(BINDIR))


# BUILD EVERYTHING
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) -o $@ $^ $(LIB)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $< $(INC)


# REMOVE OLD FILES
clean:
	rm -f $(OBJS) $(EXEC)
