#pragma once

//Windows
#include <Windows.h>
#include "dwmapi.h"
#pragma comment(lib, "Dwmapi.lib")

//C++ standard library
#include <string>
#include <thread>
#include <bitset>
#include <queue>
#include <optional>
#include <future>
#include <span>
#include <numbers>
#include <atomic>
#include <array>

//DirectX11
#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

//Helpers
#include <wrl/client.h>

//lib
#include "Audio.h"
#include "CommonStates.h"
#include "SimpleMath.h"
#include "SpriteFont.h"
#include "WICTextureLoader.h"