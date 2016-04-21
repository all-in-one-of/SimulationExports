#include "VRayExporter.h"
#include <ngl/Util.h>
#include <ngl/NGLStream.h>
#include <ngl/Transformation.h>
void createCube(VRayExporter &_scene);


int main()
{

  char fname[50];
  int frame=0;
  for(int i=0; i<1; i+=10)
  {


  sprintf(fname,"scenes/cube.%03d.vrscene",frame++);
  std::cout<<"Writing File "<<fname<<'\n';
  VRayExporter scene(fname);
  scene.setWorldUp(ngl::Vec3::up());

  ngl::Mat4 view=ngl::lookAt(ngl::Vec3(0,0.5,3.2),
                             ngl::Vec3(0,0,0),
                             ngl::Vec3(0,1,0));

  scene.renderView(view);
  scene.setImageSize(600,450);
  scene.setFOV(ngl::radians(45.0));
  scene.setBGColour(0.4,0.4,0.4);
   //std::system("render.sh");

  const std::string lights=R"(

// BRDFDiffuse Box01_brdf {
//   color=Color(1.0, 0, 0);
//   transparency=Color(0.0, 0.0, 0.0);
// }

// LightOmni Omni01 {
//   color=Color(1,1,1)*100;
//   transform=Transform(Matrix(Vector(1.0, 0.0, 0.0), Vector(0.0, 1.0, 0.0), Vector(0.0, 0.0, 1.0)), Vector(0, 0, 4));
// }

   MtlDiffuse Red_brdf {
    reflection=Color(0.0, 0.0, 0.0);
     diffuse=Color(1.0, 0.01, 0.01);
     diffuse_tex=Trollbmp_tex;
   }
  MtlDiffuse Blue_brdf {
    reflection=Color(0.3, 0.3, 0.3);
     diffuse=Color(1.0, 1.0, 1.0);
  diffuse_tex=Trollbmp_tex;

      }
   UVWGenChannel bmp_uvwgen {
     uvw_channel=1;
   }
   BitmapBuffer bmp_buffer {
     file="cobbles.png";
     filter_blur=0;
     filter_type=2;
   }
   TexBitmap bmp_tex {
     bitmap=bmp_buffer;
     uvwgen=bmp_uvwgen;
   }
     BitmapBuffer Trollbmp_buffer {
       file="TrollColour.png";
       filter_blur=0;
       filter_type=2;
     }
     TexBitmap Trollbmp_tex {
       bitmap=Trollbmp_buffer;
       uvwgen=bmp_uvwgen;
     }



   MtlDiffuse Floor_brdf {
     diffuse=Color(1.0, 1.0, 1.0);
     diffuse_tex=bmp_tex;
      }
   MtlDiffuse Green_brdf {
    reflection=Color(0.0, 0.0, 0.0);
     diffuse=Color(0.0, 1.0, 0.01);
     diffuse_tex=Trollbmp_tex;

      }
    LightRectangle VRayLight02 {
     color=Color(30.0, 30.0, 30.0);
     u_size=30.0;
     v_size=30.0;
     subdivs=8;
     transform=Transform(Matrix(Vector(1.0, 0.0, 0.0), Vector(0.0, 1.0, 0.0), Vector(0.0, 0.0, 1.0)), Vector(16.0, 0.0, 256.0));
     photonSubdivs=1000;
     causticSubdivs=1500;
     storeWithIrradianceMap=0;
   }

   SettingsGI {
     on=1;
     primary_engine=0;
     primary_multiplier=1.0;
     secondary_engine=3;
     secondary_multiplier=1.0;
     reflect_caustics=0;
     refract_caustics=0;
   }
)";


scene.writeRawDataToStream(lights);
//createCube(scene);
//scene.writeObj("Helix","Helix.obj");
scene.writeObj("Troll","troll.obj");
scene.writeObj("Cube","cube.obj");

ngl::Transformation t;
t.setPosition(-0.5,0,0);
//t.setScale(0.5,1.0,0.5);

t.setRotation(0,25,0);
scene.writeNode("trollNode","Troll","Red_brdf",t.getMatrix());
t.setPosition(0.5,0,0);
t.setRotation(0,0,0);

scene.writeNode("trollNode2","Troll","Blue_brdf",t.getMatrix());
t.setPosition(0,-1.0,0);
t.setScale(20,0.1,20);
t.setRotation(0,0,0);

scene.writeNode("floor","Cube","Floor_brdf",t.getMatrix());

std::cout<<"end frame \n";
  }
}


void createCube(VRayExporter &_scene)
{
std::vector<ngl::Vec3> verts={
// 0         1        2         3
{-1,1,-1},{1,1,-1},{-1,-1,-1},{1,-1,-1}, // back
// 4         5        6         7
{-1,1,1},{1,1,1},{-1,-1,1},{1,-1,1}, // front
// 8        9        10         11
{-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}, // left
// 12       13        14         15
{1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1}, // right
// 16       17        18         19
{-1,1,-1},{1,1,-1},{-1,1,1},{1,1,1}, // top
// 20       21        22         23
{-1,-1,-1},{1,-1,-1},{-1,-1,1},{1,-1,1}, // bottom
};
std::vector <ngl::Vec3> indices={
  {2,0,3},{1,3,0}, //back
  {6,4,7},{5,7,4}, // front
  {10,8,11},{9,11,8}, // left
  {14,12,15},{13,15,12}, // right
  {18,16,19},{17,19,16}, //top
  {22,20,23},{21,23,20} //bottom

  };

std::vector<ngl::Vec3> normals={
{0,0,1},  //front
{0,0,-1}, //back

{-1,0,0}, // left
{1,0,0}, // right
{0,1,0}, // top
{0,-1,0}, // bottom

};

std::vector<ngl::Vec3> faceNormals={
{0,0,0},{0,0,0},
{1,1,1},{1,1,1},
{2,2,2},{2,2,2},
{3,3,3},{3,3,3},
{4,4,4},{4,4,4},
{5,5,5},{5,5,5}

};

  _scene.writeStaticMesh("cube",verts,indices,normals,faceNormals);

}



