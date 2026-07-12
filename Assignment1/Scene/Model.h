// model.h
// model class parent
// InitModel and Render are not valid functions in model class (as in u cant call Model model; model.InitModel() ... etc)

#pragma once
#include "Platform.h"

class Model{
public:
    Model(){}
    virtual ~Model(){}

    virtual void InitModel() = 0; // called once, before the first frame
    virtual void Render() = 0; // called once per frame
    virtual void Resize(int w, int h) {} // called whenever the surface/window resizes
};