#include <ngl/Util.h>
#include <ngl/Vec3.h>
#include <vector>
#include <iostream>


int main()
{
  struct vertData
  {
      ngl::Real u;
      ngl::Real v;
      ngl::Real nx;
      ngl::Real ny;
      ngl::Real nz;
      ngl::Real x;
      ngl::Real y;
      ngl::Real z;
  };
  ngl::Real theta1 = 0.0f;
  ngl::Real theta2 = 0.0f;
  ngl::Real theta3 = 0.0f;

  ngl::Real radius = 3;
  ngl::Real precision = 50;

  ngl::Real c = 0.4f;
  ngl::Real b = 1.6f;

  vertData d;
  std::vector <vertData> data;

  for( int i = 0; i < precision/2; ++i )
  {
   theta1 = i * ngl::TWO_PI / precision - ngl::PI2;
   theta2 = (i + 1) * ngl::TWO_PI / precision - ngl::PI2;

   for( int j = 0; j <= precision; ++j )
   {
     theta3 = j * ngl::TWO_PI / precision;

     d.nx = (1 + c * (theta2)) * cosf(theta2) * cosf(theta3);
     d.ny =  b * sinf(theta2);
     d.nz = (1 + c * (theta2)) * cosf(theta2) * sinf(theta3);
     d.x = radius * d.nx;
     d.y = radius * d.ny;
     d.z = radius * d.nz;

     d.u  = (j/static_cast<ngl::Real>(precision));
     d.v  = 2*(i+1)/static_cast<ngl::Real>(precision);

     data.push_back(d);

     d.nx = (1 + c * (theta1)) * cosf(theta1) * cosf(theta3);
     d.ny = b * sinf(theta1);
     d.nz = (1 + c * (theta1)) * cosf(theta1) * sinf(theta3);
     d.x = radius * d.nx;
     d.y = radius * d.ny;
     d.z = radius * d.nz;

     d.u  = (j/(ngl::Real)precision);
     d.v  = 2*i/(ngl::Real)precision;

     data.push_back(d);
   }
  }

  const std::string rib1=R"DELIM(
  Display "GeneralPolygon.exr" "it" "rgba"
  Format 720 576 1
  Projection "perspective" "uniform float fov" [50]
  ShadingRate 0.01

  WorldBegin
  Translate 0 0 12
  Rotate 45 1 0 1
  )DELIM";
  std::cout<<rib1<<"\n";
  // now to export
  std::cout<<"PointsPolygons [";
  for(auto i=0; i<data.size()-1; ++i)
      std::cout<<"4 ";
  std::cout<<"] ";
  // in this case the indices will be 0 1 2 3 2 3 4 5 etc
  std::cout<<"[";
  int index=0;
  for(auto i=0; i<data.size()-1; ++i)
  {
    std::cout<<index<<" "<<index+1<<" "<<index+2<<" "<<index+3<<" ";
    ++index;
  }
  std::cout<<"]";
  std::cout<<"\"vertex point P\" [";
  for(auto v : data)
  {
    std::cout<<" "<<v.x<<" "<<v.y<<" "<<v.z<<" ";
  }
  std::cout<<data[0].x<<" "<<data[0].y<<" "<<data[0].z;
  std::cout<<data[1].x<<" "<<data[1].y<<" "<<data[1].z;
  std::cout<<"]";

  std::cout<<"\"varying normal N\" [";
  for(auto v : data)
  {
    std::cout<<" "<<v.nx<<" "<<v.ny<<" "<<v.nz<<" ";
  }
  std::cout<<data[0].nx<<" "<<data[0].ny<<" "<<data[0].nz;
  std::cout<<data[1].nx<<" "<<data[1].ny<<" "<<data[1].nz;
  std::cout<<"]";

  std::cout<<"WorldEnd\n";

}
