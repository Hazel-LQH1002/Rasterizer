#include <fstream>
#include <sstream>
#include "model.h"

Model::Model(const std::string filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) 
    {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec3 v;
            for (int i : {0, 1, 2}) iss >> v[i];
            vertices.push_back(v);
        }
        else if (!line.compare(0, 2, "f ")) {
            //int v1, v2, v3 = 0;
            //iss >> trash;
            //std::vector<vec3> tempVec;
            //while (iss >> v1 >> trash >> v2 >> trash >> v3) {
            //    vec3 tempVert = vec3(v1 - 1, v2 - 1, v3 - 1);
            //    tempVec.push_back(tempVert);
            //}
            //faces.push_back(tempVec);
            iss >> trash;  // skip the 'f'
            std::vector<int> face;
            std::string vertex_str;
            while (iss >> vertex_str) {
                std::istringstream vss(vertex_str);
                std::string v_idx_str;
                std::getline(vss, v_idx_str, '/');  // only take the vertex index
                int v_idx = std::stoi(v_idx_str);
                face.push_back(v_idx - 1);  // OBJ index is 1-based
            }
            if (face.size() >= 3) {
                faces.push_back(face);  // add one face (triangle or more)
            }
        }
    }
    std::cerr << "# v# " << nverts() << " f# " << nfaces() << std::endl;
}

int Model::nverts() const { return vertices.size(); }
int Model::nfaces() const { return faces.size(); }

vec3 Model::vert(int i)
{
    return vertices[i];
}

vec3 Model::vert(int faceIdx, int vertIdx)
{
    return vertices[faces[faceIdx][vertIdx]];
}

std::vector<int> Model::face(int idx)
{
    return faces[idx];
}

