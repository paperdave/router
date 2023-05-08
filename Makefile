C_SOURCES = $(wildcard src/lib/*.c)
OBJECTS = $(patsubst src/lib/%.c,dist/lib/%.o,$(C_SOURCES))
LINKER_FLAGS=

native-router:
	@make -B all --no-print-directory

all: src/lib/router.so src/lib/index.ts src/lib/router
clean:
	rm -fr src/lib/router.so dist/*

dist:
	mkdir -p dist/lib

src/lib/router.so: $(OBJECTS)
	clang -shared -o $@ $(OBJECTS) $(LINKER_FLAGS) -g

src/lib/router: $(OBJECTS)
	clang -o $@ $(OBJECTS) $(LINKER_FLAGS) -g

dist/lib/%.o: src/lib/%.c dist
	clang -fPIC -c -o $@ $< -g

src/lib/index.ts: $(C_SOURCES)
	bun src/lib/codegen.ts $(C_SOURCES)
