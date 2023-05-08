C_SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c,dist/%.o,$(C_SOURCES))
LINKER_FLAGS=

native-router:
	@make -B all --no-print-directory

all: src/router.so src/index.ts router
clean:
	rm -fr src/router.so dist/*

dist:
	mkdir -p dist

src/router.so: $(OBJECTS)
	clang -shared -o $@ $(OBJECTS) $(LINKER_FLAGS) -g

router: $(OBJECTS)
	clang -o $@ $(OBJECTS) $(LINKER_FLAGS) -g

dist/%.o: src/%.c dist
	clang -fPIC -c -o $@ $< -g

src/index.ts: $(C_SOURCES)
	bun src/codegen.ts $(C_SOURCES)
