/*
	donut.c
	Original author: Andy Sloane
	https://www.a1k0n.net/2006/09/15/obfuscated-c-donut.html	Original
	https://www.a1k0n.net/2011/07/20/donut-math.html			Math's behind it
	https://www.a1k0n.net/2021/01/13/optimizing-donut.html		Donut without math.harderr
	
	Working on adding extra things.
*/

/* Original code */
//      			k;double sin()
//               ,cos();main(){float A=
//             0,B=0,i,j,z[1760];char b[
//           1760];printf("\x1b[2J");for(;;
//        ){memset(b,32,1760);memset(z,0,7040)
//        ;for(j=0;6.28>j;j+=0.07)for(i=0;6.28
//       >i;i+=0.02){float c=sin(i),d=cos(j),e=
//       sin(A),f=sin(j),g=cos(A),h=d+2,D=1/(c*
//       h*e+f*g+5),l=cos      (i),m=cos(B),n=s\
//      in(B),t=c*h*g-f*        e;int x=40+30*D*
//      (l*h*m-t*n),y=            12+15*D*(l*h*n
//      +t*m),o=x+80*y,          N=8*((f*e-c*d*g
//       )*m-c*d*e-f*g-l        *d*n);if(22>y&&
//       y>0&&x>0&&80>x&&D>z[o]){z[o]=D;;;b[o]=
//       ".,-~:;=!*#$@"[N>0?N:0];}}/*#****!!-*/
//        printf("\x1b[H");for(k=0;1761>k;k++)
//         putchar(k%80?b[k]:10);A+=0.04;B+=
//           0.02;}}/*****####*******!!=;:~
//             ~::==!!!**********!!!==::-
//               .,~~;;;========;;;:~-.
//                   ..,--------,*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

void Original(void)
{
	float A = 0;
	float B = 0;
	float z[1760];
	char b[1760];
	printf("\x1b[2J");
	for(;;)
	{
		memset(b, 32, 1760);
		memset(z, 0, 7040);
		for(float i = 0; i < 6.28; i += 0.07)
		{
			for(float ii = 0; ii < 6.28; ii += 0.02)
			{
				float c = sin(ii);
				float d = cos(i);
				float e = sin(A);
				float f = sin(i);
				float g = cos(A);
				float h = d + 2;
				float D = 1 / (c * h * e + f * g + 5);
				float l = cos(ii);
				float m = cos(B);
				float n = sin(B);
				float t = c * h * g - f * e;
				
				int x = 40 + 30 * D * (l * h * m - t * n);
				int y = 12 + 15 * D * (l * h * n + t * m);
				int o = x + 80 * y;
				int N = 8 * (( f * e - c * d * g) * m - c * d * e - f * g - l * d * n);
				
				if( 22 > y && y > 0 && x > 0 && 80 > x && D > z[o])
				{
					z[o] = D;
					b[o] = ".,-~:;=!*#$@"[N > 0 ? N : 0];
				}
			}
		}
			
		printf("\x1b[H");
		for(int i = 0 ; i < 1761; i++)
		{
		   putchar(i % 80 ? b [i] : 10);
		}	
		A += 0.04;
		B += 0.02;
	}
}

#define ROTATE(mul, shift, x, y) 			\
	do 			 							\
	{  			 							\
		_ = x;								\
		x -= mul * y >> shift; 				\
		y += mul * _ >> shift;				\
		_ = 3145728 - x * x - y * y >> 11;	\
		x = x * _ >> 10;					\
		y = y * _ >> 10;					\
	} while(0) 		 					

void Optimised(void)
{
	int8_t b[1760], z[1760];
	
	int sA = 1024;
	int cA = 0;
	int sB = 1024;
	int cB = 0;
	int _  = 0;

	for(;;)
	{
		memset(b, 32, 1760);
		memset(z, 127, 1760);
		int sj = 0;
		int cj = 1024;
		for(int i = 0; i < 90; i++)
		{
			int si = 0;
			int ci = 1024;
			for(int ii = 0; ii < 324; ii++)
			{
				int R1 = 1;
				int R2 = 2048;
				int K2 = 5120 * 1024;

				int x0 = R1 * cj + R2;
				int x1 = ci * x0 >> 10;
				int x2 = cA * sj >> 10;
				int x3 = si * x0 >> 10;
				int x4 = R1 * x2 - (sA * x3 >> 10);
				int x5 = sA * sj >> 10;
				int x6 = K2 + R1 * 1024 * x5 + cA * x3;
				int x7 = cj * si >> 10;
				int x = 40 + 30 * (cB * x1 - sB * x4) / x6;
				int y = 12 + 15 * (cB * x4 + sB * x1) / x6;
				int N = (-cA * x7 - cB * ((-sA * x7 >> 10) + x2) - ci * (cj * sB >> 10) >> 10) - x5 >> 7;

				int o = x + 80 * y;
				int8_t zz = (x6 - K2) >> 15;
				if(22 > y && y > 0 && x > 0 && 80 > x && zz < z[o])
				{
					z[o] = zz;
					b[o] = ".,-~:;=!*#$@"[N > 0 ? N : 0];
				}
				ROTATE(5, 8, ci, si);
			}
			ROTATE(9, 7, cj, sj);
		}
		for(int i = 0; i < 1761; i++)
		{	
			putchar(i % 80 ? b[i] : 10);
		}
		ROTATE(5, 7, cA, sA);
		ROTATE(5, 8, cB, sB);
		usleep(15000);
		printf("\x1b[23A");
	}
}

int main(int argc, char * argv[])
{
	int selection = 0;
	if(argc > 1)
	{
		selection = atoi(argv[1]);
	}
	
	if(selection == 1)
	{
		Original();
	}
	else if(selection == 2)
	{
		Optimised();
	}
	else
	{
		printf("donut:\n");
		printf("1 to run original version:\n");
		printf("2 to run optimised version:\n");
	}
	return 0;
}