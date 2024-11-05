#include <stdio.h>
#include <iostream>
using namespace std;   

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <SDL2/SDL.h>
};

//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit=0;
int thread_pause=0;

int sfp_refresh_thread(void *opaque){
	thread_exit=0;
	thread_pause=0;

	while (!thread_exit) {
		if(!thread_pause){
			SDL_Event event;
			event.type = SFM_REFRESH_EVENT;
			SDL_PushEvent(&event);
		}
		SDL_Delay(40);
	}
	thread_exit=0;
	thread_pause=0;
	//Break
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}



int main(int argc, char* argv[]) {
    
    // 创建封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    // 创建解码器上下文
    AVCodecContext *pCodecCtx = NULL;

    // 创建解码器
    AVCodec *pCodec = NULL;

    // 创建帧
    AVFrame *pFrame = NULL;
    AVFrame *pFrameYUV = NULL;

    // 创建packet
    AVPacket *packet = NULL;

    //创建视频转换上下文
    struct SwsContext *img_convert_ctx = NULL;

    unsigned char *out_buffer = NULL;

    char *filepath = "../video/Titanic.ts";

    //------------SDL----------------
	int screen_w,screen_h;
	SDL_Window *screen; 		  // SDL ����
	SDL_Renderer* sdlRenderer;    // SDL ��Ⱦ��
	SDL_Texture* sdlTexture;      // SDL ����
	SDL_Rect sdlRect;             // SDL ����
	SDL_Thread *video_tid;        // SDL ��Ƶ��׽�߳�
	SDL_Event event;			  // SDL �¼�

    // 打开文件
    if(avformat_open_input(&pFormatCtx, filepath, NULL, NULL) < 0) {
        printf("Couldn't open file.\n");
        return -1;
    }

    // 获取文件信息
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        printf("Couldn't find stream information.\n");
        return -1;
    }

    // 查找视频流索引
    int video_stream_index = -1;
    for(int i = 0; i < pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if(video_stream_index == -1) {
        printf("Didn't find a video stream.\n");
        return -1;
    }

    // 获取编解码器参数
    pCodecCtx = pFormatCtx->streams[video_stream_index]->codec;

    // 查找解码器
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec == NULL) {
        printf("Codec not found.\n");
        return -1;
    }

    // 打开解码器
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec.\n");
        return -1;
    }

    // 分配AVFrame
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();

    if(pFrame == NULL || pFrameYUV == NULL) {
        printf("Could not allocate video frame.\n");
        return -1;
    }

    // 分配缓冲区
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, 
                                            pCodecCtx->width, pCodecCtx->height, 1);

    out_buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));

    // 填充缓冲区
    av_image_fill_arrays(
                          pFrameYUV->data,
                          pFrameYUV->linesize,
                          out_buffer,
                          AV_PIX_FMT_YUV420P,
                          pCodecCtx->width, 
                          pCodecCtx->height,
                          1
                        );

    // 初始化SWSContext
    img_convert_ctx = sws_getContext(pCodecCtx->width,
                                     pCodecCtx->height,
                                     pCodecCtx->pix_fmt,
                                     pCodecCtx->width,
                                     pCodecCtx->height,
                                     AV_PIX_FMT_YUV420P,
                                     SWS_BICUBIC,
                                     NULL,
                                     NULL,
                                     NULL
                                    );

    // 初始化SDL
    // 初始化SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	}

    //SDL 2.0 Support for multiple windows
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,SDL_WINDOW_OPENGL);

	if(!screen) {  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}

	//	创建渲染器
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)

	// 	创建纹理
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);  

	sdlRect.x=0;
	sdlRect.y=0;
	sdlRect.w=screen_w;
	sdlRect.h=screen_h;

	// 初始化SDL音频
	packet=(AVPacket *)av_malloc(sizeof(AVPacket));

    // 创建刷新线程
	video_tid = SDL_CreateThread(sfp_refresh_thread,NULL,NULL);
	//------------SDL End------------
	//Event Loop

    while(1) {
		SDL_WaitEvent(&event);
        if(event.type == SFM_REFRESH_EVENT) {

            while(1) {
                // 读取帧数据，解码显示
                if(av_read_frame(pFormatCtx, packet) < 0) {
                     thread_exit=1;
                }
                   
                if(packet->stream_index == video_stream_index){
                    break;
                }

                av_packet_unref(packet);
            }
            
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


                sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, 
                            pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, 
                            pFrameYUV->linesize
                            );

                // SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
                // pFrameYUV->data[0], pFrameYUV->linesize[0],
                // pFrameYUV->data[1], pFrameYUV->linesize[1],
                // pFrameYUV->data[2], pFrameYUV->linesize[2]);

                SDL_UpdateTexture( sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0] );	
                
                SDL_RenderClear( sdlRenderer );  
                SDL_RenderCopy( sdlRenderer, sdlTexture,  NULL, &sdlRect);  
                SDL_RenderPresent( sdlRenderer );  
            }
            
            av_packet_unref(packet);
            av_frame_unref(pFrame);
            
        } else if(event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym==SDLK_SPACE) {
                thread_pause=!thread_pause;
            }
				

        } else if(event.type == SDL_QUIT) {
            thread_exit=1;
        } else if(event.type == SFM_BREAK_EVENT) {
            break;
        }
	}

    sws_freeContext(img_convert_ctx);
    SDL_Quit();

    av_frame_free(&pFrame);
    av_frame_free(&pFrameYUV);
    av_packet_free(&packet);

    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}