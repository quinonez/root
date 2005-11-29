// @(#)root/minuit2:$Name:  $:$Id: MnSeedGenerator.cpp,v 1.12.2.4 2005/11/29 11:08:35 moneta Exp $
// Authors: M. Winkler, F. James, L. Moneta, A. Zsenei   2003-2005  

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2005 LCG ROOT Math team,  CERN/PH-SFT                *
 *                                                                    *
 **********************************************************************/

#include "Minuit2/MnSeedGenerator.h"
#include "Minuit2/MinimumSeed.h"
#include "Minuit2/MnFcn.h"
#include "Minuit2/GradientCalculator.h"
#include "Minuit2/InitialGradientCalculator.h"
#include "Minuit2/MnUserTransformation.h"
#include "Minuit2/MinimumParameters.h"
#include "Minuit2/FunctionGradient.h"
#include "Minuit2/MinimumError.h"
#include "Minuit2/MnMatrix.h"
#include "Minuit2/MnMachinePrecision.h"
#include "Minuit2/MinuitParameter.h"
#include "Minuit2/MnLineSearch.h"
#include "Minuit2/MnParabolaPoint.h"
#include "Minuit2/MinimumState.h"
#include "Minuit2/MnUserParameterState.h"
#include "Minuit2/MnStrategy.h"
#include "Minuit2/MnHesse.h"
#include "Minuit2/VariableMetricEDMEstimator.h"
#include "Minuit2/NegativeG2LineSearch.h"
#include "Minuit2/AnalyticalGradientCalculator.h"
#include "Minuit2/Numerical2PGradientCalculator.h"
#include "Minuit2/HessianGradientCalculator.h"
#include "Minuit2/MnPrint.h"

#include <math.h>

namespace ROOT {

   namespace Minuit2 {


MinimumSeed MnSeedGenerator::operator()(const MnFcn& fcn, const GradientCalculator& gc, const MnUserParameterState& st, const MnStrategy& stra) const {

  unsigned int n = st.VariableParameters();
  const MnMachinePrecision& prec = st.Precision();

  // initial starting values
  MnAlgebraicVector x(n);
  for(unsigned int i = 0; i < n; i++) x(i) = st.IntParameters()[i];
  double fcnmin = fcn(x);
  MinimumParameters pa(x, fcnmin);
  FunctionGradient dgrad = gc(pa);
  MnAlgebraicSymMatrix mat(n);
  double dcovar = 1.;
  if(st.HasCovariance()) {
    for(unsigned int i = 0; i < n; i++)	
      for(unsigned int j = i; j < n; j++) mat(i,j) = st.IntCovariance()(i,j);
    dcovar = 0.;
  } else {
    for(unsigned int i = 0; i < n; i++)	
      mat(i,i) = (fabs(dgrad.G2()(i)) > prec.Eps2() ? 1./dgrad.G2()(i) : 1.);
  }
  MinimumError err(mat, dcovar);
  double edm = VariableMetricEDMEstimator().Estimate(dgrad, err);
  MinimumState state(pa, err, dgrad, edm, fcn.NumOfCalls());

  NegativeG2LineSearch ng2ls;
  if(ng2ls.HasNegativeG2(dgrad, prec)) {
    state = ng2ls(fcn, state, gc, prec);
  }

  if(stra.Strategy() == 2 && !st.HasCovariance()) {
    //calculate full 2nd derivative
    MinimumState tmp = MnHesse(stra)(fcn, state, st.Trafo());
    return MinimumSeed(tmp, st.Trafo());
  }
  
  return MinimumSeed(state, st.Trafo());
}


MinimumSeed MnSeedGenerator::operator()(const MnFcn& fcn, const AnalyticalGradientCalculator& gc, const MnUserParameterState& st, const MnStrategy& stra) const {

  unsigned int n = st.VariableParameters();
  const MnMachinePrecision& prec = st.Precision();

  // initial starting values
  MnAlgebraicVector x(n);
  for(unsigned int i = 0; i < n; i++) x(i) = st.IntParameters()[i];
  double fcnmin = fcn(x);
  MinimumParameters pa(x, fcnmin);

  InitialGradientCalculator igc(fcn, st.Trafo(), stra);
  FunctionGradient tmp = igc(pa);
  FunctionGradient grd = gc(pa);
  FunctionGradient dgrad(grd.Grad(), tmp.G2(), tmp.Gstep());
  
  if(gc.CheckGradient()) {
    bool good = true;
    HessianGradientCalculator hgc(fcn, st.Trafo(), MnStrategy(2));
    std::pair<FunctionGradient, MnAlgebraicVector> hgrd = hgc.DeltaGradient(pa, dgrad);
    for(unsigned int i = 0; i < n; i++) {
      if(fabs(hgrd.first.Grad()(i) - grd.Grad()(i)) > hgrd.second(i)) {
	std::cout<<"gradient discrepancy of external Parameter "<<st.Trafo().ExtOfInt(i)<<" (internal Parameter "<<i<<") too large."<<std::endl;
	good = false;
      }
    }
    if(!good) {
      std::cout<<"Minuit does not accept user specified Gradient. To force acceptance, override 'virtual bool CheckGradient() const' of FCNGradientBase.h in the derived class."<<std::endl;
      assert(good);
    }
  }
  
  MnAlgebraicSymMatrix mat(n);
  double dcovar = 1.;
  if(st.HasCovariance()) {
    for(unsigned int i = 0; i < n; i++)	
      for(unsigned int j = i; j < n; j++) mat(i,j) = st.IntCovariance()(i,j);
    dcovar = 0.;
  } else {
    for(unsigned int i = 0; i < n; i++)	
      mat(i,i) = (fabs(dgrad.G2()(i)) > prec.Eps2() ? 1./dgrad.G2()(i) : 1.);
  }
  MinimumError err(mat, dcovar);
  double edm = VariableMetricEDMEstimator().Estimate(dgrad, err);
  MinimumState state(pa, err, dgrad, edm, fcn.NumOfCalls());

  NegativeG2LineSearch ng2ls;
  if(ng2ls.HasNegativeG2(dgrad, prec)) {
    Numerical2PGradientCalculator ngc(fcn, st.Trafo(), stra);
    state = ng2ls(fcn, state, ngc, prec);
  }

  if(stra.Strategy() == 2 && !st.HasCovariance()) {
    //calculate full 2nd derivative
    MinimumState tmp = MnHesse(stra)(fcn, state, st.Trafo());
    return MinimumSeed(tmp, st.Trafo());
  }
  
  return MinimumSeed(state, st.Trafo());
}

  }  // namespace Minuit2

}  // namespace ROOT
