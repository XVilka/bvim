#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "search.h"
#include "bvim.h"

extern core_t core;
extern state_t state;

int mlist_add(mlist ml, match_t item)
{
	mlist t = NULL;
	t = (mlist)malloc(sizeof(*t));
	if (t != NULL) {
		t->m = item;
		if (ml != NULL) {
			t->next = ml->next;
			ml->next = t;
		} else {
			ml = t;
			ml->next = NULL;
		}
	} else {
		return -1;
	}
	return 0;	
}

void mlist_free(mlist ml)
{
	mlist t = ml;
	while (t != NULL) {
		free(t);
		t = t->next;
	}
}

// Bitup fuzzy search algorithm
// pattern must be < 32 bytes
// saving results in ml parameter
// k == errors count
long bitap_fuzzy_bitwise_search_binary(mlist ml, const char *buf, long buf_length, const char *pattern, long pat_size, int k)
{
	match_t *result = NULL;
	long m = pat_size;
	unsigned long *R;
	unsigned long pattern_mask[CHAR_MAX+1];
	long i, d;
	long match_count = 0;

	result = (match_t *)malloc(sizeof(*result));

	if (pattern[0] == '\0') {
		return -1; // Error: wrong pattern
	}
	if (m > 31) {
		return -1 ; // pattern too long!
	}

	/* Initialize the bit array R */
	R = malloc((k + 1) * sizeof *R);
	for (i = 0; i <= k; ++i)
		R[i] = ~1;

	/* Initialize the pattern bitmasks */
	for (i = 0; i <= CHAR_MAX; ++i)
		pattern_mask[i] = ~0;
	for (i = 0; i < m; ++i)
		pattern_mask[pattern[i]] &= ~(1UL << i);

	for (i = 0; i < buf_length; ++i) {
		/* Update the bit arrays */
		unsigned long old_Rd1 = R[0];

		R[0] |= pattern_mask[buf[i]];
		R[0] <<= 1;

		for (d=1; d <= k; ++d) {
			unsigned long tmp = R[d];
			/* Substitution is all we care about */
			R[d] = (old_Rd1 & (R[d] | pattern_mask[buf[i]])) << 1;
			old_Rd1 = tmp;
		}

		if (0 == (R[k] & (1UL << m))) {
			result->ptr = (buf + i - m) + 1;
			result->start =  i - m + 1;
			result->length = m;
			result->str = (char*)malloc(result->length);
			memcpy(result->str, result->ptr, result->length);
			mlist_add(ml, *result);
			match_count++;
		}
	}

	free(R);
	return match_count;
}

// TODO: Add distance propose
struct found fuzzy_search(void* input, long input_size, void *pattern, long pattern_size, int algo, int distance)
{
	struct found fnd;
	fnd.cnt = 0;
	fnd.ml = NULL;

	if (algo == FUZZY_BITAP_HAMMINGTON_DISTANCE) {
		fnd.cnt = bitap_fuzzy_bitwise_search_binary(fnd.ml, input, input_size, pattern, pattern_size, distance);
	} else if (algo == FUZZY_BITAP_LEVENSHTEIN_DISTANCE) {
		bvim_error(state.mode, "Levenshtein distance: not yet supported!");
	} else {
		bvim_error(state.mode, "Wrong fuzzy search algorithm!");
	}
	return fnd;
}
