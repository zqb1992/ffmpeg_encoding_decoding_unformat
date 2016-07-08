//本程序是最简单的视频编码程序
//只使用了avcodec文件
//
//
#include<stdio.h>
#define __STDC_CONSTANT_MACROS
extern "C"
{
	#include "libavutil/opt.h"
	#include "libavutil/imgutils.h"
	#include "libavcodec/avcodec.h"
}

int coding(const char *file_in,const char *file_out,AVCodecID codec_id,int p_width,int p_height)
{

	//egister all the codecs, parsers and bitstream filters
	avcodec_register_all();
	
	// Find a registered encoder with a matching codec ID
	AVCodec *pCodec = avcodec_find_encoder(codec_id);   
	if(!pCodec)
	{
		printf("Can't find the coder!");
		return -1;
	}

	//Allocate an AVCodecContext and set its fields to default values
	AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
	if(!pCodecCtx)
	{
		printf("Allocating an AVCodecContext is failed!");
		return -1;
	}

	//set the coder parameter
	pCodecCtx->coded_width = p_width;  
	pCodecCtx->coded_height = p_height;   //vedio frame size, in pixels

	pCodecCtx->bit_rate = 148000;    //average bit rate, 400kb/s    CBR
	pCodecCtx->rc_max_rate = 200000; //max bit rate
	pCodecCtx->rc_min_rate = 100000; //min bit rate
	//pCodecCtx->bit_rate_tolerance = 2000000;


	pCodecCtx->gop_size = 50;     //The biggest frames number between the key frames

	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;  //pixel format, namely what kind of color space is used to show the pixels
	
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;  //frame rate
	
	//pCodecCtx->max_b_frames = 2; // B frame number between I/P or P/P
	
	//if (codec_id == AV_CODEC_ID_H264)
 //       av_opt_set(pCodecCtx->priv_data, "preset", "slow", 0);

	//AVDictionary *param = NULL;	
	////H264, 设置为编码延迟为立即编码
	//if(pCodecCtx->codec_id == AV_CODEC_ID_H264)
	//{  
	//  av_dict_set(&param, "preset", "slow",   0);
	//  //av_dict_set(&param, "tune",   "zerolatency", 0);
	//}  
	////H.265  
	//if(pCodecCtx->codec_id == AV_CODEC_ID_H265)
	//{  
	//  //av_dict_set(&param, "x265-params", "qp=20", 0); 
	//  av_dict_set(&param, "preset", "ultrafast", 0);  
	//  //av_dict_set(&param, "tune", "zero-latency", 0); 
	//}  

	if(avcodec_open2(pCodecCtx,pCodec,NULL)<0)  //open coder
	{
		printf("Can't open the coder!");
		return -1;
	}
	
	AVFrame *pFrame = av_frame_alloc();   //Allocate an AVFrame
	if(!pFrame)
	{
		printf("Can't alloc an AVFrame!");
		return -1;
	}
	pFrame->format = pCodecCtx->pix_fmt;
	pFrame->width = pCodecCtx->width;
	pFrame->height = pCodecCtx->height;
	int ret = av_image_alloc(pFrame->data, pFrame->linesize,
							pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,16);
	if(ret<0)
	{
		printf("Could not allocate raw pFrame buffer\n");
		return -1;
	}

	//open input / output file
	FILE *fp_in = fopen(file_in,"rb");
	if(!fp_in)
	{
		printf("Can't open %s",file_in);
		return -1;
	}
	FILE *fp_out = fopen(file_out,"wb");
	if(!fp_out)
	{
		printf("Can't open %s",file_out);
		return -1;
	}
	int y_size = pCodecCtx->width*pCodecCtx->height;
	int get_picture,i=0; 
	int framecnt=0;
	AVPacket pkt;

	/*for(i=0;i<framenum;i++)*/
	while(1)
	{
		av_init_packet(&pkt);
		pkt.data = NULL;
		pkt.size = 0;

		if(fread(pFrame->data[0],1,y_size,fp_in)<=0||        //Y
		   fread(pFrame->data[1],1,y_size/4,fp_in)<=0||      //U
		   fread(pFrame->data[2],1,y_size/4,fp_in)<=0)       //V
		{
			break;
		}
		else if(feof(fp_in))
		{
			break;
		}

		pFrame->pts = i++; //用于解码后的视频帧显示  
		ret = avcodec_encode_video2(pCodecCtx,&pkt,pFrame,&get_picture);
		if(ret<0)
		{
			printf("error code on failure!");
			return -1;
		}
		if(get_picture)
		{
			printf("Succeed to encode frame: %5d\tsize:%5d\n",framecnt,pkt.size);
			framecnt++;
			fwrite(pkt.data,1,pkt.size,fp_out);
			av_packet_unref(&pkt);
		}
	}

	//fflush coder
	//for(get_picture=1;get_picture;)
	get_picture=1;
	i=0;
	while(1)
	{
		av_init_packet(&pkt);
		pkt.data = NULL;
		pkt.size = 0;

		ret = avcodec_encode_video2(pCodecCtx,&pkt,NULL,&get_picture);
		if(ret<0)
		{
			printf("encode fail!");
			return -1;
		}
		if(get_picture)
		{
			fwrite(pkt.data,1,pkt.size,fp_out);
			printf("encode frame: %5d\tsize:%5d\n",i++,pkt.size);
			av_packet_unref(&pkt);
		}
		else{
			break;
		}

	}

	fclose(fp_in);
	fclose(fp_out);

	av_freep(&pFrame->data[0]);
	av_frame_free(&pFrame);

	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);

	return 0;
}

