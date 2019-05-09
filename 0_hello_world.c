#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
    FILE *f = NULL;
    int i = 0;
    f = fopen(filename, "w");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    
    for (i = 0; i < ysize; i++) {
        fwrite(buf + wrap * i, 1, xsize, f);
    }
    //fwrite(buf, 1, xsize * ysize, f);
    
    fclose(f);
}

int main(int argc, char *argv[])
{
    AVFormatContext *pFmtCtx = avformat_alloc_context();
    
    if (avformat_open_input(&pFmtCtx, argv[1], NULL, NULL)) {
        fprintf(stderr, "open input failed\n");
        return -1;
    }
    
    printf("Format:%s, duration:%ld us\n", pFmtCtx->iformat->long_name, pFmtCtx->duration);
    
    if (avformat_find_stream_info(pFmtCtx, NULL) < 0) {
        fprintf(stderr, "find stream info failed\n");
        return -2;
    }
    
    for (int i = 0; i < pFmtCtx->nb_streams; i++) {
        AVCodecParameters *pCodecPar = pFmtCtx->streams[i]->codecpar;
        AVCodec *pCodec = NULL;
        pCodec = avcodec_find_decoder(pCodecPar->codec_id);
        if (NULL == pCodec) {
            fprintf(stderr, "Unsupported codec!\n");
            return -3;
        }
        if (AVMEDIA_TYPE_VIDEO == pCodecPar->codec_type) {
            printf("Video Codec: resolution: %d x %d\n", pCodecPar->width, pCodecPar->height);
        } else if (AVMEDIA_TYPE_AUDIO == pCodecPar->codec_type) {
            printf("Audio Codec: %d channels, sample rate %d\n", pCodecPar->channels, pCodecPar->sample_rate);
        }
        printf("\t Codec %s ID %d bit_rate %ld\n", pCodec->long_name, pCodec->id, pCodecPar->bit_rate);
        
        AVCodecContext *pCodecCtx = NULL;
        pCodecCtx = avcodec_alloc_context3(pCodec);
        avcodec_parameters_to_context(pCodecCtx, pCodecPar);
        
        if (avcodec_open2(pCodecCtx, pCodec, NULL)) {
            fprintf(stderr, "Codec open failed!\n");
            return -4;
        }
        
        AVPacket *pPacket = av_packet_alloc();
        AVFrame *pFrame = av_frame_alloc();
        
        while (av_read_frame(pFmtCtx, pPacket) >= 0) {
            avcodec_send_packet(pCodecCtx, pPacket);
            avcodec_receive_frame(pCodecCtx, pFrame);
            printf("Frame %c (%d) pts %ld dts %ld key_frame %d [coded_picture_number %d, display_picture_number %d]\n",
                    av_get_picture_type_char(pFrame->pict_type),
                    pCodecCtx->frame_number,
                    pFrame->pts,
                    pFrame->pkt_dts,
                    pFrame->key_frame,
                    pFrame->coded_picture_number,
                    pFrame->display_picture_number
            );
            if (AVMEDIA_TYPE_VIDEO == pCodecPar->codec_type) {
                char frame_filename[30] = {0};
                snprintf(frame_filename, 29, "./frame/Frame%d_%dx%d.yuv", pCodecCtx->frame_number, pFrame->width, pFrame->height);
                save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
            }
        }
    }
    
    return 0;
}