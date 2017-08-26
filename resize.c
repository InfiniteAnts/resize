/**
 * Copies a BMP piece by piece and resizes it, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize n infile outfile\n");
        return 1;
    }

    // converting 1st argument to an int
    float f = atof(argv[1]);

    // ensuring f is postitive and less than 100
    if (f > 100 || f < 0)
    {
        fprintf(stderr, "Please enter a positive value of n less than or equal to 100.\n");
        return 5;
    }
    
    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];
    
    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);
    BITMAPFILEHEADER bf2 = bf;
    
    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);
    BITMAPINFOHEADER bi2 = bi;
    
    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // determine padding for original image
    int paddingin = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // change the headers for the resized outfile
    
    // multiplying the width with a factor of f 
    float Widthout = bi.biWidth * f;
    bi2.biWidth = round(Widthout);
    
    // multiplying the heigh with a factor of f
    float Heightout = bi.biHeight * f;
    bi2.biHeight = round(Heightout);
    
    // determine padding for the resized image
    int paddingout = (4 - (bi2.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // calculating the size of the resized image with the new width and height
    bi2.biSizeImage = ((sizeof(RGBTRIPLE) * bi2.biWidth) + paddingout) * abs(bi2.biHeight);
    
    // calculating the total size of the resized image with the headers
    bf2.bfSize = bi2.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf2, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi2, sizeof(BITMAPINFOHEADER), 1, outptr);
    
    if (f >= 1.0)
    {
        RGBTRIPLE *scanline = malloc(sizeof(RGBTRIPLE) * bi2.biWidth * round(f));
        
        int index;

        // iterate over infile's scanlines (for each row)
        for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
        {
            index = 0;
            
            // iterate over pixels in scanline (for each pixel)
            for (int j = 0; j < bi.biWidth; j++)
            {
                // temporary storage
                RGBTRIPLE triple;
            
                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
    
                // write RGB triple to outfile n times (horizontal resize)
                for (int k = 0; k < f; k++)
                {
                    scanline[index] = triple;
                    index++;
                }    
            }    
                
            for (int j = 0; j < f; j++)
            {
                fwrite(scanline, sizeof(RGBTRIPLE), bi2.biWidth, outptr);
                
                // write outfile padding
                for (int l = 0; l < paddingout; l++)
                {
                    fputc(0x00, outptr);
                }
                
            }
                
            // skip over infile padding, if any
            fseek(inptr, paddingin, SEEK_CUR);
        }
        
        free(scanline);
    
        // close infile
        fclose(inptr);
    
        // close outfile
        fclose(outptr);
    
        // success
        return 0;
    }
    
    // factor less than 1
    else
    {
        // width ratio
        float widthR = bi.biWidth / bi2.biWidth;
        
        // height ratio
        float heightR = bi.biHeight / bi2.biHeight;
        
        int wr = round(widthR);
        int hr = round(heightR);
        
        int k = 0;

        // for every row in outfile
        for (int i = 0; i < abs(bi2.biHeight); i++)
        {
            
            // set input pointer to the particalar scanline
            fseek(inptr, k * hr * ((bi.biWidth * sizeof(RGBTRIPLE)) + paddingin) + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), SEEK_SET);
            
            // for every pixel in the output
            for (int x = 0; x < bi2.biWidth; x++)
            {
                RGBTRIPLE triple;
                
                // read pixel into storage
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
                
                // write pixel from storage
                fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                                
                // input pointer goes to next pixel
                fseek(inptr, (wr - 1) * sizeof(RGBTRIPLE), SEEK_CUR);
                
                
            }
            
            // outfile padding
            for (int q = 0; q < paddingout; q++)
            {
                fputc(0x00, outptr);                
            }
            
            k++;
        }
        
        // close infile
        fclose(inptr);
    
        // close outfile
        fclose(outptr);
    
        // success
        return 0;
    }
}
