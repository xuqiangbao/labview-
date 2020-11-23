//单片机 AT89S52 或 STC89C52RC 
//功能   串口发送温湿度数据 晶振 11.0592M 波特率 9600 
//硬件   P2.0口为通讯口连接DHT11,DHT11的电源和地连接单片机的电源和地，单片机串口加MAX232连接电脑    
//****************************************************************//
#include <reg51.h>
#include <intrins.h> 
//数据类型定义
typedef unsigned char  U8;       /* defined for unsigned 8-bits integer variable 	  无符号8位整型变量  */
typedef signed   char  S8;       /* defined for signed 8-bits integer variable		  有符号8位整型变量  */
typedef unsigned int   U16;      /* defined for unsigned 16-bits integer variable 	  无符号16位整型变量 */
typedef signed   int   S16;      /* defined for signed 16-bits integer variable 	  有符号16位整型变量 */
typedef unsigned long  U32;      /* defined for unsigned 32-bits integer variable 	  无符号32位整型变量 */
typedef signed   long  S32;      /* defined for signed 32-bits integer variable 	  有符号32位整型变量 */
typedef float          F32;      /* single precision floating point variable (32bits) 单精度浮点数（32位长度） */
typedef double         F64;      /* double precision floating point variable (64bits) 双精度浮点数（64位长度） */
//宏定义
#define uchar unsigned char
#define uint unsigned int
#define   Data_0_time    4

//----------------------------------------------//
//----------------IO口定义区--------------------//
//----------------------------------------------//
sbit  DHT11_P=P1^1 ;
sbit  shidu=P1^0;
sbit LcdRs_P   = P2^7;             // 1602液晶的RS管脚       
sbit LcdRw_P   = P2^6;            // 1602液晶的RW管脚 
sbit LcdEn_P   = P2^5;            // 1602液晶的EN管脚
//----------------------------------------------//
//-------------变量定义区--------------------//
//----------------------------------------------//
U8  U8FLAG,k;
U8  U8count,U8temp;
U8  U8T_data_H,U8T_data_L,U8RH_data_H,U8RH_data_L,U8checkdata;
U8  U8T_data_H_temp,U8T_data_L_temp,U8RH_data_H_temp,U8RH_data_L_temp,U8checkdata_temp;
U8  U8comdata;
U8  outdata[5];  //定义发送的字节数	   
U8  indata[5];
U8  count, count_r=0;
U8  str[5]={"RS232"};
U16 U16temp1,U16temp2;
uchar flag;
uchar temp;                                                                // 保存温度
uchar humi;                                                                  // 保存湿度
SendData(U8 *a)
{
	outdata[0] = a[0]; 
	outdata[1] = a[1];
	outdata[2] = a[2];
	outdata[3] = a[3];
	outdata[4] = a[4];
	count = 1;
	SBUF=outdata[0];
	return(SBUF);
}
/*********************************************************/
// 毫秒级的延时函数，time是要延时的毫秒数
/*********************************************************/
void DelayMs(uint time)
{
        uint i,j;
        for(i=0;i<time;i++)
                for(j=0;j<112;j++);
}


/*********************************************************/
// 1602液晶写命令函数，cmd就是要写入的命令
/*********************************************************/
void LcdWriteCmd(uchar cmd)
{ 
        LcdRs_P = 0;
        LcdRw_P = 0;
        LcdEn_P = 0;
        P0=cmd;
        DelayMs(2);
        LcdEn_P = 1;    
        DelayMs(2);
        LcdEn_P = 0;        
}


/*********************************************************/
// 1602液晶写数据函数，dat就是要写入的数据
/*********************************************************/
void LcdWriteData(uchar dat)
{
        LcdRs_P = 1; 
        LcdRw_P = 0;
        LcdEn_P = 0;
        P0=dat;
        DelayMs(2);
        LcdEn_P = 1;    
        DelayMs(2);
        LcdEn_P = 0;
}


/*********************************************************/
// 1602液晶初始化函数
/*********************************************************/
void LcdInit()
{
        LcdWriteCmd(0x38);        // 16*2显示，5*7点阵，8位数据口
        LcdWriteCmd(0x0C);        // 开显示，不显示光标
        LcdWriteCmd(0x06);        // 地址加1，当写入数据后光标右移
        LcdWriteCmd(0x01);        // 清屏
}


/*********************************************************/
// 液晶光标定位函数
/*********************************************************/
void LcdGotoXY(uchar line,uchar column)
{
        // 第一行
        if(line==0)        
                LcdWriteCmd(0x80+column); 
        // 第二行
        if(line==1)        
                LcdWriteCmd(0x80+0x40+column); 
}


/*********************************************************/
// 液晶输出字符串函数
/*********************************************************/
void LcdPrintStr(uchar *str)
{
        while(*str!='\0')                                         // 判断是否到字符串的尽头了
                LcdWriteData(*str++);
}


/*********************************************************/
// 液晶输出数字
/*********************************************************/
void LcdPrintNum(uchar num)
{
        LcdWriteData(num/10+48);                // 十位
        LcdWriteData(num%10+48);                 // 个位
}


/*********************************************************/
// 液晶显示内容的初始化
/*********************************************************/
void LcdShowInit()
{
        LcdGotoXY(0,0);                                                                                // 光标定位
        LcdPrintStr("  jiu  zhe  ?  ?");        // 第0行显示的内容
        LcdGotoXY(1,0);                                                                           // 光标定位        
        LcdPrintStr("T:   C   H:  %RH");        // 第1行显示的内容
        LcdGotoXY(1,4);                                                                                // 温度单位摄氏度上面的圆圈符号
        LcdWriteData(0xdf);        
}


/*********************************************************/
// 10us级延时程序
/*********************************************************/
void Delay10us()
{
        _nop_();        // 执行一条指令，延时1微秒
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
}


/*********************************************************/
// 读取DHT11单总线上的一个字节
/*********************************************************/
uchar DhtReadByte(void)
{   
        bit bit_i; 
        uchar j;
        uchar dat=0;

        for(j=0;j<8;j++)    
        {
                while(!DHT11_P);        // 等待低电平结束        
                Delay10us();                        // 延时
                Delay10us();
                Delay10us();
                if(DHT11_P==1)                // 判断数据线是高电平还是低电平
                {
                        bit_i=1; 
                        while(DHT11_P);
                } 
                else
                {
                        bit_i=0;
                }
                dat<<=1;                                   // 将该位移位保存到dat变量中
                dat|=bit_i;    
        }
    return(dat);  
}


/*********************************************************/
// 读取DHT11的一帧数据，湿高、湿低(0)、温高、温低(0)、校验码
/*********************************************************/
void ReadDhtData()
{             
        uchar HumiHig;                // 湿度高检测值
        uchar HumiLow;          // 湿度低检测值 
        uchar TemHig;                        // 温度高检测值
        uchar TemLow;                        // 温度低检测值
    uchar check;                // 校验字节 
        
        DHT11_P=0;                                // 主机拉低20ms
        DelayMs(20);
        DHT11_P=1;                                // DATA总线由上拉电阻拉高 主机延时40us                

        Delay10us();                         // 延时等待30us
        Delay10us();
        Delay10us();

        while(!DHT11_P);        // 等待DHT的低电平结束
        while(DHT11_P);                // 等待DHT的高电平结束

        //进入数据接收状态
        HumiHig = DhtReadByte();         // 湿度高8位
        HumiLow = DhtReadByte();         // 湿度低8为，总为0
        TemHig  = DhtReadByte();         // 温度高8位 
        TemLow  = DhtReadByte();         // 温度低8为，总为0 
        check   = DhtReadByte();        // 8位校验码，其值等于读出的四个字节相加之和的低8位

        while(!DHT11_P);
        DHT11_P=1;                                                                // 拉高总线

        if(check==HumiHig + HumiLow + TemHig + TemLow)                 // 如果收到的数据无误
        {
                temp=TemHig;                         // 将温度的检测结果赋值给全局变量temp
                humi=HumiHig;                        // 将湿度的检测结果赋值给全局变量humi
        }
}


//延时
 void Delay(U16 j)
    {   U8 i;
	    for(;j>0;j--)
	  { 	
		for(i=0;i<27;i++);

	  }
    }
//10us延时     
 void  Delay_10us(void)
      {
        U8 i;
        i--;
        i--;
        i--;
        i--;
        i--;
        i--;
       }
	
 void  COM(void)
{
    U8 i;   
    for(i=0;i<8;i++)	   
	  {
		 U8FLAG=2;	
	   	 while((!DHT11_P)&&U8FLAG++);		//总线被DTH11拉高，准备发送数据
			Delay_10us();
		    Delay_10us();
			Delay_10us();
	  		U8temp=0;
	     if(DHT11_P)
		    U8temp=1;
		    U8FLAG=2;
		 while((DHT11_P)&&U8FLAG++);	  //超时则跳出for循环	 80us后总线再次被拉低			  
	   	 if(U8FLAG==1)break;		 //判断数据位是0还是1
		   U8comdata<<=1;			// 如果高电平高过预定0高电平值则数据位为 1 
	   	   U8comdata|=U8temp;        //0
	  }
	   
}

	//--------------------------------
	//-----湿度读取子程序 ------------
	//--------------------------------
	//----以下变量均为全局变量--------
	//----温度高8位== U8T_data_H------
	//----温度低8位== U8T_data_L------
	//----湿度高8位== U8RH_data_H-----
	//----湿度低8位== U8RH_data_L-----
	//----校验 8位 == U8checkdata-----
	//----调用相关子程序如下----------
	//---- Delay();, Delay_10us();,COM(); 
	//--------------------------------

void RH(void)		   //湿度
{
	  //主机拉低总线18ms，保证DHT11能检测到起始信号
       DHT11_P=0;
	   Delay(180);	   
	   DHT11_P=1;
	 //总线由上拉电阻拉高 主机延时20us ，结束开始信号
	   Delay_10us();
	   Delay_10us();
	   Delay_10us();
	   Delay_10us();
	 //主机设为输入（输出为高电平）判断从机响应信号 
	   DHT11_P=1;
	 //判断从机是否有80us低电平响应信号 如不响应则跳出，响应则向下运行	  
	 if(!DHT11_P)		  
	  {
	     U8FLAG=2;	  	 
	   while((!DHT11_P)&&U8FLAG++); //判断从机是否发出 80us 的低电平响应信号是否结束
	     U8FLAG=2;	  
	   while((DHT11_P)&&U8FLAG++);	//判断从机是否发出 80us 的高电平，如发出则进入数据接收状态
	 //数据接收状态		 
	   COM();
	   U8RH_data_H_temp=U8comdata;
	   COM();
	   U8RH_data_L_temp=U8comdata;
	   COM();
	   U8T_data_H_temp=U8comdata;
	   COM();
	   U8T_data_L_temp=U8comdata;
	   COM();
	   U8checkdata_temp=U8comdata;
	   DHT11_P=1;			//数据接收完毕，总线被主机拉高，准备下一次采集
	 //数据校验 
	   U8temp=(U8T_data_H_temp+U8T_data_L_temp+U8RH_data_H_temp+U8RH_data_L_temp);
	   if(U8temp==U8checkdata_temp)
	   {
	   	  U8RH_data_H=U8RH_data_H_temp;
	   	  U8RH_data_L=U8RH_data_L_temp;
		  U8T_data_H=U8T_data_H_temp;
	   	  U8T_data_L=U8T_data_L_temp;
	   	  U8checkdata=U8checkdata_temp;
	   }
	   }
}
	
//----------------------------------------------
//main()功能描述:  AT89C51  11.0592MHz 	串口发 
//送温湿度数据,波特率 9600 
//----------------------------------------------
void main()
{
 	shidu=1;
	//uchar str[6]={"RS232"};
	/* 系统初始化 */
	        LcdInit();                                                // 液晶功能的初始化                        
        LcdShowInit();                                 // 液晶显示的初始化
	TMOD = 0x20;	  //定时器T1使用工作方式2
	TH1 = 253;        // 设置初值
	TL1 = 253;
	TR1 = 1;          // 开始计时
	SCON = 0x50;	  //工作方式1，波特率9600bps，允许接收   
	ES = 1;
	EA = 1;           // 打开所以中断   
	TI = 0;
	RI = 0;
	SendData(str) ;   //发送到串口 
	Delay(1);         //延时100US（12M晶振)

	while(1)
	{  
	                    EA=0;                                                                // 读取温湿度数值前，关闭总中断
                ReadDhtData();                        // 读取温湿度数据
                EA=1;                                                                // 读取温湿度数值后，开启总中断
        
                LcdGotoXY(1,2);                         // 定位到要显示温度的地方
                LcdPrintNum(temp);        // 显示温度值
                LcdGotoXY(1,11);                // 定位到要显示湿度的地方
                LcdPrintNum(humi);        // 显示湿度值



                DelayMs(2200);                  // 进行延时，可实现不同的采集频率
	   //------------------------
	   //调用温湿度读取子程序 
	   RH();
	   //串口显示程序 
	   //--------------------------

	   str[0]=U8RH_data_H;
	   str[1]=U8RH_data_L;
	   str[2]=U8T_data_H;
	   str[3]=U8T_data_L;
	   str[4]=U8checkdata;
	   SendData(str) ;  //发送到串口  
	   //读取模块数据周期不易小于 2S 
	   Delay(20000);
	}
	
}

void RSINTR() interrupt 4 using 2
{
	U8 InPut3;
	if(TI==1) //发送中断	  
	{
		TI=0;
		if(count!=5) //发送完5位数据	 
		{
			SBUF= outdata[count];
			count++;
		}
	}

	if(RI==1)	 //接收中断		  
	{	
		
		InPut3=SBUF;
		if(InPut3=='S')
		  shidu=0;			 //开启加湿器
		if(InPut3=='G')
		  shidu=1; 			 //关闭加湿器
		indata[count_r]=InPut3;
		count_r++;
		RI=0;								 
		if (count_r==5)//接收完4位数据 
		{
		//数据接收完毕处理。
			count_r=0;
		str[0]=indata[0];
		 str[1]=indata[1];
		   str[2]=indata[2];
			 str[3]=indata[3];
				 str[4]=indata[4];
				 P0=0;
		}
	}
}

