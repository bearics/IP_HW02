#define _CRT_SECURE_NO_DEPRECATE

#define PI 3.141592653589793238462643383279

#include <stdio.h>
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

unsigned char** memAllocate2D(int sx, int sy)
{
	unsigned char** data = NULL;

	data = new unsigned char*[sy];

	for (int i = 0; i < sy; i++)
	{
		data[i] = new unsigned char[sx];
	}

	return data;
}

void memFree2D(unsigned char **data, int sy)
{
	for (int n = 0; n < sy; n++)
	{
		delete[] data[n];
	}
	delete[] data;
}

unsigned char** readFile(string path, int sx, int sy)
{
	// read file function

	FILE* fp;

	unsigned char** data = memAllocate2D(sx, sy);

	fp = fopen(path.c_str(), "rb");

	if (fp != NULL)
	{	// succeed in opening file
		for (int i = 0; i < sy; i++)
		{
			fread(data[i], sizeof(unsigned char), sx, fp);
		}
		fclose(fp);
	}
	else
	{	// fail to open file
		printf("Cannot open file\n");
		return NULL;
	}

	return data;
}

int writeFile(string path, unsigned char** data, int sx, int sy)
{
	FILE* fp;
	fp = fopen(path.c_str(), "wb+");

	for (int i = 0; i < sy; i++)
	{
		fwrite(data[i], sizeof(unsigned char), sx, fp);
	}
	fclose(fp);
	memFree2D(data, (int)sy);

	return 0;
}



unsigned char bilinearInterpolation(unsigned char** data, double ori_x, double ori_y, int sx, int sy)
{
	unsigned char m[2][2];

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			int py = floor(ori_y) + i < sy ? floor(ori_y) + i : floor(ori_y);
			int px = floor(ori_x) + j < sx ? floor(ori_x) + j : floor(ori_x);
			m[i][j] = data[py][px];
		}
	}

	double d1 = ori_x - floor(ori_x);
	double d2 = 1 - d1;
	double d3 = ori_y - floor(ori_y);
	double d4 = 1 - d3;

	return (unsigned char)(d4 * (d2 * (int)m[0][0] + d1 * (int)m[0][1]) + d3 * (d2 * (int)m[1][0] + d1 * (int)m[1][1]));
}


void rotateRAW(unsigned char** data, int sx, int sy, double degree)
{
	unsigned char** outData = memAllocate2D(sx, sy);

	double ori_x, ori_y;
	unsigned char pixel;
	double rad = -degree * PI / 180.0;
	double cc = cos(rad), ss = sin(-rad);
	double xc = (double)sx / 2.0, yc = (double)sy / 2.0;	// x,y 's center

	for (int y = 0; y < sy; y++)
	{
		for (int x = 0; x < sx; x++)
		{
			ori_x = xc + ((double)y - yc) * ss + ((double)x - xc) * cc;
			ori_y = yc + ((double)y - yc) * cc - ((double)x - xc) * ss;
			pixel = (unsigned char)0;
			if (((ori_y >= 0 && ori_y < sy) && (ori_x >= 0 && ori_x < sx)))
				pixel = bilinearInterpolation(data, ori_x, ori_y, 256, 256);
			outData[y][x] = pixel;
		}
	}

	string path = "c:\\lena256_rotate_" + to_string((int)(degree)) + ".raw";

	writeFile(path, outData, (int)sx, (int)sy);
	cout << "finish to write file\n";

}

void histogramEqualRAW(unsigned char** data, int sx, int sy)
{
	double* hist_Eq = new double[256];
	unsigned char** outData = memAllocate2D(sx, sy);
	int sum = 0;

	for (int i = 0; i < 256; i++)
	{
		hist_Eq[i] = 0;
	}

	for (int y = 0; y < sy; y++)
	{
		for (int x = 0; x < sx; x++)
		{
			hist_Eq[data[y][x]]++;
		}
	}

	for (int i = 0; i < 256; i++)
	{
		if (hist_Eq[i] != 0)
		{
			sum += hist_Eq[i];
			hist_Eq[i] = (double)sum / (sx * sy);
		}
	}

	for (int y = 0; y < sy; y++)
	{
		for (int x = 0; x < sx; x++)
		{
			outData[y][x] = (hist_Eq[data[y][x]] * 255);
		}
	}


	string path = "c:\\lena256_histogramEqualization.raw";

	writeFile(path, outData, (int)sx, (int)sy);
	cout << "finish to write file\n";
}

unsigned char** Padding(unsigned char** In, int sy, int sx, int filterSize)
{
	int paddingSize = (int)(filterSize / 2);
	unsigned char** Pad = memAllocate2D(sy + 2 * paddingSize, sx + 2 * paddingSize);

	for (int h = 0; h < sy; h++)
	{
		for (int w = 0; w < sx; w++)
		{
			Pad[h + paddingSize][w + paddingSize] = In[h][w];
		}
	}
	for (int h = 0; h < paddingSize; h++)
	{
		for (int w = 0; w < sx; w++)
		{
			Pad[h][w + paddingSize] = In[0][w];
			Pad[h + (sy - 1) + paddingSize][w + paddingSize] = In[sy - 1][w];
		}
	}

	for (int h = 0; h < sy; h++)
	{
		for (int w = 0; w < paddingSize; w++)
		{
			Pad[h + paddingSize][w] = In[h][0];
			Pad[h + paddingSize][w + (sx - 1) + paddingSize] = In[h][sx - 1];
		}
	}

	for (int h = 0; h < paddingSize; h++)
	{
		for (int w = 0; w < paddingSize; w++)
		{
			Pad[h][w] = In[0][0];
			Pad[h + (sy - 1) + paddingSize][w] = In[sy - 1][0];
			Pad[h][w + (sx - 1) + paddingSize] = In[0][sx - 1];
			Pad[h + (sy - 1) + paddingSize][w + (sx - 1) + paddingSize] = In[sy - 1][sx - 1];
		}
	}

	return Pad;
}

void filterRAW(unsigned char** data, int sx, int sy, double** filter, int filterSize, string name)
{
	unsigned char** padData = Padding(data, sy, sx, filterSize);
	unsigned char** outData = memAllocate2D(sx, sy);
	int padSize = (int)(filterSize / 2);
	int nsx = sx + 2 * padSize;
	int nsy = sy + 2 * padSize;

	for (int y = 0; y < nsy - 2 * padSize; y++)
	{
		for (int x = 0; x < nsx - 2 * padSize; x++)
		{
			double res = 0;
			for (int fy = 0; fy < filterSize; fy++)
			{
				for (int fx = 0; fx < filterSize; fx++)
				{
					res += filter[fy][fx] * padData[y + fy][x + fx];
				}
			}
			if (res > 255) res = 255;
			else if (res < 0) res = 0;

			outData[y][x] = (unsigned char)res;
		}

	}

	string path = "c:\\lena256_filter_" + name + ".raw";
	writeFile(path, outData, (int)sx, (int)sy);
	cout << "finish to write file" << endl;

}

void averageFilter9Raw(unsigned char** data, int sx, int sy)
{
	double** filter = NULL;

	filter = new double*[3];

	for (int i = 0; i < 3; i++)
	{
		filter[i] = new double[3];
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			filter[j][i] = 1.0 / 9.0;
		}
	}

	filterRAW(data, sx, sy, filter, 3, "average9");


}

void averageFilter49Raw(unsigned char** data, int sx, int sy)
{
	double** filter = NULL;

	filter = new double*[7];

	for (int i = 0; i < 7; i++)
	{
		filter[i] = new double[3];
	}

	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			filter[j][i] = 1.0 / 49.0;
		}
	}

	filterRAW(data, sx, sy, filter, 7, "average49");
}

void smoothFilter(unsigned char** data, int sx, int sy)
{
	double** filter = NULL;
	double inputFilter[3][3] = {
		1,2,1,
		2,4,2,
		1,2,1
	};
	
	filter = new double*[3];

	for (int i = 0; i < 3; i++)
	{
		filter[i] = new double[4];
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			filter[j][i] = inputFilter[j][i] / 16.0;
		}
	}

	filterRAW(data, sx, sy, filter, 3, "smooth");
}

void SharpeningFilter(unsigned char** data, int sx, int sy)
{
	double** filter = NULL;
	double inputFilter[3][3] = {
		0, -1, 0,
		-1, 5, -1,
		0, -1, 0
	};

	filter = new double*[3];

	for (int i = 0; i < 3; i++)
	{
		filter[i] = new double[4];
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			filter[i][j] = inputFilter[i][j];
		}
	}

	filterRAW(data, sx, sy, filter, 3, "sharpening");
}


void medianFilter(unsigned char** data, int sx, int sy, int filterSize, string name)
{
	double** filter = NULL;

	int padSize = (int)(filterSize / 2);
	int nsx = sx + 2 * padSize;
	int nsy = sy + 2 * padSize;

	unsigned char* order = new unsigned char[filterSize*filterSize];

	unsigned char** padData = Padding(data, sy, sx, filterSize);
	unsigned char** outData = memAllocate2D(sx, sy);

	filter = new double*[filterSize];

	for (int i = 0; i < filterSize; i++)
	{
		filter[i] = new double[filterSize+1];
	}

	for (int y = 0; y < nsy - 2 * padSize; y++)
	{
		int count = 0;
		for (int x = 0; x < nsx - 2 * padSize; x++)
		{
			for (int fy = 0; fy < filterSize; fy++)
			{
				for (int fx = 0; fx < filterSize; fx++)
				{
					order[count++] = padData[y + fy][x + fx];
				}
			}
			sort(order, order + (filterSize*filterSize));
			outData[y][x] = order[(filterSize*filterSize) / 2];
			count = 0;
		}		
	}

	string path = "c:\\lena256_filter_median_" + to_string(filterSize) + "_ " + name + ".raw";

	writeFile(path, outData, (int)sx, (int)sy);
	cout << "finish to write file\n";
}



int main(void)
{
	unsigned char **input = NULL;
	unsigned char **lena256 = NULL;
	unsigned char **filter1 = NULL;
	unsigned char **lena256_n5 = NULL;
	unsigned char **lena256_n10 = NULL;
	unsigned char **lena256_n25 = NULL;
	unsigned char **out_data = memAllocate2D(256, 256);

	input = readFile("c:\\input.raw", 256, 256);
	histogramEqualRAW(input, 256, 256);

	lena256 = readFile("c:\\lena256.raw", 256, 256);
	averageFilter9Raw(lena256, 256, 256);
	averageFilter49Raw(lena256, 256, 256);
	smoothFilter(lena256, 256, 256);

	filter1 = readFile("c:\\lena256_filter_average9.raw", 256, 256);
	SharpeningFilter(filter1, 256, 256);

	lena256_n5 = readFile("c:\\lena256_n5.raw", 256, 256);
	lena256_n10 = readFile("c:\\lena256_n10.raw", 256, 256);
	lena256_n25 = readFile("c:\\lena256_n25.raw", 256, 256);
	medianFilter(lena256_n5, 256, 256, 3, "n5");
	medianFilter(lena256_n10, 256, 256, 3, "n10");
	medianFilter(lena256_n25, 256, 256, 3, "n25");

	return 0;
}
