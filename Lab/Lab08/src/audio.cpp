// Example Code for getting data from audio sensor
#include "mbed.h"
#define BETA 3975

AnalogIn sensor_left(GPIO0);          // Get left sound -> GPIO0
AnalogIn sensor_right(GPIO2);         // Get right sound -> GPIO2
//DigitalOut vcc(GPIO0);

//double audio_converter(int RawADC) {
//    double audio;
//    audio= (float) 10000.0 * ((65536.0 / RawADC) - 1.0);
//    audio = (1/((log(audio/10000.0)/BETA) + (1.0/298.15)));
//    printf("ori: %f\n", audio);
//
//    return audio;
//}

int main() {
    printf("\r\nAudio Test program");
    printf("\r\n********\r\n");
    float val_left, val_right;
    while (true) {
        val_left = sensor_left.read();
        val_right = sensor_right.read();
        printf("Left sound: %f\n",val_left);
        printf("Right sound: %f\n",val_right);
        wait(1);
    }
}