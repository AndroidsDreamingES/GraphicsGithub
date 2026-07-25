// Scene/Ground.h - a big quad, Phong-lit, checkerboard computed in the fragment
// shader from world-space position. no texture, no vbo beyond one quad/

#pragma once

#include "Model.h"

class Ground : public Model{
    public:
    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;
};