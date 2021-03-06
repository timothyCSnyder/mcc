#include "parser.hpp"

Parser::Parser(const std::vector<token*>& token_stream)
{
	std::vector<token*>::const_iterator it;

	it = token_stream.begin();

	while (it != token_stream.end())
	{
		tokens.push_back(*it);
		it++;
	}

	first = tokens.begin();
	last = tokens.end();
	
}

token_name Parser::lookahead() {

	if(first != last)
		return (*first)->getType();

	return tok_eof;
}

token_name Parser::lookahead(int n)
{
	std::vector<token*>::const_iterator it{ first };

	for (it; it != first + n; ++it);

	return (*it)->getType();
}

token* Parser::accept()
{
		assert(first != last);
		token* current = *first;
		++first;
		return current;
}

token* Parser::accept_if(token_name tok)
{
	assert(first != last);
	if (tok != (*first)->getType())
		std::cout << '\n\n' << tok << " != " << (*first)->getType() << '\n\n';
	assert(tok == (*first)->getType());
	token* current = *first;
	++first;
	return current;
}

inline void Parser::match(token_name tok)
{
	if ((*first)->getType() == tok)
	{
		accept();
		return;
	}
	throw std::runtime_error("Invalid match");
}

inline void Parser::match_if(token_name tok)
{
	if ((*first)->getType() == tok)
	{
		accept();
	}
	throw std::runtime_error("Invalid match");
}

/*	TYPE PARSING	*/

type* Parser::parse_basic_type()
{

	switch (lookahead())
	{
		case tok_ts_void:
			accept();
			return semantics.new_void_type();
		case tok_ts_bool:
			accept();
			return semantics.new_bool_type();
		case tok_ts_int:
			accept();
			return semantics.new_int_type();
		case tok_ts_float:
			accept();
			return semantics.new_float_type();
		case tok_ts_char: 
			accept();
			return semantics.new_char_type();
	}

	throw std::runtime_error("Expected basic-type");
}

/*
token* Parser::match_if_postfix_type()
{
	switch (lookahead())
	{
		case tok_mul:
		{
			return accept();
		}
		case tok_kw_const:
		{
			return accept();
		}
		case tok_kw_volatile:
		{
			return accept();
		}
		case tok_left_bracket:
		{
			match(tok_left_bracket);
			if (lookahead() != tok_right_brace)
			{
				expr* e = parse_expr();
			}
			match(tok_right_brace);
			return accept();
		}
	}

	return nullptr;
}
*/

type* Parser::parse_postfix_type()
{
	/*
	type* basic_type = parse_basic_type();
	while (match_if_postfix_type())
	{
		continue;
	}
	*/
	
	switch (lookahead(1))
	{
		case tok_mul:
		{
			accept();
			return nullptr;
		}
		case tok_kw_const:
		case tok_kw_volatile:
		{
			accept();
			return nullptr;
		}

		case tok_left_bracket:
			match(tok_left_bracket);
			if (lookahead() != tok_right_brace)
			{
				expr* e = parse_expr();
			}
			match(tok_right_brace);
			return nullptr;

		default:
		{
			return parse_basic_type();
		}
	}
	

	throw std::runtime_error("Expected postfix type");
}


type* Parser::parse_reference_type()
{
	type* post_type = parse_postfix_type();
	while (lookahead() == tok_bitw_and)
	{
		accept();
	}

	return post_type;
}

type* Parser::parse_type()
{
	type* ref_type = parse_reference_type();

	return ref_type;
}

/*	EXPRESSION PARSING	*/

expr* Parser::parse_primary_expr()
{
	switch(lookahead())
	{
		case tok_binary_integer:
		case tok_decimal_integer:
		case tok_hexadecimal_integer:
			return semantics.new_integer_literal(accept());
		case tok_boolean:
			return semantics.new_boolean_literal(accept());
		case tok_floating_point:
			return semantics.new_float_literal(accept());
		case tok_character:
			return semantics.new_char_literal(accept());
		case tok_string:
			return semantics.new_string_literal(accept());
		case tok_identifier:
		{
			return semantics.new_identifier(accept());
		}

		case tok_left_paren:
			match(tok_left_paren);
			expr* e = parse_expr();
			match(tok_right_paren);
			return e;
	}

	throw std::runtime_error("Expected primary expression");
}

token*Parser::match_if_postfix_expr()
{
	switch (lookahead()) 
	{
		case tok_left_paren:
		case tok_left_bracket:
			return accept();
	}

	return nullptr;
}

expr* Parser::parse_postfix_expr()
{
	token* name = *first;
	expr* e = parse_primary_expr();

	while (match_if_postfix_expr())
	{
		std::vector<expr*> arg_list;
		arg_list = parse_arg_list();
		accept(); 
		e = semantics.new_postfix_expr(e, arg_list);
	}

	return e;
}

token*Parser::match_if_arg_list()
{
	if (lookahead() == tok_comma)
	{
		return accept();
	}

	return nullptr;
}

std::vector<expr*> Parser::parse_arg_list()
{
	std::vector<expr*> arg_list;
	expr* arg = parse_arg();
	arg_list.push_back(arg);

	while (match_if_arg_list())
	{
		expr* arg_more = parse_arg();
		arg_list.push_back(arg_more);
	}

	return arg_list;
}


expr* Parser::parse_arg()
{
	return parse_expr();
}

expr* Parser::parse_unary_expr()
{
	switch (lookahead())
	{
		case tok_add:
		case tok_sub:
		case tok_bitw_not:
		case tok_logical_not:
		case tok_logical_and:
		case tok_mul:
		{
			accept();
			expr* unary_expr = parse_unary_expr();
			return semantics.new_unary_expr(lookahead(), unary_expr);
		}
		default:
		{
			return parse_postfix_expr();
		}
	}
}

expr* Parser::parse_cast_expr()
{
	expr* unary_expr = parse_unary_expr();
	if (lookahead() == tok_kw_as) 
	{
		accept();
		type* t = parse_type();
		return semantics.new_cast_expr(unary_expr, t);
	}

	return unary_expr;
}

token* Parser::match_if_mul_expr()
{
	switch (lookahead()) 
	{
		case tok_mul:
		case tok_div:
		case tok_rem:
			return accept();
	}

	return nullptr;
}

expr* Parser::parse_mul_expr()
{
	expr* cast_expr = parse_cast_expr();

	while (token* tok = match_if_mul_expr())
	{
		expr* cast_expr_more = parse_cast_expr();
		cast_expr = semantics.new_mul_expr(tok->getType(), cast_expr, cast_expr_more);
	}

	return cast_expr;
}

token*Parser::match_if_add_expr()
{
	switch (lookahead()) {
		case tok_add:
		case tok_sub:
			return accept();
		default:
			return nullptr;
	}
}

expr* Parser::parse_add_expr()
{
	expr* mul_expr = parse_mul_expr();

	while (token* tok = match_if_add_expr())
	{
		expr* mul_expr_more = parse_mul_expr();
		mul_expr = semantics.new_add_expr(tok->getType(), mul_expr, mul_expr_more);
	}

	return mul_expr;
}

token*Parser::match_if_shift_expr()
{
	switch (lookahead()) 
	{
		case tok_shift_left:
		case tok_shift_right:
			return accept();
	}

	return nullptr;
}

expr* Parser::parse_shift_expr()
{
	expr* add_expr = parse_add_expr();
	while (token* tok = match_if_shift_expr())
	{
		expr* add_expr_more = parse_add_expr();
		add_expr = semantics.new_shift_expr(tok->getType(), add_expr, add_expr_more);
	}

	return add_expr;
}

token*Parser::match_if_rel_expr()
{
	switch (lookahead()) 
	{
		case tok_rel_lt:
		case tok_rel_gt:
		case tok_rel_le:
		case tok_rel_ge:
			return accept();
	}

	return nullptr;
}

expr* Parser::parse_rel_expr()
{
	expr* shift_expr = parse_shift_expr();

	while (token* tok = match_if_rel_expr())
	{
		expr* shift_expr_more = parse_shift_expr();
		shift_expr = semantics.new_rel_expr(tok->getType(), shift_expr, shift_expr_more);
	}

	return shift_expr;
}

token*Parser::match_if_eq_expr()
{
	switch (lookahead()) 
	{
		case tok_rel_eq:
		case tok_rel_neq:
			return accept();
	}

	return nullptr;
}

expr* Parser::parse_eq_expr()
{
	expr* rel_expr = parse_rel_expr();

	while (token* tok = match_if_eq_expr())
	{
		expr* rel_expr_more = parse_rel_expr();
		rel_expr = semantics.new_eq_expr(tok->getType(), rel_expr, rel_expr_more);
	}

	return rel_expr;
}

token*Parser::match_if_bw_and_expr()
{
	if (lookahead() == tok_bitw_and)
	{
		return accept();
	}

	return nullptr;
}

expr* Parser::parse_bw_and_expr()
{
	expr* eq_expr = parse_eq_expr();

	while (match_if_bw_and_expr())
	{
		expr* eq_expr_more = parse_eq_expr();
		eq_expr = semantics.new_bw_and_expr(eq_expr, eq_expr_more);
	}

	return eq_expr;
}


token*Parser::match_if_bw_xor_expr()
{
	if (lookahead() == tok_bitw_xor) 
	{
		return accept();
	}

	return nullptr;
}

expr* Parser::parse_bw_xor_expr()
{
	expr* bw_and_expr = parse_bw_and_expr();

	while (match_if_bw_xor_expr())
	{
		expr* bw_and_expr_more = parse_bw_and_expr();
		// This naming scheme was a mistake ..................................
		bw_and_expr = semantics.new_bw_xor_expr(bw_and_expr, bw_and_expr_more);
	}

	return bw_and_expr;
}

token*Parser::match_if_bw_or_expr()
{
	if (lookahead() == tok_bitw_or) 
	{
		return accept();
	}

	return nullptr;
}

expr* Parser::parse_bw_or_expr()
{
	expr* bw_xor_expr = parse_bw_xor_expr();

	while (match_if_bw_or_expr())
	{
		expr* bw_xor_expr_more = parse_bw_xor_expr();
		bw_xor_expr = semantics.new_bw_or_expr(bw_xor_expr, bw_xor_expr_more);
	}

	return bw_xor_expr;
}

token*Parser::match_if_logical_and_expr()
{
	if (lookahead() == tok_logical_and) 
	{
		return accept();
	}

	return nullptr;
}

expr* Parser::parse_logical_and_expr()
{
	expr* bw_or_expr = parse_bw_or_expr();

	while (match_if_logical_and_expr())
	{
		expr* bw_or_expr_more = parse_bw_or_expr();
		bw_or_expr = semantics.new_log_and_expr(bw_or_expr, bw_or_expr_more);
	}

	return bw_or_expr;
}

token*Parser::match_if_logical_or_expr()
{
	if (lookahead() == tok_logical_or) 
	{
		return accept();
	}

	return nullptr;
}

expr* Parser::parse_logical_or_expr()
{
	expr* log_and_expr = parse_logical_and_expr();

	while (match_if_logical_or_expr())
	{
		expr* log_and_expr_more = parse_logical_and_expr();
		log_and_expr = semantics.new_log_or_expr(log_and_expr, log_and_expr_more);
	}

	return log_and_expr;
}

/*%%%%%%%%%%%%%%%%%%%*/
expr* Parser::parse_conditional_expr()
{
	expr* log_or_expr = parse_logical_or_expr();

	if (lookahead() == tok_conditional_operator)
	{
		accept();
		expr* e = parse_expr();
		match(tok_colon);
		expr* cond_expr = parse_conditional_expr();

		log_or_expr = semantics.new_cond_expr(log_or_expr, e, cond_expr);
	}

	return log_or_expr;
}

expr* Parser::parse_assign_expr()
{
	expr* cond_expr = parse_conditional_expr();

	if (lookahead() == tok_assignment_operator)
	{
		accept();
		expr* assign_expr = parse_assign_expr(); 

		cond_expr = semantics.new_assign_expr(cond_expr, assign_expr);
	}

	return cond_expr;
}

expr* Parser::parse_expr()
{
	return parse_assign_expr();
}

expr* Parser::parse_const_expr()
{
	return parse_conditional_expr();
}

/*	STATEMENT PARSING	*/

stmt* Parser::parse_stmt()
{
	switch (lookahead()) 
	{
		case tok_left_brace:
		{
			return parse_block_stmt();
		}
		case tok_kw_if:
		{
			return parse_if_stmt();
		}
		case tok_kw_while:
		{
			return parse_while_stmt();
		}
		case tok_kw_break:
		{
			return parse_break_stmt();
		}
		case tok_kw_continue:
		{
			return parse_continue_stmt();
		}
		case tok_kw_return:
		{
			return parse_return_stmt();
		}
		case tok_kw_def:
		case tok_kw_let:
		case tok_kw_var:
		{
			return parse_decl_stmt();
		}
	}

	return parse_expr_stmt();

}

stmt* Parser::parse_block_stmt()
{
	match(tok_left_brace);
	semantics.new_block_scope();
	std::vector<stmt*> stmt_seq = parse_stmt_seq();
	match(tok_right_brace);
	semantics.exit_current_scope();
	return semantics.new_block_stmt(stmt_seq);
}


/* This may be buggy vvvv*/
token*Parser::is_stmt()
{
	switch (lookahead())
	{
		case tok_kw_if:
		case tok_left_bracket:
		case tok_kw_while:
		case tok_kw_break:
		case tok_kw_continue:
		case tok_kw_return:
		case tok_kw_def:
		case tok_kw_let:
		case tok_kw_var:
		case tok_identifier:
		case tok_decimal_integer:
		case tok_binary_integer:
		case tok_hexadecimal_integer:
		case tok_character:
		case tok_boolean:
			return *first;
	}

	return nullptr;
}

std::vector<stmt*> Parser::parse_stmt_seq()
{
	std::vector<stmt*> stmt_seq;

	stmt* s = parse_stmt();
	stmt_seq.push_back(s);

	while (is_stmt())
	{
		stmt* s_more = parse_stmt();
		stmt_seq.push_back(s_more);
	}

	return stmt_seq;
}

stmt* Parser::parse_if_stmt()
{
	match(tok_kw_if);
	match(tok_left_paren);

	expr* e = parse_expr();

	match(tok_right_paren);

	stmt* s = parse_stmt();

	stmt* s_more = nullptr;
	if (lookahead() == tok_kw_else) 
	{
		match(tok_kw_else);
		s_more = parse_stmt();
	}


	return semantics.new_if_stmt(e, s, s_more);
}

stmt* Parser::parse_while_stmt()
{
	match(tok_kw_while);
	match(tok_left_paren);

	expr* e = parse_expr();

	match(tok_right_paren);

	stmt* s = parse_stmt();

	return semantics.new_while_stmt(e, s);
}

stmt* Parser::parse_break_stmt()
{
	match(tok_kw_break);
	match(tok_semicolon);

	return semantics.new_break_stmt();
}

stmt* Parser::parse_continue_stmt()
{
	match(tok_kw_continue);
	match(tok_semicolon);

	return semantics.new_continue_stmt();
}

stmt* Parser::parse_return_stmt()
{
	match(tok_kw_return);
	if (lookahead() == tok_semicolon)
	{
		accept();
		return new stmt;
	}
	expr* e = parse_expr();
	match(tok_semicolon);

	return semantics.new_return_stmt(e);
}

stmt* Parser::parse_decl_stmt()
{
	decl* local_dec = parse_local_decl();


	return semantics.new_decl_stmt(local_dec);
}

stmt* Parser::parse_expr_stmt()
{
	expr* e = parse_expr();
	match(tok_semicolon);

	return semantics.new_expr_stmt(e);
}

/*	DECLARATION PARSING	*/

std::vector<decl*>* Parser::parse_program()
{
	std::vector<decl*>* program = &parse_decl_seq();
	
	return program;
}

std::vector<decl*> Parser::parse_decl_seq()
{
	std::vector<decl*> decl_list;

	while(first < last)
	{ 
		decl* d = parse_decl();
		decl_list.push_back(d);
	}

	return decl_list;
}

decl* Parser::parse_decl()
{
	switch (lookahead()) 
	{
		case tok_kw_def: 
		{
			token_name la = lookahead(2);
			if (la == tok_colon)
			{
				return parse_obj_def();
			}
			else if (la == tok_left_paren)
			{
				return parse_func_def();
			}
		}
		case tok_kw_let:
		case tok_kw_var:
			return parse_obj_def();
	}

	throw std::runtime_error("Expected declaration");
}

decl* Parser::parse_local_decl()
{
	return parse_obj_def();
}

decl* Parser::parse_obj_def()
{
	switch (lookahead())
	{
		case tok_kw_def:
		{
			return parse_val_def();
		}
		case tok_kw_let:
		{
			return parse_const_def();
		}
		case tok_kw_var:
		{
			return parse_var_def();
		}
	}

	throw std::runtime_error("Expected onject definition");
}

decl* Parser::parse_var_def()
{
	match(tok_kw_var);
	token* var_id = accept_if(tok_identifier);
	match(tok_colon);
	type* t = parse_type();
	decl* var_decl = semantics.new_var_decl(var_id, t);
	match(tok_assignment_operator);
	expr* e = parse_expr();
	match(tok_semicolon);

	return semantics.new_var_def(var_decl, e);

	/*
	switch (lookahead()) 
	{
		case tok_semicolon:
		{
			match(tok_semicolon);

			return new decl;
		}
		case tok_assignment_operator:
		{
			match(tok_assignment_operator);
			expr* e = parse_expr();
			match(tok_semicolon);

			return semantics.new_var_def(var_decl, e);
		}
	}
	*/

	//throw std::runtime_error("Expected variable definition");
}

decl* Parser::parse_const_def()
{
	match(tok_kw_let);
	token* const_id = accept_if(tok_identifier);
	match(tok_colon);
	type* t = parse_type();
	match(tok_assignment_operator);
	expr* e = parse_expr();
	match(tok_semicolon);

	decl* const_decl = semantics.new_const_decl(const_id, t);

	return semantics.new_const_def(const_decl, e);
}

decl* Parser::parse_val_def()
{
	match(tok_kw_def);
	token* val_id = accept_if(tok_identifier);
	match(tok_colon);
	type* t = parse_type();
	match(tok_assignment_operator);
	expr* e = parse_expr();
	match(tok_semicolon);

	decl* val_decl = semantics.new_val_decl(val_id, t);

	return semantics.new_val_def(val_decl, e);
}

decl* Parser::parse_func_def()
{
	match(tok_kw_def);
	token* func_id = accept_if(tok_identifier);
	match(tok_left_paren);
	std::vector<decl*> param_list;
	semantics.new_param_scope();
	if (lookahead() != tok_right_paren)
	{
		param_list = parse_param_list();
	}
	match(tok_right_paren);
	match(tok_sub);
	match(tok_rel_gt);
	semantics.exit_current_scope();

	type* t = parse_type();
	decl* func_decl = semantics.new_func_decl(func_id, param_list, t);
	stmt* block_stmt = parse_block_stmt();

	return semantics.new_func_def(func_decl, block_stmt);
}

token* Parser::match_if_param_list()
{
	if (lookahead() == tok_comma) 
	{
		return accept();
	}

	return nullptr;
}

std::vector<decl*> Parser::parse_param_list()
{
	std::vector<decl*> param_list;

	decl* param = parse_param();
	param_list.push_back(param);

	while (match_if_param_list())
	{
		decl* param_more = parse_param();
		param_list.push_back(param_more);
	}

	return param_list;
}

decl* Parser::parse_param()
{
	//match(tok_identifier);
	token* param = accept_if(tok_identifier);
	match(tok_colon);
	type* t = parse_type();

	return semantics.new_param(param, t);
}