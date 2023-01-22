# pico-audio

Steps to creates audio header file
. ffmpeg -i sample.wav -f s16le -ar 22050 -acodec pcm_s16le sample_22kHz_16bits_mono.raw
. Build this c "pcm2header.c" file and run as follow
  ./pcm2header sample_22kHz_16bits_mono.raw
. copy generated header file "sample_22kHz_16bits_mono" to "/media/" folder and adjust the number of samples if needed

Steps to build
```
$git clone
$cd pico-audio
$mkdir build
$cd build
$cmake ..
$make -j4
```
Steps to run
```
. install openocd and run this command
. openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program /.../pico-audio/build/pico-audio.elf verify reset exit"
```

Hardware assembly:
. Use Ground pin and #2 pin from Pico to connect the headphone.



Note:
```
 
```