#include "PVRTexture.h"
#include "Common.h"
#include "SexyAppBase.h"
#include "PakInterface.h"
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif

#if TARGET_OS_IPHONE == 0
// FIXME. Get this from an include file. Perhaps PVRTexLib.h
enum {
    OGL_PVRTC2 = 24,
    OGL_PVRTC4,
};
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03
#endif

#define PVR_TEXTURE_FLAG_TYPE_MASK	0xff

using namespace Sexy;
using namespace std;

typedef struct _PVRTexHeader
{
    uint32_t headerLength;
    uint32_t height;
    uint32_t width;
    uint32_t numMipmaps;
    uint32_t flags;
    uint32_t dataLength;
    uint32_t bpp;
    uint32_t bitmaskRed;
    uint32_t bitmaskGreen;
    uint32_t bitmaskBlue;
    uint32_t bitmaskAlpha;
    uint32_t pvrTag;
    uint32_t numSurfs;
} PVRTexHeader;

static const char gPVRTexIdentifier[] = "PVR!";

PVRTexture::PVRTexture() :
        mWidth(0),
        mHeight(0),
        mInternalFormat(0),
        mHasAlpha(false)
{
    mImageData.clear();
    mImageDataLength.clear();
}

PVRTexture::~PVRTexture()
{
    for (size_t i = 0; i < mImageData.size(); i++) {
        delete [] mImageData[i];
    }
}

bool PVRTexture::unpackPVRData(uint8_t* data)
{
    // Forget about endianness, just accept LE.
    const PVRTexHeader * header = (const PVRTexHeader *)data;

    // Sanity check
    if (header->headerLength != sizeof(PVRTexHeader)) {
        // TODO. Throw exception
        return false;
    }
    if (header->pvrTag != *(uint32_t*)gPVRTexIdentifier) {
        // TODO. Throw exception
        return false;
    }

    uint32_t formatFlags = header->flags & PVR_TEXTURE_FLAG_TYPE_MASK;

    // We only accept a limited set of formats
    if (formatFlags == OGL_PVRTC4) {
        mInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
    }
    else if (formatFlags == OGL_PVRTC2) {
        mInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
    }
    else {
        // TODO. Throw exception
        return false;
    }

    mWidth = header->width;
    mHeight = header->height;

    mHasAlpha = header->bitmaskAlpha != 0 ? true : false;

    int width = mWidth;
    int height = mHeight;
    uint32_t dataOffset = 0;
    int blockSize = 0;
    int widthBlocks = 0;
    int heightBlocks = 0;
    int bpp = 0;
    while (dataOffset < header->dataLength) {
        // Do something
        if (formatFlags == OGL_PVRTC4) {
            blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
            widthBlocks = width / 4;
            heightBlocks = height / 4;
            bpp = 4;
        }
        else {
            blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
            widthBlocks = width / 8;
            heightBlocks = height / 4;
            bpp = 2;
        }

        // Clamp to minimum number of blocks
        if (widthBlocks < 2)
            widthBlocks = 2;
        if (heightBlocks < 2)
            heightBlocks = 2;

        int dataSize = widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);
        uint8_t * blob = new uint8_t[dataSize];
        memcpy(blob, data + dataOffset, dataSize);
        mImageData.push_back(blob);
        mImageDataLength.push_back(dataSize);

        dataOffset += dataSize;

        width = std::max(width >> 1, 1);
        height = std::max(height >> 1, 1);
    }

    return true;
}

bool PVRTexture::initWithContentsOfFile(const string& fname)
{
    string myFname = gSexyAppBase->GetAppResourceFileName(fname);
    PFILE* file = p_fopen(myFname.c_str(), "r");
    if (file == NULL) {
        // TODO. Throw exception
        return false;
    }

    int size = file->mRecord->mSize - file->mPos;
    uint8_t* buffer = new uint8_t[size];
    int res = p_fread((void*)buffer, sizeof(uint8_t), size * sizeof(*buffer), file);

    if (size != res) {
        delete[] buffer;
        // TODO. Throw exception
        return false;
    }

    if (unpackPVRData(buffer)) {
        delete [] buffer;
        // TODO. Throw exception
        return false;
    }

    delete [] buffer;
    return true;
}

GLuint PVRTexture::CreateTexture(int x, int y, int w, int h)
{
    /* Create an OpenGL texture for thing */
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mImageDataLength[0], mImageData[0]);

    return texture;
}