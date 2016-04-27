#ifndef VRAYHELPERFUNCS_H_
#define VRAYHELPERFUNCS_H_
#include <ngl/Colour.h>
#include <ngl/Vec3.h>
#include <ostream>
#include <utility>

// Simple RAII style writer / close for groups name{ }
class StartGroup
{
  public :
    explicit StartGroup(std::ofstream *_stream,const std::string &_name)
    {
      m_stream=_stream;
      *m_stream<<_name<<"{\n";
    }
    ~StartGroup(){
      *m_stream<<"}\n";
    }

    StartGroup(const StartGroup &)=delete;
    StartGroup &operator=(const StartGroup & )=delete;
  private :
    std::ofstream *m_stream;
};


template <typename T, typename S>
void writePair(std::ofstream &_stream, T _first, S _second)
{
  _stream<<_first<<"="<<_second<<";\n"  ;
}

// specialisation for ngl::Colour
template<typename T>
void writePair(std::ofstream &_stream, T _first, const ngl::Colour &_second)
{
  _stream<<_first<<"=Color("<<_second.m_r<<","<< _second.m_g<<","<<_second.m_b<<");" ;
}

// specialisation for ngl::Vec3
template<typename T>
void writePair(std::ofstream &_stream, T _first, const ngl::Vec3 &_second)
{
  _stream<<_first<<"=Vector("<<_second.m_x<<","<< _second.m_y<<","<<_second.m_z<<");" ;
}


#endif
