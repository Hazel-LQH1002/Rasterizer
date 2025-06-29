// Rasterizer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <vector>
#include <algorithm>
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
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

int getX(const Vec2i& a, const Vec2i& b, int y)
{
	if (b.y == a.y) return a.x;
	return a.x + (b.x - a.x) * (y - a.y) / (b.y - a.y);
}

void triangle(const Vec2i &t0, const Vec2i& t1, const Vec2i& t2, TGAImage& image, TGAColor color)
{
	std::vector<Vec2i> sortedVec = { t0,t1,t2 };
	std::sort(sortedVec.begin(), sortedVec.end(), [](const Vec2i& v1, const Vec2i& v2)
		{
			return v1.y < v2.y;
		});

	for (int y = 0; y < sortedVec[2].y - sortedVec[0].y; y++)
	{
		int Ax = getX(sortedVec[0], sortedVec[2], y + sortedVec[0].y);
		bool upperPart = y > (sortedVec[1].y - sortedVec[0].y) || sortedVec[1].y == sortedVec[0].y;
		int Bx;
		if (!upperPart)
		{
			Bx = getX(sortedVec[0], sortedVec[1], y + sortedVec[0].y);
		}
		else
		{
			Bx = getX(sortedVec[1], sortedVec[2], y + sortedVec[0].y);
		}
		if (Ax > Bx) std::swap(Ax, Bx);
		for (int x = Ax; x < Bx; x++)
		{
			image.set(x, sortedVec[0].y + y, color);
		}
	}
}

void drawModelLine()
{
	Model model("african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		for (int j = 0; j < 3; j++) {
			Vec3f v0 = model.vert(face[j]);
			Vec3f v1 = model.vert(face[(j + 1) % 3]);

			int x0 = (v0.x + 1.) * width / 2.;
			int y0 = (1. - v0.y) * height / 2.;
			int x1 = (v1.x + 1.) * width / 2.;
			int y1 = (1. - v1.y) * height / 2.;
			//»­Ïß
			line(x0, y0, x1, y1, image, white);
		}
	}
	image.write_tga_file("output.tga");

}

void drawModelTriangleFilled()
{
	Model model("african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		Vec2i screen_coords[3];
		for (int j = 0; j < 3; j++) {
			Vec3f v = model.vert(face[j]);
			screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
		}
		triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, white);
	}
}
int main(int argc, char** argv)
{
	TGAImage image(200, 200, TGAImage::RGB);
	Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
	triangle(t0[0], t0[1], t0[2], image, red);
	triangle(t1[0], t1[1], t1[2], image, white);
	triangle(t2[0], t2[1], t2[2], image, green);
	image.flip_vertically();
	image.write_tga_file("output.tga");
	return 0;
}