#define _USE_MATH_DEFINES

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include "Utility.h"
#include "vertexdata.h"


// Set the basic perspective projection matrix
void setProjectionMatrix(const float& angleOfView, const float& near, const float& far, Matrix44f& M)
{
    float scale = 1 / tan(angleOfView * 0.5 * M_PI / 180);
    M[0][0] = scale;
    M[1][1] = scale;
    M[2][2] = -far / (far - near);
    M[3][2] = -far * near / (far - near);
    M[2][3] = -1;
    M[3][3] = 0;
}


void multPointMatrix(const Vec3f& in, Vec3f& out, const Matrix44f& M)
{
    out.x = in.x * M[0][0] + in.y * M[1][0] + in.z * M[2][0] + /* in.z = 1 */ M[3][0];
    out.y = in.x * M[0][1] + in.y * M[1][1] + in.z * M[2][1] + /* in.z = 1 */ M[3][1];
    out.z = in.x * M[0][2] + in.y * M[1][2] + in.z * M[2][2] + /* in.z = 1 */ M[3][2];
    float w = in.x * M[0][3] + in.y * M[1][3] + in.z * M[2][3] + /* in.z = 1 */ M[3][3];

    if (w != 1) {
        out.x /= w;
        out.y /= w;
        out.z /= w;
    }
}

void BasicProjMatrix()
{
    uint32_t imageWidth = 512, imageHeight = 512;
    Matrix44f Mproj;
    Matrix44f worldToCamera;
    worldToCamera[3][1] = -10;
    worldToCamera[3][2] = -20;
    float angleOfView = 90;
    float near = 0.1;
    float far = 100;

    
    setProjectionMatrix(angleOfView, near, far, Mproj);
    unsigned char* buffer = new unsigned char[imageWidth * imageHeight];
    memset(buffer, 0x0, imageWidth * imageHeight);

    
    for (uint32_t i = 0; i < numVertices; ++i) {
        Vec3f vertCamera, projectedVert;

        
        multPointMatrix(vertices[i], vertCamera, worldToCamera);

        multPointMatrix(vertCamera, projectedVert, Mproj);
        if (projectedVert.x < -1 || projectedVert.x > 1 || projectedVert.y < -1 || projectedVert.y > 1) continue;
        
        uint32_t x = std::min(imageWidth - 1, (uint32_t)((projectedVert.x + 1) * 0.5 * imageWidth));
        uint32_t y = std::min(imageHeight - 1, (uint32_t)((1 - (projectedVert.y + 1) * 0.5) * imageHeight));
        buffer[y * imageWidth + x] = 255;
    }
    
    std::ofstream ofs;
    ofs.open("output//Basic.ppm");
    ofs << "P5\n" << imageWidth << " " << imageHeight << "\n255\n";
    ofs.write((char*)buffer, imageWidth * imageHeight);
    ofs.close();
    delete[] buffer;
}



void gluPerspective(
    const float& angleOfView,
    const float& imageAspectRatio,
    const float& n, const float& f,
    float& b, float& t, float& l, float& r)
{
    float scale = tan(angleOfView * 0.5 * M_PI / 180) * n;
    r = imageAspectRatio * scale, l = -r;
    t = scale, b = -t;
}

void glFrustum(
    const float& b, const float& t, const float& l, const float& r,
    const float& n, const float& f,
    Matrix44f& M)
{
    // set OpenGL perspective projection matrix
    M[0][0] = 2 * n / (r - l);
    M[0][1] = 0;
    M[0][2] = 0;
    M[0][3] = 0;

    M[1][0] = 0;
    M[1][1] = 2 * n / (t - b);
    M[1][2] = 0;
    M[1][3] = 0;

    M[2][0] = (r + l) / (r - l);
    M[2][1] = (t + b) / (t - b);
    M[2][2] = -(f + n) / (f - n);
    M[2][3] = -1;

    M[3][0] = 0;
    M[3][1] = 0;
    M[3][2] = -2 * f * n / (f - n);
    M[3][3] = 0;
}


void GLProjMatrix()
{
    uint32_t imageWidth = 512, imageHeight = 512;
    Matrix44f Mproj;
    Matrix44f worldToCamera;
    worldToCamera[3][1] = -10;
    worldToCamera[3][2] = -20;
    float angleOfView = 90;
    float near = 0.1;
    float far = 100;
    float imageAspectRatio = imageWidth / (float)imageHeight;
    float b, t, l, r;


    gluPerspective(angleOfView, imageAspectRatio, near, far, b, t, l, r);


    glFrustum(b, t, l, r, near, far, Mproj);

    unsigned char* buffer = new unsigned char[imageWidth * imageHeight];
    memset(buffer, 0x0, imageWidth * imageHeight);


    for (uint32_t i = 0; i < numVertices; ++i) {
        Vec3f vertCamera, projectedVert;

        multPointMatrix(vertices[i], vertCamera, worldToCamera);

        multPointMatrix(vertCamera, projectedVert, Mproj);
        if (projectedVert.x < -imageAspectRatio || projectedVert.x > imageAspectRatio || projectedVert.y < -1 || projectedVert.y > 1) continue;
        uint32_t x = std::min(imageWidth - 1, (uint32_t)((projectedVert.x + 1) * 0.5 * imageWidth));
        uint32_t y = std::min(imageHeight - 1, (uint32_t)((1 - (projectedVert.y + 1) * 0.5) * imageHeight));
        buffer[y * imageWidth + x] = 255;
    }

    std::ofstream ofs;
    ofs.open("output//glProj.ppm");
    ofs << "P5\n" << imageWidth << " " << imageHeight << "\n255\n";
    ofs.write((char*)buffer, imageWidth * imageHeight);
    ofs.close();
    delete[] buffer;
}

void glOrtho(
    const float& b, const float& t, const float& l, const float& r,
    const float& n, const float& f,
    Matrix44f& M)
{
    // set OpenGL perspective projection matrix
    M[0][0] = 2 / (r - l);
    M[0][1] = 0;
    M[0][2] = 0;
    M[0][3] = 0;

    M[1][0] = 0;
    M[1][1] = 2 / (t - b);
    M[1][2] = 0;
    M[1][3] = 0;

    M[2][0] = 0;
    M[2][1] = 0;
    M[2][2] = -2 / (f - n);
    M[2][3] = 0;

    M[3][0] = -(r + l) / (r - l);
    M[3][1] = -(t + b) / (t - b);
    M[3][2] = -(f + n) / (f - n);
    M[3][3] = 1;
}

void GLOrthoProjMatrix()
{
    uint32_t imageWidth = 512, imageHeight = 512;
    Matrix44f Mproj;
    Matrix44f worldToCamera = { 0.95424, 0.20371, -0.218924, 0, 0, 0.732087, 0.681211, 0, 0.299041, -0.650039, 0.698587, 0, -0.553677, -3.920548, -62.68137, 1 };

    float near = 0.1;
    float far = 100;
    float imageAspectRatio = imageWidth / (float)imageHeight; 

   
    const float kInfinity = std::numeric_limits<float>::max();
    Vec3f minWorld(kInfinity), maxWorld(-kInfinity);
    for (uint32_t i = 0; i < numVertices; ++i) {
        if (vertices[i].x < minWorld.x) minWorld.x = vertices[i].x;
        if (vertices[i].y < minWorld.y) minWorld.y = vertices[i].y;
        if (vertices[i].z < minWorld.z) minWorld.z = vertices[i].z;
        if (vertices[i].x > maxWorld.x) maxWorld.x = vertices[i].x;
        if (vertices[i].y > maxWorld.y) maxWorld.y = vertices[i].y;
        if (vertices[i].z > maxWorld.z) maxWorld.z = vertices[i].z;
    }

   
    Vec3f minCamera, maxCamera;
    multPointMatrix(minWorld, minCamera, worldToCamera);
    multPointMatrix(maxWorld, maxCamera, worldToCamera);

   
    float maxx = std::max(fabs(minCamera.x), fabs(maxCamera.x));
    float maxy = std::max(fabs(minCamera.y), fabs(maxCamera.y));
    float max = std::max(maxx, maxy);
    float r = max * imageAspectRatio, t = max;
    float l = -r, b = -t;

 
    glOrtho(b, t, l, r, near, far, Mproj);
    unsigned char* buffer = new unsigned char[imageWidth * imageHeight];
    memset(buffer, 0x0, imageWidth * imageHeight);

    for (uint32_t i = 0; i < numVertices; ++i) {
        Vec3f vertCamera, projectedVert;

        multPointMatrix(vertices[i], vertCamera, worldToCamera);

       
        multPointMatrix(vertCamera, projectedVert, Mproj);
        if (projectedVert.x < -imageAspectRatio || projectedVert.x > imageAspectRatio || projectedVert.y < -1 || projectedVert.y > 1) continue;

        uint32_t x = std::min(imageWidth - 1, (uint32_t)((projectedVert.x + 1) * 0.5 * imageWidth));
        uint32_t y = std::min(imageHeight - 1, (uint32_t)((1 - (projectedVert.y + 1) * 0.5) * imageHeight));
        buffer[y * imageWidth + x] = 255;
    }

    std::ofstream ofs;
    ofs.open("output//glOrthoProj.ppm");
    ofs << "P5\n" << imageWidth << " " << imageHeight << "\n255\n";
    ofs.write((char*)buffer, imageWidth * imageHeight);
    ofs.close();
    delete[] buffer;
}

int main(int argc, char** argv)
{
    BasicProjMatrix();
    GLProjMatrix();
    GLOrthoProjMatrix();

    return 0;
}