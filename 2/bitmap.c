#include<stdio.h>
#include<string.h>
#include<math.h>
#include"bitmap.h"

//filenameのBitmapファイルを読み込み、高さと幅、RGB情報をimg構造体に入れる
Image *Read_Bmp(char *filename)
{
	int i, j;
	int real_width;					//データ上の1行分のバイト数
	unsigned int width, height;			//画像の横と縦のピクセル数
	unsigned int color;			//何bitのBitmapファイルであるか
	FILE *fp;
	unsigned char header_buf[HEADERSIZE];	//ヘッダ情報を取り込む
	unsigned char *bmp_line_data;  //画像データ1行分
	Image *img;

	if((fp = fopen(filename, "rb")) == NULL){
		fprintf(stderr, "Error: %s could not read.", filename);
		return NULL;
	}

	fread(header_buf, sizeof(unsigned char), HEADERSIZE, fp); //ヘッダ部分全てを取り込む

	//最初の2バイトがBM(Bitmapファイルの印)であるか
	if(strncmp(header_buf, "BM", 2)){
		fprintf(stderr, "Error: %s is not Bitmap file.", filename);
		return NULL;
	}

	memcpy(&width, header_buf + 18, sizeof(width)); //画像の見た目上の幅を取得
	memcpy(&height, header_buf + 22, sizeof(height)); //画像の高さを取得
	memcpy(&color, header_buf + 28, sizeof(unsigned int)); //何bitのBitmapであるかを取得

	//24bitで無ければ終了
	if(color != 24){
		fprintf(stderr, "Error: %s is not 24bit color image", filename);
		return NULL;
	}

	//RGB情報は画像の1行分が4byteの倍数で無ければならないためそれに合わせている
	real_width = width*3 + width%4;

	//画像の1行分のRGB情報を取ってくるためのバッファを動的に取得
	if((bmp_line_data = (unsigned char *)malloc(sizeof(unsigned char)*real_width)) == NULL){
		fprintf(stderr, "Error: Allocation error.\n");
		return NULL;
	}

	//RGB情報を取り込むためのバッファを動的に取得
	if((img = Create_Image(width, height)) == NULL){
		free(bmp_line_data);
		fclose(fp);
		return NULL;
	}

	//BitmapファイルのRGB情報は左下から右へ、下から上に並んでいる
	for(i=0; i<height; i++){
		fread(bmp_line_data, 1, real_width, fp);
		for(j=0; j<width; j++){
			img->data[(height-i-1)*width + j].b = bmp_line_data[j*3];
			img->data[(height-i-1)*width + j].g = bmp_line_data[j*3 + 1];
			img->data[(height-i-1)*width + j].r = bmp_line_data[j*3 + 2];
		}
	}

	free(bmp_line_data);

	fclose(fp);

	return img;
}

int Write_Bmp(char *filename, Image *img)
{
	int i, j;
	FILE *fp;
	int real_width;
	unsigned char *bmp_line_data; //画像1行分のRGB情報を格納する
	unsigned char header_buf[HEADERSIZE]; //ヘッダを格納する
	unsigned int file_size;
	unsigned int offset_to_data;
	unsigned long info_header_size;
	unsigned int planes;
	unsigned int color;
	unsigned long compress;
	unsigned long data_size;
	long xppm;
	long yppm;

	if((fp = fopen(filename, "wb")) == NULL){
		fprintf(stderr, "Error: %s could not open.", filename);
		return 1;
	}

	real_width = img->width*3 + img->width%4;

	//ここからヘッダ作成
	file_size = img->height * real_width + HEADERSIZE;
	offset_to_data = HEADERSIZE;
	info_header_size = INFOHEADERSIZE;
	planes = 1;
	color = 24;
	compress = 0;
	data_size = img->height * real_width;
	xppm = 1;
	yppm = 1;

	header_buf[0] = 'B';
	header_buf[1] = 'M';
	memcpy(header_buf + 2, &file_size, sizeof(file_size));
	header_buf[6] = 0;
	header_buf[7] = 0;
	header_buf[8] = 0;
	header_buf[9] = 0;
	memcpy(header_buf + 10, &offset_to_data, sizeof(file_size));
	header_buf[11] = 0;
	header_buf[12] = 0;
	header_buf[13] = 0;

	memcpy(header_buf + 14, &info_header_size, sizeof(info_header_size));
	header_buf[15] = 0;
	header_buf[16] = 0;
	header_buf[17] = 0;
	memcpy(header_buf + 18, &img->width, sizeof(img->width));
	memcpy(header_buf + 22, &img->height, sizeof(img->height));
	memcpy(header_buf + 26, &planes, sizeof(planes));
	memcpy(header_buf + 28, &color, sizeof(color));
	memcpy(header_buf + 30, &compress, sizeof(compress));
	memcpy(header_buf + 34, &data_size, sizeof(data_size));
	memcpy(header_buf + 38, &xppm, sizeof(xppm));
	memcpy(header_buf + 42, &yppm, sizeof(yppm));
	header_buf[46] = 0;
	header_buf[47] = 0;
	header_buf[48] = 0;
	header_buf[49] = 0;
	header_buf[50] = 0;
	header_buf[51] = 0;
	header_buf[52] = 0;
	header_buf[53] = 0;

	//ヘッダの書き込み
	fwrite(header_buf, sizeof(unsigned char), HEADERSIZE, fp);

	if((bmp_line_data = (unsigned char *)malloc(sizeof(unsigned char)*real_width)) == NULL){
		fprintf(stderr, "Error: Allocation error.\n");
		fclose(fp);
		return 1;
	}

	//RGB情報の書き込み
	for(i=0; i<img->height; i++){
		for(j=0; j<img->width; j++){
			bmp_line_data[j*3]			=	img->data[(img->height - i - 1)*img->width + j].b;
			bmp_line_data[j*3 + 1]	=	img->data[(img->height - i - 1)*img->width + j].g;
			bmp_line_data[j*3 + 2]			=	img->data[(img->height - i - 1)*img->width + j].r;
		}
		//RGB情報を4バイトの倍数に合わせている
		for(j=img->width*3; j<real_width; j++){
			bmp_line_data[j] = 0;
		}
		fwrite(bmp_line_data, sizeof(unsigned char), real_width, fp);
	}

	free(bmp_line_data);

	fclose(fp);

	return 0;
}

Image *Create_Image(int width, int height)
{
	Image *img;

	if((img = (Image *)malloc(sizeof(Image))) == NULL){
		fprintf(stderr, "Allocation error\n");
		return NULL;
	}

	if((img->data = (Rgb*)malloc(sizeof(Rgb)*width*height)) == NULL){
		fprintf(stderr, "Allocation error\n");
		free(img);
		return NULL;
	}

	img->width = width;
	img->height = height;

	return img;
}

//動的に取得したRGB情報の開放
void Free_Image(Image *img)
{
	free(img->data);
	free(img);
}

void Grayscale(Image *img)
{
	int i, j;
	unsigned int index;
	unsigned char color;

	for(i=0; i<img->height; i++){
		for(j=0; j<img->width; j++){
			index = i*img->width + j;

			color = (img->data[index].r + img->data[index].g + img->data[index].b)/3;
//			printf("%hhu\n" , color);
			img->data[index].r = img->data[index].g = img->data[index].b = color;
		}
	}
}

void Posterization(Image *img)
{
  int i, j;
  unsigned int index;
  unsigned char color;
  int num = 4;
  Grayscale(img);

  for(i=0; i<img->height; i++){
    for(j=0; j<img->width; j++){
      index = i*img->width + j;

      color = (img->data[index].r / (255 / num) ) * (255 / num);

      img->data[index].r =
                img->data[index].g = img->data[index].b = color;
    }
  }
}

void Mirror(Image *img)
{
  int i, j;
  unsigned int index;
  unsigned int index2;
  unsigned char color;

  for(i=0; i<img->height; i++){
    for(j=0; j<img->width; j++){
      index = i*img->width + j;
      if(j < (img->width / 2)) {
        index2 = i*img->width + (img->width - j);
        img->data[index2].r = img->data[index].r * 1;
        img->data[index2].g = img->data[index].g * 1;
        img->data[index2].b = img->data[index].b * 1;
      }
    }
  }
}

void Put(Image *img)
{
  int i,j;
  unsigned int index;

  for(i=0; i<img->height; i++) {
    for(j=0; j<img->width; j++) {
      index = i*img->width + j;
      printf("W:%d, H:%d, R:%d, G:%d, B:%d\n",
              i, j,
              img->data[index].r, img->data[index].g, img->data[index].b);
    }
  }
}

void Template_Matching(Image *img, Image *templat, Image *nccim)
{
  int i, j;
  int ti, tj;
  unsigned int index, tindex, nindex;
  double imgpx, templatepx, color, tn, in, inpro;

  for(i=0; i<nccim->height; i++) {
    for(j=0; j<nccim->width; j++) {
      nindex = i*nccim->width + j;
      inpro = 0;
      in = 0;
      tn = 0;

      for(ti=0; ti < templat->height; ti++) {
        for (tj = 0; tj < templat->width; tj++) {
          tindex = ti*templat->width + tj;
          templatepx = templat->data[tindex].r;

          index = (i+ti-templat->height)*img->width + j + tj - (templat->width - 1);
//          index =  (i+ti-templat->height/2)*img->width+j+tj-templat->width/2;
//          if(0<=index && index<img->width*img->height) {
//            imgpx = img->data[index].r;
//          } else {
//            printf("i:%d, ti:%d, j:%d, tj: %d, index:%d", i, ti, j, tj, index);
//            continue;
//          }
//
          if (i+ti-templat->height > 0 & i+ti-templat->height < img->height &
            j + tj - (templat->width - 1) > 0 & j + tj - (templat->width - 1) < img->width ) {
            imgpx = img->data[index].r;
          } else {
            continue;
          }
          in += pow(imgpx, 2);
          tn += pow(templatepx, 2);
          inpro +=  imgpx * templatepx;
        }
      }

      color = (inpro / sqrt(in * tn)-0.9)*10*255;
//      color = 1;
      nccim->data[nindex].r = nccim->data[nindex].g = nccim->data[nindex].b = (int)color;
    }
  }
}

void NCC(Image *temp, Image *img, Image *out)
{
  int i, j, k, l;
  unsigned int index, index1, index2;
  double color;
  double sumt, sumi, sumit;
  double ncc[out->height][out->width];
  double maxncc = 0;
  for(i = 0; i < out->height; i++){
    for(j = 0; j < out->width; j++){
      sumi = 0, sumt = 0, sumit = 0;
      index = i*out->width+j;
      for(k = 0; k < temp->height; k++){
        for(l = 0; l < temp->width; l++){
          index1 = (i+k-temp->height/2)*img->width+j+l-temp->width/2;
          index2 = k*temp->width+l;
          if(index1 < 0 || index1 >= img->height*img->width){
            continue;
          }
          sumt += pow(temp->data[index2].r, 2);
          sumi += pow(img->data[index1].r, 2);
          sumit += img->data[index1].r*temp->data[index2].r;
        }
      }
      ncc[i][j] = sumit/sqrt(sumi*sumt);
      printf("%d", ncc[i][j]);
      color = (ncc[i][j]-0.9)*10*255;
      out->data[index].r = out->data[index].g = out->data[index].b = (int)color;
      if(ncc[i][j] > maxncc){
        maxncc = ncc[i][j];
      }
    }
  }
  for(i = 0; i < out->height; i++){
    for(j = 0; j < out->width; j++){
      if(ncc[i][j] == maxncc){
        index = i*out->width+j;
        out->data[index].r = 255;
        out->data[index].g = out->data[index].b = 0;
      }
    }
  }
}