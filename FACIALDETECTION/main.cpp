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

#include "FacePreprocess.h"
#include <opencv2/core.hpp>
#include <condition_variable>
#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"
#include "MTCNN/mtcnn_opencv.hpp"

#include <sw/redis++/redis++.h>
#include <experimental/filesystem>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>

#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>

using namespace std;
using namespace cv;
using namespace sw::redis;
using namespace std::chrono_literals;

class MTCNN;

/*REDIS*/
int face_count = 0;
Redis *_redis = new Redis("tcp://127.0.0.1:6379");
const char arcface_model[30] = "y1-arcface-emore_109";
const string prefix = "PATH_TO/models/linux";

mongocxx::instance instance{};
mongocxx::uri uri("mongodb://127.0.0.1:27017");
mongocxx::client client(uri);
mongocxx::database db = client["photo_bomb"];
mongocxx::collection _collection = db["videos"];

int frames_validator;
int frames_counter = 0;

std::string frame_rate;
std::string frame_number;

std::string frame_position;
std::string frame_position_time;

int main(int argc, char* argv[])
{
    vector<string> capture_data(argv, argv+argc);
    capture_data.erase(capture_data.begin());

    char buff[10];
    char string[10];
    const int stage = 4;
    float factor = 0.709f;
    const int minSize = 20;
    const int avg_face = 15;
    vector<double> angle_list;

    long frame_progress;

    MTCNN detector(prefix);
    MTCNN * _detector = &detector;
    float threshold[3] = {0.7f, 0.6f, 0.6f};
    std::string video_id = capture_data[1];
    std::string dataSource = capture_data[0];
    double fps, current, score, angle, padding;
    std::vector<uint8_t>* detected_face_buffer = new std::vector<uint8_t>();
    std::vector<uint8_t>* recognised_face_buffer = new std::vector<uint8_t>();

    // gt face landmark
    float v1[5][2] = {
            {30.2946f, 51.6963f},
            {65.5318f, 51.5014f},
            {48.0252f, 71.7366f},
            {33.5493f, 92.3655f},
            {62.7299f, 92.2041f}};

    cv::Mat src(5, 2, CV_32FC1, v1);
        
    if(capture_data.size() > 0){
        // Create a Subscriber.
        auto sub = _redis->subscriber();

        // Set callback functions.
        sub.on_message([&capture_data,buff,string,factor,angle_list,src,_detector,avg_face,minSize,stage,threshold,video_id,frame_progress,dataSource](std::string channel, std::string msg) {

            if(channel==capture_data[1]+"-pt"){
                int _pos = msg.find(":");

                frame_position= msg.substr(0 , _pos);
                frame_position_time = msg.substr(_pos + 1);
            }

            if(channel==capture_data[1]+"-fcp"){
                int Pos = msg.find(":");

                frame_number= msg.substr(0 , Pos);
                frame_rate = msg.substr(Pos + 1);
            }

            if(channel==capture_data[1]+"-fd"){
                int pos = msg.find(":");

                std::string faceTime = msg.substr(0 , pos);
                std::string frame_raw_data = msg.substr(pos + 1);
            
                std::vector<unsigned char> pic_data(frame_raw_data.begin(), frame_raw_data.end());
                cv::Mat vmat(pic_data, true);
                cv::Mat video_frame = cv::imdecode(vmat, 1);
                //grab video metrics from queue
                try{
                    ++frames_counter;
                    int frame_progress= ((double)stoi(frame_position)/stoi(frame_number))*100;
                    if(stoi(frame_position)%stoi(frame_rate)  == 0){
                        vector<FaceInfo> faceInfo = _detector->Detect_mtcnn(video_frame, minSize, threshold, factor, stage);
                        int faceSize = faceInfo.size();
                        
                        if(faceSize > 0){
                            //some face detected
                            cv::Mat facial_video_frame =video_frame.clone();
                            for (int i = 0; i < faceInfo.size(); i++) {
                                int x = (int) faceInfo[i].bbox.xmin;
                                int y = (int) faceInfo[i].bbox.ymin;
                                int w = (int) (faceInfo[i].bbox.xmax - faceInfo[i].bbox.xmin + 1);
                                int h = (int) (faceInfo[i].bbox.ymax - faceInfo[i].bbox.ymin + 1);
                                // Perspective Transformation
                                float v2[5][2] =
                                        {{faceInfo[i].landmark[0], faceInfo[i].landmark[1]},
                                        {faceInfo[i].landmark[2], faceInfo[i].landmark[3]},
                                        {faceInfo[i].landmark[4], faceInfo[i].landmark[5]},
                                        {faceInfo[i].landmark[6], faceInfo[i].landmark[7]},
                                        {faceInfo[i].landmark[8], faceInfo[i].landmark[9]},
                                        };
                                
                                cv::Mat dst(5, 2, CV_32FC1, v2);
                                memcpy(dst.data, v2, 2 * 5 * sizeof(float));
                                //extract detected face 
                                if(faceInfo[i].bbox.score >=  0.999){
                                    
                                    face_count++;
                                    cv::Mat frame = facial_video_frame.clone();

                                    cv::Mat _frame = frame.clone();

                                    cv::Rect roi(x, y, w, h);
                                    cv::Mat rFace = _frame(roi);
                                    cv::Mat live_rFace = rFace.clone();
                                    
                                    int xLeftTop = (int) faceInfo[i].bbox.xmin;
                                    int yLeftTop = (int) faceInfo[i].bbox.ymin;
                                    int xRightBottom = (int) faceInfo[i].bbox.xmax;
                                    int yRightBottom = (int) faceInfo[i].bbox.ymax;
                                    
                                    cv::Mat aligned = frame.clone();

                                    cv::Mat m = FacePreprocess::similarTransform(dst, src);
                                    cv::warpPerspective(frame, aligned, m, cv::Size(112, 112), INTER_LINEAR);
                                    cv::resize(aligned, aligned, Size(112, 112), 0, 0, INTER_LINEAR);

                                    //get time 
                                    float frame_time = (((double)frame_progress/stoi(frame_rate))*stoi(frame_number))/100;
                                    //error correction
                                    float _frame_time = frame_time -(frame_time *0.1);
                                    auto end = std::chrono::system_clock::now();
                                    char filename[100];
                                    std::string _dataSource = dataSource;
                                    auto TimeStamp =  std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                                    sprintf(filename, "/var/www/html/images/%ld.jpg", TimeStamp);
                                    std::time_t end_time = std::chrono::system_clock::to_time_t(end); 
                                    auto builder1 = bsoncxx::builder::stream::document{};
                                    bsoncxx::document::value doc_value = builder1
                                            << "id" << video_id
                                            << "image" <<  "http://127.0.0.1/images/"+ std::to_string(TimeStamp)+".jpg"
                                            << "videourl" << "http://127.0.0.1"+_dataSource.erase(0,13)
                                            << "time" << std::to_string(_frame_time)
                                            << "progress" << std::to_string(frame_progress)
                                            << "streamurl" << "http://127.0.0.1/recordings/videos/"+video_id+"/stream.m3u8"

                                    << bsoncxx::builder::stream::finalize;
                                    cv::imwrite(filename, aligned);
                                    //PUT TO DB
                                    _collection.insert_one(doc_value.view());
                                    
 std::cout << "detected face " << faceInfo[i].bbox.score <<" progress " << std::to_string(frame_progress)<<" frame position "<< frame_position << " time  " << frame_position_time << " frame number " << frame_number <<" frame time  " << std::to_string(frame_time) <<" frame time with error  " << std::to_string(_frame_time) << std::endl;

                                }

                            }
                        }
                    }

                    //RELEASE RESOURCES UPON SUCESSFUL TRAINING
                    frames_validator = stoi(frame_number) - frames_counter;
                    if(frames_validator ==0){
                        std::cout << " TRAINING OF  VIDEO "<< video_id << " FINISHED"<< std::endl;
                        exit(3);
                    }

                }catch( cv::Exception& e ){
                    const char* err_msg = e.what();
                    std::cout << err_msg << std::endl;
                }
            }
        });

        // Subscribe to multiple channels
        sub.subscribe({capture_data[1]+"-fd",capture_data[1]+"-pt",capture_data[1]+"-fcp"});

        // Consume messages in a loop.
        while (true) {
            try {
                sub.consume();
            } catch (...) {
                // Handle exceptions.
            }
        }
    }

    //capture->release();
    //exit(3);
    return 0;
}
