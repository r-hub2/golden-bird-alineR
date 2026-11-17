#include <R.h>
#include <Rdefines.h>
#include <stdio.h>
#include <string.h>

#include "glob.h"
#include "Word.h"
#include "wagner.h"

extern "C"{
  
  SEXP exchange(SEXP s, SEXP coeff, SEXP skpen){
    //first line is for test  int a=20;char aback1[20];char aback2[20];strcpy(aback1,"tu");strcpy(aback2,"tuF");printf("look at the returns:\n");printf("%s %s %d\n",aback1,aback2,a);
    
    // The words used to be strcpy()'d into char[20] buffers and the C++ code
    // assumed short words; longer ones overflowed its buffers. Check the
    // length against the (now consistent) limit ELEN instead.
    if (TYPEOF(s) != STRSXP || LENGTH(s) != 2)
        Rf_error("expected a character vector of two words");
    const char *s1 = CHAR(STRING_ELT(s,0));
    const char *s2 = CHAR(STRING_ELT(s,1));
    if (strlen(s1) >= (size_t) ELEN || strlen(s2) >= (size_t) ELEN)
        Rf_error("word too long: at most %d encoded characters are supported", ELEN - 1);
    // The C++ assert()s are compiled out (R builds with -DNDEBUG), so input
    // they assumed away reached the code: an empty word or one starting with
    // a modifier wrote to phon[-1]; NA weights overflowed. Reject these here.
    // Unknown codes after the first character stay ignored as before (this
    // is how the "@" placeholder of aline(mark = TRUE) works).
    for (int k = 0; k < 2; k++) {
        if (STRING_ELT(s, k) == NA_STRING)
            Rf_error("word %d is NA", k + 1);
        const char *w = (k == 0) ? s1 : s2;
        if (w[0] == '\0')
            Rf_error("word %d is empty (were all of its characters invalid?)", k + 1);
        if (w[0] < 'a' || w[0] > 'z')
            Rf_error("word %d must start with a known segment (encoded a-z), not '%c'", k + 1, w[0]);
    }
    if (TYPEOF(skpen) != INTSXP || LENGTH(skpen) != 1 || INTEGER(skpen)[0] == NA_INTEGER)
        Rf_error("skip penalty must be a single non-missing number");
    // R passes 12 feature weights (Syllabic ... Round); the skip penalty
    // arrives separately in skpen. The old code read 13 values here, one
    // past the end of the vector (CRAN valgrind: "Invalid read of size 4").
    if (TYPEOF(coeff) != INTSXP || LENGTH(coeff) != 12)
        Rf_error("feature weights must be an integer vector of length 12");
    int vec[12];
    for(int i=0;i<12;i++) {
        vec[i]=INTEGER(coeff)[i];
        if (vec[i] == NA_INTEGER) Rf_error("feature weights must not be NA");
    }
    int SkipPen=INTEGER(skpen)[0];     //printf("the skip penalty input:%d\n",SkipPen);

        int a=0;
        int *score;
        score=&a;
        //int vec[12]={1,2,3,4,5,6,7,8,9,0,11,12};
        char aback1[ABACK];
        char aback2[ABACK];
        aback1[0] = aback2[0] = '\0';

        Word::applyRedundancyRules();
        Word* w1 = new Word(s1);
        Word* w2 = new Word(s2);
        align( w1, w2,score, aback1, aback2, vec, SkipPen);
        delete w1;
        delete w2;
        
        //printf("look at the outputs from character functions:\n");printf("%s\n",aback1);printf("%s\n",aback2); printf("The score: %d\n",a);       
        a=*score;

    
    
    // This is the output part, where a list is now successfully returned!
    SEXP alignment1=PROTECT(allocVector(STRSXP,1));
    SEXP alignment2=PROTECT(allocVector(STRSXP,1));
    SEXP simscore=PROTECT(allocVector(REALSXP,1));
    SEXP result=PROTECT(allocVector(VECSXP,3));
    SET_STRING_ELT(alignment1,0,mkChar(aback1));
    SET_STRING_ELT(alignment2,0,mkChar(aback2));
    REAL(simscore)[0]=a;
    SET_VECTOR_ELT(result,0,alignment1);
    SET_VECTOR_ELT(result,1,alignment2);
    SET_VECTOR_ELT(result,2,simscore);
    UNPROTECT(4);
    return(result);
  }
  
  
}

float gP::ScoreMargin = 1.0;
int gP::MaxScore = conv(35); 
int gP::MaxCmpScore = conv(45);
//int gP::SkipCost = conv(10);
int gP::VwlDecr = conv(10);
//int gP::SalCoef[FT_LEN] = { 5, 40, 50, 10, 10, 10, 10, 5, 1, 5, 5, 5, 10 };
