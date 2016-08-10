#ifndef _TOOLS_YS_
#define _TOOLS_YS_
#include <iostream>  
#include <fstream>  
#include <iterator>  
#include <vector>
#include <String> 
#include <time.h>
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

using namespace std;

//////////////////////////////////// Macro Definiton 

#define OBSERVATION			0
#define OUTPUT_INSIGHTDATA  0
#define NEW_FEATURESYSTEM   0
#define PARAMETER_SELECTION 0

#define TIME_SYSTEM         1

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
	SATD_RDCost,
	NewFeature_1,
	NewFeature_2,
	NewFeature_3,
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
	BayesNan,
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
#define NUM_FEATURE	           4;

extern FILE*		g_pFile_Performance; //= fopen("precison.txt", "at");
extern ofstream*	g_InsightDataSet;
///////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Time system 
///////////////////////////////////////////////////////////////////////////
#if TIME_SYSTEM  // Extern 
extern FILE*  g_pFile_TimeSystem;//   fopen("SummaryTotal_Time.txt", "at");

extern Double g_dTime_Total;

extern Double g_dTime_xCompressCU[4];

//CheckRDcostIntra
extern Double g_dTime_xCheckRDCostIntra_2Nx2N[4];
extern Double g_dTime_xCheckRDCostIntra_NxN;

extern Double g_dTime_estIntraPredLumaQT_2Nx2N[4];
extern Double g_dTime_estIntraPredLumaQT_NxN;
// RMD
extern Double g_dTime_RMD_2Nx2N[4];
extern Double g_dTime_RMD_NxN;

// SATD
extern Double g_dTime_SATD_2Nx2N[4];
extern Double g_dTime_SATD_NxN;

//RDO
extern Double g_dTime_RDO_2Nx2N[4];
extern Double g_dTime_RDO_NxN;

// Overall 
extern Double g_dTime_Training[4];
extern Double g_dTime_FeatureExtraction[4];
extern Double g_dTime_Prediction[4];
#endif


#if SVM_ONLINE_TRAIN //  extern declaration of global model objects 
extern model* **	model_linear; // [uiDepth][FeatureType]
extern CvSVM**		model_CvSVM;
extern CvRTrees**	model_CvRTrees;
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
extern Int		  g_iFrameToBeEncoded;

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





























template <typename T> void OutputFeatureVector(ofstream* file, T* feature, Int FeatureLength, Double label, UInt uiDepth, FeatureFormat format);
template <typename T> void FormFeatureVector(TComDataCU*& rpcBestCU, UInt uiDepth, T*& feature, Int& FeatureLength, FeatureType Type);
void OutputInsightDataSet(TComDataCU*& rpcBestCU, UInt uiDepth);
template <typename T> void DoPrediction(UInt uiDepth, T* feature, Int FeatureLength, Double& label_predict, ModelType modelType, FeatureType featureType);
template <typename T> void DoPrediction(UInt uiDepth, Int x_OBF, Double& label_predict, ModelType modelType);

void OnlineTrain(Int uiDepth, ModelType modelType, FeatureType featureType);
void OnlineTrain_NewFeatureSystem(Int uiDepth);

void createYSGlobal();
void YSGlobalControl();
void createStat();
void resetYSGlobal(Int uiDepth);
void SetDecisionSwitch(int uiDepth, ModelType modelType);
void deleteYSGlobal();

void openOutputFiles();
void closeOutputFiles();


#if TIME_SYSTEM  // Declaration of Functions
void resetTimeSystem();
void printTimeSystem();
#endif


/////////////// geters 
FeatureFormat getFeatureFormat(ModelType modelType);
string		  getTrainingSetFileName(unsigned int uiDepth, ModelType modelType, FeatureFormat featureFormat, FeatureType featureType);
string		  getOffLineModelNmae(unsigned int uiDepth, ModelType modelType, FeatureType featureType);
string		  getInsightDataSetFileName(unsigned int uiDepth);
string        getTrainingDataSetFileName(unsigned int uiDepth);
CurrentState  getCurrentState(unsigned int uiDepth);




///////////////////////  performance tools 
void countTFPN(int prediction, bool label, double***& iVerResult, unsigned int uiDepth, ModelType modelType);
void countRDLoss(int prediction, bool label, double RDCost2Nx2N, double RDCostNxN, double***& iVerResult, unsigned int uiDepth, ModelType modelType);
template <typename T> double getVerAccuracy(UInt uiDepth, ModelType modelType);
template <typename T> double getSkipPrecision(UInt uiDepth, ModelType modelType);
template <class T> double getTermPrecision(UInt uiDepth, ModelType modelType);
template <class T> double getSkipSensitivity(UInt uiDepth, ModelType modelType);
template <class T> double getTermSensitivity(UInt uiDepth, ModelType modelType);




void   printPerformance(unsigned int uiDepth, ModelType modelType);
void   outputPerformance(unsigned int uiDepth, ModelType modelType);
double FeatureProcess(Int Feature, Int FeatureType, Int uiDepth);


/////////////// CDM  CU decision map 
int LoadCDM(string fileName, cv::Mat& matData, int dim, const int* size, int matChns);
int WriteCDM(string fileName, cv::Mat& matData, int dim, const int* size);
int Test_3DMat();

/////////////////////////////////
/// basic_tools
///////////////////////////////////
///////////  Mat read and write 
int WriteData(string fileName, Mat& matData);
int LoadData(string fileName, cv::Mat& matData, int matRows, int matCols, int matChns);

//// create array 
template <typename T> void create3dFileArray(T***& array, int d_1, int d_2, int d_3);
template <typename T> void create2dFileArray(T**& array, int d_1, int d_2);
template <typename T> void create3dArray(T***& array, int d_1, int d_2, int d_3);
template <typename T> void create2dArray(T**& array, int d_1, int d_2);
// clear array 
template <typename T> void clear2dArray(T**& array, int d_1, int d_2);
template <typename T> void clear3dArray(T***& array, int d_1, int d_2, int d_3);

// set array 
template <typename T> void setArray(T*& array, int d_1, T value);

// delete array 
template <typename T> void delete3dArray(T***& array, int d_1, int d_2, int d_3);
template <typename T> void delete2dArray(T**& array, int d_1, int d_2);
// print array 
template <typename T> void print3dArray(T*** array, int d_1, int d_2, int d_3);


Int findPos(Double *data, Int right, Double tgt);


class TMVFeature {

public:
	Double m_adFeature[4][26];
	TMVFeature() {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 26; j++) {
				m_adFeature[i][j] = 0;
			}
		}
	}

	static Double getSubBlockMean(Pel* pBlock, UInt uiWidth, Int iFrom, Int iTo, Int jFrom, Int jTo) {
		Int iSum = 0;
		for (int i = iFrom; i < iTo; i++) {
			for (int j = jFrom; j < jTo; j++) {
				iSum += pBlock[i * uiWidth + j];
			}
		}
		//return (Double)iSum;
		return ((Double)iSum) / (iTo - iFrom) / (jTo - jFrom);
	}

	static Double getSubBlockVariance(Pel* pBlock, Double iMean, UInt uiWidth, Int iFrom, Int iTo, Int jFrom, Int jTo) {
		Int iSum = 0;
		for (int i = iFrom; i < iTo; i++)  {
			for (int j = jFrom; j < jTo; j++) {
				Int tmp = pBlock[i * uiWidth + j] - iMean;
				iSum += tmp > 0 ? tmp : -tmp;
			}
		}
		return ((Double)iSum) / (iTo - iFrom) / (jTo - jFrom);
	}


};

class T3x3Filter {
private:
	Int m_aiMask[3][3];
public:

	T3x3Filter() {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				m_aiMask[i][j] = 0;
			}
		}
		m_aiMask[1][1] = 1;
	}

	T3x3Filter(Int* aiMask) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				m_aiMask[i][j] = aiMask[i * 3 + j];
			}
		}
	}

	Pel* filter(Pel* pOrg, UInt uiWidth, UInt uiHeight, UInt uiStride) {
		Pel* pFilt = (Pel*)xMalloc(Pel, uiWidth * uiHeight);
		memset(pFilt, 0, sizeof(Pel)* uiWidth * uiHeight);
		for (int y = 1; y < uiHeight - 1; y++) {
			for (int x = 1; x < uiWidth - 1; x++) {
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 3; j++) {
						pFilt[y * uiWidth + x] += m_aiMask[i][j] * pOrg[(y - 1 + i) * uiStride + (x - 1 + j)];
					}
				}
			}
		}
		return pFilt;
	}
};

TMVFeature* getTMVFeature(TComDataCU*& rpcBestCU);


class BayesModel{
private:
	int J[4][256][2];
	int N[4][256][2];
public:
	BayesModel(){
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 256; j++){
				for (int k = 0; k < 2; k++){
					J[i][j][k] = 0;
					N[i][j][k] = 0;
				}
			}
		}
	}
	void clearModel(){
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 256; j++){
				for (int k = 0; k < 2; k++){
					J[i][j][k] = 0;
					N[i][j][k] = 0;
				}
			}
		}
	}
	void updateModel(int uiDepth, int J0, int J1,int N_OBF){
		if (J0>J1){
			J[uiDepth][N_OBF][0] += J0 - J1;
			N[uiDepth][N_OBF][1] ++;
		}
		else{
			J[uiDepth][N_OBF][1] += J1 - J0;
			N[uiDepth][N_OBF][0] ++;
		}
	}

	int predict(int uiDepth,int N_OBF){
		bool bUseWeight = true;
		if (bUseWeight){

		if (J[uiDepth][N_OBF][0] < J[uiDepth][N_OBF][1]){
			return -1;
		}
		else{
			return +1;
		}

		}
		else{

		if (N[uiDepth][N_OBF][1] < N[uiDepth][N_OBF][0]){
			return -1;
		}
		else{
			return +1;
		}

		}
	}

	void outputModel(){
		FILE* pFILE = fopen("BayesModel.txt", "at");
		for (int uiDepth = 0; uiDepth < 4; uiDepth++){
			for (int j = 0; j < 2; j++){
				for (int N_OBF = 0; N_OBF < 256; N_OBF++){
				fprintf(pFILE, "%d\t", N[uiDepth][N_OBF][j]);
				}
				fprintf(pFILE, "\n");
			}
			fprintf(pFILE, "\n");
		}
	fclose(pFILE);
	}
};


extern BayesModel g_cBayesModel_Nan;
extern BayesModel g_cBayesModel_N_OBF;

#endif










