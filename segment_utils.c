/*
 * segment_utils.c
 *
 *  Created on: Oct 16, 2012
 *      Author: chris
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/types.h>


#include "chris_global.h"
#include "chris_error.h"
#include "segment_yy.h"

void parse_option_argument(Segment_U * seg_union ,int argc, char *argv[]) {

	/*parse options*/
	int next_option;
	static int version_num = 0;
	//short char option
	const char * const short_option = "vhi:m:d:t:p:n:";

	//long char option struction array
	const struct option long_option[] = {
			{ "version", 0, NULL, 'v' },
			{ "help",	0, NULL, 'h' },
			{ "input", 1, NULL, 'i' }, //input file
			{ "mode", 1, NULL, 'm' }, //execute mode
			{ "dir", 1, NULL, 'd' }, //ts storage directory
			{ "segment_time", 1, NULL, 't' }, //segment duration
			{ "prefix_ts", 1, NULL, 'p' }, //the prefix in the m3u8 file
			{ "m3u8_name", 1, NULL, 'n' }, //m3u8 name
			{ NULL, 0, NULL, 0 }
	};

	do {

		next_option = getopt_long(argc, argv, short_option, long_option, NULL);

		switch (next_option) {

		case 'v': 	//version
			printf("**********************************\n");
			printf("  segment version:%s \n\n", SEG_VERSION);
			break;
		case 'h': //help
			fprintf(stderr,
					"\n**********************************\n"
					"--version			view the software version,with no argument\n"
					"--input			assign the input file ,at least one input file,at most 20 input files\n"
					"--mode 			you must assign the program work mode ,you can use 0:transcode ,1:vod ,2:live\n"
					"--dir				the directory where ts and m3u8 file storage\n"
					"--segment_time		the segment duration time in second\n"
					"--prefix_ts		the prefix of the ts file in the m3u8 file \n"
					"--m3u8_name		name of the m3u8 file\n"
					"\n");
			break;
		case 'i': //input file
			if(seg_union->input_nb > MAX_INPUT_NUM){
				printf("input files number is larger than the MAX_INPUT_NUM \n");
				exit(OUT_MAX_INPUT_NUM);
			}
//			strcpy(seg_union->input_file[seg_union->input_nb] ,optarg);
			seg_union->input_file[seg_union->input_nb] = optarg;
			//printf("&optarg = %p \n" ,optarg);
			seg_union->input_nb ++;
			break;
		case 'm': //the program work mode
			printf("mode_type = -%s-\n" ,optarg);
			seg_union->mode_type = atoi(optarg);
			break;
		case 'd': //ts storage directory
			seg_union->storage_dir = optarg;
			break;
		case 't': //segment duration
			seg_union->segment_duration = atof(optarg);
			break;
		case 'p': //the ts file in the m3u8 file
			seg_union->ts_prfix_name = optarg;
			break;
		case 'n': //the m3u8 file
			seg_union->m3u8_name = optarg;
			break;

		case '?':

			printf("  invalid options  ,please use"
					" '%s  --help '  to find some information\n", argv[0]);
			break;
		default:
			break;
		}
	} while (next_option != -1);


	/*	check the input argument valid or not*/
	if( seg_union->mode_type == -1	||
			seg_union->input_nb == 0 ||
			seg_union->m3u8_name == NULL ||
			seg_union->segment_duration == 0 ||
			seg_union->storage_dir == NULL ||
			seg_union->ts_prfix_name == NULL){

		printf("Segment Invalid argument ..\n");
		exit(SEG_INVALID_ARGUMENT);
	}

}

void create_directory(char *storage_dir) {

	DIR *dir = opendir(storage_dir);
	if (dir == NULL) {

		int tmp_ret = mkdir(storage_dir, 0777);
		if (tmp_ret != 0) {
			printf("mkdir  directory failed\n");
			exit(CREAT_M3U8_DIR_FAIL);
		}
	} else {
		closedir(dir);
	}

}

