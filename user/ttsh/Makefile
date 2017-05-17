.PHONY: all

SRCDIR=src
OUTDIR=out
DEPDIR=.dep

#CXX= g++
CXX= ../../g++-supos
CXXFLAGS= -I $(SRCDIR) -I $(OUTDIR) --std=c++14
CXXWFLAGS= -Wall -Wextra $(CXXFLAGS)
SRC=$(wildcard $(SRCDIR)/*.cpp)
OBJ=$(patsubst $(SRCDIR)/%.cpp,$(OUTDIR)/%.o,$(SRC))  $(OUTDIR)/parse.o $(OUTDIR)/lex.o
DEPF = $(wildcard $(DEPDIR)/*.d)
PARSER = $(OUTDIR)/parse.o

all: ttsh

ttsh: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

$(OUTDIR)/parse.hpp : $(PARSER)


$(OUTDIR)/%.o: $(SRCDIR)/%.cpp $(PARSER)
	@mkdir -p $(OUTDIR)
	@mkdir -p $(DEPDIR)
	$(CXX) $(CXXWFLAGS) -c $< -o $@
	@$(CXX) $(CXXFLAGS) -MM -MT '$@' -MF $(patsubst $(SRCDIR)/%.cpp, $(DEPDIR)/%.d, $<)  $<

$(OUTDIR)/lex.o: $(SRCDIR)/lex.l $(PARSER)
	@mkdir -p $(OUTDIR)
	flex -o $(OUTDIR)/lex.cpp $<
	$(CXX) $(CXXFLAGS) -c $(OUTDIR)/lex.cpp -o $@

$(OUTDIR)/parse.o: $(SRCDIR)/parse.y
	@mkdir -p $(OUTDIR)
	bison -r all -d $< -o $(OUTDIR)/parse.cpp
	vim $(OUTDIR)/parse.cpp -u NONE -es '+g/try$$/norm j%d2dd%' '+g/try$$/norm d2d' '+wq'
	$(CXX) $(CXXFLAGS) -c $(OUTDIR)/parse.cpp -o $@

clean:
	rm -rf $(OUTDIR)
	rm -rf $(DEPDIR)

include $(DEPF)
