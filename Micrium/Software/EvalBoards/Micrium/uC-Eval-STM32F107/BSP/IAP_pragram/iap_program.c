#include <includes.h>
#include "string.h"
#include    "csnr_package_deal.h"


#define	USER_APP_START_ADDR		(STM32_FLASH_BASE + 0x00010000	)
#define	USER_APP_PRO_SIZE		0x0000FFFF	

#define	USER_BACK_START_ADDR	(STM32_FLASH_BASE + 0x00020000	)

#define IAP_WRITE_1024          1024
#define	IAP_PARA_START_ADDR     (STM32_FLASH_BASE + 0x00030000)
#define	IAP_PARA_PRO_SIZE		0x0000FFFF	

 

////////////////////////////////////////////
extern  void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize);

int  GetSysBaseTick(void);

uint32  GetSysTime(void)
{
    uint32    tmp64;
    tmp64 = GetSysBaseTick();
    
    return     (tmp64);      
}


void    FeedDog(void)
{
    IWDG_ReloadCounter();
}

//void   DelayX10ms(u32   delay)
//{
////    uint32  i= 1000000;
////    while(i--);
//      return;
//}

void	DelayX10ms(uint32	Time)
{
	uint32	i,SysTime;
	for(i=0;i<Time;i++)
	{
		SysTime = GetSysTime();
		
		while(GetSysTime()-SysTime == 0)
		{
			FeedDog();
		}		
	}
}

#define zyIrqDisable()  CPU_INT_DIS()
#define zyIrqEnable()   CPU_INT_EN()


//对flash 进行擦写
//默认进入长度len=128
#define		MAX_SEC_NUM   22
void	IAP_pragramDeal(uint8	*buf,uint32	len)
{
	static	uint32	curaddr = USER_APP_START_ADDR;
	static	uint8	iapbuf[2048];
	static	uint32	iapbuflen = 0;
	static	uint8   times = 0;
    
    CPU_SR_ALLOC();
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

void iwdg_init(void){
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);//??IWDG_PR?IWDG_RLR????
    IWDG_SetReload(0xfff);//???????0xfff
    IWDG_SetPrescaler(IWDG_Prescaler_64);//????????32   
    IWDG_Enable();//?????
}


void	DataSend_CSNR(unsigned char SourceAddr,unsigned char DistAddr,unsigned char	*DataBuf,unsigned int	DataLen);
extern	unsigned char  CSNR_GetData(unsigned char	*RecBuf,unsigned char RecLen,unsigned char	*DataBuf,unsigned char	*InfoLen) ;
void    IAP_ProgramCopy(void);
void printfcom1(const char *format, ...);
void    HoldUartRecSta(void);                                        //维护串口状态
u32 ReadUartBuf(u8 *buf,u32 len);        //从串口缓存区读数据
extern  uint8	GetRecFramNum(void);
extern  void	SetFramNum(uint8 num);
extern  uint8	GetFramNum(void);
extern  void Jumpto_APP(void);

char    startstring[]={"IAP_pragram start!"};
char    pramstring[]={"."};
char    holdstring[]={"!"};
char    endstring[]={"IAP_pragram end!"};

uint8		recbuf[256];
uint8		databuf[256];

void	IAP_PragramTask(void)
{
		static	    uint32  	time = 0;
		static	    uint8		praflg = 0;
        static     uint8       comflg = 0; 
		

//		uint32		reclen 	= 0;	
		uint32		datalen = 0;
        
		uint32		tmp;

//		char		*p;
//		uint32		i;
		static		uint8	lstrecnum = 0;
        
        //while(1)
        printfcom1("\r\n");
        printfcom1("\r\nboot start stm32 V1.01");

		time = GetSysTime();
        iwdg_init();

		while(1)
		{
            DelayX10ms(1);

            HoldUartRecSta();
            
            tmp = ReadUartBuf(recbuf,sizeof(recbuf));   //è?′??úêy?Y

            //if(GetFramRecEndFlgIAP())
            if(tmp > 10 )
            {
//                tmp = ReadRs485DataIAP(recbuf);
//                ClearFramRecEndFlgIAP();
                if(tmp == 0)
                    continue ;
        
                comflg = 1;
                CSNR_GetData(recbuf,tmp,databuf,(uint8 *)&datalen); 

                if(praflg == 0)
                {
                    databuf[datalen] = '\0';
                    
                    if(strcmp((char *)&databuf[0],startstring) == 0)				//±à3ì?D?¨
                    {
                        lstrecnum = GetRecFramNum();
                        SetFramNum(lstrecnum);

                        praflg = 1;
                        DataSend_CSNR(0x80,0xA1,(uint8 *)startstring,strlen(startstring));	   
                    
                        printfcom1(" s:%s\r\n",startstring);
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

                        IAP_pragramDeal(databuf,datalen);					//±à3ì						   	
#ifdef CVI													
                        databuf[datalen] = '\0';
                        printfcom1texbox(databuf);
#endif				
                        DataSend_CSNR(0x80,0xA1,pramstring,strlen(pramstring));	 
                        printfcom1("%s",pramstring);
                        
                        if(datalen != 128)												//?áê?±à3ì
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
                if(comflg == 0)
                {
                    if(GetSysTime() - time > 1000)										//í?3?
                    {
                        printfcom1("\r\nGetSysTime() - time > 1000");
                        break;
                    }
                }
                else
                {
                    if(GetSysTime() - time > 1500)										//í?3?
                    {
                        printfcom1("\r\nGetSysTime() - time > 1000");
                        break;
                    }
                }
            }
            else
            {
                if(GetSysTime() - time > 2500)										//í?3?
                {
                    printfcom1("\r\nGetSysTime() - time > 2500");
                    break;
                }
            }
		}	
		
//		p =  (uint8 *)USER_APP_START_ADDR;
//		for(i = 0;i < 0x20000;i++ )
//		{
//			printfcom1("%02x",*p);
//			p++;
//		}

        //20170812 D???±?·Y????±′1|?ü?￡
        
        IAP_ProgramCopy();
        
		printfcom1("\r\nboot end\r\n");
        
        iwdg_init();
        FeedDog();
		//Boot();
        Jumpto_APP();
}

typedef  struct   _stcIAPPara_
{
    uint16  hardver;        //????
    uint16  softver;        //????
    uint32  softsize;       //????
    uint32  addr;           //????
    uint32  framenum;       //???
    uint16  code;           //??? 01,????????
    uint16  crc16;
}stcIAPPara;

/*******************************************************************************
 * ?    ?: IAP_WriteParaFlash
 * ?    ?: ?IAP??????Flash?
 * ????: stcIAPCtrl
 * ????: ?
 * ?  ?: redmorningcn.
 * ????: 2017-08-08
 * ?    ?:
 * ????:
 *******************************************************************************/
void    IAP_ReadParaFlash(stcIAPPara *sIAPPara)
{
    CPU_SR_ALLOC();

    uint32_t result[4];
    
    zyIrqDisable();                                 //???
    
    memcpy((uint8 *)sIAPPara,(uint8 *)IAP_PARA_START_ADDR,sizeof(stcIAPPara));
            
    zyIrqEnable();                                  //?????
}


/*******************************************************************************
 * ?    ?: IAP_WriteParaFlash
 * ?    ?: ?IAP??????Flash?
 * ????: stcIAPCtrl
 * ????: ?
 * ?  ?: redmorningcn.
 * ????: 2017-08-08
 * ?    ?:
 * ????:
 *******************************************************************************/
void    IAP_WriteParaFlash(stcIAPPara *sIAPPara)
{
    CPU_SR_ALLOC();

    //IAP_STATUS_CODE status;
    uint32_t result[4];

//    uint32_t flash_prog_area_sec_start;
//    uint32_t flash_prog_area_sec_end;
    
    zyIrqDisable();										                                //???

//    flash_prog_area_sec_start   = 	GetSecNum(IAP_PARA_START_ADDR);
//    flash_prog_area_sec_end 	=  	GetSecNum(IAP_PARA_START_ADDR + IAP_PARA_PRO_SIZE); //????
//
//    status = EraseSector(flash_prog_area_sec_start, flash_prog_area_sec_end);           //??????
//    status = BlankCheckSector(flash_prog_area_sec_start, flash_prog_area_sec_end,
//                &result[0], &result[1]);
    
     iap_write_appbin(     IAP_PARA_START_ADDR,
                            (uint8 *)sIAPPara,
                            256
                            );           
    //??flash??,??????
//    status 	= CopyRAM2Flash( (uint8_t *)IAP_PARA_START_ADDR,
//                                  (uint8 *)sIAPPara,
//                                   256
//                            );                                                          //????
//    
//    status =  Compare((uint8_t *)IAP_PARA_START_ADDR,
//                            (uint8 *)sIAPPara,
//                            sizeof(stcIAPPara)
//                    );                                                                  //????
            
    zyIrqEnable();                                                                      //?flash???,?????
}

uint8 buf[1024];
/*******************************************************************************
 * ?    ?: IAP_WriteFlash
 * ?    ?: ?????Flash?
 * ????: stcIAPCtrl
 * ????: ?
 * ?  ?: redmorningcn.
 * ????: 2017-08-08
 * ?    ?:
 * ????:
 *******************************************************************************/
void    IAP_CopyAppToFlash(stcIAPPara *sIAPPara)
{
    //IAP_STATUS_CODE status;
    CPU_SR_ALLOC();
    uint32_t result[4];
    //?áè?flashêy?Y
    uint32_t finshlen = 0;
    int i;
    
    uint32_t flash_prog_area_sec_start;
    uint32_t flash_prog_area_sec_end;

    zyIrqDisable();										//???
    
    //2?3y??òaD′μ???óò
//    flash_prog_area_sec_start   = 	GetSecNum(USER_APP_START_ADDR);
//    flash_prog_area_sec_end 	=  	GetSecNum(USER_APP_START_ADDR + USER_APP_PRO_SIZE);//????
//
//    status = EraseSector(flash_prog_area_sec_start, flash_prog_area_sec_end);           //???????
//
//    status = BlankCheckSector(flash_prog_area_sec_start, flash_prog_area_sec_end,
//                  &result[0], &result[1]);

    finshlen = 0;
    //μ??·?????òò?μ?êy?Y??3¤?è￡?í?3?
    while(  finshlen < USER_APP_PRO_SIZE 
        &&  finshlen < sIAPPara->softsize )
    
    {
        memcpy(buf,(uint8 *)(USER_BACK_START_ADDR + finshlen),IAP_WRITE_1024);   //è?backup????êy?Y
        
//        status 	= CopyRAM2Flash((uint8 *)(USER_APP_START_ADDR + finshlen),       //??backup??êy?YD′è?app??   
//                                         buf,
//                                         IAP_WRITE_1024
//                                );                      
        iap_write_appbin((USER_APP_START_ADDR + finshlen),       //??backup??êy?YD′è?app??   
                                         buf,
                                         IAP_WRITE_1024);
//        status =  Compare((uint8 *)(USER_APP_START_ADDR + finshlen),             //±è??D′è??úèY     
//                                     buf,
//                                     IAP_WRITE_1024
//                                );
        
        finshlen += IAP_WRITE_1024;                     //μ??·à??ó  
        
        //for(i = 0; i< IAP_WRITE_1024;i++)
        {
        //    printfcom1("%02x",buf[i]);
        }
        FeedDog();
    }   
    
    zyIrqEnable();                                  //?flash???,?????   0731-22689503   6246  7405
}
#define     IAP_COPY_BACK_APP    0x01
uint16 GetCrc16Check(uint8 *Buf, uint16 Len);
/*******************************************************************************
 * ?    ?: IAP_WriteFlash
 * ?    ?: ?????Flash?
 * ????: stcIAPCtrl
 * ????: ?
 * ?  ?: redmorningcn.
 * ????: 2017-08-08
 * ?    ?:
 * ????:
 *******************************************************************************/
void    IAP_ProgramCopy(void)
{
    uint16  crc16;
    
    stcIAPPara  sIapPara;
        
    IAP_ReadParaFlash(&sIapPara);                          //?áIAP2?êy
    
    crc16 = GetCrc16Check((uint8 *)&sIapPara,sizeof(stcIAPPara)-2);  //??D￡?é
    
    FeedDog();

    if( crc16 == sIapPara.crc16 &&                          //D￡?é?yè·￡??òcopy??á?￡??ò?′DDcopy2ù×÷,
        sIapPara.code == IAP_COPY_BACK_APP &&               //êy?Y?? êy?Y′óD? ê?·?óDD§
        sIapPara.softsize > 0x100 &&
        sIapPara.softsize < USER_APP_PRO_SIZE
      )
    {
        // printfcom1("\r\n???ˉ?ó??.....\r\n");

        IAP_CopyAppToFlash(&sIapPara);                      //êy?Y′óback??copy?áapp??

        sIapPara.code   = 0;
        crc16 = GetCrc16Check((uint8 *)&sIapPara,sizeof(stcIAPPara)-2);  //????D￡?é
        sIapPara.crc16  = crc16;             
        //DT??copy??á?￡???D?D′è?
        IAP_WriteParaFlash(&sIapPara);
    
    }
}
