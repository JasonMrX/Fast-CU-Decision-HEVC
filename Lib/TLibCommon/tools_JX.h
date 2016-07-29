#include "TLibCommon/CommonDef.h"

class TMVFeature {
public:
  Int m_adFeature[4][26];
  TMVFeature() {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 26; j++) {
        m_adFeature[i][j] = 0;
      }
    }
  }
  
  static Int getSubBlockMean(Pel* pBlock, UInt uiWidth, Int iFrom, Int iTo, Int jFrom, Int jTo) {
    Int iSum = 0;
    for (int i = iFrom; i < iTo; i++) {
      for (int j = jFrom; j < jTo; j++) {
        iSum += pBlock[i * uiWidth + j]; 
      }
    }
    return iSum / (iTo - iFrom) / (jFrom - jTo);
  }

  static Int getSubBlockVariance(Pel* pBlock, Int iMean, UInt uiWidth, Int iFrom, Int iTo, Int jFrom, Int jTo) {
    Int iSum = 0;
    for (int i = iFrom; i < iTo; i++)  {
      for (int j = jFrom; j < jTo; j++) {
        Int tmp = pBlock[i * uiWidth + j] - iMean;
        iSum += tmp > 0 ? tmp : -tmp;
      }
    }
    return iSum / (iTo - iFrom) / (jFrom - jTo);
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
    memset(pFilt, 0, sizeof(Pel) * uiWidth * uiHeight);
    for (int y = 1; y < uiHeight - 1; y++) {
      for (int x = 1; x < uiWidth - 1; x++) {
        for (int i = 0; i < 3; i++) {
          for (int j = 0; j < 3; j++) {
            pFilt[y * uiWidth + x] = m_aiMask[i][j] * pOrg[(y - 1 + i) * uiStride + (x - 1 + j)];
          }
        }
      }
    }
    return pFilt;
  }
};

Double 
/*
class TMVFeature {

private:
  
    ad[Partition][Mean/Variance]
    Partition:
      Q: Quadratic
      V: Vertical
      H: Horizontal
      D: Diagonal
      A: Anti-Diagonal
    M/V:
      M: Mean
      V: Variance
  
  Double* adQM;
  Double* adQV;
  Double* adVM;
  Double* adVV;
  Double* adHM;
  Double* adHV;
  Double* adDM;
  Double* adDV;
  Double* adAM;
  Double* adAV;
  Double* dMean;
  Double* dVariance;

public:

  TMVFeature() {
    adQM = NULL;
    adQV = NULL;
    adVM = NULL;
    adVV = NULL;
    adHM = NULL;
    adHV = NULL;
    adDM = NULL;
    adDV = NULL;
    adAM = NULL;
    adAV = NULL;
    dMean = NULL;
    dVariance = NULL;
  }
  virtual ~TMVFeature() {
    if (adQM) delete adQM;
    if (adQV) delete adQV;
    if (adVM) delete adVM;
    if (adVV) delete adVV;
    if (adHM) delete adHM;
    if (adHV) delete adHV;
    if (adDM) delete adDM;
    if (adDV) delete adDV;
    if (adAM) delete adAM;
    if (adAV) delete adAV;
    if (dMean) delete dMean;
    if (dVariance) delete dVariance;
  }

  Double* getQM() { return adQM; }
  Double* getQV() { return adQV; }
  Double* getVM() { return adVM; }
  Double* getVV() { return adVV; }
  Double* getHM() { return adHM; }
  Double* getHV() { return adHV; }
  Double* getDM() { return adDM; }
  Double* getDV() { return adDV; }
  Double* getAM() { return adAM; }
  Double* getAV() { return adAV; }
  Double* getMean() { return dMean; }
  Double* getVariance() { return dVariance; }

  Void setQM(Double* QM) { adQM = QM; }
  Void setQV(Double* QV) { adQV = QV; }
  Void setVM(Double* VM) { adVM = VM; }
  Void setVV(Double* VV) { adVV = VV; }
  Void setHM(Double* HM) { adHM = HM; }
  Void setHV(Double* HV) { adHV = HV; }
  Void setDM(Double* DM) { adDM = DM; }
  Void setDV(Double* DV) { adDV = DV; }
  Void setAM(Double* AM) { adAM = AM; }
  Void setAV(Double* AV) { adAV = AV; }
  Void setMean(Double* Mean) { dMean = Mean; }
  Void setVariance(Double* Variance) { dVariance = Variance; }

};
*/