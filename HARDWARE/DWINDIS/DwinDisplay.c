#include "DwinDisplay.h"    
#include "string.h"
#include "delay.h"
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOS使用
#include "task.h"
#include "semphr.h"	  
#endif

//=======================================================================
//以下为迪文串口屏程序

u8 ChooseMode;//0:卖家模式  1：买家模式 
bool DrawbackStatus;//页面返回标志
bool IsSurePublish;//确认发布标志
bool CancelPublish;//取消发布标志
bool IsSureToBuy;//确认购买标志
bool UpdataFlag;//更新标志
u8 IdleBOX=8;//空闲箱子数
u8 CurrentUserID=0xFF;//当前用户的编号
u8 CurrentPage;//当前界面的编号
u8 CurrentBoxNum;//当前箱子的编号
BoxImformate Box[8];//定义了8个箱子(编号0~7)
BoxImformate *BoxPointer;//定义了用于指向箱子的指针
Customer *AccountPointer;//定义了用于指向账户的指针


//切换页面命令
const u8 ChangePageCmd[10]={0X5A,0XA5,0X07,0X82,0X00,0X84,0X5A,0X01,0X00,0X03};
const u8 ResetDwinDisplay[10]={0X5A,0XA5,0X07,0X82,0X00,0X04,0X55,0XAA,0X5A,0XA5};
//用户身份信息录入
//方小东
const u8 UserName0[12]={0X5A,0XA5,0X09,0X82,0X10,0X50,0XB7,0XBD,0XD0,0XA1,0XB6,0XAB};
//叶凡
const u8 UserName1[12]={0X5A,0XA5,0X07,0X82,0X10,0X50,0XD2,0XB6,0XB7,0XB2};
//董大伟
const u8 UserName2[12]={0X5A,0XA5,0X09,0X82,0X10,0X50,0XB6,0XAD,0XB4,0XF3,0XCE,0XB0};
//清空用户数组
const u8 Blank[12]={0X5A,0XA5,0X09,0X82,0X10,0X50,0X00,0X00,0X00,0X00,0X00,0X00};
	
//封装串口屏打印函数
//入口参数：要打印内容的地址
void DWin_DisplayPrint(u8* address)
{
	u8 i;
	for(i=0;i<(address[2]+3);i++)
	{ 
		while((USART2->SR&0X40)==0){}//等待上一次数据发送完成  
		USART2->DR=address[i];
//			
//		//测试使用
//		while((USART1->SR&0X40)==0){}//等待上一次数据发送完成  
//		USART1->DR=address[i];			
	}	
}
//迪文串口屏初始化
void DwinInit(void)
{
	DWin_DisplayPrint((u8*)ResetDwinDisplay);
}
//迪文串口屏页面切换
//入口参数：需要切换到的页面(十六进制数)
void Dwin_ChangePage(u8 page)
{
	u8 index=0;
	u8 temp[10];
	//填充切换页面数据
	for(index=0;index<(CHANGE_PAGECMD-1);index++)
	{
		temp[index]=ChangePageCmd[index];
	}
//	printf("index:%x\r\n",index);
	temp[index]=page;
//	printf("page:%x\r\n",page);
	//发送命令
	DWin_DisplayPrint(temp);
}

u8 buff[40];//GBK内码暂存区
//迪文串口屏文字显示指令
//1:填充buff   0:清空buff
void DwinDisplayCMD(u8 mode)
{
	u8 DataNum=6,temp=0;
	//按协议封装
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

//协议验证
void Protocol_cheak(void)
{
	u8 temp;
	if(USART2_RX_STA&0x8000)//如果数据接收已经完成
	{
		if(USART2_RX_BUF[0]==FRAME_START0)//帧头0x5A验证正确
		{
			if(USART2_RX_BUF[1]==FRAME_START1)//帧头0xA5验证正确
			{
				if(USART2_RX_BUF[(USART2_RX_BUF[2]+1)]==FRAME_END0)//得到数据长度
				{
					if(USART2_RX_BUF[(USART2_RX_BUF[2]+2)]==FRAME_END1)
					{
						//====处理收到的文字========
						printf("接收到文字\r\n");  
						TxtProcess();
					}
				}
				else
				{
					 //====处理接收到的数据========
						printf("接收到数据\r\n");
						DataProcess();
				}	
			}
			temp=(USART2_RX_BUF[2]+2);
			memset(USART2_RX_BUF,0,temp);
			USART2_RX_STA=0;//标志为清零，开始下一次接收
		}
	}
}

//数据处理
void DataProcess(void)
{
	//模式判断
	if(USART2_RX_BUF[ADDRESS_H]==0x16&&USART2_RX_BUF[ADDRESS_L]==0x00)
	{
		if(USART2_RX_BUF[8]==0xDD)	{ChooseMode=0xAA;printf("卖家模式\r\n");}				//卖家模式
			else if(USART2_RX_BUF[8]==0xFF)	{ChooseMode=0XDD;printf("买家模式\r\n");}	//买家模式
	}
	//[请刷卡界面]返回
	if(USART2_RX_BUF[ADDRESS_H]==0x16&&USART2_RX_BUF[ADDRESS_L]==0x20)
	{
		if(USART2_RX_BUF[7]==0xDB&&USART2_RX_BUF[8]==0xDB)
			DrawbackStatus=true;//返回键按下
		printf("返回键按下\r\n");
	}
	//箱子判断
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
								printf("箱子5\r\n");
				break;
			case 0x60:BoxPointer=&Box[5];
								CurrentBoxNum=5;
								printf("箱子6\r\n");
				break;
			case 0x70:BoxPointer=&Box[6];
								CurrentBoxNum=6;						
								printf("箱子7\r\n");
				break;
			case 0x80:BoxPointer=&Box[7];
								CurrentBoxNum=7;						
								printf("箱子8\r\n");
				break;	
			default:break;
		}
	}
	//价格数据处理
		if(USART2_RX_BUF[ADDRESS_H]==0x11&&USART2_RX_BUF[ADDRESS_L]==0x20)
	{
//		BoxPointer->GoodsPrice=((USART2_RX_BUF[7]*256)|USART2_RX_BUF[8])/10;
		BoxPointer->GoodsPrice[0]=USART2_RX_BUF[7];
		BoxPointer->GoodsPrice[1]=USART2_RX_BUF[8];    
//		printf("价格是：%d\r\n",BoxPointer->GoodsPrice);
	}
	
	//确认发布处理
	if(USART2_RX_BUF[ADDRESS_H]==0x14&&USART2_RX_BUF[ADDRESS_L]==0x20)
	{
		if(USART2_RX_BUF[7]==0xAB&&USART2_RX_BUF[8]==0xAA)
		{
			printf("确认发布了\r\n");
			IsSurePublish=true;
		}
	}
	//取消发布
	if(USART2_RX_BUF[ADDRESS_H]==0x14&&USART2_RX_BUF[ADDRESS_L]==0x40)
	{
		if(USART2_RX_BUF[7]==0xAB&&USART2_RX_BUF[8]==0xBB)
		{
			printf("取消发布了\r\n");
			CancelPublish=true;
		}
	}
	//录入信息返回
	if(USART2_RX_BUF[ADDRESS_H]==0x16&&USART2_RX_BUF[ADDRESS_L]==0x30)
	{
		if(USART2_RX_BUF[7]==0xDB&&USART2_RX_BUF[8]==0x55)
		{
			printf("返回了\r\n");
			DrawbackStatus=true;
		}
	}	
	//确认购买
	if(USART2_RX_BUF[ADDRESS_H]==0x14&&USART2_RX_BUF[ADDRESS_L]==0x60)
	{
		if(USART2_RX_BUF[7]==0xAB&&USART2_RX_BUF[8]==0xCC)
		{
			printf("确认购买了\r\n");
			IsSureToBuy=true;;//确认购买标志
		}
	}		
}

//判断身份
u8 CheakID(u8 *IDNumber)
{
	DWin_DisplayPrint((u8 *)Blank);
	if(IDNumber[0]==0xE0||BoxPointer->Owern==1) 
	{	
		//写入身份信息
		DWin_DisplayPrint((u8 *)UserName0);
		CurrentUserID=1;
	}
	else if(IDNumber[0]==0x67||BoxPointer->Owern==2)
	{
		//写入身份信息
		DWin_DisplayPrint((u8 *)UserName1);
		CurrentUserID=2;
	}
	else if(IDNumber[0]==0x10||BoxPointer->Owern==3)
	{
		//写入身份信息
		DWin_DisplayPrint((u8 *)UserName2);
		CurrentUserID=3;
	}
	return CurrentUserID;
}

//修改空闲箱子数量	
void ChangeIDLE_Box(u8 BoxNum,u8 AdressH,u8 AdressL)
{
	u8 tem[10]={0};
	//按协议封装
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
//文字处理
void TxtProcess(void)
{
	u8 temp,index=9,i=0;
	//获取需要保存的数据个数
	temp=USART2_RX_BUF[8];
	//判断是否为物品名称
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

//文字显示
void TxtDisplay(void)
{
	//填充信息
	ReadMesgAndFill(0,0x10,0x10);
	ReadMesgAndFill(1,0x11,0x20);
	ReadMesgAndFill(2,0x11,0x50);
}

//清空原内存中的信息
void DeleteImformation(void)
{
	ReadMesgAndFill(3,0x10,0x10);//清空  物品名称
	ReadMesgAndFill(3,0x11,0x20);//清空	 价格
	ReadMesgAndFill(3,0x11,0x50);//清空	 说明	

}

//清空结构体中的信息
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

//读取箱子结构体中的数据，并填充
//mode: 0 填充物品名称  ； 1 填充价格  ； 2 填充说明；
void ReadMesgAndFill(u8 mode,u8 AdressH,u8 AdressL)
{
	u8 TempBuff[40]={0};
	u8 index=6,tmp=0;
	//按协议封装
	TempBuff[0]=FRAME_START0;
	TempBuff[1]=FRAME_START1;
	TempBuff[3]=0x82;
	TempBuff[4]=AdressH;
	TempBuff[5]=AdressL;
	taskENTER_CRITICAL();           //进入临界区
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
	taskEXIT_CRITICAL();            //退出临界区
}

