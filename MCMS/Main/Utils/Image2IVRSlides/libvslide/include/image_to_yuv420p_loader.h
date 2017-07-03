#ifndef __IMAGE_TO_YUV420P_LOADER_H__
#define __IMAGE_TO_YUV420P_LOADER_H__

#include <stdint.h>

class ImageToYUV420pLoader
{
public:
    ImageToYUV420pLoader();
    ~ImageToYUV420pLoader();

    uint8_t * LoadImageToYuvBuffer(char const * fileName, int & width, int & height);

private:
    uint8_t * LoadBmpFileToYuvBuffer(char const * fileName, int & widthOut, int & heightOut);
    uint8_t * LoadJpegFileToYuvBuffer(char const * fileName, int & widthOut, int & heightOut);
    
private:
    ImageToYUV420pLoader(ImageToYUV420pLoader const &);
    ImageToYUV420pLoader & operator=(ImageToYUV420pLoader const &);
    
    uint8_t * pYuvBuffer_;
    int width_;
    int height_;
};

#endif

