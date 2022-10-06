#pragma once

#include <iostream>
#include <stdio.h>


using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

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