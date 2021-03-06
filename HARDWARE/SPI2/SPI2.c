#include "SPI2.h"
#include "rc522.h"

//SPI2的初始化
void SPI2_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC->APB2ENR|=1<<3|1<<2; //开启PA,PB时钟
	GPIOB->CRH&=0x000FFFFF;
	GPIOB->CRH|=0x94900000;  //PB13\14\15
	GPIOA->CRL&=0xFFFFFFF0;
	
	RCC->APB2ENR|=(1<<3); //GPIOB引脚时钟
	RCC->APB1ENR|=(1<<14);//开启SPI2时钟
	GPIOB->CRH&=0x000FFFFF;
	GPIOB->CRH|=0xB4B00000;//配置PB13、PB14、PB15
	GPIOB->BSRR|=7<<13;  //上拉
	
//	GPIOA->CRL&=0xFF00FFFF;
//	GPIOA->CRL|=0x00330000;  //PF0\PF1浮空输出，输出50MHZ
//	GPIOA->BRR=0X06;   //下拉
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //LED0-->PB.5 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
 GPIO_SetBits(GPIOA,GPIO_Pin_8);						 //PB.5 输出高

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				 //LED0-->PB.5 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5

 SET_SPI_CS;
 CLR_RC522RST;
	
	SPI2->CR1|= (0<<11)| //8位数据帧
				(0<<10)| //全双工模式
				(1<<9)|  //启用软件从设备管理
				(1<<8)|  //选择从设备
				(0<<7)|  //先左移MSB
				(1<<2)|  //配置为主设备
				(0<<1)|  //CPOL：时钟极性
				(0<<0);  //CPHA：时钟相位
}
//SPI2速度的设置 ，传入的SPEED为一个数值
//000： fPCLK/2   001： fPCLK/4   010： fPCLK/8    011： fPCLK/16
//100： fPCLK/32  101： fPCLK/64  110： fPCLK/128  111： fPCLK/256
//SPI2接在APB1总线上，最大时钟频率为36MHZ，fPCLK=fAPB1=36MHZ
void SPI2_SpeedSet(u8 speed)
{
	speed&=0x07;
	SPI2->CR1&=0xFFC7;
	SPI2->CR1|=speed<<3;
	SPI2->CR1|=(1<<6);
}
//对spi2的读写操作
u8 SPI2_ReadWriteByte(u8 TxData)
{		
	u16 retry=0;				 
	while((SPI2->SR&1<<1)==0)		//等待发送区空	
	{
		retry++;
		if(retry>=0XFFFE)return 0; 	//超时退出
	}			  
	SPI2->DR=TxData;	 	  		//发送一个byte 
	retry=0;
	while((SPI2->SR&1<<0)==0) 		//等待接收完一个byte  
	{
		retry++;
		if(retry>=0XFFFE)return 0;	//超时退出
	}	  						    
	return SPI2->DR;          		//返回收到的数据				    
}
u8 SPIWriteByte(u8 Byte)
{
	while((SPI2->SR&0X02)==0);		//等待发送区空	  
	SPI2->DR=Byte;	 	            //发送一个byte   
	while((SPI2->SR&0X01)==0);      //等待接收完一个byte  
	return SPI2->DR;          	    //返回收到的数据			
}










