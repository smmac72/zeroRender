#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_()
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v "))
        {
            iss >> trash;
            Vector3f v;
            iss >> v.x;
            iss >> v.y;
            iss >> v.z;
            verts_.push_back(v);
        }
        else if (!line.compare(0, 2, "f "))
        {
            iss >> trash;
            std::vector<Vector3i> face;
            Vector3i sub;
            while (iss >> sub.x >> trash >> sub.y >> trash >> sub.z)
            {
                sub.x--;
                sub.y--;
                sub.z--;
                face.push_back(sub);
            }
            faces_.push_back(face);
        }
        else if (!line.compare(0, 3, "vn "))
        {
            iss >> trash >> trash;
            Vector3f norm;
            iss >> norm.x;
            iss >> norm.y;
            iss >> norm.z;
            norms_.push_back(norm);
        }
        else if (!line.compare(0, 3, "vt "))
        {
            iss >> trash >> trash;
            Vector2f uv;
            iss >> uv.x;
            iss >> uv.y;
            uv_.push_back(uv);
        }
    }
    //std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
    load_texture(filename, "_diffuse.tga", diffusemap_);
}

Model::~Model()
{
}

int Model::nverts()
{
    return (int)verts_.size();
}

int Model::nfaces()
{
    return (int)faces_.size();
}

Vector3f Model::vert(int i)
{
    return verts_[i];
}
std::vector<int> Model::face(int idx)
{
    std::vector<int> face;
    for (int i = 0; i < (int)faces_[idx].size(); i++)
        face.push_back(faces_[idx][i].x);
    return face;
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage& img)
{
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot != std::string::npos)
    {
        texfile = texfile.substr(0, dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}

TGAColor Model::diffuse(Vector2i uv)
{
    return diffusemap_.get(uv.x, uv.y);
}

Vector2f Model::uv(int iface, int nvert)
{
    int idx = faces_[iface][nvert].y;
    return Vector2f(uv_[idx].x * diffusemap_.get_width(), uv_[idx].y * diffusemap_.get_height());
}