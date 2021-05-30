#include "usart.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "usart2.h"
#include "timers.h"


//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define TASK1_TASK_PRIO		2
//任务堆栈大小	
#define TASK1_STK_SIZE 		128  
//任务句柄
TaskHandle_t Task1Task_Handler;
//任务函数
void task1_task(void *pvParameters);


//任务优先级
#define DWINUSART_TASK_PRIO		3
//任务堆栈大小	
#define DWINUSART_STK_SIZE 		256  
//任务句柄
TaskHandle_t DwinUsart_Handler;
//任务函数
void DwinUsart_task(void *pvParameters);

TimerHandle_t 	DwinUsartTimer_Handle;			//周期定时器句柄
void DwinUsartCallback(TimerHandle_t xTimer); 	//周期定时器回调函数

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4	 
	delay_init();	    				//延时函数初始化	 
	system_init();
	uart_init(115200);					//初始化串口
	USART2_Init(115200);

		
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    //创建TASK1任务
    xTaskCreate((TaskFunction_t )task1_task,             
                (const char*    )"task1_task",           
                (uint16_t       )TASK1_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TASK1_TASK_PRIO,        
                (TaskHandle_t*  )&Task1Task_Handler);
								
		xTimerStart(DwinUsartTimer_Handle,0);	//开启周期定时器						
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

//task1任务函数
void task1_task(void *pvParameters)
{
	while(1)
	{		

    vTaskDelay(500);                           //延时1s，也就是1000个时钟节拍	
	}
}



