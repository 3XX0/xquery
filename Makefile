CXX = g++
CXXFLAGS = -O2 -W -Wextra -Wall -std=c++11
LDFLAGS = -lfl

TARGET = xquery

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

.PHONY: all clean
.SUFFIXES: .yy .cc .l

all:	$(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) 

.yy.o:
	bison -d -v $<
	$(CXX) $(CXXFLAGS) -c xquery_parser.tab.cc -o xquery_parser.o

.l.o:
	flex -o xquery_lexer.cc $<
	$(CXX)  $(CXXFLAGS) -c xquery_lexer.cc -o xquery_lexer.o

.cc.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	rm -rf $(CLEANLIST) *.o $(TARGET)
