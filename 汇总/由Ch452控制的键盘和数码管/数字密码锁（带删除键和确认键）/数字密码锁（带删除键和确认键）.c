#include<reg51.h>
#include<stdlib.h>
#include<intrins.h>
#define True 1
#define Flase 0
#define Bool int

//定义Ch452的管脚
sbit DCLK = P2^4;
sbit DIN  = P2^5;
sbit LOAD = P2^6;
sbit DOUT = P2^7;

//向Ch452中写入cmd命令
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

//定义芯片初始化
void ch452_init(void) 
{
	DIN = 0;
	DIN = 1;
	LOAD = 1;
	DCLK = 1;
	ch452_WriteCmd(0x403);//系统参数设定开启键盘扫描，显示
	ch452_WriteCmd(0x500);//不启用BCD模式
}

unsigned char DIGCode[]={0x00,0x18,0xd5,0xd9,0x78,0xe9,0xed,0x98,0xfd,0xf9,0xe4,0xbd,0x01,0xbd,0x40};

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

//全局变量区
int CurPosition = 0;
int Password[4]={1,1,1,1};
char stack[]={-2,-2,-2,-2};
unsigned char Stack_top()//获得栈顶(没有元素的个)
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
	if(top!=-1)//栈未满
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
		stack[i]=-2;
}

//关闭数码管
void ClearShow()
{
	unsigned char i=0;
	for(;i<4;i++)
		Display(i,0);
}

//用“-”隐藏显示的数字
void DiscoverShow()
{
	unsigned char i=0;
	for(;i<4;i++)
		if(stack[i]!=-2)
			Display(i,14);
		else
		{
			Display(i+1,0);
			Display(i,12);
			break;
		}
	for(i++;i<4;i++)
		Display(i,0);
}

//显示当前stack中的数字
void Show()
{
	unsigned char i=0;
	for(;i<4;i++)
		if(stack[i]!=-2)
			Display(i,stack[i]==0?13:stack[i]);
		else
		{
			Display(i+1,0);
			Display(i,12);
			break;
		}
	for(i++;i<4;i++)
		Display(i,0);
}

//判断用户输入与密码是否一致
Bool isPassword()
{
	unsigned char i=0;
	for(;i<4;i++)
		if(Password[i]!=stack[i])
			return Flase;
	return True;
}

//改密码时的操作
void ChangePassword()
{
	int i=0,key=0,entertime=0;//entertime：统计确认键按下次数

	while(1)
	{
		Show();
		key=ReadKey();
		if(key!=-1)
		{
			switch(key)
			{
				case 1:Stack_push(1);break;
				case 2:Stack_push(2);break;
				case 3:Stack_push(3);break;
				case 4:Stack_pop();break;
				case 5:Stack_push(4);break;
				case 6:Stack_push(5);break;
				case 7:Stack_push(6);break;
				case 8:
					entertime++;
					if(entertime>10&&Stack_top()==-1)//长按enter键确认密码
						break;
			}
			Beep(50);
			DelayNms(200);
		}
	}
	for(i=0;i<4;i++)
		Password[i]=stack[i];
	Beep(2000);
	Stack_clear();
	Show();
}

//密码正确时的操作
void EnterPasswordCorrect()
{
	int key=0;
	unsigned char i = 0;
	for(i=0;i<4;i++)
		stack[i]=8;
	while(1)
	{
		Show();
		DelayNms(200);
		ClearShow();
		DelayNms(200);
		key=ReadKey();
		if(key==4)
		{
			Stack_clear();
			Beep(50);
			DelayNms(200);
			ChangePassword();
			break;
		}
		if(key==8)
		{
			Stack_clear();
			return;
		}
	}
}

//初始化定时器
void InterputInit()
{
    EA   = 1;       //使能全局中断
    TMOD = 0x00;  //配置T0工作在模式0
    TH0  = 0x63;
    TL0  = 0x18;
    ET0  = 1;      //使能T0中断
    TR0  = 1;      //启动T0
}

int count = 0;
//倒计时的操作
void InterruptTimer0() interrupt 1
{
    TH0 = 0x63;   //重新加载重载值
    TL0 = 0x18;
    count++;
    if(count>=20)
    {
    	stack[3]--;
    	if(stack[3]<0)
    	{
    		stack[2]--;
    		stack[3]=9;
    		if(stack[2]<0)
    		{
    			ET0 = 0;
    			TR0 = 0;
    			Stack_clear();
    			Show();
    		}
    	}
    	count=0;
    }
    Show();
    Beep(2);
}

//倒数29秒
void Countdown29s()
{
	stack[0]=0;
	stack[1]=0;
	stack[2]=2;
	stack[3]=9;
	InterputInit();
}

//按下确认键时的操作
void Enter()
{
	static unsigned char enter_eorr_time = 0;
	unsigned char i=0,j=0;

	if(isPassword())
	{
		enter_eorr_time=0;
		EnterPasswordCorrect();
	}
	else
	{
		enter_eorr_time++;
		Beep(1000);
	}
	if(enter_eorr_time>2)
	{
		//倒计时
		Countdown29s();
		DelayNms(29000);
		enter_eorr_time=0;
	}

}

//等待用户输入密码
void EnterPassword()
{
	int i=0,key=0;
	unsigned char stack_top=0;

	while(1)
	{
		DiscoverShow();
		key=ReadKey();
		if(key!=-1)
		{
			switch(key)
			{
				case 1:Stack_push(1);break;
				case 2:Stack_push(2);break;
				case 3:Stack_push(3);break;
				case 4:Stack_pop();break;
				case 5:Stack_push(4);break;
				case 6:Stack_push(5);break;
				case 7:Stack_push(6);break;
				case 8:Enter();break;
			}
			stack_top=Stack_top();
			//显示出当前输入位
			if((stack_top>=1||stack_top==-1)&&key!=4&&key!=8)
			{
				stack_top=(stack_top==-1?3:stack_top-1);
				Display(stack_top,stack[stack_top]);
			}
			Beep(50);
			DelayNms(200);
		}
	}
}

int main()
{
	ch452_init();//初始化ch452芯片
	EnterPassword();//等待输入密码
	return 0;
}