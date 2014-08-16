/*
 * Copyright (c) 2014 Hayaki Saito
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LIBSIXEL_SIXEL_H
#define LIBSIXEL_SIXEL_H

#define PALETTE_MAX 256

typedef struct sixel_dither {
    unsigned int ref;
    unsigned char *palette;
    unsigned short *cachetable;
    int reqcolors;  /* requested colors */
    int ncolors;    /* active colors */
    int origcolors; /* original colors */
    int optimized;
    int method_for_largest;
    int method_for_rep;
    int method_for_diffuse;
    int quality_mode;
} sixel_dither_t;

/* LSImage definition */
typedef struct LSImageStruct {
    /* Palette-based image pixels */
    unsigned char *pixels;  /* pixel buffer */
    int sx;        /* width */
    int sy;        /* height */
    int depth;     /* bytes per pixel */
    int keycolor;  /* background color */
    int borrowed;  /* whether pixels is borrowed reference */
    sixel_dither_t *dither;
} LSImage, *LSImagePtr;

typedef int (* putchar_function)(int ch);
typedef int (* printf_function)(const char *fmt, ...);

typedef struct LSNode {
    struct LSNode *next;
    int pal;
    int sx;
    int mx;
    unsigned char *map;
} sixel_node_t;

typedef struct LSOutputContextStruct {
    /* compatiblity flags */

    /* 0: 7bit terminal,
     * 1: 8bit terminal */
    unsigned char has_8bit_control;

    /* 0: the terminal has sixel scrolling
     * 1: the terminal does not have sixel scrolling */
    unsigned char has_sixel_scrolling;

    /* 0: DECSDM set (CSI ? 80 h) enables sixel scrolling
       1: DECSDM set (CSI ? 80 h) disables sixel scrolling */
    unsigned char has_sdm_glitch;

    putchar_function fn_putchar;
    printf_function fn_printf;

    unsigned char conv_palette[PALETTE_MAX];
    int save_pixel;
    int save_count;
    int active_palette;

    sixel_node_t *node_top;
    sixel_node_t *node_free;
} LSOutputContext, *LSOutputContextPtr;

/* converter API */

#ifdef __cplusplus
extern "C" {
#endif

LSImagePtr
LibSixel_SixelToLSImage(unsigned char *p, int len);

int
LibSixel_LSImageToSixel(LSImagePtr im, LSOutputContextPtr context);

#ifdef __cplusplus
}
#endif

/* LSImage manipulation API */

#ifdef __cplusplus
extern "C" {
#endif

LSImagePtr
LSImage_create(int sx, int sy, int depth, int ncolors);

LSImagePtr
sixel_create_image(unsigned char *pixels, int sx, int sy, int depth,
                   int borrowed, sixel_dither_t *dither);

void
LSImage_destroy(LSImagePtr im);

void
LSImage_setpalette(LSImagePtr im, int n, int r, int g, int b);

void
LSImage_setpixels(LSImagePtr im, unsigned char *pixels);

void
LSImage_setpixel(LSImagePtr im, int x, int y, int color);

void
LSImage_copy(LSImagePtr dst, LSImagePtr src, int w, int h);

void
LSImage_fill(LSImagePtr im, int color);

void
LSImage_fillrectangle(LSImagePtr im, int x1, int y1, int x2, int y2, int color);

#ifdef __cplusplus
}
#endif


/* LSOutputContext manipulation API */

#ifdef __cplusplus
extern "C" {
#endif

LSOutputContextPtr const
LSOutputContext_create(putchar_function fn_putchar, printf_function fn_printf);

void
LSOutputContext_destroy(LSOutputContextPtr context);

#ifdef __cplusplus
}
#endif

/* Color quantization API */

/* method for finding the largest dimention for splitting,
 * and sorting by that component */
enum methodForLargest {
    LARGE_AUTO,  /* choose automatically the method for finding the largest
                    dimention */
    LARGE_NORM,  /* simply comparing the range in RGB space */
    LARGE_LUM    /* transforming into luminosities before the comparison */
};

/* method for choosing a color from the box */
enum methodForRep {
    REP_AUTO,           /* choose automatically the method for selecting
                           representative color from each box */
    REP_CENTER_BOX,     /* choose the center of the box */
    REP_AVERAGE_COLORS, /* choose the average all the color
                           in the box (specified in Heckbert's paper) */
    REP_AVERAGE_PIXELS  /* choose the averate all the pixels in the box */
};

/* method for dithering */
enum methodForDiffuse {
    DIFFUSE_AUTO,       /* choose diffusion type automatically */
    DIFFUSE_NONE,       /* don't diffuse */
    DIFFUSE_ATKINSON,   /* diffuse with Bill Atkinson's method */
    DIFFUSE_FS,         /* diffuse with Floyd-Steinberg method */
    DIFFUSE_JAJUNI,     /* diffuse with Jarvis, Judice & Ninke method */
    DIFFUSE_STUCKI,     /* diffuse with Stucki's method */
    DIFFUSE_BURKES      /* diffuse with Burkes' method */
};

/* quality modes */
enum qualityMode {
    QUALITY_AUTO,       /* choose quality mode automatically */
    QUALITY_HIGH,       /* high quality */
    QUALITY_LOW,        /* low quality */
};


/* the palette manipulation API */

#ifdef __cplusplus
extern "C" {
#endif

/* exported functions */

sixel_dither_t *
sixel_dither_create(int ncolors);

void
sixel_dither_destroy(sixel_dither_t *dither);

void
sixel_dither_ref(sixel_dither_t *dither);

void
sixel_dither_unref(sixel_dither_t *dither);

int
sixel_prepare_palette(sixel_dither_t *dither, unsigned char *data,
                      int width, int height, int depth);

int
sixel_apply_palette(LSImagePtr im);


/* deprecated */
unsigned char *
LSQ_MakePalette(unsigned char *data, int x, int y, int depth,
                int reqcolors, int *ncolors, int *origcolors,
                enum methodForLargest const methodForLargest,
                enum methodForRep const methodForRep,
                enum qualityMode const qualityMode) ;

/* deprecated */
int
LSQ_ApplyPalette(unsigned char *data, int width, int height, int depth,
                 unsigned char *palette, int ncolor,
                 enum methodForDiffuse const methodForDiffuse,
                 int foptimize,
                 unsigned short *cachetable,
                 unsigned char *result) ;


/* deprecated */
extern void
LSQ_FreePalette(unsigned char * data) ;

#ifdef __cplusplus
}
#endif


#endif  /* LIBSIXEL_SIXEL_H */

/* emacs, -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
/* vim: set expandtab ts=4 : */
/* EOF */
