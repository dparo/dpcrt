
#include "dpcrt_algorithms.h"
#include "dpcrt_allocators.h"



size_t
LevenshteinDistance(char *s, char *t)
{
       
    const size_t n = strlen(t);
    const size_t m = strlen(s);

    size_t *v0 = BUF_NEW(size_t, n);
    size_t *v1 = BUF_NEW(size_t, n);
  

    for (size_t i = 0; i < n; i ++)
    {
        v0[i] = i; 
    }

    for (size_t i = 0; i < m; i++)
    {
        v1[0] = i + 1;


        for (size_t j = 0; j < n; j++)
        {
            size_t deletion_cost     = v0[j + 1] + 1;
            size_t insertion_cost    = v1[j] + 1;
            size_t substitution_cost = v0[j];
            
            if (s[i] != t[j])
                substitution_cost += 1;

            v1[j + 1] = MIN(deletion_cost, MIN(insertion_cost, substitution_cost));
        }

        SWAP(size_t*, v0, v1);
    }
    

    size_t result = v0[n];
    BUF_FREE(v0);
    BUF_FREE(v1);
    return result;
}


size_t
LongestCommonSubSequence(char *s, char *t)
{
    // TODO IMPLEMENT ME
    return 0;
}

