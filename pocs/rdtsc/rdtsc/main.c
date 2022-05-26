/*
 *  This file is part of the SGX-Step enclave execution control framework.
 *
 *  Copyright (C) 2017 Jo Van Bulck <jo.vanbulck@cs.kuleuven.be>,
 *                     Raoul Strackx <raoul.strackx@cs.kuleuven.be>
 *
 *  SGX-Step is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SGX-Step is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SGX-Step. If not, see <http://www.gnu.org/licenses/>.
 */

#include <sgx_urts.h>
#include "Enclave/encl_u.h"
#include <sys/mman.h>
#include <signal.h>
#include "libsgxstep/enclave.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/cpu.h"
#include "libsgxstep/sched.h"
#include "libsgxstep/config.h"
#include "libsgxstep/idt.h"

#define DBG_ENCL                1
#define IA32_TIME_STAMP_COUNTER 0x10

int fault_fired;

void fault_handler(int signo, siginfo_t * si, void  *ctx)
{
    ASSERT( fault_fired < 5);

    switch ( signo )
    {
      case SIGSEGV:
        info("Caught page fault (base address=%p)", si->si_addr);
        break;

      default:
        info("Caught unknown signal '%d'", signo);
        abort();
    }

    fault_fired++;
}

void attacker_config_page_table(void)
{
    struct sigaction act, old_act;

    /* Specify #PF handler with signinfo arguments */
    memset(&act, sizeof(sigaction), 0);
    act.sa_sigaction = fault_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO;

    /* Block all signals while the signal is being handled */
    sigfillset(&act.sa_mask);
    ASSERT(!sigaction( SIGSEGV, &act, &old_act ));
}

int do_attack = 0;

void test_ocall(void)
{
    info("hi from test_ocall; do_attack=%d\n", do_attack);
    if (do_attack)
    {
        wrmsr_on_cpu(IA32_TIME_STAMP_COUNTER, VICTIM_CPU, 100);
    }
}

int main( int argc, char **argv )
{
    int updated;
    sgx_enclave_id_t eid;
    uint64_t tsc1 = 0, tsc2 = 0;

    info("Creating enclave...");
    SGX_ASSERT( sgx_create_enclave( "./Enclave/encl.so", /*debug=*/DBG_ENCL,
                                NULL, &updated, &eid, NULL ) );

    claim_cpu(VICTIM_CPU);

    for (do_attack = 0; do_attack < 2; do_attack++)
    {
        info_event("calling enclave with do_attack=%d", do_attack);
        SGX_ASSERT( enclave_ecall(eid, &tsc1, &tsc2) );
        info("enclave measured TSC1=%llu; TSC2=%llu; diff=%llu\n", tsc1, tsc2, tsc2-tsc1);
    }

    info("all is well; exiting..");
    SGX_ASSERT( sgx_destroy_enclave( eid ) );
    return 0;
}
