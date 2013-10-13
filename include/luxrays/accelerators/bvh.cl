#line 2 "bvh_kernel.cl"

/***************************************************************************
 *   Copyright (C) 1998-2013 by authors (see AUTHORS.txt)                  *
 *                                                                         *
 *   This file is part of LuxRays.                                         *
 *                                                                         *
 *   LuxRays is free software; you can redistribute it and/or modify       *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   LuxRays is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   LuxRays website: http://www.luxrender.net                             *
 ***************************************************************************/

typedef struct {
	union {
		struct {
			// I can not use BBox here because objects with a constructor are not
			// allowed inside an union.
			float bboxMin[3];
			float bboxMax[3];
		} bvhNode;
		struct {
			uint v[3];
			uint meshIndex, triangleIndex;
		} triangleLeaf;
		struct {
			uint leafIndex;
			uint transformIndex;
			uint meshOffsetIndex;
		} bvhLeaf; // Used by MBVH
	};
	// Most significant bit is used to mark leafs
	uint nodeData;
	int pad0; // To align to float4
} BVHAccelArrayNode;

#define BVHNodeData_IsLeaf(nodeData) ((nodeData) & 0x80000000u)
#define BVHNodeData_GetSkipIndex(nodeData) ((nodeData) & 0x7fffffffu)
#if (BVH_NODES_PAGE_COUNT > 1)
#define BVHNodeData_GetPageIndex(nodeData) (((nodeData) & 0x70000000u) >> 28)
#define BVHNodeData_GetNodeIndex(nodeData) ((nodeData) & 0x0fffffffu)
#endif

#if (BVH_NODES_PAGE_COUNT > 1)
void NextNode(uint *pageIndex, uint *nodeIndex) {
	++(*nodeIndex);
	if (*nodeIndex >= BVH_NODES_PAGE_SIZE) {
		*nodeIndex = 0;
		++(*pageIndex);
	}
}
#endif

#if (BVH_NODES_PAGE_COUNT == 8)
#define ACCELERATOR_INTERSECT_PARAM_DECL , __global Point *accelVertPage0, __global Point *accelVertPage1, __global Point *accelVertPage2, __global Point *accelVertPage3, __global Point *accelVertPage4, __global Point *accelVertPage5, __global Point *accelVertPage6, __global Point *accelVertPage7, __global BVHAccelArrayNode *accelNodePage0, __global BVHAccelArrayNode *accelNodePage1, __global BVHAccelArrayNode *accelNodePage2, __global BVHAccelArrayNode *accelNodePage3, __global BVHAccelArrayNode *accelNodePage4, __global BVHAccelArrayNode *accelNodePage5, __global BVHAccelArrayNode *accelNodePage6, __global BVHAccelArrayNode *accelNodePage7
#define ACCELERATOR_INTERSECT_PARAM , accelVertPage0, accelVertPage1, accelVertPage2, accelVertPage3, accelVertPage4, accelVertPage5, accelVertPage6, accelVertPage7, accelNodePage0, accelNodePage1, accelNodePage2, accelNodePage3, accelNodePage4, accelNodePage5, accelNodePage6, accelNodePage7
#elif (BVH_NODES_PAGE_COUNT == 7)
#define ACCELERATOR_INTERSECT_PARAM_DECL , __global Point *accelVertPage0, __global Point *accelVertPage1, __global Point *accelVertPage2, __global Point *accelVertPage3, __global Point *accelVertPage4, __global Point *accelVertPage5, __global Point *accelVertPage6, __global BVHAccelArrayNode *accelNodePage0, __global BVHAccelArrayNode *accelNodePage1, __global BVHAccelArrayNode *accelNodePage2, __global BVHAccelArrayNode *accelNodePage3, __global BVHAccelArrayNode *accelNodePage4, __global BVHAccelArrayNode *accelNodePage5, __global BVHAccelArrayNode *accelNodePage6
#define ACCELERATOR_INTERSECT_PARAM , accelVertPage0, accelVertPage1, accelVertPage2, accelVertPage3, accelVertPage4, accelVertPage5, accelVertPage6, accelNodePage0, accelNodePage1, accelNodePage2, accelNodePage3, accelNodePage4, accelNodePage5, accelNodePage6
#elif (BVH_NODES_PAGE_COUNT == 6)
#define ACCELERATOR_INTERSECT_PARAM_DECL , __global Point *accelVertPage0, __global Point *accelVertPage1, __global Point *accelVertPage2, __global Point *accelVertPage3, __global Point *accelVertPage4, __global Point *accelVertPage5, __global BVHAccelArrayNode *accelNodePage0, __global BVHAccelArrayNode *accelNodePage1, __global BVHAccelArrayNode *accelNodePage2, __global BVHAccelArrayNode *accelNodePage3, __global BVHAccelArrayNode *accelNodePage4, __global BVHAccelArrayNode *accelNodePage5
#define ACCELERATOR_INTERSECT_PARAM , accelVertPage0, accelVertPage1, accelVertPage2, accelVertPage3, accelVertPage4, accelVertPage5, accelNodePage0, accelNodePage1, accelNodePage2, accelNodePage3, accelNodePage4, accelNodePage5
#elif (BVH_NODES_PAGE_COUNT == 5)
#define ACCELERATOR_INTERSECT_PARAM_DECL , __global Point *accelVertPage0, __global Point *accelVertPage1, __global Point *accelVertPage2, __global Point *accelVertPage3, __global Point *accelVertPage4, __global BVHAccelArrayNode *accelNodePage0, __global BVHAccelArrayNode *accelNodePage1, __global BVHAccelArrayNode *accelNodePage2, __global BVHAccelArrayNode *accelNodePage3, __global BVHAccelArrayNode *accelNodePage4
#define ACCELERATOR_INTERSECT_PARAM , accelVertPage0, accelVertPage1, accelVertPage2, accelVertPage3, accelVertPage4, accelNodePage0, accelNodePage1, accelNodePage2, accelNodePage3, accelNodePage4
#elif (BVH_NODES_PAGE_COUNT == 4)
#define ACCELERATOR_INTERSECT_PARAM_DECL , __global Point *accelVertPage0, __global Point *accelVertPage1, __global Point *accelVertPage2, __global Point *accelVertPage3, __global BVHAccelArrayNode *accelNodePage0, __global BVHAccelArrayNode *accelNodePage1, __global BVHAccelArrayNode *accelNodePage2, __global BVHAccelArrayNode *accelNodePage3
#define ACCELERATOR_INTERSECT_PARAM , accelVertPage0, accelVertPage1, accelVertPage2, accelVertPage3, accelNodePage0, accelNodePage1, accelNodePage2, accelNodePage3
#elif (BVH_NODES_PAGE_COUNT == 3)
#define ACCELERATOR_INTERSECT_PARAM_DECL , __global Point *accelVertPage0, __global Point *accelVertPage1, __global Point *accelVertPage2, __global BVHAccelArrayNode *accelNodePage0, __global BVHAccelArrayNode *accelNodePage1, __global BVHAccelArrayNode *accelNodePage2
#define ACCELERATOR_INTERSECT_PARAM , accelVertPage0, accelVertPage1, accelVertPage2, accelNodePage0, accelNodePage1, accelNodePage2
#elif (BVH_NODES_PAGE_COUNT == 2)
#define ACCELERATOR_INTERSECT_PARAM_DECL , __global Point *accelVertPage0, __global Point *accelVertPage1, __global BVHAccelArrayNode *accelNodePage0, __global BVHAccelArrayNode *accelNodePage1
#define ACCELERATOR_INTERSECT_PARAM , accelVertPage0, accelVertPage1, accelNodePage0, accelNodePage1
#elif (BVH_NODES_PAGE_COUNT == 1)
#define ACCELERATOR_INTERSECT_PARAM_DECL , __global Point *accelVertPage0, __global BVHAccelArrayNode *accelNodePage0
#define ACCELERATOR_INTERSECT_PARAM , accelVertPage0, accelNodePage0
#elif
ERROR: unsuported BVH_NODES_PAGE_COUNT !!!
#endif

void Accelerator_Intersect(
		Ray *ray,
		RayHit *rayHit
		ACCELERATOR_INTERSECT_PARAM_DECL
		) {
	// Initialize vertex page references
#if (BVH_VERTS_PAGE_COUNT > 1)
	__global Point *accelVertPages[BVH_VERTS_PAGE_COUNT];
#if defined(BVH_VERTS_PAGE0)
	accelVertPages[0] = accelVertPage0;
#endif
#if defined(BVH_VERTS_PAGE1)
	accelVertPages[1] = accelVertPage1;
#endif
#if defined(BVH_VERTS_PAGE2)
	accelVertPages[2] = accelVertPage2;
#endif
#if defined(BVH_VERTS_PAGE3)
	accelVertPages[3] = accelVertPage3;
#endif
#if defined(BVH_VERTS_PAGE4)
	accelVertPages[4] = accelVertPage4;
#endif
#if defined(BVH_VERTS_PAGE5)
	accelVertPages[5] = accelVertPage5;
#endif
#if defined(BVH_VERTS_PAGE6)
	accelVertPages[6] = accelVertPage6;
#endif
#if defined(BVH_VERTS_PAGE7)
	accelVertPages[7] = accelVertPage7;
#endif
#endif

	// Initialize node page references
#if (BVH_NODES_PAGE_COUNT > 1)
	__global BVHAccelArrayNode *accelNodePages[BVH_NODES_PAGE_COUNT];
#if defined(BVH_NODES_PAGE0)
	accelNodePages[0] = accelNodePage0;
#endif
#if defined(BVH_NODES_PAGE1)
	accelNodePages[1] = accelNodePage1;
#endif
#if defined(BVH_NODES_PAGE2)
	accelNodePages[2] = accelNodePage2;
#endif
#if defined(BVH_NODES_PAGE3)
	accelNodePages[3] = accelNodePage3;
#endif
#if defined(BVH_NODES_PAGE4)
	accelNodePages[4] = accelNodePage4;
#endif
#if defined(BVH_NODES_PAGE5)
	accelNodePages[5] = accelNodePage5;
#endif
#if defined(BVH_NODES_PAGE6)
	accelNodePages[6] = accelNodePage6;
#endif
#if defined(BVH_NODES_PAGE7)
	accelNodePages[7] = accelNodePage7;
#endif

	const uint rootNodeData = accelNodePage0[0].nodeData;
	const uint stopPage = BVHNodeData_GetPageIndex(rootNodeData);
	const uint stopNode = BVHNodeData_GetNodeIndex(rootNodeData); // Non-existent
	uint currentPage = 0; // Root Node Page
#else
	const uint stopNode = BVHNodeData_GetSkipIndex(accelNodePage0[0].nodeData); // Non-existent
#endif

	const float3 rayOrig = (float3)(ray->o.x, ray->o.y, ray->o.z);
	const float3 rayDir = (float3)(ray->d.x, ray->d.y, ray->d.z);
	const float mint = ray->mint;
	float maxt = ray->maxt;

	const float3 invRayDir = 1.f / rayDir;

	uint hitMeshIndex = NULL_INDEX;
	uint hitTriangleIndex = NULL_INDEX;
	uint currentNode = 0; // Root Node

	float b1, b2;
#if (BVH_NODES_PAGE_COUNT == 1)
	while (currentNode < stopNode) {
		__global BVHAccelArrayNode *node = &accelNodePage0[currentNode];
#else
	while ((currentPage < stopPage) || (currentNode < stopNode)) {
		__global BVHAccelArrayNode *accelNodePage = accelNodePages[currentPage];
		__global BVHAccelArrayNode *node = &accelNodePage[currentNode];
#endif
		// Read the node
		__global float4 *data = (__global float4 *)node;
		const float4 data0 = *data++;
		const float4 data1 = *data;

		//const uint nodeData = node->nodeData;
		const uint nodeData = as_uint(data1.s2);
		if (BVHNodeData_IsLeaf(nodeData)) {
			// It is a leaf, check the triangle

			//const uint i0 = node->triangleLeaf.v[0];
			//const uint i1 = node->triangleLeaf.v[1];
			//const uint i2 = node->triangleLeaf.v[2];
			const uint v0 = as_uint(data0.s0);
			const uint v1 = as_uint(data0.s1);
			const uint v2 = as_uint(data0.s2);

#if (BVH_VERTS_PAGE_COUNT == 1)
			// Fast path for when there is only one memory page
			const float3 p0 = VLOAD3F(&accelVertPage0[v0].x);
			const float3 p1 = VLOAD3F(&accelVertPage0[v1].x);
			const float3 p2 = VLOAD3F(&accelVertPage0[v2].x);
#else
			const uint pv0 = (v0 & 0xe0000000u) >> 29;
			const uint iv0 = (v0 & 0x1fffffffu);
			__global Point *vp0 = accelVertPages[pv0];
			const float3 p0 = VLOAD3F(&vp0[iv0].x);

			const uint pv1 = (v1 & 0xe0000000u) >> 29;
			const uint iv1 = (v1 & 0x1fffffffu);
			__global Point *vp1 = accelVertPages[pv1];
			const float3 p1 = VLOAD3F(&vp1[iv1].x);

			const uint pv2 = (v2 & 0xe0000000u) >> 29;
			const uint iv2 = (v2 & 0x1fffffffu);
			__global Point *vp2 = accelVertPages[pv2];
			const float3 p2 = VLOAD3F(&vp2[iv2].x);
#endif

			//const uint triangleIndex = node->triangleLeaf.triangleIndex;
			const uint meshIndex = as_uint(data0.s3);
			const uint triangleIndex = as_uint(data1.s0);

			Triangle_Intersect(rayOrig, rayDir, mint, &maxt, &hitMeshIndex, &hitTriangleIndex,
					&b1, &b2, meshIndex, triangleIndex, p0, p1, p2);
#if (BVH_NODES_PAGE_COUNT == 1)
			++currentNode;
#else
			NextNode(&currentPage, &currentNode);
#endif
		} else {
			// It is a node, check the bounding box
			//const float3 pMin = VLOAD3F(&node->bvhNode.bboxMin[0]);
			//const float3 pMax = VLOAD3F(&node->bvhNode.bboxMax[0]);
			const float3 pMin = (float3)(data0.s0, data0.s1, data0.s2);
			const float3 pMax = (float3)(data0.s3, data1.s0, data1.s1);

			if (BBox_IntersectP(pMin, pMax, rayOrig, invRayDir, mint, maxt)) {
#if (BVH_NODES_PAGE_COUNT == 1)
				++currentNode;
#else
				NextNode(&currentPage, &currentNode);
#endif
			} else {
#if (BVH_NODES_PAGE_COUNT == 1)
				// I don't need to use BVHNodeData_GetSkipIndex() here because
				// I already know the flag (i.e. the last bit) is 0
				currentNode = nodeData;
#else
				currentPage = BVHNodeData_GetPageIndex(nodeData);
				currentNode = BVHNodeData_GetNodeIndex(nodeData);
#endif
			}
		}
	}

	rayHit->t = maxt;
	rayHit->b1 = b1;
	rayHit->b2 = b2;
	rayHit->meshIndex = hitMeshIndex;
	rayHit->triangleIndex = hitTriangleIndex;
}

__kernel __attribute__((work_group_size_hint(64, 1, 1))) void Accelerator_Intersect_RayBuffer(
		__global Ray *rays,
		__global RayHit *rayHits,
		const uint rayCount
		ACCELERATOR_INTERSECT_PARAM_DECL
		) {
	// Select the ray to check
	const int gid = get_global_id(0);
	if (gid >= rayCount)
		return;

	Ray ray;
	Ray_ReadAligned4_Private(&rays[gid], &ray);

	RayHit rayHit;
	Accelerator_Intersect(
		&ray,
		&rayHit
		ACCELERATOR_INTERSECT_PARAM
		);

	// Write result
	__global RayHit *memRayHit = &rayHits[gid];
	memRayHit->t = rayHit.t;
	memRayHit->b1 = rayHit.b1;
	memRayHit->b2 = rayHit.b2;
	memRayHit->meshIndex = rayHit.meshIndex;
	memRayHit->triangleIndex = rayHit.triangleIndex;
}
