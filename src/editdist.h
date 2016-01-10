#ifndef EDITDIST_H_
#define EDITDIST_H_

#include <algorithm>
#ifndef _MSC_VER
#include <alloca.h>
#else
#include <malloc.h>
#endif

/**
 * Computes the levenshtein distance between two arrays.
 * Algorithm lifted directly from wikipedia, seems to work fine.
 */
template <typename T>
int edit_distance(const T *s1, int len1, const T *s2, int len2)
{
	// degenerate cases
	if(s1 == s2) return 0;
	if(len1 == 0) return len2;
	if(len2 == 0) return len1;

	// create two work vectors of integer distances
	int *v0 = (int*)alloca((len2 + 1) * sizeof *v0);
	int *v1 = (int*)alloca((len2 + 1) * sizeof *v1);

	// initialize v0 (the previous row of distances)
	// this row is A[0][i]: edit distance for an empty s
	// the distance is just the number of characters to delete from t
	for(int i = 0; i < len2 + 1; i++) {
		v0[i] = i;
	}

	for(int i = 0; i < len1; i++) {
		// calculate v1 (current row distances) from the previous row v0

		// first element of v1 is A[i+1][0]
		//   edit distance is delete (i+1) chars from s to match empty t
		v1[0] = i + 1;

		// use formula to fill in the rest of the row
		for(int j = 0; j < len2; j++) {
			int cost = (s1[i] == s2[j]) ? 0 : 1;
			v1[j + 1] = std::min(v1[j] + 1, std::min(v0[j + 1] + 1, v0[j] + cost));
		}

		// copy v1 (current row) to v0 (previous row) for next interation
		for(int j = 0; j < len2 + 1; j++) {
			v0[j] = v1[j];
		}
	}

	return v1[len2];
}


// convenience function for simple strings
inline int edit_distance(const char *s1, const char *s2)
{
	int len1 = strlen(s1);
	int len2 = strlen(s2);

	return edit_distance(s1, len1, s2, len2);
}


#endif	// EDITDIST_H_
