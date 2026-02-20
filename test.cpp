#include <StdAfx.h>

//=============================================================================
// COPYRIGHT KONINKLIJKE PHILIPS ELECTRONICS N.V. 2012
// All rights are reserved. Reproduction in whole or in part is
// prohibited without the written consent of the copyright owner.
//
//             Copyright (c) 2012 by Philips Ultrasound
//             Confidential - All Rights Reserved
//============================================================================

//--------------------------------------------------------------------------
// (1) Include the immediate header file
//--------------------------------------------------------------------------
#include <ulsi/ulsiExceptions.h>
#include <ulsi/ulsiTypes.h>
#include <Mapper.h>

//--------------------------------------------------------------------------
// (2) Include local/private headers for this project/domain
//--------------------------------------------------------------------------
//Location Of Interest subsystem
#include <loiLOI_3dSbTrimBoxObject.h>
#include <loiLOI_AcqRegObject.h>
#include <loiLOI_AttenImgRegObject.h>
#include <loiLOI_CursorObject.h>
#include <loiLOI_DopplerVolumObject.h>
#include <loiLOI_EchoRegObject.h>
#include <loiLOI_FVObject.h>
#include <loiLOI_ColorRoiRegObject.h>
#include <loiLOI_LfiObject.h>
#include <loiLOI_FocRegObject.h>
#include <loiLOI_LoiObject.h>
#include <loiLOI_LinearTrapPotSPObject.h>
#include <loiLOI_MlineObject.h>
#include <loiLOI_ElineObject.h>
#include <loiLOI_NormRegObject.h>
#include <loiLOI_OneDPotSPObject.h>
#include <loiLOI_POIObject.h>
#include <loiLOI_PotSPShapeObject.h>
#include <loiLOI_PotPlaneObject.h>
#include <loiLOI_LatPotPlaneObject.h>
#include <loiLOI_TranPotPlaneObject.h>
#include <loiLOI_RedRegObject.h>
#include <loiLOI_RotPotPlaneObject.h>
#include <loiLOI_LatSpecLoiObject.h>
#include <loiLOI_SPWObject.h>
#include <loiLOI_SVObject.h>
#include <loiLOI_ZoomRegObject.h>
#include <loiLOI_SectorPotSPObject.h>
#include <loiLOI_ElevLoiObject.h>
#include <loiLOI_ElevRegObject.h>
#include <loiLOI_LatLoiObject.h>
#include <loiLOI_InitializerObject.h>
#include <loiLOI_ActRegObject.h>
#include <loiLOI_GSObject.h>
#include <loiLOI_SecZoomObject.h>
#include <loiLOI_FvPotAreaObject.h>
#include <loiLOI_LfvAcqSpecObject.h>
#include <loiLOI_StrainElastoObject.h>
#include <loiLOI_ShearElastoObject.h>
#include <loiLOI_ShearwaveImagingObject.h>
#include <loiPERS_AcqRegObject.h>
#include <loiPERS_PotShapeObject.h>
#include <loiLOI_QFlowObject.h>



//--------------------------------------------------------------------------
// (3) Include public headers for this domain
//--------------------------------------------------------------------------
#include <ActiveSetIdentifiers.h>
//--------------------------------------------------------------------------
// (4) Include public headers for other domains
//--------------------------------------------------------------------------
#include <ulsiMC_AcqImgModeObject.h>
#include <ulsiMC_ColorTwoDDataObject.h>
#include <ulsiMC_ImageSessionObject.h>
#include <ulsiRPL_3dRpModeObject.h>
#include <ulsiDM_OccupiedPipeObject.h>
#include <ulsiMC_3DSessionObject.h>
#include <ulsiMC_ElastoObject.h>
#include <vizThreeDViz_BridgeBridge.h>
#include <vizThreeDVizBridgeBBridge.h>
#include <ulsimcInternBridge.h>
#include <ulsiEnums.h>
#include <ulsiMC_TwoDDataRegioObject.h>
#include <loiLOI_PotPlaneDepObject.h>

#include <MapperContexts.h>
// Messagebus writing
#include <CoreFinally.h>
#include <MbWriters.h>
#include <ImagingAppDisplayEcho2dPostProcess.h>
#include <ImagingAppAcquisitionLocationOfInterest.h>

//--------------------------------------------------------------------------
// (5) Include external, third-party headers
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// (6) Static definitions local to this file
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// (7) Use other namespaces (sparingly) and/or individual symbols from them
//--------------------------------------------------------------------------
// _VDB_USE
// _XUML_USE
// _ATL_USI_USE

//--------------------------------------------------------------------------
// (8) The rest, method/function definitions, etc.
//--------------------------------------------------------------------------

enum EActiveRegion
{
    SINGLETON_REGION = 0,
    ACTIVE_ACQUISITION,
    MLINE_ACQUISITION
} ;


// Forward declaration for the MB writer singleton defined in MapUsiToVdb.cpp
class MbWriters;
MbWriters* GetMbWriters();


void Mapper::loiActiveSets(ulsi_OccupiedSignalPathPipeline_c *pOccupiedPipeline, ulsi_AcquisitionImagingMode_c *
                           /*pActiveImagingMode*/, ulsi_ImagingModeSession_c * /*pActiveImagingSession*/)
{

    // The OccupiedSignalPathPipeline tree identified by *pOccupiedPipeline is selected for traversal.
    MAKE_SINGLE_ACTIVE(ulsi_, 0, OccupiedSignalPathPipeline, pOccupiedPipeline)

    //-------------------------------------------------------
    // LOI
    //-------------------------------------------------------
    MAKE_SELECTED_ACTIVES     (loi_, 0, ThreeDStandbyTrimBox,   PipelineId,         pOccupiedPipeline->m_PipelineId);

    // Primary Plane
    MAKE_SELECTED_ACTIVES_2_IDENTIFIERS    (loi_, 0,  PotentialPlane,  Name, ulsi_ELoiPotPlaneName_c::PRIMARY, PipelineId, pOccupiedPipeline->m_PipelineId);
    MAKE_SELECTED_ACTIVES                  (loi_, 0, PotentialPlaneDepth, PipelineId, pOccupiedPipeline->m_PipelineId);

    // Find the current scan plane shape by tracing it from the Primary Potential Plane
    MAKE_ACTIVE                            (loi_, 0, PotentialPlane,      R686,  0, PotentialPlaneShape);
    MAKE_ACTIVE                            (loi_, 0, PotentialPlaneShape, R8301, 0, PersistingPotentialPlaneShape);

    // Secondary Plane
    MAKE_SELECTED_ACTIVES_2_IDENTIFIERS    (loi_, 1,  PotentialPlane,  Name,   ulsi_ELoiPotPlaneName_c::SECONDARY, PipelineId, pOccupiedPipeline->m_PipelineId);
    MAKE_ACTIVE_CAST_TO_DERIVED            (loi_, 1,  PotentialPlane,     0,   LateralPotentialPlane);


    // echo active region
    MAKE_SELECTED_ACTIVES    (loi_, SINGLETON_REGION, ActiveRegion,    PipelineId,       pOccupiedPipeline->m_PipelineId);

    MAKE_ACTIVE              (loi_, SINGLETON_REGION, ActiveRegion,      R480,        0,              EchoRegion);
    MAKE_ACTIVE              (loi_, 0, EchoRegion,                R618,        0,              AcquisitionRegion);
    // Elevation region - used for biplane mappings
    MAKE_ACTIVE              (loi_, 0, AcquisitionRegion,         R694_Part,   694,            AcquisitionRegion);

    // Acquisition Zoom Region.  This is for the mapping for the graphic ROI box before entering the state where the
    // region is zoomed for acquisition.
    MAKE_SELECTED_ACTIVES    (loi_, 200, HighDefinitionZoomRegion,    PipelineId,          pOccupiedPipeline->m_PipelineId);
    MAKE_ACTIVE              (loi_, 200, HighDefinitionZoomRegion,    R610,       202,     EchoRegion);
    MAKE_ACTIVE              (loi_, 202, EchoRegion,                  R618,       202,     AcquisitionRegion);

    // Find the Zoom Region that is related to the Standby trim box - it will either be the HD Zoom or the Secondary Zoom
    MAKE_ACTIVE              (loi_, 0, ThreeDStandbyTrimBox,   R698,       206,            AcquisitionRegion);

    //Normal Region (holds parameters specifc to depth)
    MAKE_SELECTED_ACTIVES(loi_, 0, NormalRegion, PipelineId, pOccupiedPipeline->m_PipelineId);

    //Reduced Width Region (holds parameters specific to Sector Width
    MAKE_SELECTED_ACTIVES(loi_, 0, ReducedWidthRegion, PipelineId, pOccupiedPipeline->m_PipelineId);

    // Elevation zoom region - biplane mappings
    MAKE_ACTIVE              (loi_, 202, AcquisitionRegion,       R694_Part,      203,     AcquisitionRegion);

    //Focal Point and Focal Region. There is only one focal point and focal region that is active.  It is related
    //to each instance of echo region.  Trace from the active echo region to the focal region and focal point.
    // update for loi split rwc 12/01/2004
    MAKE_ACTIVE              (loi_, SINGLETON_REGION,   ActiveRegion,     R480,       800,        EchoRegion);
    MAKE_ACTIVE              (loi_, 800, EchoRegion,               R664,       800,        FocalRegion);
    MAKE_ACTIVE              (loi_, 800, FocalRegion,              R609,       800,        LocationofFocalInterest);


    //Color active region rwc 12/01/04
    MAKE_ACTIVE              (loi_, SINGLETON_REGION, ActiveRegion,          R497,       0,       ColorROIRegion);
    MAKE_ACTIVE_CAST_TO_DERIVED              (loi_, 0, ColorROIRegion,    1,       LateralSpecificLoi);
    MAKE_ACTIVE              (loi_, 1, LateralSpecificLoi,                R684,     1,       LateralPotentialPlane);
    MAKE_ACTIVE_CAST_TO_DERIVED (loi_, 1, LateralPotentialPlane,       2,       PotentialPlane);
    MAKE_ACTIVE              (loi_, 2, PotentialPlane,                R686,         2,       PotentialPlaneShape);


    MAKE_ACTIVE              (loi_, 0, ColorROIRegion,              R618,        1,         AcquisitionRegion);
    MAKE_ACTIVE(loi_, 1, AcquisitionRegion, R8300, 1, PersistingAcquisitionRegion);

    // Elevation color region - biplane mappings
    MAKE_ACTIVE              (loi_, 1, AcquisitionRegion,           R694_Part,  204,       AcquisitionRegion);

    // update for loi split rwc 12/01/2004
    MAKE_ACTIVE     (loi_, SINGLETON_REGION, ActiveRegion,        R697,       10,         AcquisitionCursor);
    MAKE_ACTIVE_CAST_TO_DERIVED      (loi_, 10, AcquisitionCursor,           0,        DopplerVolume);
    MAKE_ACTIVE_CAST_TO_DERIVED              (loi_, 0,  DopplerVolume,             0,         SampleVolume);
    MAKE_ACTIVE_CAST_TO_DERIVED              (loi_, 0,  DopplerVolume,              1,         FocalVolume);
    MAKE_ACTIVE_CAST_TO_DERIVED              (loi_, 0,  DopplerVolume,             2,         StaticPw);

    //___ Active Cursor Navigation __________________________________________________________________________________________________________________

    MAKE_ACTIVE      (loi_, SINGLETON_REGION, ActiveRegion,        R697,       20,        AcquisitionCursor);
    MAKE_ACTIVE_CAST_TO_DERIVED         (loi_, 20, AcquisitionCursor,                   110,        SampleVolume);

    MAKE_ACTIVE         (loi_, 110, SampleVolume,                 R615,       1,          PointOfInterest);
    MAKE_ACTIVES        (loi_, 1,  PointOfInterest,                  R684,       2,          LateralSpecificLoi);
    MAKE_ACTIVE         (loi_, 2,  LateralSpecificLoi,                R684,       2,          LateralPotentialPlane);
    MAKE_ACTIVE_CAST_TO_DERIVED         (loi_, 2,  LateralPotentialPlane,        3,          PotentialPlane);
    MAKE_ACTIVE         (loi_, 3,  PotentialPlane,                    R686,       3,          PotentialPlaneShape);
    //________________________________________________________________________________________________________________________________________________

    MAKE_ACTIVE     (loi_, SINGLETON_REGION, ActiveRegion,        R697,       30,        AcquisitionCursor);
    MAKE_ACTIVE_CAST_TO_DERIVED         (loi_, 30, AcquisitionCursor,                   210,        FocalVolume);
    MAKE_ACTIVE         (loi_, 210, FocalVolume,                 R615,       2,          PointOfInterest);
    MAKE_ACTIVES        (loi_, 2,  PointOfInterest,                   R684,       3,          LateralSpecificLoi);
    MAKE_ACTIVE         (loi_, 3,  LateralSpecificLoi,                R684,       3,          LateralPotentialPlane);
    MAKE_ACTIVE_CAST_TO_DERIVED         (loi_, 3,  LateralPotentialPlane,            4,          PotentialPlane);
    MAKE_ACTIVE         (loi_, 4,  PotentialPlane,                    R686,       4,          PotentialPlaneShape);

    MAKE_ACTIVE      (loi_, SINGLETON_REGION, ActiveRegion,        R697,       40,        AcquisitionCursor);
    MAKE_ACTIVE_CAST_TO_DERIVED         (loi_, 40, AcquisitionCursor,                   310,        StaticPw);
    MAKE_ACTIVE         (loi_, 310, StaticPw,                 R615,       3,          PointOfInterest);
    MAKE_ACTIVES        (loi_, 2,  PointOfInterest,    R684,       4,          LateralSpecificLoi);
    MAKE_ACTIVE         (loi_, 4,  LateralSpecificLoi,                R684,       4,          LateralPotentialPlane);
    MAKE_ACTIVE_CAST_TO_DERIVED         (loi_, 4,  LateralPotentialPlane,         5,          PotentialPlane);
    MAKE_ACTIVE         (loi_, 5,  PotentialPlane,                    R686,       5,          PotentialPlaneShape);

    MAKE_ACTIVE      (loi_, SINGLETON_REGION, ActiveRegion,        R697,       50,         AcquisitionCursor);
    MAKE_ACTIVE_CAST_TO_DERIVED     (loi_, 50,        AcquisitionCursor,               0,              Mline);
    MAKE_ACTIVE     (loi_, 0,         Mline,                      R643,       2,              AcquisitionRegion);
    // Echo ELine
    MAKE_SELECTED_ACTIVES           (loi_, 0, Eline,    PipelineId,      pOccupiedPipeline->m_PipelineId);

    //Elevation region
    MAKE_ACTIVE                     (loi_, SINGLETON_REGION, ActiveRegion,       R672,       1,      ElevationRegion);
    MAKE_ACTIVE                     (loi_, 1,         ElevationRegion,    R618,       11,     AcquisitionRegion);


    //Full volume potential area
    MAKE_SELECTED_ACTIVES(loi_, 0, FullVolumePotentialEchoArea, PipelineId, pOccupiedPipeline->m_PipelineId);

    // need the base class from ulsi to test which flavor of elastography the system is in.  Either instance will do.
    MAKE_SELECTED_ACTIVES_2_IDENTIFIERS (ulsi_, 0, Elastography,  ElastoID,   static_cast <PTC_CString_c>(L"STRAIN"), PipelineId, pOccupiedPipeline->m_PipelineId);

    // Strain Elastography region.
    MAKE_SELECTED_ACTIVES    (loi_, 400, StrainElastoRegion,         PipelineId,           pOccupiedPipeline->m_PipelineId);
    MAKE_ACTIVE              (loi_, 400, StrainElastoRegion,         R618,             402,   AcquisitionRegion);

    // Shear Elastography region.
    MAKE_SELECTED_ACTIVES    (loi_, 500, ShearElastoRegion,          PipelineId,            pOccupiedPipeline->m_PipelineId);
    MAKE_ACTIVE              (loi_, 500, ShearElastoRegion,          R618,             502,    AcquisitionRegion);

    // Shearwave Elastography region
    MAKE_SELECTED_ACTIVES    (loi_,   0, QFlowObject,                 PipelineId,            pOccupiedPipeline->m_PipelineId);

    MAKE_SELECTED_ACTIVES    (loi_, 600, ShearwaveImagingRegion,      PipelineId,            pOccupiedPipeline->m_PipelineId);
    MAKE_ACTIVE              (loi_, 600, ShearwaveImagingRegion,      R618,             602,    AcquisitionRegion);

    MAKE_SELECTED_ACTIVES    (loi_,   0, AttenImagingRegion,          PipelineId,            pOccupiedPipeline->m_PipelineId);
    // AttenImagingRegion does not link with any acquisition regions, so no separate MAKE_ACTIVE lines are needed.
}


void Mapper::mapLoiToVdb(int pset, int pipeIndex)
{
    auto ctx = BuildMappingContext(pipeIndex, true);

    // Scanplane shape
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlaneShape, Shape_VDB, 0, VDB_SYS_SCAN_PLANE_FORMAT);

    // Secondary plane (biplane)
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, PotentialPlane, TiltAngle_VDB, 1, VDB_2D_ECHO_TILT_STEER_POSITION_TARGET, MapSecondaryPlane(ctx));
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, PotentialPlane, RotationAngle_VDB, 1, VDB_2D_ECHO_MATRIX_PLANE_POSITION_TARGET, MapSecondaryPlane(ctx));
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, PotentialPlane, DisplayedRotationAngle_VDB, 1, VDB_ASP_ECHO_2D_MATRIX_PLANE_POSITION, MapSecondaryPlane(ctx));
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, PotentialPlane, biPlaneOperationalMode_VDB, 1, VDB_2D_ECHO_BIPLANE_OPERATING_MODE, MapSecondaryPlane(ctx));
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, PotentialPlane, TiltActive_VDB, 1, VDB_2D_ECHO_MC_TILT_ENABLED, MapSecondaryPlane(ctx));

    // Elevation acquisition region across R694 (biplane)
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 694, AcquisitionRegion, StartWidth_VDB, 1, VDB_2D_ECHO_START_WIDTH_USI_TARGET, MapSecondaryPlane(ctx));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 694, AcquisitionRegion, StopWidth_VDB, 1, VDB_2D_ECHO_STOP_WIDTH_USI_TARGET, MapSecondaryPlane(ctx));

    // Secondary plane zoom box (biplane)
    MAP_TO_VDB_IF(loi_, pipeIndex, 203, AcquisitionRegion, StartWidth_VDB, 1, VDB_ASP_ECHO_2D_ZOOM_BOX_START_WIDTH_TARGET, MapSecondaryPlane(ctx));
    MAP_TO_VDB_IF(loi_, pipeIndex, 203, AcquisitionRegion, StopWidth_VDB, 1, VDB_ASP_ECHO_2D_ZOOM_BOX_STOP_WIDTH_TARGET, MapSecondaryPlane(ctx));

    // Secondary plane color box (biplane + color singleton)
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 204, AcquisitionRegion, StartWidth_VDB, 1, VDB_COLOR_2D_START_WIDTH_TARGET, (MapSecondaryPlane(ctx) && ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 204, AcquisitionRegion, StopWidth_VDB, 1, VDB_COLOR_2D_STOP_WIDTH_TARGET, (MapSecondaryPlane(ctx) && ctx.m_isColorSingleton));

    // Primary plane (pset 0)
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, TiltAngle_VDB, 0, VDB_2D_ECHO_TILT_STEER_POSITION_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, gateDepth_VDB, 0, VDB_2D_ECHO_BIPLANE_DOPPLER_GATE_DEPTH);
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, gatelateralAngle_VDB, 0, VDB_2D_ECHO_BIPLANE_DOPPLER_GATE_LATERAL_ANGLE);
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, isxPlaneRotatedAndTilted_VDB, 0, VDB_2D_ECHO_IS_BIPLANE_ROTATED_TILTED);
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, RotationAngle_VDB, 0, VDB_2D_ECHO_MATRIX_PLANE_POSITION_TARGET);
    MAP_TO_VDB_OR_DEFAULT(loi_, pipeIndex, 0, PotentialPlane, teeButtonInPressedState_VDB, false, 0, VDB_2D_ECHO_MATRIX_TEE_BUTTON_PRESSED);
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, DisplayedRotationAngle_VDB, 0, VDB_ASP_ECHO_2D_MATRIX_PLANE_POSITION);
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, biPlaneOperationalMode_VDB, 0, VDB_2D_ECHO_BIPLANE_OPERATING_MODE);
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, TiltActive_VDB, 0, VDB_2D_ECHO_MC_TILT_ENABLED);

    // Primary plane color ROI (color singleton without elasto and not full-volume)
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 1, AcquisitionRegion, StartWidth_VDB, 0, VDB_COLOR_2D_START_WIDTH_TARGET, ColorWithoutElasto2DOnly(ctx));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 1, AcquisitionRegion, StopWidth_VDB, 0, VDB_COLOR_2D_STOP_WIDTH_TARGET, ColorWithoutElasto2DOnly(ctx));
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, PersistingAcquisitionRegion, StartWidth_VDB, 0, VDB_COLOR_2D_START_WIDTH_TARGET, (!ctx.m_elastoActive && !ctx.m_isMatrixFullVolumeMode));
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, PersistingAcquisitionRegion, StopWidth_VDB, 0, VDB_COLOR_2D_STOP_WIDTH_TARGET, (!ctx.m_elastoActive && !ctx.m_isMatrixFullVolumeMode));
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, PersistingAcquisitionRegion, StartDepth_VDB, 0, VDB_COLOR_2D_START_DEPTH_TARGET, (!ctx.m_elastoActive && !ctx.m_isMatrixFullVolumeMode));
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, PersistingAcquisitionRegion, StopDepth_VDB, 0, VDB_COLOR_2D_STOP_DEPTH_TARGET, (!ctx.m_elastoActive && !ctx.m_isMatrixFullVolumeMode));

    // Primary plane echo ROI (not full-volume)
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StartWidth_VDB, 0, VDB_2D_ECHO_START_WIDTH_USI_TARGET, !ctx.m_isMatrixFullVolumeMode);
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StopWidth_VDB, 0, VDB_2D_ECHO_STOP_WIDTH_USI_TARGET, !ctx.m_isMatrixFullVolumeMode);

    // Primary plane zoom box
    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion, StartWidth_VDB, 0, VDB_ASP_ECHO_2D_ZOOM_BOX_START_WIDTH_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion, StopWidth_VDB, 0, VDB_ASP_ECHO_2D_ZOOM_BOX_STOP_WIDTH_TARGET);

    // Color steering and scanplane (color singleton)
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, PotentialPlane, SteeringAngle_VDB, pset, VDB_COLOR_2D_LINEAR_STEERING_ANGLE_TARGET, ctx.m_isColorSingleton);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, PersistingPotentialPlaneShape, SteeringAngle_VDB, pset, VDB_COLOR_2D_LINEAR_STEERING_ANGLE_TARGET, ctx.m_isColorSingleton);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, PotentialPlaneShape, ColorPwShape_VDB, pset, VDB_COLOR_2D_SCAN_PLANE_FORMAT_TARGET, ctx.m_isColorSingleton);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, PersistingPotentialPlaneShape, ColorPwShape_VDB, pset, VDB_COLOR_2D_SCAN_PLANE_FORMAT_TARGET, ctx.m_isColorSingleton);

    // Cursor and zoom graphics
    MAP_TO_VDB(loi_, pipeIndex, 0, Eline, LateralPosition_VDB, pset, VDB_SYS_TWOD_CURSOR_LATERAL_POI_POSITION_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 0, Eline, AxialPosition_VDB, pset, VDB_SYS_TWOD_CURSOR_AXIAL_POI_POSITION_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 200, HighDefinitionZoomRegion, ZoomGraphicsDisplayed_VDB, pset, VDB_ASP_ECHO_2D_ZOOM_BOX_VISIBLE);

    // Echo region
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StartDepth_VDB, pset, VDB_2D_ECHO_START_DEPTH_TARGET, ctx.m_isLive);
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StopDepth_VDB, pset, VDB_2D_ECHO_STOP_DEPTH_TARGET, ctx.m_isLive);
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, EchoRegion, WideScreenEnabled_VDB, pset, VDB_2D_ECHO_SCT_WIDE_SCREEN_ENABLED);
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, EchoRegion, VrocEnabled_VDB, pset, VDB_2D_ECHO_VROC_IMAGING_ENABLED);

    // Unzoomed mappings based on zoom acquiring
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StartDepthUnzoomed_VDB, pset, VDB_2D_ECHO_START_DEPTH_UNZOOMED_TARGET, MapZoomAcquiring(ctx));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StopDepthUnzoomed_VDB, pset, VDB_2D_ECHO_STOP_DEPTH_UNZOOMED_TARGET, MapZoomAcquiring(ctx));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StartWidthUnzoomed_VDB, pset, VDB_2D_ECHO_START_WIDTH_UNZOOMED_TARGET, MapZoomAcquiring(ctx));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StopWidthUnzoomed_VDB, pset, VDB_2D_ECHO_STOP_WIDTH_UNZOOMED_TARGET, MapZoomAcquiring(ctx));

    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StartDepth_VDB, pset, VDB_2D_ECHO_START_DEPTH_UNZOOMED_TARGET, MapZoomNotAcquiring(ctx));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StopDepth_VDB, pset, VDB_2D_ECHO_STOP_DEPTH_UNZOOMED_TARGET, MapZoomNotAcquiring(ctx));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StartWidth_VDB, pset, VDB_2D_ECHO_START_WIDTH_UNZOOMED_TARGET, MapZoomNotAcquiring(ctx));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StopWidth_VDB, pset, VDB_2D_ECHO_STOP_WIDTH_UNZOOMED_TARGET, MapZoomNotAcquiring(ctx));

    // Card behavior: right image only style on biplane
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, EchoRegion, DisplayStyleAlt_VDB, 1, VDB_2D_ECHO_ECHO_REGION_DISPLAY_STYLE, MapCardBiplaneRightImageOnly(ctx, pset));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, EchoRegion, DisplayStyle_VDB, pset, VDB_2D_ECHO_ECHO_REGION_DISPLAY_STYLE, !MapCardBiplaneRightImageOnly(ctx, pset));

    // Zoom box depth (always mapped)
    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion, StartDepth_VDB, pset, VDB_ASP_ECHO_2D_ZOOM_BOX_START_DEPTH_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion, StopDepth_VDB, pset, VDB_ASP_ECHO_2D_ZOOM_BOX_STOP_DEPTH_TARGET);

    // Focal region
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 800, LocationofFocalInterest, Depth_VDB, pset, VDB_TWOD_FOCAL_POSITION_TARGET, ctx.m_isLive);
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 800, FocalRegion, StartDepth_VDB, pset, VDB_2D_ECHO_START_FOCAL_REGION_TARGET, ctx.m_isLive);
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 800, FocalRegion, StopDepth_VDB, pset, VDB_2D_ECHO_STOP_FOCAL_REGION_TARGET, ctx.m_isLive);

    // Doppler
    MAP_TO_VDB(loi_, pipeIndex, 3, PotentialPlane, SteeringAngle_VDB, pset, VDB_DOP_STEERING_ANGLE);
    MAP_TO_VDB(loi_, pipeIndex, 4, PotentialPlane, SteeringAngle_VDB, pset, VDB_DOP_STEERING_ANGLE);
    MAP_TO_VDB(loi_, pipeIndex, 0, SampleVolume, Size_VDB, pset, VDB_DOP_SV_SIZE_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 2, StaticPw, Size_VDB, pset, VDB_DOP_SV_SIZE_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 2, StaticPw, CurrentMaxAxialPosition_VDB, pset, VDB_DOP_CURRENT_MAX_SV_DEPTH_STATIC_PW);
    MAP_TO_VDB(loi_, pipeIndex, 2, StaticPw, SteeringAngle_VDB, pset, VDB_DOP_STEERING_ANGLE);
    MAP_TO_VDB(loi_, pipeIndex, 0, DopplerVolume, Depth_VDB, pset, VDB_DOP_AXIAL_POI_POSITION_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 0, DopplerVolume, LateralPosition_VDB, pset, VDB_DOP_LATERAL_POI_POSITION_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 3, PotentialPlaneShape, ColorPwShape_VDB, pset, VDB_DOP_SCAN_PLANE_FORMAT_TARGET);

    // M-mode
    MAP_TO_VDB(loi_, pipeIndex, 0, Mline, LateralPosition_VDB, pset, VDB_MM_ECHO_LATERAL_POI_POSITION_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 0, Mline, AxialPosition_VDB, pset, VDB_MM_ECHO_AXIAL_POI_POSITION_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 2, AcquisitionRegion, StartDepth_VDB, pset, VDB_MM_ECHO_START_DEPTH_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 2, AcquisitionRegion, StopDepth_VDB, pset, VDB_MM_ECHO_STOP_DEPTH_TARGET);

    // Acquisition state
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, EchoRegion, Name_VDB, pset, VDB_ASP_ECHO_2D_ECHO_ACQUISITION_STATE);

    // QFlow (color without elasto)
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, QFlowObject, QFlowEnabled_VDB, 0, VDB_COLOR_2D_QFLOW_ENABLED, ColorWithoutElasto(ctx));
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, QFlowObject, QFlowEnabled_VDB, 1, VDB_COLOR_2D_QFLOW_ENABLED, (ColorWithoutElasto(ctx) && MapSecondaryPlane(ctx)));
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, PotentialPlane, TiltDepth_VDB, 0, VDB_COLOR_2D_QFLOW_POINT_DEPTH_TARGET, ColorWithoutElasto(ctx));
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, PotentialPlane, TiltAngle_VDB, 0, VDB_COLOR_2D_QFLOW_POINT_ELEVATION_TARGET, ColorWithoutElasto(ctx));
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, PotentialPlane, TiltAngle_VDB, 0, VDB_COLOR_2D_QFLOW_POINT_WIDTH_TARGET, ColorWithoutElasto(ctx));

    // Color depth (color without elasto 2D only)
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 1, AcquisitionRegion, StartDepth_VDB, pset, VDB_COLOR_2D_START_DEPTH_TARGET, ColorWithoutElasto2DOnly(ctx));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 1, AcquisitionRegion, StopDepth_VDB, pset, VDB_COLOR_2D_STOP_DEPTH_TARGET, ColorWithoutElasto2DOnly(ctx));

    // Elastography blocks driven by flavors in ctx
    MAP_TO_VDB_IF(loi_, pipeIndex, 402, AcquisitionRegion, StartWidth_VDB, pset, VDB_2D_ECHO_ELASTPQ_START_WIDTH_TARGET, ctx.m_strainFlavor);
    MAP_TO_VDB_IF(loi_, pipeIndex, 402, AcquisitionRegion, StartWidth_VDB, pset, VDB_COLOR_2D_START_WIDTH_TARGET, ctx.m_strainFlavor);
    MAP_TO_VDB_IF(loi_, pipeIndex, 402, AcquisitionRegion, StopWidth_VDB, pset, VDB_2D_ECHO_ELASTPQ_STOP_WIDTH_TARGET, ctx.m_strainFlavor);
    MAP_TO_VDB_IF(loi_, pipeIndex, 402, AcquisitionRegion, StopWidth_VDB, pset, VDB_COLOR_2D_STOP_WIDTH_TARGET, ctx.m_strainFlavor);
    MAP_TO_VDB_IF(loi_, pipeIndex, 402, AcquisitionRegion, StartDepth_VDB, pset, VDB_2D_ECHO_ELASTPQ_START_DEPTH_TARGET, ctx.m_strainFlavor);
    MAP_TO_VDB_IF(loi_, pipeIndex, 402, AcquisitionRegion, StartDepth_VDB, pset, VDB_COLOR_2D_START_DEPTH_TARGET, ctx.m_strainFlavor);
    MAP_TO_VDB_IF(loi_, pipeIndex, 402, AcquisitionRegion, StopDepth_VDB, pset, VDB_2D_ECHO_ELASTPQ_STOP_DEPTH_TARGET, ctx.m_strainFlavor);
    MAP_TO_VDB_IF(loi_, pipeIndex, 402, AcquisitionRegion, StopDepth_VDB, pset, VDB_COLOR_2D_STOP_DEPTH_TARGET, ctx.m_strainFlavor);

    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StartWidth_VDB, pset, VDB_2D_ECHO_ELASTPQ_START_WIDTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StartWidth_VDB, pset, VDB_COLOR_2D_START_WIDTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StopWidth_VDB, pset, VDB_2D_ECHO_ELASTPQ_STOP_WIDTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StopWidth_VDB, pset, VDB_COLOR_2D_STOP_WIDTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StartDepth_VDB, pset, VDB_2D_ECHO_ELASTPQ_START_DEPTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StartDepth_VDB, pset, VDB_COLOR_2D_START_DEPTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StopDepth_VDB, pset, VDB_2D_ECHO_ELASTPQ_STOP_DEPTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StopDepth_VDB, pset, VDB_COLOR_2D_STOP_DEPTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));

    MAP_TO_VDB_IF(loi_, pipeIndex, 500, ShearElastoRegion, ElastoStrikeZoneStartWidth_VDB, 0, VDB_2D_ECHO_ELASTPQ_STRIKE_ZONE_START_WIDTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 500, ShearElastoRegion, ElastoStrikeZoneStopWidth_VDB, 0, VDB_2D_ECHO_ELASTPQ_STRIKE_ZONE_STOP_WIDTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 500, ShearElastoRegion, ElastoStrikeZoneStartDepth_VDB, 0, VDB_2D_ECHO_ELASTPQ_STRIKE_ZONE_START_DEPTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 500, ShearElastoRegion, ElastoStrikeZoneStopDepth_VDB, 0, VDB_2D_ECHO_ELASTPQ_STRIKE_ZONE_STOP_DEPTH_TARGET, (ctx.m_shearFlavor && !ctx.m_strainFlavor));

    MAP_TO_VDB_IF(loi_, pipeIndex, 602, AcquisitionRegion, StartWidth_VDB, pset, VDB_2D_ECHO_ELASTPQ_START_WIDTH_TARGET, (ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 602, AcquisitionRegion, StartWidth_VDB, pset, VDB_COLOR_2D_START_WIDTH_TARGET, (ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 602, AcquisitionRegion, StopWidth_VDB, pset, VDB_2D_ECHO_ELASTPQ_STOP_WIDTH_TARGET, (ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 602, AcquisitionRegion, StopWidth_VDB, pset, VDB_COLOR_2D_STOP_WIDTH_TARGET, (ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 602, AcquisitionRegion, StartDepth_VDB, pset, VDB_2D_ECHO_ELASTPQ_START_DEPTH_TARGET, (ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 602, AcquisitionRegion, StartDepth_VDB, pset, VDB_COLOR_2D_START_DEPTH_TARGET, (ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 602, AcquisitionRegion, StopDepth_VDB, pset, VDB_2D_ECHO_ELASTPQ_STOP_DEPTH_TARGET, (ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 602, AcquisitionRegion, StopDepth_VDB, pset, VDB_COLOR_2D_STOP_DEPTH_TARGET, (ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));

    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StartWidth_VDB, pset, VDB_2D_ECHO_ELASTPQ_START_WIDTH_TARGET, (!ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StopWidth_VDB, pset, VDB_2D_ECHO_ELASTPQ_STOP_WIDTH_TARGET, (!ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StartDepth_VDB, pset, VDB_2D_ECHO_ELASTPQ_START_DEPTH_TARGET, (!ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));
    MAP_TO_VDB_IF(loi_, pipeIndex, 502, AcquisitionRegion, StopDepth_VDB, pset, VDB_2D_ECHO_ELASTPQ_STOP_DEPTH_TARGET, (!ctx.m_sswiFlavor && !ctx.m_shearFlavor && !ctx.m_strainFlavor));

    // Atten imaging
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, AttenImagingRegion, StartDepth_VDB, pset, VDB_2D_ATTEN_START_DEPTH_TARGET, ctx.m_isAttenImaging);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, AttenImagingRegion, StartWidth_VDB, pset, VDB_2D_ATTEN_START_WIDTH_TARGET, ctx.m_isAttenImaging);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, AttenImagingRegion, StopDepth_VDB, pset, VDB_2D_ATTEN_STOP_DEPTH_TARGET, ctx.m_isAttenImaging);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, AttenImagingRegion, StopWidth_VDB, pset, VDB_2D_ATTEN_STOP_WIDTH_TARGET, ctx.m_isAttenImaging);

    // Card behavior guard
    MAP_TO_VDB_IF(loi_, pipeIndex, 11, AcquisitionRegion, StartWidth_VDB, pset, VDB_ASP_ECHO_2D_ZOOM_BOX_START_ELEVATION_TARGET, !ctx.m_isCardBehavior);
    MAP_TO_VDB_IF(loi_, pipeIndex, 11, AcquisitionRegion, StopWidth_VDB, pset, VDB_ASP_ECHO_2D_ZOOM_BOX_STOP_ELEVATION_TARGET, !ctx.m_isCardBehavior);

    // Elevation graphics (not biplane)
    MAP_TO_VDB(loi_, pipeIndex, 11, AcquisitionRegion, StartWidth_VDB, 0, VDB_THREED_START_ELEVATION_USI_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 11, AcquisitionRegion, StopWidth_VDB, 0, VDB_THREED_STOP_ELEVATION_USI_TARGET);
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, ElevationRegion, ElevationRoiGraphicStartWidth_VDB, 0, VDB_ASP_ECHO_2D_ZOOM_BOX_START_ELEVATION_TARGET, !MapSecondaryPlane(ctx));
    MAP_TO_VDB_IF(loi_, pipeIndex, 1, ElevationRegion, ElevationRoiGraphicStopWidth_VDB, 0, VDB_ASP_ECHO_2D_ZOOM_BOX_STOP_ELEVATION_TARGET, !MapSecondaryPlane(ctx));
    MAP_TO_VDB(loi_, pipeIndex, 1, ElevationRegion, ElevationPosition_VDB, 0, VDB_MATRIX_3D_ELEVATION_IMAGING_POSITION);

    // 3D viz trim box (only when 3D session active)
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, ThreeDStandbyTrimBox, StopWidth_VDB, 0, VDB_ASP_THREED_TRIM_BOX_GRAPHICS_STOP_WIDTH_TARGET, ctx.m_is3dSession);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, ThreeDStandbyTrimBox, StartWidth_VDB, 0, VDB_ASP_THREED_TRIM_BOX_GRAPHICS_START_WIDTH_TARGET, ctx.m_is3dSession);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, ThreeDStandbyTrimBox, StartDepth_VDB, 0, VDB_ASP_THREED_TRIM_BOX_GRAPHICS_START_DEPTH_TARGET, ctx.m_is3dSession);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, ThreeDStandbyTrimBox, StopDepth_VDB, 0, VDB_ASP_THREED_TRIM_BOX_GRAPHICS_STOP_DEPTH_TARGET, ctx.m_is3dSession);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, ThreeDStandbyTrimBox, CurveApexPtAngleOffset_VDB, 0, VDB_ASP_THREED_TRIM_BOX_GRAPHICS_ADJUSTCURVE_OFFSET_X, ctx.m_is3dSession);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, ThreeDStandbyTrimBox, CurveApexPtRadiusOffset_VDB, 0, VDB_ASP_THREED_TRIM_BOX_GRAPHICS_ADJUSTCURVE_OFFSET_Y, ctx.m_is3dSession);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, ThreeDStandbyTrimBox, CurveLineEnabled_VDB, 0, VDB_ASP_THREED_TRIM_BOX_GRAPHICS_ADJUSTCURVE_ENABLED, ctx.m_is3dSession);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, ThreeDStandbyTrimBox, Enabled_VDB, 0, VDB_ASP_THREED_TRIM_BOX_GRAPHICS_VISIBLE, ctx.m_is3dSession);

    // Full volume mode - keep at the end to override other mappings
    if (ctx.m_isMatrixFullVolumeMode)
    {
        UlscMapper::vdbSet(pipeIndex, 0, VDB_THREED_IS_FULL_VOLUME_WORKFLOW, ctx.m_isFullVolumeConfig);
    }
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, FullVolumePotentialEchoArea, ActiveEchoThreedOpt_VDB, 0, VDB_THREED_ECHO_OPT, ctx.m_isMatrixFullVolumeMode);
    MAP_TO_VDB_IF(loi_, pipeIndex, 0, FullVolumePotentialEchoArea, ActiveColorThreedOpt_VDB, 0, VDB_THREED_FLOW_OPT, (ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));

    // Full-volume color/echo gates based on zoom and color
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StartDepth_VDB, 0, VDB_COLOR_2D_START_DEPTH_TARGET, (!ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StopDepth_VDB, 0, VDB_COLOR_2D_STOP_DEPTH_TARGET, (!ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 1, AcquisitionRegion, StartWidth_VDB, 0, VDB_COLOR_2D_START_WIDTH_TARGET, (!ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 1, AcquisitionRegion, StopWidth_VDB, 0, VDB_COLOR_2D_STOP_WIDTH_TARGET, (!ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StartWidth_VDB, 0, VDB_2D_ECHO_START_WIDTH_USI_TARGET, (!ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StopWidth_VDB, 0, VDB_2D_ECHO_STOP_WIDTH_USI_TARGET, (!ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));

    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 1, AcquisitionRegion, StartWidth_VDB, 0, VDB_COLOR_2D_START_WIDTH_TARGET, (ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 1, AcquisitionRegion, StopWidth_VDB, 0, VDB_COLOR_2D_STOP_WIDTH_TARGET, (ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 1, AcquisitionRegion, StartDepth_VDB, 0, VDB_COLOR_2D_START_DEPTH_TARGET, (ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 1, AcquisitionRegion, StopDepth_VDB, 0, VDB_COLOR_2D_STOP_DEPTH_TARGET, (ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StartWidth_VDB, 0, VDB_2D_ECHO_START_WIDTH_USI_TARGET, (ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StopWidth_VDB, 0, VDB_2D_ECHO_STOP_WIDTH_USI_TARGET, (ctx.m_isZoomAcquiring && ctx.m_isMatrixFullVolumeMode && ctx.m_isColorSingleton));

    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StartWidth_VDB, 0, VDB_2D_ECHO_START_WIDTH_USI_TARGET, (ctx.m_isMatrixFullVolumeMode && !ctx.m_isColorSingleton));
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE_IF(loi_, pipeIndex, 0, AcquisitionRegion, StopWidth_VDB, 0, VDB_2D_ECHO_STOP_WIDTH_USI_TARGET, (ctx.m_isMatrixFullVolumeMode && !ctx.m_isColorSingleton));
}
void Mapper::mapOci(int pipeIndex)
{
    // Displayed Primary Plane Rotation - Mapped to pset 0
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, DisplayedRotationAngle_VDB, OCI_TISSUE_PSET, VDB_ASP_ECHO_2D_MATRIX_PLANE_POSITION);
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, DisplayedRotationAngle_VDB, OCI_CONTRAST_PSET, VDB_ASP_ECHO_2D_MATRIX_PLANE_POSITION);

    // Scanplane shape
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlaneShape, Shape_VDB, OCI_TISSUE_PSET, VDB_SYS_SCAN_PLANE_FORMAT);
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlaneShape, Shape_VDB, OCI_CONTRAST_PSET, VDB_SYS_SCAN_PLANE_FORMAT);
}

void Mapper::mapMultipleEchoLoiToVdb(int pipeIndex)
{
    // The Rotation angle and tilt the lateral and the elevation plane can be different so map each
    // appropriately.

    // Primary Plane Tilt - Mapped to pset 0
    MAP_TO_VDB(loi_, pipeIndex,    0,    PotentialPlane,                TiltAngle_VDB,     OCI_TISSUE_PSET,  VDB_2D_ECHO_TILT_STEER_POSITION_TARGET);
    MAP_TO_VDB(loi_, pipeIndex,    0,    PotentialPlane,                TiltAngle_VDB,     OCI_CONTRAST_PSET,  VDB_2D_ECHO_TILT_STEER_POSITION_TARGET);
    // Primary Plane Rotation - Mapped to pset 0
    MAP_TO_VDB(loi_, pipeIndex,    0,    PotentialPlane,                RotationAngle_VDB, OCI_TISSUE_PSET,  VDB_2D_ECHO_MATRIX_PLANE_POSITION_TARGET);
    MAP_TO_VDB(loi_, pipeIndex,    0,    PotentialPlane,                RotationAngle_VDB, OCI_CONTRAST_PSET,  VDB_2D_ECHO_MATRIX_PLANE_POSITION_TARGET);

    // Primary Plane Tilt Active
    MAP_TO_VDB(loi_, pipeIndex,    0,    PotentialPlane,                TiltActive_VDB,    OCI_TISSUE_PSET, VDB_2D_ECHO_MC_TILT_ENABLED);
    MAP_TO_VDB(loi_, pipeIndex,    0,    PotentialPlane,                TiltActive_VDB,    OCI_CONTRAST_PSET, VDB_2D_ECHO_MC_TILT_ENABLED);

    // X Plane Rotate Tilt Active
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, isxPlaneRotatedAndTilted_VDB, OCI_TISSUE_PSET, VDB_2D_ECHO_IS_BIPLANE_ROTATED_TILTED);
    MAP_TO_VDB(loi_, pipeIndex, 0, PotentialPlane, isxPlaneRotatedAndTilted_VDB, OCI_CONTRAST_PSET, VDB_2D_ECHO_IS_BIPLANE_ROTATED_TILTED);

      // The active Echo Region for the Primary plane
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion,        StartWidth_VDB,             OCI_TISSUE_PSET,  VDB_2D_ECHO_START_WIDTH_USI_TARGET        );
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion,        StartWidth_VDB,             OCI_CONTRAST_PSET,  VDB_2D_ECHO_START_WIDTH_USI_TARGET        );

    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion,        StopWidth_VDB,              OCI_TISSUE_PSET,  VDB_2D_ECHO_STOP_WIDTH_USI_TARGET         );
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion,        StopWidth_VDB,              OCI_CONTRAST_PSET,  VDB_2D_ECHO_STOP_WIDTH_USI_TARGET         );

    // Zoom box for the primary plane
    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion,      StartWidth_VDB,             OCI_TISSUE_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_START_WIDTH_TARGET );
    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion,      StartWidth_VDB,             OCI_CONTRAST_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_START_WIDTH_TARGET );

    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion,      StopWidth_VDB,              OCI_TISSUE_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_STOP_WIDTH_TARGET  );
    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion,      StopWidth_VDB,              OCI_CONTRAST_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_STOP_WIDTH_TARGET  );

    MAP_TO_VDB(loi_, pipeIndex, 0,   Eline,                    LateralPosition_VDB,        OCI_TISSUE_PSET,  VDB_SYS_TWOD_CURSOR_LATERAL_POI_POSITION_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 0,   Eline,                    LateralPosition_VDB,        OCI_CONTRAST_PSET,  VDB_SYS_TWOD_CURSOR_LATERAL_POI_POSITION_TARGET);

    MAP_TO_VDB(loi_, pipeIndex, 0,   Eline,                    AxialPosition_VDB,          OCI_TISSUE_PSET,  VDB_SYS_TWOD_CURSOR_AXIAL_POI_POSITION_TARGET);
    MAP_TO_VDB(loi_, pipeIndex, 0,   Eline,                    AxialPosition_VDB,          OCI_CONTRAST_PSET,  VDB_SYS_TWOD_CURSOR_AXIAL_POI_POSITION_TARGET);

    MAP_TO_VDB(loi_, pipeIndex, 200, HighDefinitionZoomRegion, ZoomGraphicsDisplayed_VDB,  OCI_TISSUE_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_VISIBLE);
    MAP_TO_VDB(loi_, pipeIndex, 200, HighDefinitionZoomRegion, ZoomGraphicsDisplayed_VDB,  OCI_CONTRAST_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_VISIBLE);

    //The active Echo Region
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion,        StartDepth_VDB,             OCI_TISSUE_PSET,  VDB_2D_ECHO_START_DEPTH_TARGET        );
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion,        StartDepth_VDB,             OCI_CONTRAST_PSET,  VDB_2D_ECHO_START_DEPTH_TARGET        );

    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion,        StopDepth_VDB,              OCI_TISSUE_PSET,  VDB_2D_ECHO_STOP_DEPTH_TARGET         );
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion,        StopDepth_VDB,              OCI_CONTRAST_PSET,  VDB_2D_ECHO_STOP_DEPTH_TARGET         );

    if (loi_HighDefinitionZoomRegion_c::isZoomAcquiring(pipeIndex + 1))
    {
        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StartDepthUnzoomed_VDB, OCI_TISSUE_PSET, VDB_2D_ECHO_START_DEPTH_UNZOOMED_TARGET);
        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StartDepthUnzoomed_VDB, OCI_CONTRAST_PSET, VDB_2D_ECHO_START_DEPTH_UNZOOMED_TARGET);

        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StopDepthUnzoomed_VDB, OCI_TISSUE_PSET, VDB_2D_ECHO_STOP_DEPTH_UNZOOMED_TARGET);
        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StopDepthUnzoomed_VDB, OCI_CONTRAST_PSET, VDB_2D_ECHO_STOP_DEPTH_UNZOOMED_TARGET);

        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StartWidthUnzoomed_VDB, OCI_TISSUE_PSET, VDB_2D_ECHO_START_WIDTH_UNZOOMED_TARGET);
        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StartWidthUnzoomed_VDB, OCI_CONTRAST_PSET, VDB_2D_ECHO_START_WIDTH_UNZOOMED_TARGET);

        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StopWidthUnzoomed_VDB, OCI_TISSUE_PSET, VDB_2D_ECHO_STOP_WIDTH_UNZOOMED_TARGET);
        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StopWidthUnzoomed_VDB, OCI_CONTRAST_PSET, VDB_2D_ECHO_STOP_WIDTH_UNZOOMED_TARGET);
    }
    else
    {
        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StartDepth_VDB, OCI_TISSUE_PSET, VDB_2D_ECHO_START_DEPTH_UNZOOMED_TARGET);
        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StartDepth_VDB, OCI_CONTRAST_PSET, VDB_2D_ECHO_START_DEPTH_UNZOOMED_TARGET);

        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StopDepth_VDB, OCI_TISSUE_PSET, VDB_2D_ECHO_STOP_DEPTH_UNZOOMED_TARGET);
        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StopDepth_VDB, OCI_CONTRAST_PSET, VDB_2D_ECHO_STOP_DEPTH_UNZOOMED_TARGET);

        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StartWidth_VDB, OCI_TISSUE_PSET, VDB_2D_ECHO_START_WIDTH_UNZOOMED_TARGET);
        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StartWidth_VDB, OCI_CONTRAST_PSET, VDB_2D_ECHO_START_WIDTH_UNZOOMED_TARGET);

        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StopWidth_VDB, OCI_TISSUE_PSET, VDB_2D_ECHO_STOP_WIDTH_UNZOOMED_TARGET);
        MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, AcquisitionRegion, StopWidth_VDB, OCI_CONTRAST_PSET, VDB_2D_ECHO_STOP_WIDTH_UNZOOMED_TARGET);
    }

    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, EchoRegion,               WideScreenEnabled_VDB,      OCI_TISSUE_PSET,  VDB_2D_ECHO_SCT_WIDE_SCREEN_ENABLED   );
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, EchoRegion,               WideScreenEnabled_VDB,      OCI_CONTRAST_PSET,  VDB_2D_ECHO_SCT_WIDE_SCREEN_ENABLED   );

    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, EchoRegion,               VrocEnabled_VDB,            OCI_TISSUE_PSET,   VDB_2D_ECHO_VROC_IMAGING_ENABLED);
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, EchoRegion,               VrocEnabled_VDB,            OCI_CONTRAST_PSET, VDB_2D_ECHO_VROC_IMAGING_ENABLED);

    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, EchoRegion,               DisplayStyle_VDB,           OCI_TISSUE_PSET,  VDB_2D_ECHO_ECHO_REGION_DISPLAY_STYLE );
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0, EchoRegion,               DisplayStyle_VDB,           OCI_CONTRAST_PSET,  VDB_2D_ECHO_ECHO_REGION_DISPLAY_STYLE );

    //Map the Acquisition Zoom Region, whether or not it is actively being used to acquire echo data.
    //These VDB parameters are subscribed to by Annotated Signal Path.
    //Another attribute, VDB_ASP_ECHO_2D_ECHO_ACQUISITION_STATE, tells ASP which region is active.
    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion,      StartDepth_VDB,             OCI_TISSUE_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_START_DEPTH_TARGET );
    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion,      StartDepth_VDB,             OCI_CONTRAST_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_START_DEPTH_TARGET );

    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion,      StopDepth_VDB,              OCI_TISSUE_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_STOP_DEPTH_TARGET  );
    MAP_TO_VDB(loi_, pipeIndex, 206, AcquisitionRegion,      StopDepth_VDB,              OCI_CONTRAST_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_STOP_DEPTH_TARGET  );

    //-----------------------------------------------------------------------------------------------
    // See Focal Point/Region comments in mapLoiToVdb for additional info.
    //-------------------------------------------------------------------------------------------------

    //There is one focal region.
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 800,  LocationofFocalInterest,Depth_VDB,                  OCI_TISSUE_PSET,  VDB_TWOD_FOCAL_POSITION_TARGET);
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 800,  LocationofFocalInterest,Depth_VDB,                  OCI_CONTRAST_PSET,  VDB_TWOD_FOCAL_POSITION_TARGET);

    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 800,  FocalRegion,            StartDepth_VDB,             OCI_TISSUE_PSET,  VDB_2D_ECHO_START_FOCAL_REGION_TARGET);
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 800,  FocalRegion,            StartDepth_VDB,             OCI_CONTRAST_PSET,  VDB_2D_ECHO_START_FOCAL_REGION_TARGET);

    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 800,  FocalRegion,            StopDepth_VDB,              OCI_TISSUE_PSET,  VDB_2D_ECHO_STOP_FOCAL_REGION_TARGET);
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 800,  FocalRegion,            StopDepth_VDB,              OCI_CONTRAST_PSET,  VDB_2D_ECHO_STOP_FOCAL_REGION_TARGET);

     //---------------------------------------------------------------------
    //Active Echo Region.
    //---------------------------------------------------------------------
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0,     EchoRegion,             Name_VDB,        OCI_TISSUE_PSET,  VDB_ASP_ECHO_2D_ECHO_ACQUISITION_STATE);
    MAP_TO_VDB_FROM_INSTANCE_OR_STALE_CACHE(loi_, pipeIndex, 0,     EchoRegion,             Name_VDB,        OCI_CONTRAST_PSET,  VDB_ASP_ECHO_2D_ECHO_ACQUISITION_STATE);


     //CQ 16331: This conditional is in place to avoid mapping down 2 values for the same VDB parameters while in card behavior.
    //Note:  isWorkFlowCardBehavior evaluates to true for ie and false for iu, even for cardiology specific TSP's on iu.

    bool isCardBehavior = loi_LOIInitializer_c::isWorkFlowCardBehavior();

    if (!isCardBehavior)
    {
        MAP_TO_VDB(loi_, pipeIndex, 11,     AcquisitionRegion,       StartWidth_VDB,                   OCI_TISSUE_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_START_ELEVATION_TARGET);
        MAP_TO_VDB(loi_, pipeIndex, 11,     AcquisitionRegion,       StartWidth_VDB,                   OCI_CONTRAST_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_START_ELEVATION_TARGET);

        MAP_TO_VDB(loi_, pipeIndex, 11,     AcquisitionRegion,       StopWidth_VDB,                    OCI_TISSUE_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_STOP_ELEVATION_TARGET);
        MAP_TO_VDB(loi_, pipeIndex, 11,     AcquisitionRegion,       StopWidth_VDB,                    OCI_CONTRAST_PSET,  VDB_ASP_ECHO_2D_ZOOM_BOX_STOP_ELEVATION_TARGET);
    }

}