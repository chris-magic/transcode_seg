#lib_path=/home/chris/work/ffmpeg/refs/ffmpeg_104_transcode/lib
#include_path=/home/chris/work/ffmpeg/refs/ffmpeg_104_transcode/include
lib_path=/home/chris/work/ffmpeg/refs/aac_h264/lib
include_path=/home/chris/work/ffmpeg/refs/aac_h264/include

target=transcode
#src_file = test_main.c	\
#			input_handle.c	\
#			output_handle.c
src_file = segment_main.c	\
			segment_utils.c	\
			segment_yy.c
all:
	gcc -g ${src_file} -o ${target} -I${include_path} -L${lib_path} -lavformat -lavcodec  -lavutil -lswscale -lm -lz -lpthread -lx264 -L./ -lfaac