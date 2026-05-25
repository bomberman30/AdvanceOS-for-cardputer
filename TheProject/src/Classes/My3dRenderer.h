#pragma once

#include "./GlobalParentClass.h"
#include <Arduino.h>
#include <M5Cardputer.h>
#include <map>
struct Material {
    uint16_t color;
};
struct Vec2
{
    float x, y;
};
struct Vec3
{
    float x, y, z;
};
struct Face {
    uint16_t a, b, c;
    float  z;
    uint16_t color; 
};

class My3dRenderer : public GlobalParentClass
{
public:

int DrawMode=1;
int Beckground=1;

  Vec3 lightDir = { 0.0f, 0.0f, -1.0f };


    My3dRenderer(MyOS *os) : GlobalParentClass(os) {}

    void Begin() override;

    void Loop() override;
    void Draw() override;
         void OnExit() override;

void input();
void DrawCoordinate();

Vec3 ambientColor = {0.8f, 0.35f, 0.1f};// sunset

float ambientIntensity = 0.3f;       


Vec3 camPos = {0, 0, -5.0f}; 
float camRotX = 0, camRotY = 0; 
private:

bool autoRotate=true;

bool backFaceCulling = true;

float rotX = 0.0f;
float rotY = 0.0f;
float rotZ = 0.0f;



std::vector<Vec3> vertices;
std::vector<Face> faces;
std::map<String, Material> materials;


void drawWireframe();
void drawWithFaces();
void draGreyFaces();



Vec2 project(Vec3 v);


bool loadOBJFromSD(const char *path);
void loadMTL(const char *path);

Vec3 rotate(Vec3 v);
bool isBackFace(const Vec2& a, const Vec2& b, const Vec2& c);
Vec3 cross(const Vec3& a, const Vec3& b);
float dot(const Vec3& a, const Vec3& b);
uint16_t shadeColor(float light);
uint16_t applyLightToColor(uint16_t color, float light);
float getTransformedZ(Vec3 v);
uint16_t applyLightWithTint(uint16_t color, float diffuse, Vec3 ambCol, float ambInt) ;
int calculateFPS();

};
