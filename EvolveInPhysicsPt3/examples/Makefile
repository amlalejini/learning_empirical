# Flags to use regardless of compiler
#CFLAGS_all := -Wall -Wno-unused-function -std=c++11 -I../../ -I./
CFLAGS_all := -Wall -Wno-unused-variable -Wno-unused-function -std=c++11 -I../../../Empirical/ -I./

# Emscripten compiler information
CXX_web := emcc
CXX_native := g++

OFLAGS_native := -g -pedantic

OFLAGS_web := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -g4 -DEMP_NDEBUG -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -g4 -DNDEBUG -s TOTAL_MEMORY=67108864 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -Werror -DNDEBUG -s TOTAL_MEMORY=67108864
#OFLAGS_web := -Oz -DNDEBUG -s TOTAL_MEMORY=67108864 -s ASSERTIONS=1
#OFLAGS_web := -Os -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_native := $(CFLAGS_all) $(OFLAGS_native)

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../../Empirical/emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1


JS_TARGETS := Shapes.js

default: web

web: $(JS_TARGETS)

Shapes.js: Shapes.cc
	$(CXX_web) $(CFLAGS_web) Shapes.cc -o Shapes.js

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'
