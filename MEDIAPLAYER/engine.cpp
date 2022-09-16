#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include <spawn.h>
#include <iostream>
#include <algorithm>
#include "helpers.hh"

using namespace std;

 vector<string> capture_destination;

int main(int argc, char* argv[])
{
   
    vector<string> _capture_source(argv, argv+argc);
    _capture_source.erase(_capture_source.begin());
    std::cout << " POST DATA  " << _capture_source[0] << " VIDEOS FINISHED " << std::endl;

    vector<string> capture_source;
    vector<string> capture_data;
 
    std::stringstream superVideo(_capture_source[0]);
    string video;
    while (superVideo >> video)
    {
        capture_data.push_back(video);
        if (superVideo.peek() == ',')
            superVideo.ignore();
    }

    char chars[] = "[-{,\'}_]";

    for (int j=0;j<capture_data.size();j++)
    {
        std::string data = capture_data.at(j);
        for (unsigned int i = 0; i < strlen(chars); ++i)
        {
            data.erase(std::remove(data.begin(), data.end(), chars[i]), data.end());
        }
        std::string _video = data.substr(data.find(":")+1);
        capture_source.push_back(_video);
        std::string _channels = data.substr(0 ,data.find(":"));
        capture_destination.push_back(_channels);
    }

   
    const char* args[] = {
        "./MEDIAPLAYER",
        capture_source[0].c_str(),
        capture_destination[0].c_str(),
        "camera arguments.",
        nullptr
    };

    fprintf(stderr, "About to spawn media player process from pid %d\n", getpid());
    pid_t p;
    int r = posix_spawn(&p, "./MEDIAPLAYER", nullptr, nullptr,
                       (char**) args, nullptr);
    //assert(r == 0);
    std::cout << r << std::endl;

    fprintf(stderr, "Child pid %d should run my media player\n", p);
return 0;
}