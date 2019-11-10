#include <AT89x51.H>		
#include <intrins.h>
#define uchar unsigned char  //无符号字符型 宏定义	变量范围0~255 uchar
#define uint  unsigned int	 //无符号整型 宏定义	变量范围0~65535 uint
#define ulong  unsigned long

//IO 引脚定义 0/1
sbit c_send = P3^2; //超声波发送
sbit c_recive = P3^3; //超声波接收

sbit beep = P1^0;   //蜂鸣器引脚

uint  time=0;
uint  timer=0;
uchar posit=0;

ulong S=0;
bit      flag =0;

uchar const discode[] ={ 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90 };

uchar const positon[4]={ 0xfe, 0xfd, 0xfb, 0xf7};

uchar disbuff[4]={ 0, 0, 0, 0};

void delayms(uint ms)
{
	uchar i=100,j;
	for(;ms;ms--) 
	{
		while(--i)
		{
			j=10;
			while(--j);
		}
	}
}


 void Display(void)
{

	P0=disbuff[posit];

	P2=positon[posit];
	if(++posit>=4)
	{
	 	posit=0;
	}
}

void Conut(void)
{
	uint ms;
	time=TH0*256+TL0;

	TH0=0;
	TL0=0;
	  
	S=(time*1.87)/10;     //算出来是MM	 
		
	if(S >= 5000 || S <= 30 || flag == 1)
	{
		flag = 0;
		disbuff[0] = disbuff[1] = disbuff[3] = disbuff[4] = 0xdf;
	}	 
	else
	{
		disbuff[0]=discode[S/1000];                  
		disbuff[1]=discode[S/100%10];                  
		disbuff[2]=discode[S/10%10]&0x7f;                  	  
		disbuff[3]=discode[S%10];	
	}


	if(S > 800)
	{
		beep =1;
	}
	else if( 300 < S && S <= 800)
	{	 
		if(500 < S && S <= 800)
		{
		  	ms = (S - 300) / 5;
		}
		else if(300 < S && S <= 500)
		{
			ms = (S - 300) / 10;
		}
	 	beep =0;
		delayms(50);
		beep =1;
		delayms(ms);
	}
	else if(S <= 300)
	{
	 	beep = 0;
	}
		 	
}

void time0() interrupt 1 		 //T0中断用来计数器溢出,超过测距范围
{
    flag=1;							 //中断溢出标志
}

void time1() interrupt 3 		 //T1中断用来扫描数码管和计800MS启动模块
{	
	uint i = 20;

	TH1=0xf8;
	TL1=0x30;
	
	Display();

	timer++;

	if(timer >= 100)			//800MS  启动一次模块
	{
		timer=0;
		c_send=1;			                
		
		_nop_();
		while(--i);

		c_send=0;
	} 
}

void  main(  void  )
{  
	beep = 1;

    TMOD=0x11;		   //设T0为方式1，GATE=1；
	TH0=0;
	TL0=0;          
	TH1=0xf8;		   //2MS定时
	TL1=0x30;
	ET0=1;             //允许T0中断
	ET1=1;			   //允许T1中断
	TR1=1;			   //开启定时器
	EA=1;			   //开启总中断

	while(1)
	{
		Display();
		while(!c_recive);		//当RX为零时等待
		TR0=1;			    //开启计数
		
		while(c_recive);			//当RX为1计数并等待
		TR0=0;				//关闭计数

		Conut();			//计算
	}
}
         