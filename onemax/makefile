# MakeFile for onemax 'learning empirical project'

# Global flags
CFLAGS_all := -Wall -Wno-unused-variable -Wno-unused-function -std=c++11

# Compiler
CXX_web := emcc
CXX_native := g++-4.8

# Other flags
OFLAGS_native := -g -pedantic

# Bringing flag options together
CFLAGS_native := $(CFLAGS_all)
CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../Empirical/emtools/library_emp.js --js-library ../../d3-emscripten/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1 -s DEMANGLE_SUPPORT=1

JS_TARGETS := onemax.js

default: native

web: $(JS_TARGETS)
native: onemax_evolve.cc
	$(CXX_native) $(CFLAGS_native) onemax_evolve.cc -o onemax_evolve

onemax.js: onemax_web.cc
	mkdir -p web
	$(CXX_web) $(CFLAGS_web) onemax_web.cc -o web/onemax.js

# emcc -Wall -Wno-unused-variable -Wno-unused-function -std=c++11 --js-library ../../Empirical/emtools/library_emp.js --js-library ../../d3-emscripten/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1 -s DEMANGLE_SUPPORT=1 onemax_web.cc -o web/onemax.js
