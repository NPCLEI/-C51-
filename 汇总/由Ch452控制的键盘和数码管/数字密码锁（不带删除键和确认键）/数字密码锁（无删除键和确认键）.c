#include<STC15F2K60S2.h>
#include<stdlib.h>
#include<intrins.h>
#define True 1
#define Flase 0
#define Bool int

sbit DOUT = P2^7;
sbit LOAD = P2^6;
sbit DIN  = P2^5;
sbit DCLK = P2^4;

void ch452_WriteCmd(unsigned int cmd)
{
	int i;
	LOAD=0;
	for(i=0;i<12;i++) /*输入12位数据，低位在前,&表按位与*/
	{
		DIN=cmd&1;
		DCLK=0;
		DCLK=1;
		cmd>>=1; /*上升沿有效*/
	}
	LOAD=1; /*上升沿加载数据*/
	_nop_();_nop_();
	_nop_();_nop_();
}

void ch452_init(void) //定义芯片初始化
{
	DIN = 0;
	DIN = 1;
	LOAD = 1;
	DCLK = 1;
	ch452_WriteCmd(0x403);//系统参数设定开启键盘扫描，显示
	ch452_WriteCmd(0x500);//不启用BCD模式
}

unsigned char DIGCode[]={0xbd,0x18,0xd5,0xd9,0x78,0xe9,0xed,0x98,0xfd,0xf9,0xe4,0xbd,0x01,0x00,0xFF};

//读数据
unsigned int ch452_read()
{
	unsigned char i,j,keyw=0,cmd=0x07;
	LOAD=0;
	//发送读取按键命令
  	for(j=0;j<4;j++)
  	{
    	DIN=cmd&0x01;
    	DCLK=0;
    	cmd>>=1;
    	DCLK=1;
  	}
  	LOAD=1;
  	for(j=0;j<100;j++);
	//读取键值
  	for(i=0;i<7;i++)
 	{
	    keyw<<=1;           
	    if(DOUT)
	    {
	    	keyw++;
	    }
	    DCLK=0;
	    DCLK=1;
  	}
  	return keyw;
}

//数码管显示
void Display(unsigned char n,unsigned char m)
{
	ch452_WriteCmd(0xc00+n*256+DIGCode[m]);
}

unsigned int ReadKey()
{
	unsigned int keya,keynum=0;
	keya=ch452_read();
	switch(keya)
	{
        case 0x44:keynum=1;break;
        case 0x45:keynum=2;break;
        case 0x46:keynum=3;break;
        case 0x47:keynum=4;break;
        case 0x4c:keynum=5;break;
        case 0x4d:keynum=6;break;
        case 0x4e:keynum=7;break;
        case 0x4f:keynum=8;break;
        default:keynum=-1;
	}
	return keynum;
}

/*********************************/


//n个指令周期
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

//设置端口区
sbit Speaker = P4^5;
void Beep(float time)
{
	int i=0;
	Speaker=1;
	for(;i<time;i++)
	{
		Speaker = ~Speaker;
		N_Nop_(222);
		Speaker = ~Speaker;
		N_Nop_(222);
	}
}

int psd[]={2,0,2,0};
char digtal[]={-2,-2,-2,-2};

unsigned char digtal_top()
{
	unsigned char i=0;
	for(;i<4;i++)
		if(digtal[i]==-2)
			return i;
	return -1;
}
void add_digtal(unsigned char c)
{
	unsigned char top = digtal_top();
	if(top!=-1)
		digtal[top]=c;
}
void del_digtal()
{
	unsigned char top = digtal_top();
	if(top==-1)
		digtal[3]=-2;
	else
		digtal[top-1]=-2;
}
void clear_digtal()
{
	unsigned char i = 0;
	for(;i<4;i++)
		digtal[i]=-2;
}

void closeDigtal()
{
	unsigned char i=0;
	for(;i<4;i++)
		Display(i,13);
}

void uncover()
{
	unsigned char i=0;
	for(;i<4;i++)
		if(digtal[i]!=-2)
			Display(i,14);
		else
		{
			Display(i+1,13);
			Display(i,12);
			break;
		}
	for(i++;i<4;i++)
		Display(i,13);
}

Bool Infer()
{
	unsigned char i=0;
	for(;i<4;i++)
		if(psd[i]!=digtal[i])
			return Flase;
	return True;
}

void showDigtal()
{
	unsigned char i=0;
	for(;i<4;i++)
		if(digtal[i]!=-2)
			Display(i,digtal[i]);
		else
		{
			Display(i+1,13);
			Display(i,12);
			break;
		}
	for(i++;i<4;i++)
		Display(i,13);
}

void changepsw()
{
	int i=0,key=0;
	unsigned char digtalTop=0;
	
	while(1)
	{
		showDigtal();
		key=ReadKey();
		if(key!=-1)
		{
			add_digtal(key-1);
			digtalTop=digtal_top();
			digtalTop=digtalTop==-1?3:digtalTop-1;
			Display(digtalTop,digtal[digtalTop]);
			if(digtal[3]!=-2)
				goto label;
			Beep(50);
			DelayNms(200);
		}
	}
	label:
	for(i=0;i<4;i++)
		psd[i]=digtal[i];
	Beep(2000);
	clear_digtal();
	showDigtal();
}

void waitBack()
{
	int key=0;
	unsigned char i = 0;
	while(1)
	{
		closeDigtal();
		ch452_WriteCmd(0xc00+i*256+0x81);
		DelayNms(200);
		i++;
		if(i>3)i=0;
		key=ReadKey();
		if(key==7){clear_digtal();DelayNms(1000);changepsw();break;}
		if(key==8)break;
	}
}

void Lock()
{
	unsigned char l=0x25,o=0xbd,c=0xa5,k=0x64;
	ch452_WriteCmd(0xc00+0*256+l);
	ch452_WriteCmd(0xc00+1*256+o);
	ch452_WriteCmd(0xc00+2*256+c);
	ch452_WriteCmd(0xc00+3*256+k);
	while(1)
	{
		DelayNms(200);
	}
}

char count = 0;
void Inferpsd()
{
	unsigned char i=0,j=0;
	if(Infer())
	{
		count=0;
		clear_digtal();
		waitBack();
	}
	else
	{
		count++;
		clear_digtal();
		Beep(800);
	}
	if(count>2)
		Lock();
}

void waitEnter()
{
	int i=0,key=0;
	unsigned char digtalTop=0;
	while(1)
	{
		uncover();
		key=ReadKey();
		if(key!=-1)
		{
			add_digtal(key-1);
			digtalTop=digtal_top();
			digtalTop=digtalTop==-1?3:digtalTop-1;
			Display(digtalTop,digtal[digtalTop]);
			if(digtal[3]!=-2)
			{
				DelayNms(200);
				uncover();
				Inferpsd();
			}
			DelayNms(200);
		}
	}
}

int main()
{
	ch452_init();
	waitEnter();
	return 0;
}