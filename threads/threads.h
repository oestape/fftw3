/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* $Id: threads.h,v 1.1 2002-08-29 05:44:33 stevenj Exp $ */

#ifndef __THREADS_H__
#define __THREADS_H__

#include "ifftw.h"
#include "dft.h"
#include "rdft.h"

typedef struct {
     uint min, max, thr_num;
     void *data;
} spawn_data;

typedef void *(*spawn_function) (spawn_data *);

void X(spawn_loop)(uint loopmax, uint nthreads,
		   spawn_function proc, void *data);
int X(threads_init)(void);

/* configurations */

void X(dft_thr_vrank_geq1_register)(planner *p);
void X(rdft_thr_vrank_geq1_register)(planner *p);
void X(rdft2_thr_vrank_geq1_register)(planner *p);

solver *X(mksolver_dft_ct_dit_thr)(kdft_dit codelet, const ct_desc *desc);
solver *X(mksolver_rdft_hc2hc_dit_thr)(khc2hc codelet, const hc2hc_desc *desc);
solver *X(mksolver_rdft_hc2hc_dif_thr)(khc2hc codelet, const hc2hc_desc *desc);

void X(threads_conf_standard)(planner *p);

#endif /* __THREADS_H__ */
