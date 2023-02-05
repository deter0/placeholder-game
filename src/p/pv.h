#pragma once

#include <stdint.h>

struct pv2f {
  float x;
  float y;
  
  pv2f() {
    this->x = 0.f;
    this->y = 0.f;
  }
  pv2f(float x, float y) {
    this->x = x;
    this->y = y;
  }
  
  pv2f operator+(pv2f rs) {
    pv2f res;
    res.x = this->x + rs.x;
    res.y = this->y + rs.y;
    return res;
  }
  pv2f operator+=(pv2f rs) {
    this->x += rs.x;
    this->y += rs.y;
    return *this;
  }
  
  pv2f operator-(pv2f rs) {
    pv2f res;
    res.x = this->x - rs.x;
    res.y = this->y - rs.y;
    return res;
  }
  
  pv2f operator*(pv2f rs) {
    pv2f res;
    res.x = this->x * rs.x;
    res.y = this->y * rs.y;
    return res;
  }
  
  pv2f operator/(pv2f rs) {
    pv2f res;
    res.x = this->x / rs.x;
    res.y = this->y / rs.y;
    return res;
  }
};
