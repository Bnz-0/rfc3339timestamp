STD = -std=gnu99
FLAGS = -Wall -Wextra -Werror -pedantic
EXAMPLES_DIR = examples
EXAMPLES_SRC := $(shell find $(EXAMPLES_DIR)/ -name "*.c")
EXAMPLES := $(patsubst %.c, %, $(EXAMPLES_SRC))


test: rfc3339.h test.c
	$(CC) $(STD) $(FLAGS) test.c -o test
	./test

examples: rfc3339.h $(EXAMPLES)

$(EXAMPLES_DIR)/%: $(EXAMPLES_DIR)/%.c
	$(CC) $(STD) $(FLAGS) $< -o $@

# require astyle
format:
	astyle --project=./astylerc rfc3339.h test.c

# require cppcheck
check:
	cppcheck --check-level=exhaustive rfc3339.h

clean:
	rm -f test $(EXAMPLES)
