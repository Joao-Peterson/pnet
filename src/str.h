#ifndef _STR_HEADER_
#define _STR_HEADER_

#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

// ------------------------------------------------------------ String -------------------------------------------------------------

typedef struct{
	bool managed;
	char *raw;
	size_t len;
}string_t;

string_t *string_new(size_t size);

string_t *string_copy(string_t *string);

string_t *string_from(char *string);

string_t *string_wrap(char *string);

string_t *string_vsprint(const char *fmt, size_t buffer_size, va_list args);

string_t *string_sprint(const char *fmt, size_t buffer_size, ...);

void string_destroy(string_t *string);

char *string_unwrap(string_t *string);

void string_println(string_t *string);

bool string_cmp_raw(string_t *a, char *b);

bool string_cmp(string_t *a, string_t *b);

void string_cat_raw(string_t *dest, char *src);

void string_cat(string_t *dest, string_t *src);

void string_cat_vfmt(string_t *string, const char *fmt, size_t buffer_size, va_list args);

void string_cat_fmt(string_t *string, const char *fmt, size_t buffer_size, ...);

void string_trim_end(string_t *string, size_t size);

#endif