/*
    UTIL.H header for reading and writing JPG in C++ uing jpeglib.h
    http://www.mit.edu/afs.new/sipb/project/amaya/src/Amaya/thotlib/image/jpeghandler.c
*/
#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <jpeglib.h>

using namespace std;

static inline int
ReadJPEG (char *filename, int *width, int *height, int *channels, unsigned char *(image[])){
    FILE *infile;
    if ((infile = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", filename);
        return 0;
    }
    struct jpeg_error_mgr jerr;
    struct jpeg_decompress_struct cinfo;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);
    *width = cinfo.output_width, *height = cinfo.output_height;
    *channels = cinfo.num_components;
    *image = (unsigned  char*) malloc(*width * *height * *channels * sizeof(*image));

    JSAMPROW rowptr[1];
    int row_stride = *width * *channels;
    while (cinfo.output_scanline < cinfo.output_height) {
        rowptr[0] = *image + row_stride * cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, rowptr, 1);
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 1;
}


static inline void
WriteJPEG(string fileN, int width, int height, int channels, unsigned char image[], int quality = 100){
    FILE *outfile;
    if ((outfile = fopen(fileN.c_str(), "wb")) == NULL) {
        fprintf(stderr, "can't open %s\n", fileN.c_str());
        exit(1);
    }
    struct jpeg_error_mgr jerr;
    struct jpeg_compress_struct cinfo;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo,outfile);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = channels;
    cinfo.in_color_space = channels == 1 ? JCS_GRAYSCALE : JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW rowptr[1];
    int row_stride = width * channels;
    while (cinfo.next_scanline < cinfo.image_height) {
        rowptr[0] = & image[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, rowptr, 1);
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
}

#endif 
