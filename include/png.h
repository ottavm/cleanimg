/*
 * BSD 3-Clause License
 *
 * Copyright 2026 Murilo Ottávio A. Branco Reis
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PNG_H
#define PNG_H

#include <stdio.h>
#include <stdint.h>

struct png_tag_header {
	uint32_t len;
	uint32_t type;
};

struct png_tag {
	struct png_tag_header hdr;
	uint8_t* data;
	uint32_t crc;
};

typedef struct png_file {
	size_t tag_count;
	struct png_tag* tags;
} PNG;

int pngtcmp(struct png_tag_header* hdr, const char (*tstr)[4]);
/* Checks if the file `fptr` matches the PNG file header signature. 
 * If so, returns 1 (i.e., true), otherwise, returns 0 (i.e., false)
 * Note: this function will set the current file position indicator
 * to the beggining of the file. However, it will restore the
 * indicator position on exit. */
int is_file_png(FILE* fptr);

int png_taghdr_read(struct png_tag_header* dst, FILE* fptr);
int png_read_tag(struct png_tag* dst, FILE* fptr);

int png_read(PNG* dst, char* img_path);
int png_write(PNG* src, FILE* fptr);

void png_free(PNG* png);

/* This function 'soft erase' the EXIF metadata tag of a PNG `struct png_file`
 * representation, keeping the original file contents intact. */
int png_remove_exif_tag(PNG* png);
/* This functions erase permanently the 'eXIf' tag of a png file `fptr`
 * by directing writing the in the file stream. */
int png_file_erase_exif(FILE* fptr);

#endif /* PNG_H */

