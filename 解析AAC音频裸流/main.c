/**
 * 最简单的视音频数据处理示例
 * Simplest MediaData Test
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本项目包含如下几种视音频测试示例：
 *  (1)像素数据处理程序。包含RGB和YUV像素格式处理的函数。
 *  (2)音频采样数据处理程序。包含PCM音频采样格式处理的函数。
 *  (3)H.264码流分析程序。可以分离并解析NALU。
 *  (4)AAC码流分析程序。可以分离并解析ADTS帧。
 *  (5)FLV封装格式分析程序。可以将FLV中的MP3音频码流分离出来。
 *  (6)UDP-RTP协议分析程序。可以将分析UDP/RTP/MPEG-TS数据包。
 *
 * This project contains following samples to handling multimedia data:
 *  (1) Video pixel data handling program. It contains several examples to handle RGB and YUV data.
 *  (2) Audio sample data handling program. It contains several examples to handle PCM data.
 *  (3) H.264 stream analysis program. It can parse H.264 bitstream and analysis NALU of stream.
 *  (4) AAC stream analysis program. It can parse AAC bitstream and analysis ADTS frame of stream.
 *  (5) FLV format analysis program. It can analysis FLV file and extract MP3 audio stream.
 *  (6) UDP-RTP protocol analysis program. It can analysis UDP/RTP/MPEG-TS Packet.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
 
int getADTSframe(unsigned char* buffer, int buf_size, unsigned char* data ,int* data_size){
	int size = 0;
 
	if(!buffer || !data || !data_size ){
		return -1;
	}
 
	while(1){

		// ADTS Header的大小最小为7字节
		if(buf_size  < 7 ){
			return -1;
		}
		//Sync words， 同步字， 固定为0xFFF,占用12位，另外一个字节的高四位为1111
		// ADTS Header 数据类似于：FF F9 50 80 2E 7F FC
		if((buffer[0] == 0xff) && ((buffer[1] & 0xf0) == 0xf0) ){
			
			// 解析ADTS frame的大小(包括 ADTS Header 和 AAC Data)
			size |= ((buffer[3] & 0x03) <<11);     //high 2 bit
			size |= buffer[4]<<3;                //middle 8 bit
			size |= ((buffer[5] & 0xe0)>>5);        //low 3bit
			break;
		}
		--buf_size;
		++buffer;
	}
 
	// buf_size 为当前读取的文件大小， size 为 ADTS frame 的大小
	// 如果当前读取的文件大小小于 ADTS frame 的大小，则说明 ADTS frame 不完整，需要继续读取
	if(buf_size < size){
		return 1;
	}
 
	memcpy(data, buffer, size);
	*data_size = size;
 
	return 0;
}
 
int simplest_aac_parser(char *url)
{
	int data_size = 0;
	int size = 0;
	int cnt=0;
	int offset=0;
 
	//FILE *myout=fopen("output_log.txt","wb+");
	FILE *myout=stdout;
 
	unsigned char *aacframe=(unsigned char *)malloc(1024*5);
	unsigned char *aacbuffer=(unsigned char *)malloc(1024*1024);
 
	FILE *ifile = fopen(url, "rb");
	if(!ifile){
		printf("Open file error");
		return -1;
	}
 
	printf("-----+- ADTS Frame Table -+------+\n");
	printf(" NUM | Profile | Frequency| Size |\n");
	printf("-----+---------+----------+------+\n");
 
	while(!feof(ifile)){
		data_size = fread(aacbuffer+offset, 1, 1024*1024-offset, ifile);
		unsigned char* input_data = aacbuffer;
 
		while(1)
		{
			// 该函数对ADTS frame进行解析，并返回ADTS frame的大小
			// 如果返回-1，则说明ADTS frame不完整，需要继续读取
			// 如果返回1，则说明ADTS frame完整，可以进行处理
			// 如果返回0，则说明ADTS frame已经处理完毕，aacframe保存了ADTS frame的数据，size保存了ADTS frame的大小
			int ret=getADTSframe(input_data, data_size, aacframe, &size);
			if(ret==-1){
				break;
			}else if(ret==1){
				memcpy(aacbuffer,input_data,data_size);
				offset=data_size;
				break;
			}
 
			char profile_str[10]={0};
			char frequence_str[10]={0};
 
			// ADTS frame的profile字段 保存在 aacframe[2]的高两位
			// 和0xC0进行与运算(1100 0000)，然后右移6位，得到profile的值
			unsigned char profile=aacframe[2]&0xC0;
			profile=profile>>6;
			switch(profile){
			case 0: sprintf(profile_str,"Main");break;
			case 1: sprintf(profile_str,"LC");break;
			case 2: sprintf(profile_str,"SSR");break;
			default:sprintf(profile_str,"unknown");break;
			}

			// ADTS frame的采样率字段 保存在 aacframe[2]的中间四位
			// 和0x3C进行与运算(0011 1100)，然后右移2位，得到采样率的值
			unsigned char sampling_frequency_index=aacframe[2]&0x3C;
			sampling_frequency_index=sampling_frequency_index>>2;
			switch(sampling_frequency_index){
			case 0: sprintf(frequence_str,"96000Hz");break;
			case 1: sprintf(frequence_str,"88200Hz");break;
			case 2: sprintf(frequence_str,"64000Hz");break;
			case 3: sprintf(frequence_str,"48000Hz");break;
			case 4: sprintf(frequence_str,"44100Hz");break;
			case 5: sprintf(frequence_str,"32000Hz");break;
			case 6: sprintf(frequence_str,"24000Hz");break;
			case 7: sprintf(frequence_str,"22050Hz");break;
			case 8: sprintf(frequence_str,"16000Hz");break;
			case 9: sprintf(frequence_str,"12000Hz");break;
			case 10: sprintf(frequence_str,"11025Hz");break;
			case 11: sprintf(frequence_str,"8000Hz");break;
			default:sprintf(frequence_str,"unknown");break;
			}
 
 
			fprintf(myout,"%5d| %8s|  %8s| %5d|\n",cnt,profile_str ,frequence_str,size);
			data_size -= size;
			input_data += size;
			cnt++;
		}   
 
	}
	fclose(ifile);
	free(aacbuffer);
	free(aacframe);
 
	return 0;
}

int main(int argc, char *argv[])
{
    simplest_aac_parser("nocturne.aac");
    return 0;
}