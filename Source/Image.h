#pragma once

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

namespace Luma {

class Image
{
public:
    // Constructor.
    Image(uint16_t width, uint16_t height) : m_width(width), m_height(height)
    {
        // Create the image buffer.
        size_t bufferSize = m_width * m_height * NUM_COMPONENTS;
        m_pImageData = new uint8_t[bufferSize];
    }

    // Destructor.
    ~Image()
    {
        delete[] m_pImageData;
        m_pImageData = nullptr;
    }

    // Returns the image data buffer.
    uint8_t* GetImageData() { return m_pImageData; }

    // Saves the image as a PNG file to the specified path, with an optional scale to enlarge the
    // image.
    void SavePNG(string sFilePath, uint8_t scale = 1)
    {
        uint8_t* pImageData = m_pImageData;
        uint16_t width = m_width;
        uint16_t height = m_height;

        // Resize the image by the specified scale if one is specified.
        if (scale > 1)
        {
            pImageData = ScaleImage(scale);
            width *= scale;
            height *= scale;
        }

        // Write the image buffer to the output file path, and delete the image buffer.
        ::stbi_write_png(
            sFilePath.c_str(), width, height, NUM_COMPONENTS, pImageData, width * NUM_COMPONENTS);

        // Delete the scaled image buffer if it was created.
        if (scale > 1)
        {
            delete[] pImageData;
        }
    }

private:
    static const uint8_t NUM_COMPONENTS = 3;

    uint8_t* m_pImageData = nullptr;
    uint16_t m_width;
    uint16_t m_height;

    // Scales (enlarges) an image buffer by the specified scale factor, returning a new buffer.
    uint8_t* ScaleImage(uint8_t scale)
    {
        // Create the destination buffer, as a multiple of the source buffer, e.g. 240x135 with a
        // scale of 8 becomes 1920x1080.
        uint16_t destWidth = m_width * scale;
        uint16_t destHeight = m_height * scale;
        size_t destBufferSize = destWidth * destHeight * NUM_COMPONENTS;
        uint8_t* pDestData = new uint8_t[destBufferSize];

        // Iterate the destination image pixels, copying the appropriate source image pixel for each.
        uint8_t* pDst = pDestData;
        for (uint16_t y = 0; y < destHeight; y++)
        {
            // Get the start of the relevant scanline from the source image, i.e. advance the source
            // scanline every "scale" scanlines (e.g. 8) of the destination image.
            const uint8_t* pSrc = &m_pImageData[(y / scale) * m_width * NUM_COMPONENTS];

            // Iterate the pixels of the destination image, copying the pixels of source image. The
            // source image pointer is advanced every "scale" (e.g. 8) pixels of the destination image.
            for (uint16_t x = 0; x < destWidth; x++)
            {
                ::memcpy(pDst, pSrc, NUM_COMPONENTS);
                pDst += NUM_COMPONENTS;
                pSrc += (x + 1) % scale == 0 ? NUM_COMPONENTS : 0;
            }
        }

        return pDestData;
    }
};

} // namespace Luma
