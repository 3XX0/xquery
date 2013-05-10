CXX = g++
CXXFLAGS = -O3 -W -Wextra -Wall -std=c++11
LDFLAGS = -lfl

ifdef USE_BOOST_GRAPHVIZ
CXXFLAGS += -DUSE_BOOST_GRAPHVIZ
LDFLAGS += -lboost_graph
endif

TARGET = xquery
DOT_AST = ast.dot

SRCS = main.cc \
       xquery_driver.cc \
       xquery_parser.yy \
       xquery_lexer.l \

OBJS = xquery_parser.o \
       xquery_lexer.o \
       xquery_driver.o \
       main.o \

CLEANLIST = xquery_parser.tab.cc \
            xquery_parser.tab.hh \
            xquery_parser.output \
            xquery_lexer.cc \
            location.hh \
            stack.hh \
            position.hh \
            ast.dot \
            ast.png

.PHONY: all clean display_ast
.SUFFIXES: .yy .cc .l

BLUE = "\033[1;34m"
NORMAL = "\033[0m"

all:	$(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) 

.yy.o:
	@echo -ne $(BLUE)
	bison -d -v $<
	@echo -ne $(NORMAL)
	$(CXX) $(CXXFLAGS) -c xquery_parser.tab.cc -o xquery_parser.o

.l.o:
	@echo -ne $(BLUE)
	flex -o xquery_lexer.cc $<
	@echo -ne $(NORMAL)
	$(CXX) $(CXXFLAGS) -c xquery_lexer.cc -o xquery_lexer.o

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(CLEANLIST) *.o $(TARGET)

display_ast: $(DOT_AST)
	dot -Gsize=10 -Tpng $(DOT_AST) > $(DOT_AST:.dot=.png)
	display $(DOT_AST:.dot=.png)
