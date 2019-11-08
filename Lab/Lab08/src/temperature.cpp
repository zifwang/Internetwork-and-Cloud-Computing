// Example Code for getting data from temperature sensor
#include "mbed.h"
#define BETA 3975

AnalogIn sensor(PB_0);          // Get data from PB_0 pin
DigitalOut vcc(GPIO0);

double Thermistor(int RawADC) {
    double Temp;
    Temp= (float) 10000.0 * ((65536.0 / RawADC) - 1.0);
    Temp = (1/((log(Temp/10000.0)/BETA) + (1.0/298.15)));
    printf("ori: %f\n", Temp);
    Temp = Temp - 273.15; // Convert Kelvin to Celcius
    //Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert C to F
    return Temp;
}

int main() {
    printf("\r\nTemperature Test program");
    printf("\r\n********\r\n");
    vcc = 1;
    unsigned int val;
    while (true) {
        val=sensor.read_u16();
        double temp = Thermistor(val);
        printf("%f\n",temp);
        wait(5);
    }
}