/*-
 * Copyright (c) 2015-2017 Ruslan Bukin <br@bsdpad.com>
 * All rights reserved.
 *
 * Portions of this software were developed by SRI International and the
 * University of Cambridge Computer Laboratory under DARPA/AFRL contract
 * FA8750-10-C-0237 ("CTSRD"), as part of the DARPA CRASH research programme.
 *
 * Portions of this software were developed by the University of Cambridge
 * Computer Laboratory as part of the CTSRD Project, with support from the
 * UK Higher Education Innovation Fund (HEIF).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * RISC-V Timer
 */

#if 0
#include "opt_platform.h"
#endif

#include <sys/cdefs.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/timetc.h>

#include <sys/proc.h>

#include <machine/bus.h>
#include <machine/cpu.h>
#include <machine/cpufunc.h>
#include <machine/intr.h>
#include <machine/asm.h>
#include <machine/trap.h>
#include <machine/fdt.h>
#include <machine/sbi.h>
#include "riscv_cpu_intc.h"

#include <dev/ofw/openfirm.h>

#define	TIMER_COUNTS		0x00
#define	TIMER_MTIMECMP(cpu)	(cpu * 8)

struct riscv_timer_pcpu_softc {
	uint64_t 		pc_nexttickevent;
	uint64_t 		pc_nextstatevent;
	u_int32_t		pc_ticks_err_sum;
};

struct riscv_timer_softc {
	struct device		 sc_dev;
	int			 sc_node;

	struct riscv_timer_pcpu_softc sc_pstat[MAXCPUS];

	u_int32_t		sc_ticks_err_cnt;
	u_int32_t		sc_ticks_per_second; // sc_clkfreq
	u_int32_t		sc_ticks_per_intr;
	u_int32_t		sc_statvar;
	u_int32_t		sc_statmin;

	void			*sc_ih;
};

static struct riscv_timer_softc *riscv_timer_sc = NULL;

int		riscv_timer_match(struct device *, void *, void *);
void		riscv_timer_attach(struct device *, struct device *, void *);

unsigned	riscv_timer_get_timecount(struct timecounter *);
int		riscv_timer_get_timebase();
int		riscv_timer_intr(void *);
void		riscv_timer_initclocks();
void		riscv_timer_delay(u_int);
void		riscv_timer_setstatclockrate(int);
void		riscv_timer_start();

static struct timecounter riscv_timer_timecount = {
	.tc_name           = "RISC-V Timecounter",
	.tc_get_timecount  = riscv_timer_get_timecount,
	.tc_poll_pps       = NULL,
	.tc_counter_mask   = ~0u,
	.tc_frequency      = 0,
	.tc_quality        = 1000,
};

struct cfattach timer_ca = {
	sizeof (struct riscv_timer_softc), riscv_timer_match, riscv_timer_attach
};

struct cfdriver timer_cd = {
	NULL, "timer", DV_DULL
};

int
riscv_timer_match(struct device *parent, void *cfdata, void *aux)
{
	if (riscv_timer_sc)	//already attached
		return 0;

	int node;
	// struct fdt_attach_args *fa = (struct fdt_attach_args *)aux;

	/*
	 * return 1 if:
	 * we can find valid "timebase-frequency" property from cpus
	 */
	if ( (node = OF_finddevice("/cpus")) == 0)
		return 0;

	return (OF_getproplen(node, "timebase-frequency") == 4);//32bit uint
}

void
riscv_timer_attach(struct device *parent, struct device *self, void *aux)
{
	struct riscv_timer_softc *sc = (struct riscv_timer_softc *)self;
	// int error; // Unused

#if 0	// XXX Not necessary?
	sc = device_get_softc(dev);
	if (riscv_timer_sc)
		return (ENXIO);

	if (device_get_unit(dev) != 0)
		return (ENXIO);
#endif

	sc->sc_ticks_per_second = riscv_timer_get_timebase();
	if (sc->sc_ticks_per_second == 0) {
		printf("Failed to resolve RISC-V Timer timebase\n");
		return;
	}

	riscv_timer_sc = sc;

	/* Setup IRQs handler */
	riscv_intc_intr_establish(IRQ_TIMER_SUPERVISOR, 0, 
			riscv_timer_intr, sc, "riscv_timer");

	riscv_clock_register(riscv_timer_initclocks, riscv_timer_delay,
	    riscv_timer_setstatclockrate, riscv_timer_start);

	riscv_timer_timecount.tc_frequency = sc->sc_ticks_per_second;
	riscv_timer_timecount.tc_priv = sc;
	tc_init(&riscv_timer_timecount);
}

int
riscv_timer_intr(void *arg)
{
	struct riscv_timer_softc *sc;
#ifdef	DEBUG_TIMER
	printf("RISC-V Timer Interrupt\n");
#endif
	sc = (struct riscv_timer_softc *)arg;

	csr_clear(sip, SIP_STIP);

#if 0	// Not relevant? FreeBSD specific?
	if (sc->et.et_active)
		sc->et.et_event_cb(&sc->et, sc->et.et_arg);
#endif

	return (0); // Handled
}

static inline uint64_t
get_cycles()
{
	return (rdtime());
}

long
get_counts(struct riscv_timer_softc *sc)
{
	uint64_t counts;

	counts = get_cycles();

	return (counts);
}

unsigned
riscv_timer_get_timecount(struct timecounter *tc)
{
	struct riscv_timer_softc *sc;

	sc = tc->tc_priv;

	return (get_counts(sc));
}

void
riscv_timer_initclocks()
{
	struct riscv_timer_softc	*sc = timer_cd.cd_devs[0];
	struct riscv_timer_pcpu_softc	*pc =
		&sc->sc_pstat[CPU_INFO_UNIT(curcpu())];
	uint64_t			 next;

	stathz = hz;
	profhz = hz * 10;

#if 0	// XXX Not necessary? RISC-V timer always runs at constant frequency
	if (sc->sc_clkfreq != riscv_timer_frequency) {
		riscv_timer_set_clockrate(agtimer_frequency);
	}
#endif

	riscv_timer_setstatclockrate(stathz);

	sc->sc_ticks_per_intr = sc->sc_ticks_per_second / hz;
	sc->sc_ticks_err_cnt = sc->sc_ticks_per_second % hz;
	pc->pc_ticks_err_sum = 0;

	/* configure virtual timer interupt */
	sc->sc_ih = riscv_intr_establish_fdt_idx(sc->sc_node, 2,
	    IPL_CLOCK|IPL_MPSAFE, riscv_timer_intr, NULL, "tick");

	next = get_cycles() + sc->sc_ticks_per_intr;
	pc->pc_nexttickevent = pc->pc_nextstatevent = next;

	sbi_set_timer(next);
	csr_set(sie, SIE_STIE);
	// Supervisor Interrupt Enabled is OFF at this point
	// Can set SSTATUS_SIE to ON below to supervisor timer interrupt loop
	// XXX Figure out where to enable Supervisor Interrupts
	// csr_set(sstatus, SSTATUS_SIE);
}

void
riscv_timer_setstatclockrate(int newhz)
{

// XXX TODO
}

void
riscv_timer_start()
{
	struct riscv_timer_softc	*sc = timer_cd.cd_devs[0];
	struct riscv_timer_pcpu_softc	*pc =
		&sc->sc_pstat[CPU_INFO_UNIT(curcpu())];
	uint64_t nextevent;

	nextevent = get_cycles() + sc->sc_ticks_per_intr;
	pc->pc_nexttickevent = pc->pc_nextstatevent = nextevent;

	riscv_intr_route(sc->sc_ih, 1, curcpu());

	sbi_set_timer(nextevent);
	csr_set(sie, SIE_STIE);
}

int
riscv_timer_get_timebase()
{
	int node, len;

	node = OF_finddevice("/cpus");
	if (node == -1) {
		printf("Can't find cpus node.\n");
		return (0);
	}

	len = OF_getproplen(node, "timebase-frequency");
	if (len != 4) {
		printf("Can't find timebase-frequency property.\n");
		return (0);
	}

	return OF_getpropint(node, "timebase-frequency", 0);
}

// XXX TODO
void
riscv_timer_delay(u_int usec)
{
	int64_t counts, counts_per_usec;
	uint64_t first, last;

	/*
	 * Check the timers are setup, if not just
	 * use a for loop for the meantime
	 */
	if (riscv_timer_sc == NULL) {
		for (; usec > 0; usec--)
			for (counts = 200; counts > 0; counts--)
				/*
				 * Prevent the compiler from optimizing
				 * out the loop
				 */
				cpufunc_nullop();
		return;
	}

#if 0	// XXX FreeBSD Specific?
	TSENTER();
#endif

	/* Get the number of times to count */
	counts_per_usec = ((riscv_timer_timecount.tc_frequency / 1000000) + 1);

	/*
	 * Clamp the timeout at a maximum value (about 32 seconds with
	 * a 66MHz clock). *Nobody* should be delay()ing for anywhere
	 * near that length of time and if they are, they should be hung
	 * out to dry.
	 */
	if (usec >= (0x80000000U / counts_per_usec))
		counts = (0x80000000U / counts_per_usec) - 1;
	else
		counts = usec * counts_per_usec;

	first = get_counts(riscv_timer_sc);

	while (counts > 0) {
		last = get_counts(riscv_timer_sc);
		counts -= (int64_t)(last - first);
		first = last;
	}

#if 0	// FreeBSD Specific?
	TSEXIT();
#endif
}
