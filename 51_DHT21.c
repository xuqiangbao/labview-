//��Ƭ�� AT89S52 �� STC89C52RC 
//����   ���ڷ�����ʪ������ ���� 11.0592M ������ 9600 
//Ӳ��   P2.0��ΪͨѶ������DHT11,DHT11�ĵ�Դ�͵����ӵ�Ƭ���ĵ�Դ�͵أ���Ƭ�����ڼ�MAX232���ӵ���    
//****************************************************************//
#include <reg51.h>
#include <intrins.h> 
//�������Ͷ���
typedef unsigned char  U8;       /* defined for unsigned 8-bits integer variable 	  �޷���8λ���ͱ���  */
typedef signed   char  S8;       /* defined for signed 8-bits integer variable		  �з���8λ���ͱ���  */
typedef unsigned int   U16;      /* defined for unsigned 16-bits integer variable 	  �޷���16λ���ͱ��� */
typedef signed   int   S16;      /* defined for signed 16-bits integer variable 	  �з���16λ���ͱ��� */
typedef unsigned long  U32;      /* defined for unsigned 32-bits integer variable 	  �޷���32λ���ͱ��� */
typedef signed   long  S32;      /* defined for signed 32-bits integer variable 	  �з���32λ���ͱ��� */
typedef float          F32;      /* single precision floating point variable (32bits) �����ȸ�������32λ���ȣ� */
typedef double         F64;      /* double precision floating point variable (64bits) ˫���ȸ�������64λ���ȣ� */
//�궨��
#define uchar unsigned char
#define uint unsigned int
#define   Data_0_time    4

//----------------------------------------------//
//----------------IO�ڶ�����--------------------//
//----------------------------------------------//
sbit  DHT11_P=P1^1 ;
sbit  shidu=P1^0;
sbit LcdRs_P   = P2^7;             // 1602Һ����RS�ܽ�       
sbit LcdRw_P   = P2^6;            // 1602Һ����RW�ܽ� 
sbit LcdEn_P   = P2^5;            // 1602Һ����EN�ܽ�
//----------------------------------------------//
//-------------����������--------------------//
//----------------------------------------------//
U8  U8FLAG,k;
U8  U8count,U8temp;
U8  U8T_data_H,U8T_data_L,U8RH_data_H,U8RH_data_L,U8checkdata;
U8  U8T_data_H_temp,U8T_data_L_temp,U8RH_data_H_temp,U8RH_data_L_temp,U8checkdata_temp;
U8  U8comdata;
U8  outdata[5];  //���巢�͵��ֽ���	   
U8  indata[5];
U8  count, count_r=0;
U8  str[5]={"RS232"};
U16 U16temp1,U16temp2;
uchar flag;
uchar temp;                                                                // �����¶�
uchar humi;                                                                  // ����ʪ��
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
// ���뼶����ʱ������time��Ҫ��ʱ�ĺ�����
/*********************************************************/
void DelayMs(uint time)
{
        uint i,j;
        for(i=0;i<time;i++)
                for(j=0;j<112;j++);
}


/*********************************************************/
// 1602Һ��д�������cmd����Ҫд�������
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
// 1602Һ��д���ݺ�����dat����Ҫд�������
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
// 1602Һ����ʼ������
/*********************************************************/
void LcdInit()
{
        LcdWriteCmd(0x38);        // 16*2��ʾ��5*7����8λ���ݿ�
        LcdWriteCmd(0x0C);        // ����ʾ������ʾ���
        LcdWriteCmd(0x06);        // ��ַ��1����д�����ݺ�������
        LcdWriteCmd(0x01);        // ����
}


/*********************************************************/
// Һ����궨λ����
/*********************************************************/
void LcdGotoXY(uchar line,uchar column)
{
        // ��һ��
        if(line==0)        
                LcdWriteCmd(0x80+column); 
        // �ڶ���
        if(line==1)        
                LcdWriteCmd(0x80+0x40+column); 
}


/*********************************************************/
// Һ������ַ�������
/*********************************************************/
void LcdPrintStr(uchar *str)
{
        while(*str!='\0')                                         // �ж��Ƿ��ַ����ľ�ͷ��
                LcdWriteData(*str++);
}


/*********************************************************/
// Һ���������
/*********************************************************/
void LcdPrintNum(uchar num)
{
        LcdWriteData(num/10+48);                // ʮλ
        LcdWriteData(num%10+48);                 // ��λ
}


/*********************************************************/
// Һ����ʾ���ݵĳ�ʼ��
/*********************************************************/
void LcdShowInit()
{
        LcdGotoXY(0,0);                                                                                // ��궨λ
        LcdPrintStr("  jiu  zhe  ?  ?");        // ��0����ʾ������
        LcdGotoXY(1,0);                                                                           // ��궨λ        
        LcdPrintStr("T:   C   H:  %RH");        // ��1����ʾ������
        LcdGotoXY(1,4);                                                                                // �¶ȵ�λ���϶������ԲȦ����
        LcdWriteData(0xdf);        
}


/*********************************************************/
// 10us����ʱ����
/*********************************************************/
void Delay10us()
{
        _nop_();        // ִ��һ��ָ���ʱ1΢��
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
// ��ȡDHT11�������ϵ�һ���ֽ�
/*********************************************************/
uchar DhtReadByte(void)
{   
        bit bit_i; 
        uchar j;
        uchar dat=0;

        for(j=0;j<8;j++)    
        {
                while(!DHT11_P);        // �ȴ��͵�ƽ����        
                Delay10us();                        // ��ʱ
                Delay10us();
                Delay10us();
                if(DHT11_P==1)                // �ж��������Ǹߵ�ƽ���ǵ͵�ƽ
                {
                        bit_i=1; 
                        while(DHT11_P);
                } 
                else
                {
                        bit_i=0;
                }
                dat<<=1;                                   // ����λ��λ���浽dat������
                dat|=bit_i;    
        }
    return(dat);  
}


/*********************************************************/
// ��ȡDHT11��һ֡���ݣ�ʪ�ߡ�ʪ��(0)���¸ߡ��µ�(0)��У����
/*********************************************************/
void ReadDhtData()
{             
        uchar HumiHig;                // ʪ�ȸ߼��ֵ
        uchar HumiLow;          // ʪ�ȵͼ��ֵ 
        uchar TemHig;                        // �¶ȸ߼��ֵ
        uchar TemLow;                        // �¶ȵͼ��ֵ
    uchar check;                // У���ֽ� 
        
        DHT11_P=0;                                // ��������20ms
        DelayMs(20);
        DHT11_P=1;                                // DATA������������������ ������ʱ40us                

        Delay10us();                         // ��ʱ�ȴ�30us
        Delay10us();
        Delay10us();

        while(!DHT11_P);        // �ȴ�DHT�ĵ͵�ƽ����
        while(DHT11_P);                // �ȴ�DHT�ĸߵ�ƽ����

        //�������ݽ���״̬
        HumiHig = DhtReadByte();         // ʪ�ȸ�8λ
        HumiLow = DhtReadByte();         // ʪ�ȵ�8Ϊ����Ϊ0
        TemHig  = DhtReadByte();         // �¶ȸ�8λ 
        TemLow  = DhtReadByte();         // �¶ȵ�8Ϊ����Ϊ0 
        check   = DhtReadByte();        // 8λУ���룬��ֵ���ڶ������ĸ��ֽ����֮�͵ĵ�8λ

        while(!DHT11_P);
        DHT11_P=1;                                                                // ��������

        if(check==HumiHig + HumiLow + TemHig + TemLow)                 // ����յ�����������
        {
                temp=TemHig;                         // ���¶ȵļ������ֵ��ȫ�ֱ���temp
                humi=HumiHig;                        // ��ʪ�ȵļ������ֵ��ȫ�ֱ���humi
        }
}


//��ʱ
 void Delay(U16 j)
    {   U8 i;
	    for(;j>0;j--)
	  { 	
		for(i=0;i<27;i++);

	  }
    }
//10us��ʱ     
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
	   	 while((!DHT11_P)&&U8FLAG++);		//���߱�DTH11���ߣ�׼����������
			Delay_10us();
		    Delay_10us();
			Delay_10us();
	  		U8temp=0;
	     if(DHT11_P)
		    U8temp=1;
		    U8FLAG=2;
		 while((DHT11_P)&&U8FLAG++);	  //��ʱ������forѭ��	 80us�������ٴα�����			  
	   	 if(U8FLAG==1)break;		 //�ж�����λ��0����1
		   U8comdata<<=1;			// ����ߵ�ƽ�߹�Ԥ��0�ߵ�ƽֵ������λΪ 1 
	   	   U8comdata|=U8temp;        //0
	  }
	   
}

	//--------------------------------
	//-----ʪ�ȶ�ȡ�ӳ��� ------------
	//--------------------------------
	//----���±�����Ϊȫ�ֱ���--------
	//----�¶ȸ�8λ== U8T_data_H------
	//----�¶ȵ�8λ== U8T_data_L------
	//----ʪ�ȸ�8λ== U8RH_data_H-----
	//----ʪ�ȵ�8λ== U8RH_data_L-----
	//----У�� 8λ == U8checkdata-----
	//----��������ӳ�������----------
	//---- Delay();, Delay_10us();,COM(); 
	//--------------------------------

void RH(void)		   //ʪ��
{
	  //������������18ms����֤DHT11�ܼ�⵽��ʼ�ź�
       DHT11_P=0;
	   Delay(180);	   
	   DHT11_P=1;
	 //������������������ ������ʱ20us ��������ʼ�ź�
	   Delay_10us();
	   Delay_10us();
	   Delay_10us();
	   Delay_10us();
	 //������Ϊ���루���Ϊ�ߵ�ƽ���жϴӻ���Ӧ�ź� 
	   DHT11_P=1;
	 //�жϴӻ��Ƿ���80us�͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������	  
	 if(!DHT11_P)		  
	  {
	     U8FLAG=2;	  	 
	   while((!DHT11_P)&&U8FLAG++); //�жϴӻ��Ƿ񷢳� 80us �ĵ͵�ƽ��Ӧ�ź��Ƿ����
	     U8FLAG=2;	  
	   while((DHT11_P)&&U8FLAG++);	//�жϴӻ��Ƿ񷢳� 80us �ĸߵ�ƽ���緢����������ݽ���״̬
	 //���ݽ���״̬		 
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
	   DHT11_P=1;			//���ݽ�����ϣ����߱��������ߣ�׼����һ�βɼ�
	 //����У�� 
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
//main()��������:  AT89C51  11.0592MHz 	���ڷ� 
//����ʪ������,������ 9600 
//----------------------------------------------
void main()
{
 	shidu=1;
	//uchar str[6]={"RS232"};
	/* ϵͳ��ʼ�� */
	        LcdInit();                                                // Һ�����ܵĳ�ʼ��                        
        LcdShowInit();                                 // Һ����ʾ�ĳ�ʼ��
	TMOD = 0x20;	  //��ʱ��T1ʹ�ù�����ʽ2
	TH1 = 253;        // ���ó�ֵ
	TL1 = 253;
	TR1 = 1;          // ��ʼ��ʱ
	SCON = 0x50;	  //������ʽ1��������9600bps���������   
	ES = 1;
	EA = 1;           // �������ж�   
	TI = 0;
	RI = 0;
	SendData(str) ;   //���͵����� 
	Delay(1);         //��ʱ100US��12M����)

	while(1)
	{  
	                    EA=0;                                                                // ��ȡ��ʪ����ֵǰ���ر����ж�
                ReadDhtData();                        // ��ȡ��ʪ������
                EA=1;                                                                // ��ȡ��ʪ����ֵ�󣬿������ж�
        
                LcdGotoXY(1,2);                         // ��λ��Ҫ��ʾ�¶ȵĵط�
                LcdPrintNum(temp);        // ��ʾ�¶�ֵ
                LcdGotoXY(1,11);                // ��λ��Ҫ��ʾʪ�ȵĵط�
                LcdPrintNum(humi);        // ��ʾʪ��ֵ



                DelayMs(2200);                  // ������ʱ����ʵ�ֲ�ͬ�Ĳɼ�Ƶ��
	   //------------------------
	   //������ʪ�ȶ�ȡ�ӳ��� 
	   RH();
	   //������ʾ���� 
	   //--------------------------

	   str[0]=U8RH_data_H;
	   str[1]=U8RH_data_L;
	   str[2]=U8T_data_H;
	   str[3]=U8T_data_L;
	   str[4]=U8checkdata;
	   SendData(str) ;  //���͵�����  
	   //��ȡģ���������ڲ���С�� 2S 
	   Delay(20000);
	}
	
}

void RSINTR() interrupt 4 using 2
{
	U8 InPut3;
	if(TI==1) //�����ж�	  
	{
		TI=0;
		if(count!=5) //������5λ����	 
		{
			SBUF= outdata[count];
			count++;
		}
	}

	if(RI==1)	 //�����ж�		  
	{	
		
		InPut3=SBUF;
		if(InPut3=='S')
		  shidu=0;			 //������ʪ��
		if(InPut3=='G')
		  shidu=1; 			 //�رռ�ʪ��
		indata[count_r]=InPut3;
		count_r++;
		RI=0;								 
		if (count_r==5)//������4λ���� 
		{
		//���ݽ�����ϴ���
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

