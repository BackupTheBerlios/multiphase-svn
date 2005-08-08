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
#include <string.h>

#include "misc.h"
#include "parser.h"

#define N 10
/* uniform grid */
static long double DX;
/* uniform timesteps */
static long double DT;

#define print_array(a) __print_array(#a, a)

static void __print_array(char *s, long double *p)
{
	int i;

	for (i = 0; i < N; i++)
		printf("%s[%u] = %s(%Lf) = %Lf\n", s, i, s, i * DX, p[i]);
}

/* permeability */
static long double K;
/* porosity */
static long double phi;
/* viscosity */
static long double mu;
/* compressibility */
static long double c;

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

	struct token *head, *cur;

	head = tokenize();
	cur = head;
	while (cur) {
		if (strcmp("DX", cur->string) == 0) {
			DX = strtold(cur->next->string, NULL);
			if (DX <= 0.0)
				die("DX = %Lf <= 0.0", DX);
		} else if (strcmp("DT", cur->string) == 0) {
			DT = strtold(cur->next->string, NULL);
			if (DT <= 0.0)
				die("DT = %Lf <= 0.0", DT);
		} else if (strcmp("K", cur->string) == 0) {
			K = strtold(cur->next->string, NULL);
			if (K <= 0.0)
				die("K = %Lf <= 0.0", K);
		} else if (strcmp("phi", cur->string) == 0) {
			phi = strtold(cur->next->string, NULL);
			if (phi <= 0.0 || phi > 1.0)
				die("phi = %Lf not in (0.0, 1.0]", phi);
		} else if (strcmp("mu", cur->string) == 0) {
			mu = strtold(cur->next->string, NULL);
			if (mu <= 0.0)
				die("mu = %Lf <= 0.0", mu);
		} else if (strcmp("c", cur->string) == 0) {
			c = strtold(cur->next->string, NULL);
			if (c <= 0.0)
				die("c = %Lf <= 0.0", c);
		}
		cur = cur->next;
	}
	free_tokens(head);

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
