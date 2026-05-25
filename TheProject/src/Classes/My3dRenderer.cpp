#include "My3dRenderer.h"
#include "../MyOS.h"
#include "FileBrowser.h"

uint16_t BeckColor = ORANGE;

void My3dRenderer::Begin()
{
    BeckColor = ORANGE;
    loadOBJFromSD(mainOS->FileSelectedInFS.c_str());
    rotX = 3.14; // for straight mesh
    rotY = 1.0;
    camPos.x = 0;
    camPos.y = -1.7;
    camPos.z = -5;
    camRotX = -0.35;
    mainOS->sprite.deleteSprite();
}

void My3dRenderer::Loop()
{
    if (mainOS->screenOff)
    {
        return;
    }
    input();
    if (autoRotate)
    {
        // rotX += 0.005f;
        rotY += 0.014f;
        // rotZ += 0.0025f;
    }
    mainOS->sprite.createSprite(240, 135 - TopOffset);

    mainOS->sprite.fillSprite(BeckColor);
    switch (DrawMode)
    {
    case 1:
        drawWithFaces();
        //backFaceCulling = true;
        break;
    case 2:
        draGreyFaces();
        //backFaceCulling = true;
        BeckColor = DARK_CHARCOAL;
        break;
    case 3:
        drawWireframe();
        //backFaceCulling = false;
        BeckColor = BLACK;
        break;
    default:
        break;
    }

    DrawCoordinate();
    mainOS->sprite.pushSprite(0, 0);
}

void My3dRenderer::Draw()
{
}

void My3dRenderer::OnExit()
{
    vertices.clear();
    faces.clear();
    materials.clear();
    // mainOS->ShowOnScreenMessage("on exit 3d render", 300);
}

void My3dRenderer::input()
{
    if (mainOS->NewKey.ifKeyJustPress('`')) // if press escape black screen
    {
        mainOS->PlayCuteEvilTone();
        mainOS->ChangeMenu(new FileBrowser(mainOS));
    }

    if (mainOS->NewKey.ifKeyJustPress('1')) // Toggle background colors/modes
    {
        DrawMode = 1;
        Beckground += 1;
        if (Beckground > 2)
        {
            Beckground = 1;
        }
        switch (Beckground)
        {
        case 1:
            BeckColor = ORANGE;
            ambientColor = {0.8f, 0.35f, 0.1f};
            break;
        case 2:
            BeckColor = CYAN;
            ambientColor = {0.2f, 0.4f, 0.6f};
            break;
        default:
            break;
        }

        backFaceCulling = true;
    }

    if (mainOS->NewKey.ifKeyJustPress('2'))
    {
        DrawMode = 2;
        backFaceCulling = true;
    }
    if (mainOS->NewKey.ifKeyJustPress('3'))
    {
        DrawMode = 3;
    }

    if (mainOS->NewKey.ifKeyJustPress('4'))
    {
        autoRotate = !autoRotate;
    }

    {
        float moveSpeed = 0.1f;
        float rotSpeed = 0.05f;

        if (mainOS->NewKey.ApplyPushedKeyEveryMS(',', 20))
            camRotY -= rotSpeed;
        if (mainOS->NewKey.ApplyPushedKeyEveryMS('/', 20))
            camRotY += rotSpeed; 
        if (mainOS->NewKey.ApplyPushedKeyEveryMS('.', 20))
            camRotX -= rotSpeed;
        if (mainOS->NewKey.ApplyPushedKeyEveryMS(';', 20))
            camRotX += rotSpeed; 

        camRotX = constrain(camRotX, -1.5f, 1.5f);

        float sinY = sin(camRotY);
        float cosY = cos(camRotY);
        float sinX = sin(camRotX);
        float cosX = cos(camRotX);

        if (mainOS->NewKey.ApplyPushedKeyEveryMS('e', 20)) // Forward
        {
            camPos.x += sinY * cosX * moveSpeed;
            camPos.y += -sinX * moveSpeed; // Negative because Y is usually up in graphics engines
            camPos.z += cosY * cosX * moveSpeed;
        }
        if (mainOS->NewKey.ApplyPushedKeyEveryMS('s', 20)) // Backward
        {
            camPos.x -= sinY * cosX * moveSpeed;
            camPos.y -= -sinX * moveSpeed;
            camPos.z -= cosY * cosX * moveSpeed;
        }

        // Strafe movement (sideways) usually remains horizontal, so no change needed
        if (mainOS->NewKey.ApplyPushedKeyEveryMS('a', 20)) // Left
        {
            camPos.x -= cosY * moveSpeed;
            camPos.z += sinY * moveSpeed;
        }
        if (mainOS->NewKey.ApplyPushedKeyEveryMS('d', 20)) // Right
        {
            camPos.x += cosY * moveSpeed;
            camPos.z -= sinY * moveSpeed;
        }

        // Vertical movement (usually fixed to world Y-axis)
        if (mainOS->NewKey.ApplyPushedKeyEveryMS('r', 20))
            camPos.y -= moveSpeed; // Up
        if (mainOS->NewKey.ApplyPushedKeyEveryMS('w', 20))
            camPos.y += moveSpeed; // Down
    }
}

void My3dRenderer::DrawCoordinate()
{
    mainOS->sprite.setTextColor(WHITE);
    mainOS->sprite.unloadFont();
    mainOS->sprite.setTextSize(1);
    mainOS->sprite.setCursor(0, 0);

    int fps = calculateFPS();
    mainOS->sprite.setTextColor(GREEN);
    if (fps < 10)
        mainOS->sprite.print("0"); // Leading zero for aesthetics
    mainOS->sprite.print(fps);
    mainOS->sprite.print(" FPS");
}

void My3dRenderer::drawWireframe()
{
    const float NEAR = 0.1f;

    for (auto &f : faces)
    {
        // Object rotation
        Vec3 v1 = rotate(vertices[f.a]);
        Vec3 v2 = rotate(vertices[f.b]);
        Vec3 v3 = rotate(vertices[f.c]);

        // Depth relative to camera
        float z1 = getTransformedZ(v1);
        float z2 = getTransformedZ(v2);
        float z3 = getTransformedZ(v3);

        // If the entire polygon is behind the camera – don't draw
        if (z1 < NEAR && z2 < NEAR && z3 < NEAR)
            continue;

        // Projection to screen
        Vec2 p1 = project(v1);
        Vec2 p2 = project(v2);
        Vec2 p3 = project(v3);

        // Back-face culling (optional)
        if (backFaceCulling && isBackFace(p1, p2, p3))
            continue;

        // Draw wireframe outlines
        mainOS->sprite.drawLine(p1.x, p1.y, p2.x, p2.y, WHITE);
        mainOS->sprite.drawLine(p2.x, p2.y, p3.x, p3.y, WHITE);
        mainOS->sprite.drawLine(p3.x, p3.y, p1.x, p1.y, WHITE);
    }
}
void My3dRenderer::drawWithFaces()
{
    // 1. שלב ההתמרה (Transformation Phase)
    // מחשבים את כל הקודקודים פעם אחת ושומרים במערך זמני
    size_t vertexCount = vertices.size();
    std::vector<Vec3> transformed(vertexCount);
    for (size_t i = 0; i < vertexCount; ++i) {
        transformed[i] = rotate(vertices[i]);
    }

    // 2. חישוב עומק לכל פאה (Depth Calculation)
    for (auto &f : faces) {
        // שימוש בקודקודים שכבר סובבו
        float zA = getTransformedZ(transformed[f.a]);
        float zB = getTransformedZ(transformed[f.b]);
        float zC = getTransformedZ(transformed[f.c]);
        f.z = (zA + zB + zC) / 3.0f;
    }

    // 3. מיון (Sorting) - שימוש ב-Lambda פשוטה כדי לא לבלבל את הקומפיילר
    std::sort(faces.begin(), faces.end(), [](const Face &lhs, const Face &rhs) {
        return lhs.z > rhs.z;
    });

    // 4. שלב הציור (Drawing Phase)
    for (auto &f : faces) {
        // שליפת הקודקודים המוכנים
        const Vec3& v1 = transformed[f.a];
        const Vec3& v2 = transformed[f.b];
        const Vec3& v3 = transformed[f.c];

        float z1 = getTransformedZ(v1);
        float z2 = getTransformedZ(v2);
        float z3 = getTransformedZ(v3);

        // בדיקה אם הפוליגון מאחורי המצלמה
        if (z1 <= 0.0f && z2 <= 0.0f && z3 <= 0.0f)
            continue;

        // חישוב נורמל
        Vec3 edge1 = {v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
        Vec3 edge2 = {v3.x - v1.x, v3.y - v1.y, v3.z - v1.z};
        Vec3 n = cross(edge1, edge2);

        // Back-face culling
        if (backFaceCulling) {
            Vec2 p1_t = project(v1);
            Vec2 p2_t = project(v2);
            Vec2 p3_t = project(v3);
            if (isBackFace(p1_t, p2_t, p3_t))
                continue;
        }

        // נורמליזציה של הנורמל
        float sqMag = n.x * n.x + n.y * n.y + n.z * n.z;
        if (sqMag > 0.0001f) {
            float invLen = 1.0f / sqrt(sqMag);
            n.x *= invLen;
            n.y *= invLen;
            n.z *= invLen;
        }

        // חישוב תאורה (Diffuse) - ללא std::max כדי למנוע את השגיאה
        float dotNL = dot(n, lightDir);
        float clampedDot = (dotNL > 0.0f) ? dotNL : 0.0f;
        float diffuse = 0.3f + 0.7f * clampedDot;

        // הקרנה לדו-מימד
        Vec2 p1 = project(v1);
        Vec2 p2 = project(v2);
        Vec2 p3 = project(v3);

        // צביעה
        uint16_t finalColor = applyLightWithTint(f.color, diffuse, ambientColor, ambientIntensity);
        mainOS->sprite.fillTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, finalColor);
    }
}

void My3dRenderer::draGreyFaces()
{
    // 1. Calculate depth for sorting (Painter's Algorithm)
    for (auto &f : faces)
    {
        Vec3 v1 = rotate(vertices[f.a]);
        Vec3 v2 = rotate(vertices[f.b]);
        Vec3 v3 = rotate(vertices[f.c]);

        f.z = (getTransformedZ(v1) +
               getTransformedZ(v2) +
               getTransformedZ(v3)) / 3.0f;
    }

    // 2. Sort - Far polygons drawn first
    std::sort(faces.begin(), faces.end(), [](const Face &a, const Face &b)
              { return a.z > b.z; });

    // 3. Draw Loop
    for (auto &f : faces)
    {
        // Transform vertices
        Vec3 v1 = rotate(vertices[f.a]);
        Vec3 v2 = rotate(vertices[f.b]);
        Vec3 v3 = rotate(vertices[f.c]);

        float z1 = getTransformedZ(v1);
        float z2 = getTransformedZ(v2);
        float z3 = getTransformedZ(v3);

        if (z1 <= 0.0f && z2 <= 0.0f && z3 <= 0.0f)
            continue;

        // Projection
        Vec2 p1 = project(v1);
        Vec2 p2 = project(v2);
        Vec2 p3 = project(v3);

        // Back-face Culling: Hide polygons facing away from camera
        if (backFaceCulling)
        {
            if (isBackFace(p1, p2, p3))
                continue;
        }

        // Calculate Polygon Normal
        Vec3 edge1 = {v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
        Vec3 edge2 = {v3.x - v1.x, v3.y - v1.y, v3.z - v1.z};

        // Cross Product for perpendicular vector
        Vec3 n;
        n.x = edge1.y * edge2.z - edge1.z * edge2.y;
        n.y = edge1.z * edge2.x - edge1.x * edge2.z;
        n.z = edge1.x * edge2.y - edge1.y * edge2.x;

        // Vector Normalization (Length 1)
        float length = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        if (length > 0)
        {
            n.x /= length;
            n.y /= length;
            n.z /= length;
        }

        // Lighting intensity (Flat Shading)
        float lightIntensity = dot(n, lightDir);

        // Clamp light intensity (Ambient minimum)
        if (lightIntensity < 0.1f)
            lightIntensity = 0.1f; 

        // Create lighting-based grayscale
        uint8_t gray = (uint8_t)(lightIntensity * 255.0f);

        // Convert to RGB565 (common for microcontroller displays)
        uint16_t finalColor = mainOS->sprite.color565(gray, gray, gray);

        mainOS->sprite.fillTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, finalColor);
    }
}

Vec2 My3dRenderer::project(Vec3 v)
{
    // 1. Translation relative to camera
    float x = v.x - camPos.x;
    float y = v.y - camPos.y;
    float z = v.z - camPos.z;

    // 2. Camera Rotation
    // Y Rotation (Left/Right)
    float cY = cos(-camRotY), sY = sin(-camRotY);
    float x1 = x * cY + z * sY;
    float z1 = -x * sY + z * cY;

    // X Rotation (Up/Down)
    float cX = cos(-camRotX), sX = sin(-camRotX);
    float y2 = y * cX - z1 * sX;
    float z2 = y * sX + z1 * cX;

    // 3. Clipping - Prevent distortions if point is behind camera
    if (z2 < 0.1f)
        z2 = 0.1f;

    // 4. Projection
    float fov = 150.0f;
    float scale = fov / z2;

    return {
        x1 * scale + 120,
        y2 * scale + 67};
}

bool My3dRenderer::loadOBJFromSD(const char *path)
{
    // Store path as String for convenience
    String finalPath = String(path);

    // Check if user accidentally selected MTL instead of OBJ
    if (finalPath.endsWith(".mtl") || finalPath.endsWith(".MTL"))
    {
        String objAttempt = finalPath;
        objAttempt.replace(".mtl", ".obj");
        objAttempt.replace(".MTL", ".obj");

        if (SD.exists(objAttempt))
        {
            finalPath = objAttempt;
        }
        else
        {
            return false;
        }
    }

    // Clear previous data
    vertices.clear();  // Vertices
    faces.clear();     // Faces
    materials.clear(); // Materials

    // Auto-load MTL based on OBJ filename
    String mtlPath = finalPath;
    mtlPath.replace(".obj", ".mtl");
    loadMTL(mtlPath.c_str());

    File file = SD.open(finalPath.c_str(), FILE_READ);
    if (!file)
        return false;

    String currentMaterial = "";
    uint16_t defaultColor = mainOS->sprite.color565(200, 200, 200);

    std::vector<int> vertexIndices;
    vertexIndices.reserve(4);

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();

        // Vertex line (v x y z)
        if (line.startsWith("v "))
        {
            Vec3 v;
            sscanf(line.c_str(), "v %f %f %f", &v.x, &v.y, &v.z);
            vertices.push_back(v);
        }
        // Active material change (usemtl name)
        else if (line.startsWith("usemtl "))
        {
            currentMaterial = line.substring(7);
            currentMaterial.trim();
        }
        // Face line (f ...)
        else if (line.startsWith("f "))
        {
            vertexIndices.clear();

            char *lineC = strdup(line.c_str());
            char *token = strtok(lineC + 2, " ");

            while (token != NULL)
            {
                int vIdx;
                // Read vertex index (ignoring textures/normals)
                if (sscanf(token, "%d", &vIdx) == 1)
                {
                    // OBJ is 1-indexed, we use 0-indexed
                    vertexIndices.push_back(vIdx - 1);
                }
                token = strtok(NULL, " ");
            }
            free(lineC);

            // Triangulation: Convert N-vertex face into triangles
            for (size_t i = 1; i < vertexIndices.size() - 1; i++)
            {
                Face f;
                f.a = vertexIndices[0];
                f.b = vertexIndices[i];
                f.c = vertexIndices[i + 1];

                // Assign color from MTL or default
                if (materials.find(currentMaterial) != materials.end())
                {
                    f.color = materials[currentMaterial].color;
                }
                else
                {
                    f.color = defaultColor;
                }

                faces.push_back(f);
            }
        }
    }

    file.close();
    materials.clear();
    return true;
}

Vec3 My3dRenderer::rotate(Vec3 v)
{
    // X Axis
    float cx = cos(rotX), sx = sin(rotX);
    float y = v.y * cx - v.z * sx;
    float z = v.y * sx + v.z * cx;
    v.y = y;
    v.z = z;

    // Y Axis
    float cy = cos(rotY), sy = sin(rotY);
    float x = v.x * cy + v.z * sy;
    z = -v.x * sy + v.z * cy;
    v.x = x;
    v.z = z;

    // Z Axis
    float cz = cos(rotZ), sz = sin(rotZ);
    x = v.x * cz - v.y * sz;
    y = v.x * sz + v.y * cz;
    v.x = x;
    v.y = y;

    return v;
}

bool My3dRenderer::isBackFace(const Vec2 &a, const Vec2 &b, const Vec2 &c)
{
    // 2D Cross Product
    float cross = (b.x - a.x) * (c.y - a.y) -
                  (b.y - a.y) * (c.x - a.x);

    return cross > 0; // Clockwise = facing backwards
}

Vec3 My3dRenderer::cross(const Vec3 &a, const Vec3 &b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

float My3dRenderer::dot(const Vec3 &a, const Vec3 &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

uint16_t My3dRenderer::shadeColor(float light)
{
    light = constrain(light, 0.2f, 1.0f); // Not completely black

    uint8_t r = 230 * light;
    uint8_t g = 5 * light;
    uint8_t b = 0 * light;

    return mainOS->sprite.color565(r, g, b);
}

void My3dRenderer::loadMTL(const char *path)
{
    materials.clear();
    File file = SD.open(path, FILE_READ);
    if (!file)
        return;

    String currentMatName = "";
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();

        if (line.startsWith("newmtl "))
        {
            currentMatName = line.substring(7);
            currentMatName.trim();
        }
        else if (line.startsWith("Kd ") && currentMatName != "")
        {
            float r, g, b;
            sscanf(line.c_str(), "Kd %f %f %f", &r, &g, &b);
            // Convert 0.0-1.0 to screen RGB565
            materials[currentMatName] = {
                mainOS->sprite.color565(r * 255, g * 255, b * 255)};
        }
    }
    file.close();
}

uint16_t My3dRenderer::applyLightToColor(uint16_t color, float light)
{
    light = constrain(light, 0.2f, 1.0f); // Minimum ambient light

    // Deconstruct RGB565
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5) & 0x3F;
    uint8_t b = color & 0x1F;

    // Apply lighting
    r = (uint8_t)(r * light);
    g = (uint8_t)(g * light);
    b = (uint8_t)(b * light);

    // Reconstruct
    return (r << 11) | (g << 5) | b;
}

float My3dRenderer::getTransformedZ(Vec3 v)
{
    // Translation relative to camera
    float x = v.x - camPos.x;
    float y = v.y - camPos.y;
    float z = v.z - camPos.z;

    // Camera Y Rotation
    float cY = cos(-camRotY), sY = sin(-camRotY);
    float z1 = -x * sY + z * cY;

    // Camera X Rotation
    float cX = cos(-camRotX), sX = sin(-camRotX);
    float z2 = y * sX + z1 * cX;

    return z2;
}

uint16_t My3dRenderer::applyLightWithTint(
    uint16_t color,
    float diffuse,
    Vec3 ambCol,
    float ambInt)
{
    // RGB565 -> float normalized 0-1
    float r = ((color >> 11) & 0x1F) / 31.0f;
    float g = ((color >> 5) & 0x3F) / 63.0f;
    float b = (color & 0x1F) / 31.0f;

    diffuse = std::max(0.0f, diffuse);

    // Ambient as true addition
    float finalR = r * diffuse + ambCol.x * ambInt;
    float finalG = g * diffuse + ambCol.y * ambInt;
    float finalB = b * diffuse + ambCol.z * ambInt;

    finalR = std::min(1.0f, finalR);
    finalG = std::min(1.0f, finalG);
    finalB = std::min(1.0f, finalB);

    return ((uint16_t)(finalR * 31) << 11) |
           ((uint16_t)(finalG * 63) << 5) |
           ((uint16_t)(finalB * 31));
}

int My3dRenderer::calculateFPS()
{
    static unsigned long lastMillis = 0;
    static int fpsCount = 0;
    static int currentFPS = 0;
    unsigned long now = millis();

    fpsCount++;

    // Update FPS once per second
    if (now - lastMillis >= 1000)
    {
        currentFPS = fpsCount;
        fpsCount = 0;
        lastMillis = now;
    }

    // Limit to 2 digits (0-99)
    if (currentFPS > 99)
        return 99;
    return currentFPS;
}