/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2015, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TEncCu.cpp
    \brief    Coding Unit (CU) encoder class
*/

#include <stdio.h>
#include "TEncTop.h"
#include "TEncCu.h"
#include "TEncAnalyze.h"
#include "TLibCommon/Debug.h"

#include <cmath>
#include <algorithm>
#include <string>
//#include "TLibCommon/svm.h"
#include "TLibCommon/linear.h"
#include "TLibCommon/tools_YS.h"
using namespace std;






#if CREAT_CDM
extern MatND** MatCDM;
#endif 
//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

/**
 \param    uhTotalDepth  total number of allowable depth
 \param    uiMaxWidth    largest CU width
 \param    uiMaxHeight   largest CU height
 \param    chromaFormat  chroma format
 */

#if MODIFICATION_YS

Bool TEncCu::xCheckRDCostIntra_Rough(
#if SKIP_RDO_ENABLE
	Bool& bSkipRDO,
#endif
	TComDataCU *&rpcBestCU,
	TComDataCU *&rpcTempCU,
	Double      &cost,
	PartSize     eSize
	DEBUG_STRING_FN_DECLARE(sDebug))
{
	Bool Partition = 0;
	DEBUG_STRING_NEW(sTest)

		UInt uiDepth = rpcTempCU->getDepth(0);

	rpcTempCU->setSkipFlagSubParts(false, 0, uiDepth);

	rpcTempCU->setPartSizeSubParts(eSize, 0, uiDepth);
	rpcTempCU->setPredModeSubParts(MODE_INTRA, 0, uiDepth);
	rpcTempCU->setChromaQpAdjSubParts(rpcTempCU->getCUTransquantBypass(0) ? 0 : m_ChromaQpAdjIdc, 0, uiDepth);

	Pel resiLuma[NUMBER_OF_STORED_RESIDUAL_TYPES][MAX_CU_SIZE * MAX_CU_SIZE];

	bSkipRDO = false;
	m_pcPredSearch->estIntraPredLumaQT(
#if SKIP_RDO_ENABLE
		bSkipRDO,
#endif
		rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth], resiLuma DEBUG_STRING_PASS_INTO(sTest));

	if (bSkipRDO){
		rpcTempCU->getTotalCost() = MAX_DOUBLE;
		return false;
	}

	m_ppcRecoYuvTemp[uiDepth]->copyToPicComponent(COMPONENT_Y, rpcTempCU->getPic()->getPicYuvRec(), rpcTempCU->getCtuRsAddr(), rpcTempCU->getZorderIdxInCtu());

	if (rpcBestCU->getPic()->getChromaFormat() != CHROMA_400)
	{
		m_pcPredSearch->estIntraPredChromaQT(rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth], resiLuma DEBUG_STRING_PASS_INTO(sTest));
	}

	m_pcEntropyCoder->resetBits();

	if (rpcTempCU->getSlice()->getPPS()->getTransquantBypassEnableFlag())
	{
		m_pcEntropyCoder->encodeCUTransquantBypassFlag(rpcTempCU, 0, true);
	}

	m_pcEntropyCoder->encodeSkipFlag(rpcTempCU, 0, true);
	m_pcEntropyCoder->encodePredMode(rpcTempCU, 0, true);
	m_pcEntropyCoder->encodePartSize(rpcTempCU, 0, uiDepth, true);
	m_pcEntropyCoder->encodePredInfo(rpcTempCU, 0);
	m_pcEntropyCoder->encodeIPCMInfo(rpcTempCU, 0, true);

	// Encode Coefficients
	Bool bCodeDQP = getdQPFlag();
	Bool codeChromaQpAdjFlag = getCodeChromaQpAdjFlag();
	m_pcEntropyCoder->encodeCoeff(rpcTempCU, 0, uiDepth, bCodeDQP, codeChromaQpAdjFlag);
	setCodeChromaQpAdjFlag(codeChromaQpAdjFlag);
	setdQPFlag(bCodeDQP);

	m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);

	rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits();
	rpcTempCU->getTotalBins() = ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
	rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion());

	xCheckDQP(rpcTempCU);

	cost = rpcTempCU->getTotalCost();

	Partition = xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth DEBUG_STRING_PASS_INTO(sDebug) DEBUG_STRING_PASS_INTO(sTest));
	return Partition;
}

#endif






Void TEncCu::create(UChar uhTotalDepth, UInt uiMaxWidth, UInt uiMaxHeight, ChromaFormat chromaFormat)
{
  Int i;

  m_uhTotalDepth   = uhTotalDepth + 1;
  m_ppcBestCU      = new TComDataCU*[m_uhTotalDepth-1];
  m_ppcTempCU      = new TComDataCU*[m_uhTotalDepth-1];

  m_ppcPredYuvBest = new TComYuv*[m_uhTotalDepth-1];
  m_ppcResiYuvBest = new TComYuv*[m_uhTotalDepth-1];
  m_ppcRecoYuvBest = new TComYuv*[m_uhTotalDepth-1];
  m_ppcPredYuvTemp = new TComYuv*[m_uhTotalDepth-1];
  m_ppcResiYuvTemp = new TComYuv*[m_uhTotalDepth-1];
  m_ppcRecoYuvTemp = new TComYuv*[m_uhTotalDepth-1];
  m_ppcOrigYuv     = new TComYuv*[m_uhTotalDepth-1];

  UInt uiNumPartitions;
  for( i=0 ; i<m_uhTotalDepth-1 ; i++)
  {
    uiNumPartitions = 1<<( ( m_uhTotalDepth - i - 1 )<<1 );
    UInt uiWidth  = uiMaxWidth  >> i;
    UInt uiHeight = uiMaxHeight >> i;

    m_ppcBestCU[i] = new TComDataCU; m_ppcBestCU[i]->create( chromaFormat, uiNumPartitions, uiWidth, uiHeight, false, uiMaxWidth >> (m_uhTotalDepth - 1) );
    m_ppcTempCU[i] = new TComDataCU; m_ppcTempCU[i]->create( chromaFormat, uiNumPartitions, uiWidth, uiHeight, false, uiMaxWidth >> (m_uhTotalDepth - 1) );

    m_ppcPredYuvBest[i] = new TComYuv; m_ppcPredYuvBest[i]->create(uiWidth, uiHeight, chromaFormat);
    m_ppcResiYuvBest[i] = new TComYuv; m_ppcResiYuvBest[i]->create(uiWidth, uiHeight, chromaFormat);
    m_ppcRecoYuvBest[i] = new TComYuv; m_ppcRecoYuvBest[i]->create(uiWidth, uiHeight, chromaFormat);

    m_ppcPredYuvTemp[i] = new TComYuv; m_ppcPredYuvTemp[i]->create(uiWidth, uiHeight, chromaFormat);
    m_ppcResiYuvTemp[i] = new TComYuv; m_ppcResiYuvTemp[i]->create(uiWidth, uiHeight, chromaFormat);
    m_ppcRecoYuvTemp[i] = new TComYuv; m_ppcRecoYuvTemp[i]->create(uiWidth, uiHeight, chromaFormat);

    m_ppcOrigYuv    [i] = new TComYuv; m_ppcOrigYuv    [i]->create(uiWidth, uiHeight, chromaFormat);
  }

  m_bEncodeDQP          = false;
  m_CodeChromaQpAdjFlag = false;
  m_ChromaQpAdjIdc      = 0;

  // initialize partition order.
  UInt* piTmp = &g_auiZscanToRaster[0];
  initZscanToRaster( m_uhTotalDepth, 1, 0, piTmp);
  initRasterToZscan( uiMaxWidth, uiMaxHeight, m_uhTotalDepth );

  // initialize conversion matrix from partition index to pel
  initRasterToPelXY( uiMaxWidth, uiMaxHeight, m_uhTotalDepth );
}

Void TEncCu::destroy()
{
  Int i;

  for( i=0 ; i<m_uhTotalDepth-1 ; i++)
  {
    if(m_ppcBestCU[i])
    {
      m_ppcBestCU[i]->destroy();      delete m_ppcBestCU[i];      m_ppcBestCU[i] = NULL;
    }
    if(m_ppcTempCU[i])
    {
      m_ppcTempCU[i]->destroy();      delete m_ppcTempCU[i];      m_ppcTempCU[i] = NULL;
    }
    if(m_ppcPredYuvBest[i])
    {
      m_ppcPredYuvBest[i]->destroy(); delete m_ppcPredYuvBest[i]; m_ppcPredYuvBest[i] = NULL;
    }
    if(m_ppcResiYuvBest[i])
    {
      m_ppcResiYuvBest[i]->destroy(); delete m_ppcResiYuvBest[i]; m_ppcResiYuvBest[i] = NULL;
    }
    if(m_ppcRecoYuvBest[i])
    {
      m_ppcRecoYuvBest[i]->destroy(); delete m_ppcRecoYuvBest[i]; m_ppcRecoYuvBest[i] = NULL;
    }
    if(m_ppcPredYuvTemp[i])
    {
      m_ppcPredYuvTemp[i]->destroy(); delete m_ppcPredYuvTemp[i]; m_ppcPredYuvTemp[i] = NULL;
    }
    if(m_ppcResiYuvTemp[i])
    {
      m_ppcResiYuvTemp[i]->destroy(); delete m_ppcResiYuvTemp[i]; m_ppcResiYuvTemp[i] = NULL;
    }
    if(m_ppcRecoYuvTemp[i])
    {
      m_ppcRecoYuvTemp[i]->destroy(); delete m_ppcRecoYuvTemp[i]; m_ppcRecoYuvTemp[i] = NULL;
    }
    if(m_ppcOrigYuv[i])
    {
      m_ppcOrigYuv[i]->destroy();     delete m_ppcOrigYuv[i];     m_ppcOrigYuv[i] = NULL;
    }
  }
  if(m_ppcBestCU)
  {
    delete [] m_ppcBestCU;
    m_ppcBestCU = NULL;
  }
  if(m_ppcTempCU)
  {
    delete [] m_ppcTempCU;
    m_ppcTempCU = NULL;
  }

  if(m_ppcPredYuvBest)
  {
    delete [] m_ppcPredYuvBest;
    m_ppcPredYuvBest = NULL;
  }
  if(m_ppcResiYuvBest)
  {
    delete [] m_ppcResiYuvBest;
    m_ppcResiYuvBest = NULL;
  }
  if(m_ppcRecoYuvBest)
  {
    delete [] m_ppcRecoYuvBest;
    m_ppcRecoYuvBest = NULL;
  }
  if(m_ppcPredYuvTemp)
  {
    delete [] m_ppcPredYuvTemp;
    m_ppcPredYuvTemp = NULL;
  }
  if(m_ppcResiYuvTemp)
  {
    delete [] m_ppcResiYuvTemp;
    m_ppcResiYuvTemp = NULL;
  }
  if(m_ppcRecoYuvTemp)
  {
    delete [] m_ppcRecoYuvTemp;
    m_ppcRecoYuvTemp = NULL;
  }
  if(m_ppcOrigYuv)
  {
    delete [] m_ppcOrigYuv;
    m_ppcOrigYuv = NULL;
  }
}

/** \param    pcEncTop      pointer of encoder class
 */
Void TEncCu::init( TEncTop* pcEncTop )
{
  m_pcEncCfg           = pcEncTop;
  m_pcPredSearch       = pcEncTop->getPredSearch();
  m_pcTrQuant          = pcEncTop->getTrQuant();
  m_pcRdCost           = pcEncTop->getRdCost();

  m_pcEntropyCoder     = pcEncTop->getEntropyCoder();
  m_pcBinCABAC         = pcEncTop->getBinCABAC();

  m_pppcRDSbacCoder    = pcEncTop->getRDSbacCoder();
  m_pcRDGoOnSbacCoder  = pcEncTop->getRDGoOnSbacCoder();

  m_pcRateCtrl         = pcEncTop->getRateCtrl();
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** 
 \param  pCtu pointer of CU data class
 */
Void TEncCu::compressCtu( TComDataCU* pCtu )
{
  // initialize CU data
  m_ppcBestCU[0]->initCtu( pCtu->getPic(), pCtu->getCtuRsAddr() );
  m_ppcTempCU[0]->initCtu(pCtu->getPic(), pCtu->getCtuRsAddr());



  // analysis of CU
  DEBUG_STRING_NEW(sDebug)
  xCompressCU( m_ppcBestCU[0], m_ppcTempCU[0], 0 DEBUG_STRING_PASS_INTO(sDebug) );

  DEBUG_STRING_OUTPUT(std::cout, sDebug)

#if YS_DEBUG  // output the Depth Information
  //std::cout << "Depth:0 X:" << m_ppcBestCU[0]->getCUPelX() << '\n';   //YS add
  //std::cout << "Depth:1 X:" << m_ppcBestCU[1]->getCUPelX() << '\n';   //YS add
#endif
#if ADAPTIVE_QP_SELECTION
  if( m_pcEncCfg->getUseAdaptQpSelect() )
  {
    if(pCtu->getSlice()->getSliceType()!=I_SLICE) //   IIII
    {
      xCtuCollectARLStats( pCtu );
    }
  }
#endif
}
/** \param  pCtu  pointer of CU data class
 */
Void TEncCu::encodeCtu ( TComDataCU* pCtu )
{
  if ( pCtu->getSlice()->getPPS()->getUseDQP() )
  {
    setdQPFlag(true);
  }

  if ( pCtu->getSlice()->getUseChromaQpAdj() )
  {
    setCodeChromaQpAdjFlag(true);
  }

  // Encode CU data
  xEncodeCU( pCtu, 0, 0 );
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================
//! Derive small set of test modes for AMP encoder speed-up
#if AMP_ENC_SPEEDUP
#if AMP_MRG
Void TEncCu::deriveTestModeAMP (TComDataCU *pcBestCU, PartSize eParentPartSize, Bool &bTestAMP_Hor, Bool &bTestAMP_Ver, Bool &bTestMergeAMP_Hor, Bool &bTestMergeAMP_Ver)
#else
Void TEncCu::deriveTestModeAMP (TComDataCU *pcBestCU, PartSize eParentPartSize, Bool &bTestAMP_Hor, Bool &bTestAMP_Ver)
#endif
{
  if ( pcBestCU->getPartitionSize(0) == SIZE_2NxN )
  {
    bTestAMP_Hor = true;
  }
  else if ( pcBestCU->getPartitionSize(0) == SIZE_Nx2N )
  {
    bTestAMP_Ver = true;
  }
  else if ( pcBestCU->getPartitionSize(0) == SIZE_2Nx2N && pcBestCU->getMergeFlag(0) == false && pcBestCU->isSkipped(0) == false )
  {
    bTestAMP_Hor = true;
    bTestAMP_Ver = true;
  }

#if AMP_MRG
  //! Utilizing the partition size of parent PU
  if ( eParentPartSize >= SIZE_2NxnU && eParentPartSize <= SIZE_nRx2N )
  {
    bTestMergeAMP_Hor = true;
    bTestMergeAMP_Ver = true;
  }

  if ( eParentPartSize == NUMBER_OF_PART_SIZES ) //! if parent is intra
  {
    if ( pcBestCU->getPartitionSize(0) == SIZE_2NxN )
    {
      bTestMergeAMP_Hor = true;
    }
    else if ( pcBestCU->getPartitionSize(0) == SIZE_Nx2N )
    {
      bTestMergeAMP_Ver = true;
    }
  }

  if ( pcBestCU->getPartitionSize(0) == SIZE_2Nx2N && pcBestCU->isSkipped(0) == false )
  {
    bTestMergeAMP_Hor = true;
    bTestMergeAMP_Ver = true;
  }

  if ( pcBestCU->getWidth(0) == 64 )
  {
    bTestAMP_Hor = false;
    bTestAMP_Ver = false;
  }
#else
  //! Utilizing the partition size of parent PU
  if ( eParentPartSize >= SIZE_2NxnU && eParentPartSize <= SIZE_nRx2N )
  {
    bTestAMP_Hor = true;
    bTestAMP_Ver = true;
  }

  if ( eParentPartSize == SIZE_2Nx2N )
  {
    bTestAMP_Hor = false;
    bTestAMP_Ver = false;
  }
#endif
}
#endif


// ====================================================================================================================
// Protected member functions
// ====================================================================================================================
/** Compress a CU block recursively with enabling sub-CTU-level delta QP
 *  - for loop of QP value to compress the current CU with all possible QP
*/




#if AMP_ENC_SPEEDUP
Void TEncCu::xCompressCU( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth DEBUG_STRING_FN_DECLARE(sDebug_), PartSize eParentPartSize )
#else
Void TEncCu::xCompressCU( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth )
#endif
{

#if TIME_SYSTEM
	clock_t clockStart_xCompressCU= clock();
#endif
	DEBUG_STRING_NEW(sDebug)
#if SVM_YS
		//// Model 1 
		Double* feature_1 = NULL;
	Int featureLength_1 = 0;
	FeatureFormat featureFormat_1 = Unknown;
	FeatureType	  featureType_1 = g_mainFeatureType;
	ModelType     modelType_1 = g_mainModelType;
	featureFormat_1 = getFeatureFormat(modelType_1);
	Double label_predict_1 = 0;
	////  Model 2
	Double* feature_2 = NULL;
	Int featureLength_2 = 0;
	FeatureFormat featureFormat_2 = Unknown;
	FeatureType featureType_2 = g_featureType_2;
	ModelType modelType_2 = g_modelType_2;
	featureFormat_2 = getFeatureFormat(modelType_2);
	Double label_predict_2 = 0;
	//// Model 3
	Double* feature_3 = NULL;
	Int featureLength_3 = 0;
	FeatureFormat featureFormat_3 = Unknown;
	FeatureType featureType_3 = g_featureType_3;
	ModelType modelType_3 = g_modelType_3;
	featureFormat_3 = getFeatureFormat(modelType_3);
	Double label_predict_3 = 0;
	// 
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    SS0 
///////////////////////////////////////////////// Normal HM data /////////////////////////////////////////////////////////////////////
	UInt uiLPelX = rpcBestCU->getCUPelX();
	UInt uiRPelX = uiLPelX + rpcBestCU->getWidth(0) - 1;
	UInt uiTPelY = rpcBestCU->getCUPelY();
	UInt uiBPelY = uiTPelY + rpcBestCU->getHeight(0) - 1;

	TComPic* pcPic = rpcBestCU->getPic();
	m_ppcOrigYuv[uiDepth]->copyFromPicYuv(pcPic->getPicYuvOrg(), rpcBestCU->getCtuRsAddr(), rpcBestCU->getZorderIdxInCtu());

	TComPicYuv* dataOrg = pcPic->getPicYuvOrg();
	Pel* pOrg = dataOrg->getAddr(COMPONENT_Y);
	UInt uiStrideOrg = dataOrg->getStride(COMPONENT_Y);

	Int BlockSize = g_uiMaxCUWidth >> uiDepth;

	const UInt uiFrameWidth = dataOrg->getWidth(COMPONENT_Y);
	const UInt uiFrameHeight = dataOrg->getHeight(COMPONENT_Y);

	Bool    bSubBranch = true;
	// variable for Cbf fast mode PU decision
	Bool    doNotBlockPu = true;
	Bool    earlyDetectionSkipMode = false;
	Bool	bBoundary = false;

	//YS add set the bBoundary early
	bBoundary = !((uiRPelX < rpcBestCU->getSlice()->getSPS()->getPicWidthInLumaSamples()) &&
		(uiBPelY < rpcBestCU->getSlice()->getSPS()->getPicHeightInLumaSamples()));

	Int	iBaseQP = xComputeQP(rpcBestCU, uiDepth);
	Int	iMinQP = g_iQP;
	Int iMaxQP = g_iQP;
	Bool isAddLowestQP = false;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//		SS1
	////////////////////////////////////////////////////Data Building Section///////////////////////////////////////
	// Encoding data
	Double	d2NRDCost = -1;
	Double	d2NDistortion = -1;
	Double	d2NBits = -1;
	Int		iIntraMode = -1;
	Double	Feature_Scale_Factor = 400;

	CurrentState currentState = getCurrentState(uiDepth);
	///////////////////////////////////////////////////////////////////////////
	// core control flag
	Bool bSkip2Nx2N = false;
	Bool bSkipRDO = false;
	Bool bEarlyTerminate = false;
	//////////////////////////////////////////////////////////////////////////
	// Assistant Decision 
	Bool bEarlyTerminate_Assistant = false;
	Int  N_NonZeroFeature = 0;
	///////////////////////////////////////////////////////////////////// Statistics  

	Bool    bPartition_Do = false;
	Bool    bPartition_True = false;

	short	 N_OBF = 0;   // number of OBF in current CU
	Int	 Ndx_LeftCol = 0;
	Int	 Ndx_UpperRow = 0;
	UInt uiHasOutlier = 0;


	Int	 N_Outlier = 0;   // number of Binarilized Outlier in current CU
	Int	 NumOutlier_LeftCol = 0;   
	Int	 NumOutlier_UpperRow = 0;   
	Double	J0 = MAX_DOUBLE;     // the RD cost of current depth  
	Double	J1 = 0;              // the RD cost of next depth 
	//  get the QP of current CU
	g_iQP = iBaseQP;

#if OUTPUT_INSIGHTDATA  // POC X Y
	if (currentState==Training&&!bBoundary){
		//cout << g_iPOC;
		//g_InsightDataSet[uiDepth] <<(int)g_iPOC << '\t';
		//g_InsightDataSet[uiDepth] << uiLPelX << '\t';
		//g_InsightDataSet[uiDepth] << uiTPelY << '\t';
	}
#endif
#if MODIFICATION_YS
	Int iIntraMode_UpperCU = rpcBestCU->getIntraMode_UpperCU();   // read the intramode of upperCU
	if (iIntraMode_UpperCU == -1)                                 // if UpperCU data is NULL , assign -1 
	{
		iIntraMode_UpperCU = -1;
	}
	if (iIntraMode_UpperCU != NULL && uiDepth == 1)
	{
		//cout << iIntraMode_UpperCU <<" ";
	}
#endif

#if GEN_OUTLIER
	TComPicYuv*		dataOBF = pcPic->getOBF();
	Pel*			pOBF = dataOBF->getAddr(COMPONENT_Y);
	UInt			uiStrideOBF = dataOBF->getStride(COMPONENT_Y);
	UInt			uiFrameWidth_OBF = dataOBF->getWidth(COMPONENT_Y);
	UInt			uiFrameHeight_OBF = dataOBF->getHeight(COMPONENT_Y);
	TComPicYuv*		dataOutlier		= pcPic->getPicYuvOutlier();
	Pel*			pOutlier		= dataOutlier->getAddr(COMPONENT_Y);
	UInt			uiStrideOutlier = dataOutlier->getStride(COMPONENT_Y);
#endif


#if SVM_YS
	Int SVM_MinDepth = SVM_MINDEPTH;
	Int SVM_MaxDepth = SVM_MAXDEPTH;
#endif
	//////////////////////////////////////////////////////////// control boolean /////////////////////////////////////
	Bool  b2Nx2NChecked = 0;
	Bool  bNxNChecked = 0;

	Double PredictionResult[NUM_MODELTYPE];
	Int  Decision[NUM_MODELTYPE];
	memset(PredictionResult, 0, sizeof(Int)*(NUM_MODELTYPE));
	memset(Decision, Unsure, sizeof(Int)*(NUM_MODELTYPE));
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if GEN_OUTLIER
#if SVM_YS
	if (uiDepth >= SVM_MinDepth && uiDepth <= SVM_MaxDepth) // Depth control
	{
#endif
		// count the number of OBF
		for (int y = 0; y < BlockSize / 4; y++)
		{
			for (int x = 0; x < BlockSize / 4; x++)
			{
				if (pOBF[x + uiLPelX / 4 + (uiTPelY / 4 + y)*uiStrideOBF] > 0){
					N_OBF++;
					N_Outlier += pOBF[x + uiLPelX / 4 + (uiTPelY / 4 + y)*uiStrideOBF];
				}
			} // end forloop
		}
		uiHasOutlier = N_OBF>0 ? 1 : 0;
		N_NonZeroFeature = N_OBF;
#if SVM_YS
	} // endif 
#endif
#endif //GEN_OUTLIER
	rpcBestCU->setNumOutlier(N_Outlier);
	rpcTempCU->setNumOutlier(N_Outlier);

	rpcBestCU->setNumOBF(N_OBF);
	rpcTempCU->setNumOBF(N_OBF);



#if OUTPUT_INSIGHTDATA  // N_outlier
	if (currentState==Training&&!bBoundary){
		//g_InsightDataSet[uiDepth] << N_Outlier << '\t';  // RDCost 2Nx2N
	}
#endif



#if NEW_FEATURESYSTEM
	Double* featureVector;
	Int featureLength;
#endif 




	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//		SS2
	//////////////////////////////////////////////	Prediction Making //////////////////////////////////////////////////////////
	/// MODEL 1
#if SVM_PREDICTION_ENABLE	
	if (uiDepth >= SVM_MinDepth && uiDepth <= SVM_MaxDepth && !bBoundary && g_bModelSwitch[modelType_1])
	{
	switch (currentState)
	{
		case Training:
			FormFeatureVector(rpcBestCU, uiDepth, feature_1, featureLength_1, featureType_1);
			break;
		case Verifying:
			FormFeatureVector(rpcBestCU, uiDepth, feature_1, featureLength_1, featureType_1);
			DoPrediction(uiDepth, feature_1, featureLength_1, label_predict_1, modelType_1, featureType_1);
			break;
		case Testing:
			if (g_bDecisionSwitch[uiDepth][modelType_1][TerminateCU] == true || g_bDecisionSwitch[uiDepth][modelType_1][Skip2Nx2N] == true)
			{
			FormFeatureVector(rpcBestCU, uiDepth, feature_1, featureLength_1, featureType_1);
			DoPrediction(uiDepth, feature_1, featureLength_1, label_predict_1, modelType_1, featureType_1);
			}
			break;
		default:
			cout << "Unknown currentState" << endl;
			break;
	}
	if (label_predict_1 > 0){
		Decision[modelType_1] = Skip2Nx2N;
	}
	else if (label_predict_1 < 0){
		Decision[modelType_1] = TerminateCU;
	}
	else {
		Decision[modelType_1] = Unsure;
	}
	}

#endif  // endif : SVM_PREDICTION_ENABLE

#if  TOGGLE_NONUSE

  const UInt numberValidComponents = rpcBestCU->getPic()->getNumberValidComponents();
	/*
  if( (g_uiMaxCUWidth>>uiDepth) >= (g_uiMaxCUWidth >> ( rpcTempCU->getSlice()->getPPS()->getMaxCuDQPDepth())) )
  {
    Int idQP = m_pcEncCfg->getMaxDeltaQP();
    iMinQP = Clip3( -rpcTempCU->getSlice()->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, iBaseQP-idQP );
    iMaxQP = Clip3( -rpcTempCU->getSlice()->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, iBaseQP+idQP );
  }
  else
  {
    iMinQP = rpcTempCU->getQP(0);
    iMaxQP = rpcTempCU->getQP(0);
  }

  if ( m_pcEncCfg->getUseRateCtrl() )
  {
    iMinQP = m_pcRateCtrl->getRCQP();
    iMaxQP = m_pcRateCtrl->getRCQP();
  }

  // transquant-bypass (TQB) processing loop variable initialisation ---
*/
  const Int lowestQP = iMinQP; // For TQB, use this QP which is the lowest non TQB QP tested (rather than QP'=0) - that way delta QPs are smaller, and TQB can be tested at all CU levels.
/*
  if ( (rpcTempCU->getSlice()->getPPS()->getTransquantBypassEnableFlag()) )
  {
    isAddLowestQP = true; // mark that the first iteration is to cost TQB mode.
    iMinQP = iMinQP - 1;  // increase loop variable range by 1, to allow testing of TQB mode along with other QPs
    if ( m_pcEncCfg->getCUTransquantBypassFlagForceValue() )
    {
      iMaxQP = iMinQP;
    }
  }
*/

  TComSlice * pcSlice = rpcTempCU->getPic()->getSlice(rpcTempCU->getPic()->getCurrSliceIdx());
  // We need to split, so don't try these modes.

  if ((uiRPelX < rpcBestCU->getSlice()->getSPS()->getPicWidthInLumaSamples()) &&
	  (uiBPelY < rpcBestCU->getSlice()->getSPS()->getPicHeightInLumaSamples()))
  {

/*	  
	  for (Int iQP = iMinQP; iQP <= iMaxQP; iQP++)
	  {
		  const Bool bIsLosslessMode = isAddLowestQP && (iQP == iMinQP);

		  if (bIsLosslessMode)
		  {
			  iQP = lowestQP;
		  }

		  m_ChromaQpAdjIdc = 0;
		  if (pcSlice->getUseChromaQpAdj())
		  {
			  Int lgMinCuSize = pcSlice->getSPS()->getLog2MinCodingBlockSize() +
				  std::max<Int>(0, pcSlice->getSPS()->getLog2DiffMaxMinCodingBlockSize() - Int(pcSlice->getPPS()->getMaxCuChromaQpAdjDepth()));
			  m_ChromaQpAdjIdc = ((uiLPelX >> lgMinCuSize) + (uiTPelY >> lgMinCuSize)) % (pcSlice->getPPS()->getChromaQpAdjTableSize() + 1);
		  }

		  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);

		  // do inter modes, SKIP and 2Nx2N

		  if (rpcBestCU->getSlice()->getSliceType() != I_SLICE)
		  {
			  // 2Nx2N
			  if (m_pcEncCfg->getUseEarlySkipDetection())
			  {
				  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2Nx2N DEBUG_STRING_PASS_INTO(sDebug));
				  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);//by Competition for inter_2Nx2N
			  }
			  // SKIP
			  xCheckRDCostMerge2Nx2N(rpcBestCU, rpcTempCU DEBUG_STRING_PASS_INTO(sDebug), &earlyDetectionSkipMode);//by Merge for inter_2Nx2N
			  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);

			  if (!m_pcEncCfg->getUseEarlySkipDetection())
			  {
				  // 2Nx2N, NxN
				  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2Nx2N DEBUG_STRING_PASS_INTO(sDebug));
				  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
				  if (m_pcEncCfg->getUseCbfFastMode())
				  {
					  doNotBlockPu = rpcBestCU->getQtRootCbf(0) != 0;
				  }
			  }
		  }


		  if (bIsLosslessMode) // Restore loop variable if lossless mode was searched.
		  {
			  iQP = iMinQP;
		  }
	
	  }
  */

	  if (!earlyDetectionSkipMode)
	  {
		  for (Int iQP = iMinQP; iQP <= iMaxQP; iQP++)
		  {
			 
			  const Bool bIsLosslessMode = isAddLowestQP && (iQP == iMinQP); // If lossless, then iQP is irrelevant for subsequent modules.
 /*
			  if (bIsLosslessMode)
			  {
				  iQP = lowestQP;
			  }

			  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);

			  // do inter modes, NxN, 2NxN, and Nx2N
			  if (rpcBestCU->getSlice()->getSliceType() != I_SLICE)
			  {   //rpcBestCU->getSlice()->getSliceType() != I_SLICE
				  // 2Nx2N, NxN

				  // condition is never ture  YS comment 
				  if (!((rpcBestCU->getWidth(0) == 8) && (rpcBestCU->getHeight(0) == 8)))
				  {
					  if (uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth && doNotBlockPu)
					  {
						  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_NxN DEBUG_STRING_PASS_INTO(sDebug));
						  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
					  }
				  }

				  if (doNotBlockPu)
				  {
					  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_Nx2N DEBUG_STRING_PASS_INTO(sDebug));
					  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
					  if (m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_Nx2N)
					  {
						  doNotBlockPu = rpcBestCU->getQtRootCbf(0) != 0;
					  }
				  }
				  if (doNotBlockPu)
				  {
					  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2NxN DEBUG_STRING_PASS_INTO(sDebug));
					  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
					  if (m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_2NxN)
					  {
						  doNotBlockPu = rpcBestCU->getQtRootCbf(0) != 0;
					  }
				  }

				  //! Try AMP (SIZE_2NxnU, SIZE_2NxnD, SIZE_nLx2N, SIZE_nRx2N)
				  if (pcSlice->getSPS()->getUseAMP() && uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth)
				  {
#if AMP_ENC_SPEEDUP
					  Bool bTestAMP_Hor = false, bTestAMP_Ver = false;

#if AMP_MRG
					  Bool bTestMergeAMP_Hor = false, bTestMergeAMP_Ver = false;

					  deriveTestModeAMP(rpcBestCU, eParentPartSize, bTestAMP_Hor, bTestAMP_Ver, bTestMergeAMP_Hor, bTestMergeAMP_Ver);
#else
					  deriveTestModeAMP (rpcBestCU, eParentPartSize, bTestAMP_Hor, bTestAMP_Ver);
#endif

					  //! Do horizontal AMP
					  if (bTestAMP_Hor)
					  {
						  if (doNotBlockPu)
						  {
							  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2NxnU DEBUG_STRING_PASS_INTO(sDebug));
							  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
							  if (m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_2NxnU)
							  {
								  doNotBlockPu = rpcBestCU->getQtRootCbf(0) != 0;
							  }
						  }
						  if (doNotBlockPu)
						  {
							  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2NxnD DEBUG_STRING_PASS_INTO(sDebug));
							  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
							  if (m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_2NxnD)
							  {
								  doNotBlockPu = rpcBestCU->getQtRootCbf(0) != 0;
							  }
						  }
					  }
#if AMP_MRG
					  else if (bTestMergeAMP_Hor)
					  {
						  if (doNotBlockPu)
						  {
							  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2NxnU DEBUG_STRING_PASS_INTO(sDebug), true);
							  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
							  if (m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_2NxnU)
							  {
								  doNotBlockPu = rpcBestCU->getQtRootCbf(0) != 0;
							  }
						  }
						  if (doNotBlockPu)
						  {
							  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_2NxnD DEBUG_STRING_PASS_INTO(sDebug), true);
							  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
							  if (m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_2NxnD)
							  {
								  doNotBlockPu = rpcBestCU->getQtRootCbf(0) != 0;
							  }
						  }
					  }
#endif

					  //! Do horizontal AMP
					  if (bTestAMP_Ver)
					  {
						  if (doNotBlockPu)
						  {
							  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_nLx2N DEBUG_STRING_PASS_INTO(sDebug));
							  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
							  if (m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_nLx2N)
							  {
								  doNotBlockPu = rpcBestCU->getQtRootCbf(0) != 0;
							  }
						  }
						  if (doNotBlockPu)
						  {
							  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_nRx2N DEBUG_STRING_PASS_INTO(sDebug));
							  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
						  }
					  }
#if AMP_MRG
					  else if (bTestMergeAMP_Ver)
					  {
						  if (doNotBlockPu)
						  {
							  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_nLx2N DEBUG_STRING_PASS_INTO(sDebug), true);
							  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
							  if (m_pcEncCfg->getUseCbfFastMode() && rpcBestCU->getPartitionSize(0) == SIZE_nLx2N)
							  {
								  doNotBlockPu = rpcBestCU->getQtRootCbf(0) != 0;
							  }
						  }
						  if (doNotBlockPu)
						  {
							  xCheckRDCostInter(rpcBestCU, rpcTempCU, SIZE_nRx2N DEBUG_STRING_PASS_INTO(sDebug), true);
							  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
						  }
					  }
#endif

#else
					  xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnU );
					  rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
					  xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_2NxnD );
					  rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
					  xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nLx2N );
					  rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );

					  xCheckRDCostInter( rpcBestCU, rpcTempCU, SIZE_nRx2N );
					  rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );

#endif
				  }
			  }   //rpcBestCU->getSlice()->getSliceType() != I_SLICE
  
  */
			  
#endif		
			  // do normal intra modes
			  // speedup for inter frames
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		SS3		  
///////////////////////////////////////////////////////////	Decision Making //////////////////////////////////

#if SVM_DECISION_ENABLE		
			  if ((currentState==Testing || currentState==Verifying) && g_bModelSwitch[Assistant_Skip] == true)   // Assistant 
			  {
				  Bool bSkipNeighb = false;
				  Decision[Assistant_Skip] = Unsure;
				  if (uiDepth < 2 && rpcTempCU->getSkipNeighb(uiDepth))
				  {
					bSkipNeighb = true;
				  }
				  if (bSkipNeighb == true)
				  {
					//  cout << 1;
					Decision[Assistant_Skip] = Skip2Nx2N;
				  }
			  }
////////////////////////////////////////////////////////////////////////////
			  if (g_bDecisionSwitch[uiDepth][modelType_1][TerminateCU] == true)
			  {
				 bEarlyTerminate = (currentState==Testing && Decision[modelType_1] == TerminateCU);
			  }
			  if (g_bDecisionSwitch[uiDepth][modelType_1][Skip2Nx2N] == true)
			  {
				 bSkip2Nx2N = (currentState==Testing && Decision[modelType_1] == Skip2Nx2N);
			  }



		//  Trust assistant 
			  if (currentState==Testing && Decision[Assistant_Skip] == Skip2Nx2N && uiDepth >= SVM_MinDepth && uiDepth <= SVM_MaxDepth && !bBoundary && g_bTrustAssistantSkip)
			  {
				  bSkip2Nx2N = true;  // always trust the decsion of Assistant ?
			  }		

		// resolve conflict decsion
			  if (bSkip2Nx2N == true && bEarlyTerminate == true)   
			  {
				  bSkip2Nx2N = false;
				  bEarlyTerminate = false;
			  }
		// Depth Excaptional condition 
			  if (uiDepth == 3 && N_NonZeroFeature > 0 && g_bDepthException)
			  {
				  bSkip2Nx2N = false;
				  bEarlyTerminate = false;
			  }
#endif  // endif : SVM_DECISION_ENABLE	


#if APPLY_CDM      //  assign value
			  // cout << g_iPOC << endl;		
			  //if (uiDepth ==0)cout << "("<<(int)uiLPelX / BlockSize << ',' << (int)uiTPelY / BlockSize << ")    ";
			  if (!bBoundary && uiDepth <=2)
			  {

				  Int Index_X = (int)uiLPelX / BlockSize;
				  Int Index_Y = (int)uiTPelY / BlockSize;
				  if (uiDepth == 0)
				  {
					//  cout << MatCDM[uiDepth]->at<float>(g_iPOC, Index_Y, Index_X);
				  }
				  
				  if (MatCDM[uiDepth]->at<float>(g_iPOC, Index_Y, Index_X) == +1)
				  {
					 bSkip2Nx2N = true;
					 bEarlyTerminate = false;
				  }
				  else if (MatCDM[uiDepth]->at<float>(g_iPOC, Index_Y, Index_X) == -1)
				  {
					bEarlyTerminate = true;
					bSkip2Nx2N = false;
				  }
				  else
				  {
					  // do nothing 
				  }
			  }
#endif


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   SS4
////////////////////////////////////////////////////////////  prediction & decision made,  Start Checking Mode/////////////////////////////////////
			  Double intraCost = 0.0;
			  if ((rpcBestCU->getSlice()->getSliceType() == I_SLICE) ||
				  (rpcBestCU->getCbf(0, COMPONENT_Y) != 0) ||
				  ((rpcBestCU->getCbf(0, COMPONENT_Cb) != 0) && (numberValidComponents > COMPONENT_Cb)) ||
				  ((rpcBestCU->getCbf(0, COMPONENT_Cr) != 0) && (numberValidComponents > COMPONENT_Cr))) // avoid very complex intra if it is unlikely
			  {
//////////////////////////////////////////// Check 2Nx2N	////////////////////////////////////
					  if (!bSkip2Nx2N)
					  {
#if TIME_SYSTEM
						  clock_t clockStart_xCheckRDCostIntra = clock();
#endif
						  Bool bChange = xCheckRDCostIntra(
#if  SKIP_RDO_ENABLE			  
							  bSkipRDO,
#endif									  						  
							  rpcBestCU, rpcTempCU, intraCost, SIZE_2Nx2N
	  
							  DEBUG_STRING_PASS_INTO(sDebug));
#if TIME_SYSTEM
						  g_dTime_xCheckRDCostIntra_2Nx2N[uiDepth] += (Double)(clock() - clockStart_xCheckRDCostIntra) / CLOCKS_PER_SEC;
#endif

#if MODIFICATION_YS			// gather the data after the RDO
						  if (bSkipRDO){
							  rpcBestCU->getTotalCost() = MAX_DOUBLE;
						  }
						  if (g_d2NRDCost[uiDepth] == -1)
						  {
							  g_d2NRDCost[uiDepth] = rpcBestCU->getTotalCost();
						  }
						  if (g_d2NBits[uiDepth] == -1)
						  {
							  g_d2NBits[uiDepth] = rpcBestCU->getTotalBits();
						  }
						  if (g_d2NDistortion[uiDepth] == -1)
						  {
							  g_d2NDistortion[uiDepth] = rpcBestCU->getTotalDistortion();
						  }
#endif		  
						  b2Nx2NChecked = true;
						  iIntraMode = (Int)	rpcBestCU->getIntraDir(CHANNEL_TYPE_LUMA, 0);
						  d2NRDCost = (Double)	rpcBestCU->getTotalCost();
						  d2NBits = (Double)	rpcBestCU->getTotalBits();
						  d2NDistortion = (Double)	rpcBestCU->getTotalDistortion();
						  J0 = rpcBestCU->getTotalCost();
					  }
					  else
					  {  
						  // 2Nx2N is Skiped 
						  rpcBestCU->getTotalCost() = MAX_DOUBLE;
						  J0 = rpcBestCU->getTotalCost();
					  }
#if OUTPUT_INSIGHTDATA  // 2Nx2N SATD  RDcost IPM 
					  if (currentState==Training&&!bBoundary){
						  //g_InsightDataSet[uiDepth] << rpcBestCU->getSATD() << '\t';
						  //g_InsightDataSet[uiDepth] << rpcBestCU->getTotalCost() << '\t';  // RDCost 2Nx2N
					      g_InsightDataSet[uiDepth] << (Int)rpcBestCU->getIntraDir(CHANNEL_TYPE_LUMA,0)<< '\t';  // RDCost 2Nx2N
					  }
#endif
///////////////////////////////////////////////////// form feature vectors and do predicitons 
					  /// MODEL 2 3
					  if (!bBoundary){
						  switch (currentState)
						  {
						  case Training:
								  if (g_bModelSwitch[modelType_2])FormFeatureVector(rpcBestCU, uiDepth, feature_2, featureLength_2, featureType_2);
								  if (g_bModelSwitch[modelType_3])FormFeatureVector(rpcBestCU, uiDepth, feature_3, featureLength_3, featureType_3);
							  break;
						  case Verifying:
								  if (g_bModelSwitch[modelType_2]){
									  FormFeatureVector(rpcBestCU, uiDepth, feature_2, featureLength_2, featureType_2);
									  DoPrediction(uiDepth, feature_2, featureLength_2, label_predict_2, modelType_2, featureType_2);
									  Decision[modelType_2] = label_predict_2 > 0 ? SkipRDO : TerminateCU;
								  }

								  if (g_bModelSwitch[modelType_3]){
									  FormFeatureVector(rpcBestCU, uiDepth, feature_3, featureLength_3, featureType_3);
									  DoPrediction(uiDepth, feature_3, featureLength_3, label_predict_3, modelType_3, featureType_3);
									  Decision[modelType_3] = label_predict_3 > 0 ? SkipRDO : TerminateCU;
								  }
							  break;
						  case Testing:
							  if (!bSkipRDO && !bSkip2Nx2N && g_bDecisionSwitch[uiDepth][modelType_3][TerminateCU] == true){
								  FormFeatureVector(rpcBestCU, uiDepth, feature_3, featureLength_3, featureType_3);
								  DoPrediction(uiDepth, feature_3, featureLength_3, label_predict_3, modelType_3, featureType_3);
								  if (label_predict_3 < 0){
									  Decision[modelType_3] = TerminateCU;
									  bEarlyTerminate = true;
								  }
								  else{
									  Decision[modelType_3] = SkipRDO;
									  bEarlyTerminate = false;
								  }
							  }
							  if (g_bDecisionSwitch[uiDepth][modelType_2][SkipRDO] == true)Decision[modelType_2] = bSkipRDO ? SkipRDO : TerminateCU;
							  break;
						  default:
							  cout << "Error: Unknown currentState!";
							  break;
						  }
					  }

					  if (uiDepth == 3 && N_NonZeroFeature > 0 && g_bDepthException)
					  {
						  bEarlyTerminate = false;
					  }

//////////decision area for early termination 


//////////////////////////////////////////// Check NxN	//////////////////////////////////////

				  if (uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth)  // Only when Depth equal MaxDepth 
				  {
					  if (!bEarlyTerminate)
					  {
						  if (rpcTempCU->getWidth(0) > (1 << rpcTempCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize()))
						  {
				           rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
							  Double tmpIntraCost;  //YS add

							  //  CheckBestMode will exhange the data , rpcBestCU always hold the smaller one
#if FORCE_TERMINATION
							  Int FORCE_DEPTH = FORCE_TERMINATION_DEPTH;
							  if (uiDepth >= FORCE_DEPTH)
							  {
								  // do nothing
							  }
							  else
							  {
								  bPartition_True = xCheckRDCostIntra(
#if  SKIP_RDO_ENABLE			  
									  bSkipRDO, //SY added
#endif										  							  
									  rpcBestCU, rpcTempCU, tmpIntraCost, SIZE_NxN			  									 								 
									 DEBUG_STRING_PASS_INTO(sDebug));  //  CheckBestMode will exhange the data  
							  }
#else

#if TIME_SYSTEM
							  clock_t clockStart_xCheckRDCostIntra = clock();
#endif
							 bPartition_True = xCheckRDCostIntra(
#if  SKIP_RDO_ENABLE			  
								 bSkipRDO, //SY added
#endif			 								 			 
								 rpcBestCU, rpcTempCU, tmpIntraCost, SIZE_NxN
								 DEBUG_STRING_PASS_INTO(sDebug));  //  CheckBestMode will exhange the data  
#endif		

#if TIME_SYSTEM
							 g_dTime_xCheckRDCostIntra_NxN += (Double)(clock() - clockStart_xCheckRDCostIntra) / CLOCKS_PER_SEC;
#endif
							 Int BestSATD_NxN;
							 // Set the J1 for MaxDepth
							  if (bPartition_True){
								  J1 = rpcBestCU->getTotalCost();
								  BestSATD_NxN = rpcBestCU->getSATD();
							  }
							  else
							  {
								  J1 = rpcTempCU->getTotalCost();
								  BestSATD_NxN = rpcTempCU->getSATD();
							  }
#if OUTPUT_INSIGHTDATA  // NxN  SATD RDCost
							  if (currentState==Training&&!bBoundary){
								  //g_InsightDataSet[3] << BestSATD_NxN << '\t';
								 // g_InsightDataSet[3] << J1 << '\t';  // RDCost NxN

							  }
#endif

							  intraCost = std::min(intraCost, tmpIntraCost);
							  //rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
							  bNxNChecked = true;
						  }
					  }
				  }  // if end if (uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth) NxN
			  } // if end
////////////////////////////////////////////////////////////////////////  Mode Checked go SubCUs  //////////////////////////////////////////

/*
#if  TOGGLE_NONUSE
			  //test PCM
			  if (pcPic->getSlice(0)->getSPS()->getUsePCM()
				  && rpcTempCU->getWidth(0) <= (1 << pcPic->getSlice(0)->getSPS()->getPCMLog2MaxSize())
				  && rpcTempCU->getWidth(0) >= (1 << pcPic->getSlice(0)->getSPS()->getPCMLog2MinSize()))
			  {
				  UInt uiRawBits = getTotalBits(rpcBestCU->getWidth(0), rpcBestCU->getHeight(0), rpcBestCU->getPic()->getChromaFormat(), g_bitDepth);
				  UInt uiBestBits = rpcBestCU->getTotalBits();
				  if ((uiBestBits > uiRawBits) || (rpcBestCU->getTotalCost() > m_pcRdCost->calcRdCost(uiRawBits, 0)))
				  {
					  xCheckIntraPCM(rpcBestCU, rpcTempCU);
					  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
				  }
			  }	
			  if (bIsLosslessMode) // Restore loop variable if lossless mode was searched.
			  {
				  iQP = iMinQP;
			  }
#endif
*/
		  }
	  } //  end  if (!earlyDetectionSkipMode)
	  
//////////////////////////////////////////////////////////////////////////////////////// finalize the RDcost of BestCU
	  if (rpcBestCU->getTotalCost() != MAX_DOUBLE) // YS add this if statement 
	  {

	  m_pcEntropyCoder->resetBits();
	  m_pcEntropyCoder->encodeSplitFlag(rpcBestCU, 0, uiDepth, true);  
	  rpcBestCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // split bits
	  rpcBestCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
	  rpcBestCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcBestCU->getTotalBits(), rpcBestCU->getTotalDistortion());

	  }

	  if (uiDepth < (g_uiMaxCUDepth - g_uiAddCUDepth)){ // for Depth 0 1 2
		  J0 = rpcBestCU->getTotalCost();       // the bits here may be different with bits before 
	  }
#if OUTPUT_INSIGHTDATA   // depth==3 JBest Label
		  if (uiDepth==3  && currentState==Training&&!bBoundary){
			  //g_InsightDataSet[3] << rpcBestCU->getTotalCost() << '\t';  // RDCost 2Nx2N
			  g_InsightDataSet[3] << bPartition_True << '\n';
		  }
#endif
#if NEW_FEATURESYSTEM
		  if (uiDepth == 3 && currentState==Training && !bBoundary){
			  g_TrainingDataSet[3] << bPartition_True<< '\t';
		  }
#endif
#if  TOGGLE_NONUSE
	  // Early CU determination
	  if (m_pcEncCfg->getUseEarlyCU() && rpcBestCU->isSkipped(0))
	  {
		  bSubBranch = false;
	  }
	  else
	  {
		  bSubBranch = true;
	  }
#endif

	  if (bEarlyTerminate)
	  {
		  bSubBranch = false;
	  }


#if FORCE_TERMINATION
  Int FORCE_DEPTH = FORCE_TERMINATION_DEPTH;
  if (uiDepth >= FORCE_DEPTH)
  {
	  bSubBranch = false;
  }
#endif 

  }   // endif: not bBoundry
  else
  {
    bBoundary = true;
  }


/*
#if  TOGGLE_NONUSE
  // copy orginal YUV samples to PCM buffer
  if( rpcBestCU->isLosslessCoded(0) && (rpcBestCU->getIPCMFlag(0) == false))
  {
    xFillPCMBuffer(rpcBestCU, m_ppcOrigYuv[uiDepth]);
  }

  if( (g_uiMaxCUWidth>>uiDepth) == (g_uiMaxCUWidth >> ( rpcTempCU->getSlice()->getPPS()->getMaxCuDQPDepth())) )
  {
    Int idQP = m_pcEncCfg->getMaxDeltaQP();
    iMinQP = Clip3( -rpcTempCU->getSlice()->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, iBaseQP-idQP );
    iMaxQP = Clip3( -rpcTempCU->getSlice()->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, iBaseQP+idQP );
  }
  else if( (g_uiMaxCUWidth>>uiDepth) > (g_uiMaxCUWidth >> ( rpcTempCU->getSlice()->getPPS()->getMaxCuDQPDepth())) )
  {
    iMinQP = iBaseQP;
    iMaxQP = iBaseQP;
  }
  else
  {
    const Int iStartQP = rpcTempCU->getQP(0);
    iMinQP = iStartQP;
    iMaxQP = iStartQP;
  }

  if ( m_pcEncCfg->getUseRateCtrl() )
  {
    iMinQP = m_pcRateCtrl->getRCQP();
    iMaxQP = m_pcRateCtrl->getRCQP();
  }

  if ( m_pcEncCfg->getCUTransquantBypassFlagForceValue() )
  {
    iMaxQP = iMinQP; // If all TUs are forced into using transquant bypass, do not loop here.
  }
#endif
*/

#if TIME_SYSTEM // End of xcompressCU 
  g_dTime_xCompressCU[uiDepth] += (Double)(clock() - clockStart_xCompressCU) / CLOCKS_PER_SEC;
#endif

 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //		SS6		
 //////////////////////////////////////////  Go SubCUs  /////////////////////////////////////////////////////////////////
	// recursively call the compressCU
  if (bSubBranch && uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth){
	  for (Int iQP = iMinQP; iQP <= iMaxQP; iQP++)
	  {
		  const Bool bIsLosslessMode = false; // False at this level. Next level down may set it to true.
		  rpcTempCU->initEstData(uiDepth, iQP, bIsLosslessMode);
		  if (bSubBranch && uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth)
		  {
			  UChar       uhNextDepth = uiDepth + 1;
			  TComDataCU* pcSubBestPartCU = m_ppcBestCU[uhNextDepth];

			  TComDataCU* pcSubTempPartCU = m_ppcTempCU[uhNextDepth];
			  DEBUG_STRING_NEW(sTempDebug)
				  // loop four subCUs 
			  for (UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++)
			  {
				  pcSubBestPartCU->initSubCU(rpcTempCU, uiPartUnitIdx, uhNextDepth, iQP);           // clear sub partition datas or init.
				  pcSubTempPartCU->initSubCU(rpcTempCU, uiPartUnitIdx, uhNextDepth, iQP);           // clear sub partition datas or init.
				  if ((pcSubBestPartCU->getCUPelX() < pcSlice->getSPS()->getPicWidthInLumaSamples()) && (pcSubBestPartCU->getCUPelY() < pcSlice->getSPS()->getPicHeightInLumaSamples()))
				  {
					  if (0 == uiPartUnitIdx) //initialize RD with previous depth buffer
					  {
						  m_pppcRDSbacCoder[uhNextDepth][CI_CURR_BEST]->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
					  }
					  else
					  {
						  m_pppcRDSbacCoder[uhNextDepth][CI_CURR_BEST]->load(m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]);
					  }
#if MODIFICATION_YS
					  pcSubBestPartCU->setIntraMode_UpperCU(iIntraMode);
					  pcSubTempPartCU->setIntraMode_UpperCU(iIntraMode);
#endif
#if AMP_ENC_SPEEDUP
					  DEBUG_STRING_NEW(sChild)
					  if (!rpcBestCU->isInter(0))
					  {
						  xCompressCU(pcSubBestPartCU, pcSubTempPartCU, uhNextDepth DEBUG_STRING_PASS_INTO(sChild), NUMBER_OF_PART_SIZES);
					  }
					  else
					  {
						  cout << "fuck xCompressCU( pcSubBestPartCU, pcSubTempPartCU, uhNextDepth DEBUG_STRING_PASS_INTO(sChild), rpcBestCU->getPartitionSize(0) )  ";
						  xCompressCU(pcSubBestPartCU, pcSubTempPartCU, uhNextDepth DEBUG_STRING_PASS_INTO(sChild), rpcBestCU->getPartitionSize(0));
					  }
					  DEBUG_STRING_APPEND(sTempDebug, sChild)
#else
					  xCompressCU( pcSubBestPartCU, pcSubTempPartCU, uhNextDepth );  // recursively compress CU in next depth
#endif

					  rpcTempCU->copyPartFrom(pcSubBestPartCU, uiPartUnitIdx, uhNextDepth);         // Keep best part data to current temporary data.
					  xCopyYuv2Tmp(pcSubBestPartCU->getTotalNumPart()*uiPartUnitIdx, uhNextDepth);
				  }  // if end 
				  else
				  {
					  // statement true,at the end row of image
					  pcSubBestPartCU->copyToPic(uhNextDepth);
					  rpcTempCU->copyPartFrom(pcSubBestPartCU, uiPartUnitIdx, uhNextDepth);
				  }
			  }  // for loop end :loop four subCUs

			  if (!bBoundary)
			  {
				  m_pcEntropyCoder->resetBits();
				  m_pcEntropyCoder->encodeSplitFlag(rpcTempCU, 0, uiDepth, true);

				  rpcTempCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // split bits
				  rpcTempCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
			  }
			  // calculate the current RD cost of rpcTempCU
			  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion());

			  if ((g_uiMaxCUWidth >> uiDepth) == (g_uiMaxCUWidth >> (rpcTempCU->getSlice()->getPPS()->getMaxCuDQPDepth())) && rpcTempCU->getSlice()->getPPS()->getUseDQP())
			  {
				  Bool hasResidual = false;
				  for (UInt uiBlkIdx = 0; uiBlkIdx < rpcTempCU->getTotalNumPart(); uiBlkIdx++)
				  {
					  if ((rpcTempCU->getCbf(uiBlkIdx, COMPONENT_Y)
						  || (rpcTempCU->getCbf(uiBlkIdx, COMPONENT_Cb) && (numberValidComponents > COMPONENT_Cb))
						  || (rpcTempCU->getCbf(uiBlkIdx, COMPONENT_Cr) && (numberValidComponents > COMPONENT_Cr))))
					  {
						  hasResidual = true;
						  break;
					  }
				  }

				  UInt uiTargetPartIdx = 0;
				  if (hasResidual)
				  {
#if !RDO_WITHOUT_DQP_BITS
					  m_pcEntropyCoder->resetBits();
					  m_pcEntropyCoder->encodeQP(rpcTempCU, uiTargetPartIdx, false);
					  rpcTempCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
					  rpcTempCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
					  // calculate the current RD cost of rpcTempCU
					  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion());
#endif
					  Bool foundNonZeroCbf = false;
					  rpcTempCU->setQPSubCUs(rpcTempCU->getRefQP(uiTargetPartIdx), 0, uiDepth, foundNonZeroCbf);
					  assert(foundNonZeroCbf);
				  }
				  else
				  {
					  rpcTempCU->setQPSubParts(rpcTempCU->getRefQP(uiTargetPartIdx), 0, uiDepth); // set QP to default QP
				  }
			  }

			  m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);

			  // TODO: this does not account for the slice bytes already written. See other instances of FIXED_NUMBER_OF_BYTES
			  Bool isEndOfSlice = rpcBestCU->getSlice()->getSliceMode() == FIXED_NUMBER_OF_BYTES
				  && (rpcBestCU->getTotalBits() > rpcBestCU->getSlice()->getSliceArgument() << 3);
			  Bool isEndOfSliceSegment = rpcBestCU->getSlice()->getSliceSegmentMode() == FIXED_NUMBER_OF_BYTES
				  && (rpcBestCU->getTotalBits() > rpcBestCU->getSlice()->getSliceSegmentArgument() << 3);
			  if (isEndOfSlice || isEndOfSliceSegment)
			  {
				  // the condition is never true  YS comment 
				  if (m_pcEncCfg->getCostMode() == COST_MIXED_LOSSLESS_LOSSY_CODING)
				  {
					  rpcBestCU->getTotalCost() = rpcTempCU->getTotalCost() + (1.0 / m_pcRdCost->getLambda());
				  }
				  else
				  {
					  rpcBestCU->getTotalCost() = rpcTempCU->getTotalCost() + 1;
				  }
			  }

			  if (bSkip2Nx2N || bSkipRDO){
				  // if all use the prediction set the current RD cost to MAX_DOUBLE and go to next depth directly 
				  rpcBestCU->getTotalCost() = MAX_DOUBLE;  // this is necessary
			  }
			  J0 = rpcBestCU->getTotalCost();
			  J1 = rpcTempCU->getTotalCost();

			  Bool	tempbool = rpcBestCU->getTotalCost() > rpcTempCU->getTotalCost();
			  Double BestCost = rpcBestCU->getTotalCost();

			  bPartition_True = xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth DEBUG_STRING_PASS_INTO(sDebug) DEBUG_STRING_PASS_INTO(sTempDebug) DEBUG_STRING_PASS_INTO(false)); // RD compare current larger prediction
			  bPartition_Do = bPartition_True;
#if OUTPUT_INSIGHTDATA  // J0 J1 JBest Label 
			  if (currentState==Training&&!bBoundary){
				  //g_InsightDataSet[uiDepth] << J0 << '\t';
				  //g_InsightDataSet[uiDepth] << J1 << '\t';
				  //g_InsightDataSet[uiDepth] << rpcBestCU->getTotalCost() << '\t';
				  //g_InsightDataSet[uiDepth] << bPartition_True << '\t';
			  }
#endif

#if NEW_FEATURESYSTEM 
			  if (currentState==Training&&!bBoundary){
				  g_TrainingDataSet[uiDepth] << bPartition_True << '\t';
			  }
#endif
			  if ((J0 > J1) != bPartition_True && !bBoundary){
				  cout << "error !  ";
				  cout << uiLPelX << ' ' << uiTPelY << ' ' << uiDepth << endl;
				  cout << b2Nx2NChecked << ' ' << J0 << ' ' << J1 << ' ' << BestCost << ' ' << bPartition_True << endl;
			  }
		  } // endif : (bSubBranch && uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth )
	  }  // for loop end : (Int iQP=iMinQP; iQP<=iMaxQP; iQP++) end
  }



 //////////////////////////////////////////////  SubCUs finished  ///////////////////////////////////////////////////////////////////////////////

  



 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //		SS6		
 //////////////////////////////////////////  Updating  Verification Data /////////////////////////////////////////////////////////////////
	  if (!bBoundary && currentState==Verifying)
	  {
		for (int i = 0; i < NUM_MODELTYPE; i++){
			if (g_bModelSwitch[i] == true){
				countTFPN(Decision[(ModelType)i], bPartition_True, g_iVerResult, uiDepth, (ModelType)i);
				countRDLoss(Decision[(ModelType)i], bPartition_True, J0, J1, g_iVerResult, uiDepth, (ModelType)i);
			}
		}
	  }
 //////////////////////////////////////////   Updating  Training Data ///////////////////////////////////////////////////////////////////////////
#if OUTPUT_CDM      //  assign value
	  if (!bBoundary )
	  {
		  Int Index_X = (int)uiLPelX / BlockSize;
		  Int Index_Y = (int)uiTPelY / BlockSize;

		  if (bPartition_True == true)
		  {

			  MatCDM[uiDepth]->at<float>(g_iPOC, Index_Y, Index_X) = +1.0;
		  }
		  else if (bPartition_True == false )
		  {
			  MatCDM[uiDepth]->at<float>(g_iPOC, Index_Y, Index_X)  = -1.0;
		  }
		  else
		  {
			  MatCDM[uiDepth]->at<float>(g_iPOC, Index_Y, Index_X) = -100;
		  }
	  }
#endif




#if SVM_TRAININGSET_SAVE  // Output training set to file 
	 if (currentState==Training && !bBoundary){
		// for (int i = 0; i < NUM_MODELTYPE; i++){
			 if (true){
				 ofstream* TrainingSet_1;
				 TrainingSet_1 = &g_TrainingSet[uiDepth][getFeatureFormat(modelType_1)][featureType_1];

				 ofstream* TrainingSet_2;
				 TrainingSet_2 = &g_TrainingSet[uiDepth][getFeatureFormat(modelType_2)][featureType_2];

				 ofstream* TrainingSet_3;
				 TrainingSet_3 = &g_TrainingSet[uiDepth][getFeatureFormat(modelType_3)][featureType_3];

					 Double label = 0;
					 if (bPartition_True == true)
					 {
						 label = +1;
					 }
					 else
					 {
						 label = -1;
					 }
					 if (g_iFeatureLength[uiDepth][featureType_1] == 0)
					 {
						 g_iFeatureLength[uiDepth][featureType_1] = featureLength_1;
					 }
					 if (g_bModelSwitch[modelType_1])OutputFeatureVector(TrainingSet_1, feature_1, featureLength_1, label, uiDepth, getFeatureFormat(modelType_1));
					 if (g_bModelSwitch[modelType_2])OutputFeatureVector(TrainingSet_2, feature_2, featureLength_2, label, uiDepth, getFeatureFormat(modelType_2));
					 if (g_bModelSwitch[modelType_3])OutputFeatureVector(TrainingSet_3, feature_3, featureLength_3, label, uiDepth, getFeatureFormat(modelType_3));
					 g_iSampleNumber[uiDepth][featureType_1]++;

					 if (g_bModelSwitch[BayesNan] == true){
						 g_cBayesModel_Nan.updateModel(uiDepth, J0, J1, N_OBF);
					 }
			 }


#if OUTPUT_INSIGHTDATA  // JX Feature
#if GET_MV_FEATURE
			 TMVFeature* feature_x = getTMVFeature(rpcBestCU);

			 for (int iFiltIdx = 0; iFiltIdx < 5; iFiltIdx++){
				 g_InsightDataSet[uiDepth] << endl;
				 for (int j = 0; j < 26;j++){
					 g_InsightDataSet[uiDepth] << feature_x->m_adFeature[iFiltIdx][j] << '\t';
				 }
			 }
			 g_InsightDataSet[uiDepth] << endl;
			 delete feature_x;
#endif

#endif

#if OUTPUT_INSIGHTDATA  // OBF
			 for (int i = 0; i < featureLength_1; i++){
				// g_InsightDataSet[uiDepth] << feature_1[i] << '\t';
			 }
				 //g_InsightDataSet[uiDepth] << endl;
#endif



#if NEW_FEATURESYSTEM 
				 for (int i = 0; i < featureLength_1; i++){
					 g_TrainingDataSet[uiDepth] << feature_1[i] << '\t';
				 }
				 g_TrainingDataSet[uiDepth] << endl;
#endif

	}
#endif



#if SVM_YS    // free block_ExtendOBF
	 if (feature_1!= NULL)
	delete[] feature_1;
	 if (feature_2!= NULL)
	delete[] feature_2;
	 if (feature_3 != NULL)
	delete[] feature_3;
#endif

  DEBUG_STRING_APPEND(sDebug_, sDebug);
  rpcBestCU->copyToPic(uiDepth);                                                     // Copy Best data to Picture for next partition prediction.
  xCopyYuv2Pic( rpcBestCU->getPic(), rpcBestCU->getCtuRsAddr(), rpcBestCU->getZorderIdxInCtu(), uiDepth, uiDepth, rpcBestCU, uiLPelX, uiTPelY );   // Copy Yuv data to picture Yuv
  if (bBoundary)
  {
    return;
  }
  // Assert if Best prediction mode is NONE
  // Selected mode's RD-cost must be not MAX_DOUBLE.
  assert( rpcBestCU->getPartitionSize ( 0 ) != NUMBER_OF_PART_SIZES       );
  assert( rpcBestCU->getPredictionMode( 0 ) != NUMBER_OF_PREDICTION_MODES );
  assert( rpcBestCU->getTotalCost     (   ) != MAX_DOUBLE                 );
}

/** finish encoding a cu and handle end-of-slice conditions
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiDepth
 * \returns Void
 */
Void TEncCu::finishCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();
  TComSlice * pcSlice = pcCU->getPic()->getSlice(pcCU->getPic()->getCurrSliceIdx());
  //Calculate end address
  const Int  currentCTUTsAddr = pcPic->getPicSym()->getCtuRsToTsAddrMap(pcCU->getCtuRsAddr());
  const Bool isLastSubCUOfCtu = pcCU->isLastSubCUOfCtu(uiAbsPartIdx);
  if ( isLastSubCUOfCtu )
  {
    // The 1-terminating bit is added to all streams, so don't add it here when it's 1.
    // i.e. when the slice segment CurEnd CTU address is the current CTU address + 1.
    if (pcSlice->getSliceSegmentCurEndCtuTsAddr() != currentCTUTsAddr+1)
    {
      m_pcEntropyCoder->encodeTerminatingBit( 0 );
    }
  }
}

/** Compute QP for each CU
 * \param pcCU Target CU
 * \param uiDepth CU depth
 * \returns quantization parameter
 */
Int TEncCu::xComputeQP( TComDataCU* pcCU, UInt uiDepth )
{
  Int iBaseQp = pcCU->getSlice()->getSliceQp();
  g_iQP = iBaseQp;
  Int iQpOffset = 0;
  if ( m_pcEncCfg->getUseAdaptiveQP() )
  {
    TEncPic* pcEPic = dynamic_cast<TEncPic*>( pcCU->getPic() );
    UInt uiAQDepth = min( uiDepth, pcEPic->getMaxAQDepth()-1 );
    TEncPicQPAdaptationLayer* pcAQLayer = pcEPic->getAQLayer( uiAQDepth );
    UInt uiAQUPosX = pcCU->getCUPelX() / pcAQLayer->getAQPartWidth();
    UInt uiAQUPosY = pcCU->getCUPelY() / pcAQLayer->getAQPartHeight();
    UInt uiAQUStride = pcAQLayer->getAQPartStride();
    TEncQPAdaptationUnit* acAQU = pcAQLayer->getQPAdaptationUnit();

    Double dMaxQScale = pow(2.0, m_pcEncCfg->getQPAdaptationRange()/6.0);
    Double dAvgAct = pcAQLayer->getAvgActivity();
    Double dCUAct = acAQU[uiAQUPosY * uiAQUStride + uiAQUPosX].getActivity();
    Double dNormAct = (dMaxQScale*dCUAct + dAvgAct) / (dCUAct + dMaxQScale*dAvgAct);
    Double dQpOffset = log(dNormAct) / log(2.0) * 6.0;
    iQpOffset = Int(floor( dQpOffset + 0.49999 ));
  }

  return Clip3(-pcCU->getSlice()->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA), MAX_QP, iBaseQp+iQpOffset );
}

/** encode a CU block recursively
 * \param pcCU
 * \param uiAbsPartIdx
 * \param uiDepth
 * \returns Void
 */
Void TEncCu::xEncodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();

  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;

  TComSlice * pcSlice = pcCU->getPic()->getSlice(pcCU->getPic()->getCurrSliceIdx());
  if( ( uiRPelX < pcSlice->getSPS()->getPicWidthInLumaSamples() ) && ( uiBPelY < pcSlice->getSPS()->getPicHeightInLumaSamples() ) )
  {
    m_pcEntropyCoder->encodeSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
  }
  else
  {
    bBoundary = true;
  }

  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( pcPic->getNumPartitionsInCtu() >> (uiDepth<<1) )>>2;
    if( (g_uiMaxCUWidth>>uiDepth) == (g_uiMaxCUWidth >> ( pcCU->getSlice()->getPPS()->getMaxCuDQPDepth())) && pcCU->getSlice()->getPPS()->getUseDQP())
    {
      setdQPFlag(true);
    }

    if( (g_uiMaxCUWidth>>uiDepth) == (g_uiMaxCUWidth >> ( pcCU->getSlice()->getPPS()->getMaxCuChromaQpAdjDepth())) && pcCU->getSlice()->getUseChromaQpAdj())
    {
      setCodeChromaQpAdjFlag(true);
    }

    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
      if( ( uiLPelX < pcSlice->getSPS()->getPicWidthInLumaSamples() ) && ( uiTPelY < pcSlice->getSPS()->getPicHeightInLumaSamples() ) )
      {
        xEncodeCU( pcCU, uiAbsPartIdx, uiDepth+1 );
      }
    }
    return;
  }

  if( (g_uiMaxCUWidth>>uiDepth) >= (g_uiMaxCUWidth >> ( pcCU->getSlice()->getPPS()->getMaxCuDQPDepth())) && pcCU->getSlice()->getPPS()->getUseDQP())
  {
    setdQPFlag(true);
  }

  if( (g_uiMaxCUWidth>>uiDepth) >= (g_uiMaxCUWidth >> ( pcCU->getSlice()->getPPS()->getMaxCuChromaQpAdjDepth())) && pcCU->getSlice()->getUseChromaQpAdj())
  {
    setCodeChromaQpAdjFlag(true);
  }

  if (pcCU->getSlice()->getPPS()->getTransquantBypassEnableFlag())
  {
    m_pcEntropyCoder->encodeCUTransquantBypassFlag( pcCU, uiAbsPartIdx );
  }

  if( !pcCU->getSlice()->isIntra() )
  {
    m_pcEntropyCoder->encodeSkipFlag( pcCU, uiAbsPartIdx );
  }

  if( pcCU->isSkipped( uiAbsPartIdx ) )
  {
    m_pcEntropyCoder->encodeMergeIndex( pcCU, uiAbsPartIdx );
    finishCU(pcCU,uiAbsPartIdx,uiDepth);
    return;
  }

  m_pcEntropyCoder->encodePredMode( pcCU, uiAbsPartIdx );
  m_pcEntropyCoder->encodePartSize( pcCU, uiAbsPartIdx, uiDepth );

  if (pcCU->isIntra( uiAbsPartIdx ) && pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2Nx2N )
  {
    m_pcEntropyCoder->encodeIPCMInfo( pcCU, uiAbsPartIdx );

    if(pcCU->getIPCMFlag(uiAbsPartIdx))
    {
      // Encode slice finish
      finishCU(pcCU,uiAbsPartIdx,uiDepth);
      return;
    }
  }

  // prediction Info ( Intra : direction mode, Inter : Mv, reference idx )
  m_pcEntropyCoder->encodePredInfo( pcCU, uiAbsPartIdx );

  // Encode Coefficients
  Bool bCodeDQP = getdQPFlag();
  Bool codeChromaQpAdj = getCodeChromaQpAdjFlag();
  m_pcEntropyCoder->encodeCoeff( pcCU, uiAbsPartIdx, uiDepth, bCodeDQP, codeChromaQpAdj );
  setCodeChromaQpAdjFlag( codeChromaQpAdj );
  setdQPFlag( bCodeDQP );

  // --- write terminating bit ---
  finishCU(pcCU,uiAbsPartIdx,uiDepth);
}

Int xCalcHADs8x8_ISlice(Pel *piOrg, Int iStrideOrg)
{
  Int k, i, j, jj;
  Int diff[64], m1[8][8], m2[8][8], m3[8][8], iSumHad = 0;

  for( k = 0; k < 64; k += 8 )
  {
    diff[k+0] = piOrg[0] ;
    diff[k+1] = piOrg[1] ;
    diff[k+2] = piOrg[2] ;
    diff[k+3] = piOrg[3] ;
    diff[k+4] = piOrg[4] ;
    diff[k+5] = piOrg[5] ;
    diff[k+6] = piOrg[6] ;
    diff[k+7] = piOrg[7] ;

    piOrg += iStrideOrg;
  }

  //horizontal
  for (j=0; j < 8; j++)
  {
    jj = j << 3;
    m2[j][0] = diff[jj  ] + diff[jj+4];
    m2[j][1] = diff[jj+1] + diff[jj+5];
    m2[j][2] = diff[jj+2] + diff[jj+6];
    m2[j][3] = diff[jj+3] + diff[jj+7];
    m2[j][4] = diff[jj  ] - diff[jj+4];
    m2[j][5] = diff[jj+1] - diff[jj+5];
    m2[j][6] = diff[jj+2] - diff[jj+6];
    m2[j][7] = diff[jj+3] - diff[jj+7];

    m1[j][0] = m2[j][0] + m2[j][2];
    m1[j][1] = m2[j][1] + m2[j][3];
    m1[j][2] = m2[j][0] - m2[j][2];
    m1[j][3] = m2[j][1] - m2[j][3];
    m1[j][4] = m2[j][4] + m2[j][6];
    m1[j][5] = m2[j][5] + m2[j][7];
    m1[j][6] = m2[j][4] - m2[j][6];
    m1[j][7] = m2[j][5] - m2[j][7];

    m2[j][0] = m1[j][0] + m1[j][1];
    m2[j][1] = m1[j][0] - m1[j][1];
    m2[j][2] = m1[j][2] + m1[j][3];
    m2[j][3] = m1[j][2] - m1[j][3];
    m2[j][4] = m1[j][4] + m1[j][5];
    m2[j][5] = m1[j][4] - m1[j][5];
    m2[j][6] = m1[j][6] + m1[j][7];
    m2[j][7] = m1[j][6] - m1[j][7];
  }

  //vertical
  for (i=0; i < 8; i++)
  {
    m3[0][i] = m2[0][i] + m2[4][i];
    m3[1][i] = m2[1][i] + m2[5][i];
    m3[2][i] = m2[2][i] + m2[6][i];
    m3[3][i] = m2[3][i] + m2[7][i];
    m3[4][i] = m2[0][i] - m2[4][i];
    m3[5][i] = m2[1][i] - m2[5][i];
    m3[6][i] = m2[2][i] - m2[6][i];
    m3[7][i] = m2[3][i] - m2[7][i];

    m1[0][i] = m3[0][i] + m3[2][i];
    m1[1][i] = m3[1][i] + m3[3][i];
    m1[2][i] = m3[0][i] - m3[2][i];
    m1[3][i] = m3[1][i] - m3[3][i];
    m1[4][i] = m3[4][i] + m3[6][i];
    m1[5][i] = m3[5][i] + m3[7][i];
    m1[6][i] = m3[4][i] - m3[6][i];
    m1[7][i] = m3[5][i] - m3[7][i];

    m2[0][i] = m1[0][i] + m1[1][i];
    m2[1][i] = m1[0][i] - m1[1][i];
    m2[2][i] = m1[2][i] + m1[3][i];
    m2[3][i] = m1[2][i] - m1[3][i];
    m2[4][i] = m1[4][i] + m1[5][i];
    m2[5][i] = m1[4][i] - m1[5][i];
    m2[6][i] = m1[6][i] + m1[7][i];
    m2[7][i] = m1[6][i] - m1[7][i];
  }

  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 8; j++)
    {
      iSumHad += abs(m2[i][j]);
    }
  }
  iSumHad -= abs(m2[0][0]);
  iSumHad =(iSumHad+2)>>2;
  return(iSumHad);
}

Int  TEncCu::updateCtuDataISlice(TComDataCU* pCtu, Int width, Int height)
{
  Int  xBl, yBl;
  const Int iBlkSize = 8;

  Pel* pOrgInit   = pCtu->getPic()->getPicYuvOrg()->getAddr(COMPONENT_Y, pCtu->getCtuRsAddr(), 0);
  Int  iStrideOrig = pCtu->getPic()->getPicYuvOrg()->getStride(COMPONENT_Y);
  Pel  *pOrg;

  Int iSumHad = 0;
  for ( yBl=0; (yBl+iBlkSize)<=height; yBl+= iBlkSize)
  {
    for ( xBl=0; (xBl+iBlkSize)<=width; xBl+= iBlkSize)
    {
      pOrg = pOrgInit + iStrideOrig*yBl + xBl;
      iSumHad += xCalcHADs8x8_ISlice(pOrg, iStrideOrig);
    }
  }
  return(iSumHad);
}

/** check RD costs for a CU block encoded with merge
 * \param rpcBestCU
 * \param rpcTempCU
 * \param earlyDetectionSkipMode
 */
Void TEncCu::xCheckRDCostMerge2Nx2N( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU DEBUG_STRING_FN_DECLARE(sDebug), Bool *earlyDetectionSkipMode )
{
  assert( rpcTempCU->getSlice()->getSliceType() != I_SLICE );
  TComMvField  cMvFieldNeighbours[2 * MRG_MAX_NUM_CANDS]; // double length for mv of both lists
  UChar uhInterDirNeighbours[MRG_MAX_NUM_CANDS];
  Int numValidMergeCand = 0;
  const Bool bTransquantBypassFlag = rpcTempCU->getCUTransquantBypass(0);

  for( UInt ui = 0; ui < rpcTempCU->getSlice()->getMaxNumMergeCand(); ++ui )
  {
    uhInterDirNeighbours[ui] = 0;
  }
  UChar uhDepth = rpcTempCU->getDepth( 0 );
  rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N, 0, uhDepth ); // interprets depth relative to CTU level
  rpcTempCU->getInterMergeCandidates( 0, 0, cMvFieldNeighbours,uhInterDirNeighbours, numValidMergeCand );

  Int mergeCandBuffer[MRG_MAX_NUM_CANDS];
  for( UInt ui = 0; ui < numValidMergeCand; ++ui )
  {
    mergeCandBuffer[ui] = 0;
  }

  Bool bestIsSkip = false;

  UInt iteration;
  if ( rpcTempCU->isLosslessCoded(0))
  {
    iteration = 1;
  }
  else
  {
    iteration = 2;
  }
  DEBUG_STRING_NEW(bestStr)

  for( UInt uiNoResidual = 0; uiNoResidual < iteration; ++uiNoResidual )
  {
    for( UInt uiMergeCand = 0; uiMergeCand < numValidMergeCand; ++uiMergeCand )
    {
      if(!(uiNoResidual==1 && mergeCandBuffer[uiMergeCand]==1))
      {
        if( !(bestIsSkip && uiNoResidual == 0) )
        {
          DEBUG_STRING_NEW(tmpStr)
          // set MC parameters
          rpcTempCU->setPredModeSubParts( MODE_INTER, 0, uhDepth ); // interprets depth relative to CTU level
          rpcTempCU->setCUTransquantBypassSubParts( bTransquantBypassFlag, 0, uhDepth );
          rpcTempCU->setChromaQpAdjSubParts( bTransquantBypassFlag ? 0 : m_ChromaQpAdjIdc, 0, uhDepth );
          rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N, 0, uhDepth ); // interprets depth relative to CTU level
          rpcTempCU->setMergeFlagSubParts( true, 0, 0, uhDepth ); // interprets depth relative to CTU level
          rpcTempCU->setMergeIndexSubParts( uiMergeCand, 0, 0, uhDepth ); // interprets depth relative to CTU level
          rpcTempCU->setInterDirSubParts( uhInterDirNeighbours[uiMergeCand], 0, 0, uhDepth ); // interprets depth relative to CTU level
          rpcTempCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[0 + 2*uiMergeCand], SIZE_2Nx2N, 0, 0 ); // interprets depth relative to rpcTempCU level
          rpcTempCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[1 + 2*uiMergeCand], SIZE_2Nx2N, 0, 0 ); // interprets depth relative to rpcTempCU level

          // do MC
          m_pcPredSearch->motionCompensation ( rpcTempCU, m_ppcPredYuvTemp[uhDepth] );
          // estimate residual and encode everything
          m_pcPredSearch->encodeResAndCalcRdInterCU( rpcTempCU,
                                                     m_ppcOrigYuv    [uhDepth],
                                                     m_ppcPredYuvTemp[uhDepth],
                                                     m_ppcResiYuvTemp[uhDepth],
                                                     m_ppcResiYuvBest[uhDepth],
                                                     m_ppcRecoYuvTemp[uhDepth],
                                                     (uiNoResidual != 0) DEBUG_STRING_PASS_INTO(tmpStr) );

#ifdef DEBUG_STRING
          DebugInterPredResiReco(tmpStr, *(m_ppcPredYuvTemp[uhDepth]), *(m_ppcResiYuvBest[uhDepth]), *(m_ppcRecoYuvTemp[uhDepth]), DebugStringGetPredModeMask(rpcTempCU->getPredictionMode(0)));
#endif

          if ((uiNoResidual == 0) && (rpcTempCU->getQtRootCbf(0) == 0))
          {
            // If no residual when allowing for one, then set mark to not try case where residual is forced to 0
            mergeCandBuffer[uiMergeCand] = 1;
          }

          Int orgQP = rpcTempCU->getQP( 0 );
          xCheckDQP( rpcTempCU );
          xCheckBestMode(rpcBestCU, rpcTempCU, uhDepth DEBUG_STRING_PASS_INTO(bestStr) DEBUG_STRING_PASS_INTO(tmpStr));

          rpcTempCU->initEstData( uhDepth, orgQP, bTransquantBypassFlag );

          if( m_pcEncCfg->getUseFastDecisionForMerge() && !bestIsSkip )
          {
            bestIsSkip = rpcBestCU->getQtRootCbf(0) == 0;
          }
        }
      }
    }

    if(uiNoResidual == 0 && m_pcEncCfg->getUseEarlySkipDetection())
    {
      if(rpcBestCU->getQtRootCbf( 0 ) == 0)
      {
        if( rpcBestCU->getMergeFlag( 0 ))
        {
          *earlyDetectionSkipMode = true;
        }
        else if(m_pcEncCfg->getFastSearch() != SELECTIVE)
        {
          Int absoulte_MV=0;
          for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
          {
            if ( rpcBestCU->getSlice()->getNumRefIdx( RefPicList( uiRefListIdx ) ) > 0 )
            {
              TComCUMvField* pcCUMvField = rpcBestCU->getCUMvField(RefPicList( uiRefListIdx ));
              Int iHor = pcCUMvField->getMvd( 0 ).getAbsHor();
              Int iVer = pcCUMvField->getMvd( 0 ).getAbsVer();
              absoulte_MV+=iHor+iVer;
            }
          }

          if(absoulte_MV == 0)
          {
            *earlyDetectionSkipMode = true;
          }
        }
      }
    }
  }
  DEBUG_STRING_APPEND(sDebug, bestStr)
}


#if AMP_MRG
Void TEncCu::xCheckRDCostInter( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize DEBUG_STRING_FN_DECLARE(sDebug), Bool bUseMRG)
#else
Void TEncCu::xCheckRDCostInter( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, PartSize ePartSize )
#endif
{
  DEBUG_STRING_NEW(sTest)

  // prior to this, rpcTempCU will have just been reset using rpcTempCU->initEstData( uiDepth, iQP, bIsLosslessMode );
  UChar uhDepth = rpcTempCU->getDepth( 0 );

  rpcTempCU->setPartSizeSubParts  ( ePartSize,  0, uhDepth );
  rpcTempCU->setPredModeSubParts  ( MODE_INTER, 0, uhDepth );
  rpcTempCU->setChromaQpAdjSubParts( rpcTempCU->getCUTransquantBypass(0) ? 0 : m_ChromaQpAdjIdc, 0, uhDepth );

#if AMP_MRG
  rpcTempCU->setMergeAMP (true);
  m_pcPredSearch->predInterSearch ( rpcTempCU, m_ppcOrigYuv[uhDepth], m_ppcPredYuvTemp[uhDepth], m_ppcResiYuvTemp[uhDepth], m_ppcRecoYuvTemp[uhDepth] DEBUG_STRING_PASS_INTO(sTest), false, bUseMRG );
#else
  m_pcPredSearch->predInterSearch ( rpcTempCU, m_ppcOrigYuv[uhDepth], m_ppcPredYuvTemp[uhDepth], m_ppcResiYuvTemp[uhDepth], m_ppcRecoYuvTemp[uhDepth] );
#endif

#if AMP_MRG
  if ( !rpcTempCU->getMergeAMP() )
  {
    return;
  }
#endif

  m_pcPredSearch->encodeResAndCalcRdInterCU( rpcTempCU, m_ppcOrigYuv[uhDepth], m_ppcPredYuvTemp[uhDepth], m_ppcResiYuvTemp[uhDepth], m_ppcResiYuvBest[uhDepth], m_ppcRecoYuvTemp[uhDepth], false DEBUG_STRING_PASS_INTO(sTest) );
  rpcTempCU->getTotalCost()  = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

#ifdef DEBUG_STRING
  DebugInterPredResiReco(sTest, *(m_ppcPredYuvTemp[uhDepth]), *(m_ppcResiYuvBest[uhDepth]), *(m_ppcRecoYuvTemp[uhDepth]), DebugStringGetPredModeMask(rpcTempCU->getPredictionMode(0)));
#endif

  xCheckDQP( rpcTempCU );
  xCheckBestMode(rpcBestCU, rpcTempCU, uhDepth DEBUG_STRING_PASS_INTO(sDebug) DEBUG_STRING_PASS_INTO(sTest));
}

Bool TEncCu::xCheckRDCostIntra( 
#if  SKIP_RDO_ENABLE			  
								Bool			&bSkipRDO, //SY added
#endif	
								TComDataCU *&rpcBestCU,
                                TComDataCU *&rpcTempCU,
                                Double      &cost,
                                PartSize     eSize,
								Bool        bMakeChange
                                DEBUG_STRING_FN_DECLARE(sDebug) )
{

  DEBUG_STRING_NEW(sTest)

  UInt uiDepth = rpcTempCU->getDepth( 0 );
  // initialize rpcTempCU 
  rpcTempCU->setSkipFlagSubParts( false, 0, uiDepth );
  rpcTempCU->setPartSizeSubParts( eSize, 0, uiDepth );
  rpcTempCU->setPredModeSubParts( MODE_INTRA, 0, uiDepth );
  rpcTempCU->setChromaQpAdjSubParts( rpcTempCU->getCUTransquantBypass(0) ? 0 : m_ChromaQpAdjIdc, 0, uiDepth );
  Pel resiLuma[NUMBER_OF_STORED_RESIDUAL_TYPES][MAX_CU_SIZE * MAX_CU_SIZE];
  Bool Partition=0;
#if  SKIP_RDO_ENABLE
  bSkipRDO = false;
  CurrentState  currentState = getCurrentState(uiDepth);
#endif
  // do the intra prediction for Luma
#if TIME_SYSTEM
  clock_t clockStart_estIntraPredLumaQT = clock();
#endif
  m_pcPredSearch->estIntraPredLumaQT( 
#if SKIP_RDO_ENABLE
	  bSkipRDO,
#endif	  
	  rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth], resiLuma DEBUG_STRING_PASS_INTO(sTest) );

#if TIME_SYSTEM
  if (eSize == SIZE_2Nx2N){
	  g_dTime_estIntraPredLumaQT_2Nx2N[uiDepth] += (Double)(clock() - clockStart_estIntraPredLumaQT) / CLOCKS_PER_SEC;
  }
  else{
	  g_dTime_estIntraPredLumaQT_NxN += (Double)(clock() - clockStart_estIntraPredLumaQT) / CLOCKS_PER_SEC;
  }
#endif
#if OUTPUT_INSIGHTDATA
#if GET_SATD_FEATURE
  if (eSize == SIZE_2Nx2N)
  {
    rpcTempCU->calSATDRatio();
	  // only output once
	  for (UInt uiFIdx = 0; uiFIdx < 4; uiFIdx++)
	  {
		  //g_InsightDataSet[uiDepth] << endl;
		  for (UInt uiMode = 0; uiMode < 35; uiMode++)
		  {
			  g_InsightDataSet[uiDepth] << rpcTempCU->getSATDFeature(uiMode, uiFIdx) << '\t';

		  }

      g_InsightDataSet[uiDepth] << endl;
	  }
    g_InsightDataSet[uiDepth] << rpcTempCU->getSATDRatio() << endl;
  }
#endif
#endif


#if SKIP_RDO_ENABLE
  if (bSkipRDO && currentState == Testing){
	  g_uiNumCU_Decision[uiDepth][SkipRDO]+=1;
	  rpcTempCU->getTotalCost() = MAX_DOUBLE;
	 return false;
  }
  else
#endif
  {
	  m_ppcRecoYuvTemp[uiDepth]->copyToPicComponent(COMPONENT_Y, rpcTempCU->getPic()->getPicYuvRec(), rpcTempCU->getCtuRsAddr(), rpcTempCU->getZorderIdxInCtu());

	
// do the intra prediction for Chroma 

	  if (rpcBestCU->getPic()->getChromaFormat() != CHROMA_400)
	  {
		  m_pcPredSearch->estIntraPredChromaQT(rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth], resiLuma DEBUG_STRING_PASS_INTO(sTest));
	  } 


	  m_pcEntropyCoder->resetBits();

	  if (rpcTempCU->getSlice()->getPPS()->getTransquantBypassEnableFlag())
	  {
		  cout << "rpcTempCU->getSlice()->getPPS()->getTransquantBypassEnableFlag()";
		  m_pcEntropyCoder->encodeCUTransquantBypassFlag(rpcTempCU, 0, true);
	  }//

	  // encoding  CUdata
	  m_pcEntropyCoder->encodeSkipFlag(rpcTempCU, 0, true);
	  m_pcEntropyCoder->encodePredMode(rpcTempCU, 0, true);
	  m_pcEntropyCoder->encodePartSize(rpcTempCU, 0, uiDepth, true);
	  m_pcEntropyCoder->encodePredInfo(rpcTempCU, 0);
	  m_pcEntropyCoder->encodeIPCMInfo(rpcTempCU, 0, true);

	  // Encode Coefficients
	  Bool bCodeDQP = getdQPFlag();
	  Bool codeChromaQpAdjFlag = getCodeChromaQpAdjFlag();
	  m_pcEntropyCoder->encodeCoeff(rpcTempCU, 0, uiDepth, bCodeDQP, codeChromaQpAdjFlag);
	  setCodeChromaQpAdjFlag(codeChromaQpAdjFlag);
	  setdQPFlag(bCodeDQP);

	  m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);

	  rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits();
	  rpcTempCU->getTotalBins() = ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
	  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost(rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion());

	  xCheckDQP(rpcTempCU);
  }
  cost = rpcTempCU->getTotalCost();

  if (bMakeChange == true){
	  Partition = xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth DEBUG_STRING_PASS_INTO(sDebug) DEBUG_STRING_PASS_INTO(sTest));
  }
  else
  {
	  Partition = false;
  }
  return Partition;
}


/** Check R-D costs for a CU with PCM mode.
 * \param rpcBestCU pointer to best mode CU data structure
 * \param rpcTempCU pointer to testing mode CU data structure
 * \returns Void
 *
 * \note Current PCM implementation encodes sample values in a lossless way. The distortion of PCM mode CUs are zero. PCM mode is selected if the best mode yields bits greater than that of PCM mode.
 */
Void TEncCu::xCheckIntraPCM( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU )
{
  UInt uiDepth = rpcTempCU->getDepth( 0 );

  rpcTempCU->setSkipFlagSubParts( false, 0, uiDepth );

  rpcTempCU->setIPCMFlag(0, true);
  rpcTempCU->setIPCMFlagSubParts (true, 0, rpcTempCU->getDepth(0));
  rpcTempCU->setPartSizeSubParts( SIZE_2Nx2N, 0, uiDepth );
  rpcTempCU->setPredModeSubParts( MODE_INTRA, 0, uiDepth );
  rpcTempCU->setTrIdxSubParts ( 0, 0, uiDepth );
  rpcTempCU->setChromaQpAdjSubParts( rpcTempCU->getCUTransquantBypass(0) ? 0 : m_ChromaQpAdjIdc, 0, uiDepth );

  m_pcPredSearch->IPCMSearch( rpcTempCU, m_ppcOrigYuv[uiDepth], m_ppcPredYuvTemp[uiDepth], m_ppcResiYuvTemp[uiDepth], m_ppcRecoYuvTemp[uiDepth]);

  m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);

  m_pcEntropyCoder->resetBits();

  if ( rpcTempCU->getSlice()->getPPS()->getTransquantBypassEnableFlag())
  {
    m_pcEntropyCoder->encodeCUTransquantBypassFlag( rpcTempCU, 0,          true );
  }

  m_pcEntropyCoder->encodeSkipFlag ( rpcTempCU, 0,          true );
  m_pcEntropyCoder->encodePredMode ( rpcTempCU, 0,          true );
  m_pcEntropyCoder->encodePartSize ( rpcTempCU, 0, uiDepth, true );
  m_pcEntropyCoder->encodeIPCMInfo ( rpcTempCU, 0, true );

  m_pcRDGoOnSbacCoder->store(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);

  rpcTempCU->getTotalBits() = m_pcEntropyCoder->getNumberOfWrittenBits();
  rpcTempCU->getTotalBins() = ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
  rpcTempCU->getTotalCost() = m_pcRdCost->calcRdCost( rpcTempCU->getTotalBits(), rpcTempCU->getTotalDistortion() );

  xCheckDQP( rpcTempCU );
  DEBUG_STRING_NEW(a)
  DEBUG_STRING_NEW(b)
  xCheckBestMode(rpcBestCU, rpcTempCU, uiDepth DEBUG_STRING_PASS_INTO(a) DEBUG_STRING_PASS_INTO(b));
}

/** check whether current try is the best with identifying the depth of current try
 * \param rpcBestCU
 * \param rpcTempCU
 * \param uiDepth
 */
Bool TEncCu::xCheckBestMode( TComDataCU*& rpcBestCU, TComDataCU*& rpcTempCU, UInt uiDepth DEBUG_STRING_FN_DECLARE(sParent) DEBUG_STRING_FN_DECLARE(sTest) DEBUG_STRING_PASS_INTO(Bool bAddSizeInfo) )
{
	Bool Change = false;
	if (rpcTempCU->getTotalCost() < rpcBestCU->getTotalCost()){
		Change = true;
	}
	
	if (Change)
  {
    TComYuv* pcYuv;
    // Exchange Information data
    TComDataCU* pcCU = rpcBestCU;
    rpcBestCU = rpcTempCU;
    rpcTempCU = pcCU;

    // Change Prediction data
    pcYuv = m_ppcPredYuvBest[uiDepth];
    m_ppcPredYuvBest[uiDepth] = m_ppcPredYuvTemp[uiDepth];
    m_ppcPredYuvTemp[uiDepth] = pcYuv;

    // Change Reconstruction data
    pcYuv = m_ppcRecoYuvBest[uiDepth];
    m_ppcRecoYuvBest[uiDepth] = m_ppcRecoYuvTemp[uiDepth];
    m_ppcRecoYuvTemp[uiDepth] = pcYuv;
    pcYuv = NULL;
    pcCU  = NULL;

    // store temp best CI for next CU coding
    m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]->store(m_pppcRDSbacCoder[uiDepth][CI_NEXT_BEST]);

#ifdef DEBUG_STRING
    DEBUG_STRING_SWAP(sParent, sTest)
    const PredMode predMode=rpcBestCU->getPredictionMode(0);
    if ((DebugOptionList::DebugString_Structure.getInt()&DebugStringGetPredModeMask(predMode)) && bAddSizeInfo)
    {
      std::stringstream ss(stringstream::out);
      ss <<"###: " << (predMode==MODE_INTRA?"Intra   ":"Inter   ") << partSizeToString[rpcBestCU->getPartitionSize(0)] << " CU at " << rpcBestCU->getCUPelX() << ", " << rpcBestCU->getCUPelY() << " width=" << UInt(rpcBestCU->getWidth(0)) << std::endl;
      sParent+=ss.str();
    }
#endif
  }
	return Change;
}

Void TEncCu::xCheckDQP( TComDataCU* pcCU )
{
  UInt uiDepth = pcCU->getDepth( 0 );

  if( pcCU->getSlice()->getPPS()->getUseDQP() && (g_uiMaxCUWidth>>uiDepth) >= (g_uiMaxCUWidth >> ( pcCU->getSlice()->getPPS()->getMaxCuDQPDepth())) )
  {
    if ( pcCU->getQtRootCbf( 0) )
    {
#if !RDO_WITHOUT_DQP_BITS
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeQP( pcCU, 0, false );
      pcCU->getTotalBits() += m_pcEntropyCoder->getNumberOfWrittenBits(); // dQP bits
      pcCU->getTotalBins() += ((TEncBinCABAC *)((TEncSbac*)m_pcEntropyCoder->m_pcEntropyCoderIf)->getEncBinIf())->getBinsCoded();
      pcCU->getTotalCost() = m_pcRdCost->calcRdCost( pcCU->getTotalBits(), pcCU->getTotalDistortion() );
#endif
    }
    else
    {
      pcCU->setQPSubParts( pcCU->getRefQP( 0 ), 0, uiDepth ); // set QP to default QP
    }
  }
}

Void TEncCu::xCopyAMVPInfo (AMVPInfo* pSrc, AMVPInfo* pDst)
{
  pDst->iN = pSrc->iN;
  for (Int i = 0; i < pSrc->iN; i++)
  {
    pDst->m_acMvCand[i] = pSrc->m_acMvCand[i];
  }
}
Void TEncCu::xCopyYuv2Pic(TComPic* rpcPic, UInt uiCUAddr, UInt uiAbsPartIdx, UInt uiDepth, UInt uiSrcDepth, TComDataCU* pcCU, UInt uiLPelX, UInt uiTPelY )
{
  UInt uiAbsPartIdxInRaster = g_auiZscanToRaster[uiAbsPartIdx];
  UInt uiSrcBlkWidth = rpcPic->getNumPartInCtuWidth() >> (uiSrcDepth);
  UInt uiBlkWidth    = rpcPic->getNumPartInCtuWidth() >> (uiDepth);
  UInt uiPartIdxX = ( ( uiAbsPartIdxInRaster % rpcPic->getNumPartInCtuWidth() ) % uiSrcBlkWidth) / uiBlkWidth;
  UInt uiPartIdxY = ( ( uiAbsPartIdxInRaster / rpcPic->getNumPartInCtuWidth() ) % uiSrcBlkWidth) / uiBlkWidth;
  UInt uiPartIdx = uiPartIdxY * ( uiSrcBlkWidth / uiBlkWidth ) + uiPartIdxX;
  m_ppcRecoYuvBest[uiSrcDepth]->copyToPicYuv( rpcPic->getPicYuvRec (), uiCUAddr, uiAbsPartIdx, uiDepth - uiSrcDepth, uiPartIdx);

  m_ppcPredYuvBest[uiSrcDepth]->copyToPicYuv( rpcPic->getPicYuvPred (), uiCUAddr, uiAbsPartIdx, uiDepth - uiSrcDepth, uiPartIdx);
}

Void TEncCu::xCopyYuv2Tmp( UInt uiPartUnitIdx, UInt uiNextDepth )
{
  UInt uiCurrDepth = uiNextDepth - 1;
  m_ppcRecoYuvBest[uiNextDepth]->copyToPartYuv( m_ppcRecoYuvTemp[uiCurrDepth], uiPartUnitIdx );
  m_ppcPredYuvBest[uiNextDepth]->copyToPartYuv( m_ppcPredYuvBest[uiCurrDepth], uiPartUnitIdx);
}

/** Function for filling the PCM buffer of a CU using its original sample array
 * \param pCU pointer to current CU
 * \param pOrgYuv pointer to original sample array
 */
Void TEncCu::xFillPCMBuffer     ( TComDataCU* pCU, TComYuv* pOrgYuv )
{
  const ChromaFormat format = pCU->getPic()->getChromaFormat();
  const UInt numberValidComponents = getNumberValidComponents(format);
  for (UInt componentIndex = 0; componentIndex < numberValidComponents; componentIndex++)
  {
    const ComponentID component = ComponentID(componentIndex);

    const UInt width  = pCU->getWidth(0)  >> getComponentScaleX(component, format);
    const UInt height = pCU->getHeight(0) >> getComponentScaleY(component, format);

    Pel *source      = pOrgYuv->getAddr(component, 0, width);
    Pel *destination = pCU->getPCMSample(component);

    const UInt sourceStride = pOrgYuv->getStride(component);

    for (Int line = 0; line < height; line++)
    {
      for (Int column = 0; column < width; column++)
      {
        destination[column] = source[column];
      }

      source      += sourceStride;
      destination += width;
    }
  }
}

#if ADAPTIVE_QP_SELECTION
/** Collect ARL statistics from one block
  */
Int TEncCu::xTuCollectARLStats(TCoeff* rpcCoeff, TCoeff* rpcArlCoeff, Int NumCoeffInCU, Double* cSum, UInt* numSamples )
{
  for( Int n = 0; n < NumCoeffInCU; n++ )
  {
    TCoeff u = abs( rpcCoeff[ n ] );
    TCoeff absc = rpcArlCoeff[ n ];

    if( u != 0 )
    {
      if( u < LEVEL_RANGE )
      {
        cSum[ u ] += ( Double )absc;
        numSamples[ u ]++;
      }
      else
      {
        cSum[ LEVEL_RANGE ] += ( Double )absc - ( Double )( u << ARL_C_PRECISION );
        numSamples[ LEVEL_RANGE ]++;
      }
    }
  }

  return 0;
}

//! Collect ARL statistics from one CTU
Void TEncCu::xCtuCollectARLStats(TComDataCU* pCtu )
{
  Double cSum[ LEVEL_RANGE + 1 ];     //: the sum of DCT coefficients corresponding to data type and quantization output
  UInt numSamples[ LEVEL_RANGE + 1 ]; //: the number of coefficients corresponding to data type and quantization output

  TCoeff* pCoeffY = pCtu->getCoeff(COMPONENT_Y);
  TCoeff* pArlCoeffY = pCtu->getArlCoeff(COMPONENT_Y);

  UInt uiMinCUWidth = g_uiMaxCUWidth >> g_uiMaxCUDepth;
  UInt uiMinNumCoeffInCU = 1 << uiMinCUWidth;

  memset( cSum, 0, sizeof( Double )*(LEVEL_RANGE+1) );
  memset( numSamples, 0, sizeof( UInt )*(LEVEL_RANGE+1) );

  // Collect stats to cSum[][] and numSamples[][]
  for(Int i = 0; i < pCtu->getTotalNumPart(); i ++ )
  {
    UInt uiTrIdx = pCtu->getTransformIdx(i);

    if(pCtu->isInter(i) && pCtu->getCbf( i, COMPONENT_Y, uiTrIdx ) )
    {
      xTuCollectARLStats(pCoeffY, pArlCoeffY, uiMinNumCoeffInCU, cSum, numSamples);
    }//Note that only InterY is processed. QP rounding is based on InterY data only.

    pCoeffY  += uiMinNumCoeffInCU;
    pArlCoeffY  += uiMinNumCoeffInCU;
  }

  for(Int u=1; u<LEVEL_RANGE;u++)
  {
    m_pcTrQuant->getSliceSumC()[u] += cSum[ u ] ;
    m_pcTrQuant->getSliceNSamples()[u] += numSamples[ u ] ;
  }
  m_pcTrQuant->getSliceSumC()[LEVEL_RANGE] += cSum[ LEVEL_RANGE ] ;
  m_pcTrQuant->getSliceNSamples()[LEVEL_RANGE] += numSamples[ LEVEL_RANGE ] ;
}
#endif
//! \}
