#include "kalman_filter.h"


void kalman1d_init(Kalman1D *kf, float x0, float P0, float Q, float R) {
    kf->x = x0;
    kf->P = P0;
    kf->Q = Q;
    kf->R = R;
}

float kalman1d_update(Kalman1D *kf, float z) {
    // predict
    float x_pred = kf->x;
    float P_pred = kf->P + kf->Q;

    // update
    float K = P_pred / (P_pred + kf->R);
    kf->x = x_pred + K * (z - x_pred);
    kf->P = (1.0f - K) * P_pred;

    return kf->x; // filtrirani RSSI
}
