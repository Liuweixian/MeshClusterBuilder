//
//  MeshClusterBuilder.hpp
//  MeshClusterBuilder
//
//  Created by 刘伟贤 on 2021/9/30.
//

#ifndef MeshClusterBuilder_hpp
#define MeshClusterBuilder_hpp

#include <stdio.h>
#include "Vector3.h"
#include "AABB.h"
#include "TypeDef.h"
#include "MeshCluster.hpp"

class MeshClusterBuilder
{
public:
    MeshClusterBuilder();
    ~MeshClusterBuilder();
    
    virtual void SetClusterSize(int nClusterSize);
protected:
    int m_nClusterSize;
};

#endif /* MeshClusterBuilder_hpp */
