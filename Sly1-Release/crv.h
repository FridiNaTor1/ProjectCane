#pragma once
#include "lo.h"
#include "bez.h"

enum CRVK
{
	CRVK_Nil = -1,
	CRVK_Linear = 0,
	CRVK_Cubic = 1,
	CRVK_Max = 2
};

struct CRV
{
	union
	{
		struct VTCRV* pvtcrv;
		struct VTCRVL* pvtcrvl;
		struct VTCRVC* pvtcrvc;
	};

	CRVK crvk;
	int fClosed;
	int ccv;
	std::vector <float> mpicvu;
	std::vector <float> mpicvs;
	std::vector <glm::vec3> mpicvpos;
};

struct CRVL : public CRV
{

};

struct CTCE
{
	glm::vec3 apos[20];
	float mpiposs[20];
};

struct CRVC : public CRV
{
	public:
	std::vector <glm::vec3> mpicvdposIn;
	std::vector <glm::vec3> mpicvdposOut;
	CTCE ctce;
	int icvCache;
};

std::shared_ptr <CRV> PcrvNew(CRVK crvk);
void  LoadCrvlFromBrx(CRVL *pcrvl, CBinaryInputStream* pbis);
void  ConvertCrvl(CRVL* pcrvl, glm::mat4* pmatSrc, glm::mat4* pmatDst);
void  ConvertApos(int cpos, glm::vec3* apos, glm::mat4& pmatSrc, glm::mat4& pmatDst);
void  MeasureCrvl(CRVL *pcrvl);
float SMeasureApos(int cpos, glm::vec3* apos, float* mpiposs);

void LoadCrvcFromBrx(CRVC *pcrvc, CBinaryInputStream* pbis);
void ConvertCrvc(CRVC* pcrvc, glm::mat4& pmatSrc, glm::mat4& pmatDst);
void InvalidateCrvcCache(CRVC* pcrvc);
void MeasureCrvc(CRVC* pcrvc);

struct VTCRVL
{
	void (*pfnLoadCrvlFromBrx)(CRVL*, CBinaryInputStream*) = LoadCrvlFromBrx;
	void (*pfnConvertCrvl)(CRVL*, glm::mat4*, glm::mat4*) = ConvertCrvl;
	void (*pfnMeasureCrvl)(CRVL*) = MeasureCrvl;
};

struct VTCRVC
{
	void (*pfnLoadCrvcFromBrx)(CRVC*, CBinaryInputStream*) = LoadCrvcFromBrx;
	void (*pfnConvertCrvc)(CRVC*, glm::mat4&, glm::mat4&) = ConvertCrvc;
	void (*pfnMeasureCrvc)(CRVC*) = MeasureCrvc;
};

inline VTCRVL g_vtcrvl;
inline VTCRVC g_vtcrvc;