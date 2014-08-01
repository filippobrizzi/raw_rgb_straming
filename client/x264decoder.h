#ifndef X264DECODER_H
#define X264DECODER_H

#ifdef __cplusplus
#define __STDINT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#include <iostream>
#include <stdint.h>
#include <fstream>

extern "C" {
    #include "x264.h"
    #include <libswscale/swscale.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/frame.h>
}


class x264Decoder{

public:

    void initialize(int w, int h);
    bool decodeFrame(char *src_buffer, unsigned int size, char *rgb_buffer);
private:
    AVFrame* av_frame_;
    AVFrame *av_frame_RGB_;

    // Use this context to convert your BGR Image to YUV image since x264 do not support RGB input
    AVCodec* av_codec_ = NULL;
    AVCodecContext* av_codec_context_ = NULL;
    SwsContext* convert_context_;
    int image_w_;
    int image_h_;
    int rgb_size_;
};


#endif // X264ENCODER_H
