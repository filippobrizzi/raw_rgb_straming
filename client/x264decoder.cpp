#include "x264decoder.h"



static AVFrame * icv_alloc_picture_FFMPEG(int pix_fmt, int width, int height, bool alloc)
{
    AVFrame * picture;
    uint8_t * picture_buf;
    int size;

    picture = avcodec_alloc_frame();
    if (!picture)
        return NULL;
    size = avpicture_get_size( (PixelFormat) pix_fmt, width, height);
    if(alloc) 
    {
        picture_buf = (uint8_t *) malloc(size);
        if (!picture_buf)
        {
            avcodec_free_frame (&picture);
            std::cout << "picture buff = NULL" << std::endl;
            return NULL;
        }
        avpicture_fill((AVPicture *)picture, picture_buf,  (PixelFormat) pix_fmt, width, height);
    }
    return picture;
}


void x264Decoder::initialize(int w = 640, int h = 480)
{     
    avcodec_register_all();  
    image_h_ = h;
    image_w_ = w;

    rgb_size_ = avpicture_get_size(PIX_FMT_RGB24, image_w_, image_h_);
    av_codec_ = avcodec_find_decoder(CODEC_ID_H264); 
    av_codec_context_ = avcodec_alloc_context3(av_codec_);  
    av_codec_context_->width = image_w_;
    av_codec_context_->height = image_h_;
    av_codec_context_->extradata = NULL;
    av_codec_context_->pix_fmt = PIX_FMT_YUV420P;
    avcodec_open2(av_codec_context_, av_codec_, NULL);  
  
    convert_context_ = sws_getContext(image_w_, image_h_, PIX_FMT_YUV420P, image_w_, image_h_, PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    
    av_frame_ = icv_alloc_picture_FFMPEG(PIX_FMT_YUV420P, image_w_, image_h_, true);
    av_frame_RGB_ = icv_alloc_picture_FFMPEG(PIX_FMT_RGB24, image_w_, image_h_, true);

}

bool x264Decoder::decodeFrame(char *src_buffer, unsigned int size, char *rgb_buffer)
{
    AVPacket av_packet;
    av_new_packet(&av_packet, size); 
    av_packet.data = (uint8_t *)src_buffer;
    av_packet.size = size;    

    int frame_finished = 0;
    int av_return = avcodec_decode_video2(av_codec_context_, av_frame_, &frame_finished, &av_packet );
    if(av_return <= 0 || !frame_finished)
        return false;
    //Convert the frame from YUV420 to RGB24
    sws_scale(convert_context_, av_frame_->data, av_frame_->linesize, 0, image_h_, av_frame_RGB_->data, av_frame_RGB_->linesize);   
    //Manadatory function to copy the image form an AVFrame to a generic buffer.
    avpicture_layout((AVPicture *)av_frame_RGB_, PIX_FMT_RGB24, image_w_, image_h_, (unsigned char *)rgb_buffer, rgb_size_);
    
    return true;
}


