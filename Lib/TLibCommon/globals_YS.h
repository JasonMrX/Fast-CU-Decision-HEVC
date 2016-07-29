#include <iostream>  
#include <fstream>  
#include <iterator>  
#include <vector>
#include <String> 
#include "cvheaders.h"
#include "TLibCommon/CommonDef.h"
#include "TLibCommon/TComYuv.h"
#include "TLibCommon/TComPic.h"
#include "TLibCommon/TComPrediction.h"
#include "TLibCommon/TComTrQuant.h"
#include "TLibCommon/TComBitCounter.h"
#include "TLibCommon/TComDataCU.h"
#include "TLibEncoder/TEncCu.h"
#include "TLibCommon/linear.h"

//////////////////////////////////// Macro Definiton 

#define OBSERVATION			0
#define OUTPUT_INSIGHTDATA  1
#define NEW_FEATURESYSTEM   0
#define PARAMETER_SELECTION 0

#define NUM_CU_DEPTH		4
#define NUM_MODELTYPE		15
#define NUM_RESULTTYPE		7
#define NUM_DECISIONTYPE	4
#define NUM_FEATURETYPE		20
#define NUM_FEATUREFORMAT	4


////////////////////////////////////  Enumeration
enum FeatureFormat
{
	Unknown,
	LibLinear,
	OpenCV,
	BayesStat
};

enum FeatureType
{
	N_OBF,
	OBF,
	EOBF,
	SOC,
	SUBSOC,
	BOutlier,
	MIXED,
	Outlier,
	NewFeature,
	OBF_SATD,
	OBF_SATD_RDCost,
	NoneFeature
};

enum FeatureName
{

	J_RMD_2Nx2N,
	J_RDO_2Nx2N

};


enum ModelType
{

	SVM_0 = 0,
	SVM_1,
	SVM_2,
	SVM_OpenCV,
	RTS_OpenCV,
	Naive,
	BayesDecision,
	Assistant_Skip,
	BayesNew,
	NoneModel
};

enum ResultType{
	NoResult = -1,
	TP,
	FP,
	TN,
	FN,
	FPLoss,
	FNLoss,
};

enum DecisionType{
	Unsure = 0,
	Skip2Nx2N,
	TerminateCU,
	SkipRDO
};

enum  CurrentState{
	Training,
	Verifying,
	Testing
};

extern double g_C[3];
extern double g_W_P[3];
extern double g_W_N[3];

//////////////////////////////////////////////////////////////////////////
// New feature system 
///////////////////////////////////////////////////////////////////////////
extern Mat*         g_TrainingDataMat; // [uiDepth]
extern ofstream*	g_TrainingDataSet;

extern FILE*		g_pFile_Performance; //= fopen("precison.txt", "at");
extern ofstream*	g_InsightDataSet;
///////////////////////////////////////////////////////////////////////////




#if SVM_ONLINE_TRAIN //  extern declaration of global model objects 
extern model* **	model_linear; // [uiDepth][FeatureType]
extern CvSVM*		CvSVM_model;
extern CvRTrees*	CvRTrees_model;
#endif




extern const char * FeatureFormat_Names[];
extern const char * FeatureType_Names[];
extern const char * ModelType_Names[];

#if ENABLE_YS_GLOBAL  // extern declaration of global variables 

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Training Statistic
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// SVM feature Scale Factor 
extern		Double* g_ScaleFactor;
extern		Double* g_d2NRDCost;
extern		Double* g_d2NBits;
extern		Double* g_d2NDistortion;
extern		Double* g_dSOC;


extern double***	g_iVerResult;// [uiDepth][ModelType][TFPN]
extern bool***		g_bDecisionSwitch;// [uiDepth][ModelType][DecisionType]
extern bool*		g_bModelSwitch;//		[ModelType]

#endif
//////////////////////////////////////
//global control variables  
//////////////////////////////////////
extern char*      g_pchInputFile;
extern Int        g_iQP;
extern Int        g_iSourceWidth;
extern Int        g_iSourceHeight;
extern Double     g_dBitRate;
extern Double     g_dYUVPSNR;
extern Double     g_dET;

extern Int			g_iPOC;  
extern Int*			g_iP;   //  [uiDepth]
extern Int*			g_iT;	//  [uiDepth]   
extern Int*			g_iV;	//  [uiDepth]

extern Double***	g_dPrecision_Th;    //  [uiDepth][ModelType][DecisionType]
extern Double		g_dPrecision_Th_Default;
extern ModelType	g_mainModelType;
extern FeatureType	g_mainFeatureType;
extern ModelType	g_modelType_2;
extern FeatureType	g_featureType_2;

extern ModelType	g_modelType_3;
extern FeatureType	g_featureType_3;


extern ofstream***	g_TrainingSet;//[uiDepth][FeatureFormat][FeatureType]
extern Int**		g_iFeatureLength; // [uiDepth][FeatureType]
extern Int**		g_iSampleNumber; // [uiDepth][FeatureType]

// contral bools 
extern Bool			g_bTrustAssistantSkip;
extern Bool			g_bMultiModel;
extern Bool			g_bDepthException;

// variables for SkipRDO rhis are couted during testing 
extern UInt			g_uiNumCU_Decision[NUM_CU_DEPTH][NUM_DECISIONTYPE];

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////