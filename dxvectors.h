#ifndef __DXVECTORS_H__
#define __DXVECTORS_H__
#include <d3d12.h>
#include "SimpleMath.h"
typedef DirectX::SimpleMath::Vector3 Vector3f;
typedef DirectX::SimpleMath::Vector2 Vector2f;
typedef DirectX::XMINT2 Vector2i; // simplemath supports only XMFLOAT. probably interchangeable
typedef DirectX::XMINT3 Vector3i;
#endif //__DXVECTORS_H__