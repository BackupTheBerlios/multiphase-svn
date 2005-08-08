/* 1-phase flow simulator.
   Copyright (C) 2005 Alexey Dobriyan <adobriyan@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <stdio.h>
#include <stdlib.h>

#include "misc.h"

#define N 10
/* uniform grid */
#define DX 20.0
/* uniform timesteps */
#define DT 0.05

#define print_array(a) __print_array(#a, a)

static void __print_array(char *s, long double *p)
{
	int i;

	for (i = 0; i < N; i++)
		printf("%s[%u] = %s(%f) = %Lf\n", s, i, s, i * DX, p[i]);
}

/* permeability */
#define K 50.0
/* porosity */
#define phi 0.15
/* viscosity */
#define mu 0.7
/* compressibility */
#define c 1.4

/* Initial conditions */
static long double p_x0(int i)
{
	if (i < 0 || i > N - 1)
		die("%d not in [%d, %d]", i, 0, N - 1);
	if (i == 0)
		return 105.0;
	if (i == N - 1)
		return 90.0;
	return 100.0;
}

/* Left boundary condition */
static long double p_lt(int t)
{
	if (t < 0)
		die("%d not in [0, \\infty)", t);
	if (t < 10)
		return 105.0;
	return 90.0;
}

/* Right boundary condition */
static long double p_rt(int t)
{
	if (t < 0)
		die("%d not in [0, \\infty)", t);
	if (t < 10)
		return 90.0;
	return 105.0;
}

static long double d2f_dx2(int i, long double *f)
{
	if (i < 1 || i > N - 2)
		die("%d not in [%u, %u]", i, 1, N - 2);
	return (f[i - 1] - 2 * f[i] + f[i + 1]) / (DX * DX);
}

int main(void)
{
	/* timestep */
	int t;
	/* current pressure p(x, t) */
	long double p[N];
	int i;

	for (i = 0; i < N; i++)
		p[i] = p_x0(i);
	print_array(p);

	t = 0;
	for (t = 0; t < 20; t++) {
		/* p(x, t + dt) */
		long double p1[N];

		p1[0] = p_lt(t);
		for (i = 1; i <= N - 2; i++)
			p1[i] = p[i] + DT * K / (phi * mu * c) * d2f_dx2(i, p);
		p1[N - 1] = p_rt(t);

		for (i = 0; i < N; i++)
			p[i] = p1[i];
		print_array(p);
	}

	return EXIT_SUCCESS;
}
