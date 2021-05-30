#ifndef _STORE_H
#define _STORE_H
#include "sys.h"
#include "rc522.h"
#include "stdio.h"
#include "string.h"

extern unsigned char CT[2];//������
extern unsigned char SN[4]; //����
extern unsigned char RFID[16];			//���RFID 
extern unsigned char RFIDUSER[16];			//���RFID 
extern unsigned char KEYA_1B[6];   //1B��ַ������ԿA
extern unsigned char KEYB_1B[6];   //1B��ַ������ԿB
extern unsigned char MONEY[4]; //��ֵ��ۿ���
extern unsigned char IsManager;
extern unsigned char USERKEYINITA[6];
extern unsigned char USERKEYINITB[6]; //�û�����ԿB (����ԱȨ��)
extern unsigned char CARDKEYINIT[6];
extern unsigned char WalletInitData[16]; //Ǯ�����ݳ�ʼ��
extern unsigned char RFID1[16];
extern u8 defalut[6];
extern u32 prevMoney,laterMoney;
extern u16 Recharge;  //��ֵ���
extern u16 Consume;    // �ۿ���
extern u8 addrUserCon;
extern u8 addrUserDat;  //�û����洢Ǯ���Ŀ��ַ������ʹ��0x08��ַ�飬�ǹ���Ա����������
extern u8 addrAdmin; //����Ա��֤�����Ϣʹ�õ�����0x18~0x1B  ���ݿ�����ԿB��֤��ֻ�ܶ�����д
extern u8 Mode;    //0������Աģʽ 1;�û�ģʽ
u8 store(u8 addr,u8* pwd,u8 mode);   //��֤����Ա�û�
u8 ReadCard(u8 addr,u8* pwd,u8 mode);  //����
u8 userRecharge(u8 addr,u8* pwd,u16 money,u8 mode);//��ֵ
u8 userConsume(u8 addr,u8* pwd,u16 money,u8 mode) ;  //�ۿ�
u8 MoneyChangeToArr(u16 PMoney,u8* moneyArr);  //��ֵת����������ʮ���ƣ�ת����������
u32 displayMoney(u8 addr,u8* pwd,u8 mode);
u8 changeUserPwd(u8 dataAddr,u8 controlAddr,u8* pwd,u8* oldpwd,u8* newpwd,u8 mode);
u8 wallet_Init(u8 addr,u8* pwd);
#endif


