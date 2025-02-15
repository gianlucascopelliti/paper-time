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
#include "encl_t.h"
#include <stdint.h>

uint64_t rdtsc(void)
{
    uint64_t a, d;
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    a = (d << 32) | a;
    return a;
}

void enclave_ecall(uint64_t *tsc1, uint64_t *tsc2)
{
    *tsc1 = rdtsc();

    /* Just an ocall to transfer explicitly out of the enclave, this could also
     * be done with a timer interrupt using SGX-Step. */
    test_ocall();

    *tsc2 = rdtsc();
}
