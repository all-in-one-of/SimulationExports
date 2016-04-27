#ifndef VRAYEXPORTER_H_
#define VRAYEXPORTER_H_
#include <fstream>
#include <string>
#include <vector>
#include <ngl/Colour.h>
#include <ngl/Mat4.h>

class VRayExporter
{
  public :
    VRayExporter(const std::string &_fname);
    ~VRayExporter();
    void close();
    void open(const std::string &_fname);
    void renderView(const ngl::Mat4 &_tx);
    void setWorldUp(const ngl::Vec3 &_up);
    void setImageSize(int _w, int _h);
    void setFOV(float _fov);
    void setBGColour(const ngl::Colour &_c);
    void setBGColour(float _r, float _g, float _b);
    void writeColour(const ngl::Colour &_v);

    void writeVector(const ngl::Vec3 &_v);
    void writeStaticMesh(const std::string &_name, const std::vector<ngl::Vec3> &_verts,
                         const std::vector<ngl::Vec3> &_faces,
                         const std::vector<ngl::Vec3> &_normals,
                         const std::vector<ngl::Vec3> &_faceNormals);
    void writeStaticMeshWithChannel(const std::string &_name, const std::vector<ngl::Vec3> &_verts,
                         const std::vector<ngl::Vec3> &_faces,
                         const std::vector<ngl::Vec3> &_normals,
                         const std::vector<ngl::Vec3> &_faceNormals,
                         const std::vector<ngl::Vec3> &_chan,
                         const std::vector<ngl::Vec3> &_chanIndex );

    void writeNode(const std::string &_nodeName, const std::string &_geoName,
                   const std::string &_materialName,
                   const ngl::Mat4 &_tx);
    void writeRawDataToStream(const std::string &_d);

    void writeObj(const std::string &_name, const std::string &_objFile);

    void includeFile(const std::string &_fname);
    enum class  PointModes : int {multipoints=3, multistreak=5, points=6, spheres=7, sprites=8,streak=9};

    void writeGeomParticle(const std::string &_name, const std::vector<ngl::Vec3> &_pos, const std::vector<ngl::Colour> &_colours,PointModes _mode=PointModes::spheres);

  private :

    void mat4ToVrayTransform(const ngl::Mat4 &_m);
    std::ofstream m_stream;
    bool m_isOpen=false;
    void removeCharFromStream(long _amount=1);
    void listVector(const std::string &_name, const std::vector<ngl::Vec3> &_list);
    void listColour(const std::string &_name, const std::vector<ngl::Colour> &_list);

};




#endif
