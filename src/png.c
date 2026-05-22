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

#include "png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

int pngtcmp(struct png_tag_header* hdr, const char (*tstr)[4])
{
	if (hdr == NULL)
		return -1;

	return memcmp(&hdr->type, tstr, sizeof(char) * 4);
}

int is_file_png(FILE* fptr)
{
	int ret;
	uint8_t header[8];

	static const uint8_t PNG_SIGNATURE[8] = {
		0x89, 0x50, 0x4E, 0x47,
		0x0D, 0x0A, 0x1A, 0x0A,
	};

	if (fptr == NULL)
		return -1;

	rewind(fptr);
	if (ftell(fptr) == -1) {
		perror("rewind");
		return -1;
	}

	fread(header, sizeof(char) * 8, 1, fptr);
	ret = memcmp(PNG_SIGNATURE, header, sizeof(char) * 8);

	return !ret;
}

int png_taghdr_read(struct png_tag_header* dst, FILE* fptr)
{
	int e;
	struct png_tag_header buffer;

	e = fread(&buffer, sizeof(uint32_t) * 2, 1, fptr);
	if (e < 1) {
		if (feof(fptr)) {
			return EOF;
		} else if (ferror(fptr)) {
			return 1;
		} else {
			fprintf(stderr, "ERROR: fread returned a short value. Unknown error, probably undefined behavior.\n");
			return 1;
		}
	}

	buffer.len = ntohl(buffer.len);
	*dst = buffer;
	
	return 0;
}

int png_read_tag(struct png_tag* dst, FILE* fptr)
{
	int e;
	struct png_tag buffer = {0};
	uint8_t* chnk_data_buffer;

	e = png_taghdr_read(&buffer.hdr, fptr);
	switch (e) {
		case 1:
			perror("png_taghdr_read");
			return 1;
			break;
		case EOF:
			return EOF;
			break;
		default:
			break;
	}

	if (buffer.hdr.len <= 0) {
		buffer.data = NULL;
		
		e = fread(&buffer.crc, sizeof(uint32_t), 1, fptr);
		if (e < 1) {
			if (feof(fptr)) {
				return EOF;
			} else if (ferror(fptr)) {
				return 1;
			} else {
				fprintf(stderr, "ERROR: fread returned a short value. Unknown error, probably undefined behavior.\n");
				return 1;
			}
		}
	} else {
		size_t read;

		chnk_data_buffer = malloc(sizeof(*chnk_data_buffer) * buffer.hdr.len);
		if (chnk_data_buffer == NULL) {
			perror("malloc");
			abort();
		}

		read = fread(chnk_data_buffer, sizeof(uint8_t), buffer.hdr.len, fptr);
		read += fread(&buffer.crc, 1, sizeof(uint32_t), fptr);
		if (read < buffer.hdr.len + sizeof(uint32_t)) {
			if (feof(fptr)) {
				return EOF;
			} else if (ferror(fptr)) {
				return 1;
			} else {
				fprintf(stderr, "ERROR: fread returned a short value. Unknown error, probably undefined behavior.\n");
				return 1;
			}
		}

		buffer.data = chnk_data_buffer;
	}
	
	*dst = buffer;

	return 0;
}

int png_read(PNG* dst, char* img_path)
{
	PNG png_buffer;
	FILE* fptr;
	size_t tag_count;
	struct png_tag tag_buffer;
	struct png_tag* tag_list;

	if ((img_path == NULL) || (dst == NULL))
		return -1;

	fptr = fopen(img_path, "r");
	if (fptr == NULL)
		return 1;

	if (!is_file_png(fptr)) {
		fprintf(stderr, "ERROR: File \"%s\" does not contain a valid PNG header.\n", img_path);
		return 1;
	}

	tag_count = 10;
	tag_list = malloc(sizeof(*tag_list) * tag_count);
	if (tag_list == NULL) {
		perror("malloc");
		abort();
	}

	size_t i = 0;
	while (png_read_tag(&tag_buffer, fptr) != EOF) {
		if (i >= tag_count) {
			tag_count += tag_count / 2;
			tag_list = realloc(tag_list, sizeof(*tag_list) * tag_count);
			if (tag_list == NULL) {
				perror("realloc");
				abort();
			}
		}

		tag_list[i] = tag_buffer;
		i++;
	}

	if (tag_count > i) {
		tag_count = i;
		tag_list = realloc(tag_list, sizeof(*tag_list) * tag_count);
		if (tag_list == NULL) {
			perror("realloc");
			abort();
		}
	}

	png_buffer.tags = tag_list;
	png_buffer.tag_count = tag_count;

	*dst = png_buffer;

	fclose(fptr);

	return 0;
}

void png_free(PNG* png)
{
	if (png == NULL)
		return;

	if (png->tags) {
		for (size_t i = 0; i < png->tag_count; ++i) {
			struct png_tag* tag;

			tag = &png->tags[i];
			if ((tag->hdr.len > 0) && (tag->data != NULL))
				free(tag->data);
		}
		free(png->tags);
	}
}

int png_write(PNG* src, FILE* fptr)
{
	if ((src == NULL) || (fptr == NULL))
		return -1;

	static const uint8_t PNG_SIGNATURE[8] = {
		0x89, 0x50, 0x4E, 0x47,
		0x0D, 0x0A, 0x1A, 0x0A,
	};
	struct png_tag* tag;
	struct png_tag buffer;

	fwrite(PNG_SIGNATURE, sizeof(PNG_SIGNATURE), 1, fptr);
	for (size_t i = 0; i < src->tag_count; ++i) {
		tag = &src->tags[i];
		buffer = *tag;
		buffer.hdr.len = htonl(buffer.hdr.len);

		fwrite(&buffer.hdr, sizeof(buffer.hdr), 1, fptr);
		if (tag->hdr.len > 0) {
			fwrite(buffer.data, sizeof(uint8_t), tag->hdr.len, fptr);
		}
		fwrite(&buffer.crc, sizeof(buffer.crc), 1, fptr);
	}

	return 0;
}

int png_remove_exif_tag(PNG* png)
{
	static const char EXIF_TAG_SIGNATURE[4] = {0x65, 0x58, 0x49, 0x66};
	struct png_tag empty_tag = {0};
	struct png_tag* tag;

	if (png == NULL)
		return -1;

	for (size_t i = 0; i < png->tag_count; ++i) {
		tag = &png->tags[i];

		if (pngtcmp(&tag->hdr, &EXIF_TAG_SIGNATURE) == 0) {
			if (tag->hdr.len > 0)
				free(tag->data);
			
			*tag = empty_tag;

			memmove(&png->tags[i], &png->tags[i+1], sizeof(*tag) * (png->tag_count - (i + 1)));
		}
	}

	return 0;
}

int png_file_erase_exif(FILE* fptr)
{
	static const char EXIF_TAG_SIGNATURE[4] = {0x65, 0x58, 0x49, 0x66};
	static const uint8_t PNG_SIGNATURE[8] = {
		0x89, 0x50, 0x4E, 0x47,
		0x0D, 0x0A, 0x1A, 0x0A,
	};

	struct png_tag_header buffer;
	struct stat st;
	uint8_t* data_buffer;
	long pos;
	long deleted;

	if (fstat(fileno(fptr), &st) != 0) {
		perror("stat");
		return 1;
	}

	deleted = 0;
	pos = 0;
	data_buffer = mmap(NULL, st.st_size, (PROT_WRITE | PROT_READ), MAP_SHARED, fileno(fptr), 0);

	if (memcmp(&data_buffer[pos], PNG_SIGNATURE, sizeof(PNG_SIGNATURE)) != 0)
		return 1;
	pos += 8;

	while (pos < st.st_size) {
		long noffset;

		buffer = *(struct png_tag_header*)&data_buffer[pos];
		buffer.len = ntohl(buffer.len);
		noffset = pos + (sizeof(uint32_t) * 3) + buffer.len;

		if (pngtcmp(&buffer, &EXIF_TAG_SIGNATURE) == 0) {
			deleted = (sizeof(uint32_t) * 3) + buffer.len;
			memmove(&data_buffer[pos], &data_buffer[noffset], st.st_size - noffset);
			msync(data_buffer, st.st_size - deleted, MS_SYNC);
			continue;
		}

		pos = noffset;
	}

	munmap(data_buffer, st.st_size);

	ftruncate(fileno(fptr), st.st_size - deleted);

	return 0;
}

