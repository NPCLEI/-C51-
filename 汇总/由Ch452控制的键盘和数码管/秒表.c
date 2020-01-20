#include<reg51.h>
#include<stdlib.h>
#include<intrins.h>
#define True 1
#define Flase 0
#define Bool char
/*********************************/

sbit DOUT = P2^7;
sbit LOAD = P3^3;
sbit DIN  = P2^6;
sbit DCLK = P2^5;

/*sbit DOUT = P2^7;
sbit LOAD = P2^6;
sbit DIN  = P2^5;
sbit DCLK = P2^4;*/

void ch452_WriteCmd(unsigned int cmd)
{
	int i;
	LOAD=0;
	for(i=0;i<12;i++) /*ÊäÈë12Î»Êý¾Ý£¬µÍÎ»ÔÚÇ°,&±í°´Î»Óë*/
	{
		DIN=cmd&1;
		DCLK=0;
		DCLK=1;
		cmd>>=1; /*ÉÏÉýÑØÓÐÐ§*/
	}
	LOAD=1; /*ÉÏÉýÑØ¼ÓÔØÊý¾Ý*/
	_nop_();_nop_();
	_nop_();_nop_();
}

void ch452_init(void) //¶¨ÒåÐ¾Æ¬³õÊ¼»¯
{
	DIN = 0;
	DIN = 1;
	LOAD = 1;
	DCLK = 1;
	ch452_WriteCmd(0x403);//ÏµÍ³²ÎÊýÉè¶¨¿ªÆô¼üÅÌÉ¨Ãè£¬ÏÔÊ¾
	ch452_WriteCmd(0x500);//²»ÆôÓÃBCDÄ£Ê½
}

unsigned char DIGCode[]={0x00,0x18,0xd5,0xd9,0x78,0xe9,0xed,0x98,0xfd,0xf9,0xe4,0xbd,0x01,0xbd,0x40};

//¶ÁÊý¾Ý
unsigned int ch452_read()
{
	unsigned char i,j,keyw=0,cmd=0x07;
	LOAD=0;
	//·¢ËÍ¶ÁÈ¡°´¼üÃüÁî
  	for(j=0;j<4;j++)
  	{
    	DIN=cmd&0x01;
    	DCLK=0;
    	cmd>>=1;
    	DCLK=1;
  	}
  	LOAD=1;
  	for(j=0;j<100;j++);
	//¶ÁÈ¡¼üÖµ
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

//ÊýÂë¹ÜÏÔÊ¾
void Display(unsigned char n,unsigned char m)
{
	ch452_WriteCmd(0xc00+n*256+DIGCode[m]);
}

//ÊýÂë¹ÜÏÔÊ¾
void DisplayWithPoint(unsigned char n,unsigned char m)
{
	ch452_WriteCmd(0xc00+n*256+(DIGCode[m]|0x02));
}

//ÊýÂë¹ÜÏÔÊ¾
void NonDisplay(unsigned char n,unsigned char m)
{
	ch452_WriteCmd(0xc00+n*256+(~DIGCode[m]));
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


//n¸öÖ¸ÁîÖÜÆÚ
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

//ÉèÖÃ¶Ë¿ÚÇø
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
//È«¾Ö±äÁ¿Çø
char stack[]={0,0,0,0};
unsigned char Stack_top()//»ñµÃÕ»¶¥(Ã»ÓÐÔªËØµÄ¸ö)
{
	unsigned char i=0;
	for(;i<4;i++)
		if(stack[i]==-2)
			return i;
	return -1;
}
void Stack_push(unsigned char c)
{
	unsigned char top = Stack_top();
	if(top!=-1)//Õ»Î´Âú
		stack[top]=c;
}
void Stack_pop()
{
	unsigned char top = Stack_top();
	if(top==-1)
		stack[3]=-2;
	else
		stack[top-1]=-2;
}
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

void DiscoverShow()
{
	unsigned char i=0;
	for(;i<4;i++)
		Display(i,14);
}

void Show()
{
	unsigned char i=0,j;
	for(;i<4;i++)
	{
		j=stack[i]==0?13:stack[i];
		if(i!=2)
			Display(i,j);
		else
			DisplayWithPoint(i,j);
	}
}

void InterputInit()
{
    TMOD=0x02;    //ÉèÖÃ¶¨Ê±Æ÷0Îª¹¤×÷·½Ê½2
    TH0=6;   //×°Èë³õÖµ
    TL0=6;    
    EA=1;    //¿ª×ÜÖÐ¶Ï
    ET0=1;   //¿ª¶¨Ê±Æ÷ÖÐ¶Ï
    TR0=0;  //Æô¶¯¶¨Ê±Æ÷0
}

void Second_Autoincrement()
{
	stack[2]++;
	if(stack[2]==10)
	{
		stack[2]=0;
		stack[1]++;
		if(stack[1]==10)
		{
			stack[1]=0;
			stack[0]++;
			
		}
	}
}

void Millisecond_Autoincrement()
{
	stack[3]++;
	if(stack[3]==10)
	{
		stack[3]=0;
		Second_Autoincrement();
	}

}

int count = 0;
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

unsigned char saving[10][4];

void Save()
{
	unsigned char i,j;
	for(i=9;i>0;i--)
		for(j=0;j<4;j++)
			saving[i][j]=saving[i-1][j];
	for(j=0;j<4;j++)
		saving[0][j]=stack[j];
	Beep(200);
}

void SaveInit()
{
	unsigned char i,j;
	for(i=0;i<10;i++)
		for(j=0;j<4;j++)
			saving[i][j]=i;
}

unsigned char showingsaving=0;
void ShowSaving(Bool dir)//1up,0down
{
	static char cursavingposition=0;
	unsigned char j;
	if(showingsaving==0)
	{
		cursavingposition=0;
		for(j=0;j<4;j++)
			stack[j]=saving[cursavingposition][j];
		showingsaving=1;
		return;
	}
	showingsaving=1;
	if(TR0!=1)
	{
		if(dir)
		{
			cursavingposition++;
			if(cursavingposition>9)
				cursavingposition=9;
			for(j=0;j<4;j++)
				stack[j]=saving[cursavingposition][j];
		}
		else
		{
			cursavingposition--;
			if(cursavingposition<0)
				cursavingposition=0;
			for(j=0;j<4;j++)
				stack[j]=saving[cursavingposition][j];
		}
	}
}

void EnterTime()
{
	int i=0,key=0,enter_count=0;
	unsigned char stack_top=0;

	while(1)
	{
		if(TR0==1||stack[3]>0||showingsaving)
			Show();
		else
			DiscoverShow();
		key=ReadKey();
		if(key!=-1)
		{
			switch(key)
			{
				case 1:ShowSaving(1);break;
				case 3:Save();showingsaving=0;break;
				case 4:Stack_clear();break;
				case 5:ShowSaving(0);break;
				case 7:
					TR0=1;
					DelayNms(500);
					if(showingsaving==1)
					{
						Stack_clear();
						showingsaving=0;
					}
					break;
				case 8:TR0=0;DelayNms(500);showingsaving=0;break;
			}
			Beep(20);
			DelayNms(200);
		}
		DelayNms(50);
	}
}

int main()
{
	ch452_init();//³õÊ¼»¯ch452Ð¾Æ¬
	SaveInit();
	InterputInit();
	EnterTime();
	return 0;
}