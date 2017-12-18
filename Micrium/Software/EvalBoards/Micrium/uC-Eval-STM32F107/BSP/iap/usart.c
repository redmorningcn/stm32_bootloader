#include "usart.h"	
#include "stdarg.h"	

////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  

#endif

#include "stm32f10x_usart.h"            //ucos ʹ��	  
#include "stm32f10x_gpio.h"            //ucos ʹ��	  


//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F4̽���߿�����
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/6/10
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
////////////////////////////////////////////////////////////////////////////////// 	  
 
void printfcom1(const char *format, ...);
//
////////////////////////////////////////////////////////////////////
////�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
//#if 1
//#pragma import(__use_no_semihosting)             
////��׼����Ҫ��֧�ֺ���                 
//struct __FILE 
//{ 
//	int handle; 
//}; 
//
//FILE __stdout;       
////����_sys_exit()�Ա���ʹ�ð�����ģʽ    
//void _sys_exit(int x) 
//{ 
//	x = x; 
//} 
////�ض���fputc���� 
//int fputc(int ch, FILE *f)
//{ 	
//	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
//	USART1->DR = (u8) ch;      
//	return ch;
//}
//#endif
 
#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	

#define EnableUartSend()        GPIO_SetBits(GPIOE,GPIO_Pin_15)
#define EnableUartRec()         GPIO_ResetBits(GPIOE,GPIO_Pin_15)
unsigned int GetSysBaseTick(void);


#define MBREN3_GPIO_PIN        GPIO_Pin_15             /* PE.15 */
#define MBREN3_GPIO_PORT       GPIOE
#define MBREN3_GPIO_RCC        RCC_APB2Periph_GPIOE     

void EnUartContrlIO_Init(void)
{    	 
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(MBREN3_GPIO_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = MBREN3_GPIO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(MBREN3_GPIO_PORT, &GPIO_InitStructure);

    GPIO_ResetBits(MBREN3_GPIO_PORT, MBREN3_GPIO_PIN);  // �͵�ƽ����ʹ��  
}   

typedef struct  _stcmemcontrol 
{
    u8      *pbuf;      //mem�ռ�ָ��
    u16     buflen;     
    u16     pwrite;     //дαָ��
    u16     pread;      //��αָ��
    u16     pwriteflg;   //����ʶʱ��дָ��λ�á�
    u16     flg;        //��ǣ�b[0] = 1��ǣ� �[
}stcmemcontrol;

u8      uartbuf[1024];
stcmemcontrol   g_uart;
extern  uint64_t g_systime;

void    initmemcontrol(stcmemcontrol *memcontrl, u8 *buf,u32 len)
{
    memcontrl->flg      = 0;
    memcontrl->pbuf     = buf;
    memcontrl->pread    = 0;
    memcontrl->pwrite   = 0;
    memcontrl->pwriteflg=0;
    memcontrl->buflen   = len;
}
void USART3_IRQHandler_noos(void);                	//����3�жϷ������

//��ʼ��IO ����1 
//bound:������
void uart_init(u32 baud){
   //GPIO�˿�����
    
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
   
    INT16U            BSP_INT_ID_USARTx;
    USART_TypeDef*    USARTx;
    CPU_FNCT_VOID     USARTx_RxTxISRHandler;
    
    USARTx                    = USART3;
    BSP_INT_ID_USARTx         = BSP_INT_ID_USART3;
    
    //USARTx_RxTxISRHandler     = USART3_IRQHandler;
    USARTx_RxTxISRHandler     = USART3_IRQHandler_noos;
    
    /* Enable USART3 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* Configure USART1 Rx (PB.11) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* Configure USART1 Tx (PB.10) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /***********************************************
    * ������ configuration
    */
    USART_InitStructure.USART_BaudRate              = baud;
    USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits              = USART_StopBits_1;
    USART_InitStructure.USART_Parity                = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;
    
    /***********************************************
    * ������ 
    */
    USART_DeInit(USARTx);
    USART_Init(USARTx, &USART_InitStructure);
    
    /***********************************************
    * ������ 
    */
    //if ( port_nbr != 3 )
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
    //USART_ITConfig(USARTx, USART_IT_IDLE, ENABLE);
    
    /***********************************************
    * ������ 
    */
    USART_ClearFlag(USARTx,USART_FLAG_TXE);
    USART_ClearFlag(USARTx,USART_FLAG_RXNE);
    //USART_ClearFlag(USARTx,USART_FLAG_IDLE);      
    
    /***********************************************
    * ������ 
    */
    USART_Cmd(USARTx, ENABLE);
    
    /***********************************************
    * ������ 
    */
    BSP_IntVectSet(BSP_INT_ID_USARTx, USARTx_RxTxISRHandler);
    BSP_IntEn(BSP_INT_ID_USARTx);
	
    
    EnUartContrlIO_Init();
    initmemcontrol((stcmemcontrol  *)&g_uart,uartbuf,sizeof(uartbuf));           //3?��??����??��?����??o��?
}


uint64_t l_uartrectime = 0;                 //���ڽ���ʱ��

#define REC_FLG     1                       /*���ձ�ʶ*/

#define END_FLG     0                       /*������ʶ*/


void USART3_IRQHandler_noos(void)                	//����3�жϷ������
{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
    
    l_uartrectime =  GetSysBaseTick();

    g_uart.flg |= (0x01<< REC_FLG);
    
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)   //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
        Res =USART_ReceiveData(USART3);//(USART1->DR);	    //��ȡ���յ�������
        //printfcom1("%c",Res);
        uartbuf[ g_uart.pwrite ] =  Res;         //���ݱ���

        g_uart.pwrite = (g_uart.pwrite + 1)%g_uart.buflen;
        
    } 
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
#endif	


void    HoldUartRecSta(void)                                        //ά������״̬
{
    static  uint64_t systime;
    systime = GetSysBaseTick();
    //if(systime >  (l_uartrectime + (84000000/9)/100))         //10ms�����ݣ���ΪͨѶ����
    if(systime >  (l_uartrectime + 10))         //10ms�����ݣ���ΪͨѶ����
    {
        if(     (  (g_uart.flg &(0x01 << END_FLG)) == 0 &&  (g_uart.flg &(0x01 << REC_FLG)))     //��ʱ
            ||  (  (g_uart.pwrite < g_uart.pread)       &&  (g_uart.pwrite+10 == g_uart.pread))  //д��  ��10�ռ� 
          )   
        {
            g_uart.flg   |= (0x01 << END_FLG);           //���ݽ��ս���
            g_uart.flg   &= ~(0x01 << REC_FLG);          //�����ݽ���

            g_uart.pwriteflg = g_uart.pwrite; //��ʶ������ʱ��ʶ
            //printfcom1("\r\n pwrite %d",g_uart.pwrite);
        }
    }
}    
//
//uint8	GetFramRecEndFlg(void)
//{
////	return GetCOM1EndFlg(); 
//    return (g_uart.flg   & (0x01 << END_FLG)); 
//}
//
////???����?����3������??  ??��e
//void	ClearFramRecEndFlg(void)
//{
////	ClearCOM1EndFlg();  
//    g_uart.flg   &= ~(0x01 << END_FLG);          //�����ݽ���
//}



u32 ReadUartBuf(u8 *buf,u32 len)        //�Ӵ��ڻ�����������
{
    if((g_uart.flg & (0x01<< END_FLG)) == 0)   //�޽�����ʶ���˳�
        return 0;
    
    g_uart.flg &= ~(0x01<< END_FLG);           //�������ʶ
    
    u32 noreadlen  = (g_uart.pwriteflg + g_uart.buflen - g_uart.pread) % g_uart.buflen; //������ܻ�����������Ϊ������
    
    u32 readlen    = (noreadlen>len)?len:noreadlen;    //ȡ��������

    for(u32 i=0;i<readlen;i++)
        buf[i] = g_uart.pbuf[(g_uart.pread + i)%g_uart.buflen];
    
    g_uart.pread = (g_uart.pread + readlen)%g_uart.buflen;
        
    //printfcom1("\r\n readlen %d",readlen);
    
    return  readlen;                            //���ض�ȡ�����ݳ���
}
 
u32 SendDataCom1(u8 *buf,u32 len)
{
    EnableUartSend();       //IO�˿ڱ�Ϊ���Ͷ˿�

    for(u32 i = 0;i < len;i++)
    {
        uint32  times = 10000;
        USART_SendData(USART3,buf[i]);
        
        while(  USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET
            &&  times-- );  //�����˳�����
    }
    
    EnableUartRec();        //IO�˿ڱ�Ϊ���ܶ˿�

    return  len;
}

void printfcom1(const char *format, ...) 
{ 
   va_list  argptr;
//   uint32   cnt;
   char     a[255];
   
   strlen(format);
   if(strlen(format) > (sizeof(a) - 55))
   {
//        return FALSE;
		return	;
   }
   va_start(argptr, format);
   
//   cnt = vsprintf(a, format, argptr);
   vsprintf(a, format, argptr);
   
   va_end(argptr);
   
   SendDataCom1((u8 *)a,strlen(a));
//   return(cnt);
}

void    TestUart(void)
{
    printfcom1("\r\nstart\r\n");
    while(1)
    {
        u32 len = 5;
        u8 buf[256]={0,1,2,3,4,5};

        HoldUartRecSta();
        
        len = ReadUartBuf(buf,sizeof(buf));
        if( len )
            SendDataCom1(buf,len);
        
    }
}
                

uint8	GetFramRecEndFlgIAP(void)
{
//	return GetCOM1EndFlg(); 
    return ((GetCOM2EndFlg())|(GetCOM3EndFlg())); 
}

//???����?����3������??  ??��e
void	ClearFramRecEndFlgIAP(void)
{
//	ClearCOM1EndFlg();  
		ClearCOM2EndFlg();  
		ClearCOM3EndFlg();  
}

uint16	ReadRs485DataIAP(uint8 *Buf)
{
	uint16	len = 0;
	 
	if(GetCOM2EndFlg())
	{
		len = ReadCOM2(Buf,256); 
		return	len; 
	}
	
	if(GetCOM3EndFlg())
	{
		len = ReadCOM3(Buf,256);
		return	len;  
	}	
}


