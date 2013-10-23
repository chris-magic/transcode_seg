/*
 * output_handle.c
 *
 *  Created on: Sep 24, 2012
 *      Author: chris
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "libavutil/samplefmt.h"
#include "libavutil/opt.h"


#include "output_handle.h"
#include "input_handle.h"

#include "chris_error.h"
#include "chris_global.h"

#include "segment_utils.h"

//use to copy video stream
AVStream *add_output_stream(AVFormatContext *output_format_context, AVStream *input_stream) {
    AVCodecContext *input_codec_context;
    AVCodecContext *output_codec_context;
    AVStream *output_stream;

    output_stream = avformat_new_stream(output_format_context, 0);
    if (!output_stream) {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }

    input_codec_context = input_stream->codec;
    output_codec_context = output_stream->codec;

    output_codec_context->codec_id = input_codec_context->codec_id;
    output_codec_context->codec_type = input_codec_context->codec_type;
    output_codec_context->codec_tag = input_codec_context->codec_tag;
    output_codec_context->bit_rate = input_codec_context->bit_rate;
    output_codec_context->extradata = input_codec_context->extradata;
    output_codec_context->extradata_size = input_codec_context->extradata_size;

    if(av_q2d(input_codec_context->time_base) * input_codec_context->ticks_per_frame > av_q2d(input_stream->time_base) && av_q2d(input_stream->time_base) < 1.0/1000) {
        //in here
    	output_codec_context->time_base = input_codec_context->time_base;
        output_codec_context->time_base.num *= input_codec_context->ticks_per_frame;
    }
    else {
    	//printf("else \n");
        output_codec_context->time_base = input_stream->time_base;
    }
//    printf("time_base = %f \n"  ,1/av_q2d(output_codec_context->time_base));
//    while(1);
    switch (input_codec_context->codec_type) {
        case AVMEDIA_TYPE_VIDEO:
            output_codec_context->pix_fmt = input_codec_context->pix_fmt;
            output_codec_context->width = input_codec_context->width;
            output_codec_context->height = input_codec_context->height;
            output_codec_context->has_b_frames = input_codec_context->has_b_frames;

            if (output_format_context->oformat->flags & AVFMT_GLOBALHEADER) {
                output_codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;
            }
            break;
    default:
        break;
    }

    return output_stream;
}
//参考了output-example.c
static AVStream * add_video_stream (AVFormatContext *fmt_ctx ,enum AVCodecID codec_id ,Output_Context *ptr_output_ctx){

	AVCodecContext *avctx;
	AVStream *st;

	//add video stream
	st = avformat_new_stream(fmt_ctx ,NULL);
	if(st == NULL){
		printf("in out file ,new video stream fimplicit dailed ..\n");
		exit(NEW_VIDEO_STREAM_FAIL);
	}

	//set the index of the stream
	st->id = ID_VIDEO_STREAM;

	//set AVCodecContext of the stream
	avctx = st->codec;

	avctx->codec_id = codec_id;
	avctx->codec_type = AVMEDIA_TYPE_VIDEO;

	//resolution
	avctx->width = ptr_output_ctx->width;//VIDEO_WIDTH;
	avctx->height = ptr_output_ctx->height;//VIDEO_HEIGHT;

	//set bit rate
	avctx->bit_rate = ptr_output_ctx->video_rate;//VIDEO_BIT_RATE;
	avctx->rc_max_rate = ptr_output_ctx->video_rate;//VIDEO_BIT_RATE;
	avctx->rc_min_rate = ptr_output_ctx->video_rate;//VIDEO_BIT_RATE;
	avctx->bit_rate_tolerance = ptr_output_ctx->video_rate;//VIDEO_BIT_RATE;
	avctx->rc_buffer_size = ptr_output_ctx->video_rate;//VIDEO_BIT_RATE;
	avctx->rc_initial_buffer_occupancy = avctx->rc_buffer_size * 3 / 4;
	avctx->rc_buffer_aggressivity = (float)1.0;
	avctx->rc_initial_cplx = 0.5;



	avctx->pix_fmt = PIX_FMT_YUV420P;
	avctx->me_range = 24;
	avctx->qcompress = 0.6;
	avctx->qmin = 10;
	avctx->qmax = 51;
	avctx->max_qdiff = 4;


	avctx->time_base.den = ptr_output_ctx->frame_rate;//VIDEO_FRAME_RATE;
	avctx->time_base.num = 1;

	//key frame
	avctx->keyint_min = ptr_output_ctx->frame_rate;//VIDEO_FRAME_RATE;
	avctx->scenechange_threshold = 0;
	avctx->gop_size = ptr_output_ctx->frame_rate;//VIDEO_FRAME_RATE;

	//other
	avctx->global_quality = 6;

	avctx->thread_count = 1;
//	avctx->cqp = 26;
	avctx->refs = 3;
	avctx->trellis = 2;

	avctx->me_method = 8;
	avctx->me_range = 16;
	avctx->me_subpel_quality = 7;
	avctx->qmin = 10;
	avctx->qmax = 51;
	avctx->rc_initial_buffer_occupancy = 0.9;
	avctx->i_quant_factor = 1.0/1.40f;  //
	avctx->b_quant_factor = 1.30f;	//值越大 B frame 劣化越严重
	avctx->chromaoffset = 0;
	avctx->max_qdiff = 4;
	avctx->qcompress = 0.6f;		//affect mbtree
	avctx->qblur = 0.5f;
	avctx->noise_reduction = 0;
	avctx->scenechange_threshold = 40;

	avctx->flags |= CODEC_FLAG_LOOP_FILTER;
	avctx->me_cmp = FF_CMP_CHROMA;

	avctx->level = 21;
	avctx->profile = FF_PROFILE_H264_MAIN;   // do not effective
	// some formats want stream headers to be separate(for example ,asfenc.c ,but not mpegts)
	if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		avctx->flags |= CODEC_FLAG_GLOBAL_HEADER;

	return st;
}



static AVStream * add_audio_stream (AVFormatContext *fmt_ctx ,enum AVCodecID codec_id ,Output_Context *ptr_output_ctx){
	AVCodecContext *avctx;
	AVStream *st;

	//add video stream
	st = avformat_new_stream(fmt_ctx ,NULL);
	if(st == NULL){
		printf("in out file ,new video stream failed ..\n");
		exit(NEW_VIDEO_STREAM_FAIL);
	}

	//set the index of the stream
	st->id = 1;

	//set AVCodecContext of the stream
	avctx = st->codec;
	avctx->codec_id = codec_id;
	avctx->codec_type = AVMEDIA_TYPE_AUDIO;

	avctx->sample_fmt = AV_SAMPLE_FMT_S16;
	avctx->bit_rate = ptr_output_ctx->audio_rate;//AUDIO_BIT_RATE;
	printf("ptr_output_ctx->audio_rate= %d................\n" ,ptr_output_ctx->audio_rate);
	avctx->sample_rate = ptr_output_ctx->sample;//AUDIO_SAMPLE_RATE;//ptr_input_ctx->audio_codec_ctx->sample_rate/*44100*/;

	avctx->channel_layout = AV_CH_LAYOUT_STEREO;
	avctx->channels = av_get_channel_layout_nb_channels(avctx->channel_layout);
	printf("avctx->channels = %d \n" ,avctx->channels);
//	avctx->channels = ptr_output_ctx->channel;//2;

	// some formats want stream headers to be separate(for example ,asfenc.c ,but not mpegts)
	if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		avctx->flags |= CODEC_FLAG_GLOBAL_HEADER;

	return st;
}

int init_output(Output_Context *ptr_output_ctx, char* output_file ,Input_Context *input_ctx){

	//set AVOutputFormat
    /* allocate the output media context */
	printf("output_file = %s \n" ,output_file);
    avformat_alloc_output_context2(&ptr_output_ctx->ptr_format_ctx, NULL, NULL, output_file);
    if (ptr_output_ctx->ptr_format_ctx == NULL) {
        printf("Could not deduce[推断] output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&ptr_output_ctx->ptr_format_ctx, NULL, "mpeg", output_file);
        if(ptr_output_ctx->ptr_format_ctx == NULL){
        	 printf("Could not find suitable output format\n");
        	 exit(NOT_GUESS_OUT_FORMAT);
        }
    }
    //in here ,if I get AVOutputFormat succeed ,the filed audio_codec and video_codec will be set default.
    ptr_output_ctx->fmt = ptr_output_ctx->ptr_format_ctx->oformat;


    /* add audio stream and video stream 	*/
    ptr_output_ctx->video_stream = NULL;
    ptr_output_ctx->audio_stream = NULL;

    ptr_output_ctx->audio_codec_id = CODEC_ID_AAC; //aac
    ptr_output_ctx->video_codec_id = CODEC_ID_H264; //h264

    if (ptr_output_ctx->fmt->video_codec != CODEC_ID_NONE) {

    	if(ptr_output_ctx->vcodec_copy_mark == 0){ //libx264 reencode
        	ptr_output_ctx->video_stream = add_video_stream(ptr_output_ctx->ptr_format_ctx, ptr_output_ctx->video_codec_id ,ptr_output_ctx);

    	}else if(ptr_output_ctx->vcodec_copy_mark == 1){ //format
    		ptr_output_ctx->video_stream = add_output_stream(ptr_output_ctx->ptr_format_ctx, input_ctx->ptr_format_ctx->streams[input_ctx->video_index]);
    	}

    	if(ptr_output_ctx->video_stream == NULL){
    		printf("in output ,add video stream failed \n");
    		exit(ADD_VIDEO_STREAM_FAIL);
    	}

    }

    if (ptr_output_ctx->fmt->audio_codec != CODEC_ID_NONE) {

    	ptr_output_ctx->audio_stream = add_audio_stream(ptr_output_ctx->ptr_format_ctx, ptr_output_ctx->audio_codec_id ,ptr_output_ctx);
    	if(ptr_output_ctx->audio_stream == NULL){
    		printf(".in output ,add audio stream failed \n");
    		exit(ADD_AUDIO_STREAM_FAIL);
    	}
    }


    /*	malloc buffer	*/
    ptr_output_ctx->encoded_yuv_pict = avcodec_alloc_frame();
    if(ptr_output_ctx->encoded_yuv_pict == NULL){
		printf("yuv_frame allocate failed %s ,%d line\n" ,__FILE__ ,__LINE__);
		exit(MEMORY_MALLOC_FAIL);
	}
    int size = avpicture_get_size(ptr_output_ctx->video_stream->codec->pix_fmt ,
    		ptr_output_ctx->video_stream->codec->width ,
    		ptr_output_ctx->video_stream->codec->height);

    printf("size = %d ,width = %d \n" ,size ,ptr_output_ctx->video_stream->codec->width );
    ptr_output_ctx->pict_buf = av_malloc(size);
    if(ptr_output_ctx->pict_buf == NULL){
    	printf("pict allocate failed ...\n");
    	exit(MEMORY_MALLOC_FAIL);
    }
    //bind
    avpicture_fill((AVPicture *)ptr_output_ctx->encoded_yuv_pict ,ptr_output_ctx->pict_buf ,
    				ptr_output_ctx->video_stream->codec->pix_fmt ,
    		    		ptr_output_ctx->video_stream->codec->width ,
    		    		ptr_output_ctx->video_stream->codec->height);


    /*	init some member value */
    ptr_output_ctx->audio_resample = 0;
    ptr_output_ctx->swr = NULL;
    ptr_output_ctx->base_ipts = 0;
    //segment time
    ptr_output_ctx->prev_segment_time = 0.0;
    ptr_output_ctx->curr_segment_time = 0.0;
    ptr_output_ctx->start_time_mark = 0;
    ptr_output_ctx->frame_count = 0;

    /*output the file information */
    av_dump_format(ptr_output_ctx->ptr_format_ctx, 0, output_file, 1);

    //fifo
    ptr_output_ctx->fifo = av_fifo_alloc(1024);
	if (!ptr_output_ctx->fifo) {
		exit (1);
	}
	av_log(NULL, AV_LOG_WARNING ,"--av_fifo_size(ost->fifo) = %d \n" ,av_fifo_size(ptr_output_ctx->fifo));  //输出是0？！


	return 0;
}


//===========================================================
static void open_video (Output_Context *ptr_output_ctx ,AVStream * st){

	AVCodec *video_encode;
	AVCodecContext *video_codec_ctx;

	video_codec_ctx = st->codec;

	//find video encode
	video_encode = avcodec_find_encoder(video_codec_ctx->codec_id);
	if(video_encode == NULL){
		printf("in output ,open_video ,can not find video encode.\n");
		exit(NO_FIND_VIDEO_ENCODE);
	}

	AVDictionary *opts = NULL;

//	av_dict_set(&opts, "profile", "baseline", 0);
	av_dict_set(&opts, "profile", "high", 0);
	//open video encode
	if(avcodec_open2(video_codec_ctx ,video_encode ,&opts/*NULL*/) < 0){

		printf("in open_video function ,can not open video encode.\n");
		exit(OPEN_VIDEO_ENCODE_FAIL);
	}

	//set video encoded buffer
	ptr_output_ctx->video_outbuf = NULL;
	if (!(ptr_output_ctx->ptr_format_ctx->oformat->flags & AVFMT_RAWPICTURE)) {//in ffmpeg,only nullenc and yuv4mpeg have this flags
																		//so ,mp4 and mpegts both go in here
		printf(".....malloc video buffer ...\n");
		ptr_output_ctx->video_outbuf_size = VIDEO_OUT_BUF_SIZE;
		ptr_output_ctx->video_outbuf = av_malloc(ptr_output_ctx->video_outbuf_size);
		if(ptr_output_ctx->video_outbuf == NULL){
			printf("video_outbuf malloc failed ...\n");
			exit(MEMORY_MALLOC_FAIL);
		}
	}


}

static void open_audio (Output_Context *ptr_output_ctx ,AVStream * st){

	AVCodec *audio_encode;
	AVCodecContext *audio_codec_ctx;

	audio_codec_ctx = st->codec;

	//find audio encode
	audio_encode = avcodec_find_encoder(audio_codec_ctx->codec_id);
	if(audio_encode == NULL){
		printf("in output ,open_audio ,can not find audio encode.\n");
		exit(NO_FIND_AUDIO_ENCODE);
	}

//    //add acc experimental
//    audio_codec_ctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL; //if not set ,the follow codec can not perform

	//open audio encode
	if(avcodec_open2(audio_codec_ctx ,audio_encode ,NULL) < 0){

		printf("in open_audio function ,can not open audio encode.\n");
		exit(OPEN_AUDIO_ENCODE_FAIL);
	}

	ptr_output_ctx->audio_outbuf_size = AUDIO_OUT_BUF_SIZE;
	ptr_output_ctx->audio_outbuf = av_malloc(ptr_output_ctx->audio_outbuf_size);
	if (ptr_output_ctx->audio_outbuf == NULL) {
		printf("audio_outbuf malloc failed ...\n");
		exit(MEMORY_MALLOC_FAIL);
	}

    /* ugly hack for PCM codecs (will be removed ASAP with new PCM
       support to compute the input frame size in samples */
	int audio_input_frame_size;
    if (audio_codec_ctx->frame_size <= 1) {
    	audio_input_frame_size = ptr_output_ctx->audio_outbuf_size / audio_codec_ctx->channels;
    	printf("&&$$&&#&#&#&&#&#&#&&#\n\n");
    	sleep(10);
        switch(st->codec->codec_id) {
        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
            audio_input_frame_size >>= 1;
            break;
        default:
            break;
        }
    } else {
        audio_input_frame_size = audio_codec_ctx->frame_size;
    }
    //ptr_output_ctx->samples = av_malloc(audio_input_frame_size * 2 * audio_codec_ctx->channels);

}


void open_stream_codec(Output_Context *ptr_output_ctx){

	if(ptr_output_ctx->vcodec_copy_mark == 0){ //use libx264 codec reencode
		open_video (ptr_output_ctx ,ptr_output_ctx->video_stream);
	}

	open_audio (ptr_output_ctx ,ptr_output_ctx->audio_stream);

}

void encode_video_frame(Output_Context *ptr_output_ctx, AVFrame *pict,
		Input_Context *ptr_input_ctx ) {

	static int frame_count = 0;
	int nb_frames;
	double sync_ipts;
	double duration = 0;

	/* compute the duration */
	duration =
			FFMAX(av_q2d(ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->time_base),
							av_q2d(ptr_input_ctx->video_codec_ctx->time_base));

	if (ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->avg_frame_rate.num)
		duration = FFMAX(duration, 1/av_q2d(ptr_input_ctx->ptr_format_ctx->streams[ptr_input_ctx->video_index]->avg_frame_rate));

	duration /= av_q2d(ptr_output_ctx->video_stream->codec->time_base);


	/*	compute the sync_ipts ,use for to determine duplicate or drop the encode pic*/
	sync_ipts = ptr_output_ctx->sync_ipts / av_q2d(ptr_output_ctx->video_stream->codec->time_base);


    /* by default, we output a single frame ,there is no different in fps of input file  and fps of output file*/
    nb_frames = 1;


    //compute the vdelta ,do not forget the duration
    double vdelta = sync_ipts - frame_count + duration;

	// FIXME set to 0.5 after we fix some dts/pts bugs like in avidec.c
	if (vdelta < -1.1)
		nb_frames = 0;
	else if (vdelta > 1.1)
		nb_frames = lrintf(vdelta);

	//set chris_count
	//set chris_count
	int tmp_count;
	for (tmp_count = 0; tmp_count < nb_frames; tmp_count++) {
		av_init_packet(&ptr_output_ctx->pkt);
		//encode the image
		int video_encoded_out_size;
		pict->pts = frame_count++;
		ptr_output_ctx->frame_count = frame_count;
		int got_output = 0;
		video_encoded_out_size = avcodec_encode_video2(
				ptr_output_ctx->video_stream->codec,
				&ptr_output_ctx->pkt ,
				pict ,
				&got_output);
		if (video_encoded_out_size < 0) {
			fprintf(stderr, "Error encoding video frame\n");
			exit(VIDEO_ENCODE_ERROR);
		}

		if(got_output){

		//	printf("pts = %ld\n" ,ptr_output_ctx->pkt.pts);
			ptr_output_ctx->pkt.stream_index = ptr_output_ctx->video_stream->index;

			if (ptr_output_ctx->pkt.pts
					!= AV_NOPTS_VALUE)
				ptr_output_ctx->pkt.pts = av_rescale_q(
						ptr_output_ctx->pkt.pts ,  //
						ptr_output_ctx->video_stream->codec->time_base,
						ptr_output_ctx->video_stream->time_base);

			//printf("after pts = %ld ,time_base in avcodecContext %f \n" ,ptr_output_ctx->pkt.pts , 1/av_q2d(ptr_output_ctx->video_stream->codec->time_base));
//			printf("time_base in stream %f \n"  ,1/av_q2d(ptr_output_ctx->video_stream->time_base));
#if 1
			//judge if key frame or not
			if(ptr_output_ctx->pkt.flags & AV_PKT_FLAG_KEY){
				//init segment_time
				record_segment_time(ptr_output_ctx);
			}
#endif
			av_write_frame(ptr_output_ctx->ptr_format_ctx, &ptr_output_ctx->pkt);
			av_free_packet(&ptr_output_ctx->pkt);
		}

	}
}

void encode_audio_frame(Output_Context *ptr_output_ctx , uint8_t *buf ,int buf_size){

	int ret;
	AVCodecContext *c = ptr_output_ctx->audio_stream->codec;


	//packet for output
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	//frame for input
	AVFrame *frame = avcodec_alloc_frame();
	if (frame == NULL) {
		printf("frame malloc failed ...\n");
		exit(1);
	}

	static unsigned int audio_count = 0;

	frame->nb_samples = buf_size /
					(c->channels * av_get_bytes_per_sample(c->sample_fmt));

	frame->pts = audio_count ;
	audio_count += frame->nb_samples;


	if ((ret = avcodec_fill_audio_frame(frame, c->channels, AV_SAMPLE_FMT_S16,
				buf, buf_size, 1)) < 0) {
		av_log(NULL, AV_LOG_FATAL, ".Audio encoding failed\n");
		exit(AUDIO_ENCODE_ERROR);
	}
	int got_packet = 0;
	if (avcodec_encode_audio2(c, &pkt, frame, &got_packet) < 0) {
		av_log(NULL, AV_LOG_FATAL, "..Audio encoding failed\n");
		exit(AUDIO_ENCODE_ERROR);
	}

	if(got_packet){
		if (pkt.pts != AV_NOPTS_VALUE) {

			pkt.pts = av_rescale_q(pkt.pts,
										ptr_output_ctx->audio_stream->codec->time_base,
										ptr_output_ctx->audio_stream->time_base);

		}
		pkt.dts = pkt.pts;		//audio packet dts set method
		pkt.stream_index = ptr_output_ctx->audio_stream->index;
		av_write_frame(ptr_output_ctx->ptr_format_ctx, &pkt);
		av_free_packet(&pkt);
	}

	av_free(frame);

}



/*	add silence audio data	*/
static void generate_silence(uint8_t* buf, enum AVSampleFormat sample_fmt, size_t size)
{
    int fill_char = 0x00;
    if (sample_fmt == AV_SAMPLE_FMT_U8)
        fill_char = 0x80;
    memset(buf, fill_char, size);
}

void encode_flush(Output_Context *ptr_output_ctx , int nb_ostreams){

	int i ;

	for (i = 0; i < nb_ostreams; i++){

		AVStream *st = ptr_output_ctx->ptr_format_ctx->streams[i];
		AVCodecContext *enc = st->codec;
		int stop_encoding = 0;

		for (;;){
            AVPacket pkt;
            int fifo_bytes;
            av_init_packet(&pkt);
            pkt.data = NULL;
            pkt.size = 0;

            switch (st->codec->codec_type) {
            /*audio stream*/
            case AVMEDIA_TYPE_AUDIO:
            {


                    fifo_bytes = av_fifo_size(ptr_output_ctx->fifo);
                    if (fifo_bytes > 0) {
                            /* encode any samples remaining in fifo */
                            int frame_bytes = fifo_bytes;
                            av_fifo_generic_read(ptr_output_ctx->fifo, ptr_output_ctx->audio_buf, fifo_bytes, NULL);

                            /* pad last frame with silence if needed */
                            frame_bytes = enc->frame_size * enc->channels *
                                                      av_get_bytes_per_sample(enc->sample_fmt);
                            if (ptr_output_ctx->allocated_audio_buf_size < frame_bytes)
                                    exit(1);
                            generate_silence(ptr_output_ctx->audio_buf+fifo_bytes, enc->sample_fmt, frame_bytes - fifo_bytes);

                            printf("audio ...........\n");
                            encode_audio_frame(ptr_output_ctx, ptr_output_ctx->audio_buf, frame_bytes);

                    } else {
                            /* flush encoder with NULL frames until it is done
                               returning packets */
                            int got_packet = 0;
                            int ret1;
                            ret1 = avcodec_encode_audio2(enc, &pkt, NULL, &got_packet);
                            if ( ret1 < 0) {
                                    av_log(NULL, AV_LOG_FATAL, "..Audio encoding failed\n");
                                    exit(AUDIO_ENCODE_ERROR);
                            }

                            printf("audio ...........\n");
                            if (ret1 == 0){
                                    stop_encoding = 1;
                                    break;
                            }
                            pkt.pts = 0;
                            pkt.stream_index = ptr_output_ctx->audio_stream->index;

                            av_write_frame(ptr_output_ctx->ptr_format_ctx, &pkt);

                            av_free(&pkt);
                    }

                    break;

            }
            /*video stream*/
            case AVMEDIA_TYPE_VIDEO:
            {
            	if(ptr_output_ctx->vcodec_copy_mark == 0){ //libx264 reencode
					int got_output = 0;
					int nEncodedBytes = avcodec_encode_video2(
							ptr_output_ctx->video_stream->codec, &pkt, NULL,
							&got_output);

					if (nEncodedBytes < 0) {
						av_log(NULL, AV_LOG_FATAL, "Video encoding failed\n");
						exit(VIDEO_FLUSH_ERROR);
					}

					printf("video ...........\n");
					if (got_output) {
						pkt.stream_index = ptr_output_ctx->video_stream->index;

						if (ptr_output_ctx->video_stream->codec->coded_frame->pts
								!= AV_NOPTS_VALUE)
							pkt.pts = ptr_output_ctx->frame_count++;
						pkt.pts = av_rescale_q(pkt.pts,
								ptr_output_ctx->video_stream->codec->time_base,
								ptr_output_ctx->video_stream->time_base);

						if (ptr_output_ctx->video_stream->codec->coded_frame->key_frame)
							pkt.flags |= AV_PKT_FLAG_KEY;

						//judge key frame
						if (ptr_output_ctx->pkt.flags && AV_PKT_FLAG_KEY) {
							//init segment_time
							record_segment_time(ptr_output_ctx);

						}

						av_write_frame(ptr_output_ctx->ptr_format_ctx, &pkt); //segmentation fault???????

						av_free_packet(&pkt);
					} else if (nEncodedBytes == 0) {
						stop_encoding = 1;
						break;
					}
            	}
                break;
            }
			default:
				stop_encoding = 1;
				break;
			}//end switch

			if(stop_encoding) break;

			if(ptr_output_ctx->vcodec_copy_mark == 1){ //video codec copy
				printf("complete the %d.ts ,and write the m3u8 file..\n" ,ptr_output_ctx->segment_no);
				write_m3u8_body( ptr_output_ctx ,ptr_output_ctx->curr_segment_time - ptr_output_ctx->prev_segment_time ,1);
				break;
			}

		}//end for


	}


}

void do_audio_out(Output_Context *ptr_output_ctx ,Input_Context *ptr_input_ctx ,AVFrame *decoded_frame){

    AVCodecContext *enc = ptr_output_ctx->audio_stream->codec;
    //AVCodecContext *dec = ptr_input_ctx->audio_codec_ctx;

	static uint8_t  **dst_data = NULL;
	static int dst_linesize;
	static int dst_nb_samples, max_dst_nb_samples;  //only init once time
	static int dst_nb_channels ;
	if(ptr_output_ctx->swr == NULL){  // make sure ,this operation perform only once time
		ptr_output_ctx->swr = swr_alloc();
		if (!ptr_output_ctx->swr) {
			printf("swr context failed ...\n");
			exit(1);
		}

		av_opt_set_int(ptr_output_ctx->swr, "in_channel_layout", decoded_frame->channel_layout, 0);
		av_opt_set_int(ptr_output_ctx->swr, "in_sample_rate", decoded_frame->sample_rate, 0);
		av_opt_set_sample_fmt(ptr_output_ctx->swr, "in_sample_fmt", decoded_frame->format, 0);

		av_opt_set_int(ptr_output_ctx->swr, "out_channel_layout", enc->channel_layout, 0);
		av_opt_set_int(ptr_output_ctx->swr, "out_sample_rate", enc->sample_rate, 0);
		av_opt_set_sample_fmt(ptr_output_ctx->swr, "out_sample_fmt", enc->sample_fmt, 0);

		if (swr_init(ptr_output_ctx->swr) < 0) {
			av_log(NULL, AV_LOG_FATAL, "swr_init() failed\n");
			swr_free(&ptr_output_ctx->swr);
		}

		/* compute the number of converted samples: buffering is avoided
		 * ensuring that the output buffer will contain at least all the
		 * converted input samples */
		max_dst_nb_samples = dst_nb_samples = av_rescale_rnd(
				decoded_frame->nb_samples, enc->sample_rate, decoded_frame->sample_rate, AV_ROUND_UP);
		/* buffer is going to be directly written to a rawaudio file, no alignment */
		dst_nb_channels = av_get_channel_layout_nb_channels(enc->channel_layout);
		int ret = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize,
				dst_nb_channels, dst_nb_samples, enc->sample_fmt, 0);

		if (ret < 0) {
			fprintf(stderr, "Could not allocate destination samples\n");
			exit(1);
		}

	}
	int ret1 = swr_convert(ptr_output_ctx->swr, dst_data, dst_nb_samples,
			(const uint8_t **)ptr_input_ctx->audio_decode_frame->data,
			ptr_input_ctx->audio_decode_frame->nb_samples);

	int dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
			ret1, enc->sample_fmt, 1);

	if (av_fifo_realloc2(ptr_output_ctx->fifo,
			av_fifo_size(ptr_output_ctx->fifo) + dst_bufsize) < 0) {
		av_log(NULL, AV_LOG_FATAL, "av_fifo_realloc2() failed\n");
		exit(1);
	}
	av_fifo_generic_write(ptr_output_ctx->fifo, dst_data[0], dst_bufsize, NULL);

	int frame_bytes = ptr_output_ctx->audio_stream->codec->frame_size
			* av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * dst_nb_channels;

	ptr_output_ctx->audio_buf = malloc(frame_bytes);
	while (av_fifo_size(ptr_output_ctx->fifo) >= frame_bytes) {
		av_fifo_generic_read(ptr_output_ctx->fifo, ptr_output_ctx->audio_buf,
				frame_bytes, NULL);
		encode_audio_frame(ptr_output_ctx, ptr_output_ctx->audio_buf,
				frame_bytes);
	}

	free(ptr_output_ctx->audio_buf);
	//swr_free(&ptr_output_ctx->swr);
}

void free_output_memory(Output_Context *ptr_output_ctx){


	//malloc in open_video
	av_free(ptr_output_ctx->video_outbuf);

	//malloc in open_audio
	av_free(ptr_output_ctx->audio_outbuf);

	//audio buffer
	av_fifo_free(ptr_output_ctx->fifo);

	av_free(ptr_output_ctx->pict_buf);

	av_free(ptr_output_ctx->encoded_yuv_pict);

	//close codecs
	avcodec_close(ptr_output_ctx->video_stream->codec);
	avcodec_close(ptr_output_ctx->audio_stream->codec);




	//free
	avformat_free_context(ptr_output_ctx->ptr_format_ctx);



}
