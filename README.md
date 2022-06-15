# Practical-FFmpeg-Examples

## Install
```
git clone https://github.com/developer0hye/Practical-FFmpeg-Examples.git
cd Practical-FFmpeg-Examples
./download_ffmpeg.bat
```

## Build
```
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

## Run

### video2jpgs
argument 1: Video file

argument 2: Quality factor(Lower is better.)[1, 31]
```
cd build
./video2jpgs.exe your_video.avi 1
```