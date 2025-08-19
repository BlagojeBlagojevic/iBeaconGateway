#ifndef TAG_H
#define TAG_H
#include "defs.h"
#include "kalman_filter.h"
#include <stdint.h>
#include <string.h>
#include<time.h>
#define MAX_TIME 25

static volatile int time_limit = 60 * 1000 * 1000;
static volatile int rssi_treshold = -100;


//TBD PROBOBLY SOME CALCULATION IN APROXIMATE POS OF A BEACON
typedef struct {
	int64_t systemTime;  //U MICROSECUNDAMA
	char time[MAX_TIME];
	Kalman1D kalman;
	int majorID;
	int minorID;
	int  rssi;
	int  refpower;
	uint8_t proximity_uuid[16];
	uint8_t isStanding;
	} Tag;


#define MAX_TAGS 100
static volatile Tag g_tag[MAX_TAGS];

static volatile uint8_t saved_uuids[MAX_TAGS][16];
static volatile uint8_t num_reg_uuid;


static volatile int  num_g_tag = 0;

static void addTag(const char* time, int majorID, int minorID, int rssi, int refpower, uint8_t *uuid);
extern void parseTags(const char* content);
extern void checkIfTagIsForRemoval(void);
static void printTags(void);
static void printTag(int i);
extern void AddTagTask(void* parms);
extern void RemoveTagTask(void* parms);
static void WriteTagToAFile(Tag tempTag, Topics topic);
extern int isUuidRegister(uint8_t *uuid);
#endif
