/*
 * segment_yy.c
 *
 *  Created on: Oct 16, 2012
 *      Author: chris
 */
#include <stdio.h>

#include "chris_error.h"
#include "chris_global.h"
#include "segment_yy.h"

int init_seg_union(Segment_U ** segment_union ,int argc ,char * argv[]) {   //传递是指针变量的地址

	if ((*segment_union = malloc(sizeof(Segment_U))) == NULL) {
		printf("segment union malloc failed .\n");
		exit(MEMORY_MALLOC_FAIL);
	}

	Segment_U * seg_union =(*segment_union);
	memset(seg_union ,0 ,sizeof(Segment_U));

	printf("sizeof(Segment_U) = %d \n" ,sizeof(Segment_U));

	seg_union->mode_type = -1;

	/*	parse option */
	parse_option_argument(seg_union ,argc ,argv);

	/*	create directory	*/
	create_directory(seg_union->storage_dir);

	/*init the input file decoder info*/


	/*init the output file encoder info*/
	return 0;
}


