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
/** \file     encmain.cpp
    \brief    Encoder application main
*/
#include <time.h>
#include <iostream>
#include "TAppEncTop.h"
#include "TAppCommon/program_options_lite.h"
#include "TLibCommon/linear.h"
#include "TLibCommon/tools_YS.h"

using namespace cv;
//! \ingroup TAppEncoder
//! \{
#if  ENABLE_YS_GLOBAL  // declare the g_fileBayesModel and g_filePerformance
ofstream g_fileBayesModel;
ofstream g_filePerformance;
#endif

#if OUT_OUTLIER  // create ofstream
ofstream OutlierYuvFile;
ofstream OBFFile;
#endif

#include "../Lib/TLibCommon/Debug.h"


#if CREAT_CDM
MatND** MatCDM;
#endif 



// ====================================================================================================================
// Main function
// ====================================================================================================================
#if USE_TEST_MAIN
int main(int argc, char* argv[])
{
	Test_3DMat();
	system("pause");

	return 0;
}
#else


int main(int argc, char* argv[])
{

#if CREAT_CDM
	MatCDM = new MatND*[4];
#endif 

#if  SAVE_ENCODING_RESULT
  FILE* pFileET = fopen("summaryTotal.txt", "at");
  FILE* pFile_Analysis = fopen("Analysis.txt", "at");
#endif	


#if TIME_SYSTEM  // reset
  resetTimeSystem();
#endif




 TAppEncTop  cTAppEncTop;
  // print information
  fprintf( stdout, "\n" );
  fprintf( stdout, "HM software: Encoder Version [%s] (including RExt)", NV_VERSION );
  fprintf( stdout, NVM_ONOS );
  fprintf( stdout, NVM_COMPILEDBY );
  fprintf( stdout, NVM_BITS );
  fprintf( stdout, "\n\n" );

  // create application encoder class
  cTAppEncTop.create();

  // parse configuration
  try
  {
    if(!cTAppEncTop.parseCfg( argc, argv ))  // parse cfg
    {
      cTAppEncTop.destroy();
#if ENVIRONMENT_VARIABLE_DEBUG_AND_TEST
      EnvVar::printEnvVar();
#endif
      return 1;
    }
  }
  catch (df::program_options_lite::ParseFailure &e)
  {
    std::cerr << "Error parsing option \""<< e.arg <<"\" with argument \""<< e.val <<"\"." << std::endl;
    return 1;
  }

#if PRINT_MACRO_VALUES
  printMacroSettings();
#endif

#if ENVIRONMENT_VARIABLE_DEBUG_AND_TEST
  EnvVar::printEnvVarInUse();
#endif
  createYSGlobal();
  YSGlobalControl();
  createStat();
  openOutputFiles();

#if ENABLE_YS_GLOBAL     //open files
  string FileName_g_fileBayesModel = "BayesModel.txt";
  g_fileBayesModel.open(FileName_g_fileBayesModel.c_str());

  string FileName_g_filePerformance = "Performance.txt";
  g_filePerformance.open(FileName_g_filePerformance.c_str());

#endif

#if OUT_OUTLIER
  string FileYuvName = "Outlier.yuv";
  OutlierYuvFile.open(FileYuvName.c_str());
  string FileOBFName = "OBF.yuv";
  OBFFile.open(FileOBFName.c_str());
#endif

  // starting time
  Double dResult;
  clock_t lBefore = clock();

  // call encoding function
  cTAppEncTop.encode();

  // ending time
  dResult = (Double)(clock()-lBefore) / CLOCKS_PER_SEC;
  g_dET = dResult;
  printf("\n Total Time: %12.3f sec.\n", dResult);

#if TIME_SYSTEM   // print 
g_dTime_Total = dResult;
printTimeSystem();
#endif


#if  SAVE_ENCODING_RESULT
  fprintf(pFileET,"\t%f\n", dResult);
#if OBSERVATION
  for (int i = 0; i < 4; i++)
  {
	  fprintf(pFile_Analysis, "Depth:%d\t\t", i);
  }
  fprintf(pFile_Analysis, "\n");
#endif
  fclose(pFile_Analysis);
  fclose(pFileET);
#endif




  // destroy application encoder class
  cTAppEncTop.destroy();

#if OUT_OUTLIER  //close Files
  OutlierYuvFile.close();
  OBFFile.close();
#endif

#if ENABLE_YS_GLOBAL  //   g_fileBayesModel.close();
  g_fileBayesModel.close();
  closeOutputFiles();
#endif

#if CREAT_CDM
  for (int i = 0; i < 4; i++)
  {
	  delete[] MatCDM[i];
	  //MatCDM[i] = NULL;
  }
  delete[] MatCDM;
#endif 




deleteYSGlobal();





#if OBSERVATION
 system("pause");
#endif

  return 0;
}

#endif
