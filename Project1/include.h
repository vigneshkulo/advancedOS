/*
 *  Submitted By    : Vignesh Kulothungan
 *  Subject         : Advanced Operating Systems - Project 1
 *  Module          : include.h
 */

/* -------- Shared Static Definitions Start -------- */
#define SUCCESS                 1
#define FAILURE                 -1
#define MAX_SEND                -2
#define MAX_RECEIVE             -3

#define SEND                    1
#define RECEIVE                 2
#define EXIT                    3
/* -------- Shared Static Definitions End -------- */

/* -------- Shared Data Structures Start -------- */
typedef struct
{
        int type;
        int msgId;
        int rcvId;
        int sendId;
        int timeStamp;
        int propTimeStamp;
}strMsg;

typedef struct
{
        int type;
        int sendMsgNum;
        strMsg recvMsg;
} strPipeMsg;
/* -------- Shared Data Structures End -------- */

