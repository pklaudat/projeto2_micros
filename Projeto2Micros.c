#include <MKL25Z4.h>
// Paulo Ricardo Klaudat Neto - 275760
// Lucas Raupp Hans - 275761
// João Vitor Cabrera - 00281729
int tempo;
short stop; // stop = 1 para
short posic=0;
short porta_aberta;
short tempo_porta=0;
short tempo_espera=3;
short grill_ativo=0;
short nivel_potencia;
short cont1=0,cont2=0,cont3=0;
short saida=0;
short acaba_programa=0;
short umidade_max;
short descongelar=0;
unsigned int umidade=0;
unsigned int valor_AD;
typedef struct {
	short reseta_programa;
	short menu;
	short conta_stop;
	char time[4];
	short grill_menu;
	short descongelar_menu;
	short potencia;
} programa;
void reset(programa *prog){
	short i;
	prog->reseta_programa = 0;
	prog->menu = 0;
	tempo = 0;
	stop = 1; // não utilizada no inicio do programa
	for(i=0;i<4;i++)
	prog->time[i] = '0';
	prog->conta_stop=0;
	posic=0;
	prog->grill_menu=0;
	prog->descongelar_menu=0;
	grill_ativo=0;
	prog->potencia=2;
	nivel_potencia=2;
	GPIOD_PSOR = (1<<0)+(1<<2);
	GPIOD_PCOR = (1<<3);
	umidade_max = 0;
	descongelar=0;
	
}
void atraso_40u(){
	TPM0_MOD = 853;
	while((TPM0_SC & (1<<7))==0);
	TPM0_SC |= (1<<7);
}
void atraso(int cont){
	while(cont>0){
		atraso_40u();
		cont--;
	}
}

void lcd_comando(unsigned char cmd){
	unsigned char aux ;
	aux = cmd & 0x0F;
	GPIOB_PDOR = (aux<<8);
	aux = cmd & 0xF0;
	GPIOE_PDOR = (aux>>2);
	GPIOE_PCOR = (1<<29); // RS = 0;
	GPIOE_PSOR = (1<<23); // E = 1;
	GPIOE_PCOR = (1<<23); // E = 0;
	atraso(1);
}
void lcd_goto(char linha, char coluna){
	if(linha==1)
		lcd_comando(0x80+coluna);
	else 
		lcd_comando(0xC0+coluna);
	
}

void lcd_escreve(unsigned char comando){
	unsigned char aux ;
	aux = comando & 0x0F;
	GPIOB_PDOR = (aux<<8);
	aux = comando & 0xF0;
	GPIOE_PDOR = (aux>>2);
	GPIOE_PSOR = (1<<29); // RS = 1;
	GPIOE_PSOR = (1<<23); // E = 1;
	GPIOE_PCOR = (1<<23); // E = 0;
	atraso(30);
}
void lcd_init(){
	GPIOE_PCOR = (1<<29); // RS = 0;
	lcd_comando(0x38);
	lcd_comando(0x38);
	lcd_comando(0x0C);
	lcd_comando(0x06);
	lcd_comando(0x01);
	atraso(50);
}
void inicializa_kit(){
	// portas digitais
	SIM_SCGC5 = (1<<13)+(1<<10)+(1<<11); // PORT E, B, C
	SIM_SCGC6 = (1<<23)+(1<<24);
	
	GPIOE_PDDR = (1<<2)+(1<<3)+(1<<4)+(1<<5) + (1<<23) + (1<<29) +(1<<30);
	GPIOB_PDDR = (1<<8)+(1<<9)+(1<<10)+(1<<11); //+(1<<18)+(1<<19); //(1<<19);
	GPIOB_PDDR &= ~(1<<0);
	GPIOB_PDDR &= ~(1<<1);
	
	GPIOB_PDDR &= ~(1<<2);
	GPIOB_PDDR &= ~(1<<3);
	GPIOC_PDDR = (1<<8)+(1<<9)+(1<<10)+(1<<11);
	GPIOC_PDDR |= (1<<1); // Saída no pino 1 - Lâmpada 
	GPIOC_PDDR &= ~(1<<2); // Entrada Pino 2 - Porta 
	
	PORTC_PCR1 = (1<<8);
	PORTC_PCR2 = (1<<8)+(1<<4); // Ativado debounce
	PORTC_PCR8 = (1<<8);
	PORTC_PCR9 = (1<<8);
	PORTC_PCR10 = (1<<8);
	PORTC_PCR11 = (1<<8);
	
	PORTE_PCR2 = (1<<8);
	PORTE_PCR3 = (1<<8);
	PORTE_PCR4 = (1<<8);
	PORTE_PCR5 = (1<<8);
	PORTE_PCR23 = (1<<8); //E
	PORTE_PCR29 = (1<<8); // RS
	PORTE_PCR30 = (1<<8); // saída periodica 
	PORTE_PCR8 = (1<<8);
	PORTE_PCR9 = (1<<8);
	PORTE_PCR10 = (1<<8);
	PORTE_PCR11 = (1<<8);
	
	PORTB_PCR8 = (1<<8);
	PORTB_PCR9 = (1<<8);
	PORTB_PCR10 = (1<<8);
	PORTB_PCR11 = (1<<8);
//  PORTB_PCR19 = (1<<8);
	//PORTB_PCR18 = (1<<8);
	PORTB_PCR0 = (1<<8)+(1<<4);
	PORTB_PCR1 = (1<<8)+(1<<4);
	PORTB_PCR2 = (1<<8)+(1<<4);
	PORTB_PCR3 = (1<<8)+(1<<4);
	//TPM0 
	SIM_SOPT2 = (1<<24);
	TPM0_SC = (1<<3) ; // Prescaler 1
	TPM0_MOD = 853; // 40 us
	
	// PIT
	PIT_MCR = 1;
	PIT_LDVAL0 = 1066666/5; // 100 ms segundo
	PIT_TCTRL0 = (1<<1)+1; 
	NVIC_EnableIRQ(PIT_IRQn);	
	
	// PWM - Prato para três triges tristes
	SIM_SCGC6 |= (1<<25); // Habilita TPM1 
	GPIOE_PDDR |= (1<<20); // configura pino 20 como saída
	PORTE_PCR20 = (3<<8); // TPM1CH 0
	TPM1_SC = (1<<3)+3; //clock interno prescaler 8
	TPM1_MOD = 40000; // 15ms
	TPM1_C0SC = (1<<5)+(1<<3);
	TPM1_C0V = 0; // PWM de 25%	

	
	// DAC - PINO E30
	SIM_SCGC6 |= (1<<31); //habilita DAC0
	GPIOE_PDDR |= (1<<30); //comfigura como saida
	PORTE_PCR30 = 3;
	DAC0_C0 = (1 << 7); //DACEN = 1 (habilita DAC)
	saida = 2500; // Inicia em potência média
	DAC0_DAT0H = saida/256;
	DAC0_DAT0L = saida;
	
  
	
	// ADC0 2.0 BALA
	SIM_SCGC6 |= (1<<27);
	ADC0_CFG1 = (1<<2); //12 bits
  ADC0_SC1A = 0x03;
	ADC0_SC3 = 2; // media 16 amostras
	// Temperatura ambiente 1.38 V : 28º
	
	// ADC umidade 
	ADC0_SC1B = 0x07; // liga canal 7 pino D6
	ADC0_CFG2 = (1<<4);
	
	
	
	//while(ADC0_SC2&(1<<7)==)
	
	
	// GRILL
	GPIOC_PDDR |= (1<<12); // LED do grill
	PORTC_PCR12 = (1<<8);
	GPIOC_PCOR = (1<<12); // inicializa o grill desligado;
	// LEDS POTENCIA
	SIM_SCGC5 |= (1<<12); // Liga Port D
	GPIOD_PDDR = (1<<0)+(1<<5)+(1<<2)+(1<<3);
	PORTD_PCR0 = (1<<8);
	PORTD_PCR5 = (1<<8); // Buzzer
	PORTD_PCR2 = (1<<8);
	PORTD_PCR3 = (1<<8);
	
	GPIOD_PDDR |= (1<<1); // ligar led azul
	PORTD_PCR1 = (1<<8); // ligar led azul
	
	// Motor do passinho
	GPIOC_PDDR |= (1<<3)+(1<<4)+(1<<5)+(1<<6);
	PORTC_PCR3 = (1<<8);
	PORTC_PCR4 = (1<<8);
	PORTC_PCR5 = (1<<8);
	PORTC_PCR6 = (1<<8);
	GPIOC_PCOR = (1<<3)+(1<<4)+(1<<5)+(1<<6);
	
	SIM_SCGC5 |= (1<<9);
	GPIOA_PDDR = (1<<1);
	PORTA_PCR1 = (1<<8);
	GPIOA_PCOR = (1<<1);
		
}
void lcd_putc(char caractere){
	switch(caractere){
		case '\f': 
			lcd_comando(0x01); // Apaga LCD
			atraso(50);
		break;
		case '\n':
			lcd_goto(2,0);
		break;
		default:
				GPIOE_PSOR = (1<<29); // RS =1;
				lcd_escreve(caractere);
		break;
	}
}
void lcd_printf(const char *s){
	GPIOE_PSOR = (1<<29);
	while(*s){
		lcd_putc(*s++);
	}
}


void lcd_apaga(){
	lcd_comando(0x01);
	atraso(50);
}
char debounce (char linha, char coluna){
	char k=0, key_last=0, key_now=0, bounce=5;
	atraso(25); // delay de cerca de 1ms
	if(linha==0){ 
		GPIOC_PCOR = (1<<11);
		GPIOC_PSOR = (1<<8)+(1<<9)+(1<<10);
	}
	if(linha==1){
		GPIOC_PCOR = (1<<10);
		GPIOC_PSOR = (1<<8)+(1<<9)+(1<<11);
	
	}
	if(linha==2){
		GPIOC_PCOR = (1<<9);
		GPIOC_PSOR = (1<<8)+(1<<11)+(1<<10);
	
	}
	if(linha==3) {
		GPIOC_PCOR = (1<<8);
		GPIOC_PSOR = (1<<11)+(1<<9)+(1<<10);
	
	}
	while(k!=bounce){
		atraso(25); // delay de cerca de 1 ms
		if(coluna==0) 
			key_now = (GPIOB_PDIR&(1<<3))  ;
		if(coluna==1) 
			key_now = (GPIOB_PDIR&(1<<2));
		if(coluna==2) 
			key_now = (GPIOB_PDIR&(1<<1));
		if(coluna==3) 
			key_now = (GPIOB_PDIR&(1<<0));
		if(key_now==key_last)
			k++;
		else k=0;
		
		key_last = key_now;
			
	}
	return key_now;
}
char trata_teclado(){
	char linha,coluna;
	char vetor[16] = "123A456B789C*0#D";
	for(linha=0;linha<4;linha++)
		for(coluna=0;coluna<4;coluna++)
			if(debounce(linha,coluna)==0){
				while(debounce(linha,coluna)==0);
				GPIOA_PSOR = (1<<1);
				atraso(400);
				GPIOA_PCOR = (1<<1);
				
				return vetor[linha*4+coluna];
				
			}
			return 'x';
}
void print_configurado(programa *a){
	
	lcd_goto(2,6);
	
	if(posic==1) { 
		lcd_printf("00:0");
		lcd_putc(a->time[0]);
	}
	if(posic==2){
		lcd_printf("00:");
		lcd_putc(a->time[0]);
		lcd_putc(a->time[1]);
	}
	if(posic==3){
		lcd_printf("0");
		lcd_putc(a->time[0]);
		lcd_putc(':');
		lcd_putc(a->time[1]);
		lcd_putc(a->time[2]);

	}
	if(posic==4){
		lcd_putc(a->time[0]);
		lcd_putc(a->time[1]);
		lcd_putc(':');
		lcd_putc(a->time[2]);
		lcd_putc(a->time[3]);
		

	}
	
	
}

void le_teclado(programa *prog){
	// L1 -> L4 : PTC 11 -> 8
	// C1 -> C4 : PTB 3 -> 0
	if((GPIOC_PDIR&(1<<2))==0){
		// GPIOC_PCOR = (1<<1); // Se a porta abrir desliga a Lâmpada;
		porta_aberta = 1;
		stop = 1;
		GPIOC_PSOR = (1<<1);
		
	} else { 
		porta_aberta = 0;
		if(stop==1){
			GPIOC_PCOR = (1<<1);
			
		}
	
	}
	if(stop==0){
		TPM1_C0V = 5000;			// Faz prato girar
		DAC0_DAT0H = saida/256;
		DAC0_DAT0L = saida;
		
	
	} else if(stop==1){
							TPM1_C0V = 0; // Faz prato parar
							DAC0_DAT0H = 0;
							DAC0_DAT0L = 0;
	
	}
	
	char tecla;
	tecla = trata_teclado();
	
	switch(tecla){
		case '0': 
		//	lcd_printf("0
			
		if(prog->menu==0 & posic<4 & posic!=0){
				prog->time[posic]='0';
				posic++;
				print_configurado(prog);
			}
				
			break;
		case '1': 
		//	lcd_printf("1");
			if(prog->menu==0 & posic<4){
				prog->time[posic]='1';
				posic++;
				print_configurado(prog);
			}
		
		break;
		case '2':
		//	lcd_printf("2");
			if(prog->menu==0 & posic<4){
				prog->time[posic]='2';
				posic++;
				print_configurado(prog);
				
			}
		break;
		case '3':
		//	lcd_printf("3");
			if(prog->menu==0 & posic<4){
				prog->time[posic]='3';
				posic++;
				print_configurado(prog);
			}
		break;
		case '4':
		//	lcd_printf("4");
			if(prog->menu==0 & posic<4){
				prog->time[posic]='4';
				posic++;
				print_configurado(prog);
			}
		break;
		case '5':
		//	lcd_printf("5");
			if(prog->menu==0 & posic<4){
				prog->time[posic]='5';
				posic++;
				print_configurado(prog);
			}
		break;
		case '6':
		//	lcd_printf("6");
			if(prog->menu==0 & posic<4){
				prog->time[posic] = '6';
				posic++;
				print_configurado(prog);
			}
		break;
		case '7':
		//	lcd_printf("7");
			if(prog->menu==0 & posic<4){	
				prog->time[posic]='7';
				posic++;
				print_configurado(prog);
			}
		break;
		case '8':
		//	lcd_printf("8");
			if(prog->menu==0 & posic<4){
				prog->time[posic] ='8';
				posic++;
				print_configurado(prog);
			}
		break;
		case '9':
		//	lcd_printf("9");
			if(prog->menu==0 & posic<4){
				prog->time[posic]='9';
				posic++;
				print_configurado(prog);
			}
		break;
		case 'A': // +30 seg
			lcd_goto(2,6);
			
		if(prog->menu == 0){
				GPIOC_PSOR = (1<<1);
				lcd_printf("00:30");
				prog->time[1] = '0';
				prog->time[0] = '3';
				prog->time[2] = '0';
				prog->time[3] = '0';
				
				posic=2;
				prog->menu = 1;
			}
			
			if(prog->menu == 1 & tempo>0){
				tempo = tempo+30;
			}
			
		break;
		case 'B': // Grill
			//lcd_printf("B");
				if(stop==1){
				prog->menu = 2;
				prog->grill_menu++;
				}			
				
		break;
		case 'C': // Prato Rápido
			//lcd_printf("C");
			
			if(stop==1){
				
				if(prog->menu!=4)
					prog->potencia = 0;
			
				prog->potencia++;
			
				prog->menu = 4;
			}
		break;
		case 'D': //Descongelar
			//lcd_printf("D");
				if(stop==1){
				grill_ativo=0;
				prog->menu = 3;
				prog->descongelar_menu++;
				if(prog->descongelar_menu==2){ // Carne de boi
					prog->time[0] = '7';
					prog->time[1] = '2';
					prog->time[2] = '0';
					prog->time[3] = '0';
					posic=3;
				}
				else if(prog->descongelar_menu==3){ //Frango
					prog->time[0] = '4';
					prog->time[1] = '5';
					prog->time[2] = '0';
					prog->time[3] = '0';
					posic=3;
				}
				else if(prog->descongelar_menu==4){ // Peixe
					prog->time[0] = '5';
					prog->time[1] = '3';
					prog->time[2] = '0';
					prog->time[3] = '0';
					posic=3;
				}
				else if(prog->descongelar_menu==3){ //Feijao
					prog->time[0] = '5';
					prog->time[1] = '0';
					prog->time[2] = '0';
					prog->time[3] = '0';
					posic=3;
				}
				
			}
				
		break;
		case '*': // Pausar / Cancelar
			//lcd_printf("*");
			if(prog->conta_stop==1 ){
				reset(prog);
			}
				
			if(prog->menu==1){
				stop = 1;
				prog->conta_stop = 1; // Quando testar conta stop de novo e for 1, reseta o programa.
				GPIOC_PCOR = (1<<1); // Desliga lâmpada
				GPIOC_PCOR = (1<<12); // Desliga o grill
			}
			
			if(prog->menu==0){
				prog->reseta_programa=1;	
			}
	
			break;
		
		case '#': // Ligar e Continuar
			if(posic!=0){
			if(prog->conta_stop==1){
				prog->conta_stop = 0;
				stop = 0;
			}
			if(prog->menu==1){
				stop=0;
				posic=1;
				GPIOC_PSOR = (1<<1); //Liga Lâmpada
			}
			if(prog->menu==0 & porta_aberta==0){
				prog->menu = 1;
				GPIOC_PSOR = (1<<1); // Liga lâmpada
			}
			if(prog->menu==2 & porta_aberta==0){
				stop=0;
				prog->menu=1;
				GPIOC_PSOR = (1<<1); // Light up 
				prog->grill_menu=0;
			}
			if(prog->menu==3 & porta_aberta==0){
				stop=0;
				prog->menu=1;
				GPIOC_PSOR = (1<<1); // Light up 
				prog->descongelar_menu=0;
			}
			if(prog->menu==4)
				prog->menu=0;
			
		}
			tempo_espera=0;
			//lcd_printf("#");
			
		break;
		default : break;
		
			}
		atraso(50);
}

void char_to_short_time(programa *prog){ // Converte o tempo em inteiro para fazer contagem regressiva!!!
		int segundos;
		int min[2],seg[2];
		int ts, tmin;
	
		if(posic==1){
			min[1] = 0;
			min[0] = 0; //0
			seg[1] = 0; //1
			seg[0] = (prog->time[0]-48); //2
			
		
		}
		
		if(posic==2){
			min[1] = 0;
			min[0] = 0; //0
			seg[1] = (prog->time[0]-48)*10; //1
			seg[0] = (prog->time[1]-48); //2
			
		
		}
		
		if(posic==3){
			min[1] = 0;
			min[0] = (prog->time[0]-48); //0
			seg[1] = (prog->time[1]-48)*10; //1
			seg[0] = (prog->time[2]-48); //2
			//seg[0] = (prog->time[3]-48); //3
		
		}
		
		if(posic==4){
			min[1] = (prog->time[0]-48)*10; //0
			min[0] = (prog->time[1]-48); //1
			seg[1] = (prog->time[2]-48)*10; //2
			seg[0] = (prog->time[3]-48); //3
		}
		
		segundos = ((min[1]+min[0])*60 +(seg[1]+seg[0]));
		ts = segundos % 60;
		tmin = segundos/60;
		
		prog->time[0] = (tmin/10)+'0';
		prog->time[1] = tmin%10 + '0';
		prog->time[2] = ts/10 + '0';
		prog->time[3] = ts%10 + '0';
		
		tempo = segundos;
}
void controlador(){
	int erro;
	int p, kp,ki,pi;
	int i_ant=0,i=0;
	
	kp = 1;
	ki = 1;
	
	erro = saida-valor_AD;
	p = kp*erro;
	i = i_ant+(erro*ki/10);
	pi = p+i;
	i_ant = i;
	saida = pi;
	DAC0_DAT0H = saida/256;
	DAC0_DAT0L = saida;

}
void PIT_IRQHandler(){ // Conta Tempo e printa contagem regressiva
 //int erro=0,Kp=0,Kd=0,P=0,t_ant=0,PD=0,D=0;
	
	if(stop==0){
		if(++cont1==50){ // x 5
			cont1=0;
			if(tempo>=0){
			
				tempo--;
			}
		}
	}
	
	if(grill_ativo!=0 || nivel_potencia!=0){
		if(++cont2==50){
			cont2=0;
			tempo_espera--;
		
		}
		if(tempo_espera<=0) tempo_espera=0;
	}
	if(stop==0){
		
		++cont3;
		if(cont3==1){ GPIOC_PSOR = (1<<3); GPIOC_PCOR = (1<<6);}
		else if(cont3==2){ GPIOC_PCOR = (1<<3); GPIOC_PSOR = (1<<4); }
		else if(cont3==3){ GPIOC_PCOR = (1<<4); GPIOC_PSOR = (1<<5); }
		else if(cont3==4){ GPIOC_PCOR = (1<<5); GPIOC_PSOR = (1<<6); cont3=0;}
		
		
	} else if(stop==1) { 
		GPIOC_PCOR = (1<<3);
		GPIOC_PCOR = (1<<4);
		GPIOC_PCOR = (1<<5);
		GPIOC_PCOR = (1<<6);
		cont3=0; 
	}
	
	if(stop==1){
		acaba_programa++;
		if(acaba_programa==6000) //x5
			tempo=0;
	} 
	if(stop==0){
			acaba_programa=0;
	//		controlador();
	}
	
	
	PIT_TFLG0 = 1;

	// Lê o valor do potênciometro para definir a umidade
	if(descongelar==1) {
	ADC0_CFG2  = 0;
	ADC0_SC1B = 31;
	ADC0_SC1A = 0x03; // Inicia nova conversao
	
	while(ADC0_SC2 & (1<<7)); // espera terminar de converter
	atraso(1);
	umidade = ADC0_RA; // salva valor na xabaleta 
	
		if(umidade>umidade_max){    // liga led azul 
		GPIOD_PCOR = (1<<1);
	} else GPIOD_PSOR = (1<<1);

	if(umidade>umidade_max ){
		stop=1;
	} else stop =0;
	
	
	}
/*	if(stop==0){
		ADC0_CFG2 = 0;
		ADC0_SC1B = 31;

		ADC0_SC1A = 0x03;
		while(ADC0_SC2 & (1<<7));
		atraso(1);
		erro=saida-ADC0_RA;
		P = erro*Kp;
		D = Kd*5*(t_ant-ADC0_RA)/100;
		t_ant = ADC0_RA;
		PD = (P+D);
		saida = PD;

	}
*/	
} 

void contagem_regressiva(programa *a){
	
	short ts,tmin,tempo_grill;
	
	tempo_grill=0;
	
	if(grill_ativo!=0){
		tempo_grill = 0.3*tempo;
		if(grill_ativo==2 & tempo_grill > 300)
			tempo_grill = 300;
		if(grill_ativo==3 & tempo_grill > 180)
			tempo_grill = 180;
			}

	
	lcd_goto(1,5);
	
//	tempo=100;		
	stop=0;
	while(tempo!=0){		
		
			
			if(tempo<=tempo_grill & grill_ativo!=0) {
					GPIOC_PSOR = (1<<12);
			}	else GPIOC_PCOR = (1<<12);
			
			le_teclado(a);
			
			lcd_goto(2,6);	
			
			ts = tempo % 60;
			tmin = tempo/60;
			
			a->time[0] = (tmin/10)+'0';
			a->time[1] = tmin%10 + '0';
			a->time[2] = ts/10 + '0';
			a->time[3] = ts%10 + '0';
			
			
			if((tempo/60)<9)
			lcd_putc('0');
			else lcd_putc(a->time[0]);
		
			lcd_putc(a->time[1]);
			lcd_putc(':');
		
			if((tempo%60)<9)
				lcd_putc('0');
			else lcd_putc(a->time[2]);
				lcd_putc(a->time[3]);
	
			
		}
		
		GPIOC_PCOR = (1<<12);

		if(stop==0)
		{
		lcd_goto(2,6);
		lcd_printf("00:00");
		atraso(5000);
		lcd_apaga();
		lcd_goto(1,3);
		
		lcd_printf("Concluido");
		GPIOA_PTOR = (1<<1);
		atraso(10000);
		GPIOA_PTOR = (1<<1);
		atraso(10000);
		GPIOA_PTOR = (1<<1);
		atraso(10000);
		GPIOA_PTOR = (1<<1);
		atraso(10000);
		GPIOA_PTOR = (1<<1);
		atraso(10000);
		GPIOA_PTOR = (1<<1);
		atraso(10000);
		GPIOA_PTOR = (1<<1);
		atraso(20000);
		GPIOA_PCOR = (1<<1);
		atraso(20000);	
		}
		reset(a);
		
}
void converte_temperatura(){

}
void menu(programa *prog){
	short segundos,minutos,tempo;



	switch(prog->menu){ // x seleciona o menu 
		case 0: // Menu que digita o tempo de aquecimento
		//estado_prato(0); // Prato desligado
		posic  = 0;
		prog->time[0] = '0';
		prog->time[1] = '1';
		prog->time[2] = '2';
		prog->time[3] = '3';
		
		
		lcd_apaga();
		lcd_goto(1,0);
		lcd_printf(" Digite o Tempo:");
		lcd_goto(2,0);
		lcd_printf("      00:00     ");
		lcd_goto(2,10);
		if(grill_ativo!=0){
			lcd_goto(2,15);
			lcd_putc('G');
		}
		atraso(50);
	
		while(prog->menu==0){	
		le_teclado(prog); 			
			
		if(prog->reseta_programa==1)
		prog->menu = 10;
	}	
			


	
		
		atraso(70);
		break;
		
		case 1:  // Forno ativo - Esquentando os bagulhos 
			lcd_apaga();
			lcd_goto(1,0);
			lcd_printf("  Forno Ativo:  ");
			stop = 1;
			if(grill_ativo!=0){
				lcd_goto(2,15);
				lcd_putc('G');
			}
			char_to_short_time(prog);
			
			minutos = (tempo/60);
			segundos = (tempo % 60);
			lcd_goto(2,6);
			atraso(2);
			
			if(minutos<9) 	
				lcd_putc('0');
			else lcd_putc(prog->time[0]);
			lcd_putc(prog->time[1]);
			lcd_putc(':');

			if(segundos<9)
				lcd_putc('0');
			else lcd_putc(prog->time[2]);
			
			lcd_putc(prog->time[3]);
			atraso(50);
			
			
			contagem_regressiva(prog);
			
		break;
		case 2: // GRILL
					
				
				
			switch(prog->grill_menu){
					case 1:
							lcd_apaga();
							lcd_goto(1,5);
							lcd_printf("MODO");
							lcd_goto(2,5);
							lcd_printf("GRILL");
						//	lcd_goto(2,15);
						//	lcd_putc('G');
							grill_ativo=1;
							tempo_espera=3;
							while(prog->grill_menu==1 & tempo_espera>0){
								le_teclado(prog);
							}
							if(tempo_espera==0){
									prog->menu=0;
									prog->grill_menu=0;
							}
					break;
					case 2:
							lcd_apaga();
							lcd_goto(1,5);
							lcd_printf("Carnes");
							lcd_goto(2,3);
							lcd_printf("Tmax: 05:00");
						//	lcd_goto(2,15);
						//	lcd_putc('G');
							grill_ativo=2;
							tempo_espera=3;
							while(prog->grill_menu==2 & tempo_espera>0){
								le_teclado(prog);
							}
							if(tempo_espera==0){
									prog->menu=0;
									prog->grill_menu=0;
							}
					break;	
					case 3:
							lcd_apaga();
							lcd_goto(1,5);
							lcd_printf("Pizzas");
							lcd_goto(2,3);
							lcd_printf("Tmax: 03:00");
					//		lcd_goto(2,15);
						//	lcd_putc('G');
							grill_ativo=3;
							tempo_espera=3;
							while(prog->grill_menu==3 & tempo_espera>0){
								le_teclado(prog);
							}
							if(tempo_espera==0){
									prog->menu=0;
									prog->grill_menu=0;
							}
					break;
					case 4:
						lcd_apaga();
						lcd_goto(1,3);
						lcd_printf("MODO GRILL");
						lcd_goto(2,3);
						lcd_printf("Desligado");
						atraso(50000);
						prog->reseta_programa=1;
					break;	
					default: break;
				}
		
				

			break;
			case 3: // Modo descongelar
						
			switch(prog->descongelar_menu){
					case 1:
							lcd_apaga();
							lcd_goto(1,2);
							lcd_printf("DESCONGELAR");
							lcd_goto(2,1);
							lcd_printf("-- Alimento --");
							while(prog->descongelar_menu==1){
								le_teclado(prog);
							}
					break;
					case 2:
							lcd_apaga();
							lcd_goto(1,2);
							lcd_printf("DESCONGELAR");
							lcd_goto(2,1);
							lcd_printf(" -- Carnes --");
							while(prog->descongelar_menu==2){
								le_teclado(prog);
							}
							umidade_max = 1600;
							descongelar = 1;
					break;	
					case 3:
							lcd_apaga();
							lcd_goto(1,2);
							lcd_printf("DESCONGELAR");
							lcd_goto(2,1);
							lcd_printf(" -- Frango --");
							while(prog->descongelar_menu==3){
								le_teclado(prog);
							}
							umidade_max = 2800;
							descongelar=1;
					break;
					case 4:
						lcd_apaga();
						lcd_goto(1,2);
						lcd_printf("DESCONGELAR");
						lcd_goto(2,1);
						lcd_printf(" -- Peixes --");
						while(prog->descongelar_menu==4){
							le_teclado(prog);
						}
						umidade_max = 3200;
						descongelar =1;
					break;
					case 5:
						lcd_apaga();
						lcd_goto(1,2);
						lcd_printf("DESCONGELAR");
						lcd_goto(2,1);
						lcd_printf("-- Sorvete --");
						while(prog->descongelar_menu==5){
							le_teclado(prog);
						}
						break;
					case 6:
							prog->reseta_programa=1;
					break;
					
					default: break;
				}
				break;
					case 4: // Modo - potencia
					
					switch(prog->potencia){
					case 1:
							lcd_apaga();
							lcd_goto(1,3);
							lcd_printf("  POTENCIA");
							lcd_goto(2,3);
							lcd_printf("-- BAIXA --");
							GPIOD_PCOR = (1<<2)+(1<<3);
							GPIOD_PSOR = (1<<0);
							tempo_espera=3;
							nivel_potencia=1;
							saida = 1000;
					//		DAC0_DAT0H = saida/256;
					//		DAC0_DAT0L = saida;
	
							while(prog->potencia==1 & tempo_espera>0){
								le_teclado(prog);
							}
							if(tempo_espera==0){
									prog->menu=0;
									prog->potencia=1;
							}
					break;
					case 2:
							lcd_apaga();
							lcd_goto(1,3);
							lcd_printf("  POTENCIA");
							lcd_goto(2,3);
							lcd_printf("-- MEDIA --");
							GPIOD_PSOR = (1<<2);
							nivel_potencia=2;
							tempo_espera=3;
							saida = 2500;
					//		DAC0_DAT0H = saida/256;
					//		DAC0_DAT0L = saida;
	
							while(prog->potencia==2 & tempo_espera>0){
								le_teclado(prog);
							}
							if(tempo_espera==0){
									prog->menu=0;
									prog->potencia=2;
							}
					break;	
					case 3:
							lcd_apaga();
							lcd_goto(1,3);
							lcd_printf("  POTENCIA");
							lcd_goto(2,3);
							lcd_printf("-- ALTA --");
							GPIOD_PSOR = (1<<3);
							tempo_espera=3;
							nivel_potencia=3;
							saida = 4050;
						//	DAC0_DAT0H = saida/256;
						//	DAC0_DAT0L = saida;
	
							while(prog->potencia==3 & tempo_espera>0){
								le_teclado(prog);
							}
							if(tempo_espera==0){
									prog->menu=0;
									prog->potencia=3;
							}
					break;
					case 4:
						prog->reseta_programa=1;
					default: break;
				}
					
					break;
				default: break;

	}
}
int main(){
	programa a;
	reset(&a);
	inicializa_kit();
	lcd_init();

	GPIOD_PSOR = (1<<1);
	
	while(1){
		if(a.reseta_programa==1){
			reset(&a);
		}
		menu(&a);
			
		} 
	}

