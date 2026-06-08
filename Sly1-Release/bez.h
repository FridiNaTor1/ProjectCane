#pragma once
#include "lo.h"

float SBezierPosLength(float dtSeg, float tSeg, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1);
void  TesselateBezier(float dtSeg, float tStart, float tEnd, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, int cpos, glm::vec3* apos);
void  EvaluateBezierPos(float dtSeg, float tSeg, float svt, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, glm::vec3* ppos, glm::vec3* pv, glm::vec3* pdv);