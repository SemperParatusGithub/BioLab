#pragma once

#include <iostream>
#include <stdio.h>

#include <vector>
#include <imgui.h>


using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;


struct Signal
{
	int id = -1;
	std::string label = "Unnamed";
	ImVec4 color;
	std::vector<float> xValues, yValues;
};

enum class Font
{
	Default = 0,
	OpenSans,
	OpenSansHeading,
	BigIcons
};

typedef struct {
	float x, y;
} Vector2f;
typedef struct
{
	float x, y, z;
} Vector3f;
typedef struct
{
	float x, y, z, w;
} Vector4f;

typedef struct
{
	double x, y;
} Vector2d;
typedef struct
{
	double x, y, z;
} Vector3d;
typedef struct
{
	double x, y, z, w;
} Vector4d;


#ifdef _DEBUG
	#define LOG_INFO(...)  { printf("[INFO]: ");  printf(__VA_ARGS__); printf("\n"); }
	#define LOG_ERROR(...) { printf("[ERROR]: "); printf(__VA_ARGS__); printf("\n"); }
#else
	#define LOG_INFO(...) 
	#define LOG_ERROR(...)
#endif


#define BIOLAB_WIN32_DIRECTX12
//#define BIOLAB_GLFW_OPENGL3

#define BIOLAB_PLATFORM_WINDOWS
#define BIOLAB_DEBUG_CONSOLE