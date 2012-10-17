/*
 * segment_utils.h
 *
 *  Created on: Oct 16, 2012
 *      Author: chris
 */

#ifndef SEGMENT_UTILS_H_
#define SEGMENT_UTILS_H_

#include "segment_yy.h"
/*
 * function :parse_option
 *
 * */
void parse_option_argument( Segment_U * segment_union, int argc, char *argv[]);


/*
 *
 * function :create storage if not exist
 * */
void create_directory(char *storage_name);


#endif /* SEGMENT_UTILS_H_ */
