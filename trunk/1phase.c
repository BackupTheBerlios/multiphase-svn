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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "misc.h"
#include "parser.h"

static int N;
/* uniform grid */
static long double DX;
/* uniform timesteps */
static long double DT;
/* number of timesteps */
static long NT;

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
		.type = FLOAT,
	}, {
		.name = "DT",
		.value = &DT,
		.type = FLOAT,
	}, {
		.name = "NT",
		.value = &NT,
		.type = INTEGER,
	}, {
		.name = "K",
		.value = &K,
		.type = FLOAT,
	}, {
		.name = "phi",
		.value = &phi,
		.type = FLOAT,
	}, {
		.name = "mu",
		.value = &mu,
		.type = FLOAT,
	}, {
		.name = "c",
		.value = &c,
		.type = FLOAT,
	}, {
		.name = "p_x0",
		.type = ARRAY,
	}, {
		.name = "p_lt",
		.type = ARRAY,
	}, {
		.name = "p_rt",
		.type = ARRAY,
	}, {
	}
};

/* Initial conditions */
static long double *p_x0_data;

static long double p_x0(int i)
{
	if (i < 0 || i > N - 1)
		die("%d not in [%d, %d]", i, 0, N - 1);
	return p_x0_data[i];
}

/* Left boundary condition */
static long double *p_lt_data;
static int p_lt_N;

static long double p_lt(int t)
{
	if (t < 0)
		die("%d not in [0, \\infty)", t);
	/* Last value in input remains forever. */
	if (t >= p_lt_N)
		return p_lt_data[p_lt_N - 1];
	return p_lt_data[t];
}

/* Right boundary condition */
static long double *p_rt_data;
static int p_rt_N;

static long double p_rt(int t)
{
	if (t < 0)
		die("%d not in [0, \\infty)", t);
	/* Last value in input remains forever. */
	if (t >= p_rt_N)
		return p_rt_data[p_rt_N - 1];
	return p_rt_data[t];
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
	long double *p;
	/* p(x, t + dt) */
	long double *p1;
	int i;

	struct token *head;
	struct param *p_x0_param, *p_lt_param, *p_rt_param;

	head = tokenize();
	get_values(params, head);
	free_tokens(head);

	p_x0_param = find_param("p_x0", params);
	p_x0_data = p_x0_param->value;
	N = p_x0_param->nr_elements;

	p_lt_param = find_param("p_lt", params);
	p_lt_data = p_lt_param->value;
	p_lt_N = p_lt_param->nr_elements;

	p_rt_param = find_param("p_rt", params);
	p_rt_data = p_rt_param->value;
	p_rt_N = p_rt_param->nr_elements;

	if (N > SIZE_MAX / sizeof(*p))
		die("N = %u > %zu", N, SIZE_MAX / sizeof(*p));
	p = xmalloc(N * sizeof(*p));
	p1 = xmalloc(N * sizeof(*p1));

	for (i = 0; i < N; i++)
		if (p_x0_data[i] < 0.0)
			die("p_x0[%u] = %Lf < 0.0", i, p_x0_data[i]);
	if (DX <= 0.0)
		die("DX = %Lf <= 0.0", DX);
	if (DT <= 0.0)
		die("DT = %Lf <= 0.0", DT);
	if (NT <= 0)
		die("NT = %ld <= 0", NT);
	if (K <= 0.0)
		die("K = %Lf <= 0.0", K);
	if (phi <= 0.0 || phi > 1.0)
		die("phi = %Lf not in (0.0, 1.0]", phi);
	if (mu <= 0.0)
		die("mu = %Lf <= 0.0", mu);
	if (c <= 0.0)
		die("c = %Lf <= 0.0", c);
	if (DT / (DX * DX) > 1 / (2 * (K / (phi * mu * c))))
		die("finite difference scheme isn't numerically stable: "
			"\\frac{\\Delta t}{(\\Delta t)^2} = %Lf > "
			"\\frac{1}{2 \\frac{K}{\\phi \\mu \\c}} = %Lf",
			DT / (DX * DX), 1 / (2 * (K / (phi * mu * c))));

	for (i = 0; i < N; i++)
		p[i] = p_x0(i);
	print_array(p);

	for (t = 1; t < NT; t++) {
		p1[0] = p_lt(t);
		for (i = 1; i <= N - 2; i++)
			p1[i] = p[i] + DT * K / (phi * mu * c) * d2f_dx2(i, p);
		p1[N - 1] = p_rt(t);

		for (i = 0; i < N; i++)
			p[i] = p1[i];
		print_array(p);
	}

	free_arrays(params);
	free(p);
	free(p1);

	return EXIT_SUCCESS;
}
