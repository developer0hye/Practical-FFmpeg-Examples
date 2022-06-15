#include <iostream>
#include <fstream>
#include <string>

extern "C" 
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

int save_frame_as_jpeg(AVFrame *pFrame, int FrameNo, int qscale=1) {
    const AVCodec *jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!jpegCodec) {
        return -1;
    }
    AVCodecContext *jpegContext = avcodec_alloc_context3(jpegCodec);
    if (!jpegContext) {
        return -1;
    }

    jpegContext->pix_fmt = (AVPixelFormat)pFrame->format;
    jpegContext->height = pFrame->height;
    jpegContext->width = pFrame->width;
    jpegContext->time_base.num = 1;
    jpegContext->time_base.den = 1; 
    jpegContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    jpegContext->flags |= AV_CODEC_FLAG_QSCALE;
    jpegContext->qmin = qscale;
    
    if (avcodec_open2(jpegContext, jpegCodec, NULL) < 0) {
        std::cout << "failed to open jpegCodec through avcodec_open2" << std::endl;
        return -1;
    }

    AVPacket* pPacket = av_packet_alloc();
    if (!pPacket)
    {
        std::cout << "failed to allocate memory for AVPacket" << std::endl;
        return -1;
    }

    int response = avcodec_send_frame(jpegContext, pFrame);
    char errorStr[AV_ERROR_MAX_STRING_SIZE] = {0};

    if (response < 0) 
    {
        av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, response);
        std::cout << "Error while sending a frame to the decoder: " << errorStr << std::endl;
        return response;
    }
    
    std::string file_name = std::to_string(FrameNo) + std::string(".jpg");
    std::ofstream fout(file_name, std::ios::binary);
    while (response >= 0) 
    {
        response = avcodec_receive_packet(jpegContext, pPacket);
        if (response >= 0)
        {
            std::cout << "pPacket->size: " << pPacket->size << std::endl;
            fout.write((char*)pPacket->data, pPacket->size);
            break;
        }
        else if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
            return -1;
        else if (response < 0) {
            av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, response);
            std::cout << "Error during encoding: " << errorStr << std::endl;
            return response;
        }
        av_packet_unref(pPacket);
    }
    // fclose(f);
    avcodec_close(jpegContext);
    std::cout<<"write" <<std::endl;
    return 0;
}

int main(int argc, char** argv)
{
    if(argc != 3)
    {
      std::cout << "Enter your video file and qscale(1(high quality) ~ 31(low quality))" << std::endl;
      std::cout << "Example) video2jpgs.exe your_video.avi 3" << std::endl;
      return -1;
    }

    std::string file_path(argv[1]);

    AVFormatContext *pFormatContext = avformat_alloc_context();

    if (avformat_open_input(&pFormatContext, file_path.c_str(), NULL, NULL) != 0) {
        std::cout << "ERROR could not open the file" << std::endl;
        return -1;
    }

    if (avformat_find_stream_info(pFormatContext,  NULL) < 0) {
        std::cout << "ERROR could not get the stream info" << std::endl;
        return -1;
    }

    const AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters =  nullptr;
    int video_stream_index = -1;
    double video_fps = 0.;

    for (int i = 0; i < pFormatContext->nb_streams; i++)
    {
        AVCodecParameters *pLocalCodecParameters =  pFormatContext->streams[i]->codecpar;
        const AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            video_fps = static_cast<double>(pFormatContext->streams[i]->avg_frame_rate.num)/static_cast<double>(pFormatContext->streams[i]->avg_frame_rate.den);
            pCodec = pLocalCodec;
            pCodecParameters = pLocalCodecParameters;
            std::cout << "Video Codec: resolution " << pLocalCodecParameters->width << " x " << pLocalCodecParameters->height << " FPS " << video_fps << std::endl;
            break;
        }
    }

    if (pCodec == nullptr) {
        std::cout << "File " << file_path << " does not contain a video stream!" << std::endl;
        return -1;
    }

    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext)
    {
        std::cout << "failed to allocate memory for AVCodecContext" << std::endl;
        return -1;
    }

    // Fill the codec context based on the values from the supplied codec parameters
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0)
    {
        std::cout << "failed to copy codec params to codec context" << std::endl;
        return -1;
    }

    // Initialize the AVCodecContext to use the given AVCodec.
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
    if (avcodec_open2(pCodecContext, pCodec, NULL) < 0)
    {
        std::cout << "failed to open codec through avcodec_open2" << std::endl;
        return -1;
    }

    // https://ffmpeg.org/doxygen/trunk/structAVFrame.html
    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame)
    {
        std::cout << "failed to allocated memory for AVFrame" << std::endl;
        return -1;
    }
    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket)
    {
        std::cout << "failed to allocated memory for AVPacket" << std::endl;
        return -1;
    }

    while (true)
    {
        int ret = av_read_frame(pFormatContext, pPacket);
        if ( ret < 0)
        {
            break;
        }
        // if it's the video stream
        if (pPacket->stream_index == video_stream_index) 
        {
            int response = avcodec_send_packet(pCodecContext, pPacket);
            char errorStr[AV_ERROR_MAX_STRING_SIZE] = {0};

            if (response < 0) 
            {
                av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, response);
                std::cout << "Error while sending a packet to the decoder: " << errorStr << std::endl;
                return response;
            }

            while (response >= 0)
            {
                response = avcodec_receive_frame(pCodecContext, pFrame);
                if (response >= 0)
                {
                    std::cout<<"successfully decoded"<<std::endl;
                    save_frame_as_jpeg(pFrame, pFrame->pts, atoi(argv[2]));
                }
                else if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) 
                {
                    break;
                } 
                else if (response < 0) 
                {
                    av_make_error_string(errorStr, AV_ERROR_MAX_STRING_SIZE, response);
                    std::cout << "Error while receiving a frame from the decoder: " << errorStr << std::endl;
                    return response;
                }
            }
        }
        av_packet_unref(pPacket);
    }
    return 0;
}