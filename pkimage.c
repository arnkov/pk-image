#include "qoi.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"
#define SOKOL_IMPL
#include "sokol_args.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ENDS_WITH(S, E) (strcmp(S + strlen(S) - (sizeof(E)-1), E) == 0)

static const char* usage = "pkimage in=path/to/image out=output/image [mask=path/to/maskimage] [r=resize_factor]";

typedef struct {
    void* pix;
    int w, h;
} image_desc;

static void release_image_desc(image_desc* desc) {
    assert(desc);
    if (desc->pix) {
        free(desc->pix);
    }
}

static image_desc load_image(const char* path) {
    image_desc ret = { 0 };
    if (ENDS_WITH(path, ".png")) {
        ret.pix = (void*)stbi_load(path, &ret.w, &ret.h, NULL, 4);
    }
    else if (ENDS_WITH(path, ".qoi")) {
        qoi_desc qoi = { 0 };
        ret.pix = (void*)qoi_read(path, &qoi, 4);
        ret.w = qoi.width;
        ret.h = qoi.height;
    }
    return ret;
}

static int write_image(image_desc* desc, const char* path) {
    if (ENDS_WITH(path, ".qoi")) {
        qoi_desc qoi = { 0 };
        qoi.channels = 4;
        qoi.width = desc->w;
        qoi.height = desc->h;
        qoi.colorspace = QOI_SRGB;
        return qoi_write(path, desc->pix, &qoi);
    }
    else if (ENDS_WITH(path, ".png")) {
        return stbi_write_png(path, desc->w, desc->h, 4, desc->pix, 0);
    }
    return 0;
}

static image_desc resize_image(image_desc* desc, int s) {
    puts("Resizing image...");
    image_desc ret = { 0 };
    ret.w = desc->w / s;
    ret.h = desc->h / s;
    ret.pix = stbir_resize_uint8_srgb(desc->pix, desc->w, desc->h, 0, NULL, ret.w, ret.h, 0, STBIR_RGBA);
    return ret;
}

static void pack_red_into_alpha(image_desc* base, image_desc* mask) {
    if (!base->pix || !mask->pix || base->w != mask->w || base->h != mask->h) {
        puts("Error: Image dimensions do not match!");
        return;
    }

    unsigned char* base_pix = (unsigned char*)base->pix;
    unsigned char* mask_pix = (unsigned char*)mask->pix;
    int num_pixels = base->w * base->h;

    for (int i = 0; i < num_pixels; i++) {
        base_pix[i * 4 + 3] = mask_pix[i * 4]; // Copy red channel into alpha
    }
}

int main(int argc, char* argv[]) {
    sargs_setup(&(sargs_desc) { .argc = argc, .argv = argv });

    if (!sargs_exists("in") || !sargs_exists("out")) {
        puts(usage);
        sargs_shutdown();
        return 0;
    }

    image_desc in = load_image(sargs_value("in"));
    if (!in.pix) {
        puts("Failed to load input image!");
        sargs_shutdown();
        return 0;
    }

    if (sargs_exists("mask")) {
        image_desc mask = load_image(sargs_value("mask"));
        if (mask.pix) {
            pack_red_into_alpha(&in, &mask);
            release_image_desc(&mask);
        }
        else {
            puts("Failed to load mask image!");
        }
    }

    if (sargs_exists("r")) {
        int factor = atoi(sargs_value("r"));
        if (factor > 1) {
            image_desc resized = resize_image(&in, factor);
            release_image_desc(&in);
            in = resized;
        }
        else {
            puts("Resize factor must be greater than 1. Skipping resizing.");
        }
    }

    int ok = write_image(&in, sargs_value("out"));
    if (!ok) {
        puts("Failed to write image!");
    }

    release_image_desc(&in);
    sargs_shutdown();
    puts("Done.");
    return 0;
}
