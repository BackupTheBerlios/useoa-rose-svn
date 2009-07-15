#include "MemSage2OA.h"
#include "SageOACallGraph.h"
#include "common.h"

#define ROSE_0_8_9a      

using namespace std;
using namespace UseOA;
using namespace SageInterface;

bool debug = false;

bool is_lval(OA::OA_ptr<OA::MemRefExpr> mre);

void
SageIRMemRefIterator::create(OA::StmtHandle h)
{
  SgNode *node = mIR.getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  // if haven't already determined the set of memrefs for the program
  // then call initMemRefMaps
  if (mIR.mStmt2allMemRefsMap.empty() ) {
      //mIR.initMemRefAndPtrAssignMaps();
  }

  // loop through MemRefHandle's for this statement and for now put them
  // into our own list
  std::set<OA::MemRefHandle>::iterator setIter;
  for (setIter=mIR.mStmt2allMemRefsMap[h].begin();
       setIter!=mIR.mStmt2allMemRefsMap[h].end(); setIter++)
    {
      mMemRefList.push_back(*setIter);
    }

}

SageMemRefHandleIterator::SageMemRefHandleIterator (OA::OA_ptr<std::list<OA::MemRefHandle> > pList)
    : OA::IRHandleListIterator<OA::MemRefHandle>(pList)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_MemSage2OA:ALL", debug);
}


void SageIRInterface::initMemRefAndPtrAssignMaps() {
}

