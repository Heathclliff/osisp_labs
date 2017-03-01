#include <stdio.h>
#include <math.h>

int main(int  argc,char *argv[]){
	int passport,KolvoVar,ResultVar;
	passport=atoi(argv[1]);
	printf("Vash passport: %d\n",passport);
	KolvoVar=atoi(argv[2]);
	printf("Kol-vo variantov: %d\n",KolvoVar);
	int i=0;
	int randomX;
	for (i=0;i<passport;i++){
		randomX=rand();
	}
	ResultVar=randomX%KolvoVar+1;
	printf("Vash variant: %d\n",ResultVar);
	return 0;
}
