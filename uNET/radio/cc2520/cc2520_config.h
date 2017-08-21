/*
 * cc2520_config.h
 *
 *  Created on: Mar 7, 2017
 *      Author: user
 */

#ifndef UNET_RADIO_CC2520_BACKUP_CC2520_CONFIG_H_
#define UNET_RADIO_CC2520_BACKUP_CC2520_CONFIG_H_


#include "hardware.h"


#ifndef BV
#define BV(x) (1 << x)
#endif


/*
 * SPI bus - CC2520 pin configuration.
 */
#define CC2520_CONF_SYMBOL_LOOP_COUNT 2604      /* 326us msp430X @ 16MHz */
/*each cycle run at 63ns, so it's intended that each instruction runs in 2 cycles - ~128ns */
//#define IN "IN"
//#define OUT "OUT"

/* P1.6 - Input: FIFOP from CC2520 */
#define CC2520_FIFOP_PORT(type)    P1##type
#define CC2520_FIFOP_PIN           6
/* P1.5 - Input: FIFO from CC2520 */
#define CC2520_FIFO_PORT(type)     P1##type
#define CC2520_FIFO_PIN            5
/* P1.7 - Input: CCA from CC2520 */
#define CC2520_CCA_PORT(type)      P1##type
#define CC2520_CCA_PIN             7
/* P2.0 - Input:  SFD from CC2520 */
#define CC2520_SFD_PORT(type)      P2##type
#define CC2520_SFD_PIN             0
/* P3.0 - Output: SPI Chip Select (CS_N) */
#define CC2520_CSN_PORT(type)      P3##type
#define CC2520_CSN_PIN             0
/* P4.3 - Output: VREG_EN to CC2520 */
#define CC2520_VREG_PORT(type)     P4##type
#define CC2520_VREG_PIN            3
/* P4.4 - Output: RESET_N to CC2520 */
#define CC2520_RESET_PORT(type)    P4##type
#define CC2520_RESET_PIN           4

#define CC2520_IRQ_VECTOR PORT1_VECTOR

/* Pin status.CC2520 */
#define CC2520_FIFOP_IS_1 (!!(CC2520_FIFOP_PORT(IN) & BV(CC2520_FIFOP_PIN)))
#define CC2520_FIFO_IS_1  (!!(CC2520_FIFO_PORT(IN) & BV(CC2520_FIFO_PIN)))
#define CC2520_CCA_IS_1   (!!(CC2520_CCA_PORT(IN) & BV(CC2520_CCA_PIN)))
#define CC2520_SFD_IS_1   (!!(CC2520_SFD_PORT(IN) & BV(CC2520_SFD_PIN)))

/* The CC2520 reset pin. */
#define SET_RESET_INACTIVE()   (CC2520_RESET_PORT(OUT) |=  BV(CC2520_RESET_PIN))
#define SET_RESET_ACTIVE()     (CC2520_RESET_PORT(OUT) &= ~BV(CC2520_RESET_PIN))

/* CC2520 voltage regulator enable pin. */
#define SET_VREG_ACTIVE()       (CC2520_VREG_PORT(OUT) |=  BV(CC2520_VREG_PIN))
#define SET_VREG_INACTIVE()     (CC2520_VREG_PORT(OUT) &= ~BV(CC2520_VREG_PIN))

/* CC2520 rising edge trigger for external interrupt 0 (FIFOP). */
#define CC2520_FIFOP_INT_INIT() do {                  \
    CC2520_FIFOP_PORT(IES) &= ~BV(CC2520_FIFOP_PIN);  \
    CC2520_CLEAR_FIFOP_INT();                         \
  } while(0)

/* CC2520 falling edge trigger for external (SFD). */
/* On TX SFD goes to 1 at the start of TX and goes to 0
 * at the end of TX */
#define CC2520_SFD_INT_INIT() do {                  \
    CC2520_SFD_PORT(IES) |= BV(CC2520_SFD_PIN);  \
    CC2520_CLEAR_SFD_INT();                         \
  } while(0)


/* FIFOP on external interrupt 0. */
/* FIFOP on external interrupt 0. */
#define CC2520_ENABLE_FIFOP_INT()          do { P1IE |= BV(CC2520_FIFOP_PIN); } while (0)
#define CC2520_DISABLE_FIFOP_INT()         do { P1IE &= ~BV(CC2520_FIFOP_PIN); } while (0)
#define CC2520_CLEAR_FIFOP_INT()           do { P1IFG &= ~BV(CC2520_FIFOP_PIN); } while (0)


/* SFD external pin interrupt */
#define CC2520_ENABLE_SFD_INT()          do { P2IE |= BV(CC2520_SFD_PIN); } while (0)
#define CC2520_DISABLE_SFD_INT()         do { P2IE &= ~BV(CC2520_SFD_PIN); } while (0)
#define CC2520_CLEAR_SFD_INT()           do { P2IFG &= ~BV(CC2520_SFD_PIN); } while (0)


/*
 * Enables/disables CC2520 access to the SPI bus (not the bus).
 * (Chip Select)
 */

 /* ENABLE CSn (active low) */
extern void clock_delay(unsigned int i);
#define CC2520_SPI_ENABLE()     do{ UCB0CTL1 &= ~UCSWRST;  clock_delay(5); P3OUT &= ~BIT0;clock_delay(5);}while(0)
 /* DISABLE CSn (active low) */
#define CC2520_SPI_DISABLE()    do{clock_delay(5);UCB0CTL1 |= UCSWRST;clock_delay(1); P3OUT |= BIT0;clock_delay(5);}while(0)
#define CC2520_SPI_IS_ENABLED() ((CC2520_CSN_PORT(OUT) & BV(CC2520_CSN_PIN)) != BV(CC2520_CSN_PIN))


#endif /* UNET_RADIO_CC2520_BACKUP_CC2520_CONFIG_H_ */
