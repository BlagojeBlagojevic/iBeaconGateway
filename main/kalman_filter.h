#ifndef _KALMAN_H
#define _KALMAN_F

#define PROCES_VAR     9.0f
#define VAR0          10.0f
#define MESURMENT_VAR  0.1f

typedef struct {
    float x;   // procjena RSSI (dB)
    float P;   // varijanca
    float Q;   // procesna varijanca
    float R;   // mjerenje varijanca
} Kalman1D;

void  kalman1d_init(Kalman1D *kf, float x0, float P0, float Q, float R);
float kalman1d_update(Kalman1D *kf, float z);

#endif