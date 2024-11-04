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
	AVFormatContext	*pFormatCtx;   // 存储输入或输出格式的上下文
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;    // 编解码器上下文
	AVCodec			*pCodec;       // 编解码器
	AVFrame	*pFrame,*pFrameYUV;    // AVFrame 是 FFmpeg 中用于存储一帧解码后数据的结构体
	unsigned char *out_buffer;
	AVPacket *packet;              // 存储一帧压缩编码数据
	int y_size;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;  // 图像格式转换上下文

	char filepath[]="../video/bigbuckbunny_480x272.h265";
	//SDL---------------------------
	int screen_w=0,screen_h=0;
	SDL_Window *screen;            // SDL 窗口
	SDL_Renderer* sdlRenderer;     // SDL 渲染器
	SDL_Texture* sdlTexture;       // SDL 纹理
	SDL_Rect sdlRect;              // SDL 矩形

	FILE *fp_yuv;

	// ffmpeg 4.0 以后，av_register_all() 和 avformat_network_init() 不再需要
	// av_register_all();
	// avformat_network_init();

	// 初始化格式上下文
	pFormatCtx = avformat_alloc_context();

	// 打开输入视频文件
	if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return -1;
	}

	// 获取视频信息
	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}

	// 获取视频流索引
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

	// 获取视频解码器
	pCodecCtx=pFormatCtx->streams[videoindex]->codec;
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL){
		printf("Codec not found.\n");
		return -1;
	}

	// 打开解码器
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}
	
	// 初始化AVFrame
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
	
	// 初始化SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 

	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	//SDL 2.0 Support for multiple windows
	// 创建窗口
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,
		SDL_WINDOW_OPENGL);

	if(!screen) {  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}

	// 创建渲染器
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)

	/*
		SDL_Texture* SDL_CreateTexture(SDL_Renderer* renderer,  
                               Uint32 format,  
                               int access,  
                               int w,  
                               int h);

		SDL_Renderer* renderer：
			这是一个指向 SDL_Renderer 结构的指针，
			它代表了用于绘制纹理的渲染器。渲染器是与特定窗口关联的，并且负责将纹理（和其他图形内容）渲染到该窗口上。
			在你的代码片段中，sdlRenderer 应该是一个之前已经通过 SDL_CreateRenderer 函数创建的渲染器。
		Uint32 format：
			这指定了纹理的像素格式。SDL_PIXELFORMAT_IYUV 是一种特定的像素格式，它用于存储YUV颜色空间的图像数据。
			YUV颜色空间通常用于视频处理，因为它允许对亮度和色度信息进行独立编码，这对于压缩和传输视频数据非常有用。
			然而，需要注意的是，并非所有的SDL后端和硬件都支持 SDL_PIXELFORMAT_IYUV。如果你的系统不支持这种格式，
			SDL_CreateTexture 可能会失败。
		int access：
			这指定了纹理的访问模式。SDL_TEXTUREACCESS_STREAMING 表示纹理将用于流式更新，即纹理的内容将在渲染过程中频繁更改。
			这与 SDL_TEXTUREACCESS_STATIC（内容不变）和 SDL_TEXTUREACCESS_TARGET（用作渲染目标）等其他访问模式相对。
		int w, int h：
			这两个参数指定了纹理的宽度和高度（以像素为单位）。在你的代码片段中，这些值是从 pCodecCtx（一个指向编解码器上下文的指针）中获取的，
			这意味着纹理的大小与解码后的视频帧的大小相匹配。
	*/
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);  

	sdlRect.x=0;
	sdlRect.y=0;
	sdlRect.w=screen_w;
	sdlRect.h=screen_h;

	// 读取一帧数据
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

				// 更新纹理
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
			// 解码一帧视频数据，保存在pFrame中
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
			if(got_picture){
				// 转换图像格式，保存在pFrameYUV中
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
				// 更新纹理
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

