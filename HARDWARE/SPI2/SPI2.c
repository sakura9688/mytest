#include "SPI2.h"
#include "rc522.h"

//SPI2�ĳ�ʼ��
void SPI2_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC->APB2ENR|=1<<3|1<<2; //����PA,PBʱ��
	GPIOB->CRH&=0x000FFFFF;
	GPIOB->CRH|=0x94900000;  //PB13\14\15
	GPIOA->CRL&=0xFFFFFFF0;
	
	RCC->APB2ENR|=(1<<3); //GPIOB����ʱ��
	RCC->APB1ENR|=(1<<14);//����SPI2ʱ��
	GPIOB->CRH&=0x000FFFFF;
	GPIOB->CRH|=0xB4B00000;//����PB13��PB14��PB15
	GPIOB->BSRR|=7<<13;  //����
	
//	GPIOA->CRL&=0xFF00FFFF;
//	GPIOA->CRL|=0x00330000;  //PF0\PF1������������50MHZ
//	GPIOA->BRR=0X06;   //����
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //LED0-->PB.5 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.5
 GPIO_SetBits(GPIOA,GPIO_Pin_8);						 //PB.5 �����

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				 //LED0-->PB.5 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.5

 SET_SPI_CS;
 CLR_RC522RST;
	
	SPI2->CR1|= (0<<11)| //8λ����֡
				(0<<10)| //ȫ˫��ģʽ
				(1<<9)|  //����������豸����
				(1<<8)|  //ѡ����豸
				(0<<7)|  //������MSB
				(1<<2)|  //����Ϊ���豸
				(0<<1)|  //CPOL��ʱ�Ӽ���
				(0<<0);  //CPHA��ʱ����λ
}
//SPI2�ٶȵ����� �������SPEEDΪһ����ֵ
//000�� fPCLK/2   001�� fPCLK/4   010�� fPCLK/8    011�� fPCLK/16
//100�� fPCLK/32  101�� fPCLK/64  110�� fPCLK/128  111�� fPCLK/256
//SPI2����APB1�����ϣ����ʱ��Ƶ��Ϊ36MHZ��fPCLK=fAPB1=36MHZ
void SPI2_SpeedSet(u8 speed)
{
	speed&=0x07;
	SPI2->CR1&=0xFFC7;
	SPI2->CR1|=speed<<3;
	SPI2->CR1|=(1<<6);
}
//��spi2�Ķ�д����
u8 SPI2_ReadWriteByte(u8 TxData)
{		
	u16 retry=0;				 
	while((SPI2->SR&1<<1)==0)		//�ȴ���������	
	{
		retry++;
		if(retry>=0XFFFE)return 0; 	//��ʱ�˳�
	}			  
	SPI2->DR=TxData;	 	  		//����һ��byte 
	retry=0;
	while((SPI2->SR&1<<0)==0) 		//�ȴ�������һ��byte  
	{
		retry++;
		if(retry>=0XFFFE)return 0;	//��ʱ�˳�
	}	  						    
	return SPI2->DR;          		//�����յ�������				    
}
u8 SPIWriteByte(u8 Byte)
{
	while((SPI2->SR&0X02)==0);		//�ȴ���������	  
	SPI2->DR=Byte;	 	            //����һ��byte   
	while((SPI2->SR&0X01)==0);      //�ȴ�������һ��byte  
	return SPI2->DR;          	    //�����յ�������			
}










