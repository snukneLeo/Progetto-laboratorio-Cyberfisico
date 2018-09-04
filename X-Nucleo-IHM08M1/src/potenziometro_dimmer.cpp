/*
#include <mbed.h>

AnalogIn input(PB_1);
Serial pci(USBTX,USBRX);
PwmOut led(LED1);

int main()
{
    uint16_t saples[1024];
        

    while(1)
    {
        float ligth = input.read(); 
        //float pciValue = pci.printf("\n %f", input.read());  
        pci.printf("\n %d", input.read_u16());  //alternativa
        // wait(0.002f);
        //printf("Valore Potenziometro: %f\n",pciValue);

        led = ligth;
        //pci.printf("%i\n",led);
        //led = led + 0.1;
        wait(0.2);
        if(led >= 1.0) 
        {
            //printf("Limite raggiunto...\n");
            wait(0.2);
            led = 0;
        }
    }
}
*/