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
//Vec3f light_dir(0.2, 0.15, -1).normalize();
//Vec3f light_dir = Vec3f(0.2, 0.15, -1).normalize();
Vec3f light_dir = Vec3f(0, -1, -1).normalize();
Vec3f camera(0, 0, 3);
int width = 800;
int height = 800;
int depth = 255;
Model* model = NULL;
Vec3f up = Vec3f(0.f, 1.f, 0.f);
Vec3f center = Vec3f(0.f, 0.f, 1.f);
Vec3f eye = Vec3f(2.f, 1.f, 3.f);

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
	Matrix m = Matrix::identity(4);

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
void triangle(Vec3i* pts, Vec3f* norms, Vec2i* uv, float* ity, float* dist, int* zbuffer, TGAImage& image)
{
	Vec3i t0 = pts[0];
	Vec3i t1 = pts[1];
	Vec3i t2 = pts[2];

	Vec3f n0 = norms[0];
	Vec3f n1 = norms[1];
	Vec3f n2 = norms[2];

	Vec2i uv0 = uv[0];
	Vec2i uv1 = uv[1];
	Vec2i uv2 = uv[2];

	float ity0 = ity[0];
	float ity1 = ity[1];
	float ity2 = ity[2];

	float dis0 = dist[0];
	float dis1 = dist[1];
	float dis2 = dist[2];
	if (t0.y == t1.y && t0.y == t2.y) return;
	//分割成两个三角形
	if (t0.y > t1.y) { std::swap(t0, t1); std::swap(uv0, uv1); std::swap(ity0, ity1);}
	if (t0.y > t2.y) { std::swap(t0, t2); std::swap(uv0, uv2); std::swap(ity0, ity2);}
	if (t1.y > t2.y) { std::swap(t1, t2); std::swap(uv1, uv2); std::swap(ity1, ity2);}


	//用高度做循环控制
	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
		//计算A,B两点的坐标
		Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
		Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;
		//计算A,B两点的光照强度
		float ityA = ity0 + (ity2 - ity0) * alpha;
		float ityB = second_half ? ity1 + (ity2 - ity1) * beta : ity0 + (ity1 - ity0) * beta;
		//计算UV
		Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
		Vec2i uvB = second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;
		//计算UV
		Vec3f normA = n0 + (n2 - n0) * alpha;
		Vec3f normB = second_half ? n1 + (n2 - n1) * beta : n0 + (n1 - n0) * beta;
		//计算距离
		float disA = dis0 + (dis2 - dis0) * alpha;
		float disB = second_half ? dis1 + (dis2 - dis1) * beta : dis0 + (dis1 - dis0) * beta;
		if (A.x > B.x) { std::swap(A, B); std::swap(ityA, ityB); }
		//x坐标作为循环控制
		for (int j = A.x; j <= B.x; j++) {
			float phi = B.x == A.x ? 1. : (float)(j - A.x) / (B.x - A.x);
			//计算当前需要绘制点P的坐标，光照强度
			Vec3i    P = Vec3f(A) + Vec3f(B - A) * phi;
			float ityP = ityA + (ityB - ityA) * phi;
			ityP = std::min(1.f, std::abs(ityP) + 0.01f);
			//Vec3f norm = (normA + (normB - normA) * phi);
			//ityP = std::max(0.f, norm * light_dir);
			Vec2i uvP = uvA + (uvB - uvA) * phi;
			float disP = disA + (disB - disA) * phi;
			int idx = P.x + P.y * width;
			//边界限制
			if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) continue;
			if (zbuffer[idx] < P.z) {
				zbuffer[idx] = P.z;
				TGAColor color = model->diffuse(uvP);
				image.set(P.x, P.y, TGAColor(color.bgra[2], color.bgra[1], color.bgra[0]) * ityP * (20.f / std::pow(disP, 2.f)));
				//image.set(P.x, P.y, TGAColor(255,255,255)* ityP);
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
				image.set(P.x, P.y, TGAColor(color.bgra[2] * intensity, color.bgra[1] * intensity, color.bgra[0] * intensity, 255));
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

Matrix rotateAndTranslateObj(Vec3f& eyes, Vec3f& centre)
{
	Vec3f z = (eyes - centre).normalize();
	Vec3f x = (up ^ z).normalize();
	Vec3f y = (z ^ x).normalize();
	Matrix rotation = Matrix::identity(4);
	Matrix translation = Matrix::identity(4);
	for (int i = 0; i < 3; i++)
	{
		rotation[i][3] = -centre[i];

		rotation[0][i] = x[i];
		rotation[1][i] = y[i];
		rotation[2][i] = z[i];
	}
	Matrix res = rotation * translation;
	return res;
}

void drawModelTriangleFilled()
{
	//add z buffer
	int* zbuffer = new int[width * height];
	for (int i = 0; i < width * height; i++)
	{
		zbuffer[i] = std::numeric_limits<int>::min();
	}

	Matrix Projection = Matrix::identity(4);
	Projection[3][2] = -1.f / (eye - center).norm();

	Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

	model = new Model("african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);

	Vec3f zAxis = Vec3f(0.f, 0.f, 1.f);
	Matrix viewRotTran = rotateAndTranslateObj(eye, zAxis);

	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		Vec3i screen_coords[3];
		Vec3f world_coords[3];
		Vec3f norms[3];
		float intensity[3];
		float distance[3];
		for (int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j]);
			Matrix m_v = viewRotTran * Matrix(v);
			screen_coords[j] = Vec3f(ViewPort * Projection * m_v);
			Vec3f new_v = Vec3f(m_v);
			//screen_coords[j] = Vec3f(ViewPort * Projection * m_v);
			//screen_coords[j] = world2screen(v, ViewPort, Projection);
			world_coords[j] = new_v;
			norms[j] = model->norm(i, j);
			intensity[j] = model->norm(i, j) * light_dir;
			distance[j] = std::pow((std::pow(new_v.x - eye.x, 2.0f) + std::pow(new_v.y - eye.y, 2.0f) + std::pow(new_v.z - eye.z, 2.0f)), 0.5f);
		}

		Vec2i uv[3];
		for (int k = 0; k < 3; k++) {
			uv[k] = model->uv(i, k);
		}
		triangle(screen_coords, norms, uv, intensity, distance, zbuffer, image);

	}
	image.flip_vertically();
	image.write_tga_file("output.tga");
}


int main(int argc, char** argv)
{

	drawModelTriangleFilled();
	return 0;
}
