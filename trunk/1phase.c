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

static struct param params[] = {
	{
		.name = "DX",
		.value = &DX,
		.type = NUMBER,
	}, {
		.name = "DT",
		.value = &DT,
		.type = NUMBER,
	}, {
		.name = "K",
		.value = &K,
		.type = NUMBER,
	}, {
		.name = "phi",
		.value = &phi,
		.type = NUMBER,
	}, {
		.name = "mu",
		.value = &mu,
		.type = NUMBER,
	}, {
		.name = "c",
		.value = &c,
		.type = NUMBER,
	}, {
	},
};

/* Initial conditions */
static long double p_x0(int i)
{
	if (i < 0 || i > N - 1)
		die("%s: %d not in [%d, %d]", __func__, i, 0, N - 1);
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
		die("%s: %d not in [0, \\infty)", __func__, t);
	if (t < 10)
		return 105.0;
	return 90.0;
}

/* Right boundary condition */
static long double p_rt(int t)
{
	if (t < 0)
		die("%s: %d not in [0, \\infty)", __func__, t);
	if (t < 10)
		return 90.0;
	return 105.0;
}

static long double d2f_dx2(int i, long double *f)
{
	if (i < 1 || i > N - 2)
		die("%s: %d not in [%u, %u]", __func__, i, 1, N - 2);
	return (f[i - 1] - 2 * f[i] + f[i + 1]) / (DX * DX);
}

int main(void)
{
	/* timestep */
	int t;
	/* current pressure p(x, t) */
	long double p[N];
	int i;

	struct token *head;

	head = tokenize();
	get_values(head, params);
	free_tokens(head);

	if (DX <= 0.0)
		die("%s: DX = %Lf <= 0.0", __func__, DX);
	if (DT <= 0.0)
		die("%s: DT = %Lf <= 0.0", __func__, DT);
	if (K <= 0.0)
		die("%s: K = %Lf <= 0.0", __func__, K);
	if (phi <= 0.0 || phi > 1.0)
		die("%s: phi = %Lf not in (0.0, 1.0]", __func__, phi);
	if (mu <= 0.0)
		die("%s: mu = %Lf <= 0.0", __func__, mu);
	if (c <= 0.0)
		die("%s: c = %Lf <= 0.0", __func__, c);

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
