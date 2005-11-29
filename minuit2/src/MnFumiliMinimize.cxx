// @(#)root/minuit2:$Name:  $:$Id: MnFumiliMinimize.cpp,v 1.1.4.4 2005/11/29 11:08:35 moneta Exp $
// Authors: M. Winkler, F. James, L. Moneta, A. Zsenei   2003-2005  

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2005 LCG ROOT Math team,  CERN/PH-SFT                *
 *                                                                    *
 **********************************************************************/

#include "Minuit2/MnFumiliMinimize.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/FumiliMinimizer.h"

namespace ROOT {

   namespace Minuit2 {



// need to reimplement otherwise base class method is done

FunctionMinimum MnFumiliMinimize::operator()(unsigned int maxfcn, double toler) {

  assert(fState.IsValid());
  unsigned int npar = VariableParameters();
//   assert(npar > 0);
  if(maxfcn == 0) maxfcn = 200 + 100*npar + 5*npar*npar;
  FunctionMinimum min = Minimizer().Minimize( Fcnbase(), fState, fStrategy, maxfcn, toler);
  fNumCall += min.NFcn();
  fState = min.UserState();
  return min;
}

  }  // namespace Minuit2

}  // namespace ROOT
