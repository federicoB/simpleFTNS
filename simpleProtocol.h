/* 
 * File:   simpleProtocol.h
 * Author: Riccardo Maffei
 *
 * Created on 16 novembre 2016, 0.38
 */

#ifndef SIMPLEPROTOCOL_H
#define SIMPLEPROTOCOL_H

#include <stdint.h>

/**
 * The simple test protocol packet.
 * Fixed size 64bit.
 */
typedef uint64_t SimpleProtocolPacket;

#define SPPINIT(X) X = 0x0000000000000000
#define GETSYN(X) ((X >> 63) & 0x0000000000000001)
#define SETSYN(X) X = (X | 0x8000000000000000)
#define GETSET(X) ((X >> 62) & 0x0000000000000001)
#define SETSET(X) X = (X | 0x4000000000000000)
#define GETINC(X) ((X >> 61) & 0x0000000000000001)
#define SETINC(X) X = (X | 0x2000000000000000)
#define GETGET(X) ((X >> 60) & 0x0000000000000001)
#define SETGET(X) X = (X | 0x1000000000000000)
#define GETFIN(X) ((X >> 59) & 0x0000000000000001)
#define SETFIN(X) X = (X | 0x0800000000000000)
#define GETERR(X) ((X >> 58) & 0x0000000000000001)
#define SETERR(X) X = (X | 0x0400000000000000)
#define GETSUC(X) ((X >> 57) & 0x0000000000000001)
#define SETSUC(X) X = (X | 0x0200000000000000)
#define GETVAR(X) (X & 0x01ffffffffffffff) >> 32
#define SETVAR(X, Y) X = (X | ((((uint64_t) Y) & 0x0000000001ffffff) << 32)) //NOTE: the following SET macros work only if the field is pre-zeroed
#define GETVAL(X) (X & 0x00000000ffffffff)
#define SETVAL(X, Y) X = (X | (((uint64_t) Y) & 0x00000000ffffffff))
#define GETERC(X) (X & 0x01ffffffffffffff) >> 32
#define SETERC(X, Y) X = (X | ((((uint64_t) Y) & 0x0000000001ffffff) << 32))
#define GETTKN(X) (X & 0x01ffffffffffffff) >> 25
#define SETTKN(X, Y) X = (X | ((((uint64_t) Y) & 0x00000000ffffffff) << 25))
#define SPPTONTW(X) htobe64(X)
#define SPPTOHST(X) be64toh(X)



#endif /* SIMPLEPROTOCOL_H */

