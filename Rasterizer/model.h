#ifndef MODEL_H
#define MODEL_H
#include <vector>
#include "geometry.h"
class Model
{
private:
	std::vector<Vec3f> vertices;
	std::vector<std::vector<int>> faces;

public:
	Model(const std::string filename);

	int nverts() const;
	int nfaces() const;
	Vec3f vert(int i);
	Vec3f vert(int faceIdx, int vertIdx);
	std::vector<int> face(int idx);
};
#endif // !MODEL_H
