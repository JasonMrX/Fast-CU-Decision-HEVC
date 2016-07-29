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
#include "TLibCommon/globals_YS.h"
using namespace std;


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




void printPerformance(unsigned int uiDepth, ModelType modelType);
void outputPerformance(unsigned int uiDepth, ModelType modelType);
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
	Double m_adFeature[5][26];
  
  TMVFeature();

	static Double getSubBlockMean(Pel* pBlock, UInt uiWidth, Int iFrom, Int iTo, Int jFrom, Int jTo) {
		Int iSum = 0;
		for (int i = iFrom; i < iTo; i++) {
			for (int j = jFrom; j < jTo; j++) {
				iSum += pBlock[i * uiWidth + j];
			}
		}
    //return (Double)iSum;
		return ((Double) iSum) / (iTo - iFrom) / (jTo - jFrom);
	}

	static Double getSubBlockVariance(Pel* pBlock, Double iMean, UInt uiWidth, Int iFrom, Int iTo, Int jFrom, Int jTo) {
		Int iSum = 0;
    for (int i = iFrom; i < iTo; i++)  {
      for (int j = jFrom; j < jTo; j++) {
        Int tmp = pBlock[i * uiWidth + j] - iMean;
        iSum += tmp > 0 ? tmp : -tmp;
      }
    }
		return ((Double) iSum) / (iTo - iFrom) / (jTo - jFrom);
	}

};

class T3x3Filter {
private:
	Int m_aiMask[3][3];
public:

  T3x3Filter();

  T3x3Filter(Int* aiMask);

  Pel* filter(Pel* pOrg, UInt uiWidth, UInt uiHeight, UInt uiStride);

};

TMVFeature* getTMVFeature(TComDataCU*& rpcBestCU);


















