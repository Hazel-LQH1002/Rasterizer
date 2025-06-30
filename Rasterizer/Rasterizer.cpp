// Rasterizer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <algorithm>
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
Vec3f light_dir(0.2, 0.15, -1);
Vec3f camera(0, 0, 3);
int width = 800;
int height = 800;
int depth = 255;
Model* model = NULL;

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

Matrix viewport(int x, int y, int w, int h)
{
	Matrix m = Matrix::Identity(4);

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;

	m[0][3] = w / 2.f + x;
	m[1][3] = h / 2.f + y;
	m[2][3] = depth / 2.f;

	return m;

}

Vec3f barycentric(Vec3i& A, Vec3i& B, Vec3i& C, Vec3i& P)
{
	Vec3f s[2];
	for (int i = 0; i < 2; i++)
	{
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}

	Vec3f u = s[0] ^ s[1];

	if (std::abs(u[2]) > 1e-2)
	{
		return Vec3f(1.0f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	}
	return Vec3f(-1, 1, 1);
}

int getX(const Vec2i& a, const Vec2i& b, int y)
{
	if (b.y == a.y) return a.x;
	return a.x + (b.x - a.x) * (y - a.y) / (b.y - a.y);
}

void triangle(Vec3i* pts, Vec2i* uv, float* zbuffer, TGAImage& image, float intensity)
{
	Vec3i t0 = pts[0];
	Vec3i t1 = pts[1];
	Vec3i t2 = pts[2];

	Vec2i uv0 = uv[0];
	Vec2i uv1 = uv[1];
	Vec2i uv2 = uv[2];
	if (t0.y == t1.y && t0.y == t2.y) return;
	//分割成两个三角形
	if (t0.y > t1.y) { std::swap(t0, t1); std::swap(uv0, uv1); }
	if (t0.y > t2.y) { std::swap(t0, t2); std::swap(uv0, uv2); }
	if (t1.y > t2.y) { std::swap(t1, t2); std::swap(uv1, uv2); }
	//用高度做循环控制
	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) {
		//判断属于哪一部分以确定高度
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		//计算当前的比例
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here
		//A表示t0与t2之间的点
		//B表示t0与t1之间的点
		Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
		Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;
		//计算UV
		Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
		Vec2i uvB = second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;
		//保证B在A的右边
		if (A.x > B.x) { std::swap(A, B); }// std::swap(uvA, uvB);}
		//用横坐标作为循环控制，对这一行进行着色
		for (int j = A.x; j <= B.x; j++) {
			//计算当前点在AB之间的比例
			float phi = B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x);
			//计算出当前点的坐标,A，B保存了z轴信息
			Vec3i   P = Vec3f(A) + Vec3f(B - A) * phi;
			Vec2i uvP = uvA + (uvB - uvA) * phi;
			if (P.x < width && P.y < height)
			{
				//计算当前zbuffer下标=P.x+P.y*width
				int idx = P.x + P.y * width;
				//当前点的z大于zbuffer信息，覆盖掉，并更新zbuffer
				if (zbuffer[idx] < P.z) {
					zbuffer[idx] = P.z;
					TGAColor color = model->diffuse(uvP);
					image.set(P.x, P.y, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, 255));
				}
			}
		}
	}
}

void triangleBarycentric(Vec3i* pts, Vec2i* uv, float* zbuffer, TGAImage& image, float intensity)
{
	Vec3i t0 = pts[0];
	Vec3i t1 = pts[1];
	Vec3i t2 = pts[2];

	Vec2i uv0 = uv[0];
	Vec2i uv1 = uv[1];
	Vec2i uv2 = uv[2];

	Vec2i bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2i bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	//确定三角形的边框
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}

	Vec3i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);

			if (bc_screen[0] < 0.0f || bc_screen[1] < 0.0f || bc_screen[2] < 0.0f)
				continue;

			P.z = 0;
			for (int i = 0; i < 3; i++)
			{
				P.z += pts[i][2] * bc_screen[i];
			}

			int idx = int(P.x + P.y * width);
			if (P.x < 0 || P.x >= width || P.y < 0 || P.y >= height) continue;
			if (zbuffer[idx] < P.z) {
				zbuffer[idx] = P.z;
				Vec2i uvP = uv0 * bc_screen.x + uv1 * bc_screen.y + uv2 * bc_screen.z;
				TGAColor color = model->diffuse(uvP);
				image.set(P.x, P.y, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, 255));
			}
			
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
			//画线
			line(x0, y0, x1, y1, image, white);
		}
	}
	image.write_tga_file("output.tga");

}

Vec3f world2screen(const Vec3f& v, const Matrix& viewport, const Matrix& projection) 
{
	std::vector<float> vec{ v.x, v.y, v.z, 1.0f };
	std::vector<float> temp = viewport * projection * vec;
	if (temp[3] != 0) {
		temp[0] /= temp[3];
		temp[1] /= temp[3];
		temp[2] /= temp[3];
	}
	//return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);

	Vec3f result(temp);
	return result;
}

void drawModelTriangleFilled()
{
	//add z buffer
	float* zbuffer = new float[width * height];
	for (int i = 0; i < width * height; i++)
	{
		zbuffer[i] = -std::numeric_limits<float>::max();
	}
	Matrix Projection = Matrix::Identity(4);
	Projection[3][2] = -1.f / camera.z;
	Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

	model = new Model("african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		Vec3i screen_coords[3];
		Vec3f world_coords[3];
		for (int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j]);
			screen_coords[j] = world2screen(v, ViewPort, Projection);
			world_coords[j] = v;
		}
		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		n.normalize();
		float intensity = n * light_dir;
		//intensity = std::max(0.2f, intensity);
		// intensity = std::min(std::abs(intensity), 1.f);
		if (intensity > 0) 
		{
			Vec2i uv[3];
			for (int k = 0; k < 3; k++) {
				uv[k] = model->uv(i, k);
			}
			triangle(screen_coords, uv, zbuffer, image, intensity);
		}
	}
	image.flip_vertically();
	image.write_tga_file("output.tga");
}


int main(int argc, char** argv)
{

	drawModelTriangleFilled();
	return 0;
}