#pragma config FOSC = INTRCIO 
#pragma config WDTE = OFF      
#pragma config PWRTE = OFF     
#pragma config MCLRE = ON      
#pragma config CP = OFF        
#pragma config CPD = OFF       
#pragma config BOREN = OFF     
#pragma config IESO = OFF      
#pragma config FCMEN = OFF     

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>

#define _XTAL_FREQ 4000000     

#define QUEUE_SIZE 16 

#define DS_1 PORTCbits.RC0
#define DS_2 PORTCbits.RC1
#define DS_3 PORTCbits.RC2
#define DS_4 PORTCbits.RC5
#define DS_5 PORTCbits.RC4
#define DS_6 PORTCbits.RC3
#define DS_7 PORTCbits.RC6
#define DS_8 PORTCbits.RC7
#define DS_9 PORTBbits.RB4
#define DS_10 PORTBbits.RB5

#define P_DS_1 TRISCbits.TRISC0
#define P_DS_2 TRISCbits.TRISC1
#define P_DS_3 TRISCbits.TRISC2
#define P_DS_4 TRISCbits.TRISC5
#define P_DS_5 TRISCbits.TRISC4
#define P_DS_6 TRISCbits.TRISC3
#define P_DS_7 TRISCbits.TRISC6
#define P_DS_8 TRISCbits.TRISC7
#define P_DS_9 TRISBbits.TRISB4
#define P_DS_10 TRISBbits.TRISB5


#define TX PORTBbits.RB7
#define RX PORTBbits.RB6

typedef struct {
    unsigned short data; 
} adcData;

// ?? ? ???
typedef struct {
    adcData buffer[QUEUE_SIZE]; 
    int head; 
    int tail;
    int count; 
} CircularQueue;

// ? ???
void initQueue(CircularQueue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

int isQueueFull(CircularQueue *q) {
    return q->count == QUEUE_SIZE;
}

int isQueueEmpty(CircularQueue *q) {
    return q->count == 0;
}

void enqueue(CircularQueue *q, adcData data) {
    if (!isQueueFull(q)) {
        q->buffer[q->tail] = data;
        q->tail = (q->tail + 1) % QUEUE_SIZE;
        q->count++;
    }
}

adcData dequeue(CircularQueue *q) {
    adcData data = {0};
    if (!isQueueEmpty(q)) {
        data = q->buffer[q->head];
        q->head = (q->head + 1) % QUEUE_SIZE;
        q->count--;
    }
    return data;
}

// ADC ???
void ADC_Init() {
    ANSEL = 0x00;
    ANSELH = 0x00;
    TRISAbits.TRISA2 = 1; 
    ANSELbits.ANS2 = 1;    
    ADCON0bits.CHS = 2;     
    ADCON1bits.ADCS = 0b001; 
    ADCON0bits.ADFM = 1;   
    ADCON0bits.ADON = 1;  
    ADCON0bits.VCFG = 0;   
}

// ADC ??? ??
adcData ADC_Read() {
    ADCON0bits.GO = 1;  
    while (ADCON0bits.GO_nDONE); 
    adcData result;
    result.data = ((unsigned short)ADRESH << 8) | ADRESL; 
    return result;
}

void sendParallelData(unsigned short data) {
    DS_1 = (data >> 0) & 1;
    DS_2 = (data >> 1) & 1;
    DS_3 = (data >> 2) & 1;
    DS_4 = (data >> 3) & 1;
    DS_5 = (data >> 4) & 1;
    DS_6 = (data >> 5) & 1;
    DS_7 = (data >> 6) & 1;
    DS_8 = (data >> 7) & 1;
    DS_9 = (data >> 8) & 1;
    DS_10 = (data >> 9) & 1;
}

void main(void) {
    ADC_Init(); 
    CircularQueue adcQueue;
    initQueue(&adcQueue); 

    P_DS_1 = 0;
    P_DS_2 = 0;
    P_DS_3 = 0;
    P_DS_4 = 0;
    P_DS_5 = 0;
    P_DS_6 = 0;
    P_DS_7 = 0;
    P_DS_8 = 0;
    P_DS_9 = 0;
    P_DS_10 = 0;
    
    TRISBbits.TRISB7 = 0;  
    TRISBbits.TRISB6 = 1;

    TX = 0; 
    
    while (1) {
        if (!isQueueFull(&adcQueue)) {
            adcData adcValue = ADC_Read();
            enqueue(&adcQueue, adcValue);  
        }
        if(RX == 1){
            if (!isQueueEmpty(&adcQueue)) {
                adcData data = dequeue(&adcQueue); 

                TX = 1;  
                sendParallelData(data.data);  

                TX = 0;  
                //__delay_ms(10);  // ?? ??? ?? ??
            }
        }
        
    }
}
