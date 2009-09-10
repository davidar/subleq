#include<stdio.h>
main(int C,char**A){FILE*F=fopen(A[1],"r");int P=0,_[9999],*i=_;while(fscanf(F,
"%d",i++)>0);while(P>=0){int a=_[P++],b=_[P++],c=_[P++];a<0?_[b]+=getchar():b<0
?printf("%c",_[a]):(_[b]-=_[a])<=0?P=c:0;}}
