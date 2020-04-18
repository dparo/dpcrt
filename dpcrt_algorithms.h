#ifndef HGUARD_5357420049c74e87909cad414989c447
#define HGUARD_5357420049c74e87909cad414989c447

#include "dpcrt_utils.h"


function LevenshteinDistance(char s[0..m-1], char t[0..n-1]):
    // create two work vectors of integer distances
    declare int v0[n + 1]
    declare int v1[n + 1]

    // initialize v0 (the previous row of distances)
    // this row is A[0][i]: edit distance for an empty s
    // the distance is just the number of characters to delete from t
    for i from 0 to n:
        v0[i] = i

    for i from 0 to m-1:
        // calculate v1 (current row distances) from the previous row v0

        // first element of v1 is A[i+1][0]
        //   edit distance is delete (i+1) chars from s to match empty t
        v1[0] = i + 1

        // use formula to fill in the rest of the row
        for j from 0 to n-1:
            // calculating costs for A[i+1][j+1]
            deletionCost := v0[j + 1] + 1
            insertionCost := v1[j] + 1
            if s[i] = t[j]:
                substitutionCost := v0[j]
            else:
                substitutionCost := v0[j] + 1;

            v1[j + 1] := minimum(deletionCost, insertionCost, substitutionCost)

        // copy v1 (current row) to v0 (previous row) for next iteration
        swap v0 with v1
    // after the last swap, the results of v1 are now in v0
    return v0[n]

#endif /* HGUARD_5357420049c74e87909cad414989c447 */
