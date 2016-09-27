#include <stdio.h>

#define PCRE2_CODE_UNIT_WIDTH 8

#include "pcre2/config.h"
#include "pcre2/pcre2.h"
#include "main.h"

static int work_space[4096];

void pcre2_find_all(char* pattern, char* subject, int subject_len, int repeat, int mode)
{
	pcre2_code *re;
	pcre2_compile_context *comp_ctx;
	pcre2_match_data *match_data;
	pcre2_match_context *match_ctx;
	int err_code;
	PCRE2_SIZE err_offset;
	pcre2_jit_stack *stack = NULL;
	PCRE2_SIZE *ovector;
	char *ptr;
	int len;
	TIME_TYPE start, end, resolution;
	int found, time, best_time = 0;

	GET_TIME_RESOLUTION(resolution);

	comp_ctx = pcre2_compile_context_create(NULL);
	if (!comp_ctx) {
		printf("PCRE2 cannot allocate compile context\n");
		return;
	}

	pcre2_set_newline(comp_ctx, PCRE2_NEWLINE_ANYCRLF);

	re = pcre2_compile(
		(PCRE2_SPTR8) pattern,	/* the pattern */
		PCRE2_ZERO_TERMINATED,	/* length */
		PCRE2_MULTILINE,	/* options */
		&err_code,		/* for error code */
		&err_offset,		/* for error offset */
		comp_ctx);		/* use default character tables */

	if (!re) {
		printf("PCRE2 compilation failed at offset %d: [%d]\n", (int)err_offset, err_code);
		return;
	}

	pcre2_compile_context_free(comp_ctx);

	match_ctx = pcre2_match_context_create(NULL);
	if (!match_ctx) {
		printf("PCRE JIT cannot allocate match context\n");
		return;
	}

	if (mode == 2) {
		if (pcre2_jit_compile(re, PCRE2_JIT_COMPLETE)) {
			printf("PCRE JIT compilation failed\n");
			return;
		}
		stack = pcre2_jit_stack_create(65536, 65536, NULL);
		if (!stack) {
			printf("PCRE JIT cannot allocate JIT stack\n");
			return;
		}
		pcre2_jit_stack_assign(match_ctx, NULL, stack);
	}

	if (mode == 1)
		match_data = pcre2_match_data_create(32, NULL);
	else
		match_data = pcre2_match_data_create_from_pattern(re, NULL);

	if (!match_data) {
		printf("PCRE2 cannot allocate match data\n");
		return;
	}

	ovector = pcre2_get_ovector_pointer(match_data);

	do {
		found = 0;
		ptr = subject;
		len = subject_len;
		switch (mode) {
		case 0:
			GET_TIME(start);
			while (1) {
				err_code = pcre2_match(
					re,			/* the compiled pattern */
					(PCRE2_SPTR8) ptr,	/* the subject string */
					len,			/* the length of the subject */
					0,			/* start at offset 0 in the subject */
					0,			/* default options */
					match_data,		/* match data */
					match_ctx);		/* match context */

				if (err_code <= 0) {
					if (err_code == PCRE2_ERROR_NOMATCH)
						break;
					printf("PCRE pcre_exec failed with: %d\n", err_code);
					break;
				}

				// printf("match: %d %d\n", (ptr - subject) + match[0], (ptr - subject) + match[1]);
				ptr += ovector[1];
				len -= ovector[1];
				found++;
			}
			GET_TIME(end);
			break;

		case 1:
			GET_TIME(start);
			while (1) {
				err_code = pcre2_dfa_match(
					re,			/* the compiled pattern */
					(PCRE2_SPTR8) ptr,	/* the subject string */
					len,			/* the length of the subject */
					0,			/* start at offset 0 in the subject */
					0,			/* default options */
					match_data,		/* match data */
					match_ctx,		/* match context */
					work_space,		/* work space */
					4096);			/* number of elements (NOT size in bytes) */

				if (err_code <= 0) {
					if (err_code == PCRE2_ERROR_NOMATCH)
						break;
					printf("PCRE pcre_exec failed with: %d\n", err_code);
					break;
				}

				// printf("match: %d %d\n", (ptr - subject) + match[0], (ptr - subject) + match[1]);
				ptr += ovector[1];
				len -= ovector[1];
				found++;
			}
			GET_TIME(end);
			break;

		case 2:
			GET_TIME(start);
			while (1) {
				err_code = pcre2_jit_match(
					re,			/* the compiled pattern */
					(PCRE2_SPTR8) ptr,	/* the subject string */
					len,			/* the length of the subject */
					0,			/* start at offset 0 in the subject */
					0,			/* default options */
					match_data,		/* match data */
					match_ctx);		/* match context */

				if (err_code <= 0) {
					if (err_code == PCRE2_ERROR_NOMATCH)
						break;
					printf("PCRE pcre_exec failed with: %d\n", err_code);
					break;
				}

				// printf("match: %d %d\n", (ptr - subject) + match[0], (ptr - subject) + match[1]);
				ptr += ovector[1];
				len -= ovector[1];
				found++;
			}
			GET_TIME(end);
			break;
		}
		time = TIME_DIFF_IN_MS(start, end, resolution);
		if (!best_time || time < best_time)
			best_time = time;
	} while (--repeat > 0);
	printResult(mode == 0 ? "pcre" : (mode == 1 ? "pcre-dfa" : "pcre-jit"), best_time, found);

	if (stack)
		pcre2_jit_stack_free(stack);
	pcre2_match_context_free(match_ctx);
	pcre2_code_free(re);
}
