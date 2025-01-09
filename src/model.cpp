#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_()
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail())
    {
        std::cerr << "[Model] Failed to load " << filename << "! Aborting\n";
        return;
    }
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
    std::cerr << "[Model] Loaded " << filename << "\n";
    std::cerr << "\tv# " << verts_.size() << " f# " << faces_.size() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
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
Vector3f Model::vert(int iface, int nthvert)
{
    return verts_[faces_[iface][nthvert].x];
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
        if (img.read_tga_file(texfile.c_str()))
            std::cerr << "[Texture] Loaded " << texfile << "\n";
        else
            std::cerr << "[Texture] Failed to load " << texfile << "\n";
        img.flip_vertically();
    }
}

TGAColor Model::diffuse(Vector2f uvf)
{
    Vector2i uv((int)uvf.x, (int)uvf.y);
    return diffusemap_.get(uv.x, uv.y);
}

Vector2f Model::uv(int iface, int nvert)
{
    int idx = faces_[iface][nvert].y;
    return Vector2f(uv_[idx].x * diffusemap_.get_width(), uv_[idx].y * diffusemap_.get_height());
}

Vector3f Model::normal(int iface, int nvert)
{
    int idx = faces_[iface][nvert].z;
    Vector3f out = norms_[idx];
    out.Normalize();
    return out;
}
Vector3f Model::normal(Vector2f uvf)
{
    Vector2i uv((int)(uvf.x * normalmap_.get_width()), (int)(uvf.y * normalmap_.get_height()));
    TGAColor c = normalmap_.get(uv.x, uv.y);
    Vector3f res;
    res.x = (float)c.b / 255.f * 2.f - 1.f;
    res.y = (float)c.g / 255.f * 2.f - 1.f;
    res.z = (float)c.r / 255.f * 2.f - 1.f;
    return res;
}

float Model::specular(Vector2f uvf)
{
    Vector2i uv((int)(uvf.x * specularmap_.get_width()), (int)(uvf.y * specularmap_.get_height()));
    return specularmap_.get(uv.x, uv.y).r / 1.f;
}