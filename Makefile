# Compiler
CC = gcc
# Flags: -Wall (all warnings), -g (for debugging)
CFLAGS = -Wall -g -std=c99

# The target executable
TARGET = assembler

# All .ys test files
TESTS = test1_halt.ys test2_nop.ys test3_rrmovl.ys test4_irmovl.ys \
        test5_rmmovl.ys test6_mrmovl.ys test7_addl.ys test8_jXX.ys \
        test9_loop.ys test10_stack.ys

# Corresponding .bin output files
BINS = $(TESTS:.ys=.bin)
# Corresponding .hex output files for results
HEXS = $(TESTS:.ys=.hex)

# Default rule: compile assembler
all: $(TARGET)

# Build rule
$(TARGET): a3.c
	$(CC) $(CFLAGS) -o $(TARGET) a3.c

# Test rule - runs assembler on all test files
test: $(TARGET)
	@echo "----------------------------------------"
	@echo "Running Y86 Assembler Tests..."
	@echo "----------------------------------------"
	@for t in $(TESTS:.ys=); do \
		echo "Assembling $$t.ys ..."; \
		cp $$t.ys input.ys; \
		./$(TARGET) > /dev/null; \
		mv output.bin $$t.bin; \
		if command -v xxd >/dev/null 2>&1; then \
			xxd -p $$t.bin | sed 's/\(..\)/0x\1 /g' > $$t.hex; \
		elif command -v hexdump >/dev/null 2>&1; then \
			hexdump -v -e '"0x" /1 "%02x" " "' $$t.bin > $$t.hex; \
		else \
			echo "No hex conversion tool found! Skipping hex output."; \
		fi; \
		if [ -f $$t.hex.expected ]; then \
			if diff -q $$t.hex $$t.hex.expected >/dev/null; then \
				echo "PASS: $$t"; \
			else \
				echo "FAIL: $$t (Output mismatch)"; \
			fi; \
		else \
			echo "SKIP: No expected file for $$t"; \
		fi; \
		echo "----------------------------------------"; \
	done
	@echo "All tests completed."

# Clean up build files
clean:
	rm -f $(TARGET) $(BINS) $(HEXS) output.bin input.ys

# Phony targets
.PHONY: all test clean