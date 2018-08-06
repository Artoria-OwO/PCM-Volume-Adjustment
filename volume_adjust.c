/*
    使用说明：执行时输入三个参数，用空格分开
    第一个参数：int型，范围1-10，小于五为降低音量，反之为升高
    第二个参数：输入音频，为pcm
    第三个参数：输出音频，为pcm
    程序会自动执行到dB值合适为止
*/
//#define OLD_FILE_PATH "2.pcm"
//这里定义修改前的文件名
//#define VOL_FILE_PATH "out2.pcm"
//这里是修改后的文件名
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//参数为数据，采样个数
//返回值为分贝
#define VOLUMEMAX   32767

//只能对.pcm音频文件操作

char OLD_FILE_PATH[100];
char VOL_FILE_PATH[100];

int file_size(const char* filename)
{
    FILE *fp=fopen(filename,"r");
    if(!fp) return -1;
    fseek(fp,0,SEEK_END);
    int size=ftell(fp);
    fclose(fp);
    return size;
}

/*int SimpleCalculate_DB(short* pcmData, int sample)
{
    signed short ret = 0;
    if (sample > 0){
        int sum = 0;
        signed short* pos = (signed short *)pcmData;
        printf("%hd %hd\n",*pos,*pcmData);
        for (int i = 0; i < sample; i++){
            sum += abs(*pos);
            pos++;
        }
        ret = sum * 500.0 / (sample * VOLUMEMAX);
        if (ret >= 100){
            ret = 100;
        }
   }
   return ret;
}
*/

/**
 * 获取所有振幅之平均值 计算db (振幅最大值 2^16-1 = 65535 最大值是 96.32db)
 * 16 bit == 2字节 == short int
 * 无符号16bit：96.32=20*lg(65535);
 * 
 * @param pcmdata 转换成char类型，才可以按字节操作
 * @param size pcmdata的大小
 * @return
 */
int getPcmDB(const unsigned char *pcmdata, size_t size) {
 
    int db = 0;
    short int value = 0;
    double sum = 0;
    for(int i = 0; i < size; i += 2)
    {
        memcpy(&value, pcmdata+i, 2); //获取2个字节的大小（值）
        sum += abs(value); //绝对值求和
    }
    sum = sum / (size / 2); //求平均值（2个字节表示一个振幅，所以振幅个数为：size/2个）
    if(sum > 0)
    {
        db = (int)(20.0*log10(sum));
    }
    return db;
}


int volume_adjust(short  * in_buf, short  * out_buf, float in_vol)
{
    int i, tmp;
 
    // in_vol[0, 100]
    float vol = in_vol - 98;
 
    if(-98<vol && vol<0)
        vol = 1/(vol*(-1));
    else if(0<=vol && vol<=1)
        vol = 5;
/*    
    else if(1<=vol && vol<=2)
        vol = 20;
*/    
    else if(vol<=-98)
        vol = 0;
    else if(vol>=2)
        vol = 10;  //这个值可以根据你的实际情况去调整

    tmp = (*in_buf)*(in_vol/5); // 上面所有关于vol的判断，其实都是为了此处*in_buf乘以一个倍数，你可以根据自己的需要去修改
 
    // 下面的code主要是为了溢出判断
    if(tmp > 32767)
        tmp = 32767;
    else if(tmp < -32768)
        tmp = -32768;
    *out_buf = tmp;

    return 0;
}
 
void pcm_volume_control(int volume)
{
    short s16In = 0;
    short s16Out = 0;
    
    int size = 0;
 
    FILE *fp = fopen(OLD_FILE_PATH, "rb+");
    FILE *fp_vol = fopen(VOL_FILE_PATH, "wb+");
 
    while(!feof(fp))
    {
        size = fread(&s16In, 2, 1, fp);
        if(size>0)
        {        
            volume_adjust(&s16In, &s16Out, volume);
            fwrite(&s16Out, 2, 1, fp_vol);       
        }
    }

    

    fclose(fp);
    fclose(fp_vol);
}

int show_db(char *file)
{
    unsigned char *In;
    unsigned char *Out;

    FILE *fp = fopen(OLD_FILE_PATH, "r");
    FILE *fp_vol = fopen(VOL_FILE_PATH, "r");

    In = (unsigned char *)malloc(file_size(OLD_FILE_PATH)*sizeof(unsigned char));        
    Out = (unsigned char *)malloc(file_size(VOL_FILE_PATH)*sizeof(unsigned char));

    fseek(fp,0,SEEK_SET);
    fseek(fp_vol,0,SEEK_SET);
    
    fread(In, 2, file_size(OLD_FILE_PATH), fp);
    fread(Out, 2, file_size(VOL_FILE_PATH), fp_vol);

    fclose(fp);
    fclose(fp_vol);

    //printf("%d %d\n",getPcmDB(In,sizeof(In)),getPcmDB(Out,sizeof(Out)));
    if(strncmp(file,"In",2)==0)
    {
        return getPcmDB(In,sizeof(In));
    }
    else if(strncmp(file,"Out",3)==0)
    {
        return getPcmDB(Out,sizeof(Out));
    }
    else 
    {
        return -1;
    }
}

int main(int argc,char *argv[])
{
    if(argc==3)
    {
        float volume = 5.0;
        strcpy(OLD_FILE_PATH,argv[1]);
        strcpy(VOL_FILE_PATH,argv[2]);
        pcm_volume_control(volume);
        do
        {
            if(show_db((char *)"Out")-45 >= 0)
            {
                volume -= 0.75;
                pcm_volume_control(volume);
            }
            else if(show_db((char *)"Out")-45 < 0)
            {
                volume += 0.75;
                pcm_volume_control(volume);
            }
        }while(abs(show_db((char *)"Out")-40)>=2);
        printf("%d %d\n",show_db((char *)"In"),show_db((char *)"Out"));
    }
    return 0;
}

