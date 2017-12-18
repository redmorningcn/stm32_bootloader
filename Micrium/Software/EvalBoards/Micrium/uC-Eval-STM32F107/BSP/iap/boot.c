#include <includes.h>
//#include  <stm32f10x_nvic.h>


#define	USER_APP_START_ADDR		(STM32_FLASH_BASE + 0x00010000	)


typedef  void (*pFunction)(void);			    //����һ���������͵Ĳ���.
pFunction   pApp;
//void __set_CONTROL(uint32_t control);


/*******************************************************************************
 * ��    �ƣ� IAP_JumpTo()
 * ��    �ܣ� ��ת��Ӧ�ó����
 * ��ڲ�����
 * ���ڲ����� ��
 * ��    �ߣ� ������
 * �������ڣ� 2014-04-23
 * ��    �ģ� 
 * �޸����ڣ� 
 *******************************************************************************/
void JMP_Boot(u32 appAddr)
{    
    u32     JumpAddress = 0;
    u8      cpu_sr;
        
    /***********************************************
    * ������ ��������ַ
    */
    //IAP_SetAppAddr(appAddr);
    /***********************************************
    * ������ ���жϣ���ֵֹ���ж��޸�
    */
    CPU_CRITICAL_ENTER();
    /***********************************************
    * ������ ����ָ�Ĭ�ϣ��������Ӧ�ó����Ӱ�������������
    */
    //IAP_DevDeInit();  
    /***********************************************
    * ������ ��ȡӦ����ڼ���ʼ����ջָ��
    */
    SCB->VTOR = USER_APP_START_ADDR & 0x1FFFFF80;	//??��??D???����?
    JumpAddress   =*(volatile u32*) (appAddr + 4); // ��ַ+4ΪPC��ַ
    pApp          = (pFunction)JumpAddress;         // ����ָ��ָ��APP
//    __set_MSP       (*(volatile u32*) appAddr);    // ��ʼ������ջָ�루MSP��
//    __set_PSP       (*(volatile u32*) appAddr);    // ��ʼ�����̶�ջָ�루PSP��
//    __set_CONTROL   (0);                            // ����CONTROL
    /***********************************************
    * ������ ��ת��APP����
    */
    pApp();
    
    CPU_CRITICAL_EXIT();
}
//__asm void JMP_Boot( uint32_t address ){
//   LDR SP, [R0]		;Load new stack pointer address
//   LDR PC, [R0, #4]	;Load new program counter address
//}

#define zyIrqDisable()  CPU_INT_DIS()
#define zyIrqEnable()   CPU_INT_EN()


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

void Jumpto_APP(void)
{
    INT32U IapSpInitVal;

    INT32U IapJumpAddr;

    void (*pIapFun)(void);

    RCC_DeInit();//�ر�����

    NVIC_DeInit();

    //__disable_irq(); //���жϣ�����IAP���ж� APP���û��UCOSϵͳ��APP

    //��ʼ����Ҫ���жϣ���UCOS�����������Ὺ�ж�
    
    
    IapSpInitVal = *(INT32U *)USER_APP_START_ADDR;

    IapJumpAddr = *(INT32U *)(USER_APP_START_ADDR + 4);

    if((IapSpInitVal & 0x2FFE0000)==0x20000000)//���ջ����ַ�Ƿ�Ϸ�.

    {
        __set_MSP (IapSpInitVal);
        //__set_MSP       (*(volatile u32*) appAddr);    // ��ʼ������ջָ�루MSP��
        __set_PSP       (IapSpInitVal);    // ��ʼ�����̶�ջָ�루PSP��
        //__set_CONTROL   (0);    
        //__set_CONTROL   (0);
        
        pIapFun = (void (*)(void))IapJumpAddr;

        (*pIapFun) ();

    }

}

typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.   
iapfun jump2app; 
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

