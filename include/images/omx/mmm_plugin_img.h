/********************************************************************************
*                              usdk1100
*                            Module: act_decoder.c
*                 Copyright(c) 2003-2008 Actions Semiconductor,
*                            All Rights Reserved.
*
* History:
*      <author>    <time>           <version >             <desc>
*       jszeng    2009-01-05 10:00     1.0             build this file
********************************************************************************/
/*!
* \file     mmm_plugin_dev.h
* \brief 
* \author 
* \version 1.0
* \date  2010/08/31
*******************************************************************************/
#ifndef __MMM_PLUGIN_DEV_H__
#define	__MMM_PLUGIN_DEV_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef     NULL
#define		NULL		0
#endif

typedef enum
{
    /*! plugin for demuxer */
    MMM_PLUGIN_DEMUX = 0x01,
    /*! plugin for audio decoder */
    MMM_PLUGIN_AUDIO_DECODER,
    /*! plugin for video decoder */
    MMM_PLUGIN_VIDEO_DECODER,
    /*! plugin for img decoder */
    MMM_PLUGIN_IMAGE_DECODER,
    /*! plugin for muxer */
    MMM_PLUGIN_MUX,
    /*! plugin for audio encoder */
    MMM_PLUGIN_AUDIO_ENCODER,
    /*! plugin for video encoder */
    MMM_PLUGIN_VIDEO_ENCODER,
    /*! plugin for img encoder */
    MMM_PLUGIN_IMAGE_ENCODER,
    /*! plugin for image dec ipp */
    MMM_PLUGIN_IPP
}

mmm_plugin_type_t;

/*!
 * \brief
 *      get file information
 */
typedef struct
{
	char *extension;
	unsigned int file_len;
}

mmm_file_info_t;
/*!
 * \brief
 *      stream_input seek�ķ�ʽ
 */

#if 1
typedef enum
{
    MMM_DSEEK_SET=0x0,
    MMM_DSEEK_CUR,
    MMM_DSEEK_END
}
mmm_stream_seek_t;
#else
typedef enum
{
    MMM_DSEEK_SET=0x01,
    MMM_DSEEK_END,
    MMM_DSEEK_CUR
}
mmm_stream_seek_t;
#endif

/*!
 * \brief
 *      ���ݲ��������ṹ
 */
typedef struct mmm_stream_input_s
{
	void *io_ptr;
    /*! ������*/
	int (*read)(struct mmm_stream_input_s *input,unsigned int buf,unsigned int len);
	/*! д����*/
	int (*write)(struct mmm_stream_input_s *input,unsigned int buf,unsigned int len);
	/*! �ļ�ָ�붨λ*/
	int (*seek)(struct mmm_stream_input_s *input,int offset,int original);
	/*! ��ȡ��ǰ�ļ�λ��*/
	int (*tell)(struct mmm_stream_input_s *input);
	/*! ��ȡ�ļ���Ϣ*/
	int (*get_file_info)(struct mmm_stream_input_s *input,mmm_file_info_t *info);
	/*! �ͷ�stream_input_t�������*/
	int (*dispose)(struct mmm_stream_input_s *input);
	/*! ��ȡ���ݵĴ�С*/
	int (*get_data_size)(struct mmm_stream_input_s *input);
}mmm_stream_input_t;

/*!
 * \brief
 *  ��ִ��image decoder�����open����ʱ����
 */
typedef struct
{
    /*! ���ݲ������*/
	mmm_stream_input_t *input;
}mmm_imagedec_plugio_t;

/*!
 * \brief
 *  �ýṹ��¼�˲�ͬ�������Ϣ���м������ɲ������ʱʹ��
 */
typedef struct
{
    /*! �ļ�����*/
	char type;
	/*! �ļ���׺*/
	char *extension;
	/*! open���ʱ��Ҫ���ߵ�ǰ������������*/
	void *(*open)(void *plugio);
	/*! ��ȡ�ļ���Ϣ*/
	int (*get_file_info)(mmm_stream_input_t *input,void *file_info);
}mmm_plugin_info_t;

/*!
 * \brief
 *  ͼƬ���������ʽ
 */
typedef enum
{
    /*! RGB*/
    MMM_FMT_RGB=0x00,
    /*! ARGB*/
    MMM_FMT_ARGB,
    /*! YUV*/
    MMM_FMT_YUV
}mmm_img_dec_type_t;
/*!
 * \brief
 *  ͼƬ���������ɫ�ռ�
 */
typedef enum
{
	/*! RGB565*/
	PPRGB565 = 0,
	/*! RGB888*/
	PPRGB888,
	/*! ARGB565*/
	PPARGB5565,
	/*! ARGB888*/
	PPARGB8888,
	/*! YUV400*/
	PPYUV_400,
	/*! YUV420planar*/
	PPYUV_420,
	/*! YUV422planar��ͷ����������*/
	PPYUV_422,
	/*! YUV 420planar with alpha*/
	PPYUVA_420,
	/*! YUV422planar with alpha��ͷ����������*/
	PPYUVA_422,
	/*! YUV420semiplanar*/        
	PPYUV_SEMI_420,
	/*! YUV422semiplanar*/ 
	PPYUV_SEMI_422,
	/*! YUYV*/
	PPYCBYCR,
}mmm_img_color_t;

/*!
 * \brief
 *  ͼƬ��������������з�ʽ
 */
typedef enum
{
    /*! ȷʡģʽ*/
    MMM_DSP_DIR_DEFAULT=0x00,
    /*! ����ģʽ*/
    MMM_DSP_DIR_SIDELONG_FLIP,
    /*! ����ģʽ*/
    MMM_DSP_DIR_UPRIGHT_FLIP,
    /*! ����Ӧ����ʽ*/
    MMM_DSP_DIR_BOTH_FLIP
}mmm_img_data_array_type_t;
/*!
 * \brief
 *  ͼƬ����
 */
typedef struct
{
    /*! ��*/
	unsigned int year;
	/*! ��*/
	unsigned int month;
	/*! ��*/
	unsigned int day;
}mmm_image_date_t;
/*!
 * \brief
 *  ͼƬ�ļ���Ϣ
 */

typedef struct
{
	/*! ͼƬ���������ɫ�ռ�*/
	mmm_img_color_t color_space;
	/*! ��*/
	unsigned int width;
	/*! ��*/
	unsigned int height;
	/*! ����*/
	mmm_image_date_t date;
	/*! ������Ϣ*/
	unsigned char exif[2*1024];
	/*! JPG����ͼ��exif��Ϣ�е�λ��*/
	unsigned int exif_pos;
}mmm_image_file_info_t;

/*!
 * \brief
 *      YUV��Ϣ
 */
typedef struct 
{
	/*! Y��ַ*/
	unsigned char *y_buf;
	/*! Y��ַ����*/
	unsigned int y_buf_len;
	/*! U��ַ*/
	unsigned char *u_buf;
	/*! U��ַ����*/
	unsigned int u_buf_len;	
	/*! V��ַ*/
	unsigned char *v_buf;
	/*! V��ַ����*/
	unsigned int v_buf_len;	
	/*! alpha��ַ*/
	unsigned char *alpha_buf;
	/*! alpha��ַ����*/
	unsigned int alpha_buf_len;			
	/*! Y����stride*/
	unsigned int  y_stride;
	/*! U����stride*/
	unsigned int  u_stride;
	/*! V����stride*/
	unsigned int  v_stride;
	/*! alpha����stride*/
	unsigned int  alpha_stride;
}mmm_img_yuv_info_t; 

/*!
 * \brief
 *      RGB��Ϣ
 */
typedef struct 
{
	/*! rgb��ַ*/
	unsigned char *rgb_buf;
	/*! rgb��ַ����*/
	unsigned int  rgb_buf_len;
	/*! rgbstride*/		
	unsigned int  rgb_stride;	
}mmm_img_rgb_info_t;

/*!
 * \brief
 *      YCBYCR��Ϣ
 */
typedef struct 
{
	/*! YCBYCR��ַ*/
	unsigned char *ycbycr_buf;
	/*! YCBYCR��ַ����*/
	unsigned int  ycbycr_buf_len;
	/*! YCBYCR*/		
	unsigned int  ycbycr_stride;	
}mmm_img_ycbycr_info_t;

/*!
 * \brief
 *  ����ͼƬ�����Ҫ���������ͼƬ������������
 */
typedef struct
{
    /*! �����ɫ*/
	mmm_img_dec_type_t format;
	/*! �Ƿ�������ǿ*/
	unsigned int doEnhance;			
	/*! ʵ��ͼƬ�Ŀ�*/
	unsigned int width;
	/*! ʵ��ͼƬ�ĸ�*/
	unsigned int height;
	/*! �����x��ʼλ��*/
	int  xpos;
	/*! �����y��ʼλ��*/
	int  ypos;
	unsigned int phyaddr;	
	unsigned char *viraddr;
    union {
    	/*! RGB��Ϣ*/
        mmm_img_rgb_info_t rgb_info;
        /*! YUV��Ϣ*/
        mmm_img_yuv_info_t yuv_info;
	/*! YCBYCR��Ϣ*/
	mmm_img_ycbycr_info_t ycbycr_info;
    };
	/*! ʱ���*/
	unsigned int time_stamp;
	/*! JPG����ͼ��exif��Ϣ�е�λ��*/
	unsigned int exif_pos;	
}mmm_image_info_t;

/*!
 * \brief
 *  alphaͼƬ�����������
 */
typedef struct
{
    char *alpha_buffer;
    unsigned int alpha_buf_len;
}imgdec_alpha_param_t;
/*!
 * \brief
 *  ͼƬ������չ�ӿ�
 */
typedef enum
{
    MMM_SET_ALPHA_PARAM=0x00
}imgdec_ex_ops_t;
/*!
 * \brief
 *  ͼƬ������
 */
typedef struct mmm_image_plugin_s
{
    /*! ��ʼ��*/
	int (*init)(struct mmm_image_plugin_s *plugin);
	/*! ����*/
	int (*decode_img)(struct mmm_image_plugin_s *plugin,mmm_image_info_t *img);
	/*! ��չ�ӿ�*/
	int (*ex_ops)(struct mmm_image_plugin_s *plugin,int cmd,unsigned int arg);
	/*! �ͷŲ��*/
	int (*dispose)(struct mmm_image_plugin_s *plugin);
}mmm_image_plugin_t;

/*!
 * \brief
 *  ͼƬ�ƶ�����
 */
typedef enum
{
    /*! ����*/
    MMM_MOVE_UP=0x01,
    /*! ����*/
    MMM_MOVE_DOWN,
    /*! ����*/
    MMM_MOVE_LEFT,
    /*! ����*/
    MMM_MOVE_RIGHT
}mmm_img_move_type_t;
/*!
 * \brief
 *  ͼƬ��ת����
 */
typedef enum
{
    /*! ��ת90��*/
    MMM_ROTATE_LEFT90=0x01,
    /*! ��ת90��*/
    MMM_ROTATE_RIGHT90,
    /*! ��ת180��*/
    MMM_ROTATE_180
}mmm_img_rotate_type_t;
/*!
 * \brief
 *  ͼƬ���Ų���
 */
typedef enum
{
    /*! �Ŵ�*/
    MMM_ZOOM_IN=0x01,
    /*! ��С*/
    MMM_ZOOM_OUT
}mmm_img_zoom_type_t;
/*!
 * \brief
 *  ͼƬ�ԳƱ任��ʽ
 */
typedef enum
{
    /*! ˮƽ�ԳƱ任*/
    MMM_MIRROR_HORIZONTAL=0x01,
    /*! ���¶ԳƱ任*/
    MMM_MIRROR_VERTICAL
}mmm_img_mirror_type_t;
/*!
 * \brief
 *  ͼƬ����ipp����ṩ�ĺ���
 */
typedef struct mmm_ipp_proc_s
{
    /*! ��ʼ��ipp*/
	int (*init)(struct mmm_ipp_proc_s *handle);
	/*! ����ʱ�ĺ���任*/
	int (*ipp_convert)(struct mmm_ipp_proc_s *handle,mmm_image_info_t *img_in,mmm_image_info_t *img_out);//ipp����(�Ӵ�ͼbufת��Сͼbuf)������Ŀ��buf
	/*! �ƶ�*/
	int (*move)(struct mmm_ipp_proc_s *handle,unsigned int dir);
	/*! ��ת*/
	int (*rotate)(struct mmm_ipp_proc_s *handle,unsigned int dir);
	/*! ����*/
	int (*zoom)(struct mmm_ipp_proc_s *handle,unsigned int size);
	/*! �ԳƱ任*/
	int (*mirror)(struct mmm_ipp_proc_s *handle,unsigned int dir);
	/*! ��������*/
	int (*adjust_light)(struct mmm_ipp_proc_s *handle,unsigned int level);
	/*! ���¶�λ��С*/
	int (*resize)(struct mmm_ipp_proc_s *handle,mmm_image_info_t *img_out);
	/*! �ͷŲ��*/
	int (*dispose)(struct mmm_ipp_proc_s *handle);
}mmm_ipp_proc_t;
mmm_ipp_proc_t *mmm_ipp_open(void);

typedef struct 
{
	unsigned char *buf_start;
	unsigned int  buf_len;
	unsigned int  cur_len;
}buf_stream_info_t;

typedef struct {
	mmm_stream_input_t input;
	buf_stream_info_t *buf_info;
	mmm_image_plugin_t *p_imgInterface;
}actImage_caller_t;


#ifdef __cplusplus
}
#endif

#endif

