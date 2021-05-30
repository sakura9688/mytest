#ifndef _STORE_H
#define _STORE_H
#include "sys.h"
#include "rc522.h"
#include "stdio.h"
#include "string.h"

extern unsigned char CT[2];//卡类型
extern unsigned char SN[4]; //卡号
extern unsigned char RFID[16];			//存放RFID 
extern unsigned char RFIDUSER[16];			//存放RFID 
extern unsigned char KEYA_1B[6];   //1B地址扇区秘钥A
extern unsigned char KEYB_1B[6];   //1B地址扇区秘钥B
extern unsigned char MONEY[4]; //充值或扣款金额
extern unsigned char IsManager;
extern unsigned char USERKEYINITA[6];
extern unsigned char USERKEYINITB[6]; //用户卡秘钥B (管理员权限)
extern unsigned char CARDKEYINIT[6];
extern unsigned char WalletInitData[16]; //钱包数据初始化
extern unsigned char RFID1[16];
extern u8 defalut[6];
extern u32 prevMoney,laterMoney;
extern u16 Recharge;  //充值金额
extern u16 Consume;    // 扣款金额
extern u8 addrUserCon;
extern u8 addrUserDat;  //用户卡存储钱包的块地址，这里使用0x08地址块，非管理员不可增、改
extern u8 addrAdmin; //管理员验证身份信息使用的扇区0x18~0x1B  数据块由秘钥B认证后只能读不能写
extern u8 Mode;    //0：管理员模式 1;用户模式
u8 store(u8 addr,u8* pwd,u8 mode);   //验证管理员用户
u8 ReadCard(u8 addr,u8* pwd,u8 mode);  //读卡
u8 userRecharge(u8 addr,u8* pwd,u16 money,u8 mode);//充值
u8 userConsume(u8 addr,u8* pwd,u16 money,u8 mode) ;  //扣款
u8 MoneyChangeToArr(u16 PMoney,u8* moneyArr);  //数值转换，输入金额十进制，转换到数组中
u32 displayMoney(u8 addr,u8* pwd,u8 mode);
u8 changeUserPwd(u8 dataAddr,u8 controlAddr,u8* pwd,u8* oldpwd,u8* newpwd,u8 mode);
u8 wallet_Init(u8 addr,u8* pwd);
#endif


