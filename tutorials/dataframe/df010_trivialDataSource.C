/// \file
/// \ingroup tutorial_dataframe
/// \notebook
/// This tutorial illustrates how use the RDataFrame in combination with a
/// RDataSource. In this case we use a TTrivialDS, which is nothing more
/// than a simple generator: it does not interface to any existing dataset.
/// The TTrivialDS has a single column, col0, which has value n for entry n.
///
/// \macro_code
///
/// \date September 2017
/// \author Danilo Piparo

int df010_trivialDataSource()
{
   auto nEvents = 128U;
   auto d_s = ROOT::RDF::MakeTrivialDataFrame(nEvents);

   /// Now we have a regular RDataFrame: the ingestion of data is delegated to
   /// the RDataSource. At this point everything works as before.
   auto h_s = d_s.Define("x", "1./(1. + col0)").Histo1D({"h_s", "h_s", 128, 0, .6}, "x");

   /// Now we redo the same with a RDF from scratch and we draw the two histograms
   ROOT::RDataFrame d(nEvents);

   /// This lambda redoes what the TTrivialDS provides
   auto g = []() {
      static ULong64_t i = 0;
      return i++;
   };
   auto h = d.Define("col0", g).Define("x", "1./(1. + col0)").Histo1D({"h", "h", 128, 0, .6}, "x");

   auto c_s = new TCanvas();
   c_s->SetLogy();
   h_s->DrawClone();

   auto c = new TCanvas();
   c->SetLogy();
   h->DrawClone();

   return 0;
}
