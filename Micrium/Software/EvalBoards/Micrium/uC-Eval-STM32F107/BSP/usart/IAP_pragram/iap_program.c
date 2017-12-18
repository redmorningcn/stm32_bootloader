//#include <includes.h>
#include    "delay.h" 
#include    "sensorpulse.h" 
#include    "csnr_package_deal.h"

#define         FRAM_HERD0     		0x10         	//报文头
#define         FRAM_HERD1     		0x28       		//报文头				
#define     	HOST_ID      		10				//上位机地址	  		   
#define   		DEVICE_ID0      	15				//本机地址0	
#define   		DEVICE_ID1      	15				//本机地址1	
	   
#define			RS485_CHK_SUM		0x02			//b0001：CRC方式；b0010：累加和方式； b0011;累加和二进制补码方式 
#define			RS485_CHK_CRC		0x01			//b0001：CRC方式；b0010：累加和方式； b0011;累加和二进制补码方式 
#define			RS485_CHK_RESUM		0x03			//b0001：CRC方式；b0010：累加和方式； b0011;累加和二进制补码方式 

#define         FRAM_END0     		0x10         	//报文尾
#define         FRAM_END1     		0x2C       		//报文尾	

////////////////////////////////////mdk 0

 
//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH的起始地址
#define	USER_APP_START_ADDR		   (STM32_FLASH_BASE + 0x00010000)

//FLASH 扇区的起始地址
#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//扇区0起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//扇区1起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//扇区2起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//扇区3起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//扇区4起始地址, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	//扇区5起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	//扇区6起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	//扇区7起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	//扇区8起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	//扇区9起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	//扇区10起始地址,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	//扇区11起始地址,128 Kbytes  

 

////////////////////////////////////////////

unsigned char   l_recslaveaddr = 0;
unsigned char GetRecSlaveAddr(void)
{
	return	l_recslaveaddr;
}

uint32  GetSysTime(void)
{
    uint64_t    tmp64;
    tmp64 = GetSysBaseTick();
    
    return     (tmp64/((84000000/9)/100));      
}

void   DelayX10ms(u32   delay)
{
    u32 time = GetSysTime();
    while(GetSysTime()  < time + delay);

    return;
}
    

uint8	l_recframnum = 0;
uint8	GetRecFramNum(void)
{
	return 	l_recframnum;
}

//----------------------------------------------------------------------------
// 名    称：   
// 功    能：   接
// 入口参数：   无
// 出口参数：   无
//----------------------------------------------------------------------------
unsigned char  CSNR_GetData(unsigned char	*RecBuf,unsigned char RecLen,unsigned char	*DataBuf,unsigned char	*InfoLen)
{
    unsigned short		i,j,k;
    unsigned char		SumCheck8;
    unsigned char		Tmp;
    unsigned short		RecCRC16,Check16;
	
	unsigned char 		SourceAddr;	
	unsigned char		DataLen;

  
	for(j = 0;j < RecLen ;j++ )														//去重复通讯首字节
	{
		if(	 	RecBuf[j] 	== FRAM_HERD0
			&& 	RecBuf[j+1] == FRAM_HERD0
		   )
		{
			for(k = j;k < RecLen; k++)
			{
				Tmp	 = RecBuf[k+1];
				
				RecBuf[k] = Tmp;
			}
//201603			
			//j++;
		}
	}
	
    for(i = 0; i < RecLen - 10;i++)												//循环遍历
    {
		if(		((RecBuf[i+0]) == FRAM_HERD0) 
			&& 	((RecBuf[i+1]) == FRAM_HERD1)		
		  )																		//报头
		{
			DataLen = RecBuf[i+6];
			//printfcom1(("\r\n DataLen %d,%x,%x",DataLen,RecBuf[i+10+DataLen],RecBuf[i+11+DataLen]);
			if(		
					//	(RecBuf[i+2] != DeviceID) 				||
					  //	(RecBuf[i+3] != HOST_ID)            ||
						(i+10+DataLen)  > RecLen                || 
				 		(RecBuf[i+10+DataLen] 	!= FRAM_END0)	||
				 	 	(RecBuf[i+11+DataLen] 	!= FRAM_END1)
			 )
			 {	
			 	//i = i + 10 + DataLen;											//地址不对或帧尾错误，跳过此帧
				  printfcom1("\r\n		(i+10+DataLen)  > RecLen	 %d,%d",i+10+DataLen,RecLen);	
				 continue;
			 }

			SumCheck8 = GetCheckSumNR((unsigned char *)&RecBuf[i+2],5);         //索引区校验
			 
			 if(   SumCheck8 != RecBuf[i+7] )
			 {
					
				 printfcom1("\r\n sunche8  ");
				 continue;
			 }

			SourceAddr      = RecBuf[i+2];	
			l_recslaveaddr  = RecBuf[i+3];	
			l_recframnum    = RecBuf[i+4];
			//printfcom1("\r\n 号 %x",RecBuf[i+4]);				
			DataLen = RecBuf[i+6];
             //   printfcom1("\r\n DataLen %d",DataLen);				

			*InfoLen = DataLen; 
			
						
			RecCRC16 = ((unsigned short)RecBuf[i+8+DataLen]) *256 + RecBuf[i+9+DataLen];	//校验	

			switch(RecBuf[i+5]>>4)													//校验和算法判断
			{
                case	RS485_CHK_SUM:
                                //Check16 = GetCheckSum16(&RecBuf[i+2],6+DataLen);
                                ////printfcom1(("\r\n 16位校验和！");
                                break;
                case	RS485_CHK_CRC:
                                Check16 = GetCrc16Check(&RecBuf[i+2],6+DataLen);
                                ////printfcom1(("\r\n CRC校验！");
                                break;
                case	RS485_CHK_RESUM:
                                break;
			}
			
            

			if(		RecCRC16  ==	Check16 	  )
			{
				 memcpy(DataBuf,&RecBuf[i+8],DataLen);				//数据拷贝
				 
				 //return	1;				//SourceAddr
                //printfcom1("\r\nRecCRC16 %x %x",RecCRC16,Check16);				

				 return		SourceAddr;
			}
		}
	}
  
    return	0;
}

uint8	l_FramNum = 0;
uint8	GetFramNum(void)
{
	return l_FramNum;		
}

void	SetFramNum(uint8 num)
{
	l_FramNum = num;		
}

void	AddFramNum(void)
{
	l_FramNum++;		
}

void	DataSend_CSNR(unsigned char SourceAddr,unsigned char DistAddr,unsigned char	*DataBuf,unsigned int	DataLen)
{
	unsigned char	SndBuf[250] = {0};
	unsigned char	SndBufTmp[250] = {0};
	unsigned char	SumCheck8;
	unsigned short	Crc16;
	unsigned short	AddHeadNum;
	unsigned short	i;
	unsigned char	ByteStation;
	static	unsigned char FramNum =0;						//序号
	
	ByteStation = 0;
	SndBuf[ByteStation++] = FRAM_HERD0;
	SndBuf[ByteStation++] = FRAM_HERD1;
	SndBuf[ByteStation++] = SourceAddr;	 
	SndBuf[ByteStation++] = DistAddr;				//地址
//	SndBuf[ByteStation++] = FramNum++;	
	SndBuf[ByteStation++] = GetFramNum();
	SndBuf[ByteStation++] = RS485_CHK_CRC<<4;	
	SndBuf[ByteStation++] = DataLen;
	
	SumCheck8 = GetCheckSumNR(&SndBuf[2],5);		   	//索引区校验

	SndBuf[ByteStation++] = SumCheck8;

	memcpy(&SndBuf[8],DataBuf,DataLen);

	Crc16 = GetCrc16Check(&SndBuf[2],6+DataLen);

	//memcpy(&SndBuf[2+6+DataLen],(unsigned char *)&Crc16,sizeof(Crc16));
	
	SndBuf[2+6+DataLen+1] = Crc16;
	SndBuf[2+6+DataLen]   = Crc16>>8;

	SndBufTmp[0] = SndBuf[0];						//准备数据发送
	SndBufTmp[1] = SndBuf[1];
	
	AddHeadNum = 0;
	for(i = 2; i< 2+6+DataLen + 2;i++ )				//数据，补移位 FRAM_HERD0
	{
		SndBufTmp[i+AddHeadNum] = SndBuf[i];
		
		if(SndBuf[i] == FRAM_HERD0)
		{
			SndBufTmp[i+AddHeadNum+1] = FRAM_HERD0;
			AddHeadNum++;
		}
	}

	SndBufTmp[2+6+DataLen + 2 +AddHeadNum] = FRAM_END0;
	SndBufTmp[2+6+DataLen + 2 +AddHeadNum+1] = FRAM_END1;      
	
	//地址判断
//	if((DistAddr & 0xA0) == 0xA0)
//	{
//		SendCOM1(SndBufTmp, 2+6+DataLen + 2 +AddHeadNum +2) ;
    SendDataCom1(SndBufTmp, 2+6+DataLen + 2 +AddHeadNum +2) ;
	
//	}
//	else if((DistAddr & 0xC0) == 0xC0)
//	{
//		SendCom0(SndBufTmp, 2+6+DataLen + 2 +AddHeadNum +2) ; 	
//	}
//	else
//	{
//		SendCom0(SndBufTmp, 2+6+DataLen + 2 +AddHeadNum +2) ; 	
//	}
	//SendCOM1(SndBufTmp,2+6+DataLen + 2 + 2 + AddHeadNum);
	//SendCOM0(SndBufTmp,2+6+DataLen + 2 + 2 + AddHeadNum);
}



__asm void JMP_Boot( uint32_t address ){
   LDR SP, [R0]		;Load new stack pointer address
   LDR PC, [R0, #4]	;Load new program counter address
}

#define zyIrqDisable()  __disable_irq()
#define zyIrqEnable()   __enable_irq()


//启动app
void Boot( void )
{
//	 void (*userProgram)();   					           /*函数指针*/

	//SCB->VTOR = USER_APP_START_ADDR & 0x1FFFFF80;	//偏移中断向量
    SCB->VTOR = USER_APP_START_ADDR & 0x1FFFFF80;	//偏移中断向量

	JMP_Boot(USER_APP_START_ADDR);
//	userProgram = (void (*)()) (USER_APP_START_ADDR+1);
//	(*userProgram)();													/*启动						*/	 
	
}


//#include "stmflash.h"
#include "delay.h"
#include "usart.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//STM32内部FLASH读写 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/9
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

//#define	USER_APP_START_ADDR		    0x00010000 	

//读取指定地址的半字(16位数据) 
//faddr:读地址 
//返回值:对应数据.
u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu32*)faddr; 
}  
//获取某个地址所在的flash扇区
//addr:flash地址
//返回值:0~11,即addr所在的扇区
uint16_t STMFLASH_GetFlashSector(u32 addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}
//从指定地址开始写入指定长度的数据
//特别注意:因为STM32F4的扇区实在太大,没办法本地保存扇区数据,所以本函数
//         写地址如果非0XFF,那么会先擦除整个扇区且不保存扇区数据.所以
//         写非0XFF的地址,将导致整个扇区数据丢失.建议写之前确保扇区里
//         没有重要数据,最好是整个扇区先擦除了,然后慢慢往后写. 
//该函数对OTP区域也有效!可以用来写OTP区!
//OTP区域地址范围:0X1FFF7800~0X1FFF7A0F
//WriteAddr:起始地址(此地址必须为4的倍数!!)
//pBuffer:数据指针
//NumToWrite:字(32位)数(就是要写入的32位数据的个数.) 
void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite)	
{ 
  FLASH_Status status = FLASH_COMPLETE;
	u32 addrx=0;
	u32 endaddr=0;	
  if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)return;	//非法地址
	FLASH_Unlock();									//解锁 
  FLASH_DataCacheCmd(DISABLE);//FLASH擦除期间,必须禁止数据缓存
 		
	addrx=WriteAddr;				//写入的起始地址
	endaddr=WriteAddr+NumToWrite*4;	//写入的结束地址
	if(addrx<0X1FFF0000)			//只有主存储区,才需要执行擦除操作!!
	{
		while(addrx<endaddr)		//扫清一切障碍.(对非FFFFFFFF的地方,先擦除)
		{
			if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//有非0XFFFFFFFF的地方,要擦除这个扇区
			{   
				status=FLASH_EraseSector(STMFLASH_GetFlashSector(addrx),VoltageRange_3);//VCC=2.7~3.6V之间!!
				if(status!=FLASH_COMPLETE)break;	//发生错误了
			}else addrx+=4;
		} 
	}
	if(status==FLASH_COMPLETE)
	{
		while(WriteAddr<endaddr)//写数据
		{
			if(FLASH_ProgramWord(WriteAddr,*pBuffer)!=FLASH_COMPLETE)//写入数据
			{ 
				break;	//写入异常
			}
			WriteAddr+=4;
			pBuffer++;
		} 
	}
  FLASH_DataCacheCmd(ENABLE);	//FLASH擦除结束,开启数据缓存
	FLASH_Lock();//上锁
} 

//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToRead:字(4位)数
void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead)   	
{
	u32 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//读取4个字节.
		ReadAddr+=4;//偏移4个字节.	
	}
}
typedef  void (*iapfun)(void);				//定义一个函数类型的参数.   

iapfun jump2app; 
u32 iapbuf[512]; 	//2K字节缓存  
//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u32 t;
	u16 i=0;
	u32 temp;
	u32 fwaddr=appxaddr;//当前写入的地址
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=4)
	{						   
		temp =(u32)dfu[3]<<24;   
		temp|=(u32)dfu[2]<<16;    
		temp|=(u32)dfu[1]<<8;
		temp|=(u32)dfu[0];	  
		dfu +=4;//偏移4个字节
		iapbuf[i++]=temp;	    
		if( i==512 )
		{
			i=0; 
			STMFLASH_Write(fwaddr,iapbuf,512);
			fwaddr+=2048;//偏移2048  512*4=2048
		}
	} 
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.  
}

//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app();									//跳转到APP.
	}
}		 

//对flash 进行擦写
//默认进入长度len=128
#define		MAX_SEC_NUM   22
void	IAP_pragramDeal(uint8	*buf,uint32	len)
{
	static	uint32	curaddr = USER_APP_START_ADDR;
	static	uint8	iapbuf[2048];
	static	uint32	iapbuflen = 0;
	static	uint8   times = 0;

    times++;
    memcpy(&iapbuf[iapbuflen],buf,len);     //
    
    iapbuflen +=len;
	   
    if(times == 1024/128 || len != 128)
    {
        if(len != 128)
        {
            for(u32 i = iapbuflen;i<1024;i++ )
                iapbuf[i] = 0xff;	
        }


        zyIrqDisable();										//关中断
        
        iap_write_appbin(curaddr,iapbuf,1024);

        zyIrqEnable();
        
        curaddr += 1024;
        times = 0;
        iapbuflen = 0;
    }
		//DelayX10ms(2);	
}



void	IAP_PragramTask(void)
{
		static	uint32  	time = 0;
		static	uint8		praflg = 0;
		
		uint8		recbuf[256];
		uint8		databuf[256];
		uint32		reclen 	= 0;	
		uint32		datalen = 0;
		uint32		tmp;
		char		startstring[]=  {"IAP_pragram start!"};
		char		pramstring[] =  {"."};
		char		holdstring[] =  {"!"};
		char		endstring[]  =  {"IAP_pragram end!"};
		char		*p;
		uint32		i;
		static		uint8	lstrecnum = 0;
		
		time = GetSysTime();
        printfcom1("\r\n 准备。。。");
        		
		while(1)
		{
            DelayX10ms(1);
            
            HoldUartRecSta();                           //扫描串口

            tmp = ReadUartBuf(recbuf,sizeof(recbuf));   //取串口数据
            
            if( tmp > 10 )                              //如果数据大于10，进行后续操作
            {
                //printfcom1("\r\n tmp %d",tmp);
                CSNR_GetData(recbuf,tmp,databuf,(uint8 *)&datalen); //对包数据进行解析 

                //printfcom1("\r\n praflg %d",praflg);

                if(praflg == 0)                         //下载标记
                {
                    databuf[datalen] = '\0';
                
                    printfcom1("\r\n lpraflg %d",datalen);
                    
                    printfcom1("\r\n");
                    for(u8 i=0;i< datalen;i++)
                        printfcom1("%c",databuf[i]);
                    //
                    //printfcom1("\r\n string %s",databuf);

                    if(strcmp((char *)&databuf[0],startstring) == 0)				//编程判定
                    {
                        lstrecnum = GetRecFramNum();
                        SetFramNum(lstrecnum);

                        praflg = 1;
                        DataSend_CSNR(0x80,0xA1,startstring,strlen(startstring));	   
                    
                        DataSend_CSNR(0x80,0xA1,startstring,strlen(startstring));	   
                                           
                        //printfcom1(" s:%s\r\n",startstring);
                        time = GetSysTime();							
                        continue ;
                            //return;
                    }
                } 
                else																					
                {
                    printfcom1("\r\n lstrecnum %x,%x",lstrecnum, GetFramNum());
                    if(lstrecnum == GetRecFramNum())
                    {
                            DataSend_CSNR(0x80,0xA1,holdstring,strlen(holdstring));	
                            time = GetSysTime();
                    }
                    else
                    {	
                        lstrecnum = GetRecFramNum();
                        SetFramNum(lstrecnum);

                        IAP_pragramDeal(databuf,datalen);					//编程						   	
#ifdef CVI													
                        databuf[datalen] = '\0';
                        printfcom1texbox(databuf);
#endif				
                        DataSend_CSNR(0x80,0xA1,pramstring,strlen(pramstring));	 
                        printfcom1("%s",pramstring);
                        
                        if(datalen != 128)												//结束编程
                        {	
                            DataSend_CSNR(0x80,0xA1,endstring,strlen(endstring));	
                            printfcom1("\r\n len != 128");
                            break;
                        }

                        time = GetSysTime();
                    }
                }
            }
				
            if(praflg == 0)
            {
                if(GetSysTime() - time > 200)										//退出
                {
                    printfcom1("\r\nGetSysTime() - time > 1000");
                    break;
                }
            }
            else
            {
                if(GetSysTime() - time > 2000)										//退出
                {
                    printfcom1("\r\nGetSysTime() - time > 1000");
                    break;
                }
            }
		}	
		
		printfcom1("\r\n boot end");

        iwdg_init();//启动看门狗
		Boot();
}
