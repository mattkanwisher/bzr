#include "graphics/Image.h"
#include "graphics/ctmf.h"
#include "BlobReader.h"
#include "Core.h"
#include "DatFile.h"

PACK(struct TextureHeader
{
   uint32_t fileId;
   uint32_t unk;
   uint32_t width;
   uint32_t height;
   uint32_t type;
   uint32_t dataSize;
});

static vector<uint8_t> decodeDXT1(const void* data, int width, int height)
{
    throw runtime_error("DXT1 decoding not yet implemented");
}

static vector<uint8_t> decodeDXT5(const void* data, int width, int height)
{
    throw runtime_error("DXT5 decoding not yet implemented");
}

Image::Image() : _format(Invalid)
{}

Image::Image(Image&& other)
{
    _format = other._format;
    _data = move(other._data);
}

Image& Image::operator=(Image&& other)
{
    _format = other._format;
    _data = move(other._data);
    return *this;
}

void Image::create(Format format, int width, int height)
{
    _format = format;
    _width = width;
    _height = height;
    _data.clear();
    _data.resize(_width * _height * numChannels());
}

void Image::load(uint32_t fileId)
{
    auto blob = Core::get().highresDat().read(fileId);

    if(blob.empty())
    {
        blob = Core::get().portalDat().read(fileId);
    }

    if(blob.empty())
    {
        throw runtime_error("Texture not found");
    }

    BlobReader reader(blob.data(), blob.size());

    auto header = reader.readPointer<TextureHeader>();
 
    int bpp;

    if(header->type == 0x14) // BGR24
    {
        _format = BGR24;
        bpp = 24;
    }
    else if(header->type == 0x15) // BGRA32
    {
        _format = BGRA32;
        bpp = 32;
    }
    else if(header->type == 0x31545844) // DXT1
    {
        _format = BGR24;
        bpp = 4;
    }
    else if(header->type == 0x35545844) // DXT5
    {
        _format = BGRA32;
        bpp = 8;
    }
    else if(header->type == 0x65) // 16-bit paletted
    {
        throw runtime_error("Paletted texture not yet implemented");
    }
    else if(header->type == 0xf3) // RGB24
    {
        _format = RGB24;
        bpp = 24;
    }
    else if(header->type == 0xf4) // A8
    {
        _format = A8;
        bpp = 8;
    }
    else
    {
        throw runtime_error("Unrecognized texture type");
    }

    if(header->width * header->height * bpp / 8 != header->dataSize)
    {
        throw runtime_error("Texture dataSize mismatch");
    }

    auto data = reader.readPointer<uint8_t>(header->dataSize);

    reader.assertEnd();

    if(header->type == 0x31545844)
    {
        _data = decodeDXT1(data, header->width, header->height);
    }
    else if(header->type == 0x35545844)
    {
        _data = decodeDXT5(data, header->width, header->height);
    }
    else
    {
        _data.assign(data, data + header->dataSize);
    }

    _width = header->width;
    _height = header->height;
}

void Image::blit(const Image& image, int x, int y)
{
    if(_format != image._format)
    {
        throw runtime_error("Image format mismatch in blit");
    }

    if(x < 0 || x + image._width > _width || y < 0 || y + image._height > _height)
    {
        throw runtime_error("Image destination out of range");  
    }

    auto nchannels = numChannels();

    for(int row = 0; row < image._height; row++)
    {
        memcpy(_data.data() + (x + (y + row) * _width) * nchannels,
               image._data.data() + (row * image._width) * nchannels,
               image._width * nchannels);
    }
}

void Image::scale(float factor)
{
    assert(factor >= 1.0);

    int newWidth = float(_width) * factor;
    int newHeight = float(_height) * factor;
    int nchannels = numChannels();

    printf("newWidth = %d\n", newWidth);
    printf("newHeight = %d\n", newHeight);
    printf("nchannels = %d\n", nchannels);

    vector<uint8_t> newData(newWidth * newHeight * nchannels);

    //memset(newData.data(), 255, newData.size());

    for(auto dstY = 0; dstY < newHeight; dstY++)
    {
        for(auto dstX = 0; dstX < newWidth; dstX++)
        {
            auto srcFX = double(dstX) / double(newWidth) * double(_width);
            auto srcFY = double(dstY) / double(newHeight) * double(_height);

            auto srcX = (int)srcFX;
            auto srcY = (int)srcFY;

            auto xDiff = srcFX - srcX;
            auto yDiff = srcFY - srcY;

            auto xOpposite = 1.0 - xDiff;
            auto yOpposite = 1.0 - yDiff;

#define SRCPX(x, y, cn) (double)_data[((x) + (y) * _width) * nchannels + cn]
#define DSTPX(x, y, cn) newData[((x) + (y) * newWidth) * nchannels + cn]
            DSTPX(dstX, dstY, 0) =
                (SRCPX(srcX, srcY, 0) * xOpposite + SRCPX(srcX + 1, srcY, 0) * xDiff) * yOpposite +
                (SRCPX(srcX, srcY + 1, 0) * xOpposite + SRCPX(srcX + 1, srcY + 1, 0) * xDiff) * yDiff;
            DSTPX(dstX, dstY, 1) =
                (SRCPX(srcX, srcY, 1) * xOpposite + SRCPX(srcX + 1, srcY, 1) * xDiff) * yOpposite +
                (SRCPX(srcX, srcY + 1, 1) * xOpposite + SRCPX(srcX + 1, srcY + 1, 1) * xDiff) * yDiff;
            DSTPX(dstX, dstY, 2) =
                (SRCPX(srcX, srcY, 2) * xOpposite + SRCPX(srcX + 1, srcY, 2) * xDiff) * yOpposite +
                (SRCPX(srcX, srcY + 1, 2) * xOpposite + SRCPX(srcX + 1, srcY + 1, 2) * xDiff) * yDiff;
#undef SRCPX
#undef DSTPX
        }
    }

    _data = move(newData);
    _width = newWidth;
    _height = newHeight;
}

void Image::blur(int windowSize)
{
    vector<uint8_t> newData(_data.size());

    int stride = _width * numChannels();

    ctmf(_data.data(), newData.data(), _width, _height, stride, stride, windowSize, numChannels(), 512 * 1024);

    _data = move(newData);
}

Image::Format Image::format() const
{
    return _format;
}

int Image::numChannels() const
{
    switch(_format)
    {
        case A8:
            return 1;
        case BGR24:
        case RGB24:
            return 3;
        case BGRA32:
            return 4;
        default:
            assert(!"Invalid Image::Format");
            return -1;
    }
}

const void* Image::data() const
{
    return _data.data();
}

int Image::width() const
{
    return _width;
}

int Image::height() const
{
    return _height;
}