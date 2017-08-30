# rtvf
Real-time video filter

Simple real-time video processing engine written back in 2008 (or so) as a university course work.

Modular architecture consists of:
  - Decoders - source of video stream
    - live streaming from camera
    - playback of on-disk media files
    
  - Filters - processing units that can be stacked in real-time, each processing frames with different algorithms
    - grayscale filter
    - grain
    - fade in/out
    - threshold
    - etc.

Example run:
./rtvf -d MediaFileDecoder.so,file=/home/mrate/video.mpg -w 800 -h 600 -f
