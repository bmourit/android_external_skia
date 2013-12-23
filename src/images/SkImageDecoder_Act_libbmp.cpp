/*
 * Copyright 2007, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *     http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */
#include "SkImageDecoder.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkStreamHelpers.h"
#include "SkColorPriv.h"
#include "SkTDArray.h"
#include "SkTRegistry.h"

extern "C" {
#include "bmpplugin.h"
}

class SkBMPImageDecoder : public SkImageDecoder {
public:
    SkBMPImageDecoder() {}
    
    virtual Format getFormat() const SK_OVERRIDE {
        return kBMP_Format;
    }

protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode mode) SK_OVERRIDE;

private:
	typedef SkImageDecoder INHERITED
};

///////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(BMPImageDecoder);
///////////////////////////////////////////////////////////////////////////////

static bool is_bmp(SkStream* stream) {
    static const char kBmpMagic[] = { 'B', 'M' };
    
    char buffer[sizeof(kBmpMagic)];
    
    return stream->read(buffer, sizeof(kBmpMagic)) == sizeof(kBmpMagic) &&
            !memcmp(buffer, kBmpMagic, sizeof(kBmpMagic));
}
static SkImageDecoder* sk_libbmp_dfactory(SkStream* stream) {
    if (is_bmp(stream)) {
        return SkNEW(SkBMPImageDecoder);
    }
    return NULL;
}

static SkTRegistry<SkImageDecoder*, SkStream*> gReg(sk_libbmp_dfactory);

static SkImageDecoder::Format get_format_bmp(SkStream* stream) {
    if (is_bmp(stream)) {
        return SkImageDecoder::kBMP_Format;
    }
    return SkImageDecoder::kUnknown_Format;
}

static SkTRegistry<SkImageDecoder::Format, SkStream*> gFormatReg(get_format_bmp);

///////////////////////////////////////////////////////////////////////////////
static int sk_read(struct bmp_struct_def *bmp_ptr, unsigned char *data, unsigned int length) {
    SkStream* sk_stream = (SkStream*)(bmp_ptr->io_ptr);
    unsigned int bytes = sk_stream->read(data, length);
    return bytes;
}

static void sk_seek(struct bmp_struct_def *bmp_ptr, unsigned int offset, int flag) {
    SkStream* sk_stream = (SkStream*)(bmp_ptr->io_ptr); 
    if(flag == 0)
        sk_stream->rewind();
    sk_stream->skip(offset);    
    return;
}

bool SkBMPImageDecoder::onDecode(SkStream* sk_stream, SkBitmap* bm, Mode mode) {	
    int width = 0;
    int	height = 0;
    int ret = 0;
    struct bmp_struct_def *bmp_ptr = NULL;
    struct bmp_head_s *bmp_header = NULL;
    SkBitmap::Config    config;
    bmp_ptr = bmp_creat_ptr();
    bmp_header = bmp_creat_header();
    bmp_set_read_fn(bmp_ptr, (void *)sk_stream, sk_read);
    bmp_set_seek_fn(bmp_ptr, sk_seek);
    ret = read_input_bmp_header(bmp_header,bmp_ptr);

    if (ret) {
        bmp_destory_ptr(bmp_ptr,bmp_header);
        return false;
    }

 	const int sampleSize = this->getSampleSize();
    	SkScaledBitmapSampler sampler(bmp_header->image_w, bmp_header->image_h, sampleSize);
	config = SkBitmap::kARGB_8888_Config;
    	bm->setConfig(config, sampler.scaledWidth(),sampler.scaledHeight(), 0);
	if (SkImageDecoder::kDecodeBounds_Mode == mode) {
	    bmp_destory_ptr(bmp_ptr,bmp_header);
	    return true;
	}

   	if (!this->allocPixelRef(bm, NULL)) {
   	    bmp_destory_ptr(bmp_ptr,bmp_header);
   		return false;
   	}
    SkAutoLockPixels alp(*bm);
	width  = bm->width();
	height = bm->height();
	uint8_t *dst = (unsigned char *)bm->getPixels();
	bmp_header->offset_w = sampleSize;
	bmp_header->offset_h = sampleSize;
	bmp_header->image_w_out = width;
	bmp_header->image_h_out = height;
	ret = decode_bmp(bmp_header, bmp_ptr, dst);	
	if (ret) {
	    bmp_destory_ptr(bmp_ptr,bmp_header);
	    return false;	
	}

	if (bmp_header->flag_swap) {
	    swap(dst, bmp_header->image_w_out, bmp_header->image_h_out, 4);
	}
	bmp_destory_ptr(bmp_ptr,bmp_header);
	return	true;
}
