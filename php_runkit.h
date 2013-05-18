/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2006 The PHP Group, (c) 2008-2012 Dmitry Zenovich |
  +----------------------------------------------------------------------+
  | This source file is subject to the new BSD license,                  |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.opensource.org/licenses/BSD-3-Clause                      |
  | If you did not receive a copy of the license and are unable to       |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Sara Golemon <pollita@php.net>                               |
  | Modified by Dmitry Zenovich <dzenovich@gmail.com>                    |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_RUNKIT_H
#define PHP_RUNKIT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || (PHP_MAJOR_VERSION >= 6)
#include "Zend/zend_closures.h"
#endif

#define PHP_RUNKIT_VERSION					"1.0.4-dev"
#define PHP_RUNKIT_SANDBOX_CLASSNAME		"Runkit_Sandbox"
#define PHP_RUNKIT_SANDBOX_PARENT_CLASSNAME	"Runkit_Sandbox_Parent"

#define PHP_RUNKIT_IMPORT_FUNCTIONS                         0x0001
#define PHP_RUNKIT_IMPORT_CLASS_METHODS                     0x0002
#define PHP_RUNKIT_IMPORT_CLASS_CONSTS                      0x0004
#define PHP_RUNKIT_IMPORT_CLASS_PROPS                       0x0008
#define PHP_RUNKIT_IMPORT_CLASS_STATIC_PROPS                0x0010
#define PHP_RUNKIT_IMPORT_CLASSES                           (PHP_RUNKIT_IMPORT_CLASS_METHODS|PHP_RUNKIT_IMPORT_CLASS_CONSTS|\
                                                             PHP_RUNKIT_IMPORT_CLASS_PROPS|PHP_RUNKIT_IMPORT_CLASS_STATIC_PROPS)
#define PHP_RUNKIT_IMPORT_OVERRIDE                          0x0020
#define PHP_RUNKIT_OVERRIDE_OBJECTS                         0x8000

#if ZEND_MODULE_API_NO > 20050922
#define ZEND_ENGINE_2_2
#endif
#if ZEND_MODULE_API_NO > 20050921
#define ZEND_ENGINE_2_1
#endif

/* The TSRM interpreter patch required by runkit_sandbox was added in 5.1, but this package includes diffs for older versions
 * Those diffs include an additional #define to indicate that they've been applied
 */
#if (defined(ZTS) && (PHP_MAJOR_VERSION > 5 || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 0) || defined(TSRM_INTERPRETER_PATCH_APPLIED))) && defined(PHP_RUNKIT_FEATURE_SANDBOX)
#define PHP_RUNKIT_SANDBOX
#endif

/* Superglobals don't exist until PHP 4.2 */
#if (PHP_MAJOR_VERSION > 4 || (PHP_MAJOR_VERSION == 4 && PHP_MINOR_VERSION > 1)) && defined(PHP_RUNKIT_FEATURE_SUPER)
#define PHP_RUNKIT_SUPERGLOBALS
#endif

#ifdef PHP_RUNKIT_FEATURE_MODIFY
#define PHP_RUNKIT_MANIPULATION
#endif

#ifdef ZEND_ENGINE_2
# ifdef PHP_RUNKIT_MANIPULATION
#include "Zend/zend_object_handlers.h"
# endif
#endif

extern zend_module_entry runkit_module_entry;
#define phpext_runkit_ptr &runkit_module_entry

PHP_MINIT_FUNCTION(runkit);
PHP_MSHUTDOWN_FUNCTION(runkit);
PHP_RINIT_FUNCTION(runkit);
PHP_RSHUTDOWN_FUNCTION(runkit);
PHP_MINFO_FUNCTION(runkit);

PHP_FUNCTION(runkit_return_value_used);
#ifdef ZEND_ENGINE_2
PHP_FUNCTION(runkit_object_id);
#endif

#ifdef PHP_RUNKIT_MANIPULATION
PHP_FUNCTION(runkit_function_add);
PHP_FUNCTION(runkit_function_remove);
PHP_FUNCTION(runkit_function_rename);
PHP_FUNCTION(runkit_function_redefine);
PHP_FUNCTION(runkit_function_copy);
PHP_FUNCTION(runkit_method_add);
PHP_FUNCTION(runkit_method_redefine);
PHP_FUNCTION(runkit_method_remove);
PHP_FUNCTION(runkit_method_rename);
PHP_FUNCTION(runkit_method_copy);
PHP_FUNCTION(runkit_constant_redefine);
PHP_FUNCTION(runkit_constant_remove);
PHP_FUNCTION(runkit_constant_add);
PHP_FUNCTION(runkit_default_property_add);
PHP_FUNCTION(runkit_default_property_remove);
PHP_FUNCTION(runkit_class_emancipate);
PHP_FUNCTION(runkit_class_adopt);
PHP_FUNCTION(runkit_import);
#endif /* PHP_RUNKIT_MANIPULATION */

#ifdef PHP_RUNKIT_SANDBOX
PHP_FUNCTION(runkit_sandbox_output_handler);
PHP_FUNCTION(runkit_lint);
PHP_FUNCTION(runkit_lint_file);

typedef struct _php_runkit_sandbox_object php_runkit_sandbox_object;
#endif /* PHP_RUNKIT_SANDBOX */

#if defined(PHP_RUNKIT_SUPERGLOBALS) || defined(PHP_RUNKIT_SANDBOX) || defined(PHP_RUNKIT_MANIPULATION)
ZEND_BEGIN_MODULE_GLOBALS(runkit)
#ifdef PHP_RUNKIT_SUPERGLOBALS
	HashTable *superglobals;
#endif
#ifdef PHP_RUNKIT_SANDBOX
	php_runkit_sandbox_object *current_sandbox;
#endif
#ifdef PHP_RUNKIT_MANIPULATION
	HashTable *misplaced_internal_functions;
	HashTable *replaced_internal_functions;
	zend_bool internal_override;
# ifdef ZEND_ENGINE_2
	const char *name_str, *removed_method_str, *removed_function_str, *removed_parameter_str, *removed_property_str;
	zend_function *removed_function, *removed_method;
# endif
#endif
ZEND_END_MODULE_GLOBALS(runkit)
#endif

extern ZEND_DECLARE_MODULE_GLOBALS(runkit);

#ifdef ZTS
#define		RUNKIT_G(v)		TSRMG(runkit_globals_id, zend_runkit_globals *, v)
#define RUNKIT_TSRMLS_C		TSRMLS_C
#else
#define		RUNKIT_G(v)		(runkit_globals.v)
#define RUNKIT_TSRMLS_C		NULL
#endif

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || (PHP_MAJOR_VERSION >= 6)
#     define RUNKIT_REFCOUNT  refcount__gc
#     define RUNKIT_IS_REF    is_ref__gc
#     define RUNKIT_IS_CALLABLE(cb_zv, flags, cb_sp) zend_is_callable((cb_zv), (flags), (cb_sp) TSRMLS_CC)
#     define RUNKIT_FILE_HANDLE_DTOR(pHandle)        zend_file_handle_dtor((pHandle) TSRMLS_CC)
#     define RUNKIT_53_TSRMLS_PARAM(param)           (param) TSRMLS_CC
#     define RUNKIT_53_TSRMLS_ARG(arg)               arg TSRMLS_DC
#     define RUNKIT_UNDER53_TSRMLS_FETCH()
#     define RUNKIT_UNDER53                          0
#     define RUNKIT_ABOVE53                          1
#else
#     define RUNKIT_REFCOUNT  refcount
#     define RUNKIT_IS_REF    is_ref
#     define RUNKIT_IS_CALLABLE(cb_zv, flags, cb_sp) zend_is_callable((cb_zv), (flags), (cb_sp))
#     define RUNKIT_FILE_HANDLE_DTOR(pHandle)        zend_file_handle_dtor((pHandle))
#     define RUNKIT_53_TSRMLS_PARAM(param)           (param)
#     define RUNKIT_53_TSRMLS_ARG(arg)               arg
#     define RUNKIT_UNDER53_TSRMLS_FETCH()           TSRMLS_FETCH()
#     define RUNKIT_UNDER53                          1
#     define RUNKIT_ABOVE53                          0
#     define zend_hash_quick_del(ht, key, key_len, h) zend_hash_del(ht, key, key_len)
#endif

#ifdef ZEND_ACC_RETURN_REFERENCE
#     define PHP_RUNKIT_ACC_RETURN_REFERENCE         ZEND_ACC_RETURN_REFERENCE
#else
#     define PHP_RUNKIT_ACC_RETURN_REFERENCE         0x4000000
#endif

#ifndef ALLOC_PERMANENT_ZVAL
# define ALLOC_PERMANENT_ZVAL(z) \
    (z) = (zval*)malloc(sizeof(zval))
#endif

#ifdef PHP_RUNKIT_MANIPULATION
#if defined(ZEND_ENGINE_2) && !defined(zend_hash_add_or_update)
/* Why doesn't ZE2 define this? */
#define zend_hash_add_or_update(ht, arKey, nKeyLength, pData, pDataSize, pDest, flag) \
        _zend_hash_add_or_update((ht), (arKey), (nKeyLength), (pData), (pDataSize), (pDest), (flag) ZEND_FILE_LINE_CC)
#endif

#ifndef Z_ADDREF_P
#     define Z_ADDREF_P(x)                           ZVAL_ADDREF(x)
#endif

/* runkit_functions.c */
#define RUNKIT_TEMP_FUNCNAME  "__runkit_temporary_function__"
int php_runkit_check_call_stack(zend_op_array *op_array TSRMLS_DC);
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 4) || (PHP_MAJOR_VERSION > 5)
void php_runkit_clear_all_functions_runtime_cache(TSRMLS_D);
#endif
#ifdef ZEND_ENGINE_2
void php_runkit_remove_function_from_reflection_objects(zend_function *fe TSRMLS_DC);
#endif
void php_runkit_function_copy_ctor(zend_function *fe, const char *newname, int newname_len TSRMLS_DC);
int php_runkit_generate_lambda_method(const char *arguments, int arguments_len, const char *phpcode, int phpcode_len,
                                      zend_function **pfe, zend_bool return_ref TSRMLS_DC);
int php_runkit_destroy_misplaced_functions(void *pDest TSRMLS_DC);
int php_runkit_restore_internal_functions(RUNKIT_53_TSRMLS_ARG(void *pDest), int num_args, va_list args, zend_hash_key *hash_key);
int php_runkit_clean_zval(zval **val TSRMLS_DC);

/* runkit_methods.c */
int php_runkit_fetch_class(const char *classname, int classname_len, zend_class_entry **pce TSRMLS_DC);
int php_runkit_fetch_class_int(const char *classname, int classname_len, zend_class_entry **pce TSRMLS_DC);
int php_runkit_clean_children_methods(RUNKIT_53_TSRMLS_ARG(zend_class_entry *ce), int num_args, va_list args, zend_hash_key *hash_key);
int php_runkit_update_children_methods(RUNKIT_53_TSRMLS_ARG(zend_class_entry *ce), int num_args, va_list args, zend_hash_key *hash_key);
#ifdef ZEND_ENGINE_2
int php_runkit_fetch_interface(const char *classname, int classname_len, zend_class_entry **pce TSRMLS_DC);
#endif

#if PHP_MAJOR_VERSION >= 6
#define PHP_RUNKIT_FUNCTION_ADD_REF(f)	function_add_ref(f TSRMLS_CC)
#define php_runkit_locate_scope(ce, fe, methodname_lower, methodname_len)   fe->common.scope
#define PHP_RUNKIT_DECL_STRING_PARAM(param)		void *param; int32_t param##_len; zend_uchar param##_type;
#define PHP_RUNKIT_STRING_SPEC					"t"
#define PHP_RUNKIT_STRING_PARAM(param)			&param, &param##_len, &param##_type
#define PHP_RUNKIT_STRTOLOWER(param)			php_u_strtolower(param, &param##_len, UG(default_locale))
#define PHP_RUNKIT_STRING_LEN(param,addtl)		(param##_type == IS_UNICODE ? UBYTES(param##_len + (addtl)) : (param##_len + (addtl)))
#define PHP_RUNKIT_STRING_TYPE(param)			(param##_type)
#define PHP_RUNKIT_HASH_FIND(hash,param,ppvar)	zend_u_hash_find(hash, param##_type, (UChar *)param, param##_len + 1, (void*)ppvar)
#define PHP_RUNKIT_HASH_EXISTS(hash,param)		zend_u_hash_exists(hash, param##_type, (UChar *)param, param##_len + 1)
#define PHP_RUNKIT_HASH_KEY(hash_key)			((hash_key)->type == HASH_KEY_IS_UNICODE ? (hash_key)->u.unicode : (hash_key)->u.string)
#define PHP_RUNKIT_HASH_KEYLEN(hash_key)		((hash_key)->type == HASH_KEY_IS_UNICODE ? UBYTES((hash_key)->nKeyLength) : (hash_key)->nKeyLength)

#elif PHP_MAJOR_VERSION >= 5
#define PHP_RUNKIT_FUNCTION_ADD_REF(f)	function_add_ref(f)
#define php_runkit_locate_scope(ce, fe, methodname_lower, methodname_len)   fe->common.scope
#define PHP_RUNKIT_DECL_STRING_PARAM(p)			char *p; int p##_len;
#define PHP_RUNKIT_STRING_SPEC					"s"
#define PHP_RUNKIT_STRING_PARAM(p)				&p, &p##_len
#define PHP_RUNKIT_STRTOLOWER(p)				php_strtolower(p, p##_len)
#define PHP_RUNKIT_STRING_LEN(param,addtl)		(param##_len + (addtl))
#define PHP_RUNKIT_STRING_TYPE(param)			IS_STRING
#define PHP_RUNKIT_HASH_FIND(hash,param,ppvar)	zend_hash_find(hash, param, param##_len + 1, (void*)ppvar)
#define PHP_RUNKIT_HASH_EXISTS(hash,param)		zend_hash_exists(hash, param##_type, param, param##_len + 1)
#define PHP_RUNKIT_HASH_KEY(hash_key)			((hash_key)->arKey)
#define PHP_RUNKIT_HASH_KEYLEN(hash_key)		((hash_key)->nKeyLength)

#else /* PHP4 */
#define PHP_RUNKIT_FUNCTION_ADD_REF(f)			function_add_ref(f)
zend_class_entry *_php_runkit_locate_scope(zend_class_entry *ce, zend_function *fe, const char *methodname_lower, int methodname_len TSRMLS_DC);
#define php_runkit_locate_scope(ce, fe, 		methodname_lower, methodname_len)   _php_runkit_locate_scope((ce), (fe), (methodname_lower), (methodname_len) TSRMLS_CC)
#define PHP_RUNKIT_DECL_STRING_PARAM(p)			char *p; int p##_len;
#define PHP_RUNKIT_STRING_SPEC					"s"
#define PHP_RUNKIT_STRING_PARAM(p)				&p, &p##_len
#define PHP_RUNKIT_STRTOLOWER(p)				php_strtolower(p, p##_len)
#define PHP_RUNKIT_STRING_LEN(param,addtl)		(param##_len + (addtl))
#define PHP_RUNKIT_STRING_TYPE(param)			IS_STRING
#define PHP_RUNKIT_HASH_FIND(hash,param,ppvar)	zend_hash_find(hash, param, param##_len + 1, (void*)ppvar)
#define PHP_RUNKIT_HASH_EXISTS(hash,param)		zend_hash_exists(hash, param##_type, param, param##_len + 1)
#define PHP_RUNKIT_HASH_KEY(hash_key)			((hash_key)->arKey)
#define PHP_RUNKIT_HASH_KEYLEN(hash_key)		((hash_key)->nKeyLength)
#define zend_function_dtor						destroy_zend_function

#endif /* Version Agnosticism */

/* runkit_constants.c */
int php_runkit_update_children_consts(RUNKIT_53_TSRMLS_ARG(void *pDest), int num_args, va_list args, zend_hash_key *hash_key);

/* runkit_classes.c */
int php_runkit_class_copy(zend_class_entry *src, const char *classname, int classname_len TSRMLS_DC);

/* runkit_props.c */
int php_runkit_update_children_def_props(RUNKIT_53_TSRMLS_ARG(zend_class_entry *ce), int num_args, va_list args, zend_hash_key *hash_key);
int php_runkit_def_prop_add_int(zend_class_entry *ce, const char *propname, int propname_len, zval *copyval, long visibility, const char *doc_comment, int doc_comment_len, zend_class_entry *definer_class, int override, int override_in_objects TSRMLS_DC);
#ifdef ZEND_ENGINE_2
int php_runkit_def_prop_remove_int(zend_class_entry *ce, const char *propname, int propname_len, zend_class_entry *definer_class,
                                   zend_bool was_static, zend_bool remove_from_objects, zend_property_info *parent_property TSRMLS_DC);
void php_runkit_remove_property_from_reflection_objects(zend_class_entry *ce, const char *prop_name, int prop_name_len TSRMLS_DC);
#else
int php_runkit_def_prop_remove_int(zend_class_entry *ce, const char *propname, int propname_len, zend_class_entry *definer_class,
                                   zend_bool was_static, zend_bool remove_from_objects, void *parent_property TSRMLS_DC);
#endif

#ifdef ZEND_ENGINE_2
typedef struct _zend_closure {
    zend_object    std;
    zend_function  func;
    HashTable     *debug_info;
} zend_closure;
#endif
#endif /* PHP_RUNKIT_MANIPULATION */

#ifdef PHP_RUNKIT_SANDBOX
/* runkit_sandbox.c */
int php_runkit_init_sandbox(INIT_FUNC_ARGS);
int php_runkit_shutdown_sandbox(SHUTDOWN_FUNC_ARGS);

/* runkit_sandbox_parent.c */
int php_runkit_init_sandbox_parent(INIT_FUNC_ARGS);
int php_runkit_shutdown_sandbox_parent(SHUTDOWN_FUNC_ARGS);
int php_runkit_sandbox_array_deep_copy(RUNKIT_53_TSRMLS_ARG(zval **value), int num_args, va_list args, zend_hash_key *hash_key);

struct _php_runkit_sandbox_object {
	zend_object obj;

	void *context, *parent_context;

	char *disable_functions;
	char *disable_classes;
	zval *output_handler;					/* points to function which lives in the parent_context */

	unsigned char bailed_out_in_eval;		/* Patricide is an ugly thing.  Especially when it leaves bailout address mis-set */

	unsigned char active;					/* A bailout will set this to 0 */
	unsigned char parent_access;			/* May Runkit_Sandbox_Parent be instantiated/used? */
	unsigned char parent_read;				/* May parent vars be read? */
	unsigned char parent_write;				/* May parent vars be written to? */
	unsigned char parent_eval;				/* May arbitrary code be run in the parent? */
	unsigned char parent_include;			/* May arbitrary code be included in the parent? (includes require(), and *_once()) */
	unsigned char parent_echo;				/* May content be echoed from the parent scope? */
	unsigned char parent_call;				/* May functions in the parent scope be called? */
	unsigned char parent_die;				/* Are $PARENT->die() / $PARENT->exit() enabled? */
	unsigned long parent_scope;				/* 0 == Global, 1 == Active, 2 == Active->prior, 3 == Active->prior->prior, etc... */

	char *parent_scope_name;				/* Combines with parent_scope to refer to a named array as a symbol table */
	int parent_scope_namelen;
};


/* TODO: It'd be nice if objects and resources could make it across... */
#define PHP_SANDBOX_CROSS_SCOPE_ZVAL_COPY_CTOR(pzv) \
{ \
	switch (Z_TYPE_P(pzv)) { \
		case IS_RESOURCE: \
		case IS_OBJECT: \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to translate resource, or object variable to current context."); \
			ZVAL_NULL(pzv); \
			break; \
		case IS_ARRAY: \
		{ \
			HashTable *original_hashtable = Z_ARRVAL_P(pzv); \
			array_init(pzv); \
			zend_hash_apply_with_arguments(RUNKIT_53_TSRMLS_PARAM(original_hashtable), (apply_func_args_t)php_runkit_sandbox_array_deep_copy, 1, Z_ARRVAL_P(pzv) TSRMLS_CC); \
			break; \
		} \
		default: \
			zval_copy_ctor(pzv); \
	} \
	(pzv)->RUNKIT_REFCOUNT = 1; \
	(pzv)->RUNKIT_IS_REF = 0; \
}
#endif /* PHP_RUNKIT_SANDBOX */

#ifdef PHP_RUNKIT_MANIPULATION
#define PHP_RUNKIT_SPLIT_PN(classname, classname_len, pnname, pnname_len) { \
	char *colon; \
\
	if ((pnname_len) > 3 && (colon = memchr((pnname), ':', (pnname_len) - 2)) && (colon[1] == ':')) { \
		(classname) = (pnname); \
		(classname_len) = colon - (classname); \
		(pnname) = colon + 2; \
		(pnname_len) -= (classname_len) + 2; \
	} else { \
		(classname) = NULL; \
		(classname_len) = 0; \
	} \
}

#ifdef ZEND_ENGINE_2
#if RUNKIT_ABOVE53
#	define PHP_RUNKIT_ADD_MAGIC_METHOD(ce, lcmname, mname_len, fe, orig_fe) { \
		if (!strncmp((lcmname), ZEND_CLONE_FUNC_NAME, (mname_len))) { \
			(ce)->clone = (fe); (fe)->common.fn_flags |= ZEND_ACC_CLONE; \
		} else if (!strncmp((lcmname), ZEND_CONSTRUCTOR_FUNC_NAME, (mname_len))) { \
			if (!(ce)->constructor || (ce)->constructor == (orig_fe)) { \
				(ce)->constructor = (fe); (fe)->common.fn_flags |= ZEND_ACC_CTOR; \
			} \
		} else if (!strncmp((lcmname), ZEND_DESTRUCTOR_FUNC_NAME, (mname_len))) { \
			(ce)->destructor = (fe); (fe)->common.fn_flags |= ZEND_ACC_DTOR; \
		} else if (!strncmp((lcmname), ZEND_GET_FUNC_NAME, (mname_len))) { \
			(ce)->__get = (fe); \
		} else if (!strncmp((lcmname), ZEND_SET_FUNC_NAME, (mname_len))) { \
			(ce)->__set = (fe); \
		} else if (!strncmp((lcmname), ZEND_CALL_FUNC_NAME, (mname_len))) { \
			(ce)->__call = (fe); \
		} else if (!strncmp((lcmname), ZEND_UNSET_FUNC_NAME, (mname_len))) { \
			(ce)->__unset = (fe); \
		} else if (!strncmp((lcmname), ZEND_ISSET_FUNC_NAME, (mname_len))) { \
			(ce)->__isset = (fe); \
		} else if (!strncmp((lcmname), ZEND_CALLSTATIC_FUNC_NAME, (mname_len))) { \
			(ce)->__callstatic = (fe); \
		} else if (!strncmp((lcmname), ZEND_TOSTRING_FUNC_NAME, (mname_len))) { \
			(ce)->__tostring = (fe); \
		} else if ((ce)->name_length == (mname_len)) { \
			char *lowercase_name = emalloc((ce)->name_length + 1); \
			zend_str_tolower_copy(lowercase_name, (ce)->name, (ce)->name_length); \
			if (!memcmp((lcmname), lowercase_name, (mname_len))) { \
				if (!(ce)->constructor || (ce)->constructor == (orig_fe)) { \
					(ce)->constructor = (fe); \
					(fe)->common.fn_flags |= ZEND_ACC_CTOR; \
				} \
			} \
			efree(lowercase_name); \
		} \
	}
#	define PHP_RUNKIT_DEL_MAGIC_METHOD(ce, fe) { \
		if      ((ce)->constructor == (fe))                         (ce)->constructor  = NULL; \
		else if ((ce)->destructor == (fe))                          (ce)->destructor   = NULL; \
		else if ((ce)->__get == (fe))                               (ce)->__get        = NULL; \
		else if ((ce)->__set == (fe))                               (ce)->__set        = NULL; \
		else if ((ce)->__unset == (fe))                             (ce)->__unset      = NULL; \
		else if ((ce)->__isset == (fe))                             (ce)->__isset      = NULL; \
		else if ((ce)->__call == (fe))                              (ce)->__call       = NULL; \
		else if ((ce)->__callstatic == (fe))                        (ce)->__callstatic = NULL; \
		else if ((ce)->__tostring == (fe))                          (ce)->__tostring   = NULL; \
		else if ((ce)->clone == (fe))                               (ce)->clone        = NULL; \
	}
#	define PHP_RUNKIT_INHERIT_MAGIC(ce, fe, orig_fe, is_constr) { \
		if ((ce)->__get == (orig_fe) && (ce)->parent->__get == (fe)) { \
			(ce)->__get        = (ce)->parent->__get; \
		} else if ((ce)->__set        == (orig_fe) && (ce)->parent->__set == (fe)) { \
			(ce)->__set        = (ce)->parent->__set; \
		} else if ((ce)->__unset      == (orig_fe) && (ce)->parent->__unset == (fe)) { \
			(ce)->__unset      = (ce)->parent->__unset; \
		} else if ((ce)->__isset      == (orig_fe) && (ce)->parent->__isset == (fe)) { \
			(ce)->__isset      = (ce)->parent->__isset; \
		} else if ((ce)->__call       == (orig_fe) && (ce)->parent->__call == (fe)) { \
			(ce)->__call       = (ce)->parent->__call; \
		} else if ((ce)->__callstatic == (orig_fe) && (ce)->parent->__callstatic == (fe)) { \
			(ce)->__callstatic = (ce)->parent->__callstatic; \
		} else if ((ce)->__tostring == (orig_fe) && (ce)->parent->__tostring == (fe)) { \
			(ce)->__tostring   = (ce)->parent->__tostring; \
		} else if ((ce)->clone        == (orig_fe) && (ce)->parent->clone == (fe)) { \
			(ce)->clone        = (ce)->parent->clone; \
		} else if ((ce)->destructor   == (orig_fe) && (ce)->parent->destructor == (fe)) { \
			(ce)->destructor   = (ce)->parent->destructor; \
		} else if ((ce)->constructor  == (orig_fe) && (ce)->parent->constructor == (fe)) { \
			(ce)->constructor  = (ce)->parent->constructor; \
		} \
	}
#else
#	define PHP_RUNKIT_ADD_MAGIC_METHOD(ce, lcmname, mname_len, fe, orig_fe) { \
		if (!strncmp((lcmname), ZEND_CLONE_FUNC_NAME, (mname_len))) { \
			(ce)->clone = (fe); (fe)->common.fn_flags |= ZEND_ACC_CLONE; \
		} else if (!strncmp((lcmname), ZEND_CONSTRUCTOR_FUNC_NAME, (mname_len))) { \
			if (!(ce)->constructor || (ce)->constructor == (orig_fe)) { \
				(ce)->constructor = (fe); (fe)->common.fn_flags |= ZEND_ACC_CTOR; \
			} \
		} else if (!strncmp((lcmname), ZEND_DESTRUCTOR_FUNC_NAME, (mname_len))) { \
			(ce)->destructor = (fe); (fe)->common.fn_flags |= ZEND_ACC_DTOR; \
		} else if (!strncmp((lcmname), ZEND_GET_FUNC_NAME, (mname_len))) { \
			(ce)->__get = (fe); \
		} else if (!strncmp((lcmname), ZEND_SET_FUNC_NAME, (mname_len))) { \
			(ce)->__set = (fe); \
		} else if (!strncmp((lcmname), ZEND_CALL_FUNC_NAME, (mname_len))) { \
			(ce)->__call = (fe); \
		} else if (!strncmp((lcmname), ZEND_UNSET_FUNC_NAME, (mname_len))) { \
			(ce)->__unset = (fe); \
		} else if (!strncmp((lcmname), ZEND_ISSET_FUNC_NAME, (mname_len))) { \
			(ce)->__isset = (fe); \
		} else if (!strncmp((lcmname), ZEND_TOSTRING_FUNC_NAME, (mname_len))) { \
			(ce)->__tostring = (fe); \
		} else if ((ce)->name_length == (mname_len)) { \
			char *lowercase_name = emalloc((ce)->name_length + 1); \
			zend_str_tolower_copy(lowercase_name, (ce)->name, (ce)->name_length); \
			if (!memcmp((lcmname), lowercase_name, (mname_len))) { \
				if (!(ce)->constructor || (ce)->constructor == (orig_fe)) { \
					(ce)->constructor = (fe); \
					(fe)->common.fn_flags |= ZEND_ACC_CTOR; \
				} \
			} \
			efree(lowercase_name); \
		} \
	}
#	define PHP_RUNKIT_DEL_MAGIC_METHOD(ce, fe) { \
		if      ((ce)->constructor == (fe))                         (ce)->constructor  = NULL; \
		else if ((ce)->destructor == (fe))                          (ce)->destructor   = NULL; \
		else if ((ce)->__get == (fe))                               (ce)->__get        = NULL; \
		else if ((ce)->__set == (fe))                               (ce)->__set        = NULL; \
		else if ((ce)->__unset == (fe))                             (ce)->__unset      = NULL; \
		else if ((ce)->__isset == (fe))                             (ce)->__isset      = NULL; \
		else if ((ce)->__call == (fe))                              (ce)->__call       = NULL; \
		else if ((ce)->__tostring == (fe))                          (ce)->__tostring   = NULL; \
		else if ((ce)->clone == (fe))                               (ce)->clone        = NULL; \
	}
#	define PHP_RUNKIT_INHERIT_MAGIC(ce, fe, orig_fe, is_constr) { \
		if ((ce)->__get == (orig_fe) && (ce)->parent->__get == (fe)) { \
			(ce)->__get        = (ce)->parent->__get; \
		} else if ((ce)->__set        == (orig_fe) && (ce)->parent->__set == (fe)) { \
			(ce)->__set        = (ce)->parent->__set; \
		} else if ((ce)->__unset      == (orig_fe) && (ce)->parent->__unset == (fe)) { \
			(ce)->__unset      = (ce)->parent->__unset; \
		} else if ((ce)->__isset      == (orig_fe) && (ce)->parent->__isset == (fe)) { \
			(ce)->__isset      = (ce)->parent->__isset; \
		} else if ((ce)->__call       == (orig_fe) && (ce)->parent->__call == (fe)) { \
			(ce)->__call       = (ce)->parent->__call; \
		} else if ((ce)->__tostring == (orig_fe) && (ce)->parent->__tostring == (fe)) { \
			(ce)->__tostring   = (ce)->parent->__tostring; \
		} else if ((ce)->clone        == (orig_fe) && (ce)->parent->clone == (fe)) { \
			(ce)->clone        = (ce)->parent->clone; \
		} else if ((ce)->destructor   == (orig_fe) && (ce)->parent->destructor == (fe)) { \
			(ce)->destructor   = (ce)->parent->destructor; \
		} else if ((ce)->constructor  == (orig_fe) && (ce)->parent->constructor == (fe)) { \
			(ce)->constructor  = (ce)->parent->constructor; \
		} \
	}
#	endif // RUNKIT_ABOVE53
#	define PHP_RUNKIT_DESTROY_FUNCTION(fe) 	destroy_zend_function(fe TSRMLS_CC);

#	if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 4) || (PHP_MAJOR_VERSION > 5)
#		define PHP_RUNKIT_UPDATE_REFLECTION_OBJECT_NAME(object, handle, name) { \
			zval obj, *zvname, *zvnew_name; \
			INIT_ZVAL(obj); \
			Z_TYPE(obj) = IS_OBJECT; \
			Z_OBJ_HANDLE(obj) = (handle); \
			obj.RUNKIT_REFCOUNT = 1; \
			obj.RUNKIT_IS_REF = 1; \
			MAKE_STD_ZVAL(zvname); \
			ZVAL_STRING(zvname, RUNKIT_G(name_str), 1); \
			MAKE_STD_ZVAL(zvnew_name); \
			ZVAL_STRING(zvnew_name, (name), 1); \
			zend_std_write_property(&obj, zvname, zvnew_name, NULL TSRMLS_CC); \
			zval_ptr_dtor(&zvnew_name); \
			zval_ptr_dtor(&zvname); \
		}
#	else
#		define PHP_RUNKIT_UPDATE_REFLECTION_OBJECT_NAME(object, handle, name) { \
			zval obj, *zvname, *zvnew_name; \
			INIT_ZVAL(obj); \
			Z_TYPE(obj) = IS_OBJECT; \
			Z_OBJ_HANDLE(obj) = (handle); \
			obj.RUNKIT_REFCOUNT = 1; \
			obj.RUNKIT_IS_REF = 1; \
			MAKE_STD_ZVAL(zvname); \
			ZVAL_STRING(zvname, (char *) RUNKIT_G(name_str), 1); \
			MAKE_STD_ZVAL(zvnew_name); \
			ZVAL_STRING(zvnew_name, (char *) (name), 1); \
			zend_get_std_object_handlers()->write_property(&obj, zvname, zvnew_name TSRMLS_CC); \
			zval_ptr_dtor(&zvnew_name); \
			zval_ptr_dtor(&zvname); \
		}
#	endif

#	if RUNKIT_ABOVE53
#		define PHP_RUNKIT_DELETE_REFLECTION_FUNCTION_PTR(obj) { \
			if ((obj)->ptr \
			    && ((zend_function *)(obj)->ptr)->type == ZEND_INTERNAL_FUNCTION \
			    && (((zend_function *)(obj)->ptr)->internal_function.fn_flags & ZEND_ACC_CALL_VIA_HANDLER) != 0) { \
				efree((char*)((zend_function *)(obj)->ptr)->internal_function.function_name); \
				efree((obj)->ptr); \
			} \
		}
#	else
#		define PHP_RUNKIT_DELETE_REFLECTION_FUNCTION_PTR(obj) { \
			if ((obj)->free_ptr && (obj)->ptr) { \
				efree((obj)->ptr); \
			} \
		}
#	endif

	/* Struct for properties */
	typedef struct _property_reference {
		zend_class_entry *ce;
		zend_property_info prop;
	} property_reference;

	/* Struct for parameters */
	typedef struct _parameter_reference {
		zend_uint offset;
		zend_uint required;
		struct _zend_arg_info *arg_info;
		zend_function *fptr;
	} parameter_reference;

	typedef enum {
		REF_TYPE_OTHER,      /* Must be 0 */
		REF_TYPE_FUNCTION,
		REF_TYPE_PARAMETER,
		REF_TYPE_PROPERTY,
		REF_TYPE_DYNAMIC_PROPERTY
	} reflection_type_t;

	/* Struct for reflection objects */
	typedef struct {
		zend_object zo;
		void *ptr;
#if RUNKIT_ABOVE53
		reflection_type_t ref_type;
#else
		unsigned int free_ptr:1;
#endif
		zval *obj;
		zend_class_entry *ce;
#if RUNKIT_ABOVE53
		unsigned int ignore_visibility:1;
#endif
	} reflection_object;
#else
#define PHP_RUNKIT_DESTROY_FUNCTION(fe) 	destroy_zend_function(fe);
#define PHP_RUNKIT_ADD_MAGIC_METHOD(ce, lcmname, mname_len, fe, orig_fe)
#define PHP_RUNKIT_DEL_MAGIC_METHOD(ce, fe) { \
	zend_function *current_constr; \
	if (zend_hash_find(&(ce)->function_table, (ce)->name, (ce)->name_length + 1, (void*) &current_constr) == SUCCESS && \
	    current_constr && current_constr->op_array.opcodes == (fe)->op_array.opcodes && (fe) != current_constr) { \
		zend_hash_del(&ce->function_table, ce->name, ce->name_length+1); \
	} \
}
#define PHP_RUNKIT_INHERIT_MAGIC(ce, fe, orig_fe, is_constr) { \
	zend_function *current_constr = NULL; \
	if (is_constr) { \
		zend_hash_find(&(ce)->function_table, (ce)->name, (ce)->name_length + 1, (void*) &current_constr); \
	} \
	if (((orig_fe) && current_constr && current_constr->op_array.opcodes == (orig_fe)->op_array.opcodes) || \
	    (!(orig_fe) && (is_constr) && !current_constr) \
	   ) { \
		zend_hash_update(&ce->function_table, ce->name, ce->name_length + 1, (fe), sizeof(zend_function), NULL); \
		function_add_ref(fe); \
	} \
}
#endif /* ZEND_ENGINE_2 */
#endif /* PHP_RUNKIT_MANIPULATION */

#endif	/* PHP_RUNKIT_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
