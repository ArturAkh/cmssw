import FWCore.ParameterSet.Config as cms

from Configuration.Geometry.GeometryDD4hep_cff import *
DDDetectorESProducer.confGeomXMLFiles = cms.FileInPath("Geometry/HcalCommonData/data/cmsExtendedGeometry2018.xml")

from Geometry.TrackerNumberingBuilder.trackerNumberingGeometry_cff import *
from Geometry.EcalCommonData.ecalSimulationParameters_cff import *
from Geometry.HcalCommonData.hcalDDDSimConstants_cff import *
from Geometry.MuonNumbering.muonGeometryConstants_cff import *
from Geometry.MuonNumbering.muonOffsetESProducer_cff import *
