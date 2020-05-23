#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "bmp.h"

#pragma warning(disable : 4996)
bool solveMaze(int** maze, int x, int y, int** sol, int h, int w);
bool isSafe(int** maze, int x, int y, int h, int w, int** sol);
void printSol(int** maze, int** sol, int h, int w, FILE* outptr, int outpadding);

int main(int argc, char* argv[])
{

    FILE* txt = fopen("test1.txt", "w");

    // ensure proper usage
    if (argc != 3)
    {
        fprintf(stderr, "Usage: copy infile outfile\n");
        return 1;
    }

    // remember filenames
    char* infile = argv[1];
    char* outfile = argv[2];

    // open input file
    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE* outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    bi.biHeight = -(bi.biHeight);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    // write outfile's BITMAPFILEHEADER    
    int h = abs(bi.biHeight);
    int w = bi.biWidth;
    int w1 = bi.biWidth + 1;

    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {        
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            // read RGB triple from infile

            if (triple.rgbtRed == 0xff && triple.rgbtGreen == 0xff && triple.rgbtBlue == 0xff)
            {
                fputc('0', txt);
            }

            if (triple.rgbtRed == 0x00 && triple.rgbtGreen == 0x00 && triple.rgbtBlue == 0x00)
            {
                fputc('1', txt);
            }  
        }

        fputc('\n', txt);
        
        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);

    }
    // close infile
    fclose(inptr);
    fclose(txt);

    FILE* in = fopen("test1.txt", "r");
    FILE* out = fopen("test2.txt", "w");
    
    int** maze;

    maze = malloc((h) * sizeof(*maze));

    for (int i = 0; i < h; i++)
    {
        maze[i] = malloc((w1) * sizeof(*maze[i]));

        for (int j = 0; j < w1; j++)
        {
            maze[i][j] = fgetc(in);
            fputc(maze[i][j], out);
        }
    }

    int** sol;

    sol = malloc(h * sizeof(*maze));

    for (int i = 0; i < h; i++)
    {
        sol[i] = malloc(w1 * sizeof(*sol[i]));

        for (int j = 0; j < w1 - 1; j++)
        {
            sol[i][j] = maze[i][j];
        }       
    }

    fclose(in);
    fclose(out);

    printSol(maze, sol, h, w1, outptr, padding);

    fclose(outptr);

    free(maze);
    free(sol);

    printf("%d %d %d %d %d \n", w, w1, h, h-1, w1-2);

    return 0;
}

void printSol(int** maze, int** sol, int h, int w1, FILE* outptr, int outpadding)
{
    FILE* outsol = fopen("test3.txt", "w");

    if (solveMaze(maze, 0, 0, sol, h, w1) == false)
    {
        printf("no sol");
    }

    RGBTRIPLE red, white, black;

    red.rgbtRed = 0xff; red.rgbtBlue = 0x00; red.rgbtGreen = 0x00;
    white.rgbtRed = 0xff; white.rgbtBlue = 0xff; white.rgbtGreen = 0xff;
    black.rgbtRed = 0x00; black.rgbtBlue = 0x00; black.rgbtGreen = 0x00;

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w1; j++)
        {
            fputc(sol[i][j], outsol);
            if (sol[i][j] == '1')
            {
                fwrite(&black, sizeof(RGBTRIPLE), 1, outptr);
            }
            if (sol[i][j] == '0')
            {
                fwrite(&white, sizeof(RGBTRIPLE), 1, outptr);
            }
            if (sol[i][j] == '2')
            {
                fwrite(&red, sizeof(RGBTRIPLE), 1, outptr);
            }

        }
        for (int k = 0; k < outpadding; k++)
        {
            fputc(0x00, outptr);
        }
        fputc('\n', outsol);
    }
    fclose(outsol);
}


bool isSafe(int** maze, int x, int y, int h, int w1, int **sol)
{
    if (x >= 0 && x < h && y >= 0 && y < w1 - 1 && maze[x][y] == '0')
        return true;

    return false;
}

bool solveMaze(int** maze, int x, int y, int** sol, int h, int w1)
{
    if (x == h - 1 && y == w1 - 2 && maze[x][y] == '0')
    {
        sol[x][y] = '2';
        return true;
    }
    if ((isSafe(maze, x, y, h, w1, sol) == true) && sol[x][y] != '2')
    {
        sol[x][y] = '2';

        if (solveMaze(maze, x + 1, y, sol, h, w1) == true)
        {
            return true;
        }
        if (solveMaze(maze, x, y + 1, sol, h, w1) == true)
        {
            return true;
        }

        if (solveMaze(maze, x - 1, y, sol, h, w1) == true)
        {
            return true;
        }
        if (solveMaze(maze, x, y - 1, sol, h, w1) == true)
        {
            return true;
        }
        sol[x][y] = sol[x][y];

        return false;
    }

    return false;


}