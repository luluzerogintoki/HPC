src_path = src# .c files and .h files
asm_path = asm# .s files
bin_path = .

CC = gcc
CFLAGS = -O2 -march=native -lm -g -lpthread
BINS = \
	$(bin_path)/newton

.PHONY: all
all: $(BINS)

#$(bin_path)/output_file: $(src_path)/some_program.c
#	$(CC) $(CFLAGS) $< -o $@

$(bin_path)/newton: $(src_path)/newton.c $(src_path)/writing.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean
clean: 
	rm $(BINS)
