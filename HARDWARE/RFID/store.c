#include "store.h"
#include "DwinDisplay.h"
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOSʹ��
#include "task.h"  
#endif
unsigned char IsManager=0;
unsigned char CT[2];//������
unsigned char SN[4]; //����
unsigned char RFID[16];			//���RFID 
unsigned char RFIDUSER[16];			//���RFID 
unsigned char KEYA_1B[6]={8,8,8,8,8,8};   //����Ա����ԿA (����ԱȨ��)
unsigned char KEYB_1B[6]={6,5,4,3,2,1};   //����Ա����ԿB (����ԱȨ��)
unsigned char USERKEYINITA[6]={1,2,3,4,5,6}; //�û�����ԿA����ʼ����,�����û�Ȩ�ޣ�
unsigned char USERKEYINITB[6]={9,6,0,9,1,7}; //�û�����ԿB (����ԱȨ��)
unsigned char CARDKEYINIT[6]={0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char RFID1[16]={8,8,8,8,8,8,0xff,0x07,0x80,0x69,6,5,4,3,2,1};
unsigned char WalletInitCon[16]={1,2,3,4,5,6,0x80,0xf7,0x87,0x69,9,6,0,9,1,7};  //��ʼ��Ǯ�����ƿ�
unsigned char RFID3[16]={0xFF,0xBB,0xE0,0xA8,0xC9,0x56,0xFE,0xAC,0xC4,0xD2,0xB6,0x7B,0x55,0x6B,0x3F,0x19};//////////
unsigned char RFID4[16];  //�����û�������������ݻ�����
unsigned char WalletInitData[16]={0x0,0x0,0x0,0x0,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x28,0xD7,0x28,0xD7};//��ʼ��Ǯ�����ݿ�
unsigned char MONEY[4]={0x10,0x00,0x00,0x00};  //��ֵ�ۿ���

u8 defalut[6]={12,34,56,79,79,79};

u16 Recharge=500;  //��ֵ���
u16 Consume=30;    // �ۿ���
u8 addrUserCon=0x0f;  //�û����ƿ��ַ
u8 addrUserDat=0x0C;  //�û����洢Ǯ���Ŀ��ַ������ʹ��0x0C��ַ�飬�ǹ���Ա����������
u8 addrAdmin=0x18; //����Ա��֤�����Ϣʹ�õ�����0x18~0x1B  ���ݿ�����ԿB��֤��ֻ�ܶ�����д
u32 prevMoney=0,laterMoney=0;  //�������ѳ�ֵ���ʱУ�飬��ǰ�������ѳ�ֵ���Ĳ�ֵ
u8 Mode=1; //Ĭ��Ϊ�û�ģʽ

/*************************************************************
funcname:ReadCard
parameter:	u8 addr:������ַ�������������ַ�Կ�
			u8* pwd:��������A��B�������ģʽƥ�䣨�������ԿB��Ϊ����Աģʽ��
			u8 mode:	1:�û�ģʽ  0:����Աģʽ
return:  MI_OK ����ȷ����     MI_ERR����������
function:�������������裺Ѱ��-������ײ-��ѡ��-����Կ��֤
Author:Mr.Wang
Date:2018-5-12 11:06:21
**************************************************************/

u8 ReadCard(u8 addr,u8* pwd,u8 mode)
{
	u8 status=MI_ERR;
	u16 RETRY=0x10;
	u8 i;
	PcdAntennaOn(); //��������
	do
	{
		RETRY--;
		status=PcdRequest(PICC_REQALL,CT); 	
	}while(RETRY!=0&&status!=MI_OK); //�ȴ�Ѱ���ɹ�
	if(RETRY!=0&&status==MI_OK)  //Ѱ���ɹ�
	{
		status=MI_ERR;
		status = PcdAnticoll(SN);/*����ײ*/
		printf("�����кţ�");	//�����ն���ʾ,
		for(i=0;i<4;i++)
		{
			printf("%X\t",SN[i]);
			
		}
		printf("\r\n");
		if (status==MI_OK)//����ײ�ɹ�
		{
			status=MI_ERR;
			status=PcdSelect(SN); /*ѡ��*/
		}
		if(status==MI_OK)//ѡ���ɹ�
		{
			 status=MI_ERR;
			 if(mode==1)
			 {
				 printf("��֤��ԿA");
				 status =PcdAuthState(0x60,addr,pwd,SN); /*��֤��ԿA*/
			 }
			 else
			 {
				 status =PcdAuthState(0x61,addr,pwd,SN); /*��֤��ԿB*/
			 }
		}
		if(status==MI_OK)//��֤��Կ�ɹ�
		{
			printf("��֤��Կ�ɹ�");
			PcdRead(addr,RFID);
			printf("��ַ��������ݣ�");	//�����ն���ʾ,
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
		printf("��ⳬʱ��δ��⵽���ѿ�������\r\n");
//		Dwin_ChangePage(Page0);
		PcdAntennaOff();  //�ر�����
	}
	return status;
}
/*************************************************************
funcname:store
parameter:	u8 addr������Ա�����֤���ַ���Ѿ����ú�Ϊ0x18,�����Լ����ģ�ע��Ȩ�ޣ�
			u8* pwd:�������Ա����
			u8 mode:	1:�û�ģʽ  0:����Աģʽ
return: ״̬��MI_OK  MI_ERR
function:��֤����Ա�û����ǹ���Ա�û����ܽ��г�ֵ����
Author:Mr.Wang
Date:2018-5-10 16:51:04
**************************************************************/

u8 store(u8 addr,u8* pwd,u8 mode)
{
	u8 status=MI_ERR;
	printf("��������Ա��\r\n");
	status=ReadCard(addr,pwd,mode);
	if(status==MI_OK) //�����ɹ�
	{
		printf("�����ɹ�\r\n");
		if(RFID[0]==0xff&&RFID[1]==0xbb&&RFID[14]==0x3f&&RFID[15]==0x19) //�жϿ������Ƿ��ǹ���Ա
		{
			IsManager=1;
			return MI_OK;
		}
	}
	return MI_ERR;
}
/*************************************************************
funcname:userRecharge
parameter:	u8 addr���û�Ǯ�����ַ���Ѿ����ú�Ϊ0x0C,
			�����Լ����ģ�ע��ʹ��M1��Ǯ�����밴��һ����ʽ��README�ļ����и�ʽ˵��
			u8* pwd:�������Ա����
			u16 money�������ֵ���
			u8 mode:	1:�û�ģʽ  0:����Աģʽ
return: ״̬��MI_OK  MI_ERR
function:��ֵǮ��
Author:Mr.Wang
Date��2018-5-12 11:11:51
**************************************************************/
u8  userRecharge(u8 addr,u8* pwd,u16 money,u8 mode)
{
	u8 status=MI_ERR;
	printf("����Ҫ��ֵ���û���\r\n");
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
parameter:	u8 addr���û�Ǯ�����ַ���Ѿ����ú�Ϊ0x0C,
			�����Լ����ģ�ע��ʹ��M1��Ǯ�����밴��һ����ʽ��README�ļ����и�ʽ˵��
			u8* pwd:�������Ա����
			u16 money�������ֵ���
			u8 mode:	1:�û�ģʽ  0:����Աģʽ
return: ״̬��MI_OK  MI_ERR
function:��ֵǮ��
Author:Mr.Wang
Date��2018-5-12 11:11:51
**************************************************************/
u8 userConsume(u8 addr,u8* pwd,u16 money,u8 mode)  //��������Ѻͳ�ֵ�����ֿ�д
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
parameter:	u16 PMoney:��ֵ�����ѽ��
			u8* moneyArr����Ǯ����ַ������һ����ʽ����Ҫ����Ǯ����ʽд�������
return:״̬��MI_OK  MI_ERR
function:��ʮ����ת����16���Ƶ�����
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
	}while(PMoney!=0); //ת��ʮ������
	temp=moneyArr[1];
	moneyArr[1]=moneyArr[0];
	moneyArr[0]=temp;  //���õ�ֵ���н���
	temp=moneyArr[3];
	moneyArr[3]=moneyArr[2];
	moneyArr[2]=temp;  //���õ�ֵ���н���
	moneyArr[0]=moneyArr[0]<<4|moneyArr[1];
	moneyArr[1]=moneyArr[2]<<4|moneyArr[3];  //Ǯ�������Сֵ������ߣ����ֵ1,{0x01,0x00,0x00,0x00}
	moneyArr[2]=0;
	moneyArr[3]=0;  //��������ֵ50000�����Ժ�����λ��0
	return MI_OK;
}
/*************************************************************
funcname:displayMoney
parameter:	u8 addr���û�Ǯ�����ַ���Ѿ����ú�Ϊ0x0C,
			�����Լ����ģ�ע��ʹ��M1��Ǯ�����밴��һ����ʽ��README�ļ����и�ʽ˵��
			u8* pwd:�������Ա������û�����
			u8 mode:	1:�û�ģʽ  0:����Աģʽ
return:32λʮ���ƵĽ����
function:��ʾ���
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
parameter:	u8 dataAddr���û�Ǯ�����ݵ�ַ
			u8 controlAddr;�û�Ǯ���Ŀ��Ƶ�ַ
			u8* pwd���û�Ǯ������ԿB���������������û�������Ҳ���ܲ���
			u8* oldpwd���û�������
			u8* newpwd���û�������
			u8 mode���û�ģʽģʽ�����Աģʽ���˺���ֻ�ܹ���Աģʽ��
return:	״̬��MI_OK  MI_ERR
function:�����û����룬�û���Ҫ���������ƥ��ɹ���ɸ���������
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
		/*ȷ������ԿA��ͬ*/
		
		/*��������ԿA��Ҳ�����û�����*/
		
		/*ִ�и����������*/
		status=PcdRead(controlAddr,RFID4);
	}
	if(status==MI_OK)
	{
		for(i=0;i<6;i++)
		{
			RFID4[i]=newpwd[i];
		}
		status=MI_ERR;
		status=PcdWrite(controlAddr,RFID4);   //��������
	}
	return status;
}

/*************************************************************
funcname:wallet_Init
parameter:	u8 addr:ѡ���û�Ǯ�����ݵ�ַ��
			u8* pwd:��������A��B�������ģʽƥ�䣨�������ԿB��Ϊ����Աģʽ��
			u8 mode:	1:�û�ģʽ  0:����Աģʽ
return:  MI_OK ����ȷ����     MI_ERR����������
function:Ǯ����ʼ�����˺���ʹ��һ�μ��ɣ�����ʼ�����Ǯ�����Ϊ0
Author:Mr.Wang
Date:2018-5-12 11:28:58
**************************************************************/
u8 wallet_Init(u8 addr,u8* pwd)
{
	u8 status=MI_ERR,i=0;
	status=ReadCard(addr,pwd,0);
	if(status==MI_OK)
	{
		printf("�����ɹ�\r\n");
		status=MI_ERR;
		status=PcdWrite(addr,WalletInitData);   //дǮ�����ݸ�ʽ��ʼ��
	}
	if(status==MI_OK)
	{
		printf("д��Ǯ�����ݳɹ�\r\n");
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
		status=PcdWrite(addrUserCon,WalletInitCon); //дǮ�����ƣ���ԿAB���ɶ�����ԿB�ɸ�д����
	}
	if(status==MI_OK)
	{
		printf("д����ƴ洢�ɹ�\r\n");
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

