#include <stdio.h>
#include <stdlib.h>
#include <tetris.h>

int
main(int argc, char *argv[]) {
	if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    tetris_run(20, 25, NULL, atoi(argv[1]));
    return EXIT_SUCCESS;
}
