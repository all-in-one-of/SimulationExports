#include "VRayExporter.h"
#include "VRayHelperFuncs.h"
#include <iostream>
#include <ngl/Mat3.h>
#include <ngl/Vec3.h>
#include <ngl/Obj.h>
#include <ngl/NGLStream.h>
VRayExporter::VRayExporter(const std::string &_fname)
{
  open(_fname);
}

VRayExporter::~VRayExporter()
{
  if(m_isOpen ==true)
    close();
}

void VRayExporter::close()
{
  m_stream.close();
}

void VRayExporter::open(const std::string &_fname)
{
  m_stream.open(_fname.c_str(),std::ios::out);
  if (!m_stream.is_open())
  {
    std::cerr<<"problems Opening File\n";
  }
  m_stream<<"// scene file generated using ngl VRayExporter\n";

  m_isOpen=true;
}


void VRayExporter::mat4ToVrayTransform(const ngl::Mat4 &_m)
{
  ngl::Mat4 m=_m;

  m_stream<<"transform=Transform(Matrix(";
  m_stream<<"Vector("<<m.m_00<<","<<m.m_10<<","<<m.m_20<<"),\n";
  m_stream<<"Vector("<<m.m_01<<","<<m.m_11<<","<<m.m_21<<"),\n";
  m_stream<<"Vector("<<m.m_02<<","<<m.m_12<<","<<m.m_22<<")),\n";
  m_stream<<"Vector("<<m.m_30<<","<<m.m_31<<","<<-m.m_32<<"));\n";
  std::cout<<m<<"\n";
}




void VRayExporter::renderView(const ngl::Mat4 &_tx)
{
  ngl::Mat3 t(_tx);
  t.inverse();
  ngl::Vec3 d(_tx.m_30,_tx.m_31,_tx.m_32);
  ngl::Vec3 pos=-d*_tx;
  m_stream<<"RenderView renderView {\n";
  m_stream<<"transform=Transform(Matrix(";
  m_stream<<"Vector("<<t.m_00<<","<<t.m_10<<","<<t.m_20<<"),\n";
  m_stream<<"Vector("<<t.m_01<<","<<t.m_11<<","<<t.m_21<<"),\n";
  m_stream<<"Vector("<<t.m_02<<","<<t.m_12<<","<<t.m_22<<")),\n";
 // m_stream<<"Vector("<<_tx.m_30<<","<<_tx.m_31<<","<<-_tx.m_32<<"));\n";
  m_stream<<"Vector("<<-pos.m_x<<","<<-pos.m_y<<","<<pos.m_z<<"));\n";

  m_stream<<"}\n";
}

void VRayExporter::setWorldUp(const ngl::Vec3 &_up)
{
  m_stream<<"SettingsUnitsInfo vraySettingsUnitsInfo {\n";
   m_stream<<"scene_upDir=";
   writeVector(_up);
   m_stream<<";\n}\n";
}

void VRayExporter::setImageSize(int _w, int _h)
{
  StartGroup begin(&m_stream,"SettingsOutput");
  writePair(m_stream,"img_width",_w);
  writePair(m_stream,"img_height",_h);
}

void VRayExporter::setFOV(float _fov)
{
  std::cout<<"Fov is "<<_fov<<'\n';
  StartGroup cam(&m_stream,"SettingsCamera");
  writePair(m_stream,"fov",_fov);
}

void VRayExporter::setBGColour(float _r, float _g, float _b)
{
  setBGColour(ngl::Colour(_r,_g,_b));
}

void VRayExporter::setBGColour(const ngl::Colour &_c)
{
  StartGroup start(&m_stream,"SettingsEnvironment");
  writePair(m_stream,"bg_color",_c);
}

void VRayExporter::writeVector(const ngl::Vec3 &_v)
{
  m_stream<<"Vector("<<_v.m_x<<","<<_v.m_y<<","<<_v.m_z<<")";
}
void VRayExporter::writeColour(const ngl::Colour &_v)
{
  m_stream<<"Color("<<_v.m_r<<","<<_v.m_g<<","<<_v.m_b<<")";
}
void VRayExporter::writeStaticMesh(const std::string &_name,
                     const std::vector<ngl::Vec3> &_verts,
                     const std::vector<ngl::Vec3> &_faces,
                     const std::vector<ngl::Vec3> &_normals,
                     const std::vector<ngl::Vec3> &_faceNormals)
{
  m_stream<<"GeomStaticMesh "<<_name <<"{\n";
  listVector("vertices",_verts);
  listVector("faces",_faces);
  listVector("normals",_normals);
  listVector("faceNormals",_faceNormals);
  m_stream<<"}\n";
}

void VRayExporter::writeStaticMeshWithChannel(const std::string &_name,
                     const std::vector<ngl::Vec3> &_verts,
                     const std::vector<ngl::Vec3> &_faces,
                     const std::vector<ngl::Vec3> &_normals,
                     const std::vector<ngl::Vec3> &_faceNormals,
                    const std::vector<ngl::Vec3> &_chan,
                    const std::vector<ngl::Vec3> &_chanIndex )

{
  m_stream<<"GeomStaticMesh "<<_name <<"{\n";
  listVector("vertices",_verts);
  listVector("faces",_faces);
  listVector("normals",_normals);
  listVector("faceNormals",_faceNormals);


  m_stream<<"map_channels=List(\nList(1,\nList(\n";
  for(auto n : _chan)
  {
    writeVector(n);
    m_stream<<",";
  }
  // get rid of the last comma
  removeCharFromStream();
  m_stream<<"),\nList(\n";
  for(auto n : _chanIndex)
  {
    writeVector(n);
    m_stream<<",";
  }
  // get rid of last ,
  removeCharFromStream();
  m_stream<<")));";
  m_stream<<"}\n";
}

void VRayExporter::writeNode(
               const std::string &_nodeName,
               const std::string &_geoName,
               const std::string &_materialName,
               const ngl::Mat4 &_tx)
{
  m_stream<<"Node "<<_nodeName<<"{\n";
  m_stream<<"geometry="<<_geoName<<";\n";
  m_stream<<"material="<<_materialName<<";\n";
  mat4ToVrayTransform(_tx);
  m_stream<<"}\n";
}

void VRayExporter::writeRawDataToStream(const std::string &_d)
{
  m_stream<<_d;
}

void VRayExporter::writeObj(const std::string &_name,const std::string &_objFile)
{
  std::vector<ngl::Vec3> verts;
  std::vector<ngl::Vec3> vertIndex;
  std::vector<ngl::Vec3> normals;
  std::vector<ngl::Vec3> uv;
  ngl::Obj mesh(_objFile,false);

// get the obj data so we can process it locally
  std::vector <ngl::Vec3> overts=mesh.getVertexList();
  std::vector <ngl::Face> ofaces=mesh.getFaceList();
  std::vector <ngl::Vec3> onormals=mesh.getNormalList();
  std::vector <ngl::Vec3> ouv=mesh.getTextureCordList();

  int vIndex=0;

  // loop for each of the faces
  for(auto f : ofaces)
  {
    // now for each triangle in the face (remember we ensured tri above)
    for(unsigned int j=0;j<3;++j)
    {
      verts.push_back(overts[f.m_vert[j]]);
      normals.push_back(onormals[f.m_norm[j]]);
      uv.push_back(ouv[f.m_tex[j]]);
    }
    vertIndex.push_back(ngl::Vec3(vIndex,vIndex+1,vIndex+2));
    vIndex+=3;
  }

  writeStaticMeshWithChannel(_name,verts,vertIndex,normals,vertIndex,uv,vertIndex);
}


void VRayExporter::includeFile(const std::string &_fname)
{
  std::ifstream includeFile(_fname);
  if (!includeFile.is_open())
  {
   std::cerr<<"File not found "<<_fname.c_str()<<"\n";
   exit(EXIT_FAILURE);
  }
  m_stream<<includeFile.rdbuf();
}

void VRayExporter::removeCharFromStream(long _amount)
{
  // get rid of last ,
  long pos = m_stream.tellp();
  m_stream.seekp (pos-_amount);
}

void VRayExporter::listVector(const std::string &_name,const std::vector<ngl::Vec3> &_list)
{
  m_stream<<_name<<"=ListVector(\n";
  for(auto n : _list)
  {
    writeVector(n);
    m_stream<<",\n";
  }
  removeCharFromStream(2);
  m_stream<<");\n";
}

void VRayExporter::listColour(const std::string &_name, const std::vector<ngl::Colour> &_list)
{
  m_stream<<_name<<"=ListColor(\n";
  for(auto n : _list)
  {
    writeColour(n);
    m_stream<<",\n";
  }
  removeCharFromStream(2);
  m_stream<<");\n";
}

void VRayExporter::writeGeomParticle(const std::string &_name, const std::vector<ngl::Vec3> &_pos, const std::vector<ngl::Colour> &_colours, PointModes _mode)
{
  m_stream<<"GeomParticleSystem "<<_name <<"{\n";
  writePair(m_stream,"render_type",static_cast<int>(_mode )); // default sphere
  listVector("positions",_pos);
  listColour("colors",_colours);
  m_stream<<"\n}\n";

}

