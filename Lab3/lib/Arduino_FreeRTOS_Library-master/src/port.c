/*
 * FreeRTOS Kernel V11.0.1
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */


#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "Arduino_FreeRTOS.h"
#include "task.h"

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the AVR port.
 *----------------------------------------------------------*/

/* Start tasks with interrupts enabled. */
#define portFLAGS_INT_ENABLED    ( ( StackType_t ) 0x80 )

#if defined( portUSE_WDTO )
    #define portSCHEDULER_ISR    WDT_vect
#else
//STR
#if defined(portUSE_TIMER1)
    #define portSCHEDULER_ISR    TIMER1_COMPA_vect
#endif
//STR
    #warning "The user must define a Timer to be used for the Scheduler."
#endif

/*-----------------------------------------------------------*/

/* We require the address of the pxCurrentTCB variable, but don't want to know
 * any details of its type. */
typedef void TCB_t;
extern volatile TCB_t * volatile pxCurrentTCB;

/*-----------------------------------------------------------*/

/**
 *  Enable the watchdog timer, configuring it for expire after
 *  (value) timeout (which is a combination of the WDP0
 *  through WDP3 bits).
 *
 *  This function is derived from <avr/wdt.h> but enables only
 *  the interrupt bit (WDIE), rather than the reset bit (WDE).
 *
 *  Can't find it documented but the WDT, once enabled,
 *  rolls over and fires a new interrupt each time.
 *
 *  See also the symbolic constants WDTO_15MS et al.
 *
 *  Updated to match avr-libc 2.0.0
 */

#if defined( portUSE_WDTO )

static __inline__
__attribute__ ((__always_inline__))
void wdt_interrupt_enable (const uint8_t value)
{
    if (_SFR_IO_REG_P (_WD_CONTROL_REG))
    {
        __asm__ __volatile__ (
                "in __tmp_reg__,__SREG__"   "\n\t"
                "cli"                       "\n\t"
                "wdr"                       "\n\t"
                "out %0, %1"                "\n\t"
                "out __SREG__,__tmp_reg__"  "\n\t"
                "out %0, %2"                "\n\t"
                : /* no outputs */
                : "I" (_SFR_IO_ADDR(_WD_CONTROL_REG)),
                "r" ((uint8_t)(_BV(_WD_CHANGE_BIT) | _BV(WDE))),
                "r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) |
                        _BV(WDIF) | _BV(WDIE) | (value & 0x07)) )
                : "r0"
        );
    }
    else
    {
        __asm__ __volatile__ (
                "in __tmp_reg__,__SREG__"   "\n\t"
                "cli"                       "\n\t"
                "wdr"                       "\n\t"
                "sts %0, %1"                "\n\t"
                "out __SREG__,__tmp_reg__"  "\n\t"
                "sts %0, %2"                "\n\t"
                : /* no outputs */
                : "n" (_SFR_MEM_ADDR(_WD_CONTROL_REG)),
                "r" ((uint8_t)(_BV(_WD_CHANGE_BIT) | _BV(WDE))),
                "r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) |
                        _BV(WDIF) | _BV(WDIE) | (value & 0x07)) )
                : "r0"
        );
    }
}
#endif

/*-----------------------------------------------------------*/
/**
 *  Enable the watchdog timer, configuring it for expire after
 *  (value) timeout (which is a combination of the WDP0
 *  through WDP3 bits).
 *
 *  This function is derived from <avr/wdt.h> but enables both
 *  the reset bit (WDE), and the interrupt bit (WDIE).
 *
 *  This will ensure that if the interrupt is not serviced
 *  before the second timeout, the AVR will reset.
 *
 *  Servicing the interrupt automatically clears it,
 *  and ensures the AVR does not reset.
 *
 *  Can't find it documented but the WDT, once enabled,
 *  rolls over and fires a new interrupt each time.
 *
 *  See also the symbolic constants WDTO_15MS et al.
 *
 *  Updated to match avr-libc 2.0.0
 */

#if defined( portUSE_WDTO )

static __inline__
__attribute__ ((__always_inline__))
void wdt_interrupt_reset_enable (const uint8_t value)
{
    if (_SFR_IO_REG_P (_WD_CONTROL_REG))
    {
        __asm__ __volatile__ (
                "in __tmp_reg__,__SREG__"   "\n\t"
                "cli"                       "\n\t"
                "wdr"                       "\n\t"
                "out %0, %1"                "\n\t"
                "out __SREG__,__tmp_reg__"  "\n\t"
                "out %0, %2"                "\n\t"
                : /* no outputs */
                : "I" (_SFR_IO_ADDR(_WD_CONTROL_REG)),
                "r" ((uint8_t)(_BV(_WD_CHANGE_BIT) | _BV(WDE))),
                "r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) |
                        _BV(WDIF) | _BV(WDIE) | _BV(WDE) | (value & 0x07)) )
                : "r0"
        );
    }
    else
    {
        __asm__ __volatile__ (
                "in __tmp_reg__,__SREG__"   "\n\t"
                "cli"                       "\n\t"
                "wdr"                       "\n\t"
                "sts %0, %1"                "\n\t"
                "out __SREG__,__tmp_reg__"  "\n\t"
                "sts %0, %2"                "\n\t"
                : /* no outputs */
                : "n" (_SFR_MEM_ADDR(_WD_CONTROL_REG)),
                "r" ((uint8_t)(_BV(_WD_CHANGE_BIT) | _BV(WDE))),
                "r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) |
                        _BV(WDIF) | _BV(WDIE) | _BV(WDE) | (value & 0x07)) )
                : "r0"
        );
    }
}
#endif

/*-----------------------------------------------------------*/
/* actual number of ticks per second, after configuration. Not for RTC, which has 1 tick/second. */
TickType_t portTickRateHz;

/* remaining ticks in each second, decremented to enable the system_tick. Not for RTC, which has 1 tick/second. */
volatile TickType_t ticksRemainingInSec;

/*-----------------------------------------------------------*/

/*
 * Macro to save all the general purpose registers, the save the stack pointer
 * into the TCB.
 *
 * The first thing we do is save the flags then disable interrupts. This is to
 * guard our stack against having a context switch interrupt after we have already
 * pushed the registers onto the stack - causing the 32 registers to be on the
 * stack twice.
 *
 * r1 is set to zero (__zero_reg__) as the compiler expects it to be thus, however
 * some of the math routines make use of R1.
 *
 * r0 is set to __tmp_reg__ as the compiler expects it to be thus.
 *
 * #if defined(__AVR_HAVE_RAMPZ__)
 * #define __RAMPZ__ 0x3B
 * #endif
 *
 * #if defined(__AVR_3_BYTE_PC__)
 * #define __EIND__ 0x3C
 * #endif
 *
 * The interrupts will have been disabled during the call to portSAVE_CONTEXT()
 * so we need not worry about reading/writing to the stack pointer.
 */
#if defined(__AVR_3_BYTE_PC__) && defined(__AVR_HAVE_RAMPZ__)
/* 3-Byte PC Save  with RAMPZ */
#define portSAVE_CONTEXT()                                                              \
        __asm__ __volatile__ (  "push   __tmp_reg__                             \n\t"   \
                                "in     __tmp_reg__, __SREG__                   \n\t"   \
                                "cli                                            \n\t"   \
                                "push   __tmp_reg__                             \n\t"   \
                                "in     __tmp_reg__, 0x3B                       \n\t"   \
                                "push   __tmp_reg__                             \n\t"   \
                                "in     __tmp_reg__, 0x3C                       \n\t"   \
                                "push   __tmp_reg__                             \n\t"   \
                                "push   __zero_reg__                            \n\t"   \
                                "clr    __zero_reg__                            \n\t"   \
                                "push   r2                                      \n\t"   \
                                "push   r3                                      \n\t"   \
                                "push   r4                                      \n\t"   \
                                "push   r5                                      \n\t"   \
                                "push   r6                                      \n\t"   \
                                "push   r7                                      \n\t"   \
                                "push   r8                                      \n\t"   \
                                "push   r9                                      \n\t"   \
                                "push   r10                                     \n\t"   \
                                "push   r11                                     \n\t"   \
                                "push   r12                                     \n\t"   \
                                "push   r13                                     \n\t"   \
                                "push   r14                                     \n\t"   \
                                "push   r15                                     \n\t"   \
                                "push   r16                                     \n\t"   \
                                "push   r17                                     \n\t"   \
                                "push   r18                                     \n\t"   \
                                "push   r19                                     \n\t"   \
                                "push   r20                                     \n\t"   \
                                "push   r21                                     \n\t"   \
                                "push   r22                                     \n\t"   \
                                "push   r23                                     \n\t"   \
                                "push   r24                                     \n\t"   \
                                "push   r25                                     \n\t"   \
                                "push   r26                                     \n\t"   \
                                "push   r27                                     \n\t"   \
                                "push   r28                                     \n\t"   \
                                "push   r29                                     \n\t"   \
                                "push   r30                                     \n\t"   \
                                "push   r31                                     \n\t"   \
                                "lds    r26, pxCurrentTCB                       \n\t"   \
                                "lds    r27, pxCurrentTCB + 1                   \n\t"   \
                                "in     __tmp_reg__, __SP_L__                   \n\t"   \
                                "st     x+, __tmp_reg__                         \n\t"   \
                                "in     __tmp_reg__, __SP_H__                   \n\t"   \
                                "st     x+, __tmp_reg__                         \n\t"   \
                             );
#elif defined(__AVR_HAVE_RAMPZ__)
/* 2-Byte PC Save  with RAMPZ */
#define portSAVE_CONTEXT()                                                              \
        __asm__ __volatile__ (  "push   __tmp_reg__                             \n\t"   \
                                "in     __tmp_reg__, __SREG__                   \n\t"   \
                                "cli                                            \n\t"   \
                                "push   __tmp_reg__                             \n\t"   \
                                "in     __tmp_reg__, 0x3B                       \n\t"   \
                                "push   __tmp_reg__                             \n\t"   \
                                "push   __zero_reg__                            \n\t"   \
                                "clr    __zero_reg__                            \n\t"   \
                                "push   r2                                      \n\t"   \
                                "push   r3                                      \n\t"   \
                                "push   r4                                      \n\t"   \
                                "push   r5                                      \n\t"   \
                                "push   r6                                      \n\t"   \
                                "push   r7                                      \n\t"   \
                                "push   r8                                      \n\t"   \
                                "push   r9                                      \n\t"   \
                                "push   r10                                     \n\t"   \
                                "push   r11                                     \n\t"   \
                                "push   r12                                     \n\t"   \
                                "push   r13                                     \n\t"   \
                                "push   r14                                     \n\t"   \
                                "push   r15                                     \n\t"   \
                                "push   r16                                     \n\t"   \
                                "push   r17                                     \n\t"   \
                                "push   r18                                     \n\t"   \
                                "push   r19                                     \n\t"   \
                                "push   r20                                     \n\t"   \
                                "push   r21                                     \n\t"   \
                                "push   r22                                     \n\t"   \
                                "push   r23                                     \n\t"   \
                                "push   r24                                     \n\t"   \
                                "push   r25                                     \n\t"   \
                                "push   r26                                     \n\t"   \
                                "push   r27                                     \n\t"   \
                                "push   r28                                     \n\t"   \
                                "push   r29                                     \n\t"   \
                                "push   r30                                     \n\t"   \
                                "push   r31                                     \n\t"   \
                                "lds    r26, pxCurrentTCB                       \n\t"   \
                                "lds    r27, pxCurrentTCB + 1                   \n\t"   \
                                "in     __tmp_reg__, __SP_L__                   \n\t"   \
                                "st     x+, __tmp_reg__                         \n\t"   \
                                "in     __tmp_reg__, __SP_H__                   \n\t"   \
                                "st     x+, __tmp_reg__                         \n\t"   \
                             );
#else
/* 2-Byte PC Save */
#define portSAVE_CONTEXT()                                                              \
        __asm__ __volatile__ (  "push   __tmp_reg__                             \n\t"   \
                                "in     __tmp_reg__, __SREG__                   \n\t"   \
                                "cli                                            \n\t"   \
                                "push   __tmp_reg__                             \n\t"   \
                                "push   __zero_reg__                            \n\t"   \
                                "clr    __zero_reg__                            \n\t"   \
                                "push   r2                                      \n\t"   \
                                "push   r3                                      \n\t"   \
                                "push   r4                                      \n\t"   \
                                "push   r5                                      \n\t"   \
                                "push   r6                                      \n\t"   \
                                "push   r7                                      \n\t"   \
                                "push   r8                                      \n\t"   \
                                "push   r9                                      \n\t"   \
                                "push   r10                                     \n\t"   \
                                "push   r11                                     \n\t"   \
                                "push   r12                                     \n\t"   \
                                "push   r13                                     \n\t"   \
                                "push   r14                                     \n\t"   \
                                "push   r15                                     \n\t"   \
                                "push   r16                                     \n\t"   \
                                "push   r17                                     \n\t"   \
                                "push   r18                                     \n\t"   \
                                "push   r19                                     \n\t"   \
                                "push   r20                                     \n\t"   \
                                "push   r21                                     \n\t"   \
                                "push   r22                                     \n\t"   \
                                "push   r23                                     \n\t"   \
                                "push   r24                                     \n\t"   \
                                "push   r25                                     \n\t"   \
                                "push   r26                                     \n\t"   \
                                "push   r27                                     \n\t"   \
                                "push   r28                                     \n\t"   \
                                "push   r29                                     \n\t"   \
                                "push   r30                                     \n\t"   \
                                "push   r31                                     \n\t"   \
                                "lds    r26, pxCurrentTCB                       \n\t"   \
                                "lds    r27, pxCurrentTCB + 1                   \n\t"   \
                                "in     __tmp_reg__, __SP_L__                   \n\t"   \
                                "st     x+, __tmp_reg__                         \n\t"   \
                                "in     __tmp_reg__, __SP_H__                   \n\t"   \
                                "st     x+, __tmp_reg__                         \n\t"   \
                             );
#endif

/*
 * Opposite to portSAVE_CONTEXT().  Interrupts will have been disabled during
 * the context save so we can write to the stack pointer.
 */
#if defined(__AVR_3_BYTE_PC__) && defined(__AVR_HAVE_RAMPZ__)
/* 3-Byte PC Restore with RAMPZ */
#define portRESTORE_CONTEXT()                                                           \
        __asm__ __volatile__ (  "lds    r26, pxCurrentTCB                       \n\t"   \
                                "lds    r27, pxCurrentTCB + 1                   \n\t"   \
                                "ld     r28, x+                                 \n\t"   \
                                "out    __SP_L__, r28                           \n\t"   \
                                "ld     r29, x+                                 \n\t"   \
                                "out    __SP_H__, r29                           \n\t"   \
                                "pop    r31                                     \n\t"   \
                                "pop    r30                                     \n\t"   \
                                "pop    r29                                     \n\t"   \
                                "pop    r28                                     \n\t"   \
                                "pop    r27                                     \n\t"   \
                                "pop    r26                                     \n\t"   \
                                "pop    r25                                     \n\t"   \
                                "pop    r24                                     \n\t"   \
                                "pop    r23                                     \n\t"   \
                                "pop    r22                                     \n\t"   \
                                "pop    r21                                     \n\t"   \
                                "pop    r20                                     \n\t"   \
                                "pop    r19                                     \n\t"   \
                                "pop    r18                                     \n\t"   \
                                "pop    r17                                     \n\t"   \
                                "pop    r16                                     \n\t"   \
                                "pop    r15                                     \n\t"   \
                                "pop    r14                                     \n\t"   \
                                "pop    r13                                     \n\t"   \
                                "pop    r12                                     \n\t"   \
                                "pop    r11                                     \n\t"   \
                                "pop    r10                                     \n\t"   \
                                "pop    r9                                      \n\t"   \
                                "pop    r8                                      \n\t"   \
                                "pop    r7                                      \n\t"   \
                                "pop    r6                                      \n\t"   \
                                "pop    r5                                      \n\t"   \
                                "pop    r4                                      \n\t"   \
                                "pop    r3                                      \n\t"   \
                                "pop    r2                                      \n\t"   \
                                "pop    __zero_reg__                            \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
                                "out    0x3C, __tmp_reg__                       \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
                                "out    0x3B, __tmp_reg__                       \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
                                "out    __SREG__, __tmp_reg__                   \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
                             );
#elif defined(__AVR_HAVE_RAMPZ__)
/* 2-Byte PC Restore with RAMPZ */
#define portRESTORE_CONTEXT()                                                           \
        __asm__ __volatile__ (  "lds    r26, pxCurrentTCB                       \n\t"   \
                                "lds    r27, pxCurrentTCB + 1                   \n\t"   \
                                "ld     r28, x+                                 \n\t"   \
                                "out    __SP_L__, r28                           \n\t"   \
                                "ld     r29, x+                                 \n\t"   \
                                "out    __SP_H__, r29                           \n\t"   \
                                "pop    r31                                     \n\t"   \
                                "pop    r30                                     \n\t"   \
                                "pop    r29                                     \n\t"   \
                                "pop    r28                                     \n\t"   \
                                "pop    r27                                     \n\t"   \
                                "pop    r26                                     \n\t"   \
                                "pop    r25                                     \n\t"   \
                                "pop    r24                                     \n\t"   \
                                "pop    r23                                     \n\t"   \
                                "pop    r22                                     \n\t"   \
                                "pop    r21                                     \n\t"   \
                                "pop    r20                                     \n\t"   \
                                "pop    r19                                     \n\t"   \
                                "pop    r18                                     \n\t"   \
                                "pop    r17                                     \n\t"   \
                                "pop    r16                                     \n\t"   \
                                "pop    r15                                     \n\t"   \
                                "pop    r14                                     \n\t"   \
                                "pop    r13                                     \n\t"   \
                                "pop    r12                                     \n\t"   \
                                "pop    r11                                     \n\t"   \
                                "pop    r10                                     \n\t"   \
                                "pop    r9                                      \n\t"   \
                                "pop    r8                                      \n\t"   \
                                "pop    r7                                      \n\t"   \
                                "pop    r6                                      \n\t"   \
                                "pop    r5                                      \n\t"   \
                                "pop    r4                                      \n\t"   \
                                "pop    r3                                      \n\t"   \
                                "pop    r2                                      \n\t"   \
                                "pop    __zero_reg__                            \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
                                "out    0x3B, __tmp_reg__                       \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
                                "out    __SREG__, __tmp_reg__                   \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
                             );
#else
/* 2-Byte PC Restore */
#define portRESTORE_CONTEXT()                                                           \
        __asm__ __volatile__ (  "lds    r26, pxCurrentTCB                       \n\t"   \
                                "lds    r27, pxCurrentTCB + 1                   \n\t"   \
                                "ld     r28, x+                                 \n\t"   \
                                "out    __SP_L__, r28                           \n\t"   \
                                "ld     r29, x+                                 \n\t"   \
                                "out    __SP_H__, r29                           \n\t"   \
                                "pop    r31                                     \n\t"   \
                                "pop    r30                                     \n\t"   \
                                "pop    r29                                     \n\t"   \
                                "pop    r28                                     \n\t"   \
                                "pop    r27                                     \n\t"   \
                                "pop    r26                                     \n\t"   \
                                "pop    r25                                     \n\t"   \
                                "pop    r24                                     \n\t"   \
                                "pop    r23                                     \n\t"   \
                                "pop    r22                                     \n\t"   \
                                "pop    r21                                     \n\t"   \
                                "pop    r20                                     \n\t"   \
                                "pop    r19                                     \n\t"   \
                                "pop    r18                                     \n\t"   \
                                "pop    r17                                     \n\t"   \
                                "pop    r16                                     \n\t"   \
                                "pop    r15                                     \n\t"   \
                                "pop    r14                                     \n\t"   \
                                "pop    r13                                     \n\t"   \
                                "pop    r12                                     \n\t"   \
                                "pop    r11                                     \n\t"   \
                                "pop    r10                                     \n\t"   \
                                "pop    r9                                      \n\t"   \
                                "pop    r8                                      \n\t"   \
                                "pop    r7                                      \n\t"   \
                                "pop    r6                                      \n\t"   \
                                "pop    r5                                      \n\t"   \
                                "pop    r4                                      \n\t"   \
                                "pop    r3                                      \n\t"   \
                                "pop    r2                                      \n\t"   \
                                "pop    __zero_reg__                            \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
                                "out    __SREG__, __tmp_reg__                   \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
                             );
#endif
/*-----------------------------------------------------------*/

/*
 * Perform hardware setup to enable ticks from relevant Timer.
 */
void prvSetupTimerInterrupt( void );
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
uint16_t usAddress;
    /* Simulate how the stack would look after a call to vPortYield() generated by
    the compiler. */

    /* The start of the task code will be popped off the stack last, so place
    it on first. */
    usAddress = ( uint16_t ) pxCode;
    *pxTopOfStack = ( StackType_t ) ( usAddress & ( uint16_t ) 0x00ff );
    pxTopOfStack--;

    usAddress >>= 8;
    *pxTopOfStack = ( StackType_t ) ( usAddress & ( uint16_t ) 0x00ff );
    pxTopOfStack--;

#if defined(__AVR_3_BYTE_PC__)
    /* The AVR ATmega2560/ATmega2561 have 256KBytes of program memory and a 17-bit
     * program counter. When a code address is stored on the stack, it takes 3 bytes
     * instead of 2 for the other ATmega* chips.
     *
     * Store 0 as the top byte since we force all task routines to the bottom 128K
     * of flash. We do this by using the .lowtext label in the linker script.
     *
     * In order to do this properly, we would need to get a full 3-byte pointer to
     * pxCode. That requires a change to GCC. Not likely to happen any time soon.
     */
    *pxTopOfStack = 0;
    pxTopOfStack--;
#endif

    /* Next simulate the stack as if after a call to portSAVE_CONTEXT().
     * portSAVE_CONTEXT places the flags on the stack immediately after r0
     * to ensure the interrupts get disabled as soon as possible, and so ensuring
     * the stack use is minimal should a context switch interrupt occur.
     */
    *pxTopOfStack = ( StackType_t ) 0x00; /* R0 */
    pxTopOfStack--;
    *pxTopOfStack = portFLAGS_INT_ENABLED;
    pxTopOfStack--;

#if defined(__AVR_3_BYTE_PC__)
    /* If we have an ATmega256x, we are also saving the EIND register.
     * We should default to 0.
     */
    *pxTopOfStack = ( StackType_t ) 0x00;    /* EIND */
    pxTopOfStack--;
#endif

#if defined(__AVR_HAVE_RAMPZ__)
    /* We are saving the RAMPZ register.
     * We should default to 0.
     */
    *pxTopOfStack = ( StackType_t ) 0x00;    /* RAMPZ */
    pxTopOfStack--;
#endif

    /* Now the remaining registers. The compiler expects R1 to be 0. */
    *pxTopOfStack = ( StackType_t ) 0x00;    /* R1 */

    /* Leave R2 - R23 untouched */
    pxTopOfStack -= 23;

    /* Place the parameter on the stack in the expected location. */
    usAddress = ( uint16_t ) pvParameters;
    *pxTopOfStack = ( StackType_t ) ( usAddress & ( uint16_t ) 0x00ff );
    pxTopOfStack--;

    usAddress >>= 8;
    *pxTopOfStack = ( StackType_t ) ( usAddress & ( uint16_t ) 0x00ff );

    /* Leave register R26 - R31 untouched */
    pxTopOfStack -= 7;

    return pxTopOfStack;
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
    /* Setup the relevant timer hardware to generate the tick. */
    prvSetupTimerInterrupt();

    /* Restore the context of the first task that is going to run. */
    portRESTORE_CONTEXT();

    /* Simulate a function call end as generated by the compiler. We will now
     * jump to the start of the task the context of which we have just restored. */
    __asm__ __volatile__ ( "ret" );

    /* Should not get here. */
    return pdTRUE;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* It is unlikely that the ATmega port will get stopped.  If required simply
     * disable the tick interrupt here. */

    wdt_disable();      /* disable Watchdog Timer */
}
/*-----------------------------------------------------------*/

/*
 * Choose which delay function to use.
 * Arduino delay() is a millisecond granularity busy wait, that
 * that breaks FreeRTOS. So its use is limited to less than one
 * System Tick (portTICK_PERIOD_MS milliseconds).
 * FreeRTOS vTaskDelay() is relies on the System Tick which here
 * has a granularity of portTICK_PERIOD_MS milliseconds (15ms),
 * with the remainder implemented as an Arduino delay().
 */

#ifdef delay
#undef delay
#endif

extern void delay ( unsigned long ms );

//STR
#if defined( portUSE_WDTO ) || defined( portUSE_TIMER1 )
//STR
void vPortDelay( const uint32_t ms ) __attribute__ ( ( hot, flatten ) );
void vPortDelay( const uint32_t ms )
{
    if ( ms < portTICK_PERIOD_MS )
    {
        delay( (unsigned long) (ms) );
    }
    else
    {
        vTaskDelay( (TickType_t) (ms / portTICK_PERIOD_MS) );
        delay( (unsigned long) ( (ms - portTICK_PERIOD_MS) % portTICK_PERIOD_MS ) );
    }
}
#else
#warning "The user is responsible to provide function `vPortDelay()`"
#warning "Arduino uses all AVR MCU Timers, so breakage may occur"
extern void vPortDelay( const uint32_t ms ) __attribute__ ( ( hot, flatten ) );
#endif
/*-----------------------------------------------------------*/

/*
 * Manual context switch. The first thing we do is save the registers so we
 * can use a naked attribute.
 */
void vPortYield( void ) __attribute__( ( hot, flatten, naked ) );
void vPortYield( void )
{
    portSAVE_CONTEXT();
    vTaskSwitchContext();
    portRESTORE_CONTEXT();

    __asm__ __volatile__ ( "ret" );
}
/*-----------------------------------------------------------*/

/*
 * Manual context switch callable from ISRs. The first thing we do is save
 * the registers so we can use a naked attribute.
 */
void vPortYieldFromISR( void ) __attribute__( ( hot, flatten, naked ) );
void vPortYieldFromISR( void )
{
    portSAVE_CONTEXT();
    vTaskSwitchContext();
    portRESTORE_CONTEXT();

    __asm__ __volatile__ ( "reti" );
}
/*-----------------------------------------------------------*/

/*
 * Context switch function used by the tick. This must be identical to
 * vPortYield() from the call to vTaskSwitchContext() onwards. The only
 * difference from vPortYield() is the tick count is incremented as the
 * call comes from the tick ISR.
 */
void vPortYieldFromTick( void ) __attribute__( ( hot, flatten, naked ) );
void vPortYieldFromTick( void )
{
    portSAVE_CONTEXT();
    sleep_reset();        /* reset the sleep_mode() faster than sleep_disable(); */
    if( xTaskIncrementTick() != pdFALSE )
    {
        vTaskSwitchContext();
    }

    portRESTORE_CONTEXT();

    __asm__ __volatile__ ( "ret" );
}
/*-----------------------------------------------------------*/

#if defined( portUSE_WDTO )

    /*
     * Setup WDT to generate a tick interrupt.
     */
    void prvSetupTimerInterrupt( void )
    {
        /* reset watchdog */
        wdt_reset();

        /* set up WDT Interrupt (rather than the WDT Reset). */
        wdt_interrupt_enable( portUSE_WDTO );
    }

// #else
//     #warning "The user is responsible to provide function `prvSetupTimerInterrupt()`"
//     extern void prvSetupTimerInterrupt( void );
#endif

//STR
#if defined( portUSE_TIMER1 )

    void prvSetupTimerInterrupt( void )
    {
        //From http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
        // TIMER 1 for interrupt frequency 100 Hz:
        cli(); // stop interrupts
        TCCR1A = 0; // set entire TCCR1A register to 0
        TCCR1B = 0; // same for TCCR1B
        TCNT1  = 0; // initialize counter value to 0
        // set compare match register for 100 Hz increments
        OCR1A = 19999; // = 16000000 / (8 * 100) - 1 (must be <65536)
        // turn on CTC mode
        TCCR1B |= (1 << WGM12);
        // Set CS12, CS11 and CS10 bits for 8 prescaler
        TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
        // enable timer compare interrupt
        TIMSK1 |= (1 << OCIE1A);
        sei(); // allow interrupts
    }

#else
    #warning "The user is responsible to provide function `prvSetupTimerInterrupt()`"
    extern void prvSetupTimerInterrupt( void );
#endif
//STR

/*-----------------------------------------------------------*/

#if configUSE_PREEMPTION == 1

    /*
     * Tick ISR for preemptive scheduler. We can use a naked attribute as
     * the context is saved at the start of vPortYieldFromTick(). The tick
     * count is incremented after the context is saved.
     *
     * use ISR_NOBLOCK where there is an important timer running, that should preempt the scheduler.
     *
     */
    ISR( portSCHEDULER_ISR, ISR_NAKED ) __attribute__ ( ( hot, flatten ) );

/*  ISR( portSCHEDULER_ISR, ISR_NAKED ISR_NOBLOCK ) __attribute__ ( ( hot, flatten ) );
 */
    ISR( portSCHEDULER_ISR )
    {
        vPortYieldFromTick();
        __asm__ __volatile__ ( "reti" );
    }
#else /* if configUSE_PREEMPTION == 1 */

    /*
     * Tick ISR for the cooperative scheduler. All this does is increment the
     * tick count. We don't need to switch context, this can only be done by
     * manual calls to taskYIELD();
     *
     * use ISR_NOBLOCK where there is an important timer running, that should preempt the scheduler.
     */
    ISR( portSCHEDULER_ISR ) __attribute__ ( ( hot, flatten ) );

/*  ISR( portSCHEDULER_ISR, ISR_NOBLOCK ) __attribute__ ( ( hot, flatten ) );
 */
    ISR( portSCHEDULER_ISR )
    {
        xTaskIncrementTick();
    }
#endif /* if configUSE_PREEMPTION == 1 */
