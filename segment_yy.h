/*
 * segment_yy.h
 *
 *  Created on: Oct 16, 2012
 *      Author: chris
 */

#ifndef SEGMENT_YY_H_
#define SEGMENT_YY_H_

#include "input_handle.h"
#include "output_handle.h"

/*	the program execute mode */
#define 	YY_TRANSCODE    0;
#define		YY_VOD 		    1;
#define 	YY_LIVE			2;

typedef struct Segment_U{

	/*input file for segment info*/
	int input_nb;						//input file number
	char *input_file[MAX_INPUT_NUM];					//one or more file

	/*output for segment info */
	int mode_type;						//YY_TRANSCODE ,YY_VOD ,YY_LIVE
	char *storage_dir;					//directory storage the ts file and m3u8 file
	//ts
	double segment_duration;			//every segment duration
	//m3u8
	char *m3u8_name;							//m3u8 name
	char *ts_prfix_name;				//the prefix of the ts in the m3u8 file，if just transcode ,this is the ts file prefix

	/*	input file decode	*/
	INPUT_CONTEXT *input_ctx[MAX_INPUT_NUM];

	/*	output file encode	*/
	INPUT_CONTEXT *output_ctx;

}Segment_U;


/*
 * function	: init the argument of the struct Segment_U
 *
 * */
int init_seg_union(Segment_U ** seg_union , int argc ,char * argv[]);  //in here ,allocate memory ,and in the end there should be a function used to release memory



/*
 * function	: free the memory that allocate in init_seg_union function or others
 *
 * */
int free_seg_union(Segment_U * seg_union);
#endif /* SEGMENT_YY_H_ */
