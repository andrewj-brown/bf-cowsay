#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#define SIGHUP 1
#define BASE_TAPE_LEN 512

char *map_file(char *path) {
    int bf_file = open(path, O_RDONLY);
    if (bf_file < 0) {
        printf("Couldn't find bf file: %s\n", path);
        exit(1);
    }

    struct stat *bf_stat = malloc(1 * sizeof(struct stat));
    fstat(bf_file, bf_stat);

    char *bf_mapped = mmap(NULL, bf_stat->st_size, PROT_READ, MAP_PRIVATE, bf_file, 0);

    close(bf_file);
    free(bf_stat);

    return bf_mapped;
}

int main(int argc, char **argv) {
    int opt;
    char *filename;
    bool verbose = false;

    while ((opt = getopt(argc, argv, ":")) != -1) {
        switch(opt) {
            case 'v':
                verbose = true;
                break;
            default:
                printf("Usage: ./%s [-v] /path/to/bf/file\n", argv[0]);
                exit(1);
        }
    }
    
    if (optind == (argc - 1)) {
        filename = argv[optind];
    } else {
        printf("Usage: ./%s [-v] /path/to/bf/file\n", argv[0]);
        exit(1);
    }

    char *bf_mapped = map_file(filename);
    size_t mapped_pointer = 0;
    size_t addr_last_loop = -1;

    size_t tape_length = BASE_TAPE_LEN;
    unsigned char *tape_memory = calloc(tape_length, sizeof(char));
    ssize_t tape_pointer = 0;

    while(bf_mapped[mapped_pointer] != '\0') {
        switch(bf_mapped[mapped_pointer]) {
            case '+':
                tape_memory[tape_pointer] += 1;
                break;
            case '-':
                tape_memory[tape_pointer] -= 1;
                break;
            case '>':
                tape_pointer += 1;

                if (tape_pointer > tape_length) {
                    tape_memory = realloc(tape_memory, tape_length * 2);
                    memset(tape_memory + tape_length, '\0', tape_length);
                    tape_length *= 2;
                }

                break;
            case '<':
                tape_pointer -= 1;

                if (tape_pointer < 0) {
                    raise(SIGSEGV);
                }

                break;
            case '[':
                if (tape_memory[tape_pointer] == 0) {
                    while (bf_mapped[mapped_pointer] != ']' && bf_mapped[mapped_pointer] != '\0') {
                        mapped_pointer += 1;
                    }
                    if (bf_mapped[mapped_pointer] == '\0') {
                        raise(SIGILL);
                    }
                } else {
                    addr_last_loop = tape_pointer;
                }

                break;
            case ']':
                if (addr_last_loop == -1) {
                    raise(SIGILL);
                } else {
                    tape_pointer = addr_last_loop - 1;
                    addr_last_loop = -1;
                }
                
                break;
            case '.':
                write(STDOUT_FILENO, tape_memory + tape_pointer, 1);
                break;
            case ',':
                if (read(STDIN_FILENO, tape_memory + tape_pointer, 1) <= 0) {
                    raise(SIGHUP);
                }
                break;
        }

        mapped_pointer += 1;
    }

    return 0
}