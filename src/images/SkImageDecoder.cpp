/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkImageDecoder.h"
#include "SkBitmap.h"
#include "SkImagePriv.h"
#include "SkPixelRef.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkCanvas.h"
#include "Img_En.h"
#include <cutils/properties.h>
#ifdef FS_WRITE
#undef FS_WRITE 
#endif

#ifdef TIME_SAT
#include <sys/time.h>
#endif

static int img_enhance;
SK_DEFINE_INST_COUNT(SkImageDecoder::Peeker)
SK_DEFINE_INST_COUNT(SkImageDecoder::Chooser)
SK_DEFINE_INST_COUNT(SkImageDecoderFactory)

static SkBitmap::Config gDeviceConfig = SkBitmap::kNo_Config;

SkBitmap::Config SkImageDecoder::GetDeviceConfig()
{
    return gDeviceConfig;
}

void SkImageDecoder::SetDeviceConfig(SkBitmap::Config config)
{
    gDeviceConfig = config;
}

///////////////////////////////////////////////////////////////////////////////

SkImageDecoder::SkImageDecoder()
    : fPeeker(NULL)
    , fChooser(NULL)
    , fAllocator(NULL)
    , fSampleSize(1)
    , fDoEnhance(0)
    , fDefaultPref(SkBitmap::kNo_Config)
    , fDitherImage(true)
    , fUsePrefTable(false)
    , fSkipWritingZeroes(false)
    , fPreferQualityOverSpeed(false)
    , fRequireUnpremultipliedColors(false) {
}

SkImageDecoder::~SkImageDecoder() {
    SkSafeUnref(fPeeker);
    SkSafeUnref(fChooser);
    SkSafeUnref(fAllocator);
}

void SkImageDecoder::copyFieldsToOther(SkImageDecoder* other) {
    if (NULL == other) {
        return;
    }
    other->setPeeker(fPeeker);
    other->setChooser(fChooser);
    other->setAllocator(fAllocator);
    other->setSampleSize(fSampleSize);
    other->setDoEnhance(fDoEnhance);
    if (fUsePrefTable) {
        other->setPrefConfigTable(fPrefTable);
    } else {
        other->fDefaultPref = fDefaultPref;
    }
    other->setDitherImage(fDitherImage);
    other->setSkipWritingZeroes(fSkipWritingZeroes);
    other->setPreferQualityOverSpeed(fPreferQualityOverSpeed);
    other->setRequireUnpremultipliedColors(fRequireUnpremultipliedColors);
}

SkImageDecoder::Format SkImageDecoder::getFormat() const {
    return kUnknown_Format;
}

const char* SkImageDecoder::getFormatName() const {
    return GetFormatName(this->getFormat());
}

const char* SkImageDecoder::GetFormatName(Format format) {
    switch (format) {
        case kUnknown_Format:
            return "Unknown Format";
        case kBMP_Format:
            return "BMP";
        case kGIF_Format:
            return "GIF";
        case kICO_Format:
            return "ICO";
        case kJPEG_Format:
            return "JPEG";
        case kPNG_Format:
            return "PNG";
        case kWBMP_Format:
            return "WBMP";
        case kWEBP_Format:
            return "WEBP";
        default:
            SkASSERT(!"Invalid format type!");
    }
    return "Unknown Format";
}

SkImageDecoder::Peeker* SkImageDecoder::setPeeker(Peeker* peeker) {
    SkRefCnt_SafeAssign(fPeeker, peeker);
    return peeker;
}

SkImageDecoder::Chooser* SkImageDecoder::setChooser(Chooser* chooser) {
    SkRefCnt_SafeAssign(fChooser, chooser);
    return chooser;
}

SkBitmap::Allocator* SkImageDecoder::setAllocator(SkBitmap::Allocator* alloc) {
    SkRefCnt_SafeAssign(fAllocator, alloc);
    return alloc;
}

void SkImageDecoder::setSampleSize(int size) {
    if (size < 1) {
        size = 1;
    }
    fSampleSize = size;
}

void SkImageDecoder::setDoEnhance(int doEnhance) {
    
    fDoEnhance = doEnhance;
}

bool SkImageDecoder::chooseFromOneChoice(SkBitmap::Config config, int width,
                                         int height) const {
    Chooser* chooser = fChooser;

    if (NULL == chooser) {    // no chooser, we just say YES to decoding :)
        return true;
    }
    chooser->begin(1);
    chooser->inspect(0, config, width, height);
    return chooser->choose() == 0;
}

bool SkImageDecoder::allocPixelRef(SkBitmap* bitmap,
                                   SkColorTable* ctable) const {
    return bitmap->allocPixels(fAllocator, ctable);
}

///////////////////////////////////////////////////////////////////////////////

void SkImageDecoder::setPrefConfigTable(const SkBitmap::Config pref[6]) {
    if (NULL == pref) {
        fUsePrefTable = false;
    } else {
        fUsePrefTable = true;
        fPrefTable.fPrefFor_8Index_NoAlpha_src = pref[0];
        fPrefTable.fPrefFor_8Index_YesAlpha_src = pref[1];
        fPrefTable.fPrefFor_8Gray_src = SkBitmap::kNo_Config;
        fPrefTable.fPrefFor_8bpc_NoAlpha_src = pref[4];
        fPrefTable.fPrefFor_8bpc_YesAlpha_src = pref[5];
    }
}

void SkImageDecoder::setPrefConfigTable(const PrefConfigTable& prefTable) {
    fUsePrefTable = true;
    fPrefTable = prefTable;
}

SkBitmap::Config SkImageDecoder::getPrefConfig(SrcDepth srcDepth,
                                               bool srcHasAlpha) const {
    SkBitmap::Config config = SkBitmap::kNo_Config;

    if (fUsePrefTable) {
        switch (srcDepth) {
            case kIndex_SrcDepth:
                config = srcHasAlpha ? fPrefTable.fPrefFor_8Index_YesAlpha_src
                                     : fPrefTable.fPrefFor_8Index_NoAlpha_src;
                break;
            case k8BitGray_SrcDepth:
                config = fPrefTable.fPrefFor_8Gray_src;
                break;
            case k32Bit_SrcDepth:
                config = srcHasAlpha ? fPrefTable.fPrefFor_8bpc_YesAlpha_src
                                     : fPrefTable.fPrefFor_8bpc_NoAlpha_src;
                break;
        }
    } else {
        config = fDefaultPref;
    }

    if (SkBitmap::kNo_Config == config) {
        config = SkImageDecoder::GetDeviceConfig();
    }
    return config;
}

///////////////////////////////////////////////////////////////////////////////
void SkImageDecoder::image_scale(scale_param_t *dst, scale_param_t *src) {
    unsigned int hs,hd;
    unsigned int ws,wd;
    unsigned int src_stride;
    unsigned int i,j;
    unsigned int h_sample;
    unsigned int v_sample;
    unsigned int t1,t2,tx,ty;

    u_scale *pSrc;
    u_scale *pDest;
    ws = src->width;
    hs = src->height;
    wd = dst->width;
    hd = dst->height;

    src_stride = src->stride;
    pDest = (u_scale *)dst->addr;
    h_sample = (1<<SHIFTVAL)*hs/hd;
    v_sample = (1<<SHIFTVAL)*ws/wd;
    t1 = 0;

    for (i = 0; i < hd; i++) {
    	ty = t1 >> SHIFTVAL;
    	t1 += h_sample;
    	pSrc = (u_scale *)(src->addr + ty*src_stride);
    	t2 = 0;

    	for (j = 0; j < wd; j++) {
    	    tx = t2 >> SHIFTVAL;
    	    t2 += v_sample;
    	    *pDest++ = *(pSrc + tx);
        }
    }
    return;
}

int SkImageDecoder::image_enhance_func(int w, int h, SkBitmap::Config config, unsigned char *src) {
    int rt = 0;
    void *en_handle;
    En_Parm_t en_private;
    En_Input_t en_input;

    if (w*h < 4096) { //64*64
       SkDebugf("don't do enhance for small image...");
       return 0;
    }

    if (config == SkBitmap::kARGB_8888_Config)
       en_input.data_format = EN_FORMAT_ABGR8888;
    else
       en_input.data_format = EN_FORMAT_RGB565;

    en_input.media_format = EN_MEDIA_IMAGE;
    en_input.width = w;
    en_input.height = h;
    en_handle = ImgEn_Open(&en_input);

    if (en_handle == NULL) {
        return -1;
    }
    en_private.src = src;
    en_private.dst = src; 
    rt = ImgEn_Run(en_handle,&en_private);
    ImgEn_Close(en_handle);
    return rt;
}

bool SkImageDecoder::decode(SkStream* stream, SkBitmap* bm,
                            SkBitmap::Config pref, Mode mode) {
    // we reset this to false before calling onDecode
    fShouldCancelDecode = false;
    // assign this, for use by getPrefConfig(), in case fUsePrefTable is false
    fDefaultPref = pref;
    img_enhance = this->getDoEnhance();
    SkBitmap::Config config;
    struct timeval start, middle,end;
    // pass a temporary bitmap, so that if we return false, we are assured of
    // leaving the caller's bitmap untouched.
    SkBitmap    tmp;
    int width,height;
    unsigned char *enhance_src = NULL;
    int sdram_cap;
    char value[PROPERTY_VALUE_MAX];
    property_get("system.ram.total", value, "1024");
	sdram_cap=atoi(value);
#ifdef TIME_SAT    
    gettimeofday(&start,NULL);
#endif
    if (!this->onDecode(stream, &tmp, mode)) {
        return false;
    }
#ifdef TIME_SAT    
    gettimeofday(&middle,NULL);
#endif
    config = tmp.getConfig();
    if (img_enhance == 1 && sdram_cap >= SDRAM_SUPPORT && mode == SkImageDecoder::kDecodePixels_Mode \
        && (config == SkBitmap::kRGB_565_Config || config == SkBitmap::kARGB_8888_Config)) {
       SkAutoLockPixels alp(tmp);       
       enhance_src = (unsigned char *)tmp.getPixels();
       width = tmp.width();
       height = tmp.height();

       if (image_enhance_func(width, height, config, enhance_src)) {
            SkDebugf("image_enhance failed at %d...",__LINE__);
       }
    }
#ifdef TIME_SAT
    gettimeofday(&end,NULL);
    SkDebugf("total:%d ms deocde:%d ms enhance:%d ms",1000*(end.tv_sec-start.tv_sec)+(end.tv_usec-start.tv_usec)/1000, 1000*(middle.tv_sec-start.tv_sec)+(middle.tv_usec-start.tv_usec)/1000, \
            1000*(end.tv_sec-middle.tv_sec)+(end.tv_usec-middle.tv_usec)/1000);
#endif
    bm->swap(tmp);
    return true;
}

bool SkImageDecoder::decodeSubset(SkBitmap* bm, const SkIRect& rect,
                                  SkBitmap::Config pref) {
    // we reset this to false before calling onDecodeSubset
    fShouldCancelDecode = false;
    // assign this, for use by getPrefConfig(), in case fUsePrefTable is false
    fDefaultPref = pref;

    return this->onDecodeSubset(bm, rect);
}

bool SkImageDecoder::buildTileIndex(SkStream* stream,
                                int *width, int *height) {
    // we reset this to false before calling onBuildTileIndex
    this->setDoEnhance(img_enhance);
    fShouldCancelDecode = false;

    return this->onBuildTileIndex(stream, width, height);
}

bool SkImageDecoder::cropBitmap(SkBitmap *dst, SkBitmap *src, int sampleSize,
                                int dstX, int dstY, int width, int height,
                                int srcX, int srcY) {
    int w = width / sampleSize;
    int h = height / sampleSize;
    if (src->getConfig() == SkBitmap::kIndex8_Config) {
        // kIndex8 does not allow drawing via an SkCanvas, as is done below.
        // Instead, use extractSubset. Note that this shares the SkPixelRef and
        // SkColorTable.
        // FIXME: Since src is discarded in practice, this holds on to more
        // pixels than is strictly necessary. Switch to a copy if memory
        // savings are more important than speed here. This also means
        // that the pixels in dst can not be reused (though there is no
        // allocation, which was already done on src).
        int x = (dstX - srcX) / sampleSize;
        int y = (dstY - srcY) / sampleSize;
        SkIRect subset = SkIRect::MakeXYWH(x, y, w, h);
        return src->extractSubset(dst, subset);
    }
    // if the destination has no pixels then we must allocate them.
    if (dst->isNull()) {
        dst->setConfig(src->getConfig(), w, h);
        dst->setIsOpaque(src->isOpaque());

        if (!this->allocPixelRef(dst, NULL)) {
            SkDEBUGF(("failed to allocate pixels needed to crop the bitmap"));
            return false;
        }
    }
    // check to see if the destination is large enough to decode the desired
    // region. If this assert fails we will just draw as much of the source
    // into the destination that we can.
    if (dst->width() < w || dst->height() < h) {
        SkDEBUGF(("SkImageDecoder::cropBitmap does not have a large enough bitmap.\n"));
    }

    // Set the Src_Mode for the paint to prevent transparency issue in the
    // dest in the event that the dest was being re-used.
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);

    SkCanvas canvas(*dst);
    canvas.drawSprite(*src, (srcX - dstX) / sampleSize,
                            (srcY - dstY) / sampleSize,
                            &paint);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool SkImageDecoder::DecodeFile(const char file[], SkBitmap* bm,
                            SkBitmap::Config pref,  Mode mode, Format* format) {
    SkASSERT(file);
    SkASSERT(bm);

    SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(file));
    if (stream.get()) {
        if (SkImageDecoder::DecodeStream(stream, bm, pref, mode, format)) {
            bm->pixelRef()->setURI(file);
            return true;
        }
    }
    return false;
}

bool SkImageDecoder::DecodeMemory(const void* buffer, size_t size, SkBitmap* bm,
                          SkBitmap::Config pref, Mode mode, Format* format) {
    if (0 == size) {
        return false;
    }
    SkASSERT(buffer);

    SkMemoryStream  stream(buffer, size);
    return SkImageDecoder::DecodeStream(&stream, bm, pref, mode, format);
}

/**
 *  Special allocator used by DecodeMemoryToTarget. Uses preallocated memory
 *  provided if the bm is 8888. Otherwise, uses a heap allocator. The same
 *  allocator will be used again for a copy to 8888, when the preallocated
 *  memory will be used.
 */
class TargetAllocator : public SkBitmap::HeapAllocator {

public:
    TargetAllocator(void* target)
        : fTarget(target) {}

    virtual bool allocPixelRef(SkBitmap* bm, SkColorTable* ct) SK_OVERRIDE {
        // If the config is not 8888, allocate a pixelref using the heap.
        // fTarget will be used to store the final pixels when copied to
        // 8888.
        if (bm->config() != SkBitmap::kARGB_8888_Config) {
            return INHERITED::allocPixelRef(bm, ct);
        }
        // In kARGB_8888_Config, there is no colortable.
        SkASSERT(NULL == ct);
        bm->setPixels(fTarget);
        return true;
    }

private:
    void* fTarget;
    typedef SkBitmap::HeapAllocator INHERITED;
};

/**
 *  Helper function for DecodeMemoryToTarget. DecodeMemoryToTarget wants
 *  8888, so set the config to it. All parameters must not be null.
 *  @param decoder Decoder appropriate for this stream.
 *  @param stream Rewound stream to the encoded data.
 *  @param bitmap On success, will have its bounds set to the bounds of the
 *      encoded data, and its config set to 8888.
 *  @return True if the bounds were decoded and the bitmap is 8888 or can be
 *      copied to 8888.
 */
static bool decode_bounds_to_8888(SkImageDecoder* decoder, SkStream* stream,
                                  SkBitmap* bitmap) {
    SkASSERT(decoder != NULL);
    SkASSERT(stream != NULL);
    SkASSERT(bitmap != NULL);

    if (!decoder->decode(stream, bitmap, SkImageDecoder::kDecodeBounds_Mode)) {
        return false;
    }

    if (bitmap->config() == SkBitmap::kARGB_8888_Config) {
        return true;
    }

    if (!bitmap->canCopyTo(SkBitmap::kARGB_8888_Config)) {
        return false;
    }

    bitmap->setConfig(SkBitmap::kARGB_8888_Config, bitmap->width(), bitmap->height());
    return true;
}

/**
 *  Helper function for DecodeMemoryToTarget. Decodes the stream into bitmap, and if
 *  the bitmap is not 8888, then it is copied to 8888. Either way, the end result has
 *  its pixels stored in target. All parameters must not be null.
 *  @param decoder Decoder appropriate for this stream.
 *  @param stream Rewound stream to the encoded data.
 *  @param bitmap On success, will contain the decoded image, with its pixels stored
 *      at target.
 *  @param target Preallocated memory for storing pixels.
 *  @return bool Whether the decode (and copy, if necessary) succeeded.
 */
static bool decode_pixels_to_8888(SkImageDecoder* decoder, SkStream* stream,
                                  SkBitmap* bitmap, void* target) {
    SkASSERT(decoder != NULL);
    SkASSERT(stream != NULL);
    SkASSERT(bitmap != NULL);
    SkASSERT(target != NULL);

    TargetAllocator allocator(target);
    decoder->setAllocator(&allocator);

    bool success = decoder->decode(stream, bitmap, SkImageDecoder::kDecodePixels_Mode);
    decoder->setAllocator(NULL);

    if (!success) {
        return false;
    }

    if (bitmap->config() == SkBitmap::kARGB_8888_Config) {
        return true;
    }

    SkBitmap bm8888;
    if (!bitmap->copyTo(&bm8888, SkBitmap::kARGB_8888_Config, &allocator)) {
        return false;
    }

    bitmap->swap(bm8888);
    return true;
}

bool SkImageDecoder::DecodeMemoryToTarget(const void* buffer, size_t size,
                                          SkImage::Info* info,
                                          const SkBitmapFactory::Target* target) {
    if (NULL == info) {
        return false;
    }

    // FIXME: Just to get this working, implement in terms of existing
    // ImageDecoder calls.
    SkBitmap bm;
    SkMemoryStream stream(buffer, size);
    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(&stream));
    if (NULL == decoder.get()) {
        return false;
    }

    if (!decode_bounds_to_8888(decoder.get(), &stream, &bm)) {
        return false;
    }

    SkASSERT(bm.config() == SkBitmap::kARGB_8888_Config);

    // Now set info properly.
    // Since Config is SkBitmap::kARGB_8888_Config, SkBitmapToImageInfo
    // will always succeed.
    SkAssertResult(SkBitmapToImageInfo(bm, info));

    if (NULL == target) {
        return true;
    }

    if (target->fRowBytes != SkToU32(bm.rowBytes())) {
        if (target->fRowBytes < SkImageMinRowBytes(*info)) {
            SkASSERT(!"Desired row bytes is too small");
            return false;
        }
        bm.setConfig(bm.config(), bm.width(), bm.height(), target->fRowBytes);
    }

    // SkMemoryStream.rewind() will always return true.
    SkAssertResult(stream.rewind());
    return decode_pixels_to_8888(decoder.get(), &stream, &bm, target->fAddr);
}


bool SkImageDecoder::DecodeStream(SkStream* stream, SkBitmap* bm,
                          SkBitmap::Config pref, Mode mode, Format* format) {
    SkASSERT(stream);
    SkASSERT(bm);

    bool success = false;
    SkImageDecoder* codec = SkImageDecoder::Factory(stream);

    if (NULL != codec) {
        success = codec->decode(stream, bm, pref, mode);
        if (success && format) {
            *format = codec->getFormat();
            if (kUnknown_Format == *format) {
                if (stream->rewind()) {
                    *format = GetStreamFormat(stream);
                }
            }
        }
        delete codec;
    }
    return success;
}
