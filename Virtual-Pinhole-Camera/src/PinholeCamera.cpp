#define _USE_MATH_DEFINES


#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include "Utility.h"



const Vec3f verts[146] = {
    {  -2.5703,   0.78053,  -2.4e-05}, { -0.89264,  0.022582,  0.018577},
    {   1.6878, -0.017131,  0.022032}, {   3.4659,  0.025667,  0.018577},
    {  -2.5703,   0.78969, -0.001202}, { -0.89264,   0.25121,   0.93573},
    {   1.6878,   0.25121,    1.1097}, {   3.5031,   0.25293,   0.93573},
    {  -2.5703,    1.0558, -0.001347}, { -0.89264,    1.0558,    1.0487},
    {   1.6878,    1.0558,    1.2437}, {   3.6342,    1.0527,    1.0487},
    {  -2.5703,    1.0558,         0}, { -0.89264,    1.0558,         0},
    {   1.6878,    1.0558,         0}, {   3.6342,    1.0527,         0},
    {  -2.5703,    1.0558,  0.001347}, { -0.89264,    1.0558,   -1.0487},
    {   1.6878,    1.0558,   -1.2437}, {   3.6342,    1.0527,   -1.0487},
    {  -2.5703,   0.78969,  0.001202}, { -0.89264,   0.25121,  -0.93573},
    {   1.6878,   0.25121,   -1.1097}, {   3.5031,   0.25293,  -0.93573},
    {   3.5031,   0.25293,         0}, {  -2.5703,   0.78969,         0},
    {   1.1091,    1.2179,         0}, {    1.145,     6.617,         0},
    {   4.0878,    1.2383,         0}, {  -2.5693,    1.1771, -0.081683},
    {  0.98353,    6.4948, -0.081683}, { -0.72112,    1.1364, -0.081683},
    {   0.9297,     6.454,         0}, {  -0.7929,     1.279,         0},
    {  0.91176,    1.2994,         0}
};

const uint32_t numTris = 51;


const uint32_t tris[numTris * 3] = {
     4,   0,   5,   0,   1,   5,   1,   2,   5,   5,   2,   6,   3,   7,   2,
     2,   7,   6,   5,   9,   4,   4,   9,   8,   5,   6,   9,   9,   6,  10,
     7,  11,   6,   6,  11,  10,   9,  13,   8,   8,  13,  12,  10,  14,   9,
     9,  14,  13,  10,  11,  14,  14,  11,  15,  17,  16,  13,  12,  13,  16,
    13,  14,  17,  17,  14,  18,  15,  19,  14,  14,  19,  18,  16,  17,  20,
    20,  17,  21,  18,  22,  17,  17,  22,  21,  18,  19,  22,  22,  19,  23,
    20,  21,   0,  21,   1,   0,  22,   2,  21,  21,   2,   1,  22,  23,   2,
     2,  23,   3,   3,  23,  24,   3,  24,   7,  24,  23,  15,  15,  23,  19,
    24,  15,   7,   7,  15,  11,   0,  25,  20,   0,   4,  25,  20,  25,  16,
    16,  25,  12,  25,   4,  12,  12,   4,   8,  26,  27,  28,  29,  30,  31,
    32,  34,  33
};


bool computePixelCoordinates(
    const Vec3f& pWorld,
    const Matrix44f& worldToCamera,
    const float& b,
    const float& l,
    const float& t,
    const float& r,
    const float& near,
    const uint32_t& imageWidth,
    const uint32_t& imageHeight,
    Vec2i& pRaster)
{
    Vec3f pCamera;
    worldToCamera.multVecMatrix(pWorld, pCamera);
    Vec2f pScreen;
    pScreen.x = pCamera.x / -pCamera.z * near;
    pScreen.y = pCamera.y / -pCamera.z * near;

    Vec2f pNDC;
    pNDC.x = (pScreen.x + r) / (2 * r);
    pNDC.y = (pScreen.y + t) / (2 * t);
    pRaster.x = (int)(pNDC.x * imageWidth);
    pRaster.y = (int)((1 - pNDC.y) * imageHeight);

    bool visible = true;
    if (pScreen.x < l || pScreen.x > r || pScreen.y < b || pScreen.y > t)
        visible = false;

    return visible;
}

// Settings of the physical camera model:

float focalLength = 35; 

float filmApertureWidth = 0.825;
float filmApertureHeight = 0.446;
static const float inchToMm = 25.4;
float nearClippingPlane = 0.1;
float farClipingPlane = 1000;

uint32_t imageWidth = 512;
uint32_t imageHeight = 512;

enum FitResolutionGate { kFill = 0, kOverscan };
FitResolutionGate fitFilm = kOverscan;

int main(int argc, char** argv)
{
   

    float filmAspectRatio = filmApertureWidth / filmApertureHeight;
    float deviceAspectRatio = imageWidth / (float)imageHeight;

    float top = ((filmApertureHeight * inchToMm / 2) / focalLength) * nearClippingPlane;
    float right = ((filmApertureWidth * inchToMm / 2) / focalLength) * nearClippingPlane;

    float xscale = 1;
    float yscale = 1;

    switch (fitFilm) {
    default:
    case kFill:
        if (filmAspectRatio > deviceAspectRatio) {
            xscale = deviceAspectRatio / filmAspectRatio;
        }
        else {
            yscale = filmAspectRatio / deviceAspectRatio;
        }
        break;
    case kOverscan:
        if (filmAspectRatio > deviceAspectRatio) {
            yscale = filmAspectRatio / deviceAspectRatio;
        }
        else {
            xscale = deviceAspectRatio / filmAspectRatio;
        }
        break;
    }

    right *= xscale;
    top *= yscale;

    float bottom = -top;
    float left = -right;

    printf("Screen window coordinates: %f %f %f %f\n", bottom, left, top, right);
    printf("Film Aspect Ratio: %f\nDevice Aspect Ratio: %f\n", filmAspectRatio, deviceAspectRatio);
    printf("Angle of view: %f (deg)\n", 2 * atan((filmApertureWidth * inchToMm / 2) / focalLength) * 180 / M_PI);

   
    std::ofstream ofs;
    ofs.open("output//Result.svg");
    ofs << "<svg version=\"1.1\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns=\"http://www.w3.org/2000/svg\" width=\"" << imageWidth << "\" height=\"" << imageHeight << "\">" << std::endl;
    Matrix44f cameraToWorld(-0.95424, 0, 0.299041, 0, 0.0861242, 0.95763, 0.274823, 0, -0.28637, 0.288002, -0.913809, 0, -3.734612, 7.610426, -14.152769, 1);
    Matrix44f worldToCamera = cameraToWorld.inverse();
    float canvasWidth = 2, canvasHeight = 2;
    for (uint32_t i = 0; i < numTris; ++i) {
        const Vec3f& v0World = verts[tris[i * 3]];
        const Vec3f& v1World = verts[tris[i * 3 + 1]];
        const Vec3f& v2World = verts[tris[i * 3 + 2]];
        Vec2i v0Raster, v1Raster, v2Raster;

        bool visible = true;
        visible &= computePixelCoordinates(v0World, worldToCamera, bottom, left, top, right, nearClippingPlane, imageWidth, imageHeight, v0Raster);
        visible &= computePixelCoordinates(v1World, worldToCamera, bottom, left, top, right, nearClippingPlane, imageWidth, imageHeight, v1Raster);
        visible &= computePixelCoordinates(v2World, worldToCamera, bottom, left, top, right, nearClippingPlane, imageWidth, imageHeight, v2Raster);

        int val = visible ? 0 : 255;
        ofs << "<line x1=\"" << v0Raster.x << "\" y1=\"" << v0Raster.y << "\" x2=\"" << v1Raster.x << "\" y2=\"" << v1Raster.y << "\" style=\"stroke:rgb(" << val << ",0,0);stroke-width:1\" />\n";
        ofs << "<line x1=\"" << v1Raster.x << "\" y1=\"" << v1Raster.y << "\" x2=\"" << v2Raster.x << "\" y2=\"" << v2Raster.y << "\" style=\"stroke:rgb(" << val << ",0,0);stroke-width:1\" />\n";
        ofs << "<line x1=\"" << v2Raster.x << "\" y1=\"" << v2Raster.y << "\" x2=\"" << v0Raster.x << "\" y2=\"" << v0Raster.y << "\" style=\"stroke:rgb(" << val << ",0,0);stroke-width:1\" />\n";
    }
    ofs << "</svg>\n";
    ofs.close();

    return 0;
}