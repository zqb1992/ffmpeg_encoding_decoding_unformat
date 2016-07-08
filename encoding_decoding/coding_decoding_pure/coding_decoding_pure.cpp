// coding_decoding_pure.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "coding.h"
#include "decoding.h"

int _tmain(int argc, _TCHAR* argv[])
{
	//视频的分辨率和帧数
	int p_width=448;
	int p_height=336;
	//char *file_in = "../ds_480x272.yuv";
	//const char *file_in = "../bigbuckbunny_480x272.yuv";
	const char *file_in = "../QRGL.YUV";
	//h264编解码
	//const char *file_out = "ds.h264";
	//const char *file_out264 = "ds264.YUV";
	//AVCodecID codec_id = AV_CODEC_ID_H264;      
	//coding(file_in,file_out,codec_id,p_width,p_height);
	//decoding(file_out,file_out264,codec_id);

	//h265编解码
	const char *file_out = "ds.h265";
	const char *file_out265 = "ds265.YUV";
	AVCodecID codec_id = AV_CODEC_ID_H265;       
	coding(file_in,file_out,codec_id,p_width,p_height);
	decoding(file_out,file_out265,codec_id);


	return 0;
}

