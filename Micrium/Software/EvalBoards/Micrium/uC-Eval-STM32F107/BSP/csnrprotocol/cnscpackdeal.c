#include  "csnr_package_deal.h"
#include  "includes.h"
//#include  "CrcCheck.h"


//数据打包，根据stcCsnrProtocolPara信息，将数据打包成CNSR数据
//
void	DataPackage_CSNR(stcCsnrProtocolPara sprotocolpara,unsigned char 	*csnrbuf,unsigned char 	*csnrlen)
{
	unsigned char		SndBuf[256] = {0};
	unsigned char		SumCheck8;
	unsigned short	    Crc16;
	unsigned short	    AddHeadNum;
	unsigned short	    i;
	unsigned char		ByteStation;
	unsigned char 	    FramNum =0;						//序号
	unsigned char		DataLen;
	
	ByteStation = 0;
	SndBuf[ByteStation++] = FRAM_HERD0;
	SndBuf[ByteStation++] = FRAM_HERD1;
	SndBuf[ByteStation++] = sprotocolpara.sourceaddr;	 
	SndBuf[ByteStation++] = sprotocolpara.destaddr;				//地址
	SndBuf[ByteStation++] = sprotocolpara.framnum;
	SndBuf[ByteStation++] = (RS485_CHK_CRC<<4) + sprotocolpara.framcode;	
	SndBuf[ByteStation++] = sprotocolpara.datalen;
	
	DataLen = sprotocolpara.datalen;
	
	SumCheck8 = GetCheckSumNR(&SndBuf[2],5);		   	//索引区校验

	SndBuf[ByteStation++] = SumCheck8;

	memcpy(&SndBuf[8],sprotocolpara.databuf,DataLen);

	Crc16 = GetCrc16Check(&SndBuf[2],6+sprotocolpara.datalen);
	
	SndBuf[2+6+DataLen+1] = Crc16;
	SndBuf[2+6+DataLen]   = Crc16>>8;

	csnrbuf[0] = SndBuf[0];						//准备数据发送
	csnrbuf[1] = SndBuf[1];
	
	AddHeadNum = 0;
	for(i = 2; i< 2+6+DataLen + 2;i++ )				//数据，补移位 FRAM_HERD0
	{
		csnrbuf[i+AddHeadNum] = SndBuf[i];
		
		if(SndBuf[i] == FRAM_HERD0)
		{
			csnrbuf[i+AddHeadNum+1] = FRAM_HERD0;
			AddHeadNum++;
		}
	}

	csnrbuf[2+6+DataLen + 2 +AddHeadNum] 		= FRAM_END0;
	csnrbuf[2+6+DataLen + 2 +AddHeadNum+1] 	= FRAM_END1;      
	
	*csnrlen =  2+6+DataLen + 2 + AddHeadNum +2;
}


//数据解包，将接受到的数据解包到，stcCsnrProtocolPara信息
//能简析，返回1；否则，返回0
unsigned char   DataUnpack_CSNR(stcCsnrProtocolPara *sprotocolpara,unsigned char	*RecBuf,unsigned char RecLen)
{
    unsigned short		i,j,k;
    unsigned char			SumCheck8;
    unsigned char			Tmp;
    unsigned short		RecCRC16,Check16;
		unsigned char			DataLen;
		unsigned int			herdtimes = 0;
		

  if(RecLen < 10)
		return 0;		
	
	for(j = 0;j < RecLen - herdtimes;j++ )														//去重复通讯首字节
	{
		if(	 	
				RecBuf[j] 	== FRAM_HERD0
			&& 	RecBuf[j+1] == FRAM_HERD0
		   )
		{
			for(k = j;k < RecLen - herdtimes; k++)
			{
				Tmp	 = RecBuf[k+1];
				
				RecBuf[k] = Tmp;
			}
			
			herdtimes++;
		}
	}
		

    for(i = 0; i < RecLen - herdtimes - 10;i++)												//循环遍历
    {
		if(		((RecBuf[i+0]) == FRAM_HERD0) 
			&& 	((RecBuf[i+1]) == FRAM_HERD1)		
		  )																		//报头
		{
			DataLen = RecBuf[i+6];
			if(		
					(i+10+DataLen)  > RecLen							||
			 		(RecBuf[i+10+DataLen] 	!= FRAM_END0)	||
			 	 	(RecBuf[i+11+DataLen] 	!= FRAM_END1)
			 )
			 {	
			 //i = i + 10 + DataLen;											//地址不对或帧尾错误，跳过此帧
			 	continue;
			 }
			SumCheck8 = GetCheckSumNR((unsigned char *)&RecBuf[i+2],5);		   				//索引区校验
			
			if(	SumCheck8 != 	RecBuf[i+7] )
			{	
				continue;
			}
			 
			DataLen = RecBuf[i+6];
						
			RecCRC16 = ((unsigned short)RecBuf[i+8+DataLen]) *256 + RecBuf[i+9+DataLen];	//校验	

			switch(RecBuf[i+5]>>4)													//校验和算法判断
			{
					case	RS485_CHK_SUM:
									//Check16 = GetCheckSum16(&RecBuf[i+2],6+DataLen);
									////PrintfCOM0(("\r\n 16位校验和！");
									break;
					case	RS485_CHK_CRC:
									Check16 = GetCrc16Check(&RecBuf[i+2],6+DataLen);
									////PrintfCOM0(("\r\n CRC校验！");
									break;
					case	RS485_CHK_RESUM:
									break;
			}
			
			if(			RecCRC16  ==	Check16 	 )
			{
				sprotocolpara->sourceaddr 	    = RecBuf[i+2];	
				sprotocolpara->destaddr 		= RecBuf[i+3];	
				sprotocolpara->framnum   		= RecBuf[i+4];
				sprotocolpara->framcode  		= RecBuf[i+5]&0x0f;
				sprotocolpara->datalen     	= DataLen; 
				
				memcpy(sprotocolpara->databuf,&RecBuf[i+8],DataLen);				//数据拷贝
				 
				return	1;
			}
		}
	}
  
    return	0;
}

unsigned char   l_recslaveaddr = 0;

uint8	l_recframnum = 0;
uint8	GetRecFramNum(void)
{
	return 	l_recframnum;
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


unsigned char GetRecSlaveAddr(void)
{
	return	l_recslaveaddr;
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


//----------------------------------------------------------------------------
// ??    3?￡o   
// 1|    ?ü￡o   ?ó
// è??ú2?êy￡o   ?T
// 3??ú2?êy￡o   ?T
//----------------------------------------------------------------------------
unsigned char  CSNR_GetData(unsigned char	*RecBuf,unsigned char RecLen,unsigned char	*DataBuf,unsigned char	*InfoLen)
{
    unsigned short		i,j,k;
    unsigned char		SumCheck8;
    unsigned char		Tmp;
    unsigned short		RecCRC16,Check16;
	
	unsigned char 		SourceAddr;	
	unsigned char		DataLen;

  
	for(j = 0;j < RecLen ;j++ )														//è￥???′í¨??ê××??ú
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
	
    for(i = 0; i < RecLen - 10;i++)												//?-?·±éàú
    {
		if(		((RecBuf[i+0]) == FRAM_HERD0) 
			&& 	((RecBuf[i+1]) == FRAM_HERD1)		
		  )																		//±¨í·
		{
			DataLen = RecBuf[i+6];
			//PrintfCOM0(("\r\n DataLen %d,%x,%x",DataLen,RecBuf[i+10+DataLen],RecBuf[i+11+DataLen]);
			if(		
					//	(RecBuf[i+2] != DeviceID) 				||
					  //	(RecBuf[i+3] != HOST_ID) 				||
						(i+10+DataLen)  > RecLen							|| 
				 		(RecBuf[i+10+DataLen] 	!= FRAM_END0)	||
				 	 	(RecBuf[i+11+DataLen] 	!= FRAM_END1)
			 )
			 {	
			 	//i = i + 10 + DataLen;											//μ??·2????ò???2′í?ó￡?ì?1y′???
				  printfcom1("\r\n		(i+10+DataLen)  > RecLen	 %d,%d",i+10+DataLen,RecLen);	
				 continue;
			 }

			SumCheck8 = GetCheckSumNR((unsigned char *)&RecBuf[i+2],5);		   				//?÷òy??D￡?é
			 
			 if(	SumCheck8 != 	RecBuf[i+7] )
			 {
					
				 printfcom1("\r\n sunche8  ");
				 continue;
			 }

			SourceAddr = RecBuf[i+2];	
			l_recslaveaddr = RecBuf[i+3];	
			l_recframnum   = RecBuf[i+4];
			//PrintfCOM0(("\r\n o? %x",RecBuf[i+4]);				
			DataLen = RecBuf[i+6];
			*InfoLen = DataLen; 
			
						
			RecCRC16 = ((unsigned short)RecBuf[i+8+DataLen]) *256 + RecBuf[i+9+DataLen];	//D￡?é	

			switch(RecBuf[i+5]>>4)													//D￡?éoí??·¨?D??
			{
					case	RS485_CHK_SUM:
									//Check16 = GetCheckSum16(&RecBuf[i+2],6+DataLen);
									////PrintfCOM0(("\r\n 16??D￡?éoí￡?");
									break;
					case	RS485_CHK_CRC:
									Check16 = GetCrc16Check(&RecBuf[i+2],6+DataLen);
									////PrintfCOM0(("\r\n CRCD￡?é￡?");
									break;
					case	RS485_CHK_RESUM:
									break;
			}
			
			if(		RecCRC16  ==	Check16 	  )
			{
				 memcpy(DataBuf,&RecBuf[i+8],DataLen);				//êy?Y??±′
				 
				 //return	1;				//SourceAddr
				 return		SourceAddr;
			}
		}
	}
  
    return	0;
}


