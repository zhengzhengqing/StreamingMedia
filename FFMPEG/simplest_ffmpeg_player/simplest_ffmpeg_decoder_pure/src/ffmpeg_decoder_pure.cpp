#include <stdio.h>
#include <iostream>
using namespace std;

// 使用新的FFmpeg API
// 一字节对齐


extern "C"
{
	#include <libavcodec/avcodec.h>
}

#define TEST_H264  1
#define TEST_HEVC  0

int main(int argc, char* argv[])
{
	AVCodec *pCodec;
    AVCodecContext *pCodecCtx= NULL;
	AVCodecParserContext *pCodecParserCtx=NULL;

    FILE *fp_in;
	FILE *fp_out;
    AVFrame	*pFrame;
	
	const int in_buffer_size=4096 + 16;
	unsigned char in_buffer[in_buffer_size]={0};
	unsigned char *cur_ptr;
	int cur_size;
    AVPacket packet;
	int ret, got_picture;


#if TEST_HEVC
	enum AVCodecID codec_id=AV_CODEC_ID_HEVC;
	char filepath_in[]="../video/bigbuckbunny_480x272.hevc";
#elif TEST_H264
	AVCodecID codec_id=AV_CODEC_ID_H264;
	char filepath_in[]="../video/bigbuckbunny_480x272.h264";
#else
	AVCodecID codec_id=AV_CODEC_ID_MPEG2VIDEO;
	char filepath_in[]="bigbuckbunny_480x272.m2v";
#endif

	char filepath_out[]="bigbucktest_480x272.yuv";
	int first_time=1;


	// 查找解码器
    pCodec = avcodec_find_decoder(codec_id);
    if (!pCodec) {
        printf("Codec not found\n");
        return -1;
    }

	// 边解码器上下文
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx){
        printf("Could not allocate video codec context\n");
        return -1;
    }

	// av_parser_init()：初始化AVCodecParserContext。其参数是codec_id,所以同时只能解析一种
	pCodecParserCtx=av_parser_init(codec_id);
	if (!pCodecParserCtx){
		printf("Could not allocate video parser context\n");
		return -1;
	}

    
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec\n");
        return -1;
    }
	//Input File
    fp_in = fopen(filepath_in, "rb");
    if (!fp_in) {
        printf("Could not open input stream\n");
        return -1;
    }
	//Output File
	fp_out = fopen(filepath_out, "wb");
	if (!fp_out) {
		printf("Could not open output YUV file\n");
		return -1;
	}

    pFrame = av_frame_alloc();
	av_init_packet(&packet);

	while (1) {

        cur_size = fread(in_buffer, 1, in_buffer_size, fp_in);
        if (cur_size == 0)
            break;
        cur_ptr=in_buffer;

        while (cur_size>0){

			// av_parser_parse2()拿到AVPaket数据，将一个个AVPaket数据解析组成完整的一帧未解码的压缩数据
			// 输入必须是只包含视频编码数据“裸流”（例如H.264、HEVC AAC码流文件），
			// 而不能是包含封装格式的媒体数据（例如AVI、MKV、MP4）。
			int len = av_parser_parse2(
				pCodecParserCtx, pCodecCtx,
				&packet.data, &packet.size,
				cur_ptr , cur_size ,
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

			cur_ptr += len;
			cur_size -= len;

			if(packet.size==0)
				continue;

			//Some Info from AVCodecParserContext
			printf("[Packet]Size:%6d\t",packet.size);
			switch(pCodecParserCtx->pict_type){
				case AV_PICTURE_TYPE_I: printf("Type:I\t");break;
				case AV_PICTURE_TYPE_P: printf("Type:P\t");break;
				case AV_PICTURE_TYPE_B: printf("Type:B\t");break;
				default: printf("Type:Other\t");break;
			}
			printf("Number:%4d\n",pCodecParserCtx->output_picture_number);

			// 向解码器发送压缩的数据包
			int ret = avcodec_send_packet(pCodecCtx, &packet);
			cout <<"ret 1 = " << ret << endl;
			if(ret < 0){
				cout<<"Error sending a packet for decoding"<<endl;
				return -1;
			}

			while(ret >= 0){
				// 从解码器中取出解码后的数据
				ret = avcodec_receive_frame(pCodecCtx, pFrame);
				if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					cout<<"Error receiving a frame for decoding"<<endl;
					break;
				}
					
				else if(ret < 0){
					cout<<"Error during decoding"<<endl;
					return -1;
				}
				cout <<"ret 2 = " << ret << endl;

				if(ret >= 0){
				    //Y, U, V
					for(int i=0;i<pFrame->height;i++){
						fwrite(pFrame->data[0]+pFrame->linesize[0]*i,1,pFrame->width,fp_out);
					}

					for(int i=0;i<pFrame->height/2;i++){
						fwrite(pFrame->data[1]+pFrame->linesize[1]*i,1,pFrame->width/2,fp_out);
					}

					for(int i=0;i<pFrame->height/2;i++){
						fwrite(pFrame->data[2]+pFrame->linesize[2]*i,1,pFrame->width/2,fp_out);
					}

					// 写入YUV420P格式的帧数据, 改写法会导致播放yuv时花屏,经过了解发现是内存对齐导致的
					// fwrite(pFrame->data[0], 1, pCodecCtx->width * pCodecCtx->height, fp_out);
					// fwrite(pFrame->data[1], 1, pCodecCtx->width * pCodecCtx->height / 4, fp_out);
					// fwrite(pFrame->data[2], 1, pCodecCtx->width * pCodecCtx->height / 4, fp_out);
					
				}
			}
			av_packet_unref(&packet);
		}
		
    }


    fclose(fp_in);
	fclose(fp_out);

	av_parser_close(pCodecParserCtx);

	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);

	return 0;
}

