/* Input parser.
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

#ifndef __PARSER_H__
#define __PARSER_H__

struct token {
	struct token *next;
	char *string;
};

struct token * tokenize(void);
void free_tokens(struct token *token);

enum param_type {
	FLOAT,
	INTEGER,
	ARRAY,
};

struct param {
	char *name;
	void *value;
	int found;
	enum param_type type;
	/* used only by ARRAY */
	int nr_elements;
};

void get_values(struct param *params, struct token *token);
void free_arrays(struct param *params);
struct param * find_param(char *name, struct param *params);

#endif /* ndef __PARSER_H__ */
