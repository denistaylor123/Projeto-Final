/*
 * File:   main.c
 * Author: Denis
 *
 * Created on 12 de Julho de 2021, 20:17
 */


#include <xc.h>
#include <pic18f4520.h>
#include <stdio.h>
#include "Config_Fuses.h"
#include "io.h"
#include "lcd.h"
#include "bits.h"
#include "adc.h"
#include <math.h>
#include "PWM.h"


//**************variaveis global **********************//
#define Varredura PORTDbits.RD0
#define Sin_Operacao PORTDbits.RD1
#define Bloq_Controle PORTDbits.RD2
#define Botao PORTBbits.RB0
#define Sensor PORTBbits.RB1
#define Chave PORTBbits.RB2
#define _XTAL_FREQ 8000000


char linha_1[16];
char linha_2[16];
char linha_3[16];
char linha_4[17];
char linha_5[17];
int Dado_1 = 1023;
int Flag_Botao;
int Flag_Sensor;
int Flag_Chave;
float Ii = 0;
float Kp = 2;
float Ki = 10;
float dT = 0.040;
float Er, Ic, Io, Ir, Vc, Vo, Vu, Po;
float PWM_Porcento;

//*************** Função Limite de Integração******************

float limiteTensaoCorrente(float x) {
    if (x < 0) {
        return 0; //valor minimo de tensão
    }
    if (x > 5) {
        return 5; //valor máximo de tensão
    }
    return x;
}

void main(void) {
    TRISA = 0x07;
    PORTA = 0x00;
    TRISC = 0xC1;
    PORTC = 0x00;
    TRISD = 0x00;
    PORTD = 0x00;
    TRISE = 0x00;
    PORTE = 0x00;
    TRISB = 0xFF;
    PORTB = 0x00;
    lcdInit();
    Init_ADC();
    InitPWM();
    
    
    lcdCommand(0x01);
    lcdCommand(0x81);
    lcdString("PROJETO FINAL");
    lcdCommand(0xC1);
    lcdString("CONTROLE MPPT");
    lcdCommand(0xD0);
    lcdString("EMBARCADO UNIFEI");
    __delay_ms(5000);
    lcdCommand(0x01);
   
    Sin_Operacao = 1;
    Flag_Botao = 0;
    Flag_Sensor = 0;
    Flag_Chave = 0;

    while (1) {

        if (Chave == 0) {
            if (Flag_Chave == 0) {
                Flag_Chave = 1;
            } else
                Flag_Chave = 0;
        }
        //*************** Leitura do Botão de Bloqueio ********************

        //Saída de Sinalização
        if (Botao == 0) {
            if (Flag_Botao == 0) {
                Flag_Botao = 1;
                Sin_Operacao = 0; //Sinal de Conntrole Desliga
            } else {
                Flag_Botao = 0;
                Sin_Operacao = 1;
            }
            __delay_ms(15);
        }
        if (Sensor == 0) {
            if (Flag_Sensor == 0) {
                Flag_Sensor = 1;
            } else {
                Flag_Sensor = 0;
                Flag_Botao = 0;
                Sin_Operacao = 1;
            }
            __delay_ms(15);
        }
        //************Condições para Controle********************
        if ((Flag_Sensor == 1) && (Flag_Botao == 1)) {
            //Controle será habilitado
            Bloq_Controle = 0; //desliga o sinal de Sistema bloqueado e libeera
            Varredura = 1; // led começa a pisca indicando o inicio da varredura

            //Leitura da Tensão do Sistema
            Vo = Ler_Tensao(0); //conversão de dados 5*Dado / 1024

            //Leitura do Corrente do Sistema
            Io = Ler_Tensao(1); //5*Dado / 1024 conversão

            Po = Vo*Io; //calculando valaor de Potencia

            //Leitura de referência da corrente
            Ir = Ler_Tensao(2); //conversão de dados 5*Dado / 1024

            //Leitura da Chave de Composição da referência
            if (Flag_Chave == 1) {
                Ic = Ir + 0.01 * Vo; //Faz composição de referência
            } else
                Ic = Ir; //utilizaza somente a referência sem composição

            //Conrole PI de Corrente de inicio
            Er = Ic - Io; //Erro (reverso)da Malha de Corrente

            Ii = Ii + Er*dT; //Parte integral

            //Limites de integração
            Ii = limiteTensaoCorrente(Ii);

            //Soma da Parte Proporcional e Integral
            Vc = Kp * Er + Ki*Ii;

            //Limite d Informação de Comando
            Vc = limiteTensaoCorrente(Vc);

            Dado_1 = (int) (1023 * Vc / 5.0); //Conversão de Dados
            EscrevePWM1(Dado_1); //Valor de Saida de Controle 1

            Vu = Vc;
            if (Vu < 3.333) {
                Vu = 3.333; //Valor correspodente a ALFA = 120
            }
            __delay_ms(15);

            //Indicação de Varredura
            Varredura = 0;
        } else {
            //Desasbilita o Controle
            Bloq_Controle = 1;
            //Sinaliza Controle não Atuando
            Dado_1 = 0;
            Ii = 0;
            EscrevePWM1(Dado_1);
        }
        sprintf(linha_1, "Vo:%1.2f", Ler_Tensao(0)); //Grava texto linha 1
        sprintf(linha_2, "Io:%1.2f", Ler_Tensao(1)); //Grava texto linha 2
        sprintf(linha_3, "Po:%1.2f", Ler_Tensao(0)* Ler_Tensao(1)); //Grava texto linha 3
        sprintf(linha_5, "Ic:%1.2f", Ic); //Grava texto linha 4
        sprintf(linha_4, "PWM:%2.3f", EscrevePWM1(Dado_1)); //Grava texto linha 4

        lcdCommand(0x80); //posiciona o cursor na linha 1, caracter 1
        lcdString(linha_1); //escreve texto na linha 1 do lcd
        lcdCommand(0xC0); //posiciona o cursor na linha 1, caracter 10
        lcdString(linha_2); //escreve texto na linha 1 do lcd
        lcdCommand(0x90); //posiciona o cursor na linha 2, caracter 1
        lcdString(linha_3); //escreve texto na linha 2 do lcd
        lcdCommand(0x99); //escreve testo na linha 3, caracter 5
        lcdString(linha_5); //escreve na linha 3 do lcd
        lcdCommand(0xD0); //posiciona o cursor na linha 2, caracter 10
        lcdString(linha_4); //escreve texto na linha 2 do lcd

    }

}
