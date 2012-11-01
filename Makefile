lib_path=/home/chris/work/ffmpeg/refs/aac_h264/lib
include_path=/home/chris/work/ffmpeg/refs/aac_h264/include

target=transcode1
#src_file = test_main.c	\
#			input_handle.c	\
#			output_handle.c
src_file = segment_main.c	\
			segment_utils.c	\
			segment_yy.c	\
			input_handle.c	\
			output_handle.c	
all:
	gcc -g -Wall  ${src_file} -o ${target} -I${include_path} -L${lib_path} -lavformat -lavcodec  -lavutil -lswscale -lswresample -lm -lz -lpthread -lx264 -L./ -lfaac