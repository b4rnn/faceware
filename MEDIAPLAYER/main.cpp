#include <array>
#include <thread>
#include <stdio.h>
#include <chrono>
#include <vector>
#include <time.h>
#include <iostream>
#include <iterator>
#include <unistd.h>
#include "stdlib.h"

#include <string>
#include <sstream>
#include <fstream>
#include <algorithm> 

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <opencv2/core.hpp>
#include <condition_variable>
#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"

#include <sw/redis++/redis++.h>
#include <experimental/filesystem>

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

using namespace std;
using namespace cv;
using namespace sw::redis;
using namespace std::chrono_literals;

namespace fs = std::experimental::filesystem;

int frames_validator;
int frames_counter = 0;

/*REDIS*/
Redis *_redis = new Redis("tcp://127.0.0.1:6379");

int main(int argc, char* argv[])
{

    vector<string> capture_data(argv, argv+argc);
    capture_data.erase(capture_data.begin());

    std::string id = capture_data[1];

    //GATHER INDIVIDUAL LIVE CAM METRICS
    VideoCapture *capture = new VideoCapture(capture_data[0] , cv::CAP_FFMPEG);
    int frame_count = capture->get(cv::CAP_PROP_FRAME_COUNT);
    //Width
    int frame_width = capture->get(cv::CAP_PROP_FRAME_WIDTH); 
    //Height
    int frame_height = capture->get(cv::CAP_PROP_FRAME_HEIGHT);
    //Frame Rate 
    int frame_rate = capture->get(cv::CAP_PROP_FPS);

    //ENCODER
    AVFormatContext *format_ctx = avformat_alloc_context();
    std::string videoUrl  = capture_data[0];
    const char *outFname  = videoUrl.c_str();

    int directory_status,_directory_status;
    string folder_name = "/var/www/html/recordings/videos/"+capture_data[1];

    uintmax_t n = fs::remove_all(folder_name);

    const char* dirname = folder_name.c_str();
    directory_status = mkdir(dirname,0777);

    std::string segment_list_type = folder_name + "/stream.m3u8";

    avformat_alloc_output_context2(&format_ctx, nullptr, "hls", segment_list_type.c_str());
    if(!format_ctx) {
        return 1;
    }

    if (!(format_ctx->oformat->flags & AVFMT_NOFILE)) {
        int avopen_ret  = avio_open2(&format_ctx->pb, outFname,
                                    AVIO_FLAG_WRITE, nullptr, nullptr);
        if (avopen_ret < 0)  {
            std::cout << "failed to open stream output context, stream will not work! " <<  outFname << std::endl;
            return 1;
        }
    }

    AVCodec *out_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    AVStream *out_stream = avformat_new_stream(format_ctx, out_codec);
    AVCodecContext *out_codec_ctx = avcodec_alloc_context3(out_codec);
    //set codec params
    const AVRational dst_fps = {frame_rate, 1};
    out_codec_ctx->codec_tag = 0;
    out_codec_ctx->codec_id = AV_CODEC_ID_H264;
    out_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    out_codec_ctx->width = frame_width;
    out_codec_ctx->height = frame_height;
    out_codec_ctx->gop_size = 12;
    out_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    out_codec_ctx->framerate = dst_fps;
    out_codec_ctx->time_base.num = 1;
    out_codec_ctx->time_base.den = frame_rate; // fps
    out_codec_ctx->thread_type = FF_THREAD_SLICE;
    
    out_codec_ctx->time_base = av_inv_q(dst_fps);
    if (format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    {
        out_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    int ret_avp = avcodec_parameters_from_context(out_stream->codecpar, out_codec_ctx);
    if (ret_avp < 0)
    {
        std::cout << "Could not initialize stream codec parameters!" << std::endl;
        exit(1);
    }
        
    if (!directory_status){
        std::cout << "CREATED DIR " <<  dirname << std::endl;
        std::string hls_segment_filename = folder_name + "/filename%03d.ts";
        std::cout << "HLS SEGMENT " <<  hls_segment_filename << std::endl;

        AVDictionary *hlsOptions = NULL;

        av_dict_set(&hlsOptions, "c", "copy", 0);
        av_dict_set(&hlsOptions, "preset", "slow", 0);
        av_dict_set(&hlsOptions, "tune", "zerolatency", 0);
        av_dict_set(&hlsOptions, "crf", "20", 0); 
        av_dict_set(&hlsOptions, "force_key_frames", "expr:gte(t,n_forced*3)", AV_DICT_DONT_OVERWRITE);

        av_dict_set(&hlsOptions,     "hls_segment_type",   "mpegts", 0);
        av_dict_set(&hlsOptions,     "segment_list_type",  "m3u8",   0);
        av_dict_set(&hlsOptions,     "hls_segment_filename", hls_segment_filename.c_str(),   0);
        av_dict_set(&hlsOptions,     "hls_playlist_type", "vod", 0);
        av_dict_set(&hlsOptions,     "segment_time_delta", "1.0", 0);
        av_dict_set(&hlsOptions,     "hls_flags", "append_list", 0);
        av_dict_set_int(&hlsOptions, "reference_stream",   out_stream->index, 0);

        int ret_avo = avcodec_open2(out_codec_ctx, out_codec, &hlsOptions);
        if (ret_avo < 0)
        {
            std::cout << "Could not open video encoder!" << std::endl;
            exit(1);
        }
        out_stream->codecpar->extradata = out_codec_ctx->extradata;
        out_stream->codecpar->extradata_size = out_codec_ctx->extradata_size;

        av_dump_format(format_ctx, 0, outFname, 1);

        SwsContext * pSwsCtx = sws_getContext(frame_width,frame_height,AV_PIX_FMT_BGR24, frame_width, frame_height , AV_PIX_FMT_YUV420P , SWS_FAST_BILINEAR, NULL, NULL, NULL);
                
        if (pSwsCtx == NULL) {
            fprintf(stderr, "Cannot initialize the sws context\n");
            return -1;
        }
        

        int ret_avfw = avformat_write_header(format_ctx, &hlsOptions);
        if (ret_avfw < 0)
        {
            std::cout << "Could not write header!" << std::endl;
            exit(1);
        }

        std::vector<uint8_t>* imgbuf = new std::vector<uint8_t>(frame_height * frame_width * 3 + 16);
        std::vector<uint8_t>* framebuf = new std::vector<uint8_t>(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, frame_width, frame_height, 1));
        cv::Mat* image = new cv::Mat(frame_height, frame_width, CV_8UC3, imgbuf->data(), frame_width * 3);

        std::cout << "DONE REMUXING " <<  outFname << std::endl;

        int av_ret;
        AVFrame *frame = av_frame_alloc();

        //DECODER
        
        if(capture_data.size() > 0){
            // Create a Subscriber.
            auto sub = _redis->subscriber();

            // Set callback functions.
            sub.on_message([frame,framebuf,frame_width,frame_height,image,out_codec_ctx,out_stream,format_ctx,pSwsCtx,frame_count](std::string channel, std::string msg) {
                ++frames_counter;
                int pos = msg.find(":");
                
                std::string frame_number = msg.substr(0 , pos);
                std::string frame_raw_data = msg.substr(pos + 1);
            
                std::vector<unsigned char> pic_data(frame_raw_data.begin(), frame_raw_data.end());
                cv::Mat vmat(pic_data, true);
                cv::Mat video_frame = cv::imdecode(vmat, 1);
                
                av_image_fill_arrays(frame->data, frame->linesize, framebuf->data(), AV_PIX_FMT_YUV420P, frame_width, frame_height, 1);
                frame->width = frame_width;
                frame->height = frame_height;
                frame->format = static_cast<int>(AV_PIX_FMT_YUV420P);
                
                *image = video_frame;
            
                const int stride[] = {static_cast<int>(image->step[0])};
                sws_scale(pSwsCtx, &image->data, stride , 0 , frame_height , frame->data, frame->linesize); 
                frame->pts += av_rescale_q(1, out_codec_ctx->time_base, out_stream->time_base);

                std::cout <<frame->pts << " WIDTH " <<  frame->width <<" HEIGHT " << frame->height << " COUNTER "<< frames_counter << std::endl;
                
                AVPacket pkt = {0};
                av_init_packet(&pkt);

                if(out_stream != NULL){
                    
                    int ret_frame = avcodec_send_frame(out_codec_ctx, frame);
                    if (ret_frame < 0)
                    {
                        std::cout << "Error sending frame to codec context!" << std::endl;
                        exit(1);
                    }
                    int ret_pkt = avcodec_receive_packet(out_codec_ctx, &pkt);
                    if (ret_pkt < 0)
                    {
                        std::cout << "Error receiving packet from codec context!" << " WIDTH " <<  frame_width <<" HEIGHT " << frame_height << std::endl;
                        exit(1);
                    }

                    if (pkt.pts == AV_NOPTS_VALUE || pkt.dts == AV_NOPTS_VALUE) {
                        av_log (format_ctx, AV_LOG_ERROR,
                            "Timestamps are unset in a packet for stream% d\n", out_stream-> index);
                        return AVERROR (EINVAL);
                    }

                    if (pkt.pts < pkt.dts) {
                    av_log (format_ctx, AV_LOG_ERROR, "pts%" PRId64 "<dts%" PRId64 "in stream% d\n",
                        pkt.pts, pkt.dts,out_stream-> index);
                    return AVERROR (EINVAL);
                    }
                    if (pkt.stream_index == out_stream->index){
                        av_interleaved_write_frame(format_ctx, &pkt);
                    }

                }

                //Release resources upon completion
                frames_validator = frame_count - frames_counter;
                if(frames_validator ==0){
                    std::cout << " TRAINING OF  VIDEO FINISHED " << std::endl;
                    exit(3);
                }

            });

            // Subscribe to channels and patterns.
            sub.subscribe(capture_data[1]+"-mp");

            // Consume messages in a loop.
            while (true) {
                try {
                    sub.consume();
                } catch (...) {
                    // Handle exceptions.
                }
            }
        }
        
    }
    
    return 0;
}
