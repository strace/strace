/*
 * Copyright (c) 2021 Srikavin Ramkumar <srikavinramkumar@gmail.com>
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef SYMBOLS_H
# define SYMBOLS_H

# include "ast.h"

/*
 * Returns a error string if the given type is
 * invalid. Otherwise, returns NULL if the type is
 * valid.
 */
char *
resolve_type(struct ast_type *out, char *name,
	     struct ast_type_option_list *options);

/*
 * Returns NULL if successfully added a symbol.
 * If the symbol is already defined, returns the
 * source node for the previous definition.
 */
struct ast_node *
symbol_add(char *name, struct ast_node *source);

/*
 * Gets the definition of a previously added symbol.
 * Returns NULL if symbol is not stored.
 */
struct ast_node *
symbol_get(char *name);

#endif //SYMBOLS_H
