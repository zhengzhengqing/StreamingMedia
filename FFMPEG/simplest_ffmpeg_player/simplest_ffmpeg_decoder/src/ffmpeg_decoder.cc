#include <stdio.h>
#include <iostream>
#include <unistd.h>

using namespace std;

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

// 新版本ffmpeg api

int main(int argc, char *argv[]) {

    const char *filename = "../video/Titanic.mkv";
    FILE *pFile = fopen("out_put.yuv", "wb+");
    if(pFile == NULL) {
        cout << "Could not open output file" << endl;
        return -1;
    }

    // 初始化多媒体格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    if(pFormatCtx == NULL) {
        cout << "Could not allocate memory for format context" << endl;
        return -1;
    }

    // 打开多媒体文件
    if(avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0) {
        cout << "Could not open file" << endl;
        return -1;
    }

    // 获取多媒体文件信息
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        cout << "Could not find stream information" << endl;
        return -1;
    }

    // 查找视频流
    int videoStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO,
                                                -1, -1, NULL, 0);   

    if(videoStreamIndex == -1) {
        cout << "Could not find video stream" << endl;
        return -1;
    }

    //获取解码器
    AVCodecContext *pCodecCtx = pFormatCtx->streams[videoStreamIndex]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    // 打开解码器
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        cout << "Could not open codec" << endl;
        return -1;
    }

    // 分配帧和YUV帧
    AVFrame *frame = av_frame_alloc();
    AVFrame *frame_yuv = av_frame_alloc();

    if(frame == NULL || frame_yuv == NULL) {
        cout << "Could not allocate frame" << endl;
        return -1;
    }

    // 分配YUV帧缓冲区
    uint8_t *out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, 
                                                pCodecCtx->width, pCodecCtx->height, 1));
    av_image_fill_arrays(frame_yuv->data, frame_yuv->linesize, out_buffer,AV_PIX_FMT_YUV420P,
                                                pCodecCtx->width, pCodecCtx->height, 1);

    // 用于图像缩放和像素格式转换的上下文
    // 允许你在视频处理过程中将图像从一个分辨率和像素格式转换到另一个分辨率和像素格式
    /*
    struct SwsContext *sws_getContext(  
        int srcW,               // 源图像的宽度  
        int srcH,               // 源图像的高度  
        enum AVPixelFormat srcFormat,  // 源图像的像素格式  
        int dstW,               // 目标图像的宽度  
        int dstH,               // 目标图像的高度  
        enum AVPixelFormat dstFormat,  // 目标图像的像素格式  
        int flags,              // 转换标志，指定使用的算法和其他选项  
        SwsFilter *srcFilter,   // 源图像滤波器，通常设置为 NULL  
        SwsFilter *dstFilter,   // 目标图像滤波器，通常设置为 NULL  
        const double *param     // 额外的参数，通常设置为 NULL  
    );
    */
    struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                            pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                            SWS_BILINEAR, NULL, NULL, NULL);
    
    // 保存未解码的帧
    AVPacket packet;
    while(av_read_frame(pFormatCtx, &packet) >= 0) {
        if(packet.stream_index == videoStreamIndex) {
            int ret = avcodec_send_packet(pCodecCtx, &packet); // 向解码器发送待解码的数据包
            if(ret < 0) {
                cout << "Error sending a packet for decoding" << endl;
                return -1;
            }

            while(ret >= 0) {
                ret = avcodec_receive_frame(pCodecCtx, frame); // 从解码器接收解码后的帧
                if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if(ret < 0) {
                    cout << "Error during decoding" << endl;
                    return -1;
                }

                /*
                int sws_scale(  
                    struct SwsContext *c,               // 缩放和转换上下文  
                    const uint8_t *const srcSlice[],    // 源图像数据切片数组  
                    const int srcStride[],              // 源图像每行的字节数（stride）数组  
                    int srcSliceY,                      // 源图像中要转换的起始行的Y坐标  
                    int srcSliceH,                      // 源图像中要转换的行数（高度）  
                    uint8_t *const dst[],               // 目标图像数据切片数组  
                    const int dstStride[]               // 目标图像每行的字节数（stride）数组  
                );

                struct SwsContext *c:
                    这是一个指向 SwsContext 结构体的指针，该结构体包含了执行缩放和转换所需的所有上下文信息。这个上下文通常是通过调用 sws_getContext 函数创建的。
                const uint8_t *const srcSlice[]:
                    这是一个指向 uint8_t* 常量指针数组的指针，每个指针指向源图像的一个平面（例如，对于 YUV420P 格式，会有 Y、U 和 V 三个平面）。注意，虽然这里使用了二级指针（指针的指针），但实际上它通常被当作一个指向固定大小数组的指针来使用，数组的大小取决于源图像的像素格式。
                const int srcStride[]:
                    这是一个整数数组，每个元素对应源图像一个平面的 stride（每行的字节数）。stride 通常大于或等于图像的宽度（以像素为单位）乘以每个像素的字节数，以容纳行对齐或其他格式要求。
                int srcSliceY:
                    这是源图像中要转换的起始行的 Y 坐标（垂直偏移）。它指定了从源图像的哪一行开始转换。
                int srcSliceH:
                    这是源图像中要转换的行数（高度）。它指定了要转换的源图像区域的高度。
                uint8_t *const dst[]:
                    这是一个指向 uint8_t* 指针数组的指针，每个指针指向目标图像的一个平面。与源图像数据类似，目标图像数据也被组织为平面数组。
                const int dstStride[]:
                    这是一个整数数组，每个元素对应目标图像一个平面的 stride。与源图像的 stride 数组类似，它指定了目标图像每个平面的每行字节数。
                */

                sws_scale(sws_ctx, (const uint8_t * const *)frame->data, frame->linesize, 0, 
                            pCodecCtx->height, frame_yuv->data, frame_yuv->linesize);
                
                // 写入YUV420P格式的帧数据
                fwrite(frame_yuv->data[0], 1, pCodecCtx->width * pCodecCtx->height, pFile);
                fwrite(frame_yuv->data[1], 1, pCodecCtx->width * pCodecCtx->height / 4 , pFile);
                fwrite(frame_yuv->data[2], 1, pCodecCtx->width * pCodecCtx->height / 4, pFile);

                // fwrite(frame_yuv->data[0], 1, frame_yuv->linesize[0] * pCodecCtx->height, pFile);
                // fwrite(frame_yuv->data[1], 1, frame_yuv->linesize[1] * pCodecCtx->height / 2 , pFile);
                // fwrite(frame_yuv->data[2], 1, frame_yuv->linesize[2] * pCodecCtx->height / 2, pFile);
                
                
            }
        }
        av_packet_unref(&packet); // 释放packet 所指向的缓冲区,并不是释放packet本身
    }

    // 释放资源
    // av_frame_free(&frame) 只释放 AVFrame 结构体本身，而不释放 frame->data 指向的缓冲区。
    // 你需要根据缓冲区的来源和分配方式来确定如何正确地释放它们。
    av_frame_free(&frame);
    av_frame_free(&frame_yuv);

    av_free(out_buffer);
    

    sws_freeContext(sws_ctx);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    fclose(pFile);
    
    
    return 1;
}