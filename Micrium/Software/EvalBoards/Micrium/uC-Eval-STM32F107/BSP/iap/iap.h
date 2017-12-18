/*******************************************************************************
 *   Filename:       iap.h
 *   Revised:        $Date: 2014-04-23
 *   Revision:       $
 *	 Writer:		 Wuming Shen.
 *
 *   Description:    
 *
 *   Notes:        
 *					   
 *			
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/

#ifndef __IAP_H__
#define __IAP_H__
/*******************************************************************************
 * INCLUDES
 */
 //#include "stm32f10x.h"

/*******************************************************************************
 * CONSTANTS
 */
#define KB						*1024

#define	FLASH_SIZE				(256KB)											// FLASH��С
#define	FLASH_SIZE_BOOT			(80KB)											// BOOT����С
#define	FLASH_SIZE_EEP			(16KB)											// ��������С

#define	FLASH_SIZE_A			(FLASH_SIZE-FLASH_SIZE_BOOT-FLASH_SIZE_EEP)/2	// ����A��С
#define	FLASH_SIZE_B			(FLASH_SIZE-FLASH_SIZE_BOOT-FLASH_SIZE_EEP)/2	// ����B��С

#define FLASH_BOOT_ADDR			0x8000000  								        // ����������ʼ��ַ(�����FLASH)
#define FLASH_APP_ADDR_A		(0x8000000 + FLASH_SIZE_BOOT)					// ��һ��Ӧ�ó�����ʼ��ַ(�����FLASH)
#define FLASH_APP_ADDR_B		(0x8000000 + FLASH_SIZE_BOOT + FLASH_SIZE_A)  	// �ڶ���Ӧ�ó�����ʼ��ַ(�����FLASH)
#define FLASH_APP_ADDR			FLASH_BOOT_ADDR  								// Ӧ�ó�����ʼ��ַ(�����FLASH)

#define FLASH_APP_ADDR_SAVE		(FLASH_SIZE_EEP - 4)  	                    // �����´����еĳ����ַ
#define FLASH_APP_STATUS		(FLASH_SIZE_EEP - 5)  	                    // ��������״̬

#if defined     ( IMAGE_A )
#define IAP_SELF_APP_ADDR      FLASH_APP_ADDR_A 
#elif defined   ( IMAGE_B )
#define IAP_SELF_APP_ADDR      FLASH_APP_ADDR_B 
#else
#define IAP_SELF_APP_ADDR      FLASH_BOOT_ADDR
#endif 
    
     
#define IAP_STS_DEF             0
#define IAP_STS_RST             1
#define IAP_STS_START           2
#define IAP_STS_PROGRAMING      3
#define IAP_STS_FINISH          4
#define IAP_STS_FAILED          5
#define IAP_STS_SUCCEED         6
     
/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
typedef struct {
    INT32U      AppAddr;                            // Ӧ�ó����ַ    INT08U      Status              : 4;            // // 0δ������1��ʼ��2�����У�3ʧ�ܣ�4�ɹ����������޳���
    INT08U      Status;                             // 0δ������1��ʼ��2�����У�3ʧ�ܣ�4�ɹ����������޳���
    INT08U      Step;
    INT16U      FrameIdx;                           // ���յ�����֡���
    INT16U      SysNbr;                             // ͬ�����к�
    INT16U      TimeOut;                            // ��ʱ�˳�IAP
    
    INT32U      FileSize;                           // �ļ���С
    INT32U      WrittenSize;                        // ��д���ļ���С
    INT32U      SectorAddr;                         // ��ǰд���������ַ
    INT32U      SectorAddrLast;                     // �ϴ�д���������ַ
    INT32U      Sectors;                            // ��������
    INT08U      WriteCtr;                           // 16֡128�ֽ����һ��2K��������
    INT16U     *pBuf;                               // ���ݻ�����ָ��
    
} StrIapState;

extern StrIapState Iap;

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

 /*******************************************************************************
 * EXTERN FUNCTIONS
 */
/***********************************************
 * ������
 */
void        IAP_DevDeInit       (void);
u8          IAP_GetAppAStatus   (void);
u8          IAP_GetBppAStatus   (void);
u32     	IAP_GetAppAddr  	(void);
u8    	    IAP_SetAppAddr  	(u32 appAddr);
u8      	IAP_JumpToApp   	(u32 flag);
u8      	IAP_JumpToAppA   	(void);
u8      	IAP_JumpToAppB   	(void);
u8          IAP_JumpToAddr      (u32 appAddr);

u8          IAP_GetStatus       (void);
void        IAP_SetStatus       (u8);

void        IAP_Reset           (void);
void        IAP_Restart         (void);
void        IAP_Finish          (void);
void        IAP_Exit            (u8 timeout);
void        IAP_Programing      (void);
BOOL        IAP_FileInfoInit    (void);
BOOL        IAP_Program        (StrIapState *iap, INT16U *buf, INT16U len, INT16U idx );
/*******************************************************************************
 * 				end of file
 *******************************************************************************/ 
#endif