#include "NoiseCaloCellsTool.h"

DECLARE_TOOL_FACTORY(NoiseCaloCellsTool)

NoiseCaloCellsTool::NoiseCaloCellsTool(const std::string& type,const std::string& name, const IInterface* parent)
  : GaudiTool(type, name, parent)
{
  declareInterface<INoiseCaloCellsTool>(this);
}

StatusCode NoiseCaloCellsTool::initialize() {
  StatusCode sc = GaudiTool::initialize();
  if (sc.isFailure()) return sc;

  //Initialize random service
  if(service( "RndmGenSvc" , m_randSvc ).isFailure()) {
    error() << "Couldn't get RndmGenSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  m_gauss.initialize(m_randSvc, Rndm::Gauss(0,m_cellNoise));

  info() << "Sigma of the cell noise: " <<  m_cellNoise * 1.e3 << " MeV" << endmsg;
  info() << "Filter noise threshold: " <<  m_filterThreshold << "*sigma" << endmsg;
  return sc;
}

void NoiseCaloCellsTool::createRandomCellNoise(std::vector<fcc::CaloHit*>& aCells) {
  double randomNoise = 0;
  for (auto& ecell : aCells) {
    randomNoise = m_gauss.shoot();
    ecell->core().energy = randomNoise;
    ecell->core().time = 0;
    ecell->core().bits = 0;
  }
}

void NoiseCaloCellsTool::filterCellNoise(std::vector<fcc::CaloHit*>& aCells) {
  //Erase a cell if it has energy bellow a threshold from the vector
  double threshold = m_filterThreshold * m_cellNoise;
  aCells.erase( std::remove_if( aCells.begin(),
      aCells.end(),
      [&threshold](fcc::CaloHit* h){return bool( h->core().energy < threshold);} ),
    aCells.end() );
}


StatusCode NoiseCaloCellsTool::finalize() {
  return GaudiTool::finalize();
}
