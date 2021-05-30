#include "store.h"
#include "DwinDisplay.h"
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOS使用
#include "task.h"  
#endif
unsigned char IsManager=0;
unsigned char CT[2];//卡类型
unsigned char SN[4]; //卡号
unsigned char RFID[16];			//存放RFID 
unsigned char RFIDUSER[16];			//存放RFID 
unsigned char KEYA_1B[6]={8,8,8,8,8,8};   //管理员卡秘钥A (管理员权限)
unsigned char KEYB_1B[6]={6,5,4,3,2,1};   //管理员卡秘钥B (管理员权限)
unsigned char USERKEYINITA[6]={1,2,3,4,5,6}; //用户卡秘钥A（初始密码,面向用户权限）
unsigned char USERKEYINITB[6]={9,6,0,9,1,7}; //用户卡秘钥B (管理员权限)
unsigned char CARDKEYINIT[6]={0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char RFID1[16]={8,8,8,8,8,8,0xff,0x07,0x80,0x69,6,5,4,3,2,1};
unsigned char WalletInitCon[16]={1,2,3,4,5,6,0x80,0xf7,0x87,0x69,9,6,0,9,1,7};  //初始化钱包控制块
unsigned char RFID3[16]={0xFF,0xBB,0xE0,0xA8,0xC9,0x56,0xFE,0xAC,0xC4,0xD2,0xB6,0x7B,0x55,0x6B,0x3F,0x19};//////////
unsigned char RFID4[16];  //保存用户更改密码的数据缓冲区
unsigned char WalletInitData[16]={0x0,0x0,0x0,0x0,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x28,0xD7,0x28,0xD7};//初始化钱包数据块
unsigned char MONEY[4]={0x10,0x00,0x00,0x00};  //充值扣款金额

u8 defalut[6]={12,34,56,79,79,79};

u16 Recharge=500;  //充值金额
u16 Consume=30;    // 扣款金额
u8 addrUserCon=0x0f;  //用户控制块地址
u8 addrUserDat=0x0C;  //用户卡存储钱包的块地址，这里使用0x0C地址块，非管理员不可增、改
u8 addrAdmin=0x18; //管理员验证身份信息使用的扇区0x18~0x1B  数据块由秘钥B认证后只能读不能写
u32 prevMoney=0,laterMoney=0;  //用于消费充值金额时校验，先前金额和消费充值金额的差值
u8 Mode=1; //默认为用户模式

/*************************************************************
funcname:ReadCard
parameter:	u8 addr:扇区地址，扇区内任意地址皆可
			u8* pwd:扇区密码A或B，必须和模式匹配（这里的秘钥B作为管理员模式）
			u8 mode:	1:用户模式  0:管理员模式
return:  MI_OK ：正确读卡     MI_ERR：读卡错误
function:读卡函数，步骤：寻卡-》防冲撞-》选卡-》秘钥验证
Author:Mr.Wang
Date:2018-5-12 11:06:21
**************************************************************/

u8 ReadCard(u8 addr,u8* pwd,u8 mode)
{
	u8 status=MI_ERR;
	u16 RETRY=0x10;
	u8 i;
	PcdAntennaOn(); //开启天线
	do
	{
		RETRY--;
		status=PcdRequest(PICC_REQALL,CT); 	
	}while(RETRY!=0&&status!=MI_OK); //等待寻卡成功
	if(RETRY!=0&&status==MI_OK)  //寻卡成功
	{
		status=MI_ERR;
		status = PcdAnticoll(SN);/*防冲撞*/
		printf("卡序列号：");	//超级终端显示,
		for(i=0;i<4;i++)
		{
			printf("%X\t",SN[i]);
			
		}
		printf("\r\n");
		if (status==MI_OK)//防冲撞成功
		{
			status=MI_ERR;
			status=PcdSelect(SN); /*选卡*/
		}
		if(status==MI_OK)//选卡成功
		{
			 status=MI_ERR;
			 if(mode==1)
			 {
				 printf("验证秘钥A");
				 status =PcdAuthState(0x60,addr,pwd,SN); /*验证秘钥A*/
			 }
			 else
			 {
				 status =PcdAuthState(0x61,addr,pwd,SN); /*验证秘钥B*/
			 }
		}
		if(status==MI_OK)//验证秘钥成功
		{
			printf("验证秘钥成功");
			PcdRead(addr,RFID);
			printf("地址里面的内容：");	//超级终端显示,
			for(i=0;i<16;i++)
			{
				printf("%X ",RFID[i]);
				
			}
			printf("\r\n");
			status=MI_OK;
		}	
	}
	else
	{
		printf("检测超时，未检测到消费卡！！！\r\n");
//		Dwin_ChangePage(Page0);
		PcdAntennaOff();  //关闭天线
	}
	return status;
}
/*************************************************************
funcname:store
parameter:	u8 addr：管理员身份认证块地址，已经设置好为0x18,可以自己更改，注意权限！
			u8* pwd:输入管理员密码
			u8 mode:	1:用户模式  0:管理员模式
return: 状态：MI_OK  MI_ERR
function:验证管理员用户，非管理员用户不能进行充值操作
Author:Mr.Wang
Date:2018-5-10 16:51:04
**************************************************************/

u8 store(u8 addr,u8* pwd,u8 mode)
{
	u8 status=MI_ERR;
	printf("请放入管理员卡\r\n");
	status=ReadCard(addr,pwd,mode);
	if(status==MI_OK) //读卡成功
	{
		printf("读卡成功\r\n");
		if(RFID[0]==0xff&&RFID[1]==0xbb&&RFID[14]==0x3f&&RFID[15]==0x19) //判断卡内容是否是管理员
		{
			IsManager=1;
			return MI_OK;
		}
	}
	return MI_ERR;
}
/*************************************************************
funcname:userRecharge
parameter:	u8 addr：用户钱包块地址，已经设置好为0x0C,
			可以自己更改，注意使用M1卡钱包必须按照一定格式！README文件中有格式说明
			u8* pwd:输入管理员密码
			u16 money：输入充值金额
			u8 mode:	1:用户模式  0:管理员模式
return: 状态：MI_OK  MI_ERR
function:充值钱包
Author:Mr.Wang
Date：2018-5-12 11:11:51
**************************************************************/
u8  userRecharge(u8 addr,u8* pwd,u16 money,u8 mode)
{
	u8 status=MI_ERR;
	printf("放入要充值的用户卡\r\n");
	if(money>50000)  return MI_ERR;
	status=ReadCard(addr,pwd,mode);
	if(status==MI_OK)
	{
		status=MI_ERR;
		prevMoney=displayMoney(addr,pwd,mode);
		status=MoneyChangeToArr(money,MONEY);
	}
	if(status==MI_OK&&IsManager==1)
	{
		status=PcdValue(PICC_INCREMENT,addr,MONEY);
	}
	if(status==MI_OK)
	{
		laterMoney=displayMoney(addr,pwd,mode);
		if((laterMoney-prevMoney)!=money)
		{
			status=MI_ERR;
		}
	}
	return status;
}
/*************************************************************
funcname:userConsume
parameter:	u8 addr：用户钱包块地址，已经设置好为0x0C,
			可以自己更改，注意使用M1卡钱包必须按照一定格式！README文件中有格式说明
			u8* pwd:输入管理员密码
			u16 money：输入充值金额
			u8 mode:	1:用户模式  0:管理员模式
return: 状态：MI_OK  MI_ERR
function:充值钱包
Author:Mr.Wang
Date：2018-5-12 11:11:51
**************************************************************/
u8 userConsume(u8 addr,u8* pwd,u16 money,u8 mode)  //这里把消费和充值函数分开写
{
	u8 status=MI_ERR;
	status=ReadCard(addr,pwd,mode);
	if(status==MI_OK)
	{
		status=MI_ERR;
		prevMoney=displayMoney(addr,pwd,mode);
		status=MoneyChangeToArr(money,MONEY);
	}
	if(status==MI_OK&&IsManager==1)
	{
		status=PcdValue(PICC_DECREMENT,addr,MONEY);
	}
	if(status==MI_OK)
	{
		laterMoney=displayMoney(addr,pwd,mode);
		if((prevMoney-laterMoney)!=money)
		{
			status=MI_ERR;
		}
	}
	return status;
}
/*************************************************************
funcname:MoneyChangeToArr
parameter:	u16 PMoney:充值或消费金额
			u8* moneyArr：在钱包地址块中有一定格式，需要按照钱包格式写金额数组
return:状态：MI_OK  MI_ERR
function:将十进制转换成16进制的数组
Author:Mr.Wang
Date:2018-5-12 11:14:37
**************************************************************/

u8 MoneyChangeToArr(u16 PMoney,u8* moneyArr)
{
	u8 i=0,temp;
	do
	{
		moneyArr[i]=PMoney%16;
		PMoney=PMoney/16;
		i++;
	}while(PMoney!=0); //转换十六进制
	temp=moneyArr[1];
	moneyArr[1]=moneyArr[0];
	moneyArr[0]=temp;  //将得到值进行交换
	temp=moneyArr[3];
	moneyArr[3]=moneyArr[2];
	moneyArr[2]=temp;  //将得到值进行交换
	moneyArr[0]=moneyArr[0]<<4|moneyArr[1];
	moneyArr[1]=moneyArr[2]<<4|moneyArr[3];  //钱包数组的小值放在左边，如充值1,{0x01,0x00,0x00,0x00}
	moneyArr[2]=0;
	moneyArr[3]=0;  //设置最多充值50000，所以后面两位置0
	return MI_OK;
}
/*************************************************************
funcname:displayMoney
parameter:	u8 addr：用户钱包块地址，已经设置好为0x0C,
			可以自己更改，注意使用M1卡钱包必须按照一定格式！README文件中有格式说明
			u8* pwd:输入管理员密码或用户密码
			u8 mode:	1:用户模式  0:管理员模式
return:32位十进制的金额数
function:显示余额
Author:Mr.Wang
Date:2018-5-12 11:19:46
**************************************************************/
u32 displayMoney(u8 addr,u8* pwd,u8 mode)
{
	u8 status=MI_ERR;
	u32 moneySum=0;
	status=ReadCard(addr,pwd,mode);
	if(status==MI_OK)
	{
		status=MI_ERR;
		status=PcdRead(addr,RFID);
	}
	if(status==MI_OK)
	{
		moneySum=RFID[0]+RFID[1]*256u+RFID[2]*65536u+RFID[3]*4294967295u;
	}
	return moneySum;
}
/*************************************************************
funcname:changeUserPwd
parameter:	u8 dataAddr：用户钱包数据地址
			u8 controlAddr;用户钱包的控制地址
			u8* pwd：用户钱包的秘钥B，以上三个无需用户操作，也不能操作
			u8* oldpwd：用户旧密码
			u8* newpwd：用户新密码
			u8 mode：用户模式模式或管理员模式（此函数只能管理员模式）
return:	状态：MI_OK  MI_ERR
function:更改用户密码，用户需要输入旧密码匹配成功后可更改新密码
Author:Mr.Wang
Date:2018-5-12 11:23:57
**************************************************************/

u8 changeUserPwd(u8 dataAddr,u8 controlAddr,u8* pwd,u8* oldpwd,u8* newpwd,u8 mode)
{
	u8 status=MI_ERR,i=0;
	status=ReadCard(dataAddr,pwd,mode);
	if(status==MI_OK)
	{
		status=MI_ERR;
		/*确认与秘钥A相同*/
		
		/*更改新秘钥A，也就是用户密码*/
		
		/*执行更改密码操作*/
		status=PcdRead(controlAddr,RFID4);
	}
	if(status==MI_OK)
	{
		for(i=0;i<6;i++)
		{
			RFID4[i]=newpwd[i];
		}
		status=MI_ERR;
		status=PcdWrite(controlAddr,RFID4);   //更改密码
	}
	return status;
}

/*************************************************************
funcname:wallet_Init
parameter:	u8 addr:选择用户钱包数据地址块
			u8* pwd:扇区密码A或B，必须和模式匹配（这里的秘钥B作为管理员模式）
			u8 mode:	1:用户模式  0:管理员模式
return:  MI_OK ：正确读卡     MI_ERR：读卡错误
function:钱包初始化，此函数使用一次即可！！初始化后的钱包金额为0
Author:Mr.Wang
Date:2018-5-12 11:28:58
**************************************************************/
u8 wallet_Init(u8 addr,u8* pwd)
{
	u8 status=MI_ERR,i=0;
	status=ReadCard(addr,pwd,0);
	if(status==MI_OK)
	{
		printf("读卡成功\r\n");
		status=MI_ERR;
		status=PcdWrite(addr,WalletInitData);   //写钱包数据格式初始化
	}
	if(status==MI_OK)
	{
		printf("写入钱包数据成功\r\n");
		status=MI_ERR;
		status=PcdRead(addr,RFID);
	}
	if(status==MI_OK)
	{
		for(i=0;i<16;i++)
		{
			printf("%x ",RFID[i]);
		}
		printf("\r\n");
		status=MI_ERR;
		status=PcdWrite(addrUserCon,WalletInitCon); //写钱包控制，秘钥AB不可读，秘钥B可改写控制
	}
	if(status==MI_OK)
	{
		printf("写入控制存储成功\r\n");
		status=MI_ERR;
		status=PcdRead(addrUserCon,RFID);
	}
	if(status==MI_OK)
	{
		for(i=0;i<16;i++)
		{
			printf("%x ",RFID[i]);
		}
		printf("\r\n");
	}
	return status;
}

