#include <array>
#include <thread>
#include <stdio.h>
#include <chrono>
#include <vector>
#include <time.h>
#include "stdlib.h"
#include <iostream>
#include <iterator>
#include <unistd.h>

#include <string>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include <spawn.h>
#include "helpers.hh"
#include <sys/stat.h>
#include <sys/types.h>

#include <opencv2/core.hpp>
#include <condition_variable>
#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"

#include <sw/redis++/redis++.h>

using namespace cv;
using namespace std;
using namespace sw::redis;

vector<string> capture_destination;
Redis *_redis = new Redis("tcp://127.0.0.1:6379");

int main(int argc, char* argv[])
{
    vector<string> capture_data(argv, argv+argc);
    capture_data.erase(capture_data.begin());

    std::string CAMERA_URL = capture_data[0];

    //GATHER INDIVIDUAL LIVE CAM METRICS
    VideoCapture *capture = new VideoCapture( CAMERA_URL , cv::CAP_FFMPEG);

    //total frames
    int frame_count = capture->get(cv::CAP_PROP_FRAME_COUNT);
    //Width
    int frame_width = capture->get(cv::CAP_PROP_FRAME_WIDTH); 
    //Height
    int frame_height = capture->get(cv::CAP_PROP_FRAME_HEIGHT);
    //Frame Rate 
    int frame_rate = capture->get(cv::CAP_PROP_FPS);

    cv::Mat frame;
    int frames_validator;
    int frames_counter = 0;

    while(true){
        try{
            ++frames_counter;
            //Grab frame from camera capture
            (*capture) >> frame;
            long frame_time = capture->get(cv::CAP_PROP_POS_MSEC);
            
            int delay_time = 1000;
            float frame_time_seconds = ((float)frame_time / delay_time);

            long frame_position = capture->get(cv::CAP_PROP_POS_FRAMES);
            
            std::ostringstream oss;
            oss << frame_time_seconds;
            auto _frame_time = oss.str();
            std::cout << " frame time  " << _frame_time << std::endl;

            std::ostringstream _oss;
            _oss << frame_position;
            auto _frame_position = _oss.str();
            
            //concanate video frame + metrics
            std::vector<uint8_t> frame_buffer;
            std::stringstream frame_super_string;
            cv::imencode(".png", frame, frame_buffer);
            for (auto c : frame_buffer) frame_super_string << c;
            std::string frame_buffer_as_string = frame_super_string.str();
            std::string data_payload = _frame_time +":"+frame_buffer_as_string;
            std::string _data_payload = data_payload;
            std::string frame_position_time_payload = _frame_position+":"+_frame_time;
            std::string frame_count_fps_payload= std::to_string(frame_count)+":"+std::to_string(frame_rate);
            _redis->publish(capture_data[1]+"-mp", data_payload);
            _redis->publish(capture_data[1]+"-fd", _data_payload);
            _redis->publish(capture_data[1]+"-pt", frame_position_time_payload);
            _redis->publish(capture_data[1]+"-fcp", frame_count_fps_payload);
            std::cout << " WIDTHs  " << frame_width << " HEIGHTs " << frame_height << " position " << _frame_position << " FPSs " << frame_rate  <<  " FNOs " << frame_count << " Timestamp "<< _frame_time << std::endl;

            frames_validator = frame_count - frames_counter;
            if(frames_validator ==0){
                std::cout << " TRAINING OF  VIDEO FINISHED " << std::endl;
                exit(3);
            }

        }catch( cv::Exception& e ){
            const char* err_msg = e.what();
            std::cout << "exception caught: corrrupted image on camera " << std::endl;
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
return 0;
}
