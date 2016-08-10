#include "tools_YS.h"


void YSGlobalControl(){
	//g_mainModelType = SVM_0; g_mainFeatureType = OBF;
	g_mainModelType = Naive; g_mainFeatureType = N_OBF;
	//g_mainModelType = RTS_OpenCV; g_mainFeatureType = BOutlier;
	//g_mainModelType = BayesNan; g_mainFeatureType = N_OBF;
	g_C[0] = 0.000002;
	g_W_P[0] = 1;
	g_W_N[0] = 1;

	g_modelType_2		= SVM_1;
	g_featureType_2 = NewFeature_1;
	g_C[1] = 0.000002;
	g_W_P[1] = 2;
	g_W_N[1] = 1;

	g_modelType_3 = SVM_2;
	g_featureType_3 = NewFeature_2;
	g_C[2] = 0.000002;
	g_W_P[2] = 50;
	g_W_N[2] = 1;

	g_bTrustAssistantSkip = false; 
	g_bMultiModel		  = false;
	g_bDepthException	  = true;

	if (g_bMultiModel){
		g_bModelSwitch[g_mainModelType] = true;
		g_bModelSwitch[BayesDecision]	= true;
		g_bModelSwitch[Assistant_Skip]	= true;
	}
	else
	{
		//g_bModelSwitch[g_mainModelType]= true;
		//g_bModelSwitch[Assistant_Skip] = true;
		//g_bModelSwitch[SVM_0]= true;
		g_bModelSwitch[Naive] = true;
		//g_bModelSwitch[Assistant_Skip] = true;
		g_bModelSwitch[SVM_1] = true;
	    g_bModelSwitch[SVM_2] = true;
		//g_bModelSwitch[RTS_OpenCV] = true;
		
		//g_bModelSwitch[BayesNan] = true;
	}
	setArray(g_iP, NUM_CU_DEPTH, 60);
	setArray(g_iT, NUM_CU_DEPTH, 2);
	setArray(g_iV, NUM_CU_DEPTH, 1);

	g_dPrecision_Th_Default = 0.8;

	g_dPrecision_Th[0][SVM_0][Skip2Nx2N] = 0.8;
	g_dPrecision_Th[1][SVM_0][Skip2Nx2N] = 0.8;
	g_dPrecision_Th[2][SVM_0][Skip2Nx2N] = 0.8;
	g_dPrecision_Th[3][SVM_0][Skip2Nx2N] = 0.8;

	g_dPrecision_Th[0][SVM_0][TerminateCU] = 0.8;
	g_dPrecision_Th[1][SVM_0][TerminateCU] = 0.8;
	g_dPrecision_Th[2][SVM_0][TerminateCU] = 0.8;
	g_dPrecision_Th[3][SVM_0][TerminateCU] = 0.8;

}

void openOutputFiles(){
	// precision 
	// sequence information
	g_pFile_Performance = fopen("summary_Performance.txt", "at");
	fprintf(g_pFile_Performance, "\tYUV:\t");
	fprintf(g_pFile_Performance, (const char*)g_pchInputFile);

	// training period information
	fprintf(g_pFile_Performance, "\tT:\t");
	fprintf(g_pFile_Performance, "%d\t",g_iT[0]);
	fprintf(g_pFile_Performance, "\tV:\t");
	fprintf(g_pFile_Performance, "%d\t", g_iV[0]);
	fprintf(g_pFile_Performance, "\tQP:\t");
	fprintf(g_pFile_Performance, "%d\n",g_iQP);
	// 
	string parameter;
	parameter += "\tC:\t";
	parameter += to_string(g_C[0]);
	parameter += "\t";
	parameter += to_string(g_C[1]);
	parameter += "\t";
	parameter += to_string(g_C[2]);
	parameter += "\n";
	parameter += "\tW_P:\t";
	parameter += to_string(g_W_P[0]);
	parameter += "\t";
	parameter += to_string(g_W_P[1]);
	parameter += "\t";
	parameter += to_string(g_W_P[2]);
	parameter += "\n";
	parameter += "\tW_N:\t";
	parameter += to_string(g_W_N[0]);
	parameter += "\t";
	parameter += to_string(g_W_N[1]);
	parameter += "\t";
	parameter += to_string(g_W_N[2]);
	parameter += "\n";
	fprintf(g_pFile_Performance, (const char*)parameter.c_str());
	// List title 
	string listTitle;
	listTitle += "\tuiDepth\t";
	listTitle += "  SkipP\t";
	listTitle += "  SkipS\t";
	listTitle += "  TermP\t";
	listTitle += "  TermS\t";
	listTitle += "\n";
	fprintf(g_pFile_Performance, (const char*)listTitle.c_str());
}

void closeOutputFiles(){
	fprintf(g_pFile_Performance,"\n");
	fclose(g_pFile_Performance);
}


void resetYSGlobal(Int uiDepth)
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Training Statistic
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	setArray(g_iFeatureLength[uiDepth], NUM_FEATURETYPE, 0);
	setArray(g_iSampleNumber[uiDepth], NUM_FEATURETYPE, 0);
	UInt Sizex = (g_uiMaxCUHeight / 4 >> uiDepth)*(g_uiMaxCUHeight / 4 >> uiDepth);
	// SVM feature ScaleFactor
	g_ScaleFactor[uiDepth]	= 100;
	g_d2NRDCost[uiDepth]	= -1;
	g_d2NBits[uiDepth]		= -1;
	g_d2NDistortion[uiDepth]= -1;
	g_dSOC[uiDepth]			= -1;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	///Verification Statistic
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	clear2dArray<double>(g_iVerResult[uiDepth], NUM_MODELTYPE, NUM_RESULTTYPE);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Model Switch /
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	clear2dArray<bool>(g_bDecisionSwitch[uiDepth], NUM_MODELTYPE, NUM_DECISIONTYPE);

#if PARAMETER_SELECTION

	if (uiDepth == 0 && g_iPOC!=0)
	{
	g_C[0] *=10;
	g_W_P[0] = 1;
	g_W_N[0] = 1;


	g_C[1] = 0.000002;
	g_W_P[1] = 1;
	g_W_N[1] += 5;

	g_C[2] = 0.000002;
	g_W_P[2] += 5;
	g_W_N[2] = 1;

	string parameter;
	parameter += "\tC:\t";
	parameter += to_string(g_C[0]);
	parameter += "\t";
	parameter += to_string(g_C[1]);
	parameter += "\t";
	parameter += to_string(g_C[2]);
	parameter += "\n";
	parameter += "\tW_P:\t";
	parameter += to_string(g_W_P[0]);
	parameter += "\t";
	parameter += to_string(g_W_P[1]);
	parameter += "\t";
	parameter += to_string(g_W_P[2]);
	parameter += "\n";
	parameter += "\tW_N:\t";
	parameter += to_string(g_W_N[0]);
	parameter += "\t";
	parameter += to_string(g_W_N[1]);
	parameter += "\t";
	parameter += to_string(g_W_N[2]);
	parameter += "\n";
	fprintf(g_pFile_Performance, (const char*)parameter.c_str());
	}
#endif
	g_cBayesModel_Nan.clearModel();
}

void createYSGlobal(){  // allocate mamery for pointers 
	g_iP = new int[NUM_CU_DEPTH];
	g_iT = new int[NUM_CU_DEPTH];
	g_iV = new int[NUM_CU_DEPTH];
	g_bModelSwitch = new bool[NUM_MODELTYPE];
	setArray(g_bModelSwitch, NUM_MODELTYPE, false);
	// 3D
	create3dArray<double>(g_iVerResult, NUM_CU_DEPTH, NUM_MODELTYPE, NUM_RESULTTYPE);      // [uiDepth][ModelType][TFPN]
	create3dArray<bool>(g_bDecisionSwitch, NUM_CU_DEPTH, NUM_MODELTYPE, NUM_DECISIONTYPE);    // [uiDepth][ModelType][DecisionType]
	create3dArray<double>(g_dPrecision_Th, NUM_CU_DEPTH, NUM_MODELTYPE, NUM_DECISIONTYPE); //  [uiDepth][ModelType][DecisionType]

	create3dFileArray(g_TrainingSet, NUM_CU_DEPTH, NUM_FEATUREFORMAT, NUM_FEATURETYPE);
	create2dFileArray(model_linear, NUM_CU_DEPTH, NUM_FEATURETYPE);
	create2dFileArray(model_CvSVM, NUM_CU_DEPTH, NUM_FEATURETYPE);
	create2dFileArray(model_CvRTrees, NUM_CU_DEPTH, NUM_FEATURETYPE);
	create2dArray(g_iFeatureLength, NUM_CU_DEPTH, NUM_FEATURETYPE);
	create2dArray(g_iSampleNumber, NUM_CU_DEPTH, NUM_FEATURETYPE);


	//create3dFileArray(g_dSATD, NUM_CU_DEPTH, 2, 2);

	//////////////////// Analytical output files 
#if OUTPUT_INSIGHTDATA	// create file array
	g_InsightDataSet = new ofstream[NUM_CU_DEPTH];
#endif
#if NEW_FEATURESYSTEM // create file array
	g_TrainingDataSet = new ofstream[NUM_CU_DEPTH];
#endif

	///////////////////
}

void createStat(){

}

void deleteYSGlobal(){
	delete[] g_iP;
	delete[] g_iT;
	delete[] g_iV;
	delete[] g_bModelSwitch;
	// 3D
	delete3dArray<double>(g_iVerResult, NUM_CU_DEPTH, NUM_MODELTYPE, NUM_RESULTTYPE);      // [uiDepth][ModelType][TFPN]
	delete3dArray<bool>(g_bDecisionSwitch, NUM_CU_DEPTH, NUM_MODELTYPE, NUM_DECISIONTYPE);    // [uiDepth][ModelType][DecisionType]
	delete3dArray<double>(g_dPrecision_Th, NUM_CU_DEPTH, NUM_MODELTYPE, NUM_DECISIONTYPE); //  [uiDepth][ModelType][DecisionType]

	// 2D
	delete3dArray(g_TrainingSet, NUM_CU_DEPTH, NUM_FEATUREFORMAT, NUM_FEATURETYPE);

	delete2dArray(model_linear, NUM_CU_DEPTH, NUM_FEATURETYPE);
	delete2dArray(model_CvSVM, NUM_CU_DEPTH, NUM_FEATURETYPE);
	delete2dArray(model_CvRTrees, NUM_CU_DEPTH, NUM_FEATURETYPE);

	delete2dArray(g_iFeatureLength, NUM_CU_DEPTH, NUM_FEATURETYPE);
	delete2dArray(g_iSampleNumber, NUM_CU_DEPTH, NUM_FEATURETYPE);

	//////////////////// Analytical output files 
#if OUTPUT_INSIGHTDATA
	delete[] g_InsightDataSet;
#endif
#if NEW_FEATURESYSTEM
	delete[] g_TrainingDataSet;
#endif
	g_cBayesModel_Nan.outputModel();
	///////////////////
}

#if TIME_SYSTEM // Definitions of Functions
void resetTimeSystem(){
	for (int i = 0; i < 4; i++){
	g_dTime_xCompressCU[i] = 0;
	//CheckRDcostIntra
	g_dTime_xCheckRDCostIntra_2Nx2N[i] = 0;
	//estIntraPredLumaQT
	g_dTime_estIntraPredLumaQT_2Nx2N[i] = 0;
	//2Nx2N
	g_dTime_RMD_2Nx2N[i]= 0;
	g_dTime_SATD_2Nx2N[i] = 0;
	g_dTime_RDO_2Nx2N[i]=0;

	// Overall 
	g_dTime_Training[i]=0;
	g_dTime_FeatureExtraction[i]=0;
	g_dTime_Prediction[i]=0;
	}

	//CheckRDcostIntra
	g_dTime_xCheckRDCostIntra_NxN = 0;
	g_dTime_estIntraPredLumaQT_NxN = 0;
	g_dTime_Total = 0;
	//NxN
	g_dTime_RMD_NxN  = 0;
	g_dTime_SATD_NxN = 0;
	g_dTime_RDO_NxN  = 0;
}
void printTimeSystem(){
	g_pFile_TimeSystem = fopen("SummaryTime.txt", "at");
	fprintf(g_pFile_TimeSystem, "\n");
	fprintf(g_pFile_TimeSystem, "YUV:");
	fprintf(g_pFile_TimeSystem, (const char*)g_pchInputFile);
	fprintf(g_pFile_TimeSystem, "\tQP:%d",g_iQP);
	fprintf(g_pFile_TimeSystem, "\tFrames:%d",g_iFrameToBeEncoded);

	// training period information
	fprintf(g_pFile_TimeSystem, "\tPeriod:%d/%d/%d", g_iT[0], g_iV[0], g_iP[0]);
	fprintf(g_pFile_TimeSystem, "\n");


	fprintf(g_pFile_TimeSystem, "Total Encoding Time\t%f\n", g_dTime_Total);

	fprintf(g_pFile_TimeSystem, "xCompressCU: \t");
	for (int i = 0; i < 4; i++){
		fprintf(g_pFile_TimeSystem, "\t%5.1f%%",g_dTime_xCompressCU[i] / g_dTime_Total*100);
	}
	fprintf(g_pFile_TimeSystem, "\n");


	fprintf(g_pFile_TimeSystem, "xCheckRDCost:\t");
	for (int i = 0; i < 4; i++){
		fprintf(g_pFile_TimeSystem, "\t%5.1f%%", g_dTime_xCheckRDCostIntra_2Nx2N[i] / g_dTime_Total * 100);
	}
	fprintf(g_pFile_TimeSystem, "\t%5.1f%%", g_dTime_xCheckRDCostIntra_NxN / g_dTime_Total * 100);
	fprintf(g_pFile_TimeSystem, "\n");

	fprintf(g_pFile_TimeSystem, "estIntraPred:\t");
	for (int i = 0; i < 4; i++){
		fprintf(g_pFile_TimeSystem, "\t%5.1f%%", g_dTime_estIntraPredLumaQT_2Nx2N[i] / g_dTime_Total * 100);
	}
	fprintf(g_pFile_TimeSystem, "\t%5.1f%%", g_dTime_estIntraPredLumaQT_NxN / g_dTime_Total * 100);
	fprintf(g_pFile_TimeSystem, "\n");


	fprintf(g_pFile_TimeSystem, "RMD:\t\t\t");
	for (int i = 0; i < 4; i++){
		//fprintf(g_pFile_TimeSystem, "Depth %d:\t%f\t%5.1f%%\n", i, g_dTime_xCompressCU[i], g_dTime_xCompressCU[i] / g_dTime_Total*100);
		fprintf(g_pFile_TimeSystem, "\t%5.1f%%", g_dTime_RMD_2Nx2N[i] / g_dTime_Total * 100);
	}
	fprintf(g_pFile_TimeSystem, "\t%5.1f%%", g_dTime_RMD_NxN/ g_dTime_Total * 100);
	fprintf(g_pFile_TimeSystem, "\n");

	fprintf(g_pFile_TimeSystem, "RDO:\t\t\t");
	for (int i = 0; i < 4; i++){
		//fprintf(g_pFile_TimeSystem, "Depth %d:\t%f\t%5.1f%%\n", i, g_dTime_xCompressCU[i], g_dTime_xCompressCU[i] / g_dTime_Total*100);
		fprintf(g_pFile_TimeSystem, "\t%5.1f%%", g_dTime_RDO_2Nx2N[i] / g_dTime_Total * 100);
	}
	fprintf(g_pFile_TimeSystem, "\t%5.1f%%", g_dTime_RDO_NxN / g_dTime_Total * 100);
	fprintf(g_pFile_TimeSystem, "\n");

	fprintf(g_pFile_TimeSystem, "FeatureExtract:\t\t");
	for (int i = 0; i < 4; i++){
		//fprintf(g_pFile_TimeSystem, "Depth %d:\t%f\t%5.1f%%\n", i, g_dTime_xCompressCU[i], g_dTime_xCompressCU[i] / g_dTime_Total*100);
		fprintf(g_pFile_TimeSystem, "\t%5.1f%%", g_dTime_FeatureExtraction[i] / g_dTime_Total * 100);
	}
	fprintf(g_pFile_TimeSystem, "\n");

	fprintf(g_pFile_TimeSystem, "Training:\t\t");
	for (int i = 0; i < 4; i++){
		//fprintf(g_pFile_TimeSystem, "Depth %d:\t%f\t%5.1f%%\n", i, g_dTime_xCompressCU[i], g_dTime_xCompressCU[i] / g_dTime_Total*100);
		fprintf(g_pFile_TimeSystem, "\t%5.1f%%", g_dTime_Training[i] / g_dTime_Total * 100);
	}
	fprintf(g_pFile_TimeSystem, "\n");

	fprintf(g_pFile_TimeSystem, "Prediction:\t\t");
	for (int i = 0; i < 4; i++){
		//fprintf(g_pFile_TimeSystem, "Depth %d:\t%f\t%5.1f%%\n", i, g_dTime_xCompressCU[i], g_dTime_xCompressCU[i] / g_dTime_Total*100);
		fprintf(g_pFile_TimeSystem, "\t%5.1f%%", g_dTime_Prediction[i] / g_dTime_Total * 100);
	}
	fprintf(g_pFile_TimeSystem, "\n");

	fclose(g_pFile_TimeSystem);
}

#endif

//////////////////////////////////////////////////////////////////////////
// New feature system 
///////////////////////////////////////////////////////////////////////////


void FormFeatureVector_NewFeatureSystem(TComDataCU*& rpcBestCU, UInt uiDepth, Double*& feature, Int& featureLength, FeatureType Type){

}

void OnlineTrain_NewFeatureSystem(Int uiDepth){

}

void FeatureProcess_NewFeatureSystem(Int uiDepth, Double*& feature, Int& featureLength, Int FeatureType){

}

void DoPrediction_NewFeatureSystem(UInt uiDepth, Double* feature, Int featureLength, Double& label_predict, ModelType modelType, FeatureType featureType){

}




void OutputInsightDataSet(TComDataCU*& rpcBestCU, UInt uiDepth){

}














template <typename T> void OutputFeatureVector(ofstream* file, T* feature, Int featureLength, Double label, UInt uiDepth, FeatureFormat format)
{	
	switch (format)
	{
	case LibLinear:
		// save the label
		if (label > 0)
		{
			*file << "+1 ";
		}
		else
		{
			*file << "-1 ";
		}
		// save the data
		for (int i = 0; i < featureLength; i++)
		{
			if (feature[i] != 0){
				//if (feature[i] < 0)cout << "fuck negtive";
			*file << i + 1 << ":";
			*file << feature[i] << "\t";
			 }
		}
		*file << endl;
		break;
	case OpenCV:
		for (int i = 0; i < featureLength; i++)
		{
			*file << feature[i] << "\t";
		}
		// end line
		if (label > 0)
		{
			*file << "+1 ";
		}
		else
		{
			*file << "-1 ";
		}
		*file << endl;
		break;
	case Unknown:
		for (int i = 0; i < featureLength; i++)
		{
			*file << feature[i] << "\t";
		}
		// end line
		if (label > 0)
		{
			*file << "+1 ";
		}
		else
		{
			*file << "-1 ";
		}
		*file << endl;
		break;
	default:

		break;
	}
	
}
template void OutputFeatureVector(ofstream* file, Double* feature, Int featureLength, Double label, UInt uiDepth, FeatureFormat format);
template void OutputFeatureVector(ofstream* file, Pel* feature, Int featureLength, Double label, UInt uiDepth, FeatureFormat format);





template <typename T>
void FormFeatureVector(TComDataCU*& rpcBestCU, UInt uiDepth, T*& feature, Int& featureLength, FeatureType Type)
{
	UInt uiLPelX = rpcBestCU->getCUPelX();
	UInt uiRPelX = uiLPelX + rpcBestCU->getWidth(0) - 1;
	UInt uiTPelY = rpcBestCU->getCUPelY();
	UInt uiBPelY = uiTPelY + rpcBestCU->getHeight(0) - 1;
	TComPic* pcPic = rpcBestCU->getPic();

	// ORG
	TComPicYuv* dataOrg = pcPic->getPicYuvOrg();
	Pel* pOrg = dataOrg->getAddr(COMPONENT_Y);
	UInt uiStrideOrg = dataOrg->getStride(COMPONENT_Y);
	const UInt uiFrameWidth = dataOrg->getWidth(COMPONENT_Y);
	const UInt uiFrameHeight = dataOrg->getHeight(COMPONENT_Y);
	//Outlier
	TComPicYuv* dataOutlier = pcPic->getPicYuvOutlier();
	Pel* pOutlier = dataOutlier->getAddr(COMPONENT_Y);
	UInt uiStrideOutlier = dataOutlier->getStride(COMPONENT_Y);

	//OBF
	TComPicYuv* dataOBF = pcPic->getOBF();
	Pel* pOBF = dataOBF->getAddr(COMPONENT_Y);
	UInt uiStrideOBF = dataOBF->getStride(COMPONENT_Y);
	UInt uiFrameWidth_OBF = dataOBF->getWidth(COMPONENT_Y);  // this is right 
	UInt uiFrameHeight_OBF = dataOBF->getHeight(COMPONENT_Y);

	// others
	Int iIntraMode_UpperCU = rpcBestCU->getIntraMode_UpperCU();  // read the intramode of upperCU
	if (iIntraMode_UpperCU == -1) // if UpperCU data is NULL , assign -1 
	{
		//cout << uiDepth<< " Null"<<endl;
		iIntraMode_UpperCU = -1;
	}
	if (iIntraMode_UpperCU != NULL)
	{
		//cout <<uiDepth <<" "<< iIntraMode_UpperCU <<endl;
	}



	Int BlockSize = g_uiMaxCUWidth >> uiDepth;

	Int Num_Outlier = rpcBestCU->getNumOutlier();
	Int Num_OBF		= rpcBestCU->getNumOBF();

	if (Type == OBF || Type == EOBF){
		Int NeighborExtend_OBF = 0;
		if (Type == EOBF){
			if (uiDepth >= 0 && uiDepth <= 3) // Depth 1 2 
			{
				NeighborExtend_OBF = 1;
			}
		}

		Int BlockSize_ExtendOBF = (BlockSize >> 2) + NeighborExtend_OBF * 2;
		featureLength = BlockSize_ExtendOBF*BlockSize_ExtendOBF;
		feature = new T[featureLength];

		Int StartPoint_X = (Int)uiLPelX / 4 - NeighborExtend_OBF;
		Int StartPoint_Y = (Int)uiTPelY / 4 - NeighborExtend_OBF;
		if (StartPoint_X + BlockSize_ExtendOBF-1)
		//Bool bIsBoundaryCU = (StartPoint_X < 0 || StartPoint_Y < 0);

		for (int y = 0; y < BlockSize_ExtendOBF; y++)
		{
			for (int x = 0; x < BlockSize_ExtendOBF; x++)
			{
				if (StartPoint_X < 0 && x == 0)    // left boundary 
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;

				}
				else if (StartPoint_Y < 0 && y == 0)   // top boundary
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
				//	cout << "top boundary " << endl;
				}
				else if ((StartPoint_X + BlockSize_ExtendOBF) > uiFrameWidth_OBF && x == BlockSize_ExtendOBF-1)    // right boundary
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
				//	cout << " right boundary " << endl;
				}
				else if ((StartPoint_Y + BlockSize_ExtendOBF) > uiFrameHeight_OBF && y == BlockSize_ExtendOBF - 1)  // bottom boundary 
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
				//	cout << "bottom boundary "<<endl;
				}
				else if (x + StartPoint_X > uiFrameWidth_OBF-1 || StartPoint_Y + y > uiFrameHeight_OBF-1)
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
				}
				else
				{
					feature[y*BlockSize_ExtendOBF + x] = (T) pOBF[x + StartPoint_X + (StartPoint_Y + y)*uiStrideOBF];
					//if (pOBF[x + StartPoint_X + (StartPoint_Y + y)*uiStrideOBF] < 0 && uiDepth == 0)
						// cout << "X\t" << x + StartPoint_X << "\tY\t" << StartPoint_Y + y << endl;
				}
				if (feature[y*BlockSize_ExtendOBF + x] != 0)
				{
				//	feature->n_nonzero++;    // Number of non-Zero feature 
				}
			}
		}
	}
	else if (Type == SOC)
	{
		Int BlockSize_Outlier = BlockSize;
		Int tmpDCTSize = 4;
		featureLength = tmpDCTSize * tmpDCTSize;
		feature = new T[featureLength];
		memset(feature, 0, sizeof(T)*featureLength);
		for (int y = 0; y < BlockSize_Outlier; y++)
		{
			for (int x = 0; x < BlockSize_Outlier; x++)
			{
				int coef_x = x%tmpDCTSize;
				int coef_y = y%tmpDCTSize;
				feature[coef_y*tmpDCTSize + coef_x] += (T)pOutlier[x + uiLPelX + (uiTPelY + y)*uiStrideOutlier];
			}
		}
	}
	else if (Type == SUBSOC)
	{
		Int BlockSize_Outlier = BlockSize;
		Int SubBlockSize_Outlier = BlockSize_Outlier / 2;
		Int tmpDCTSize = 4;
		featureLength = tmpDCTSize * tmpDCTSize * 4;
		feature = new T[featureLength];
		memset(feature, 0, sizeof(T)*featureLength);
		// subblock 1 
		for (int x = 0; x < SubBlockSize_Outlier; x++)
		{
			for (int y = 0; y < SubBlockSize_Outlier; y++)
			{
				int coef_x = x%tmpDCTSize;
				int coef_y = y%tmpDCTSize;
				feature[coef_y*tmpDCTSize + coef_x] += (T)pOutlier[x + uiLPelX + (uiTPelY + y)*uiStrideOutlier];
			}
		}
		// subblock 2 
		for (int x = 0; x < SubBlockSize_Outlier; x++)
		{
			for (int y = 0; y < SubBlockSize_Outlier; y++)
			{
				int coef_x = x%tmpDCTSize;
				int coef_y = y%tmpDCTSize;
				feature[16 + coef_y*tmpDCTSize + coef_x] += (T)pOutlier[x + SubBlockSize_Outlier + uiLPelX + (uiTPelY + y)*uiStrideOutlier];
			}
		}
		// subblock 3 
		for (int x = 0; x < SubBlockSize_Outlier; x++)
		{
			for (int y = 0; y < SubBlockSize_Outlier; y++)
			{
				int coef_x = x%tmpDCTSize;
				int coef_y = y%tmpDCTSize;
				feature[16 * 2 + coef_y*tmpDCTSize + coef_x] += (T)pOutlier[x + uiLPelX + (uiTPelY + y + SubBlockSize_Outlier)*uiStrideOutlier];
			}
		}
		// subblock 4
		for (int x = 0; x < SubBlockSize_Outlier; x++)
		{
			for (int y = 0; y < SubBlockSize_Outlier; y++)
			{
				int coef_x = x%tmpDCTSize;
				int coef_y = y%tmpDCTSize;
				feature[16 * 3 + coef_y*tmpDCTSize + coef_x] += (T)pOutlier[x + SubBlockSize_Outlier + uiLPelX + (uiTPelY + y + SubBlockSize_Outlier)*uiStrideOutlier];
			}
		}
	}
	else if (Type == MIXED)
	{
		Int BlockSize_Outlier = BlockSize;
		Int tmpDCTSize = 4;
		Int featurelength = tmpDCTSize * tmpDCTSize + 4;
		feature = new T[featurelength];  // SOC 
		memset(feature, 0, sizeof(T)*featurelength);  // OBF feature
		for (int y = 0; y < BlockSize_Outlier; y++)
		{
		for (int x = 0; x < BlockSize_Outlier; x++)
			{
				int coef_x = x%tmpDCTSize;
				int coef_y = y%tmpDCTSize;
				feature[coef_y*tmpDCTSize + coef_x] += (T)pOutlier[x + uiLPelX + (uiTPelY + y)*uiStrideOutlier];
			}
		}
	}
	else if (Type == Outlier || Type == BOutlier)
	{
		Int BlockSize_Outlier = BlockSize;
		featureLength = BlockSize_Outlier* BlockSize_Outlier ;
		feature = new T[featureLength];  // SOC 
		memset(feature, 0, sizeof(T)*featureLength);  // OBF feature
		for (int y = 0; y < BlockSize_Outlier; y++)
		{
			for (int x = 0; x < BlockSize_Outlier; x++)
			{
				if (Type == Outlier)
				{
					feature[y*BlockSize_Outlier + x] = (T)pOutlier[x + uiLPelX + (uiTPelY + y)*uiStrideOutlier];
				}
				else if (Type == BOutlier)
				{				
					if (pOutlier[x + uiLPelX + (uiTPelY + y)*uiStrideOutlier]!=0)
						feature[y*BlockSize_Outlier + x] = 1;
					else 
						feature[y*BlockSize_Outlier + x] = 0;
				}
				if (pOutlier[x + uiLPelX + (uiTPelY + y)*uiStrideOutlier] < 0 && uiDepth == 0)
					cout << "X\t" << uiLPelX << "\tY\t" << uiTPelY << endl;

			}
		}
	}
	else if (Type == N_OBF)
	{
		featureLength = 1;
		feature = new T[2];  // SOC 
		feature[0] = (T) Num_OBF;
	}
	else if (Type == OBF_SATD){
		Int NeighborExtend_OBF = 0;
		if (Type == EOBF){
			if (uiDepth >= 0 && uiDepth <= 3) // Depth 1 2 
			{
				NeighborExtend_OBF = 1;
			}
		}

		Int BlockSize_ExtendOBF = (BlockSize >> 2) + NeighborExtend_OBF * 2;
		featureLength = BlockSize_ExtendOBF*BlockSize_ExtendOBF ;
		//featureLength += 1;
		feature = new T[featureLength];

		Int StartPoint_X = (Int)uiLPelX / 4 - NeighborExtend_OBF;
		Int StartPoint_Y = (Int)uiTPelY / 4 - NeighborExtend_OBF;
		if (StartPoint_X + BlockSize_ExtendOBF - 1)
			//Bool bIsBoundaryCU = (StartPoint_X < 0 || StartPoint_Y < 0);

			//feature->n_nonzero = 0;
		for (int y = 0; y < BlockSize_ExtendOBF; y++)
		{
			for (int x = 0; x < BlockSize_ExtendOBF; x++)
			{
				if (StartPoint_X < 0 && x == 0)    // left boundary 
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
					//	cout << "left boundary  " << endl;
				}
				else if (StartPoint_Y < 0 && y == 0)   // top boundary
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
					//	cout << "top boundary " << endl;
				}
				else if ((StartPoint_X + BlockSize_ExtendOBF) > uiFrameWidth_OBF && x == BlockSize_ExtendOBF - 1)    // right boundary
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
					//	cout << " right boundary " << endl;
				}
				else if ((StartPoint_Y + BlockSize_ExtendOBF) > uiFrameHeight_OBF && y == BlockSize_ExtendOBF - 1)  // bottom boundary 
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
					//	cout << "bottom boundary "<<endl;
				}
				else if (x + StartPoint_X > uiFrameWidth_OBF - 1 || StartPoint_Y + y > uiFrameHeight_OBF - 1)
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
				}
				else
				{
					feature[y*BlockSize_ExtendOBF + x] = (T)pOBF[x + StartPoint_X + (StartPoint_Y + y)*uiStrideOBF];
					//if (pOBF[x + StartPoint_X + (StartPoint_Y + y)*uiStrideOBF] < 0 && uiDepth == 0)
					// cout << "X\t" << x + StartPoint_X << "\tY\t" << StartPoint_Y + y << endl;
				}
				if (feature[y*BlockSize_ExtendOBF + x] != 0)
				{
					//	feature->n_nonzero++;    // Number of non-Zero feature 
				}
			}
		}
		double SATD = (T)rpcBestCU->getSATD();
		if (SATD  < 0)cout << "Fuck: SATD(T) is negative" << endl;
		//feature[featureLength-1] = (T) SATD;
	
	}
	else if (Type == OBF_SATD_RDCost){
		Int NeighborExtend_OBF = 0;
		if (Type == EOBF){
			if (uiDepth >= 0 && uiDepth <= 3) // Depth 1 2 
			{
				NeighborExtend_OBF = 1;
			}
		}

		Int BlockSize_ExtendOBF = (BlockSize >> 2) + NeighborExtend_OBF * 2;
		featureLength = BlockSize_ExtendOBF*BlockSize_ExtendOBF ; 
		//featureLength += 2;

		feature = new T[featureLength];

		Int StartPoint_X = (Int)uiLPelX / 4 - NeighborExtend_OBF;
		Int StartPoint_Y = (Int)uiTPelY / 4 - NeighborExtend_OBF;
		if (StartPoint_X + BlockSize_ExtendOBF - 1)
			//Bool bIsBoundaryCU = (StartPoint_X < 0 || StartPoint_Y < 0);

			//feature->n_nonzero = 0;
		for (int y = 0; y < BlockSize_ExtendOBF; y++)
		{
			for (int x = 0; x < BlockSize_ExtendOBF; x++)
			{
				if (StartPoint_X < 0 && x == 0)    // left boundary 
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
					//	cout << "left boundary  " << endl;
				}
				else if (StartPoint_Y < 0 && y == 0)   // top boundary
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
					//	cout << "top boundary " << endl;
				}
				else if ((StartPoint_X + BlockSize_ExtendOBF) > uiFrameWidth_OBF && x == BlockSize_ExtendOBF - 1)    // right boundary
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
					//	cout << " right boundary " << endl;
				}
				else if ((StartPoint_Y + BlockSize_ExtendOBF) > uiFrameHeight_OBF && y == BlockSize_ExtendOBF - 1)  // bottom boundary 
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
					//	cout << "bottom boundary "<<endl;
				}
				else if (x + StartPoint_X > uiFrameWidth_OBF - 1 || StartPoint_Y + y > uiFrameHeight_OBF - 1)
				{
					feature[y*BlockSize_ExtendOBF + x] = 0;
				}
				else
				{
					feature[y*BlockSize_ExtendOBF + x] =(T) pOBF[x + StartPoint_X + (StartPoint_Y + y)*uiStrideOBF];
				}
			}
		}
		if (rpcBestCU->getSATD() < 0)cout << "Fuck: SATD is negative"<<endl;
		if (rpcBestCU->getTotalCost() < 0)cout << "Fuck: RDCost is negative" << endl;
		double SATD = (T)rpcBestCU->getSATD();
		
		double RDCost = (T)rpcBestCU->getTotalCost();
		
		//feature[featureLength - 2] =(T) SATD;
		//feature[featureLength - 1] =(T) RDCost;
		
	}
	else if (Type == SATD_RDCost){
		featureLength = 2;
		feature = new T[featureLength];
		double SATD = (T)rpcBestCU->getSATD();
		double RDCost = (T)rpcBestCU->getTotalCost();
		feature[0] =(T) SATD;
		feature[1] =(T) RDCost;
	}
	else if (Type == NoneFeature){
		//do nothing 
	}
	else if (Type == NewFeature || Type == NewFeature_1 || Type == NewFeature_2 || Type == NewFeature_3){
		featureLength = 35*4;
		feature = new T[featureLength];
		int i = 0;
		for (UInt uiFIdx = 0; uiFIdx < 4; uiFIdx++)
		{
			//g_InsightDataSet[uiDepth] << endl;
			for (UInt uiMode = 0; uiMode < 35; uiMode++)
			{
				feature[i] = rpcBestCU->getSATDFeature(uiMode, uiFIdx);
				i++;
			}
		}
		return;
	}
	else
	{
		cout << "Fuck :undefined Feature Type!" << endl;
	}
}
template void FormFeatureVector(TComDataCU*& rpcBestCU, UInt uiDepth, Double*& feature, Int& featureLength, FeatureType Type);
template void FormFeatureVector(TComDataCU*& rpcBestCU, UInt uiDepth, Pel*& feature, Int& featureLength, FeatureType Type);
//template void FormFeatureVector(TComDataCU*& rpcBestCU, UInt uiDepth, TMVFeature*& feature, Int& featureLength, FeatureType Type);


template <typename T>
void DoPrediction(UInt uiDepth, T* feature, Int featureLength, Double& label_predict, ModelType modelType, FeatureType featureType)
{
#if TIME_SYSTEM
	clock_t clockStart = clock();
#endif
	if (modelType == SVM_0 || modelType == SVM_1 || modelType == SVM_2)
	{
		feature_node* input_vector = new feature_node[featureLength + 1];
		int j = 0;
		for (int i = 0; i < featureLength; i++)
		{
			if (feature[i] != 0)
			{
				input_vector[j].index = i + 1;
				input_vector[j].value = feature[i];
				j++;
			}
		}
		input_vector[j].index = -1;
		model* model;
		model =  model_linear[uiDepth][featureType];
		label_predict = predict(model, input_vector);
		delete[] input_vector;
		input_vector = NULL;
		return;
	}
	else if (modelType == SVM_OpenCV)
	{
		Mat Feature_Vector = Mat::zeros(1, featureLength, CV_32FC1);
		for (int i = 0; i < featureLength; i++)
		{
		Feature_Vector.at<float>(0, i) = feature[i];
		}
		label_predict = model_CvSVM[uiDepth][featureType].predict(Feature_Vector);
		return;
	}
	else if (modelType == RTS_OpenCV)
	{
		Mat Feature_Vector = Mat::zeros(1, featureLength, CV_32FC1);
		for (int i = 0; i < featureLength; i++)
		{
			Feature_Vector.at<float>(0, i) = feature[i];
		}
		label_predict = model_CvRTrees[uiDepth][featureType].predict(Feature_Vector);
		return;
	}
	else if (modelType == BayesDecision)
	{
		return;
	}
	else if (modelType == Naive)
	{
		if (feature[0]==0){  // the current Cost Risk is smaller
			label_predict = -1;
		}
		else{
			label_predict = +1;
		}
		return;
	}
	else if (modelType == BayesNew)
	{
		label_predict = 0;
		return;
	}
	else if (modelType == BayesNan){
		label_predict = g_cBayesModel_Nan.predict(uiDepth,N_OBF);
		return;
	}
	else return;

#if TIME_SYSTEM
	g_dTime_Prediction[uiDepth] += (Double)(clock() - clockStart) / CLOCKS_PER_SEC;
#endif
}
template void DoPrediction(UInt uiDepth, Double* feature, Int featureLength, Double& label_predict, ModelType modelType, FeatureType featureType);
template void DoPrediction(UInt uiDepth, Pel* feature, Int featureLength, Double& label_predict, ModelType modelType, FeatureType featureType);



template <typename T>
void DoPrediction(UInt uiDepth, Int x_OBF, Double& label_predict, ModelType modelType)  // for BayesDecision
{

    if (modelType == BayesDecision)
	{
		Double R1 = 0;
		Double R0 = 0;
		R0 = g_dJdx01[uiDepth][x_OBF];
		R1 = g_dJdx10[uiDepth][x_OBF];
		if (R0 < R1){  // the current Cost Risk is smaller
			label_predict = -1;
		}
		else{
			label_predict = +1;
		}
		return;
	}
	else
	{
		cout << "Error : this funciton is only for BayesDecision" <<endl;
	}
}

void OnlineTrain(Int uiDepth, ModelType modelType, FeatureType featureType)
{
#if TIME_SYSTEM
	clock_t clockStart = clock();
#endif
	FeatureFormat featureFormat = getFeatureFormat(modelType);
	String TrainingSetFileName = getTrainingSetFileName(uiDepth,modelType, featureFormat, featureType);
	string modelname = getOffLineModelNmae(uiDepth, modelType, featureType);
	if ( modelType == SVM_1){
		Double C = g_C[1];//0.000002
		Double W_P=g_W_P[1];
		Double W_N=g_W_N[1];
		Bool useVarC = true;
		if (featureType == NewFeature_1 && useVarC){
			switch (uiDepth)
			{
			case 0:
				W_P = 1;
				W_N = 50;
				break;
			case 1:
				W_P = 1;
				W_N = 50;
				break;
			case 2:
				W_P = 2;
				W_N = 1;
				break;
			case 3:
				W_P = 3;
				W_N = 1;
				break;
			default:
				break;
			}
		}

		if (featureType == NewFeature_2 && useVarC){
			switch (uiDepth)
			{
			case 0:
				W_P = 1;
				W_N = 50;
				break;
			case 1:
				W_P = 1;
				W_N = 50;
				break;
			case 2:
				W_P = 2;
				W_N = 1;
				break;
			case 3:
				W_P = 3;
				W_N = 1;
				break;
			default:
				break;
			}
		}


		string  command = "train";
		command += " ";
			command += " -s 2";
		command += " ";
			command += "-c";
		command += " ";
			command += to_string(C);
		command += " ";
			command += "-w1";
		command += " ";
			command += to_string(W_P);
		command += " ";
			command += "-w-1";
		command += " ";
			command += to_string(W_N);
		command += " ";
			command += "-q";
		command += " ";
			command += TrainingSetFileName;
		command += " ";
			command += modelname;
		system(command.c_str());
		model_linear[uiDepth][featureType] = load_model(modelname.c_str());
	}
	else if (modelType == SVM_2){
		Double C = g_C[2];//0.000002
		Double W_P = g_W_P[2];
		Double W_N = g_W_N[2];
		string  command = "train";
		command += " ";
		command += " -s 2";
		command += " ";
		command += "-c";
		command += " ";
		command += to_string(C);
		command += " ";
		command += "-w1";
		command += " ";
		command += to_string(W_P);
		command += " ";
		command += "-w-1";
		command += " ";
		command += to_string(W_N);
		command += " ";
		command += "-q";
		command += " ";
		command += TrainingSetFileName;
		command += " ";
		command += modelname;
		system(command.c_str());
		model_linear[uiDepth][featureType] = load_model(modelname.c_str());
	}
	else if (modelType == SVM_0){
		Double C = g_C[0];//0.000002
		string  command = "train";
		command += " ";
		command += " -s 2";
		command += " ";
		command += "-c";
		command += " ";
		command += to_string(C);
		command += " ";
		command += "-q";
		command += " ";
		command += TrainingSetFileName;
		command += " ";
		command += modelname;
		system(command.c_str());
		model_linear[uiDepth][featureType] = load_model(modelname.c_str());
	}
	else if (modelType == SVM_OpenCV){
		Mat matData;
		Mat FeatureMat;
		Mat LabelMat;
		Int matRows;
		Int matCols;
		matRows = g_iSampleNumber[uiDepth][featureType];
		matCols = g_iFeatureLength[uiDepth][featureType] + 1;
		LoadData(TrainingSetFileName, matData, matRows, matCols, 1);
		FeatureMat = matData.colRange(0, g_iFeatureLength[uiDepth][featureType]);
		LabelMat = matData.colRange(g_iFeatureLength[uiDepth][featureType], g_iFeatureLength[uiDepth][featureType] + 1);

		CvSVMParams params;
		//SVM类型： C-Support Vector Classification  
		params.svm_type = SVM::C_SVC;

		params.C = 0.1;
		params.kernel_type = SVM::LINEAR;
		// params.kernel_type = SVM::RBF;
		// params.gamma = 2;
		// params.term_crit = TermCriteria(CV_TERMCRIT_ITER, (int) 1e7, 1e-6);

		cout << "开始训练过程" << endl;
		clock_t start, finish;
		double duration;
		start = clock();

		float tmp = 0;
		// the one class exception 
		Bool bOneClass = true;
		for (int i = 0; i < matRows; i++)
		{
			if (tmp == 0)
			{
				tmp = LabelMat.at<float>(i, 0);
			}
			else if (tmp != 0 && tmp != LabelMat.at<float>(i, 0))
			{
				bOneClass = false;
			}
		}
		if (bOneClass)
		{
			params.svm_type = SVM::ONE_CLASS;
			params.nu = 0.5;
		}

		model_CvSVM[uiDepth][featureType].train(FeatureMat, LabelMat, Mat(), Mat(), params);

		finish = clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "训练过程结束,共耗时：" << duration << "秒" << endl;
	}
	else if (modelType == RTS_OpenCV) {
		Mat matData;
		Mat FeatureMat;
		Mat LabelMat;
		Int matRows = g_iSampleNumber[uiDepth][featureType];
		Int matCols = g_iFeatureLength[uiDepth][featureType] + 1;

		LoadData(TrainingSetFileName, matData, matRows, matCols, 1);
		FeatureMat = matData.colRange(0, g_iFeatureLength[uiDepth][featureType]);
		LabelMat = matData.colRange(g_iFeatureLength[uiDepth][featureType], g_iFeatureLength[uiDepth][featureType] + 1);

		float priors[] = { 1, 1 };  // weights of each classification for classes
		CvRTParams params = CvRTParams(25, // max depth
			5, // min sample count
			0, // regression accuracy: N/A here
			false, // compute surrogate split, no missing data
			2, // max number of categories (use sub-optimal algorithm for larger numbers)
			priors, // the array of priors
			false,  // calculate variable importance
			4,       // number of variables randomly selected at node and used to find the best split(s).
			100,	 // max number of trees in the forest
			0.01f,				// forrest accuracy
			CV_TERMCRIT_ITER | CV_TERMCRIT_EPS // termination cirteria
			);

		Mat var_type = Mat(matCols, 1, CV_8U);
		CvRTrees* rtree = new CvRTrees;
		cout << "RandomTrees 开始训练过程" << endl;

		//开始计时  
		clock_t start, finish;
		double duration;
		start = clock();

		model_CvRTrees[uiDepth][featureType].train(FeatureMat, CV_ROW_SAMPLE, LabelMat,
			Mat(), Mat(), var_type, Mat(), params);
		finish = clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "训练过程结束,共耗时：" << duration << "秒" << endl;
	}
	/*else if (modelType == BayesNew){
		CurrentState currentState = getCurrentState(uiDepth);
			UInt uiMaxOutlier[4] = { 257, 65, 17, 5 };
			Double alfas[4] = { 0.8, 0.85, 0.9, 0.95 };
			Double ratio[2] = { 1.1, 0.5 };

			clock_t lBefore = clock();
			for (UInt x = 0; x < 2; x++){
				cout << "\n\tx: " << x << " Non Part: " << g_iNumCU_Train[uiDepth][x][TerminateCU] << " Part: " << g_iNumCU_Train[uiDepth][x][Skip2Nx2N] << " ";
				//partition SATD (skip RDO after SATD)
				if (g_iNumCU_Train[uiDepth][x][TerminateCU]  > 0){
					Double alfa = alfas[uiDepth];
					sort(g_dSATD[uiDepth][x][TerminateCU], g_dSATD[uiDepth][x][TerminateCU] + g_iNumCU_Train[uiDepth][x][TerminateCU] - 1);
					int position = (UInt)((Double)(g_iNumCU_Train[uiDepth][x][TerminateCU] * alfa));
					cout << "Pos: " << position << " ";
					Double g_dSATD_Th_SkipRDOTmp = g_dSATD[uiDepth][x][TerminateCU][position];
					cout << "ThTmp: " << g_dSATD_Th_SkipRDOTmp << " ";
					Bool bOK = false;
					if (g_iNumCU_Train[uiDepth][x][Skip2Nx2N]> 0){
						bOK = true;
						sort(g_dSATD[uiDepth][x][Skip2Nx2N], g_dSATD[uiDepth][x][Skip2Nx2N] + g_iNumCU_Train[uiDepth][x][Skip2Nx2N] - 1);
						//TO DO: find position
						Int posNonPartMax = findPos(g_dSATD[uiDepth][x][TerminateCU], g_iNumCU_Train[uiDepth][x][Skip2Nx2N] - 1, g_dSATD[uiDepth][x][1][g_iNumCU_Train[uiDepth][x][Skip2Nx2N] - 1]);
						if (posNonPartMax < (Double)g_iNumCU_Train[uiDepth][x][Skip2Nx2N] * 0.7)
							bOK = false;  // 
						else{
							Int posNonPar = findPos(g_dSATD[uiDepth][x][1], g_iNumCU_Train[uiDepth][x][Skip2Nx2N] - 1, g_dSATD_Th_SkipRDOTmp);
							UInt uiNumSkipNon = (UInt)((Double)(g_iNumCU_Train[uiDepth][x][TerminateCU] * (1 - alfa)));
							UInt uiNumSkipPart = g_iNumCU_Train[uiDepth][x][Skip2Nx2N] - posNonPar;
							if (uiNumSkipNon * 4.5 > uiNumSkipPart){
								bOK = false; // this can be understand 
							}
							printf("(Skip Non Part: %d, Skip Part: %d)", uiNumSkipNon, uiNumSkipPart);
						}
					}
					else{

					}

					if (bOK){
						g_dSATD_Th_SkipRDO[uiDepth][x] = g_dSATD_Th_SkipRDOTmp;
					}
					else{
						g_dSATD_Th_SkipRDO[uiDepth][x] = dSATDMax[uiDepth][x];
					}
				}
				else{
					cout << " Pos: 0 ";
					g_dSATD_Th_SkipRDO[uiDepth][x] = 0;
				}
			}
	} */
#if TIME_SYSTEM
	g_dTime_Training[uiDepth] += (Double)(clock() - clockStart) / CLOCKS_PER_SEC;
#endif
}



void countTFPN(int prediction, bool label, double*** & iVerResult, unsigned int uiDepth, ModelType modelType){
	ResultType TFPN = NoResult;
	if ((prediction == Skip2Nx2N || prediction == SkipRDO )&& label == true) TFPN = TP;
	if ((prediction == Skip2Nx2N || prediction == SkipRDO) && label == false)TFPN = FP;
	if (prediction == TerminateCU && label == true) TFPN = FN;
    if (prediction == TerminateCU && label == false)TFPN = TN;
	if (TFPN != NoResult) iVerResult[uiDepth][modelType][TFPN] += 1.0;
}

void countRDLoss(int prediction, bool label, double RDCost2Nx2N, double RDCostNxN, double***& iVerResult, unsigned int uiDepth, ModelType modelType){
	ResultType LossType = NoResult;
	double Loss = abs(RDCost2Nx2N - RDCostNxN);
	if (prediction == TerminateCU && label == true)LossType = FNLoss;
	if (prediction == Skip2Nx2N && label == false)LossType = FPLoss;		
	if (LossType != NoResult)
		iVerResult[uiDepth][modelType][LossType] += Loss;
}

void printPerformance(unsigned int uiDepth, ModelType modelType){
	switch (modelType)
	{
	case NoneModel:
		break;
	case SVM_0:
		cout << "                         Depth:" << uiDepth << "                                     " << endl;
		cout << "ModelType:\t" << ModelType_Names[modelType] << "                  " << endl;
		//cout << "g_dAccuracy :\t" << getVerAccuracy<double>(uiDepth, modelType);
		cout << "\t" << "TP:" << g_iVerResult[uiDepth][modelType][TP] << '\t' << "FP:" << g_iVerResult[uiDepth][modelType][FP] << '\t' << "TN:" << g_iVerResult[uiDepth][modelType][TN] << '\t' << "FN:" << g_iVerResult[uiDepth][modelType][FN] << endl;
		cout << "Skip precision:\t" << getSkipPrecision<double>(uiDepth, modelType) * 100 << "%" << '\t';
		cout << "Skip Sensivtivity:\t" << getSkipSensitivity<double>(uiDepth, modelType) * 100 << "%" << endl;
		cout << "Term precision:\t" << getTermPrecision<double>(uiDepth, modelType) * 100 << "%" << '\t';
		cout << "Term Sensivtivity:\t" << getTermSensitivity<double>(uiDepth, modelType)*100 << "%"<<endl;
		//cout << "RDloss by False Skip:\t" << g_iVerResult[uiDepth][modelType][FPLoss] << '\n' << "RDloss by False Term:\t" << g_iVerResult[uiDepth][modelType][FNLoss] << endl;
		break;
	case SVM_1:
		cout << "ModelType:\t" << ModelType_Names[modelType] << "                  " << endl;
		cout << "Skip precision:\t" << getSkipPrecision<double>(uiDepth, modelType) * 100 << "%" << '\t';
		cout << "Skip Sensivtivity:\t" << getSkipSensitivity<double>(uiDepth, modelType) * 100 << "%" << endl;
		break;
	case SVM_2:
		cout << "ModelType:\t" << ModelType_Names[modelType] << "                  " << endl;
		cout << "Term precision:\t" << getTermPrecision<double>(uiDepth, modelType) * 100 << "%" << '\t';
		cout << "Term Sensivtivity:\t" << getTermSensitivity<double>(uiDepth, modelType) * 100 << "%" << endl;
		break;
	case SVM_OpenCV:
		break;
	case RTS_OpenCV:
		break;
	case Naive:
		cout << "                         Depth:" << uiDepth << "                                     " << endl;
		cout << "ModelType:\t" << ModelType_Names[modelType] << "                  " << endl;
		//cout << "g_dAccuracy :\t" << getVerAccuracy<double>(uiDepth, modelType);
		cout << "\t" << "TP:" << g_iVerResult[uiDepth][modelType][TP] << '\t' << "FP:" << g_iVerResult[uiDepth][modelType][FP] << '\t' << "TN:" << g_iVerResult[uiDepth][modelType][TN] << '\t' << "FN:" << g_iVerResult[uiDepth][modelType][FN] << endl;
		cout << "Skip precision:\t" << getSkipPrecision<double>(uiDepth, modelType) * 100 << "%" << '\t';
		cout << "Skip Sensivtivity:\t" << getSkipSensitivity<double>(uiDepth, modelType) * 100 << "%" << endl;
		cout << "Term precision:\t" << getTermPrecision<double>(uiDepth, modelType) * 100 << "%" << '\t';
		cout << "Term Sensivtivity:\t" << getTermSensitivity<double>(uiDepth, modelType) * 100 << "%" << endl;
		//cout << "RDloss by False Skip:\t" << g_iVerResult[uiDepth][modelType][FPLoss] << '\n' << "RDloss by False Term:\t" << g_iVerResult[uiDepth][modelType][FNLoss] << endl;
		break;
	case BayesDecision:
		break;
	case Assistant_Skip:
		cout << "ModelType:\t" << ModelType_Names[modelType] << "                  " << endl;
		cout << "Skip precision:\t" << getSkipPrecision<double>(uiDepth, modelType) * 100 << "%" << '\t';
		cout << "Skip Sensivtivity:\t" << getSkipSensitivity<double>(uiDepth, modelType) * 100 << "%" << endl;
		break;
	case BayesNew:
		break;
	default:
		break;
	}
}

void outputPerformance(unsigned int uiDepth, ModelType modelType){

	//fprintf(g_pFile_Performance, "\t %d \t %5.1f/%5.1f \t%5.1f/%5.1f", (int)uiDepth, getSkipPrecision<double>(uiDepth, SVM_1) * 100, getSkipPrecision<double>(uiDepth, SVM_0) * 100, getSkipSensitivity<double>(uiDepth, SVM_1) * 100, getSkipSensitivity<double>(uiDepth, SVM_0) * 100);
	//fprintf(g_pFile_Performance, "\t %5.1f/%5.1f \t %5.1f/%5.1f\n", getTermPrecision<double>(uiDepth, SVM_2) * 100, getTermPrecision<double>(uiDepth, SVM_0) * 100, getTermSensitivity<double>(uiDepth, SVM_2) * 100, getTermSensitivity<double>(uiDepth, SVM_0) * 100);
	fprintf(g_pFile_Performance, "Depth: %d\t", (int)uiDepth);
	fprintf(g_pFile_Performance, "Model: ");
	fprintf(g_pFile_Performance, (char*)ModelType_Names[modelType]);
	fprintf(g_pFile_Performance, "\t %5.1f \t %5.1f \t", getSkipPrecision<double>(uiDepth, modelType) * 100, getSkipSensitivity<double>(uiDepth, modelType) * 100);
	if (g_bDecisionSwitch[uiDepth][modelType][Skip2Nx2N] == true)fprintf(g_pFile_Performance, "ON\t");
	else fprintf(g_pFile_Performance, "OFF\t");
	fprintf(g_pFile_Performance, "\t %5.1f \t %5.1f \t", getTermPrecision<double>(uiDepth, modelType) * 100, getTermSensitivity<double>(uiDepth, modelType) * 100);
	if (g_bDecisionSwitch[uiDepth][modelType][TerminateCU] == true)fprintf(g_pFile_Performance, "ON\t");
	else fprintf(g_pFile_Performance, "OFF\t");
	fprintf(g_pFile_Performance, "\n");
}



double FeatureProcess(Int Feature, Int FeatureType, Int uiDepth)
{
	Double ProcessedFeature;
	if (FeatureType == 1)   // SOC
	{
		if (Feature > 0)
		{
			ProcessedFeature = (double)Feature;
		}

		return ProcessedFeature;
	}
	if (FeatureType == 17)  // Intra Mode
	{
		if (Feature == 0)
		{
			ProcessedFeature = 0;
		}
		else if (Feature == 1)
		{
			ProcessedFeature = 0.5;
		}
		else
		{
			ProcessedFeature = 1;
		}
		return ProcessedFeature;
	}
	else if (FeatureType == 18)  // RD Cost
	{
		ProcessedFeature = (double)Feature / g_d2NRDCost[uiDepth] * 100;
		return ProcessedFeature;
	}
	else if (FeatureType == 19)  // Bits
	{
		ProcessedFeature = (double)Feature / g_d2NBits[uiDepth] * 100;
		return ProcessedFeature;
	}
	else if (FeatureType == 20) //Distortion
	{
		ProcessedFeature = (double)Feature / g_d2NDistortion[uiDepth] * 100;
		return ProcessedFeature;
	}

}

void  SetDecisionSwitch(int uiDepth, ModelType modelType){
	if (!g_bModelSwitch[modelType]) return;
	bool print =false;
	Double dPrecision_Th_Skip;
	Double dPrecision_Th_Term;
	if (g_dPrecision_Th[uiDepth][modelType][Skip2Nx2N] == 0){
		dPrecision_Th_Skip = g_dPrecision_Th_Default;
	}
	else{
		dPrecision_Th_Skip = g_dPrecision_Th[uiDepth][modelType][Skip2Nx2N];
	}
	
	if (g_dPrecision_Th[uiDepth][modelType][TerminateCU] == 0){
		dPrecision_Th_Term = g_dPrecision_Th_Default;
	}
	else{
		dPrecision_Th_Term = g_dPrecision_Th[uiDepth][modelType][TerminateCU];
	}
	// set Skip2Nx2N switch 
		if (getSkipPrecision<double>(uiDepth, modelType) > dPrecision_Th_Skip){
		g_bDecisionSwitch[uiDepth][modelType][Skip2Nx2N] = true;
		g_bDecisionSwitch[uiDepth][modelType][SkipRDO] = true;
		if(print)cout << "ON: Skip2Nx2N in Depth: " << uiDepth << " is turnned on !" << endl;
	}
	else if (print)cout << "OFF: Skip2Nx2N in Depth: " << uiDepth << " is turnned off !" << endl;
	// set terminateCU switch 
	if (getTermPrecision<double>(uiDepth, modelType) > dPrecision_Th_Term){
		g_bDecisionSwitch[uiDepth][modelType][TerminateCU] = true;
	if (print)cout << "ON: TerminateCU in Depth: " << uiDepth << " is turnned on !" << endl;
	}
	else  if (print)cout << "OFF: TerminateCU in Depth: " << uiDepth << " is turnned off !" << endl;
}

FeatureFormat getFeatureFormat(ModelType modelType){
	FeatureFormat featureFormat;
	switch (modelType)
	{
	case SVM_0:
		featureFormat = LibLinear;
		break;
	case SVM_1:
		featureFormat = LibLinear;
		break;
	case SVM_2:
		featureFormat = LibLinear;
		break;
	case SVM_OpenCV:
		featureFormat = OpenCV;
		break;
	case RTS_OpenCV:
		featureFormat = OpenCV;
		break;
	case BayesDecision:
		featureFormat = BayesStat;
		break;
	case Assistant_Skip:
		featureFormat = Unknown;
		break;
	default:
		featureFormat = Unknown;
		break;
	}
	return featureFormat;
}

string getTrainingSetFileName(unsigned int uiDepth,ModelType modelType ,FeatureFormat featureFormat, FeatureType featureType){
string filename = "data_train";
filename += "_";
filename += to_string(uiDepth);
filename += "_";
filename += ModelType_Names[modelType];
filename += "_";
filename += FeatureFormat_Names[featureFormat];
filename += "_";
filename += FeatureType_Names[featureType];
filename += ".txt";
return filename;
}

string getInsightDataSetFileName(unsigned int uiDepth){
	string filename = "InsightDataSet";
	filename += "_";
	filename += g_pchInputFile;
	filename += "_";
	filename += to_string(g_iQP);
	filename += "_";
	filename += to_string(uiDepth);
	filename += ".txt";
	return filename;
}

string getTrainingDataSetFileName(unsigned int uiDepth){
	string filename = "TrainingDataSet";
	filename += "_";
	filename += g_pchInputFile;
	filename += "_";
	filename += to_string(g_iQP);
	filename += "_";
	filename += to_string(uiDepth);
	filename += ".txt";
	return filename;
}

string  getOffLineModelNmae(unsigned int uiDepth, ModelType modelType, FeatureType featureType){
	string filename = "model";
	filename += "_";
	filename += to_string(uiDepth);
	filename += "_";
	filename += ModelType_Names[modelType];
	filename += "_";
	filename += FeatureType_Names[featureType];
	return filename;
}

CurrentState getCurrentState(unsigned int uiDepth){
	if (g_iPOC%g_iP[uiDepth] < g_iT[uiDepth])	return Training;
	else if (g_iPOC%g_iP[uiDepth] >= g_iT[uiDepth] && g_iPOC%g_iP[uiDepth] < g_iT[uiDepth] + g_iV[uiDepth])	return Verifying;
	else if (g_iPOC%g_iP[uiDepth] >= g_iT[uiDepth] + g_iV[uiDepth])	return Testing;
	else return Training;
}

template <class T> double getVerAccuracy(UInt uiDepth, ModelType modelType){
	T numTP = g_iVerResult[uiDepth][modelType][TP];
	T numTN = g_iVerResult[uiDepth][modelType][TN];
	T numFP = g_iVerResult[uiDepth][modelType][FP];
	T numFN = g_iVerResult[uiDepth][modelType][FN];
	if (numTP + numTN + numFN + numFP == 0) return 0;
	else return (Double)(numTP + numTN) / (numTP + numTN + numFN + numFP);
}
template <class T> double getSkipPrecision(UInt uiDepth, ModelType modelType){
	T numTP = g_iVerResult[uiDepth][modelType][TP];
	T numTN = g_iVerResult[uiDepth][modelType][TN];
	T numFP = g_iVerResult[uiDepth][modelType][FP];
	T numFN = g_iVerResult[uiDepth][modelType][FN];
	if (numTP + numFP == 0) return 0;
	else return(Double)(numTP) / (numTP + numFP);
}
template <class T> double getTermPrecision(UInt uiDepth, ModelType modelType){
	T numTP = g_iVerResult[uiDepth][modelType][TP];
	T numTN = g_iVerResult[uiDepth][modelType][TN];
	T numFP = g_iVerResult[uiDepth][modelType][FP];
	T numFN = g_iVerResult[uiDepth][modelType][FN];
	if ((numTN + numFN) == 0) return 0;
	else return(double)(numTN) / (numTN + numFN);
}
template <class T> double getSkipSensitivity(UInt uiDepth, ModelType modelType){
	T numTP = g_iVerResult[uiDepth][modelType][TP];
	T numTN = g_iVerResult[uiDepth][modelType][TN];
	T numFP = g_iVerResult[uiDepth][modelType][FP];
	T numFN = g_iVerResult[uiDepth][modelType][FN];
	if ((numTP + numFN) == 0) return 0;
	else return(double)(numTP) / (numTP + numFN);
}
template <class T> double getTermSensitivity(UInt uiDepth, ModelType modelType){
	T numTP = g_iVerResult[uiDepth][modelType][TP];
	T numTN = g_iVerResult[uiDepth][modelType][TN];
	T numFP = g_iVerResult[uiDepth][modelType][FP];
	T numFN = g_iVerResult[uiDepth][modelType][FN];
	if ((numTN + numFP) == 0) return 0;
	else return(double)(numTN) / (numTN + numFP);
}

/////////////// CDM  CU decision map 
int LoadCDM(string fileName, cv::Mat& matData, int dim, const int* size, int matChns)
{
	int retVal = 0;

	// 打开文件  
	ifstream inFile(fileName.c_str(), ios_base::in);
	if (!inFile.is_open())
	{
		cout << "读取文件失败" << endl;
		retVal = -1;
		return (retVal);
	}

	// 载入数据  
	istream_iterator<float> begin(inFile);    //按 float 格式取文件数据流的起始指针  
	istream_iterator<float> end;          //取文件流的终止位置  
	vector<float> inData(begin, end);      //将文件数据保存至 std::vector 中  
	cv::Mat tmpMat = cv::Mat(inData);       //将数据由 std::vector 转换为 cv::Mat  

	// 输出到命令行窗口  
	//copy(vec.begin(),vec.end(),ostream_iterator<double>(cout,"\t"));   

	// 检查设定的矩阵尺寸和通道数  
	size_t dataLength = inData.size();

	// 将文件数据保存至输出矩阵  
	//	matData = tmpMat.reshape(matChns, dim, size).clone();


	int I = size[0];
	int Y = size[1];
	int X = size[2];


	for (int i = 0; i<I; i++)
	{
		for (int y = 0; y<Y; y++)
		{
			for (int x = 0; x<X; x++)
			{
				matData.at<float>(i, y, x) = tmpMat.at<float>(i*X*Y + y*X + x);
			}
		}
	}
	//matData = tmpMat.clone();
	return (retVal);
}
int WriteCDM(string fileName, cv::Mat& matData, int dim, const int* size)
{
	int retVal = 0;

	// 打开文件  
	ofstream outFile;//(fileName.c_str(), ios_base::out);  //按新建或覆盖方式写入 
	outFile.open(fileName.c_str());
	if (!outFile.is_open())
	{
		cout << "打开文件失败" << endl;
		retVal = -1;
		return (retVal);
	}

	// 检查矩阵是否为空  
	if (matData.empty())
	{
		cout << "矩阵为空" << endl;
		retVal = 1;
		return (retVal);
	}

	int data_length = 1;
	for (int i = 0; i < dim; i++)
	{
		data_length *= size[i];
	}


	int I = size[0];
	int Y = size[1];
	int X = size[2];


	for (int i = 0; i<I; i++)
	{
		for (int y = 0; y<Y; y++)
		{
			for (int x = 0; x<X; x++)
			{
				outFile << matData.at<float>(i, y, x) << "\t";
			}
			outFile << endl;
		}
		outFile << endl;
	}

	outFile.close();
	return (retVal);
}
int Test_3DMat()
{
	int I = 10;  //POC
	int Y = 5;   //Y
	int X = 8;   // X
	int size[] = { I, Y, X };
	int size_l = sizeof(size) / sizeof(*size);
	MatND* mXOrigin = new MatND(size_l, size, CV_32FC1);
	MatND* mXOrigin_read = new MatND(size_l, size, CV_32FC1);

	float c = 0;

	for (int i = 0; i<I; i++)
	{
		for (int y = 0; y<Y; y++)
		{
			for (int x = 0; x<X; x++)
			{
				mXOrigin->at<float>(i, y, x) = c;
				c = c + 1;
			}
		}
	}

	WriteCDM("CDM.txt", *mXOrigin, size_l, &size[0]);

	LoadCDM("CDM.txt", *mXOrigin_read, size_l, &size[0], 1);


	for (int i = 0; i<I; i++)
	{
		for (int y = 0; y<Y; y++)
		{
			for (int x = 0; x<X; x++)
			{
				cout << mXOrigin_read->at<float>(i, y, x) << "\t";
			}
			cout << endl;
		}
		cout << endl;
	}
	delete mXOrigin;
	mXOrigin = NULL;

	delete mXOrigin_read;
	mXOrigin_read = NULL;

	return 1;
}
/////////////////////////////////
/// basic_tools
///////////////////////////////////

int WriteData(string fileName, cv::Mat& matData)
{
	int retVal = 0;

	// 打开文件  
	ofstream outFile;//(fileName.c_str(), ios_base::out);  //按新建或覆盖方式写入 
	outFile.open(fileName.c_str());
	if (!outFile.is_open())
	{
		cout << "打开文件失败" << endl;
		retVal = -1;
		return (retVal);
	}

	// 检查矩阵是否为空  
	if (matData.empty())
	{
		cout << "矩阵为空" << endl;
		retVal = 1;
		return (retVal);
	}

	// 写入数据  
	for (int r = 0; r < matData.rows; r++)
	{
		for (int c = 0; c < matData.cols; c++)
		{
			double data = matData.at<double>(r, c);  //读取数据，at<type> - type 是矩阵元素的具体数据格式  
			outFile << data << "\t";   //每列数据用 tab 隔开  
		}
		outFile << endl;  //换行  
	}
	outFile.close();
	return (retVal);
}
int LoadData(string fileName, cv::Mat& matData, int matRows = 0, int matCols = 0, int matChns = 0)
{
	int retVal = 0;

	// 打开文件  
	ifstream inFile(fileName.c_str(), ios_base::in);
	if (!inFile.is_open())
	{
		cout << "读取文件失败" << endl;
		retVal = -1;
		return (retVal);
	}

	// 载入数据  
	istream_iterator<float> begin(inFile);    //按 float 格式取文件数据流的起始指针  
	istream_iterator<float> end;          //取文件流的终止位置  
	vector<float> inData(begin, end);      //将文件数据保存至 std::vector 中  
	cv::Mat tmpMat = cv::Mat(inData);       //将数据由 std::vector 转换为 cv::Mat  

	// 输出到命令行窗口  
	//copy(vec.begin(),vec.end(),ostream_iterator<double>(cout,"\t"));   

	// 检查设定的矩阵尺寸和通道数  
	size_t dataLength = inData.size();
	//1.通道数  
	if (matChns == 0)
	{
		matChns = 1;
	}
	//2.行列数  
	if (matRows != 0 && matCols == 0)
	{
		matCols = dataLength / matChns / matRows;
	}
	else if (matCols != 0 && matRows == 0)
	{
		matRows = dataLength / matChns / matCols;
	}
	else if (matCols == 0 && matRows == 0)
	{
		matRows = dataLength / matChns;
		matCols = 1;
	}
	//3.数据总长度  
	if (dataLength != (matRows * matCols * matChns))
	{
		cout << "读入的数据长度 不满足 设定的矩阵尺寸与通道数要求，将按默认方式输出矩阵！" << endl;
		retVal = 1;
		matChns = 1;
		matRows = dataLength;
	}

	// 将文件数据保存至输出矩阵  
	matData = tmpMat.reshape(matChns, matRows).clone();

	return (retVal);
}
//file
template <typename T> void create3dFileArray(T***& array, int d_1, int d_2, int d_3){
	array = new T**[d_1];
	for (int i = 0; i < d_1; i++){
		array[i] = new T*[d_2];
		for (int j = 0; j < d_2; j++){
			array[i][j] = new T[d_3];
		}
	}
}


template <typename T> void create2dFileArray(T**& array, int d_1, int d_2){
	array = new T*[d_1];
	for (int i = 0; i < d_1; i++){
		array[i] = new T[d_2];
	}
}






//3D
template <typename T> void create3dArray(T***& array, int d_1, int d_2, int d_3){
	array = new T**[d_1];
	for (int i = 0; i < d_1; i++){
		array[i] = new T*[d_2];
		for (int j = 0; j < d_2; j++){
			array[i][j] = new T[d_3];
			for (int k = 0; k < d_3; k++)array[i][j][k] = 0;
		}
	}
}
//template void create3dArray(double***& array, int d_1, int d_2, int d_3);
//template void create3dArray(bool***& array, int d_1, int d_2, int d_3);
template <typename T> void clear3dArray(T***& array, int d_1, int d_2, int d_3){
	for (int i = 0; i < d_1; i++){
		for (int j = 0; j < d_2; j++){
			for (int k = 0; k < d_3; k++) array[i][j][k] = 0;
		}
	}
}

template <typename T> void delete3dArray(T***& array, int d_1, int d_2, int d_3){
	for (int i = 0; i < d_1; i++){
		for (int j = 0; j < d_2; j++)delete[] array[i][j];
		delete[] array[i];
	}
	delete[] array;
	array = NULL;
}
template void delete3dArray(double***& array, int d_1, int d_2, int d_3);
template void delete3dArray(bool***& array, int d_1, int d_2, int d_3);


// 2D
template <typename T> void create2dArray(T**& array, int d_1, int d_2){
	array = new T*[d_1];
	for (int i = 0; i < d_1; i++){
		array[i] = new T[d_2];
		for (int j = 0; j < d_2; j++) array[i][j] = 0;

	}
}

template <typename T> void clear2dArray(T**& array, int d_1, int d_2){
	for (int i = 0; i < d_1; i++){
		for (int j = 0; j < d_2; j++)array[i][j] = 0;
	}
}
template void clear2dArray(double**& array, int d_1, int d_2);
template void clear2dArray(bool**& array, int d_1, int d_2);

template <typename T> void delete2dArray(T**& array, int d_1, int d_2){
	for (int i = 0; i < d_1; i++)delete[] array[i];
	delete[] array;
	array = NULL;
}

template <typename T> void print3dArray(T*** array, int d_1, int d_2, int d_3){
	for (int i = 0; i < d_1; i++){
		for (int j = 0; j < d_2; j++){
			for (int k = 0; k < d_3; k++)cout << array[i][j][k];

			cout << endl;
		}
		cout << endl;
	}
}
template void print3dArray(double*** array, int d_1, int d_2, int d_3);
template void print3dArray(bool*** array, int d_1, int d_2, int d_3);
template <typename T> void setArray(T*& array, int d_1, T value){
	for (int i = 0; i < d_1; i++)array[i] = value;
}

Int findPos(Double *data, Int right, Double tgt){
	Int left = 0;
	while (left <= right) {
		Int mid = left + (right - left) / 2;
		if ((mid - 1 >= 0 && data[mid] > tgt && data[mid - 1] <= tgt) || (mid == 0 && data[mid] > tgt)){
			return mid;
		}
		else if (data[mid] >= tgt){
			right = mid - 1;
		}
		else{
			left = mid + 1;
		}
	}
	return left;
}


TMVFeature* getTMVFeature(TComDataCU*& rpcBestCU) {
	UInt uiLPelX = rpcBestCU->getCUPelX();
	UInt uiTPelY = rpcBestCU->getCUPelY();
	TComPicYuv* dataOrg = rpcBestCU->getPic()->getPicYuvOrg();
	UInt uiStrideOrg = dataOrg->getStride(COMPONENT_Y);
	TMVFeature* feature_x = new TMVFeature;

	UInt uiCuWidth = rpcBestCU->getWidth(0);
	UInt uiCuHeight = rpcBestCU->getHeight(0);

	Int aiHorMask[9] = { 0, 0, 0, 1, 0, -1, 0, 0, 0 };
	Int aiVerMask[9] = { 0, 1, 0, 0, 0, 0, 0, -1, 0 };
	Int aiDiagMask[9] = { 0, 0, 1, 0, 0, 0, -1, 0, 0 };
	Int aiAntDMask[9] = { 1, 0, 0, 0, 0, 0, 0, 0, -1 };
	T3x3Filter cHorFilt(aiHorMask);
	T3x3Filter cVerFilt(aiVerMask);
	T3x3Filter cDiagFilt(aiDiagMask);
	T3x3Filter cAntDFilt(aiAntDMask);

	Pel* ppCuOrg = dataOrg->getAddr(COMPONENT_Y) + uiLPelX + uiTPelY * uiStrideOrg;
	Pel** pppFilt = (Pel**)xMalloc(Pel*, 4);
	// 0: Horizontal, 1: Vertical, 2: Diagonal, 3: Anti-Diagonal

	pppFilt[0] = cHorFilt.filter(ppCuOrg, uiCuWidth, uiCuHeight, uiStrideOrg);
	pppFilt[1] = cVerFilt.filter(ppCuOrg, uiCuWidth, uiCuHeight, uiStrideOrg);
	pppFilt[2] = cDiagFilt.filter(ppCuOrg, uiCuWidth, uiCuHeight, uiStrideOrg);
	pppFilt[3] = cAntDFilt.filter(ppCuOrg, uiCuWidth, uiCuHeight, uiStrideOrg);

	for (int iFiltIdx = 0; iFiltIdx < 4; iFiltIdx++) {
		Double iMean;
		Double iVariance;
		Double* piDst = feature_x->m_adFeature[iFiltIdx];
		Pel* piSrc = pppFilt[iFiltIdx];
		// whole
		iMean = TMVFeature::getSubBlockMean(piSrc, uiCuWidth, 0, uiCuWidth, 0, uiCuWidth);
		piDst[0] = iMean;
		piDst[1] = TMVFeature::getSubBlockVariance(piSrc, iMean, uiCuWidth, 0, uiCuWidth, 0, uiCuWidth);
		// horizontal
		iMean = TMVFeature::getSubBlockMean(piSrc, uiCuWidth, 0, uiCuWidth >> 1, 0, uiCuWidth);
		piDst[2] = iMean;
		piDst[4] = TMVFeature::getSubBlockVariance(piSrc, iMean, uiCuWidth, 0, uiCuWidth >> 1, 0, uiCuWidth);

		iMean = TMVFeature::getSubBlockMean(piSrc, uiCuWidth, uiCuWidth >> 1, uiCuWidth, 0, uiCuWidth);
		piDst[3] = iMean;
		piDst[5] = TMVFeature::getSubBlockVariance(piSrc, iMean, uiCuWidth, uiCuWidth >> 1, uiCuWidth, 0, uiCuWidth);
		// vertical
		iMean = TMVFeature::getSubBlockMean(piSrc, uiCuWidth, 0, uiCuWidth, 0, uiCuWidth >> 1);
		piDst[6] = iMean;
		piDst[8] = TMVFeature::getSubBlockVariance(piSrc, iMean, uiCuWidth, 0, uiCuWidth, 0, uiCuWidth >> 1);

		iMean = TMVFeature::getSubBlockMean(piSrc, uiCuWidth, 0, uiCuWidth, uiCuWidth >> 1, uiCuWidth);
		piDst[7] = iMean;
		piDst[9] = TMVFeature::getSubBlockVariance(piSrc, iMean, uiCuWidth, 0, uiCuWidth, uiCuWidth >> 1, uiCuWidth);
		// Diagonal
		iMean = 0;
		for (Int i = 0; i < uiCuWidth; i++) {
			for (Int j = 0; j < uiCuWidth - i; j++) {
				iMean += piSrc[i * uiCuWidth + j];
			}
		}
		iMean /= (uiCuWidth * uiCuWidth >> 1);
		piDst[10] = iMean;
		iVariance = 0;
		for (Int i = 0; i < uiCuWidth; i++) {
			for (Int j = 0; j < uiCuWidth - i; j++) {
				Int tmp = piSrc[i * uiCuWidth + j] - iMean;
				iVariance = tmp > 0 ? tmp : -tmp;
			}
		}
		iVariance /= (uiCuWidth * uiCuWidth >> 1);
		piDst[12] = iVariance;

		iMean = 0;
		for (Int i = 0; i < uiCuWidth; i++) {
			for (Int j = uiCuWidth - i - 1; j < uiCuWidth; j++) {
				iMean += piSrc[i * uiCuWidth + j];
			}
		}
		iMean /= (uiCuWidth * uiCuWidth >> 1);
		piDst[11] = iMean;
		iVariance = 0;
		for (Int i = 0; i < uiCuWidth; i++) {
			for (Int j = uiCuWidth - i - 1; j < uiCuWidth; j++) {
				Int tmp = piSrc[i * uiCuWidth + j] - iMean;
				iVariance = tmp > 0 ? tmp : -tmp;
			}
		}
		iVariance /= (uiCuWidth * uiCuWidth >> 1);
		piDst[13] = iVariance;

		// Anti-Diagonal
		iMean = 0;
		for (Int i = 0; i < uiCuWidth; i++) {
			for (Int j = i; j < uiCuWidth; j++) {
				iMean += piSrc[i * uiCuWidth + j];
			}
		}
		iMean /= (uiCuWidth * uiCuWidth >> 1);
		piDst[14] = iMean;
		iVariance = 0;
		for (Int i = 0; i < uiCuWidth; i++) {
			for (Int j = i; j < uiCuWidth; j++) {
				Int tmp = piSrc[i * uiCuWidth + j] - iMean;
				iVariance = tmp > 0 ? tmp : -tmp;
			}
		}
		iVariance /= (uiCuWidth * uiCuWidth >> 1);
		piDst[16] = iVariance;

		iMean = 0;
		for (Int i = 0; i < uiCuWidth; i++) {
			for (Int j = 0; j < i + 1; j++) {
				iMean += piSrc[i * uiCuWidth + j];
			}
		}
		iMean /= (uiCuWidth * uiCuWidth >> 1);
		piDst[15] = iMean;
		iVariance = 0;
		for (Int i = 0; i < uiCuWidth; i++) {
			for (Int j = 0; j < i + 1; j++) {
				Int tmp = piSrc[i * uiCuWidth + j] - iMean;
				iVariance = tmp > 0 ? tmp : -tmp;
			}
		}
		iVariance /= (uiCuWidth * uiCuWidth >> 1);
		piDst[17] = iVariance;

		// Quadratic
		iMean = TMVFeature::getSubBlockMean(piSrc, uiCuWidth, 0, uiCuWidth >> 1, 0, uiCuWidth >> 1);
		piDst[18] = iMean;
		piDst[22] = TMVFeature::getSubBlockVariance(piSrc, iMean, uiCuWidth, 0, uiCuWidth >> 1, 0, uiCuWidth >> 1);

		iMean = TMVFeature::getSubBlockMean(piSrc, uiCuWidth, 0, uiCuWidth >> 1, uiCuWidth >> 1, uiCuWidth);
		piDst[19] = iMean;
		piDst[23] = TMVFeature::getSubBlockVariance(piSrc, iMean, uiCuWidth, 0, uiCuWidth >> 1, uiCuWidth >> 1, uiCuWidth);

		iMean = TMVFeature::getSubBlockMean(piSrc, uiCuWidth, uiCuWidth >> 1, uiCuWidth, 0, uiCuWidth);
		piDst[20] = iMean;
		piDst[24] = TMVFeature::getSubBlockVariance(piSrc, iMean, uiCuWidth, uiCuWidth >> 1, uiCuWidth, 0, uiCuWidth);

		iMean = TMVFeature::getSubBlockMean(piSrc, uiCuWidth, uiCuWidth >> 1, uiCuWidth, uiCuWidth >> 1, uiCuWidth);
		piDst[21] = iMean;
		piDst[25] = TMVFeature::getSubBlockVariance(piSrc, iMean, uiCuWidth, uiCuWidth >> 1, uiCuWidth, uiCuWidth >> 1, uiCuWidth);
	}


	for (int idx = 0; idx < 4; idx++) {
		xFree(pppFilt[idx]);
	}

	xFree(pppFilt);

	return feature_x;
}