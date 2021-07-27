
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef adc_H
#define	adc_H



//************************* variaveis global ******************************
int contador = 0;
float tensao = 0;

//*********************** inicialização do ADC **************************

void Init_ADC(void) {
    TRISA = 0b00000111; //habilita dos pinos A0 a A2 como entrada
    ADCON2 = 0xB6; //fosc/64 - 16TAD - justificado a direita
    ADCON1 = 0x0C; //AN0 - AN1 - AN2 são analógicos - vdd e vss na fonte
    ADCON0bits.ADON = 1; //liga o circuito AD
}

float Ler_Tensao(int canal_adc) {
    float tensao = 0;
    ADCON0bits.CHS = canal_adc; //seleciona o canal 
    ADCON0bits.GO_DONE = 1; //inicia a conversão
    while (ADCON0bits.GODONE == 1) { //aguarda o resultado da conversão
    }
    contador = ADRESH;
    contador <<= 8;
    contador += ADRESL; //transfere o valor para variável
    tensao = ((5 * contador) / 1023.0);
    return tensao;
}

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

