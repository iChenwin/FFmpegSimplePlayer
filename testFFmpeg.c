#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame);

int main(int argc, char *argv[])
{
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    // open video file
    if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)) {
        fprintf(stderr, "open input failed\n");
        return -1;
    }

    // retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        fprintf(stderr, "find stream info failed\n");
        return -1;
    }

    // dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, argv[1], 0);
    
    // find the video stream
    int videoStream = -1;
    videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (-1 == videoStream) {
        fprintf(stderr, "Can not find video stream!\n");
        return -1;
    }
    
    AVCodecParameters *pCodecPar = NULL;
    pCodecPar = pFormatCtx->streams[videoStream]->codecpar;
    
    // find the decoder for the video stream
    AVCodec *pCodec = NULL;
    pCodec = avcodec_find_decoder(pCodecPar->codec_id);
    if (NULL == pCodec) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }
    
    AVCodecContext *pCodecCtx = NULL;
    pCodecCtx = avcodec_alloc_context3(pCodec);
    // 因为 AVStream::codec 被弃用，AVCodecContext 需要通过 AVCodecParameters 转换得到
    avcodec_parameters_to_context(pCodecCtx, pCodecPar);
    
    //open Codec
    if (avcodec_open2(pCodecCtx, pCodec, NULL)) {
        fprintf(stderr, "Codec open failed!\n");
        return -1;
    }
    
    AVFrame *pFrame = NULL;
    AVFrame *pFrameRGB = NULL;
    // Allocate video frame
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    if (NULL == pFrameRGB || NULL == pFrame) {
        fprintf(stderr, "Alloc frame failed!\n");
        return -1;
    }
    uint8_t *buffer = NULL;
    int numBytes = 0;
    // Determine required buffer size and allocate buffer
    printf("w:%d, h:%d\n", pCodecCtx->width, pCodecCtx->height);
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
    
    printf("buffer size:%d\n", numBytes);
    
    int frameFinished = 0;
    AVPacket packet = {0};
    int i = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {
            // Decode video frame
            int ret = avcodec_send_packet(pCodecCtx, &packet);
            if (0 != ret)
                continue;
            while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                // Convert the image from its native format to RGB
                img_convert((AVPicture *)pFrameRGB, AV_PIX_FMT_RGB24, (AVPicture *)pFrame, 
                            pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
                // Save the frame to disk
                if (++i <= 5)
                    SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i);
            }
#if 0            
            avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, packet.data, packet.size);
            // Did we get a video frame?
            if (frameFinished) {
                // Convert the image from its native format to RGB
                img_convert((AVPicture *)pFrameRGB, AV_PIX_FMT_RGB24, (AVPicture *)pFrame, 
                            pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
                
                // Save the frame to disk
                if (++i <= 5)
                    SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i);
            }
#endif            
        }
    }
    // Free the packet that was allocate by av_read_frame
    av_packet_unref(&packet);
    
    // Free the RGB image
    av_free(buffer);
    av_free(pFrameRGB);
    
    // Free the YUV frame
    av_free(pFrame);
    
    // Close the codec
    avcodec_colse(pCodecCtx);
    
    // Close the video file
    av_close_input_file(pFormatCtx);
       
    return 0;
}

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
    FILE *pFile = NULL;
    char szFilename[32] = {0};
    int y = 0;
    
    // Open file
    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile = fopen(szFilename, "wb");
    if (NULL == pFile)
        return;
    
    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);
    
    // Write pixel data
    for (y = 0; y < height; y++)
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
    
    // Close file
    fclose(pFile);
    
}