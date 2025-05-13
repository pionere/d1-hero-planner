#include "d1cl2frame.h"

#include <QApplication>
#include <QDataStream>

#include "progressdialog.h"

int D1Cl2Frame::load(D1GfxFrame &frame, const QByteArray rawData, const OpenAsParam &params)
{
    unsigned width = 0;
    bool clipped = false;
    frame.width = width;
    clipped = true;

    // check if a positive width was found
    if (frame.width == 0)
        return rawData.size() == 0 ? (clipped ? 1 : 0) : -1;

    // READ {CL2 FRAME DATA}
    int frameDataStartOffset = 0;
    if (clipped) {
        if (rawData.size() != 0) {
            if (rawData.size() == 1)
                return -2;
            frameDataStartOffset = SwapLE16(*(const quint16 *)rawData.constData());
            if (frameDataStartOffset > rawData.size())
                return -2;
        }
    }

    std::vector<std::vector<D1GfxPixel>> pixels;
    std::vector<D1GfxPixel> pixelLine;
    for (int o = frameDataStartOffset; o < rawData.size(); o++) {
        quint8 readByte = rawData[o];

        if (/*readByte >= 0x00 &&*/ readByte < 0x80) {
            // Transparent pixels
            if (readByte == 0x00) {
                dProgressWarn() << QApplication::tr("Invalid CL2 frame data (0x00 found)");
            }
            for (int i = 0; i < readByte; i++) {
                // Add transparent pixel
                pixelLine.push_back(D1GfxPixel::transparentPixel());

                if (pixelLine.size() == frame.width) {
                    pixels.push_back(std::move(pixelLine));
                    pixelLine.clear();
                }
            }
        } else if (/*readByte >= 0x80 &&*/ readByte < 0xBF) {
            // RLE encoded palette index
            // Go to the palette index offset
            o++;

            for (int i = 0; i < (0xBF - readByte); i++) {
                // Add opaque pixel
                pixelLine.push_back(D1GfxPixel::colorPixel(rawData[o]));

                if (pixelLine.size() == frame.width) {
                    pixels.push_back(std::move(pixelLine));
                    pixelLine.clear();
                }
            }
        } else /*if (readByte >= 0xBF && readByte <= 0xFF)*/ {
            // Palette indices
            for (int i = 0; i < (256 - readByte); i++) {
                // Go to the next palette index offset
                o++;
                // Add opaque pixel
                pixelLine.push_back(D1GfxPixel::colorPixel(rawData[o]));

                if (pixelLine.size() == frame.width) {
                    pixels.push_back(std::move(pixelLine));
                    pixelLine.clear();
                }
            }
        }
    }
    if (!pixelLine.empty()) {
        return -2;
    }

    for (auto it = pixels.rbegin(); it != pixels.rend(); ++it) {
        frame.pixels.push_back(std::move(*it));
    }
    frame.height = frame.pixels.size();
    return clipped ? 1 : 0;
}
