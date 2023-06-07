#include "str.h"

#include <stdlib.h>
#include <stdio.h>

// ------------------------------------------------------------ String -------------------------------------------------------------

string_t *string_new(size_t len){
	string_t *str = calloc(1, sizeof(string_t));
	str->len = len;
	str->managed = true;

	if(len > 0 )
		str->raw = calloc(len + 1, sizeof(char));
	else
		str->raw = NULL;

	return str;
}

string_t *string_copy(string_t *string){
	if(string == NULL)
		return string_new(0);

	string_t *str = string_new(string->len);
	memcpy(str->raw, string->raw, str->len);
	return str;
}

string_t *string_from(char *string){
	if(string == NULL)
		return string_new(0);
		
	size_t len = strlen(string);
	string_t *str = string_new(len);
	memcpy(str->raw, string, len);
	return str;
}

string_t *string_wrap(char *string){
	if(string == NULL)
		return string_new(0);
		
	size_t len = strlen(string);
	string_t *str = string_new(0);
	str->managed = false;
	str->len = len;
	str->raw = string;
	return str;
}

string_t *string_vsprint(const char *fmt, size_t buffer_size, va_list args){
	char *buffer = malloc(buffer_size);
	vsnprintf(buffer, buffer_size - 1, fmt, args);
	string_t *str = string_from(buffer);
	free(buffer);
	return str;
}

string_t *string_sprint(const char *fmt, size_t buffer_size, ...){
	va_list args;
	va_start(args, buffer_size);
	string_t *str = string_vsprint(fmt, buffer_size, args);
	va_end(args);
	return str;
}

void string_destroy(string_t *string){
	if(string == NULL) return;
	
	if(string->managed && string->raw != NULL)
		free(string->raw);

	free(string);
	string = NULL;
}

char *string_unwrap(string_t *string){
	if(string == NULL) return NULL;
	char *tmp = string->raw;	
	free(string);
	string = NULL;
	return tmp;
}

void string_println(string_t *string){
	printf("%s\n", string->raw == NULL ? "" : string->raw);
}

bool string_cmp(string_t *a, string_t *b){
	if(a->raw == NULL || b->raw == NULL) return false;
	return !strcmp(a->raw, b->raw);
}

bool string_cmp_raw(string_t *a, char *b){
	if(a->raw == NULL || b == NULL) return false;
	return !strcmp(a->raw, b);
}

void _string_cat_raw(string_t *dest, char *src, size_t srclen){
	if(dest == NULL || src == NULL) return;

	size_t len = dest->len + srclen;
	if(dest->len == 0){
		dest->len = len;
		dest->raw = calloc(len + 1, sizeof(char));
		memcpy(dest->raw, src, len);	
	}
	else{
		dest->raw = realloc(dest->raw, len + 1);
		dest->len = len;
		strncat(dest->raw, src, len);
	}
}

void string_cat_raw(string_t *dest, char *src){
	if(dest == NULL || src == NULL) return;

	_string_cat_raw(dest, src, strlen(src));
}

void string_cat(string_t *dest, string_t *src){
	if(dest == NULL || src == NULL) return;

	_string_cat_raw(dest, src->raw, src->len);
}

void string_cat_vfmt(string_t *string, const char *fmt, size_t buffer_size, va_list args){
	if(string == NULL || buffer_size == 0) return;

	string_t *expanded = string_vsprint(fmt, buffer_size, args);
	string_cat(string, expanded);
	string_destroy(expanded);
}

void string_cat_fmt(string_t *string, const char *fmt, size_t buffer_size, ...){
	va_list args;
	va_start(args, buffer_size);
	string_cat_vfmt(string, fmt, buffer_size, args);
	va_end(args);
}

void string_trim_end(string_t *string, size_t size){
	if(string == NULL || string->len == 0) return;
	if(size >= string->len) return;

	string->len -= size;
	string->raw[string->len] = '\0';
}