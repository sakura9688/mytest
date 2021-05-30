#include "DwinDisplay.h"    
#include "string.h"
#include "delay.h"
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOSʹ��
#include "task.h"
#include "semphr.h"	  
#endif

//=======================================================================
//����Ϊ���Ĵ���������

u8 ChooseMode;//0:����ģʽ  1�����ģʽ 
bool DrawbackStatus;//ҳ�淵�ر�־
bool IsSurePublish;//ȷ�Ϸ�����־
bool CancelPublish;//ȡ��������־
bool IsSureToBuy;//ȷ�Ϲ����־
bool UpdataFlag;//���±�־
u8 IdleBOX=8;//����������
u8 CurrentUserID=0xFF;//��ǰ�û��ı��
u8 CurrentPage;//��ǰ����ı��
u8 CurrentBoxNum;//��ǰ���ӵı��
BoxImformate Box[8];//������8������(���0~7)
BoxImformate *BoxPointer;//����������ָ�����ӵ�ָ��
Customer *AccountPointer;//����������ָ���˻���ָ��


//�л�ҳ������
const u8 ChangePageCmd[10]={0X5A,0XA5,0X07,0X82,0X00,0X84,0X5A,0X01,0X00,0X03};
const u8 ResetDwinDisplay[10]={0X5A,0XA5,0X07,0X82,0X00,0X04,0X55,0XAA,0X5A,0XA5};
//�û������Ϣ¼��
//��С��
const u8 UserName0[12]={0X5A,0XA5,0X09,0X82,0X10,0X50,0XB7,0XBD,0XD0,0XA1,0XB6,0XAB};
//Ҷ��
const u8 UserName1[12]={0X5A,0XA5,0X07,0X82,0X10,0X50,0XD2,0XB6,0XB7,0XB2};
//����ΰ
const u8 UserName2[12]={0X5A,0XA5,0X09,0X82,0X10,0X50,0XB6,0XAD,0XB4,0XF3,0XCE,0XB0};
//����û�����
const u8 Blank[12]={0X5A,0XA5,0X09,0X82,0X10,0X50,0X00,0X00,0X00,0X00,0X00,0X00};
	
//��װ��������ӡ����
//��ڲ�����Ҫ��ӡ���ݵĵ�ַ
void DWin_DisplayPrint(u8* address)
{
	u8 i;
	for(i=0;i<(address[2]+3);i++)
	{ 
		while((USART2->SR&0X40)==0){}//�ȴ���һ�����ݷ������  
		USART2->DR=address[i];
//			
//		//����ʹ��
//		while((USART1->SR&0X40)==0){}//�ȴ���һ�����ݷ������  
//		USART1->DR=address[i];			
	}	
}
//���Ĵ�������ʼ��
void DwinInit(void)
{
	DWin_DisplayPrint((u8*)ResetDwinDisplay);
}
//���Ĵ�����ҳ���л�
//��ڲ�������Ҫ�л�����ҳ��(ʮ��������)
void Dwin_ChangePage(u8 page)
{
	u8 index=0;
	u8 temp[10];
	//����л�ҳ������
	for(index=0;index<(CHANGE_PAGECMD-1);index++)
	{
		temp[index]=ChangePageCmd[index];
	}
//	printf("index:%x\r\n",index);
	temp[index]=page;
//	printf("page:%x\r\n",page);
	//��������
	DWin_DisplayPrint(temp);
}

u8 buff[40];//GBK�����ݴ���
//���Ĵ�����������ʾָ��
//1:���buff   0:���buff
void DwinDisplayCMD(u8 mode)
{
	u8 DataNum=6,temp=0;
	//��Э���װ
	buff[0]=FRAME_START0;
	buff[1]=FRAME_START1;
	buff[3]=0x82;
	switch(CurrentBoxNum)
	{
		case 0:buff[4]=0x11;
					 buff[5]=0x60;
			break;
		case 1:buff[4]=0x11;
					 buff[5]=0x80;
			break;
		case 2:buff[4]=0x12;
					 buff[5]=0x00;
			break;
		case 3:buff[4]=0x12;
					 buff[5]=0x20;
			break;
		case 4:buff[4]=0x12;
					 buff[5]=0x40;
			break;
		case 5:buff[4]=0x12;
					 buff[5]=0x60;
			break;
		case 6:buff[4]=0x12;
					 buff[5]=0x80;
			break;
		case 7:buff[4]=0x13;
					 buff[5]=0x00;
			break;
	}
	
	if(mode==1)
	{
		while(BoxPointer->GoodsName[temp]!=0x00)
		{
			buff[DataNum++]=BoxPointer->GoodsName[temp++];
		}
	}
	else if(mode==0)
	{
		buff[DataNum++]=0xBF;
		buff[DataNum++]=0xD5;	
		buff[DataNum++]=0x00;
		buff[DataNum++]=0x00;	
	}
	buff[2]=DataNum-3;
}

//Э����֤
void Protocol_cheak(void)
{
	u8 temp;
	if(USART2_RX_STA&0x8000)//������ݽ����Ѿ����
	{
		if(USART2_RX_BUF[0]==FRAME_START0)//֡ͷ0x5A��֤��ȷ
		{
			if(USART2_RX_BUF[1]==FRAME_START1)//֡ͷ0xA5��֤��ȷ
			{
				if(USART2_RX_BUF[(USART2_RX_BUF[2]+1)]==FRAME_END0)//�õ����ݳ���
				{
					if(USART2_RX_BUF[(USART2_RX_BUF[2]+2)]==FRAME_END1)
					{
						//====�����յ�������========
						printf("���յ�����\r\n");  
						TxtProcess();
					}
				}
				else
				{
					 //====������յ�������========
						printf("���յ�����\r\n");
						DataProcess();
				}	
			}
			temp=(USART2_RX_BUF[2]+2);
			memset(USART2_RX_BUF,0,temp);
			USART2_RX_STA=0;//��־Ϊ���㣬��ʼ��һ�ν���
		}
	}
}

//���ݴ���
void DataProcess(void)
{
	//ģʽ�ж�
	if(USART2_RX_BUF[ADDRESS_H]==0x16&&USART2_RX_BUF[ADDRESS_L]==0x00)
	{
		if(USART2_RX_BUF[8]==0xDD)	{ChooseMode=0xAA;printf("����ģʽ\r\n");}				//����ģʽ
			else if(USART2_RX_BUF[8]==0xFF)	{ChooseMode=0XDD;printf("���ģʽ\r\n");}	//���ģʽ
	}
	//[��ˢ������]����
	if(USART2_RX_BUF[ADDRESS_H]==0x16&&USART2_RX_BUF[ADDRESS_L]==0x20)
	{
		if(USART2_RX_BUF[7]==0xDB&&USART2_RX_BUF[8]==0xDB)
			DrawbackStatus=true;//���ؼ�����
		printf("���ؼ�����\r\n");
	}
	//�����ж�
	if(USART2_RX_BUF[ADDRESS_H]==0x13&&USART2_RX_BUF[ADDRESS_L]==0x20)
	{
		UpdataFlag=true;
		switch(USART2_RX_BUF[7])
		{
			case 0x10:BoxPointer=&Box[0];
								CurrentBoxNum=0;
				break;
			case 0x20:BoxPointer=&Box[1];
								CurrentBoxNum=1;
				break;
			case 0x30:BoxPointer=&Box[2];
								CurrentBoxNum=2;			
				break;
			case 0x40:BoxPointer=&Box[3];
								CurrentBoxNum=3;
				break;
			case 0x50:BoxPointer=&Box[4];
								CurrentBoxNum=4;				
								printf("����5\r\n");
				break;
			case 0x60:BoxPointer=&Box[5];
								CurrentBoxNum=5;
								printf("����6\r\n");
				break;
			case 0x70:BoxPointer=&Box[6];
								CurrentBoxNum=6;						
								printf("����7\r\n");
				break;
			case 0x80:BoxPointer=&Box[7];
								CurrentBoxNum=7;						
								printf("����8\r\n");
				break;	
			default:break;
		}
	}
	//�۸����ݴ���
		if(USART2_RX_BUF[ADDRESS_H]==0x11&&USART2_RX_BUF[ADDRESS_L]==0x20)
	{
//		BoxPointer->GoodsPrice=((USART2_RX_BUF[7]*256)|USART2_RX_BUF[8])/10;
		BoxPointer->GoodsPrice[0]=USART2_RX_BUF[7];
		BoxPointer->GoodsPrice[1]=USART2_RX_BUF[8];    
//		printf("�۸��ǣ�%d\r\n",BoxPointer->GoodsPrice);
	}
	
	//ȷ�Ϸ�������
	if(USART2_RX_BUF[ADDRESS_H]==0x14&&USART2_RX_BUF[ADDRESS_L]==0x20)
	{
		if(USART2_RX_BUF[7]==0xAB&&USART2_RX_BUF[8]==0xAA)
		{
			printf("ȷ�Ϸ�����\r\n");
			IsSurePublish=true;
		}
	}
	//ȡ������
	if(USART2_RX_BUF[ADDRESS_H]==0x14&&USART2_RX_BUF[ADDRESS_L]==0x40)
	{
		if(USART2_RX_BUF[7]==0xAB&&USART2_RX_BUF[8]==0xBB)
		{
			printf("ȡ��������\r\n");
			CancelPublish=true;
		}
	}
	//¼����Ϣ����
	if(USART2_RX_BUF[ADDRESS_H]==0x16&&USART2_RX_BUF[ADDRESS_L]==0x30)
	{
		if(USART2_RX_BUF[7]==0xDB&&USART2_RX_BUF[8]==0x55)
		{
			printf("������\r\n");
			DrawbackStatus=true;
		}
	}	
	//ȷ�Ϲ���
	if(USART2_RX_BUF[ADDRESS_H]==0x14&&USART2_RX_BUF[ADDRESS_L]==0x60)
	{
		if(USART2_RX_BUF[7]==0xAB&&USART2_RX_BUF[8]==0xCC)
		{
			printf("ȷ�Ϲ�����\r\n");
			IsSureToBuy=true;;//ȷ�Ϲ����־
		}
	}		
}

//�ж����
u8 CheakID(u8 *IDNumber)
{
	DWin_DisplayPrint((u8 *)Blank);
	if(IDNumber[0]==0xE0||BoxPointer->Owern==1) 
	{	
		//д�������Ϣ
		DWin_DisplayPrint((u8 *)UserName0);
		CurrentUserID=1;
	}
	else if(IDNumber[0]==0x67||BoxPointer->Owern==2)
	{
		//д�������Ϣ
		DWin_DisplayPrint((u8 *)UserName1);
		CurrentUserID=2;
	}
	else if(IDNumber[0]==0x10||BoxPointer->Owern==3)
	{
		//д�������Ϣ
		DWin_DisplayPrint((u8 *)UserName2);
		CurrentUserID=3;
	}
	return CurrentUserID;
}

//�޸Ŀ�����������	
void ChangeIDLE_Box(u8 BoxNum,u8 AdressH,u8 AdressL)
{
	u8 tem[10]={0};
	//��Э���װ
	tem[0]=FRAME_START0;
	tem[1]=FRAME_START1;
	tem[2]=0x05;
	tem[3]=0x82;
	tem[4]=AdressH;
	tem[5]=AdressL;	
	tem[6]=0x00;		
	tem[7]=BoxNum;	
	DWin_DisplayPrint(tem);
}
//���ִ���
void TxtProcess(void)
{
	u8 temp,index=9,i=0;
	//��ȡ��Ҫ��������ݸ���
	temp=USART2_RX_BUF[8];
	//�ж��Ƿ�Ϊ��Ʒ����
	if(USART2_RX_BUF[ADDRESS_H]==0x10&&USART2_RX_BUF[ADDRESS_L]==0x0F)
	{
		while(temp--)
		{
			BoxPointer->GoodsName[i++]=USART2_RX_BUF[index++];
		}
	}
	else if(USART2_RX_BUF[ADDRESS_H]==0x11&&USART2_RX_BUF[ADDRESS_L]==0x4F)
	{
		while(temp--)
		{
			BoxPointer->Description[i++]=USART2_RX_BUF[index++];
		}
	}
	DwinDisplayCMD(1);
}

//������ʾ
void TxtDisplay(void)
{
	//�����Ϣ
	ReadMesgAndFill(0,0x10,0x10);
	ReadMesgAndFill(1,0x11,0x20);
	ReadMesgAndFill(2,0x11,0x50);
}

//���ԭ�ڴ��е���Ϣ
void DeleteImformation(void)
{
	ReadMesgAndFill(3,0x10,0x10);//���  ��Ʒ����
	ReadMesgAndFill(3,0x11,0x20);//���	 �۸�
	ReadMesgAndFill(3,0x11,0x50);//���	 ˵��	

}

//��սṹ���е���Ϣ
void DeleteStructImformation(void)
{
	u8 temp=0;
	while(temp<20)
	{
		buff[temp]=0x00;
		BoxPointer->GoodsName[temp]=0x00;
		BoxPointer->Description[temp++]=0x00;
	}
}

//��ȡ���ӽṹ���е����ݣ������
//mode: 0 �����Ʒ����  �� 1 ���۸�  �� 2 ���˵����
void ReadMesgAndFill(u8 mode,u8 AdressH,u8 AdressL)
{
	u8 TempBuff[40]={0};
	u8 index=6,tmp=0;
	//��Э���װ
	TempBuff[0]=FRAME_START0;
	TempBuff[1]=FRAME_START1;
	TempBuff[3]=0x82;
	TempBuff[4]=AdressH;
	TempBuff[5]=AdressL;
	taskENTER_CRITICAL();           //�����ٽ���
	if(mode==0)
	{
		while(BoxPointer->GoodsName[tmp]!=0x00)
		{
			TempBuff[index++]=BoxPointer->GoodsName[tmp++];
		}
	}
	else if(mode==1)
	{
		while(tmp<2)
		{
			TempBuff[index++]=BoxPointer->GoodsPrice[tmp++];
		}
	}
	else if(mode==2)
	{
		while(BoxPointer->Description[tmp]!=0x00)
		{
			TempBuff[index++]=BoxPointer->Description[tmp++];
		}			
	}
	else if(mode==3)
	{
		while(index<6)
		{ 
			TempBuff[index++]=0x00;
		}
		tmp=6;
	}
	TempBuff[2]=tmp+3;
	DWin_DisplayPrint(TempBuff);
	taskEXIT_CRITICAL();            //�˳��ٽ���
}

