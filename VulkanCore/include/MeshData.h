#pragma once

/*--------------------------------------------------------------------------------------
기하 데이터 구조와 버텍스 데이터
--------------------------------------------------------------------------------------*/
struct VertexWithColor
{
    float x, y, z, w;   // 버텍스 좌표
    float r, g, b, a;   // 색상 포맷 Red, Green, Blue, Alpha
};

struct VertexWithUV
{
    float x, y, z, w;   // Vertex Position
    float u, v;         // Texture format U,V
};

// 인터리빙 형식으로 좌표와 색상 속성 정보를 저장함
// 3개의 버텍스에는 직교 좌표계의 위치 정보와 RGB 색상  공간에서의 색상 정보가 저장된다.
static const VertexWithColor triangleData[] =
{
    {  0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0 },
    { 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0 },
    { -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0 },
};