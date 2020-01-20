#include <reg52.h>
#include <intrins.h>
#define uchar unsigned char
#define uint  unsigned int
//定义Max7219端口
sbit Max7219_pinCLK = P0^0;
sbit Max7219_pinCS  = P0^1;
sbit Max7219_pinDIN = P0^2;



uchar code word[7][8]={
    {0x7E,0x2,0x2,0x7E,0x40,0x40,0x40,0x7E},//2
    {0x0,0x22,0x55,0x49,0x41,0x22,0x14,0x8},
    {0x7E,0x2,0x2,0x7E,0x40,0x40,0x40,0x7E},//2
    {0x0,0x22,0x55,0x49,0x41,0x22,0x14,0x8},
    {0x48,0xFE,0xEA,0xEA,0x5F,0x4C,0x52,0x61},
    {0x7E,0x48,0x48,0x7F,0x28,0x6A,0xC9,0x18},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

void N_Nop_(int n)
{
  int i=0;
  for(;i<n;i++)
    _nop_();
}

void Delay1ms()
{
  N_Nop_(445);
}

void DelayNms(int time)
{
  int i=0;
  for(;i<time;i++)
  {
    Delay1ms();
  }
}

//--------------------------------------------
//功能：向MAX7219(U3)写入字节
//入口参数：DATA 
//出口参数：无
//说明：
void Write_Max7219_byte(uchar DATA)         
{
  uchar i;    
  Max7219_pinCS=0;    
  for(i=8;i>=1;i--)
  {     
    Max7219_pinCLK=0;
    Max7219_pinDIN=DATA&0x80;
    DATA=DATA<<1;
    Max7219_pinCLK=1;
  }                                 
}
//-------------------------------------------
//功能：向MAX7219写入数据
//入口参数：address、dat
//出口参数：无
//说明：
void Write_Max7219(uchar address,uchar dat)
{ 
  Max7219_pinCS=0;
  Write_Max7219_byte(address);           //写入地址，即数码管编号
  Write_Max7219_byte(dat);               //写入数据，即数码管显示数字 
  Max7219_pinCS=1;                        
}

void Init_MAX7219(void)
{
    Write_Max7219(0x09, 0x00);       //译码方式：BCD码
    Write_Max7219(0x0a, 0x03);       //亮度 
    Write_Max7219(0x0b, 0x07);       //扫描界限；8个数码管显示
    Write_Max7219(0x0c, 0x01);       //掉电模式：0，普通模式：1
    Write_Max7219(0x0f, 0x00);       //显示测试：1；测试结束，正常显示：0
}

uchar PanData(uchar dat,char p)
{
    long tdata = dat;
    if(p<0)
        return 0;
    tdata<<=p;
    tdata>>=8;
    return tdata;
}

void main(void)
{
    uchar i,j,k,dp;
    DelayNms(50);
    Init_MAX7219();  
    while(1)
    {
        for(j=0;j<6;j++)
            for(dp=j?8:0;dp<16;dp++)
            {
                for(i=1;i<9;i++)
                    Write_Max7219(i,PanData(word[j][i-1],dp)|PanData(word[j+1][i-1],-8+dp));
                DelayNms(200);
            }
        DelayNms(200);
    }
}
