#include<reg51.h>
#include<stdlib.h>
#include<intrins.h>

sbit DOUT = P2^7;
sbit LOAD = P2^6;
sbit DIN  = P2^5;
sbit DCLK = P2^4;

void ch452_WriteCmd(unsigned int cmd)
{
	int i;
	LOAD=0;
	for(i=0;i<12;i++) 
	{
		DIN=cmd&1;
		DCLK=0;
		DCLK=1;
		cmd>>=1; 
	}
	LOAD=1; 
	_nop_();_nop_();
	_nop_();_nop_();
}

void ch452_init(void) 
{
	DIN = 0;
	DIN = 1;
	LOAD = 1;
	DCLK = 1;
	ch452_WriteCmd(0x403);
	ch452_WriteCmd(0x500);
}

unsigned char DIGCode[]={0xbd,0x18,0xd5,0xd9,0x78,0xe9,0xed,0x98,0xfd,0xf9,0xe4,0xbd,0x01,0x00,0x40};

//?áêy?Y
unsigned int ch452_read()
{
	unsigned char i,j,keyw=0,cmd=0x07;
	LOAD=0;
  	for(j=0;j<4;j++)
  	{
    	DIN=cmd&0x01;
    	DCLK=0;
    	cmd>>=1;
    	DCLK=1;
  	}
  	LOAD=1;
  	for(j=0;j<100;j++);
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

sbit Speaker = P1^7;
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

char stack[]={0,0,0,0};
int count = 0;

void Stack_clear()
{
	unsigned char i = 0;
	for(;i<4;i++)
		stack[i]=0;
}

void ClearShow()
{
	unsigned char i=0;
	for(;i<4;i++)
		Display(i,0);
}

void Show()
{
	unsigned char i=0;
	for(i=0;i<4;i++)
	{
		if(i==1)
			DisplayWithPoint(i,stack[i]);
		else
			Display(i,stack[i]);
	}
}

void InterputInit()
{
    TMOD=0x02;    //éè???¨ê±?÷0?a1¤×÷·?ê?2
    TH0=6;   //×°è?3??μ
    TL0=6;    
    EA=1;    //?a×ü?D??
    ET0=1;   //?a?¨ê±?÷?D??
    TR0=0;  //???ˉ?¨ê±?÷0
}

void InterruptTimer0() interrupt 1
{
    count++;
    if(count==367)
    {
		Millisecond_Autoincrement();
    	count=0;
    	Show();
    }
}

int pow(int x,int n)
{
	int i = 0,s=1;
	for(;i<n;i++)
		s=s*x;
	return s;
}

int signedn = 1;
void ConvertCountToStack()
{
	int i,j=1000,k=count>0?count:-1*count;
	for(i=0;i<4;i++,j=j/10)
	{
		stack[i]=(k-k % j)/j;
		k = k - stack[i]*j;
	}
}

void WaitEnter()
{
	int key=0;

	while(1)
	{
		Show();
		key=ReadKey();
		if(key!=-1)
		{
			switch(key)
			{
				case 1:count+=signedn*1;break;
				case 2:count+=signedn*2;break;
				case 3:count+=signedn*3;break;
				case 4:count+=signedn*4;break;
				case 5:signedn=-1*signedn;break;
				case 6:;break;
				case 7:;break;
				case 8:count=0;break;
			}
			ConvertCountToStack();
			DelayNms(200);
		}
		DelayNms(50);
	}
}

int main()
{
	ch452_init();
	WaitEnter();
	return 0;
}