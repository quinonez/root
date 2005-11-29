// @(#)root/minuit2:$Name:  $:$Id: FumiliMinimizer.cpp,v 1.7.2.4 2005/11/29 11:08:35 moneta Exp $
// Authors: M. Winkler, F. James, L. Moneta, A. Zsenei   2003-2005  

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2005 LCG ROOT Math team,  CERN/PH-SFT                *
 *                                                                    *
 **********************************************************************/

#include "Minuit2/MnConfig.h"
#include "Minuit2/FumiliMinimizer.h"
#include "Minuit2/MinimumSeedGenerator.h"
#include "Minuit2/FumiliGradientCalculator.h"
#include "Minuit2/Numerical2PGradientCalculator.h"
#include "Minuit2/AnalyticalGradientCalculator.h"
#include "Minuit2/MinimumBuilder.h"
#include "Minuit2/MinimumSeed.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnUserParameterState.h"
#include "Minuit2/MnUserParameters.h"
#include "Minuit2/MnUserTransformation.h"
#include "Minuit2/MnUserFcn.h"
#include "Minuit2/FumiliFCNBase.h"
#include "Minuit2/FCNGradientBase.h"
#include "Minuit2/MnStrategy.h"
#include "Minuit2/MnPrint.h"

namespace ROOT {

   namespace Minuit2 {


// for Fumili implement Minimize here because need downcast 


FunctionMinimum FumiliMinimizer::Minimize(const FCNBase& fcn, const MnUserParameterState& st, const MnStrategy& strategy, unsigned int maxfcn, double toler) const {

  MnUserFcn mfcn(fcn, st.Trafo());
  Numerical2PGradientCalculator gc(mfcn, st.Trafo(), strategy);

  unsigned int npar = st.VariableParameters();
  if(maxfcn == 0) maxfcn = 200 + 100*npar + 5*npar*npar;
  
  MinimumSeed mnseeds = SeedGenerator()(mfcn, gc, st, strategy);
  
  // downcast fcn 

  //std::cout << "FCN type " << typeid(&fcn).Name() << std::endl;

  FumiliFCNBase * fumiliFcn = dynamic_cast< FumiliFCNBase *>( const_cast<FCNBase *>(&fcn) ); 
  if ( !fumiliFcn ) { 
    std::cout <<"FumiliMinimizer: Error : wrong FCN type. Try to use default minimizer" << std::endl;
    return  FunctionMinimum(mnseeds, fcn.Up() );
  }
   

  FumiliGradientCalculator fgc(*fumiliFcn, st.Trafo(), npar);
#ifdef DEBUG
  std::cout << "Minuit::Minimize using FumiliMinimizer" << std::endl;
#endif 
  return ModularFunctionMinimizer::Minimize(mfcn, fgc, mnseeds, strategy, maxfcn, toler);
}


// use Gradient here 
FunctionMinimum FumiliMinimizer::Minimize(const FCNGradientBase& fcn, const MnUserParameterState& st, const MnStrategy& strategy, unsigned int maxfcn, double toler) const {

  // need MnUserFcn
  MnUserFcn mfcn(fcn, st.Trafo() );
  AnalyticalGradientCalculator gc(fcn, st.Trafo());

  unsigned int npar = st.VariableParameters();
  if(maxfcn == 0) maxfcn = 200 + 100*npar + 5*npar*npar;

  MinimumSeed mnseeds = SeedGenerator()(mfcn, gc, st, strategy);

  // downcast fcn 

  FumiliFCNBase * fumiliFcn = dynamic_cast< FumiliFCNBase *>( const_cast<FCNGradientBase *>(&fcn) ); 
  if ( !fumiliFcn ) { 
    std::cout <<"FumiliMinimizer: Error : wrong FCN type. Try to use default minimizer" << std::endl;
    return  FunctionMinimum(mnseeds, fcn.Up() );
  }
  

  FumiliGradientCalculator fgc(*fumiliFcn, st.Trafo(), npar);
#ifdef DEBUG
  std::cout << "Minuit::Minimize using FumiliMinimizer" << std::endl;
#endif
  return ModularFunctionMinimizer::Minimize(mfcn, fgc, mnseeds, strategy, maxfcn, toler);

}

  }  // namespace Minuit2

}  // namespace ROOT
