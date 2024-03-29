//
//  Utilities.hpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/10/20.
//

#ifndef Utilities_hpp
#define Utilities_hpp

#include <stdio.h>
#include <vector>
#include "Vector3.h"
#include "Vector4.h"

void BuildAdjacencyList(const uint16_t* indices, uint32_t indexCount, const Vector3f* positions, uint32_t vertexCount, uint32_t* adjacency);

void BuildAdjacencyList(const uint32_t* indices, uint32_t indexCount, const Vector3f* positions, uint32_t vertexCount, uint32_t* adjacency);

Vector4f MinimumBoundingSphere(Vector3f* points, uint32_t count);

#endif /* Utilities_hpp */
