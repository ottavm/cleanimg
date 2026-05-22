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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <locale.h>
#include <limits.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "png.h"

#define AUTHOR "Murilo Ottávio A. Branco Reis"
#define FILENAME_SUFFIX "clean"

enum {
	MODE_ERASE,
	MODE_REMOVE,
};

enum {
	FORMAT_PNG,
};

struct args {
	int mode;
	int format;
	char *output;
	char **files;
	int file_count;
};

static int verbose;

void print_version()
{
	printf("cleanimg version %s\n", VERSION);
}

void print_copyright()
{
	printf("Copyright (C) 2026 %s.\n", AUTHOR);
    printf("Homepage: <%s>, bugreport: <%s>\n", PACKAGE_URL, PACKAGE_BUGREPORT);
	printf("Licensed under the BSD 3-Clause License.\n");
	printf("See: https://opensource.org/license/bsd-3-clause\n");
	printf("\nWritten by %s.\n", AUTHOR);
}

void print_help(char* bin)
{
	/* If the executable binary `bin` is not set, set it to the default
	 * value 'cleanimg'. */
	bin = (bin == NULL) ? "cleanimg" : bin;
	printf("%s (cleanimg) USAGE: %s [options] [arguments] [images]\n", bin, bin);
	printf("DESCRIPTION:\n");
	printf("\t\"%s\" deletes or erase (see --mode) the EXIF (eXIf) metadata tag from a "                          \
			"supported image file format. (see --format)\n", bin);
	printf("\tSupported file formats are (as version %s): PNG\n", VERSION);
	printf("\tcleanimg can operate in two different \"modes\": erase mode or remove mode. "                       \
			"Erase mode removes the EXIF tag from an image by overwriting its contents,\n"                        \
			"changing its structure permanently. Otherwise, in remove mode, \n");
	printf("\tcleanimg will copy the file image data (as well the tags) to an internal "                          \
			"buffer, and then removes its EXIF tag, keeping the original file intact.\n");
	printf("\tIn the latter mode, if '--output=OUTPUT' is not specified or while operating multiple "             \
		"files, cleanimg will use the original image file path, appending "                                       \
		"the suffix '%s.<format>' to it.\n", FILENAME_SUFFIX);
	printf("OPTIONS:\n");
	puts("\t--output=[OUTPUT/PATH] | -o [OUTPUT/PATH]: Sets the output image path. Notes: Only avaliable\n\t"     \
		"when not working with multiple files and with the remove mode, otherwise, it's ignored. Default\n\t"     \
		"value: ");  
	puts("\t--mode=[remove,erase]  | -m [remove,erase]: Sets the mode. \"erase\": removes the eXIf tag by\n\t"    \
		"overwriting the original image. \"remove\": removes the eXIf tag by loading the image in the memory\n\t" \
		"and deleting its eXIf tag. Does not change the original file contents. Default value: remove");
	puts("\t--verbose              | -d: Turn on verbose mode. Default: off.");
	puts("\t--format=[png]         | -f [png]: Sets the image format. Default value: png.");
	puts("\t--help                 | -h: Shows this message.");
	puts("\t--version              | -v: Shows version and copyright notice.");
}

int remove_exif_png(int filec, char** files, char* output)
{
	char* file;
	char output_path[PATH_MAX];
	PNG image;
	FILE* output_file;

	for (int i = 0; i < filec; ++i) {
		file = files[i];
		memset(output_path, 0, sizeof(output_path));

		if ((filec > 1) || (output == NULL)) {	
			snprintf(output_path, sizeof(output_path), "%.*s-%s.png", (int)strlen(file) - 4, file, FILENAME_SUFFIX);
		} else {
			snprintf(output_path, sizeof(output_path), "%s", output);
		}
		
		if (png_read(&image, file) != 0) {
			perror(file);
			continue;
		}
		
		if (verbose)
			printf("INFO: Parsing file \"%s\" -> \"%s\"... ", file, output_path);

		png_remove_exif_tag(&image);

		output_file = fopen(output_path, "w");
		if (output_file == NULL) {
			perror(output_path);
			goto free_img;
		}

		png_write(&image, output_file);
		if (verbose)
			puts("Done.");

		fclose(output_file);
free_img:
		png_free(&image);
	}

	return 0;
}

int erase_exif_png(int filec, char** files)
{
	char* file;
	FILE* fptr;

	for (int i = 0; i < filec; ++i) {
		file = files[i];
		fptr = fopen(file, "r+");
		if (fptr == NULL) {
			perror(file);
			continue;
		}
	
		if (verbose)
			printf("INFO: Erasing file \"%s\"... ", file);

		if (png_file_erase_exif(fptr) != 0) {
			if (verbose)
				puts("Failed. Skipping.");
		} else {
			if (verbose)
				puts("Done.");
		}

		fclose(fptr);
	}

	return 0;
}

int main(int argc, char** argv)
{
	int retval;
	struct args args = {0};
	int option_index;
	int c;

	retval = 0;
	setlocale(LC_ALL, "");

	/* Default values */
	args.format = FORMAT_PNG;
	args.mode = MODE_REMOVE;
	verbose = 0;

	static const struct option options[] = {
		{"help", no_argument, NULL, 'h'},
		{"format", required_argument, NULL, 'f'},
		{"mode", required_argument, NULL, 'm'},
		{"version", no_argument, NULL, 'v'},
		{"output", required_argument, NULL, 'o'},
		{"verbose", no_argument, NULL, 'd'},
		{0, 0, 0, 0},
	};

	while (1) {
		option_index = 0;
		c = getopt_long(argc, argv, "ho:df:m:v", options, &option_index);

		if (c == -1)
			break;
		
		switch (c) {
			case 'h':
				print_help(argv[0]);
				exit(0);
				break;
			case 'f':
				if (strcmp("png", optarg) == 0)
					args.format = FORMAT_PNG;
				else {
					print_help(argv[0]);
					fprintf(stderr, "ERROR: Invalid image format: %s\n", optarg);
					exit(1);
				}
				break;
			case 'm':
				if (strcmp("erase", optarg) == 0)
					args.mode = MODE_ERASE;
				else if (strcmp("remove", optarg) == 0)
					args.mode = MODE_REMOVE;
				else {
					print_help(argv[0]);
					fprintf(stderr, "ERROR: Unknown mode: %s\n", optarg);
					exit(1);
				}
				break;
			case 'o':
				args.output = strdup(optarg);
				if (args.output == NULL) {
					perror("strdup");
					exit(1);
				}
				break;
			case 'd':
				verbose = 1;
				break;
			case 'v':
				print_version();
				printf("\n");
				print_copyright();
				exit(0);
			case '?':
				exit(1);
				break;
			default:
				printf("?? getopt returned character code 0x%X ??\n", c);
				abort();
				break;
		}
	}

	if (argc <= optind) {
		print_help(argv[0]);
		fprintf(stderr, "ERROR: No file specified.\n");
		exit(1);
	}

	args.file_count = argc - optind;
	
	if ((args.output != NULL) && (args.file_count > 1)) {
		print_help(argv[0]);
		fprintf(stderr, "ERROR: --output argument does not support multiple files.\n");
		exit(1);
	}

	args.files = malloc(sizeof(char*) * args.file_count);
	if (args.files == NULL) {
		perror("malloc");
		exit(1);
	}

	for (int i = 0; i < args.file_count; ++i) {
		args.files[i] = strdup(argv[optind + i]);
		if (args.files[i] == NULL) {
			perror("strdup");
			exit(1);
		}
	}

	switch (args.format) {
		case FORMAT_PNG:
			if (args.mode == MODE_REMOVE)
				retval = remove_exif_png(args.file_count, args.files, args.output);
			else if (args.mode == MODE_ERASE)
				retval = erase_exif_png(args.file_count, args.files);
			break;
		default:
			print_help(argv[0]);
			fprintf(stderr, "ERROR: Invalid mode.\n");
			exit(1);
	}	

	/* `args.files` cleanup */
	for (int i = 0; i < args.file_count; ++i) {
		if (args.files[i] != NULL)
			free(args.files[i]);
	}
	free(args.files);

	if (args.output)
		free(args.output);

	return retval;
}

