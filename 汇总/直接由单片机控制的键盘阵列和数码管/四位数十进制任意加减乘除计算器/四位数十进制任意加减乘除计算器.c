#include<STC15F2K60S2.h>
#include<stdlib.h>
#include"NPCC51.h"
#define True 1
#define Flase 0
#define Bool char
#define Plus -1
#define Multi -2
#define Sub -3
#define Divi -4
#define End -5
#define Input_Length 16

int Input[Input_Length];
char InputPoint=0;

void Input_Append(int n)
{
	Input[InputPoint]=n;
	InputPoint++;
}

void Input_Delete(int n)
{
	int i = n;
	for (; i<Input_Length; i++)
		Input[i] = Input[i + 1];
	Input[Input_Length-1]=End;
}

//转换键，用于转换加减法
extern bit shift=0;
void KeyShift()
{
	shift=1;
}

void ClearNumber()
{
	char i=0;
	for (; i < Input_Length; ++i)
		Input[i]=0;
	InputPoint=0;
}

void PlusOrMulti()
{
	//"+":-1,"*":-2
	if(!Stack_empty())
		Input_Append(ConvertStackToNum());
	Stack_clear();
	if(Input[InputPoint]>=0)
		Input_Append(shift?Multi:Plus);
	else
		Input[InputPoint]=shift?Multi:Plus;
	shift=0;
}

void SubOrDivi()
{
	if(!Stack_empty())
		Input_Append(ConvertStackToNum());
	Stack_clear();
	if(Input[InputPoint]>=0)
		Input_Append(shift?Divi:Sub);
	else
		Input[InputPoint]=shift?Divi:Sub;
	shift=0;
}

void Calculate()
{
	//3+555*555+9-88/78
	//3 0 a 0 0 0 9 - a 0 0
	int i=0;
	float result=0,t,lastsym;
	for (;Input[i]!=End;++i)
	{
		if(Input[i]==Multi)
		{
			Input[i-1]=Input[i-1]*Input[i+1];
			Input_Delete(i);
			Input_Delete(i);
			i--;
		}
		if(Input[i]==Divi)
		{
			Input[i-1]=Input[i-1]/Input[i+1];
			Input_Delete(i);
			Input_Delete(i);
			i--;
		}
	}
	for(i=0;i<Input_Length;i++)
	{
		if(Input[i]==Plus)
			Input_Delete(i);
		if(Input[i]==Sub)
		{
			Input[i+1]=-1*Input[i+1];
			Input_Delete(i);
			continue;
		}
		if(Input[i]==End)
			Input[i]=0;
	}
	for(i=0;i<Input_Length;i++)
		result+=Input[i];
	i=result;
	ConvertToStack(i);
	while(ReadKeyBored()!=11)Show();
	ClearNumber();
	Stack_clear();
}

void WaitEnter()
{
	unsigned char k, i, key;
	key=ReadKeyBored();
	if(key!=-1)
	{
		switch(key)
		{
			case 0 :Stack_push(1);break;
		 	case 1 :Stack_push(2);break;
		 	case 2 :Stack_push(3);break;
		 	case 3 :Stack_pop();break;
		 	case 4 :Stack_push(4);break;
		 	case 5 :Stack_push(5);break;
		 	case 6 :Stack_push(6);break;
		 	case 7 :KeyShift();break;
		 	case 8 :Stack_push(7);break;
		 	case 9 :Stack_push(8);break;
		 	case 10:Stack_push(9);break;
		 	case 11:Stack_clear();ClearNumber();break;
		 	case 12:PlusOrMulti();break;
		 	case 13:Stack_push(0);break;
		 	case 14:SubOrDivi();break;
		 	case 15:
		 		if(Input[InputPoint]<0)
		 			Input[InputPoint]=End;
		 		else
		 		{
		 			Input_Append(ConvertStackToNum());
			 		Input_Append(End);
		 		}
			 	Calculate();
		 		break; 
		}	
		DelayNms(200);	
	}
}

void InterputInit()
{
	TMOD=0x02;
	TH0=6;
	TL0=6;  
	EA=1;
	ET0=1;
	TR0=1;
}

void Init()
{
	P33=1;
	P37=1;
	P36=1;
	P35=1;
	P34=1;
	ClearNumber();
}

void InterruptTimer0() interrupt 1
{
	TR0=0;
	Show();
    TR0=1;
}

void main()
{ 
	Init();
	while(1)
	{
		Show();
		WaitEnter();
	}
}