#pragma once

#include <vector>
#include <unordered_map>
#include "expression.hpp"
#include "statement.hpp"
#include "declaration.hpp"
#include "type.hpp"
#include "token.hpp"
#include "scope.hpp"


struct Semantics {

	Semantics()
	{
		sem_scope = new scope(global);
	}

	scope* sem_scope;

	decl* new_program(std::vector<decl*> dec_seq);
	decl* new_var_decl(token* tok, type* t);
	decl* new_var_def(decl* var_decl, expr* e);
	decl* new_const_decl(token* tok, type* t);
	decl* new_const_def(decl* var_decl, expr* e);
	decl* new_val_decl(token* tok, type* t);
	decl* new_val_def(decl* var_decl, expr* e);
	decl* new_func_decl(token* tok, std::vector<decl*> params, type* t);
	decl* new_func_def(decl* func_decl, stmt* func_body);
	decl* new_param(token* param, type* t);

	stmt* new_block_stmt(std::vector<stmt*> stmt_seq);
	stmt* new_if_stmt(expr* condition, stmt* if_true, stmt* if_false);
	stmt* new_while_stmt(expr* condition, stmt* statement);
	stmt* new_break_stmt();
	stmt* new_continue_stmt();
	stmt* new_return_stmt(expr* expression);
	stmt* new_decl_stmt(decl* declaration);
	stmt* new_expr_stmt(expr* expression);

	expr* new_boolean_literal(token* tok);
	expr* new_integer_literal(token* tok);
	expr* new_float_literal(token* tok);
	expr* new_char_literal(token* tok);
	expr* new_string_literal(token* tok);
	expr* new_identifier(token* tok);
	expr* new_postfix_expr(expr* expression, std::vector<expr*> args);
	expr* new_unary_expr(token_name unary_op, expr* expression);
	expr* new_cast_expr(expr* cast_expr, type* ts);
	expr* new_mul_expr(token_name mul_op, expr* lhs, expr* rhs);
	expr* new_add_expr(token_name add_op, expr* lhs, expr* rhs);
	expr* new_shift_expr(token_name shift_op, expr* lhs, expr* rhs);
	expr* new_rel_expr(token_name rel_op, expr* lhs, expr* rhs);
	expr* new_eq_expr(token_name eq_op, expr* lhs, expr* rhs);
	expr* new_bw_and_expr(expr* lhs, expr* rhs);
	expr* new_bw_xor_expr(expr* lhs, expr* rhs);
	expr* new_bw_or_expr(expr* lhs, expr* rhs);
	expr* new_log_and_expr(expr* lhs, expr* rhs);
	expr* new_log_or_expr(expr* lhs, expr* rhs);
	expr* new_cond_expr(expr* expr1, expr* expr2, expr* expr3);
	expr* new_assign_expr(expr* lhs, expr* rhs);

	type* new_void_type();
	type* new_bool_type();
	type* new_int_type();
	type* new_float_type();
	type* new_char_type();
	type* new_string_type();
	type* new_func_type(std::vector<type*> params, type_t ret_type);
	type* new_ptr_type(type* ptr_to);
	type* new_ref_type(type* ref_to);

	void new_global_scope();;
	void new_param_scope();
	void new_block_scope();
	void exit_current_scope();

	// Semantic helper functions 
	decl* lookup(std::string id);

	type* common_type_of(expr* e1, expr* e2);
	type* verify_conversion(expr* e, type* t);
	type* val_conv(expr* e);

	expr* basic_to_bool(expr* e);
	expr* to_int(expr* e);
	expr* to_float(expr*e);

	void assert_type(expr* e, type_t t);
	void assert_reference(expr* e);
	void assert_same_type(expr* e1, expr* e2);
	void assert_arithmetic(expr* e);
	void assert_int(expr* e);
	void assert_scalar(expr* e);

	bool is_scalar(expr* e);
	bool is_reference(expr* e);
	bool is_same_type(expr* e1, expr* e2);
	bool is_type(expr* e, type* t);
	bool is_arithmetic(expr* e);
	bool is_int(expr* e);
	bool val_conv_is(expr* e, type* t);

};