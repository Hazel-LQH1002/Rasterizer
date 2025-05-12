// Rasterizer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "tgaimage.h"
#include "model.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
int width = 800;
int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	for (float x = x0; x <= x1; x++)
	{
		float t = (x - x0) / (float)(x1 - x0);
		int y = (1. - t) * y0 + t * y1;
		if (steep)
		{
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}
	}
}


int main(int argc, char** argv)
{
	Model model("african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i); //创建face数组用于保存一个face的三个顶点坐标
		for (int j = 0; j < 3; j++) {
			vec3 v0 = model.vert(face[j]);
			vec3 v1 = model.vert(face[(j + 1) % 3]);
			//根据顶点v0和v1画线
			//先要进行模型坐标到屏幕坐标的转换。  (-1,-1)对应(0,0)   (1,1)对应(width,height)
			int x0 = (v0.x + 1.) * width / 2.;
			int y0 = (1. - v0.y) * height / 2.;
			int x1 = (v1.x + 1.) * width / 2.;
			int y1 = (1. - v1.y) * height / 2.;
			//画线
			line(x0, y0, x1, y1, image, white);
		}
	}
	image.write_tga_file("output.tga");
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
