/*
 * segment_main.c
 *
 *  Created on: Oct 16, 2012
 *      Author: chris
 */

#include <stdio.h>

#include "chris_global.h"
#include "chris_error.h"
#include "segment_yy.h"

int main(int argc ,char * argv[]){

	/*Segment union */
	Segment_U * seg_union = NULL;
	init_seg_union(&seg_union ,argc ,argv);

	printf("input mod %d \n" ,seg_union->mode_type);
	printf("input dir %s \n" ,seg_union->storage_dir);
	printf("input seg_union->ts_prfix_name %s \n" ,seg_union->ts_prfix_name);
	printf("input ts_prfix_name %s \n" ,seg_union->ts_prfix_name);
	printf("input ->m3u8_name %s \n" ,seg_union->m3u8_name);
	printf("input ->segment_duration %f \n" ,seg_union->segment_duration);


	/*	segmentation medium	*/
	//main function
	seg_transcode_main(seg_union);


	return 0;
}


