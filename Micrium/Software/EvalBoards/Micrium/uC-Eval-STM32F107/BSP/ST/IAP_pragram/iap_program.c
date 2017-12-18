//#include <includes.h>
#include    "delay.h" 
#include    "sensorpulse.h" 
#include    "csnr_package_deal.h"

#define         FRAM_HERD0     		0x10         	//����ͷ
#define         FRAM_HERD1     		0x28       		//����ͷ				
#define     	HOST_ID      		10				//��λ����ַ	  		   
#define   		DEVICE_ID0      	15				//������ַ0	
#define   		DEVICE_ID1      	15				//������ַ1	
	   
#define			RS485_CHK_SUM		0x02			//b0001��CRC��ʽ��b0010���ۼӺͷ�ʽ�� b0011;�ۼӺͶ����Ʋ��뷽ʽ 
#define			RS485_CHK_CRC		0x01			//b0001��CRC��ʽ��b0010���ۼӺͷ�ʽ�� b0011;�ۼӺͶ����Ʋ��뷽ʽ 
#define			RS485_CHK_RESUM		0x03			//b0001��CRC��ʽ��b0010���ۼӺͷ�ʽ�� b0011;�ۼӺͶ����Ʋ��뷽ʽ 

#define         FRAM_END0     		0x10         	//����β
#define         FRAM_END1     		0x2C       		//����β	

////////////////////////////////////mdk 0

 
//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH����ʼ��ַ
#define	USER_APP_START_ADDR		   (STM32_FLASH_BASE + 0x00010000)

//FLASH ��������ʼ��ַ
#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//����0��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//����1��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//����2��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//����3��ʼ��ַ, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//����4��ʼ��ַ, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	//����5��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	//����6��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	//����7��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	//����8��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	//����9��ʼ��ַ, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	//����10��ʼ��ַ,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	//����11��ʼ��ַ,128 Kbytes  

 

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
// ��    �ƣ�   
// ��    �ܣ�   ��
// ��ڲ�����   ��
// ���ڲ�����   ��
//----------------------------------------------------------------------------
unsigned char  CSNR_GetData(unsigned char	*RecBuf,unsigned char RecLen,unsigned char	*DataBuf,unsigned char	*InfoLen)
{
    unsigned short		i,j,k;
    unsigned char		SumCheck8;
    unsigned char		Tmp;
    unsigned short		RecCRC16,Check16;
	
	unsigned char 		SourceAddr;	
	unsigned char		DataLen;

  
	for(j = 0;j < RecLen ;j++ )														//ȥ�ظ�ͨѶ���ֽ�
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
	
    for(i = 0; i < RecLen - 10;i++)												//ѭ������
    {
		if(		((RecBuf[i+0]) == FRAM_HERD0) 
			&& 	((RecBuf[i+1]) == FRAM_HERD1)		
		  )																		//��ͷ
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
			 	//i = i + 10 + DataLen;											//��ַ���Ի�֡β����������֡
				  printfcom1("\r\n		(i+10+DataLen)  > RecLen	 %d,%d",i+10+DataLen,RecLen);	
				 continue;
			 }

			SumCheck8 = GetCheckSumNR((unsigned char *)&RecBuf[i+2],5);         //������У��
			 
			 if(   SumCheck8 != RecBuf[i+7] )
			 {
					
				 printfcom1("\r\n sunche8  ");
				 continue;
			 }

			SourceAddr      = RecBuf[i+2];	
			l_recslaveaddr  = RecBuf[i+3];	
			l_recframnum    = RecBuf[i+4];
			//printfcom1("\r\n �� %x",RecBuf[i+4]);				
			DataLen = RecBuf[i+6];
             //   printfcom1("\r\n DataLen %d",DataLen);				

			*InfoLen = DataLen; 
			
						
			RecCRC16 = ((unsigned short)RecBuf[i+8+DataLen]) *256 + RecBuf[i+9+DataLen];	//У��	

			switch(RecBuf[i+5]>>4)													//У����㷨�ж�
			{
                case	RS485_CHK_SUM:
                                //Check16 = GetCheckSum16(&RecBuf[i+2],6+DataLen);
                                ////printfcom1(("\r\n 16λУ��ͣ�");
                                break;
                case	RS485_CHK_CRC:
                                Check16 = GetCrc16Check(&RecBuf[i+2],6+DataLen);
                                ////printfcom1(("\r\n CRCУ�飡");
                                break;
                case	RS485_CHK_RESUM:
                                break;
			}
			
            

			if(		RecCRC16  ==	Check16 	  )
			{
				 memcpy(DataBuf,&RecBuf[i+8],DataLen);				//���ݿ���
				 
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
	static	unsigned char FramNum =0;						//���
	
	ByteStation = 0;
	SndBuf[ByteStation++] = FRAM_HERD0;
	SndBuf[ByteStation++] = FRAM_HERD1;
	SndBuf[ByteStation++] = SourceAddr;	 
	SndBuf[ByteStation++] = DistAddr;				//��ַ
//	SndBuf[ByteStation++] = FramNum++;	
	SndBuf[ByteStation++] = GetFramNum();
	SndBuf[ByteStation++] = RS485_CHK_CRC<<4;	
	SndBuf[ByteStation++] = DataLen;
	
	SumCheck8 = GetCheckSumNR(&SndBuf[2],5);		   	//������У��

	SndBuf[ByteStation++] = SumCheck8;

	memcpy(&SndBuf[8],DataBuf,DataLen);

	Crc16 = GetCrc16Check(&SndBuf[2],6+DataLen);

	//memcpy(&SndBuf[2+6+DataLen],(unsigned char *)&Crc16,sizeof(Crc16));
	
	SndBuf[2+6+DataLen+1] = Crc16;
	SndBuf[2+6+DataLen]   = Crc16>>8;

	SndBufTmp[0] = SndBuf[0];						//׼�����ݷ���
	SndBufTmp[1] = SndBuf[1];
	
	AddHeadNum = 0;
	for(i = 2; i< 2+6+DataLen + 2;i++ )				//���ݣ�����λ FRAM_HERD0
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
	
	//��ַ�ж�
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


//����app
void Boot( void )
{
//	 void (*userProgram)();   					           /*����ָ��*/

	//SCB->VTOR = USER_APP_START_ADDR & 0x1FFFFF80;	//ƫ���ж�����
    SCB->VTOR = USER_APP_START_ADDR & 0x1FFFFF80;	//ƫ���ж�����

	JMP_Boot(USER_APP_START_ADDR);
//	userProgram = (void (*)()) (USER_APP_START_ADDR+1);
//	(*userProgram)();													/*����						*/	 
	
}


//#include "stmflash.h"
#include "delay.h"
#include "usart.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//STM32�ڲ�FLASH��д ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/9
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

//#define	USER_APP_START_ADDR		    0x00010000 	

//��ȡָ����ַ�İ���(16λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu32*)faddr; 
}  
//��ȡĳ����ַ���ڵ�flash����
//addr:flash��ַ
//����ֵ:0~11,��addr���ڵ�����
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
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ر�ע��:��ΪSTM32F4������ʵ��̫��,û�취���ر�����������,���Ա�����
//         д��ַ�����0XFF,��ô���Ȳ������������Ҳ�������������.����
//         д��0XFF�ĵ�ַ,�����������������ݶ�ʧ.����д֮ǰȷ��������
//         û����Ҫ����,��������������Ȳ�����,Ȼ����������д. 
//�ú�����OTP����Ҳ��Ч!��������дOTP��!
//OTP�����ַ��Χ:0X1FFF7800~0X1FFF7A0F
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!)
//pBuffer:����ָ��
//NumToWrite:��(32λ)��(����Ҫд���32λ���ݵĸ���.) 
void STMFLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite)	
{ 
  FLASH_Status status = FLASH_COMPLETE;
	u32 addrx=0;
	u32 endaddr=0;	
  if(WriteAddr<STM32_FLASH_BASE||WriteAddr%4)return;	//�Ƿ���ַ
	FLASH_Unlock();									//���� 
  FLASH_DataCacheCmd(DISABLE);//FLASH�����ڼ�,�����ֹ���ݻ���
 		
	addrx=WriteAddr;				//д�����ʼ��ַ
	endaddr=WriteAddr+NumToWrite*4;	//д��Ľ�����ַ
	if(addrx<0X1FFF0000)			//ֻ�����洢��,����Ҫִ�в�������!!
	{
		while(addrx<endaddr)		//ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
		{
			if(STMFLASH_ReadWord(addrx)!=0XFFFFFFFF)//�з�0XFFFFFFFF�ĵط�,Ҫ�����������
			{   
				status=FLASH_EraseSector(STMFLASH_GetFlashSector(addrx),VoltageRange_3);//VCC=2.7~3.6V֮��!!
				if(status!=FLASH_COMPLETE)break;	//����������
			}else addrx+=4;
		} 
	}
	if(status==FLASH_COMPLETE)
	{
		while(WriteAddr<endaddr)//д����
		{
			if(FLASH_ProgramWord(WriteAddr,*pBuffer)!=FLASH_COMPLETE)//д������
			{ 
				break;	//д���쳣
			}
			WriteAddr+=4;
			pBuffer++;
		} 
	}
  FLASH_DataCacheCmd(ENABLE);	//FLASH��������,�������ݻ���
	FLASH_Lock();//����
} 

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToRead:��(4λ)��
void STMFLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead)   	
{
	u32 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//��ȡ4���ֽ�.
		ReadAddr+=4;//ƫ��4���ֽ�.	
	}
}
typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.   

iapfun jump2app; 
u32 iapbuf[512]; 	//2K�ֽڻ���  
//appxaddr:Ӧ�ó������ʼ��ַ
//appbuf:Ӧ�ó���CODE.
//appsize:Ӧ�ó����С(�ֽ�).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u32 t;
	u16 i=0;
	u32 temp;
	u32 fwaddr=appxaddr;//��ǰд��ĵ�ַ
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=4)
	{						   
		temp =(u32)dfu[3]<<24;   
		temp|=(u32)dfu[2]<<16;    
		temp|=(u32)dfu[1]<<8;
		temp|=(u32)dfu[0];	  
		dfu +=4;//ƫ��4���ֽ�
		iapbuf[i++]=temp;	    
		if( i==512 )
		{
			i=0; 
			STMFLASH_Write(fwaddr,iapbuf,512);
			fwaddr+=2048;//ƫ��2048  512*4=2048
		}
	} 
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//������һЩ�����ֽ�д��ȥ.  
}

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		MSR_MSP(*(vu32*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		jump2app();									//��ת��APP.
	}
}		 

//��flash ���в�д
//Ĭ�Ͻ��볤��len=128
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


        zyIrqDisable();										//���ж�
        
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
        printfcom1("\r\n ׼��������");
        		
		while(1)
		{
            DelayX10ms(1);
            
            HoldUartRecSta();                           //ɨ�贮��

            tmp = ReadUartBuf(recbuf,sizeof(recbuf));   //ȡ��������
            
            if( tmp > 10 )                              //������ݴ���10�����к�������
            {
                //printfcom1("\r\n tmp %d",tmp);
                CSNR_GetData(recbuf,tmp,databuf,(uint8 *)&datalen); //�԰����ݽ��н��� 

                //printfcom1("\r\n praflg %d",praflg);

                if(praflg == 0)                         //���ر��
                {
                    databuf[datalen] = '\0';
                
                    printfcom1("\r\n lpraflg %d",datalen);
                    
                    printfcom1("\r\n");
                    for(u8 i=0;i< datalen;i++)
                        printfcom1("%c",databuf[i]);
                    //
                    //printfcom1("\r\n string %s",databuf);

                    if(strcmp((char *)&databuf[0],startstring) == 0)				//����ж�
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

                        IAP_pragramDeal(databuf,datalen);					//���						   	
#ifdef CVI													
                        databuf[datalen] = '\0';
                        printfcom1texbox(databuf);
#endif				
                        DataSend_CSNR(0x80,0xA1,pramstring,strlen(pramstring));	 
                        printfcom1("%s",pramstring);
                        
                        if(datalen != 128)												//�������
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
                if(GetSysTime() - time > 200)										//�˳�
                {
                    printfcom1("\r\nGetSysTime() - time > 1000");
                    break;
                }
            }
            else
            {
                if(GetSysTime() - time > 2000)										//�˳�
                {
                    printfcom1("\r\nGetSysTime() - time > 1000");
                    break;
                }
            }
		}	
		
		printfcom1("\r\n boot end");

        iwdg_init();//�������Ź�
		Boot();
}
