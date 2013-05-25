xquery
======

Simplified processor for XQuery following the specifications of `xpath-semantics.pdf'

Dependencies
------------
- [optional] boost graph library + graphviz
- bison >= 2.5
- flex
- libxmlpp
- C++11 compiler support

Build
-----
        make clean && make

alternatively, for the graphiz support (generate ast.dot) use
        make USE_BOOST_GRAPHVIZ=true
also, you can display the ast generated with
        make ast

Usage
-----
        ./xquery filename
