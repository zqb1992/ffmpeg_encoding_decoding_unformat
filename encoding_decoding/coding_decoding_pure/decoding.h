//用来视频解码
//只使用了avcodec.lib
//

#include<stdio.h>
#define __STDC_CONSTANT_MACROS

extern "C"
{
	#include "libavcodec/avcodec.h"
	#include "libswscale/swscale.h"
}

int decoding(const char *file_in,const char *file_out,AVCodecID codec_id)
{

	avcodec_register_all();
	AVCodec *pCodec = avcodec_find_decoder(codec_id); //find decoder
	if(!pCodec)
	{
		printf("Can't find  decoder");
		return -1;
	}

	AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);  //alloc AVCodecContext
	if(!pCodecCtx)
	{
		printf("Allocating an AVCodecContext is failed!");
		return -1;
	}

	//AVCodecParser的初始化工作
	AVCodecParserContext *pCodecParserCtx = av_parser_init(codec_id);
	if (pCodec->capabilities & CODEC_CAP_TRUNCATED)
        pCodecCtx->flags |= CODEC_FLAG_TRUNCATED; // we do not send complete frames

	//打开解码器，返回0成功，负数失败
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}

	//打开输入文件
	FILE *fp_in = fopen(file_in,"rb");
	if(!fp_in)
	{
		printf("Can't open %s",file_in);
		return -1;
	}
	//打开输出文件
	FILE *fp_out = fopen(file_out,"wb");
	if(!fp_out)
	{
		printf("Can't open %s",file_out);
		return -1;
	}
	AVFrame *pFrame = av_frame_alloc();       //开辟AVFrame结构体内存，使用av_frame_free()释放内存
	AVFrame *pFrameYUV = av_frame_alloc();
	pFrameYUV->width = 480;
	pFrameYUV->height = 272;

	AVPacket pkt;
	av_init_packet(&pkt);

	int frame_cnt=0;
	int y_size=0;

	//设置缓冲，用来存储每次读入的数据
	const int in_buffer_size=4096;
	uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE]={0};
	uint8_t *out_buffer;

	int len,ren;
	int got_picture,first_time=1;

	int cur_size;
	uint8_t *cur_ptr;
	struct SwsContext *img_convert_ctx;

	while(1)
	{
		cur_size = fread(in_buffer,1,in_buffer_size,fp_in);  //读入数据
		if(cur_size==0)
		{
			break;
		}
		cur_ptr = in_buffer;        //数据存储区

		while(cur_size>0)
		{
			//Parse a packet
			//能得到视频的宽和高
			len = av_parser_parse2(
				pCodecParserCtx, pCodecCtx,
				&pkt.data, &pkt.size,
				cur_ptr , cur_size ,
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

			cur_ptr += len;
			cur_size -= len;
			
			if (pkt.size == 0) 
			{
				continue;
			}

			//decode an AVPacket
			ren = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &pkt);		
			if (ren < 0) {
				printf("Decode Error.(解码错误)\n");
				return ren;
			}
			if(got_picture)
			{
				printf("Decoded frame index: %d\n",frame_cnt);
				if(first_time)
				{
					img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
						pCodecCtx->width, pCodecCtx->height,AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 		

					out_buffer=(uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
					avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);					
					y_size=pCodecCtx->width*pCodecCtx->height;
					first_time=0;
				}

				/*
				 * 在此处添加输出YUV的代码
				 * 取自于pFrameYUV，使用fwrite()
				 */
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
					pFrameYUV->data, pFrameYUV->linesize);
				//存储解码后的包
				fwrite(pFrameYUV->data[0],1,y_size,fp_out);
				fwrite(pFrameYUV->data[1],1,y_size/4,fp_out);
				fwrite(pFrameYUV->data[2],1,y_size/4,fp_out);
				frame_cnt++;
			}
		}

	}

	//flush decoder
	pkt.data = NULL;
    pkt.size = 0;
	while(1){
		ren = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &pkt);
		if (ren < 0) {
			break;
		}
		if (!got_picture)
			break;
		if (got_picture) {
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
				pFrameYUV->data, pFrameYUV->linesize);
			printf("Flush Decoder: Succeed to decode 1 frame!\n");
			fwrite(pFrameYUV->data[0],1,y_size,fp_out);     //Y
			fwrite(pFrameYUV->data[1],1,y_size/4,fp_out);   //U
			fwrite(pFrameYUV->data[2],1,y_size/4,fp_out);   //V
		}
	}

	fclose(fp_in);
	fclose(fp_out);    //关闭文件

	av_frame_free(&pFrame);     
	av_frame_free(&pFrameYUV);  

	avcodec_close(pCodecCtx);      //关闭解码器
	
	sws_freeContext(img_convert_ctx);
	av_parser_close(pCodecParserCtx);


	return 0;
}






