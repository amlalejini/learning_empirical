# MakeFile for onemax 'learning empirical project'

# Global flags
CFLAGS_all := -Wall -Wno-unused-variable -Wno-unused-function -std=c++11 -I../../Empirical/ -I./

# Compiler
CXX_web := emcc
CXX_native := g++

# Other flags
OFLAGS_native := -g -pedantic

# Bringing flag options together
CFLAGS_native := $(CFLAGS_all)
CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../Empirical/emtools/library_emp.js --js-library ../../d3-emscripten/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1 -s DEMANGLE_SUPPORT=1

JS_TARGETS := evo_in_physics_pt1.js

default: native

web: $(JS_TARGETS)
native: evo_in_physics_pt1.cc
	$(CXX_native) $(CFLAGS_native) evo_in_physics_pt1.cc -o evo_in_physics_pt1

onemax.js: evo_in_physics_pt1_web.cc
	mkdir -p web
	$(CXX_web) $(CFLAGS_web) evo_in_physics_pt1_web.cc -o web/evo_in_physics_pt1.js

# emcc -Wall -Wno-unused-variable -Wno-unused-function -std=c++11 --js-library ../../Empirical/emtools/library_emp.js --js-library ../../d3-emscripten/library_d3.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1 -s DEMANGLE_SUPPORT=1 onemax_web.cc -o web/onemax.js
