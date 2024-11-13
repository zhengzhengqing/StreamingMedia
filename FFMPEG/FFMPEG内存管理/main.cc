/*
AVFrame 和 AVPacket 补充

typedef struct AVPacket {
    //如果 buf 不为空，AVPacket 使用引用计数机制来管理其数据缓冲区的生命周期。
    //AVBufferRef 提供引用计数功能，当 AVPacket 不再使用时，引用计数会减少，并在引用计数为 0 时释放内存。
      //如果 buf 为 NULL，则 AVPacket 的数据不使用引用计数管理。这种情况下，内存管理由 AVPacket 本身负责。
    AVBufferRef *buf;
  
    //指向实际的数据缓冲区。data 是一个指针，指向存储音频或视频数据的内存区域。
    //如果 buf 不为空，data 指向的内存由 AVBufferRef 管理。data 只是 AVBufferRef 内部数据的指针，不直接负责内存管理。
    //如果 buf 为空，data 指向的内存可能需要显式释放，通常在不再使用 AVPacket 时释放 data 所指向的内存。
    uint8_t *data;
    int   size;
   
     //提供一个 AVBufferRef，供 API 用户自由使用。FFmpeg 不会检查这个缓冲区的内容，只在 AVPacket 被取消引用时调用 av_buffer_unref() 来处理。
    //用户可以使用 opaque_ref 存储附加的私有数据。
    //FFmpeg 会在 AVPacket 取消引用时调用 av_buffer_unref() 来释放 opaque_ref。
    //在 av_packet_copy_props() 调用时，会创建新的 AVBufferRef 引用到 opaque_ref。
    AVBufferRef *opaque_ref;

} AVPacket;

typedef struct AVFrame {
    //指向包含音频或视频数据的缓冲区。对于视频帧，每个指针可能指向图像的一个平面，对于音频帧，每个指针可能指向一个通道的数据。
    //这些指针应指向在 buf 或 extended_buf 中管理的内存区域。
    //当AVFrame 被释放时，指针指向的内存会通过引用计数机制处理，具体取决于 buf 和 extended_buf 的内容。
    uint8_t *data[AV_NUM_DATA_POINTERS];
  
    //数组用于描述数据平面的每一行的大小。这可能包括额外的填充字节，用于对齐
    int linesize[AV_NUM_DATA_POINTERS];
    //extended_data 提供对 data 数组无法容纳的额外数据的访问。
    //需要注意的是，extended_data 中的指针也应指向在 buf 或 extended_buf 中管理的内存区域。
    uint8_t **extended_data;
  
    //管理data
    AVBufferRef *buf[AV_NUM_DATA_POINTERS];
    //管理extended_data
    AVBufferRef **extended_buf;
    //和AVPacket中的opaque_ref类似，此处不赘述
    AVBufferRef *opaque_ref;

}

*/

#include <iostream>
#include <stdio.h>

extern "C" {

    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libavutil/opt.h>
    #include <libswscale/swscale.h>
    #include <SDL2/SDL.h>
    #include <libavutil/imgutils.h>
}

using namespace std;

#define TEST_AVBUFFER 0
#define TEST_AVFRAME 0
#define TEST_AVPACKET 1

void custom_free(void *opaque, uint8_t *data) {
    printf("Freeing buffer: %p\n", data);
    // 如果有其他的释放逻辑，比如释放 opaque 指针，需要在这里进行
    av_free(data);
}


int main(int argc, char *argv[]) {
    
#if TEST_AVBUFFER
/**
 * AVBuffer 是 FFmpeg 中引用计数的核心结构，用于管理数据缓冲区的生命周期。
 * AVBufferRef 是 AVBuffer 的引用，使用引用计数机制来确保内存被正确释放。
 * 
 * API
 * av_buffer_alloc(size_t size): 分配一个指定大小的缓冲区。
 * AVBufferRef *av_buffer_create(uint8_t *data, size_t size, void (*free)(void *opaque, uint8_t *data),void *opaque, int flags); 已申请的数据使用AVBufferRef来管理
 * av_buffer_ref(AVBufferRef *buf): 增加缓冲区的引用计数。
 * av_buffer_unref(AVBufferRef buf): 释放一个缓冲区的引用，当引用计数为0时，释放缓冲区(该过程是自动的，无需用户管理，类似于智能指针)。
 * 
 * 
 */

    // 1. 使用 av_buffer_alloc 分配一个指定大小的缓冲区
    size_t size = 1024;
    AVBufferRef *buf1 = av_buffer_alloc(size);

    if(!buf1) {
        std::cerr << "Failed to allocate buffer" << std::endl;
        return -1;
    }

    cout << "Buffer allocated, size: " << buf1->size << endl;

    // 2. 使用 av_buffer_create 管理已经申请的缓冲区
    uint8_t *data = (uint8_t*)av_malloc(size); // 手动申请内存
    if(!data) {
        std::cerr << "Failed to allocate data" << std::endl;
        return -1;
    }

    // 使用自定义释放函数来管理 data
    AVBufferRef *buf2 = av_buffer_create(data, size, custom_free, nullptr, 0);

    if(!buf2) {
        std::cerr << "Failed to create buffer" << std::endl;
        av_buffer_unref(&buf1);
        av_free(data);
        return -1;
    }

    // 3. 使用 av_buffer_ref 增加缓冲区的引用计数
    AVBufferRef *buf3 = av_buffer_ref(buf2);

    if(!buf3) {
        std::cerr << "Failed to ref buffer" << std::endl;
        av_buffer_unref(&buf2);
        av_buffer_unref(&buf1);
        return -1;
    }
    cout<<"Buffer refed "<< endl;

    // 4. 释放缓冲区的引用，引用计数减到 0 时，缓冲区被释放
    av_buffer_unref(&buf3); // 引用计数减 1,此时buf2的引用计数为1

    // 5. 释放缓冲区
    cout <<" before unref buf2" << endl;
    av_buffer_unref(&buf2); // 引用计数减 1,此时buf1的引用计数为0。会自动调用 custom_free 释放内存 
    cout <<" after unref buf2" << endl;
    av_buffer_unref(&buf1);

    if(buf1 == nullptr) {
        cout << "buf1 is null after calling av_buffer_unref" << endl;
    }

#endif

#if TEST_AVPACKET
    /**
     * AVPacket 和 AVFrame 是 FFmpeg 中用于处理音视频数据的核心结构。
     * AVPacket 通常用于存储编码的数据包（例如压缩的音视频数据），而 AVFrame 用于存储解码后的原始音视频数据
     * av_packet_alloc 和 av_frame_alloc：用于分配 AVPacket 和 AVFrame 结构。
     * av_new_packet：为 AVPacket 分配指定大小的数据缓冲区。
     * av_frame_get_buffer：为 AVFrame 分配缓冲区，用于存储图像或音频数据。
     * av_packet_ref 和 av_frame_ref：增加引用计数，使多个指针可以安全地共享相同的数据。
     * av_packet_unref 和 av_frame_unref：减少引用计数，当引用计数为 0 时，释放数据缓冲区。
     * av_packet_free 和 av_frame_free：释放 AVPacket 和 AVFrame 结构本身。
     */

    // 1. 分配 AVPacket 结构
    AVPacket *pkt = av_packet_alloc();
    if(!pkt){
        cout << "av_packet_alloc failed" << endl;
        return -1;
    }
    cout << "av_packet_alloc success" << endl;

    // 2. 为 AVPacket 分配数据缓冲区
    int size = 1024;
    if(av_new_packet(pkt, size) < 0){
        cout << "av_new_packet failed" << endl;
        av_packet_free(&pkt);
        return -1;
    } else {
        if(pkt->buf == nullptr){
            cout << "pkt->buf is null " << endl;
        }

        if(pkt->data == nullptr){
            cout << "pkt->data is null " << endl;
        } else {
            cout << "pkt->data is not null ,size = " <<pkt->size << endl;
        }
    }
    cout << "av_new_packet success" << endl;

    // 3. 模拟填充数据
    memset(pkt->data, 111, size);

    // 4. 增加引用计数，pkt_ref和pkt 共享相同的数据
    AVPacket *pkt_ref = av_packet_alloc();
    if(!pkt_ref || av_packet_ref(pkt_ref, pkt) < 0){
        cout << "av_packet_ref failed" << endl;
        av_packet_unref(pkt);
        av_packet_free(&pkt);
        return -1;
    }
    cout << "av_packet_ref success" << endl;

    // 5. 释放引用
    if(pkt_ref->buf != nullptr){
        cout << "pkt_ref->buf is not null" << endl;
    } else {
        cout << "pkt_ref->buf is null" << endl;
    }

    // 地址应该是相同的，也就是说 pkt_ref 和 pkt 共享相同的数据
    printf("pkt_ref->data 指向的地址 = %p\n", pkt_ref->data);
    printf("pkt->data 指向的地址 = %p\n", pkt->data);
    printf("pkt->buf->data 指向的地址 = %p\n", pkt->buf->data);

    av_packet_unref(pkt_ref); //引用计数减1
   

    if(pkt_ref->buf != nullptr){
        cout << "pkt_ref->buf is not null after calling av_packet_unref " << endl;
    } else {
        cout << "pkt_ref->buf is null after calling av_packet_unref " << endl;
    }

    printf("pkt->data 指向的地址 = %p, pkt->data[0] = %d \n", pkt->data, pkt->data[0]);

    // 6.0 释放结构体本身
   
    av_packet_free(&pkt_ref);
    av_packet_free(&pkt); 
#endif

#if TEST_AVFRAME

    // 1. 分配一个AVFrame 
    AVFrame *frame = av_frame_alloc();
    if(!frame){
        cout << "av_frame_alloc failed" << endl;
        return -1;
    }

    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = 640;
    frame->height = 480;

    if(frame->data[0] == nullptr){
        cout << "frame->data is null before " << endl;
    } else {
        cout << "frame->data is not null before" << endl;
    }

    if(frame->buf[0] == nullptr){
        cout << "frame->buf is null before" << endl;
    } else {
        cout << "frame->buf is not null before" << endl;
    }

    // 2. 分配AVFrame的缓冲区
    if(av_frame_get_buffer(frame, 32) < 0) { // 32表示对齐要求
        fprintf(stderr, "Failed to allocate buffer for AVFrame\n");
        av_frame_free(&frame);
        return -1;
    }

    if(frame->data[0] == nullptr){
        cout << "frame->data is null after " << endl;
    } else {
        cout << "frame->data is not null after" << endl;
    }

    if(frame->buf[0] == nullptr){
        cout << "frame->buf is null after" << endl;
    } else {
        cout << "frame->buf is not null after" << endl;
    }

    // 3. 增加引用计数
    AVFrame *frame_ref = av_frame_alloc();
    if(av_frame_ref(frame_ref, frame) < 0) {
        fprintf(stderr, "Failed to allocate buffer for AVFrame\n");
        av_frame_free(&frame);
        return -1;
    }

    // 4. 释放引用
    av_frame_unref(frame_ref);

    if(frame->data[0] == nullptr){
        cout << "frame->data is null 1 " << endl;
    } else {
        cout << "frame->data is not null 1" << endl;
    }

    if(frame->buf[0] == nullptr){
        cout << "frame->buf is null 1" << endl;
    } else {
        cout << "frame->buf is not null 1" << endl;
    }

    // 5. 释放原始帧
    av_frame_unref(frame);

    if(frame->data[0] == nullptr){
        cout << "frame->data is null 2 " << endl;
    } else {
        cout << "frame->data is not null 2" << endl;
    }

    if(frame->buf[0] == nullptr){
        cout << "frame->buf is null 2" << endl;
    } else {
        cout << "frame->buf is not null 2" << endl;
    }

    // 6. 释放AVFrame
    av_frame_free(&frame);
    av_frame_free(&frame_ref);

#endif

    return 0;
}