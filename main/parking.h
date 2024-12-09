#ifndef PARKING_H
#define PARKING_H

#include "defs.h"
#include<stdint.h>
#define MAX_PARKING 100
typedef struct {
	char name[20];
	char time[20];
	uint8_t isOcupied;
	} ParkingPlace;

#define tag "PARKING"
//MAX OF 100 PARKINGS
static  ParkingPlace g_p[MAX_PARKING];
static int num_g_p = 0;
static void addValueToParking(const char* name, const char* time, uint8_t isOcupied);
extern void parseParkingTriger(const char* content);
extern void parseParkingInterval(const char* content);
static void printParking(int i);
static void printParkings();
static void WriteParkingToAFile(ParkingPlace tempParking, Topics topic);
extern void ParkingTask(void* parms);
#endif
