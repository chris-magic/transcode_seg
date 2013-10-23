/*
 * segment_yy.c
 *
 *  Created on: Oct 16, 2012
 *      Author: chris
 */
#include <stdio.h>

#include "libswscale/swscale.h"
#include "segment_yy.h"
#include "segment_utils.h"
#include "chris_error.h"
#include "chris_global.h"
#include "output_handle.h"

int init_seg_union(Segment_U ** segment_union ,int argc ,char * argv[]) {   //传递是指针变量的地址

	if ((*segment_union = malloc(sizeof(Segment_U))) == NULL) {
		printf("segment union malloc failed .\n");
		exit(MEMORY_MALLOC_FAIL);
	}

	Segment_U * seg_union =(*segment_union);
	memset(seg_union ,0 ,sizeof(Segment_U));

	seg_union->mode_type = -1;
	seg_union->segment_no = 0;

	/*	parse option */
	parse_option_argument(seg_union ,argc ,argv);

	/*	create directory	*/
	create_directory(seg_union->storage_dir);

	/* create m3u8 file */
	create_m3u8_name(seg_union);
	/*	splice some information	*/
	create_first_ts_name(seg_union ,seg_union->mode_type);


	printf("--------------->before transcode init function  ,seg_union->ts_name = %s..\n" ,seg_union->ts_name);
	printf("seg_union->full_m3u8_name = %s \n" ,seg_union->full_m3u8_name);
	/*	initialize the output file information*/
	av_register_all();
	avformat_network_init();
	//malloc memory
	if( (seg_union->input_ctx = malloc (sizeof(Input_Context))) == NULL){

		printf("ptr_input_ctx malloc failed .\n");
		exit(MEMORY_MALLOC_FAIL);
	}
	malloc_input_memory(seg_union->input_ctx);

	if( (seg_union->output_ctx = malloc (sizeof(Output_Context))) == NULL){

		printf("ptr_output_ctx malloc failed .\n");
		exit(MEMORY_MALLOC_FAIL);
	}



	if( strcmp( seg_union->vcodec ,"libx264") == 0){
		seg_union->output_ctx->vcodec_copy_mark = 0;
		printf("video encode in libx264 ,need reencode ..\n");
		seg_union->output_ctx->frame_rate				=		 seg_union->frame_rate;						 //frame rate
		seg_union->output_ctx->width					=		 seg_union->width;							//video width
		seg_union->output_ctx->height					=		 seg_union->height;							//video height
		seg_union->output_ctx->video_rate				=		 seg_union->video_rate;						//video bitrate

	}else if(strcmp( seg_union->vcodec ,"copy") == 0){
		seg_union->output_ctx->vcodec_copy_mark = 1;
	}


	//user can control input ,must before init_output function
	seg_union->output_ctx->audio_rate				=		 seg_union->audio_rate;						//audio bitrate
	seg_union->output_ctx->sample					=		 seg_union->sample;							//audio sample
	seg_union->output_ctx->channel					=		 seg_union->channel;						//audio channels

	/*initialize input file information*/
	init_input( seg_union->input_ctx ,seg_union->input_file[0]);  //only support one mp4 file

	//initialize the output context
	init_output(seg_union->output_ctx ,seg_union->ts_name ,seg_union->input_ctx);
	//open video and audio ,set video_out_buf and audio_out_buf
	open_stream_codec(seg_union->output_ctx);
	printf("--------------->after transcode init function ..\n");

	/*segment element in output context*/
	seg_union->output_ctx->segment_duration 		=		 seg_union->segment_duration;
	seg_union->output_ctx->segment_no				= 		 seg_union->segment_no;
	seg_union->output_ctx->dir_name_len 		    = 		 seg_union->dir_name_len;
	seg_union->output_ctx->ts_prfix_name 			= 		 seg_union->ts_prfix_name;

	seg_union->output_ctx->ts_name 					=		 seg_union->ts_name;
	seg_union->output_ctx->full_m3u8_name			=		 seg_union->full_m3u8_name;
	seg_union->output_ctx->mode_type				=		 seg_union->mode_type;

	return 0;
}


int seg_transcode_main(Segment_U * seg_union){

	Output_Context *ptr_output_ctx = seg_union->output_ctx;

    if (!(ptr_output_ctx->fmt->flags & AVFMT_NOFILE)) {		//for mp4 or mpegts ,this must be performed
        if (avio_open(&(ptr_output_ctx->ptr_format_ctx->pb), seg_union->ts_name, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "Could not open '%s'\n", seg_union->ts_name);
            exit(OPEN_MUX_FILE_FAIL);
        }
    }
    // write the stream header, if any
    avformat_write_header(ptr_output_ctx->ptr_format_ctx ,NULL);
    write_m3u8_header(ptr_output_ctx);
  //start

	Input_Context *ptr_input_ctx = seg_union->input_ctx;

	ptr_output_ctx->img_convert_ctx = sws_getContext(
			ptr_input_ctx->video_codec_ctx->width ,ptr_input_ctx->video_codec_ctx->height ,PIX_FMT_YUV420P,
			 ptr_output_ctx->video_stream->codec->width ,ptr_output_ctx->video_stream->codec->height ,PIX_FMT_YUV420P ,
			 SWS_BICUBIC ,NULL ,NULL ,NULL);


	printf("src_width = %d ,src_height = %d \n" ,ptr_input_ctx->video_codec_ctx->width ,ptr_input_ctx->video_codec_ctx->height);
	printf("dts_width = %d ,dts_height = %d \n" ,ptr_output_ctx->video_stream->codec->width ,
			ptr_output_ctx->video_stream->codec->height);

	printf("before av_read_frame ...\n");
	/*************************************************************************************/
	/*decoder loop*/
	//
	//
	//***********************************************************************************/
	while(av_read_frame(ptr_input_ctx->ptr_format_ctx ,&ptr_input_ctx->pkt) >= 0){

		/*	video stream 	*/
		if (ptr_input_ctx->pkt.stream_index == ptr_input_ctx->video_index) {

			/*** libx264 reencode	****/
			if (seg_union->output_ctx->vcodec_copy_mark == 0){ //libx264 reencode

				//decode video packet
				int got_picture = 0;
				avcodec_decode_video2(ptr_input_ctx->video_codec_ctx,
						ptr_input_ctx->yuv_frame, &got_picture,
						&ptr_input_ctx->pkt);

				if (got_picture) {
					//encode video (decoded video data in ptr_input_ctx->yuv_frame)
					//input stream 的问题。
					ptr_output_ctx->sync_ipts =
							av_q2d(
									ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->time_base)
									* (ptr_input_ctx->yuv_frame->best_effort_timestamp)
									- (double) ptr_input_ctx->ptr_format_ctx->start_time
											/ AV_TIME_BASE
									+ ptr_output_ctx->base_ipts; //current packet time in second

					//printf("ptr_output_ctx->sync_ipts = %f \n" ,ptr_output_ctx->sync_ipts);

					//first swscale
					sws_scale(ptr_output_ctx->img_convert_ctx,
							(const uint8_t* const *) ptr_input_ctx->yuv_frame->data,
							ptr_input_ctx->yuv_frame->linesize, 0,
							ptr_input_ctx->video_codec_ctx->height,
							ptr_output_ctx->encoded_yuv_pict->data,
							ptr_output_ctx->encoded_yuv_pict->linesize);

					//second swscale
					encode_video_frame(ptr_output_ctx,
							ptr_output_ctx->encoded_yuv_pict, ptr_input_ctx);
				} //end if(got_picture)


			/***********	video codec copy	****************/
			}else if(seg_union->output_ctx->vcodec_copy_mark == 1){ /*video codec copy*/
				AVBitStreamFilterContext *bsfc = ptr_input_ctx->bitstream_filters;
			    while (bsfc) {
			        AVPacket new_pkt = ptr_input_ctx->pkt;
			        int a = av_bitstream_filter_filter(bsfc, ptr_output_ctx->video_stream->codec, NULL,
			                                           &new_pkt.data, &new_pkt.size,
			                                           ptr_input_ctx->pkt.data, ptr_input_ctx->pkt.size,
			                                           ptr_input_ctx->pkt.flags & AV_PKT_FLAG_KEY);
			        if(a == 0 && new_pkt.data != ptr_input_ctx->pkt.data && new_pkt.destruct) {
			        	//printf("a = 0\n");
			        	uint8_t *t = av_malloc(new_pkt.size + FF_INPUT_BUFFER_PADDING_SIZE); //the new should be a subset of the old so cannot overflow
			            if(t) {
			                memcpy(t, new_pkt.data, new_pkt.size);
			                memset(t + new_pkt.size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
			                new_pkt.data = t;
			                new_pkt.buf = NULL;
			                a = 1;
			            } else //error
			                a = AVERROR(ENOMEM);
			        }
			        if (a > 0) {
			        	//printf("a > 0\n");
			            av_free_packet(&ptr_input_ctx->pkt);
			            new_pkt.buf = av_buffer_create(new_pkt.data, new_pkt.size,
			                                           av_buffer_default_free, NULL, 0);
			            if (!new_pkt.buf)
			                exit(1);
			        } else if (a < 0) {
			            av_log(NULL, AV_LOG_ERROR, "Failed to open bitstream filter %s for stream %d ",
			                   bsfc->filter->name, ptr_input_ctx->pkt.stream_index);
			            exit(1);
			        }
			        //ptr_input_ctx->pkt = new_pkt;
			        ptr_output_ctx->pkt = new_pkt;
			        bsfc = bsfc->next;  //break while
			    }

#if 1		/*if this branch ,i feel this stream timestamp may be have some bug?????*/
			    //alter pts for packet
			    ptr_output_ctx->pkt.pts = av_rescale_q(ptr_output_ctx->pkt.pts ,
			    							ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->time_base ,
			    							seg_union->output_ctx->video_stream->time_base );
			    //alter dts for packet
			    ptr_output_ctx->pkt.dts = av_rescale_q(ptr_output_ctx->pkt.dts ,
			    							ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->time_base ,
			    							seg_union->output_ctx->video_stream->time_base );
#else
			    ptr_output_ctx->pkt.pts = ptr_output_ctx->frame_count ++;
			    //alter pts for packet
			    ptr_output_ctx->pkt.pts = av_rescale_q(ptr_output_ctx->pkt.pts ,
			    							seg_union->output_ctx->video_stream->codec->time_base ,
			    							seg_union->output_ctx->video_stream->time_base );

				ptr_output_ctx->pkt.dts = ptr_output_ctx->pkt.pts;
#endif
			    //judge if key frame or not
				if(ptr_output_ctx->pkt.flags & AV_PKT_FLAG_KEY){
					//init segment_time
					record_segment_time(ptr_output_ctx);
				}
			    av_write_frame(ptr_output_ctx->ptr_format_ctx, &ptr_output_ctx->pkt);
			}

		/*	audio stream 	*/
		} else if (ptr_input_ctx->pkt.stream_index == ptr_input_ctx->audio_index) {
			//printf("audio ...\n");
			//decode audio packet
			uint8_t *tmp_data = ptr_input_ctx->pkt.data;
			int tmp_size = ptr_input_ctx->pkt.size;
			while (ptr_input_ctx->pkt.size > 0) {
				int got_frame = 0;
				int len = avcodec_decode_audio4(ptr_input_ctx->audio_codec_ctx,
						ptr_input_ctx->audio_decode_frame, &got_frame,
						&ptr_input_ctx->pkt);

				if (len < 0) { //decode failed ,skip frame
					fprintf(stderr, "Error while decoding audio frame\n");
					break;
				}

				if (got_frame) {
					//encode the audio data ,and write the data into the output
					do_audio_out(ptr_output_ctx ,ptr_input_ctx ,ptr_input_ctx->audio_decode_frame);
				} else { //no data
					printf("======>avcodec_decode_audio4 ,no data ..\n");
					continue;
				}

				ptr_input_ctx->pkt.size -= len;
				ptr_input_ctx->pkt.data += len;

			}

			//renew
			ptr_input_ctx->pkt.size = tmp_size;
			ptr_input_ctx->pkt.data = tmp_data;

		}

		if(&ptr_input_ctx->pkt)
			av_free_packet(&ptr_input_ctx->pkt);

	}//endwhile
	avio_flush(ptr_output_ctx->ptr_format_ctx->pb);
	avio_close(ptr_output_ctx->ptr_format_ctx->pb);
	double file_duration = ptr_input_ctx->ptr_format_ctx->duration / AV_TIME_BASE
				+ (double)( ptr_input_ctx->ptr_format_ctx->duration % AV_TIME_BASE ) / AV_TIME_BASE;


	ptr_output_ctx->base_ipts  += file_duration;  //completed files sum time duration
	printf("end while ......,time_base = %f .............> \n" ,ptr_output_ctx->base_ipts  );
	ptr_output_ctx->audio_resample = 0;

	free_input(ptr_input_ctx);
	sws_freeContext(ptr_output_ctx->img_convert_ctx);
 //end

	printf("before flush ,ptr_output_ctx->ptr_format_ctx->nb_streams = %d \n\n" ,ptr_output_ctx->ptr_format_ctx->nb_streams);
	encode_flush(ptr_output_ctx ,ptr_output_ctx->ptr_format_ctx->nb_streams);

	write_m3u8_tailer(ptr_output_ctx);
	printf("before wirite tailer ...\n\n");
	av_write_trailer(ptr_output_ctx->ptr_format_ctx );

	return 0;
}


int free_seg_union(Segment_U * seg_union){
	printf("start free segment union ...\n");

	//free input context relevance
	free_input_memory(seg_union->input_ctx);

	if(seg_union->input_ctx)
		free(seg_union->input_ctx);

	//free output context relevance
	free_output_memory(seg_union->output_ctx);
	if(seg_union->output_ctx)
		free(seg_union->output_ctx);
	//at last
	if(seg_union)
		free(seg_union);
	return 0;
}
