GUROBI=${GUROBI_HOME}
GRAPHVIZ=/usr/include/graphviz

CXXFLAGS = -O2 -std=c++17 -Wall -Wextra \
 -I/opt/local/include -I$(GUROBI)/include -I$(GRAPHVIZ)

LDFLAGS = -L/opt/local/lib -L/usr/local/lib -L$(GUROBI)/src/build -L$(GUROBI)/lib \
 -lgvc -lcgraph -l:libgurobi_c++.a -lgurobi90 -lstdc++fs

OBJDIR = obj
SRCDIR = src

TARGET  = main
SRC     = $(wildcard $(SRCDIR)/*.cpp)
HEADERS = $(wildcard $(SRCDIR)/*.h)
_OBJ = $(subst $(SRCDIR), $(OBJDIR), $(SRC))
OBJ  = $(_OBJ:.cpp=.o)

all: $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir $(OBJDIR) 

$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(CXXFLAGS) $(LDFLAGS) 

	
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

mrproper: clean
	$(RM) -r $(OBJDIR) 
	$(RM) gurobi.log

rebuild: mrproper all

clean: 
	$(RM) $(TARGET)

.PHONY: clean mrproper rebuild
