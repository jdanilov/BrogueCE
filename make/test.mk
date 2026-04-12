# Test build target
# Build with: make bin/brogue-tests
# Run with:   make test

test_sources := $(wildcard src/test/*.c)
test_objects := $(test_sources:.c=.o)

# Game sources minus the platform main.c (which has its own main())
game_sources_for_test := $(filter-out src/platform/main.c,$(sources))
game_objects_for_test := $(game_sources_for_test:.c=.o)

$(test_sources:.c=.o): %.o: %.c src/test/test_harness.h src/brogue/Rogue.h src/brogue/Globals.h src/brogue/GlobalsBase.h vars/cppflags vars/cflags make/test.mk
	@echo "  CC  $<"
	@$(CC) $(cppflags) $(cflags) -Isrc/test -c $< -o $@

bin/brogue-tests: $(game_objects_for_test) $(test_objects) vars/cflags vars/LDFLAGS vars/libs make/test.mk
	@echo "  LD  $@"
	@$(CC) $(cflags) $(LDFLAGS) -o $@ $(game_objects_for_test) $(test_objects) $(libs)

test: bin/brogue-tests
	@./bin/brogue-tests

.PHONY: test
