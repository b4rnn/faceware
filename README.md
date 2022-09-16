# FACEWARE

Analyze video files and record results as m3u8 

*IED box consists of various projects*

   * `VIDEO FRAME GRABBING / LIVESTREAM`
    `MEDIA PLAYER`
    `FACIAL DETECTION`
    
* VIDEO FRAME GRABBING / LIVESTREAM , MEDIA PLAYER AND FACIAL DETECTION have similar enviroment requirements since they run on same server *

# GENERAL SETUP

*FACEWARE has been optimised to run on cpu . No need for GPU.*
# Dependencies
```
sudo apt-get install libssl-dev
sudo apt-get install qtbase5-dev
sudo apt-get install qtdeclarative5-dev
sudo apt-get install libhdf5-serial-dev
sudo apt-get install build-essential git unzip pkg-config
sudo apt-get install libjpeg-dev libpng-dev libtiff-dev
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install libgtk2.0-dev
sudo apt-get install python3-dev python3-numpy python3-pip
sudo apt-get install libxvidcore-dev libx264-dev libgtk-3-dev
sudo apt-get install libtbb2 libtbb-dev libdc1394-22-dev
sudo apt-get install libv4l-dev v4l-utils
sudo apt-get install liblapacke-dev
sudo apt install default-jre
sudo apt install redis-server
sudo apt install pkg-config
sudo apt install -y mongodb
sudo apt-get install libboost-all-dev
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
sudo apt-get install libavresample-dev libvorbis-dev libxine2-dev
sudo apt-get install libfaac-dev libmp3lame-dev libtheora-dev
sudo apt-get install libopencore-amrnb-dev libopencore-amrwb-dev
sudo apt-get install libopenblas-dev libatlas-base-dev libblas-dev
sudo apt-get install liblapack-dev libeigen3-dev gfortran
sudo apt-get install liblapack-dev libeigen3-dev gfortran
sudo apt-get install libprotobuf-dev libgoogle-glog-dev libgflags-dev
 sudo apt install libpcre3-dev libssl-dev zlib1g-dev
sudo apt install libavcodec-dev libavformat-dev libswscale-dev libavresample-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libxvidcore-dev x264 libx264-dev libfaac-dev libmp3lame-dev libtheora-dev libvorbis-dev python3-dev libatlas-base-dev gfortran libgtk-3-dev libv4l-dev
sudo apt install libcanberra-gtk0 libcanberra-gtk-common-dev libcanberra-gtk3-0 libcanberra-gtk-dev libcanberra-gtk3-dev  libcanberra-gtk-module libcanberra-gtk3-module 
```
# CMAKE >=3.19

```
See [Install cmake from source](https://github.com/Kitware/CMake.git)
git clone https://github.com/Kitware/CMake.git
cd cmake directory then ..
./bootstrap && make && sudo make install

```
# OPENCV 4.5.0 
```
cd /tmp
wget -O opencv.zip https://github.com/opencv/opencv/archive/master.zip
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/master.zip
unzip opencv.zip
unzip opencv_contrib.zip
# Create build directory and switch into it
mkdir -p build && cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE  \
-D CMAKE_C_COMPILER=/usr/bin/gcc-7 \
-D CMAKE_INSTALL_PREFIX=/usr/local \
-D OPENCV_EXTRA_MODULES_PATH=../opencv_contrib-master/modules ../opencv-master \
-D BUILD_TIFF=ON \
-D WITH_FFMPEG=ON \
-D WITH_GSTREAMER=ON \
-D WITH_TBB=ON \
-D BUILD_TBB=ON \
-D WITH_EIGEN=ON \
-D OPENCV_ENABLE_NONFREE=ON \
-D INSTALL_C_EXAMPLES=OFF \
-D INSTALL_PYTHON_EXAMPLES=OFF \
-D PYTHON_DEFAULT_EXECUTABLE=$(which python3) \
-D BUILD_SHARED_LIBS=OFF \
-D BUILD_NEW_PYTHON_SUPPORT=ON \
-D OPENCV_GENERATE_PKGCONFIG=ON \
-D OPENCV_PC_FILE_NAME=opencv4.pc \
-D OPENCV_ENABLE_NONFREE=ON \
-D BUILD_EXAMPLES=OFF ..
```
```
make -j$(nproc)
sudo make install
sudo ldconfig
pkg-config --modversion opencv4
```
# libmongoc and libbson
```
sudo apt-get install libbson-1.0-0
sudo apt-get install libmongoc-1.0-0
```
# mongo-c driver

```
git clone https://github.com/mongodb/mongo-c-driver.git
cd mongo-c-driver
git checkout 1.17.0
mkdir cmake-build
cd cmake-build
cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ..
cmake --build .
sudo cmake --build . --target install
sudo cmake --build . --target uninstall [--]
```
# mongocxx-3.6.x

```
curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.6.5/mongo-cxx-driver-r3.6.5.tar.gz
tar -xzf mongo-cxx-driver-r3.6.5.tar.gz
cd mongo-cxx-driver-r3.6.5/build
cmake ..                                \
    -DCMAKE_BUILD_TYPE=Release          \
    -DCMAKE_INSTALL_PREFIX=/usr/local
sudo cmake --build . --target EP_mnmlstc_core
cmake --build .
sudo cmake --build . --target install
sudo cmake --build . --target uninstall [--]
```

#  hiredis
```
git clone https://github.com/redis/hiredis.git
cd hiredis
make
sudo make install
```

#  redis-plus-plus
```
git clone https://github.com/sewenew/redis-plus-plus.git
cd redis-plus-plus
mkdir build
cd build
cmake ..
make
sudo make install
```
# NGINX SERVER  + RTMP 
```
sudo apt install nginx

#Nginx Directories
    #File Directories
     cd /var/www/html/ && mkdir uploads && mkdir images && mkdir quotas && mkdir cbi && mkdir recordings && mkdir recordings/videos && mkdir recordings/cams && cd ..
```
# python 3.5 +
```
#update & upgrade
    sudo apt update
    sudo apt install software-properties-common
#repo
    sudo add-apt-repository ppa:deadsnakes/ppa
    Press [ENTER] to continue or Ctrl-c to cancel adding it.
#install
    sudo apt install python3.7
#check
    python3.7 --version
```
# pip 
```
#install
    python3 -m pip install --upgrade pip
#upgrade
    pip3 install --upgrade setuptools
    pip3 install -r requirements.txt
```
# PORTS 
```
flsk --5005 
redis --6379
mongodb --27107
uwsgi --5029:5033
```
# [1] LIVECAM

```
cd to  LIVECAM FOLDER 
    cd build  && rm -rf CMakeCache.txt ENGINE  CMakeFiles LIVEVIDEO Makefile  __pycache__  cmake_install.cmake 
    cmake ..
    make -j{NUMBER_OF_THREADS}
#If successful ... you will have the compiled project name i.e LIVEVIDEO
    CMakeCache.txt  CMakeFiles  IED  Makefile  app.py  cmake_install.cmake  libfacedetect.so
#Compile the ENGINE program
    g++ ../engine.cpp -o ENGINE
#If successful ... you will have the compiled ENGINE i.e ENGINE
    uwsgi --ini app.ini
```
# [2] MEDIAPLAYER
```
cd to  MEDIAPLAYER FOLDER 
    cd build  && rm -rf CMakeCache.txt ENGINE  CMakeFiles MEDIAPLAYER Makefile  __pycache__  cmake_install.cmake 
    cmake ..
    make -j{NUMBER_OF_THREADS}
#If successful ... you will have the compiled project name i.e MEDIAPLAYER
    CMakeCache.txt  CMakeFiles  IED  Makefile  app.py  cmake_install.cmake  libfacedetect.so
#Compile the ENGINE program
    g++ ../engine.cpp -o ENGINE
#If successful ... you will have the compiled ENGINE i.e ENGINE
    uwsgi --ini app.ini
```
# [3] FACIALDETECTION
```
cd to  FACIALDETECTION FOLDER 
    cd build  && rm -rf CMakeCache.txt ENGINE  CMakeFiles FACIALDETECTION Makefile  __pycache__  cmake_install.cmake 
    cmake ..
    make -j{NUMBER_OF_THREADS}
#If successful ... you will have the compiled project name i.e FACIALDETECTION
    CMakeCache.txt  CMakeFiles  IED  Makefile  app.py  cmake_install.cmake  libfacedetect.so
#Compile the ENGINE program
    g++ ../engine.cpp -o ENGINE
#If successful ... you will have the compiled ENGINE i.e ENGINE
    uwsgi --ini app.ini
```
# [4] BROADCAST
```
sudo vi /etc/nging/sites-enabled/default

#paste text below
    server {
            listen 80;
            listen [::]:80;
            #server_name  str.appb.casa;
            #ssl_certificate      /etc/letsencrypt/live/str.appb.casa/fullchain.pem;
            #ssl_certificate_key  /etc/letsencrypt/live/str.appb.casa/privkey.pem;

            root /var/www/html;
            index index.html;

            location /videos/ {
            #add_header 'Access-Control-Allow-Origin' '*' always;
                proxy_pass http://localhost:5005;
        }
            
            location /live/ {
            add_header 'Access-Control-Allow-Origin' '*' always;
        }

            location /recordings/ {
            add_header 'Access-Control-Allow-Origin' '*' always;
        }
        
        location /profiles/ {
            proxy_pass http://localhost:5005;
        }

        location /controls/ {
        proxy_pass http://localhost:5007;
        }
    }

    #BROADCAST
    geo $limit {
        default 1;
    }
        
    map $limit $limit_key {
            0 "";
            1 $binary_remote_addr;
    }

    limit_req_zone $limit_key zone=req_zone:10m rate=8r/s;
    #offline

    #START[ALL SINGLE VIDEO SEARCH NODES]
    server {
        listen 5029;
        listen [::]:5029;

        server_name _;

        #root /var/www/html;
        root /usr/share/nginx/html;
        index index.html;

        location / {
                # First attempt to serve request as file, then
                # as directory, then fall back to displaying a 404.
                limit_req zone=req_zone burst=1 nodelay;
                proxy_pass http://127.0.0.1:5030;
                proxy_pass_request_headers on;
                proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
                proxy_set_header Host $http_host;
                proxy_set_header X-Forwarded-Proto $scheme;
                proxy_redirect off;
                proxy_connect_timeout       300;
                proxy_send_timeout          300;
                proxy_read_timeout          300;
                send_timeout                300;
                proxy_method POST;
                    mirror /VIDEOSEARCHLIVECAM;
                            mirror /VIDEOSEARCHFACIALDETECTION;
                    mirror /VIDEOSEARCHMEDIAPLAYERRECORDER;

                    mirror_request_body on;
                    try_files $uri $uri/ =404;
        }

        #local server
        location /VIDEOSEARCHLIVECAM                    	    { internal;proxy_pass http://127.0.0.1:5031$request_uri; }
        location /VIDEOSEARCHFACIALDETECTION            	    { internal;proxy_pass http://127.0.0.1:5032$request_uri; }
        location /VIDEOSEARCHMEDIAPLAYERRECORDER          	    { internal;proxy_pass http://127.0.0.1:5033$request_uri; }
    }

#test nginx server
    sudo nginx -t
    sudo systemctl restart nginx
    sudo systemctl status nginx

cd back to   projcet FOLDER 
    python3 broadcast.py

#test using postman (127.0.0.1:5005)
```

# faceware
# faceware
