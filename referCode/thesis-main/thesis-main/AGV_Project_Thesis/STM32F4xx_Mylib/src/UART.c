#include "UART.h"
#include <string.h>
#include <stdio.h>
#define	  Tx_BUFF_SIZE		22
#define   Rx_BUFF_SIZE    1
void init_main(void);

extern uint8_t statusCompletePath;
uint8_t 	rxbuff[Rx_BUFF_SIZE], data_receive[26];
uint8_t PathByteCount_Run = 0; 
uint8_t pathRunReceive[40];
extern uint8_t unSendInformation;
char txbuff[Tx_BUFF_SIZE];	
float kp,ki,kd;
uint8_t rxdata=0, pre_rxdata;

int b = 0;
int on = 0;
int v_sv = 0;
float setVelocity = 0; 
float setki=0;
int16_t crc_cal=0, crc_receive =0;

uint8_t crc1=0,crc2=0;
float setkp_line =0.5, setki_line =0.0005, setkd_line =0.05;
float setkp_speed =2, setki_speed =2, setkd_speed =0.015;
float setkp_lift = 0.7, setki_lift = 2, setkd_lift = 0.0015;
int flag_receiver =0, i=0;
uint8_t flagPathReceived = 0;
extern uint8_t flagCompleteTask;
typedef struct{
	uint16_t Header;
	uint8_t FunctionCode; // 0x01
	uint8_t AGVID;
	float Velocity;
	float Udk;
	float Line_Position;
	float delta_Udk;
	uint16_t EndOfFrame;
}__attribute__((packed)) SendAGVInfoStruct;

extern SendAGVInfoStruct send_frame;


typedef struct{
	uint16_t Header;
	uint8_t FunctionCode; // 0xC1
	uint8_t AGVID;
  uint8_t ACK;
	uint16_t EndOfFrame;
}__attribute__((packed)) ACK_Frame_Speed;

ACK_Frame_Speed send_ACK_Speed;


typedef struct{
	uint16_t Header;
	uint8_t FunctionCode; // 0xC1
	uint8_t AGVID;
  uint8_t ACK;
	uint16_t EndOfFrame;
}__attribute__((packed)) ACK_Frame_Line;

ACK_Frame_Line send_ACK_Line;

typedef struct{
	uint16_t Header;
	uint8_t FunctionCode; // 0xC1
	uint8_t AGVID;
  uint8_t ACK;
	uint16_t EndOfFrame;
}__attribute__((packed)) ACK_Frame_Path;

ACK_Frame_Path send_ACK_Path;

typedef struct{
	uint16_t Header;
	uint8_t FunctionCode; 
	uint8_t AGVID;
  uint8_t NAK;
	uint16_t EndOfFrame;
}__attribute__((packed)) NAK_Frame_Speed;

NAK_Frame_Speed send_NAK_Speed;
typedef struct{
	uint16_t Header;
	uint8_t FunctionCode; 
	uint8_t AGVID;
  uint8_t NAK;
	uint16_t EndOfFrame;
}__attribute__((packed)) NAK_Frame_Line;

NAK_Frame_Line send_NAK_Line;

typedef struct{
	uint16_t Header;
	uint8_t FunctionCode; 
	uint8_t AGVID;
  uint8_t NAK;
	uint16_t EndOfFrame;
}__attribute__((packed)) NAK_Frame_Path;

NAK_Frame_Path send_NAK_Path;

void UART_Init(void)
{
	/* Create the typedef for GPIO,USART,DMA,NVIC */
  GPIO_InitTypeDef 	GPIO_InitStructure; 
	USART_InitTypeDef USART_InitStructure;  
	DMA_InitTypeDef   DMA_InitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;	
   
  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  /* Enable UART clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	/* Enable DMA1 clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);



  /* GPIO Configuration for UART4 Tx */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* GPIO Configuration for USART Rx */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* Connect UART4 pins to AF2 */  
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2); 
       
  /* USARTx configured as follow:
		- BaudRate = 115200 baud  
    - Word Length = 8 Bits
    - One Stop Bit
    - No parity
    - Hardware flow control disabled (RTS and CTS signals)
    - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);

  /* Enable USART */
  USART_Cmd(USART2, ENABLE);
	/* Enable UART4 DMA */
  USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	/* Enable UART4 DMA */
  USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE); 
	
	/* DMA1 Stream2 Channel4 for USART4 Rx configuration */			
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)rxbuff;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = Rx_BUFF_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream5, &DMA_InitStructure);
	DMA_Cmd(DMA1_Stream5, ENABLE);
	
	/* DMA1 Stream4 Channel4 for USART4 Tx configuration */			
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)txbuff;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = Tx_BUFF_SIZE;
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);
	DMA_Cmd(DMA1_Stream6, ENABLE);
	
	/* Enable DMA Interrupt to the highest priority */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Transfer complete interrupt mask */
  DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);
}

		
void Send_ACK_Speed(void)          // ACD de xac nhan du lieu da nhan hop le
{
    send_ACK_Speed.Header = 0xFFAA;
	  send_ACK_Speed.FunctionCode = 0xC1;
    send_ACK_Speed.AGVID = 1;
	  send_ACK_Speed.ACK = 'A';
	  send_ACK_Speed.EndOfFrame = 0x0A0D;
	  UART_SendData((uint8_t *) &send_ACK_Speed ,sizeof(send_ACK_Speed)) ;
}
void Send_ACK_Line(void)          // ACD de xac nhan du lieu da nhan hop le
{
    send_ACK_Line.Header = 0xFFAA;
	  send_ACK_Line.FunctionCode = 0xC2;
    send_ACK_Line.AGVID = 1;
	  send_ACK_Line.ACK = 'A';
	  send_ACK_Line.EndOfFrame = 0x0A0D;
	  UART_SendData((uint8_t *) &send_ACK_Line ,sizeof(send_ACK_Line)) ;
}
void Send_ACK_Path(void)          // ACD de xac nhan du lieu da nhan hop le
{
    send_ACK_Path.Header = 0xFFAA;
	  send_ACK_Path.FunctionCode = 0xC3;
    send_ACK_Path.AGVID = 1;
	  send_ACK_Path.ACK = 'A';
	  send_ACK_Path.EndOfFrame = 0x0A0D;
	  UART_SendData((uint8_t *) &send_ACK_Path ,sizeof(send_ACK_Path)) ;
}
//NAK khi nhan du lieu sai, khong trung FunctionCode
void Send_NAK_Speed(void)           
{
    send_NAK_Speed.Header = 0xFFAA;
	  send_NAK_Speed.FunctionCode = 0xD0;         
    send_NAK_Speed.AGVID = 1;
	  send_NAK_Speed.NAK = 'N';
	  send_NAK_Speed.EndOfFrame = 0x0A0D;
	  UART_SendData((uint8_t *) &send_NAK_Speed ,sizeof(send_NAK_Speed)) ;
}
void Send_NAK_Line(void)           
{
    send_NAK_Line.Header = 0xFFAA;
	  send_NAK_Line.FunctionCode = 0xD1;         
    send_NAK_Line.AGVID = 1;
	  send_NAK_Line.NAK = 'N';
	  send_NAK_Line.EndOfFrame = 0x0A0D;
	  UART_SendData((uint8_t *) &send_NAK_Line ,sizeof(send_NAK_Line)) ;
}

void Send_NAK_Path(void)           
{
    send_NAK_Path.Header = 0xFFAA;
	  send_NAK_Path.FunctionCode = 0xD2;         
    send_NAK_Path.AGVID = 1;
	  send_NAK_Path.NAK = 'N';
	  send_NAK_Path.EndOfFrame = 0x0A0D;
	  UART_SendData((uint8_t *) &send_NAK_Path ,sizeof(send_NAK_Path)) ;
}


void DMA1_Stream5_IRQHandler(void)
{
  /* Clear the DMA1_Stream2 TCIF2 pending bit */
  DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
		
	pre_rxdata = rxdata;
	rxdata = rxbuff[0];
	
		if (flag_receiver == 1)
	{
		data_receive[i] = rxdata;
		i++;
	}
	
	if (flag_receiver == 1 && pre_rxdata == 0x0A && rxdata == 0x0D ) 
	{
		
		     crc_cal = CRC_Cal(data_receive,i-4);
		     memcpy(&crc_receive,&data_receive[i-4], 2);
		   
		
		  if(data_receive[0] == 0xA1 && data_receive[1] == 0x01)
			{     
				  if(crc_cal != crc_receive)
						 Send_NAK_Path();
				  else
					{
			       memcpy(&PathByteCount_Run, &data_receive[2], 1);      				// Lay gia tri count cua Path
				     for(int i = 0; i < PathByteCount_Run; i++)                    // 
			       {
				         memcpy(&pathRunReceive[i], &data_receive[3 + i], 1);	// Luu gia tri Path vao array pathRunReceive					
				     }
						 
					   
						 if(setVelocity == 0)
						    setVelocity = 15;
						 Send_ACK_Path();
						 flagPathReceived = 1;
						 statusCompletePath = 0;
				  }
			}
		
			else if(data_receive[0] == 0xAC && data_receive[1] == 0x01 )
			{
				  if(crc_cal != crc_receive)
						  Send_NAK_Speed();
					else
					{
					   memcpy(&setkp_speed, &data_receive[2], 4);
				     memcpy(&setki_speed, &data_receive[6], 4);
				     memcpy(&setkd_speed, &data_receive[10], 4);
				     memcpy(&setVelocity, &data_receive[14], 4);
				     Send_ACK_Speed();
					}
			  	
			}
		
		 else if(data_receive[0] == 0xAD && data_receive[1] == 0x01 )
			{
				  if(crc_cal != crc_receive)
						  Send_NAK_Line();
					else
					{
				     memcpy(&setkp_line, &data_receive[2], 4);
				     memcpy(&setki_line, &data_receive[6], 4);
				     memcpy(&setkd_line, &data_receive[10], 4);
				     memcpy(&setVelocity, &data_receive[2], 4);
				     Send_ACK_Line();
					}
			}
    
			
		flag_receiver = 0;
		i =0;

	}	
	if ( flag_receiver==0 && pre_rxdata == 0xAA && rxdata == 0xFF)		flag_receiver = 1;

	
  //for(uint16_t i=0; i<Rx_BUFF_SIZE; i++)
    //Rx_Data[i] = rxbuff[i];
	

	
	DMA_Cmd(DMA1_Stream5, ENABLE);
	
}
// Send simple data to UART
void UART_Print( const char *pcString ) 
{
 while(*pcString != '\0')
	{
		USART_SendData(USART2, *pcString);
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
		pcString++;
	}
 }
int check = -1;
 // Send multiple data to UART
void UART_PrintNumber(const char *templateC,float a)
{
	 check = snprintf(txbuff,8,templateC,a);
	 UART_Print(&txbuff[0]);
	 UART_Print("\r\n");
}

void UART_SendData( uint8_t *s_data, uint16_t size ) 
{
	
	for (int i=0; i < size;i++)
	{
		USART_SendData(USART2, *(s_data+i));
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	}
}
	



	uint16_t  CRC_Cal(uint8_t messageArray[],uint8_t size)
		{
		 uint16_t CRCFull = 0xFFFF;
            char CRCLSB;

            for (int i = 0; i < size ; i++)
            {
                CRCFull = (uint16_t)(CRCFull ^ messageArray[i]);

                for (int j = 0; j < 8; j++)
                {
                    CRCLSB = (char)(CRCFull & 0x0001);
                    CRCFull = (uint16_t)((CRCFull >> 1) & 0x7FFF);

                    if (CRCLSB == 1)
                        CRCFull = (uint16_t)(CRCFull ^ 0xA001);
                }
            }
						
		return CRCFull;
	}
