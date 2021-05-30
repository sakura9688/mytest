#include "usart.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "usart2.h"
#include "timers.h"


//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define TASK1_TASK_PRIO		2
//�����ջ��С	
#define TASK1_STK_SIZE 		128  
//������
TaskHandle_t Task1Task_Handler;
//������
void task1_task(void *pvParameters);


//�������ȼ�
#define DWINUSART_TASK_PRIO		3
//�����ջ��С	
#define DWINUSART_STK_SIZE 		256  
//������
TaskHandle_t DwinUsart_Handler;
//������
void DwinUsart_task(void *pvParameters);

TimerHandle_t 	DwinUsartTimer_Handle;			//���ڶ�ʱ�����
void DwinUsartCallback(TimerHandle_t xTimer); 	//���ڶ�ʱ���ص�����

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4	 
	delay_init();	    				//��ʱ������ʼ��	 
	system_init();
	uart_init(115200);					//��ʼ������
	USART2_Init(115200);

		
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
    //����TASK1����
    xTaskCreate((TaskFunction_t )task1_task,             
                (const char*    )"task1_task",           
                (uint16_t       )TASK1_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TASK1_TASK_PRIO,        
                (TaskHandle_t*  )&Task1Task_Handler);
								
		xTimerStart(DwinUsartTimer_Handle,0);	//�������ڶ�ʱ��						
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//task1������
void task1_task(void *pvParameters)
{
	while(1)
	{		

    vTaskDelay(500);                           //��ʱ1s��Ҳ����1000��ʱ�ӽ���	
	}
}



