#ifndef _DWINDISPLAY_H        
#define _DWINDISPLAY_H
#include "usart2.h"
#include "sys.h"  
#include "stdbool.h"
#include "usart.h"

//串口屏协议相关
#define FRAME_START0 		0x5A
#define FRAME_START1 		0xA5
#define FRAME_END0			0xFF
#define FRAME_END1			0xFF
#define CHANGE_PAGECMD 	10
#define ADDRESS_H				4
#define ADDRESS_L				5
enum page
{
	Page0,
	Page1,
	Page2,
	Page3,
	Page4,
	Page5,
	Page6,
	Page7,
	Page8,
	Page9,
	Page10,
	Page11,
	Page12,
	Page13,
	Page14,
	Page15,
	Page16,
	Page17,
	Page18,
	Page19,
	Page20,
};
//===================箱子信息==========================
typedef struct 
{
	u8 Owern;//物主
	u8 GoodsPrice[4];//物品价格
	u8 GoodsName[20];//物品名称
	u8 Description[20];//物品说明
	
}BoxImformate;
//===================顾客信息==========================
typedef struct customer
{
	u8 CardID[4];
	u8 Name[6];
}Customer;

extern u8 buff[40];
extern u8 ChooseMode;
extern u8 CurrentUserID;
extern u8 IdleBOX;//空闲箱子数
extern bool DrawbackStatus;//页面返回标志
extern bool IsSurePublish;//确认发布标志
extern bool CancelPublish;//取消发布标志
extern bool UpdataFlag;//更新标志
extern u8 CurrentPage;//当前界面的编号
extern u8 CurrentBoxNum;//当前箱子的编号
extern bool IsSureToBuy;//确认购买标志
extern BoxImformate *BoxPointer;//定义了用于指向箱子的指针
void DwinInit(void);
void DataProcess(void);
void TxtProcess(void);
void Protocol_cheak(void);
void TxtDisplay(void);
void DeleteImformation(void);
void DeleteStructImformation(void);
void ChangeIDLE_Box(u8 BoxNum,u8 AdressH,u8 AdressL);//修改空闲箱子数量
void ReadMesgAndFill(u8 mode,u8 AdressH,u8 AdressL);
void DWin_DisplayPrint(u8* address);
void DwinDisplayCMD(u8 mode);
void Dwin_ChangePage(u8 page);
u8 CheakID(u8 *IDNumber);
#endif


