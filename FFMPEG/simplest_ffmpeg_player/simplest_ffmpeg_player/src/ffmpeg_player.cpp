#include <iostream>
#include <stdio.h>
using namespace std;

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <SDL2/SDL.h>
	#include <libavutil/imgutils.h>

}

#define OUTPUT_YUV420P 0

int main(int argc, char* argv[])
{
	AVFormatContext	*pFormatCtx;   // �洢����������ʽ��������
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;    // �������������
	AVCodec			*pCodec;       // �������
	AVFrame	*pFrame,*pFrameYUV;    // AVFrame �� FFmpeg �����ڴ洢һ֡��������ݵĽṹ��
	unsigned char *out_buffer;
	AVPacket *packet;              // �洢һ֡ѹ����������
	int y_size;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;  // ͼ���ʽת��������

	char filepath[]="../video/bigbuckbunny_480x272.h265";
	//SDL---------------------------
	int screen_w=0,screen_h=0;
	SDL_Window *screen;            // SDL ����
	SDL_Renderer* sdlRenderer;     // SDL ��Ⱦ��
	SDL_Texture* sdlTexture;       // SDL ����
	SDL_Rect sdlRect;              // SDL ����

	FILE *fp_yuv;

	// ffmpeg 4.0 �Ժ�av_register_all() �� avformat_network_init() ������Ҫ
	// av_register_all();
	// avformat_network_init();

	// ��ʼ����ʽ������
	pFormatCtx = avformat_alloc_context();

	// ��������Ƶ�ļ�
	if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return -1;
	}

	// ��ȡ��Ƶ��Ϣ
	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}

	// ��ȡ��Ƶ������
	videoindex=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++) 
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}

	// ��ȡ��Ƶ������
	pCodecCtx=pFormatCtx->streams[videoindex]->codec;
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL){
		printf("Codec not found.\n");
		return -1;
	}

	// �򿪽�����
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}
	
	// ��ʼ��AVFrame
	pFrame=av_frame_alloc();
	pFrameYUV=av_frame_alloc();
	out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  pCodecCtx->width, pCodecCtx->height,1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize,out_buffer,
		AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height,1);
	
	packet=(AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	av_dump_format(pFormatCtx,0,filepath,0);
	printf("-------------------------------------------------\n");
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 

#if OUTPUT_YUV420P 
    fp_yuv=fopen("output.yuv","wb+");  
#endif  
	
	// ��ʼ��SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 

	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	//SDL 2.0 Support for multiple windows
	// ��������
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,
		SDL_WINDOW_OPENGL);

	if(!screen) {  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}

	// ������Ⱦ��
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)

	/*
		SDL_Texture* SDL_CreateTexture(SDL_Renderer* renderer,  
                               Uint32 format,  
                               int access,  
                               int w,  
                               int h);

		SDL_Renderer* renderer��
			����һ��ָ�� SDL_Renderer �ṹ��ָ�룬
			�����������ڻ����������Ⱦ������Ⱦ�������ض����ڹ����ģ����Ҹ�������������ͼ�����ݣ���Ⱦ���ô����ϡ�
			����Ĵ���Ƭ���У�sdlRenderer Ӧ����һ��֮ǰ�Ѿ�ͨ�� SDL_CreateRenderer ������������Ⱦ����
		Uint32 format��
			��ָ������������ظ�ʽ��SDL_PIXELFORMAT_IYUV ��һ���ض������ظ�ʽ�������ڴ洢YUV��ɫ�ռ��ͼ�����ݡ�
			YUV��ɫ�ռ�ͨ��������Ƶ������Ϊ����������Ⱥ�ɫ����Ϣ���ж������룬�����ѹ���ʹ�����Ƶ���ݷǳ����á�
			Ȼ������Ҫע����ǣ��������е�SDL��˺�Ӳ����֧�� SDL_PIXELFORMAT_IYUV��������ϵͳ��֧�����ָ�ʽ��
			SDL_CreateTexture ���ܻ�ʧ�ܡ�
		int access��
			��ָ��������ķ���ģʽ��SDL_TEXTUREACCESS_STREAMING ��ʾ����������ʽ���£�����������ݽ�����Ⱦ������Ƶ�����ġ�
			���� SDL_TEXTUREACCESS_STATIC�����ݲ��䣩�� SDL_TEXTUREACCESS_TARGET��������ȾĿ�꣩����������ģʽ��ԡ�
		int w, int h��
			����������ָ��������Ŀ�Ⱥ͸߶ȣ�������Ϊ��λ��������Ĵ���Ƭ���У���Щֵ�Ǵ� pCodecCtx��һ��ָ�������������ĵ�ָ�룩�л�ȡ�ģ�
			����ζ������Ĵ�С���������Ƶ֡�Ĵ�С��ƥ�䡣
	*/
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);  

	sdlRect.x=0;
	sdlRect.y=0;
	sdlRect.w=screen_w;
	sdlRect.h=screen_h;

	// ��ȡһ֡����
	while(av_read_frame(pFormatCtx, packet)>=0){

		if(packet->stream_index==videoindex){
			int ret = avcodec_send_packet(pCodecCtx, packet);
			if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
			
			while (ret >= 0) {
				ret = avcodec_receive_frame(pCodecCtx, pFrame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					break;
				} else if (ret < 0) {
					printf("Error during decoding.\n");
					return -1;
				}

				// ��������
				SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
				pFrameYUV->data[0], pFrameYUV->linesize[0],
				pFrameYUV->data[1], pFrameYUV->linesize[1],
				pFrameYUV->data[2], pFrameYUV->linesize[2]);
#endif	
				
				SDL_RenderClear( sdlRenderer );  
				SDL_RenderCopy( sdlRenderer, sdlTexture,  NULL, &sdlRect);  
				SDL_RenderPresent( sdlRenderer );  
				//SDL End-----------------------
				//Delay 40ms
				SDL_Delay(40);
			}
		}


		if(packet->stream_index==videoindex){
			// ����һ֡��Ƶ���ݣ�������pFrame��
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
			if(got_picture){
				// ת��ͼ���ʽ��������pFrameYUV��
				sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
					pFrameYUV->data, pFrameYUV->linesize);
				
#if OUTPUT_YUV420P
				y_size=pCodecCtx->width*pCodecCtx->height;  
				fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y 
				fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
				fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
#endif
				//SDL---------------------------
#if 0
				SDL_UpdateTexture( sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0] );  
#else
				// ��������
				SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
				pFrameYUV->data[0], pFrameYUV->linesize[0],
				pFrameYUV->data[1], pFrameYUV->linesize[1],
				pFrameYUV->data[2], pFrameYUV->linesize[2]);
#endif	
				
				SDL_RenderClear( sdlRenderer );  
				SDL_RenderCopy( sdlRenderer, sdlTexture,  NULL, &sdlRect);  
				SDL_RenderPresent( sdlRenderer );  
				//SDL End-----------------------
				//Delay 40ms
				SDL_Delay(40);
			}
		}
		av_free_packet(packet);
	}

	sws_freeContext(img_convert_ctx);

#if OUTPUT_YUV420P 
    fclose(fp_yuv);
#endif 

	SDL_Quit();

	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

