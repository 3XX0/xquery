xquery
======

Dependencies :
- [optional] boost graph library + graphviz
- bison >= 2.5
- flex
- C++11 compiler support

Build :
make clean && make

Usage :
./xquery filename

alternatively, for the graphiz support (generate ast.dot) use
make USE_BOOST_GRAPHVIZ=true
also, you can display the ast generated with
make display_ast

