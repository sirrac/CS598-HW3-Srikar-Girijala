

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


#define MAX(a, b) (((a) > (b)) ? (a) : (b))


static void die(const char *s) {
    fprintf(stderr, "%s\n", s);
    exit(EXIT_FAILURE);
}


int N = 801;
int **grid;


static void render(char *filename) {
    int chan = 3;
    size_t pixel_count = (size_t)N * N * chan;


    unsigned char *pixels = malloc(pixel_count);
    int stride = N * chan;


    if (!pixels)
        die("malloc failed for PNG pixel buffer");


    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {


            unsigned char r = 0, g = 0, b = 0;


            switch (grid[i][j]) {
                case 0:
                    r = 15; g = 15; b = 25;
                    break;


                case 1:
                    r = 0; g = 210; b = 255;
                    break;


                case 2:
                    r = 230; g = 30; b = 130;
                    break;


                case 3:
                    r = 255; g = 200; b = 0;
                    break;


                default:
                    r = 0; g = 0; b = 0;
            }


            size_t pid = ((size_t)i * N + j) * chan;


            pixels[pid] = r;
            pixels[pid + 1] = g;
            pixels[pid + 2] = b;
        }
    }


    if (stbi_write_png(filename, N, N, chan, pixels, stride)) {
        printf("Successfully saved sandpile identity to: %s\n", filename);
    } else {
        free(pixels);
        die("Error: stb_image_write failed to save PNG");
    }


    free(pixels);
}


void stabilize() {
    int changed;


    int center = N / 2;
    int radius_sq = center * center;


    do {
        changed = 0;


        for (int i = 0; i < N; ++i) {


            int dx = i - center;
            int max_j = MAX(i, N - 1 - i);


            for (int j = 0; j <= max_j; ++j) {


                int dy = j - center;


                if (dx * dx + dy * dy > radius_sq)
                    continue;


                int grains = grid[i][j];


                if (grains < 4)
                    continue;


                int topple_count = grains / 4;

                grid[i][j] -= 4 * topple_count;

                changed = 1;

                if (i > 0)
                    grid[i - 1][j] += topple_count;

                if (j > 0)
                    grid[i][j - 1] += topple_count;

                if (i < N - 1)
                    grid[i + 1][j] += topple_count;

                if (j < N - 1)
                    grid[i][j + 1] += topple_count;
            }
        }


    } while (changed);
}


void find_identity() {


    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            grid[i][j] = 6;
        }
    }


    stabilize();


    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            grid[i][j] = 6 - grid[i][j];
        }
    }


    stabilize();
}


int main(int argc, char **argv) {


    if (argc != 2) {
        fprintf(stderr, "Usage: %s N\n", argv[0]);
        return EXIT_FAILURE;
    }


    char *end;
    uintmax_t parsed = strtoumax(argv[1], &end, 10);


    if (errno || argv[1][0] < '0' || argv[1][0] > '9' ||
        *end || !parsed || parsed > INT32_MAX) {
        die("Grid size must be a positive integer");
    }


    N = (int)parsed;


    if ((size_t)N > SIZE_MAX / sizeof(int *))
        die("Grid size too large");


    grid = malloc((size_t)N * sizeof(int *));


    if (!grid)
        die("Grid allocation failed");


    for (int i = 0; i < N; ++i) {
        grid[i] = malloc((size_t)N * sizeof(int));


        if (!grid[i])
            die("Grid row allocation failed");
    }


    find_identity();


    render("identity.png");


    for (int i = 0; i < N; ++i)
        free(grid[i]);


    free(grid);


    return EXIT_SUCCESS;
}



