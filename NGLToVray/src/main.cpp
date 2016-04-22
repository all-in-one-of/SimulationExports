#include "VRayExporter.h"
#include <ngl/Util.h>
#include <ngl/NGLStream.h>
#include <ngl/Transformation.h>
#include <array>
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
/*
  ngl::Mat4 view=ngl::lookAt(ngl::Vec3(0,5,0),
                             ngl::Vec3(0,0,0),
                             ngl::Vec3(1,0,0));

  scene.renderView(view);
  */
  ngl::Transformation tx;
  tx.setPosition(0,0,-5);
  tx.setRotation(0,i,0);
  scene.renderView(tx.getMatrix());
  scene.setImageSize(600,450);
  scene.setFOV(ngl::radians(45.0f));
  scene.setBGColour(ngl::Colour(0.4f,0.4f,0.4f));
  scene.includeFile("sceneSetup.vrscene");


//createCube(scene);
//scene.writeObj("Helix","Helix.obj");
scene.writeObj("Troll","troll.obj");
scene.writeObj("Cube","cube.obj");
scene.includeFile("scenes/teapot.vrscene");
ngl::Transformation t;
t.setPosition(-0.5f,-0.4f,0);
//t.setScale(0.5,1.0,0.5);

t.setRotation(0,25,0);
scene.writeNode("trollNode","Troll","Red_brdf",t.getMatrix());
t.setPosition(0.5f,-0.4f,0);
t.setRotation(0,0,0);

scene.writeNode("trollNode2","Troll","Blue_brdf",t.getMatrix());
t.setPosition(0.0f,-1.0f,0.0f);
t.setScale(20.0f,0.1f,20.0f);
t.setRotation(0.0f,0.0f,0.0f);

scene.writeNode("floor","Cube","Floor_brdf",t.getMatrix());
/*
int count=0;
for(float z=5.5; z>0; z-=0.8)
{
  for(float x=-2.5; x<2.5; x+=0.8)
  {
  t.setPosition(x,-0.8f,-z);
  t.setScale(0.01f,0.01f,0.01f);
  t.setRotation(90,0,0);
  char name[50];
  std::sprintf(name,"texport%d",count);
  count++;
  scene.writeNode(name,"teapot_mesh","Green_brdf",t.getMatrix());
  }
}
*/
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



