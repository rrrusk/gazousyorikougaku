#include"bitmap.h"
#include<stdlib.h>
#include<stdio.h>

int main(int argc, char *argv[])
{
//	if(argc != 3){
//		fprintf(stderr, "Usage: program <inputfile> <outputfile>\n");
//		exit(1);
//	}

	Image *colorimg;
	Image *template;
	Image *nccimg;

	if((colorimg = Read_Bmp("img/gopochi-all.bmp")) == NULL){
		exit(1);
	}

  if((template = Read_Bmp("img/gopochi-template.bmp")) == NULL){
    exit(1);
  }

	nccimg = Create_Image((colorimg->width + template->width), (colorimg->height + template->height));

  printf("%d\n", colorimg->width);
  Template_Matching(colorimg, template, nccimg);
//  NCC(template, colorimg, nccimg);
	if(Write_Bmp("img/out.bmp", nccimg)){
		exit(1);
	}

	Free_Image(colorimg);
  Free_Image(nccimg);
  Free_Image(template);

	return 0;
}

